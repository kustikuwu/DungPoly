#include "LocalGame.h"
#include "Field.h"
#include <stdexcept>
#include <utility>

LocalGame::LocalGame(const std::vector<std::pair<std::string_view, Token>>& players,
                     RequestHandler requestHandler, SyncHandler syncHandler)
        : board(players, *this),
          requestHandler(std::move(requestHandler)),
          syncHandler(std::move(syncHandler)) {
    if (!this->requestHandler || !this->syncHandler) {
        throw std::invalid_argument("LocalGame requires request and sync handlers");
    }
}

PlayerReply LocalGame::send(PlayerRequest request) {
    sync();
    PlayerReply reply = requestHandler(std::move(request));
    if (reply && reply->type == RequestType::ACTION &&
        reply->data.action == PlayerAction::FINISH_GAME) {
        board.terminate();
    }
    return reply;
}

void LocalGame::run() {
    using namespace Monopoly::Requests;
    std::size_t curPlayerNum = 0;
    while (!board.isFinished()) {
        board.setPlayerIndex(curPlayerNum);
        PlayerData& curPlayer = board.getPlayer(board.getPlayerToken(curPlayerNum));

        if (!curPlayer.alive) {
            curPlayerNum = (curPlayerNum + 1) % board.getPlayersNumber();
            continue;
        }
        const PlayerAction openingAction =
            board.send(action(curPlayer.token, {PlayerAction::ROLL_DICE, PlayerAction::EXIT_GAME}))->data.action;
        if (openingAction == PlayerAction::EXIT_GAME) {
            board.surrenderPlayer(curPlayer.token);
            board.send(message(curPlayer.token, MessageType::INFO, "Вы сдались"));
            curPlayerNum = (curPlayerNum + 1) % board.getPlayersNumber();
            continue;
        }
        if (board.isFinished()) break;
        if (curPlayer.prisoner) {
            board.getField()[curPlayer.position]->onPlayerEntry(curPlayer.token);
            curPlayerNum = (curPlayerNum + 1) % board.getPlayersNumber();
            continue;
        }
        int firstThrow = rng.nextInt(1, 6), secondThrow = rng.nextInt(1, 6);
        curPlayer.lastTrow = firstThrow + secondThrow;

        send(message(curPlayer.token, MessageType::DICE,
                     std::to_string(firstThrow) + " " + std::to_string(secondThrow)));
        bool extraTurn = false;
        if (firstThrow == secondThrow) {
            curPlayer.doubleDice++;
            extraTurn = true;
        } else {
            curPlayer.doubleDice = 0;
        }
        if (curPlayer.doubleDice == 3) {
            extraTurn = false;
            curPlayer.toPrison();
        } else {
            curPlayer.newPosition(firstThrow + secondThrow);
        }
        board.getField()[curPlayer.position]->onPlayerEntry(curPlayer.token);
        if (!extraTurn) {
            curPlayerNum = (curPlayerNum + 1) % board.getPlayersNumber();
        }
        sync();
    }
    if (!board.isFinished()) {
        board.send(message(board.getWinner(), MessageType::INFO, "Игра окончена!"));
    }
    sync();
}

const BoardModel& LocalGame::getBoard() {
    model.update(board);
    return model;
}

void LocalGame::sync() {
    syncHandler(getBoard());
}

void LocalGame::terminate() {
    board.terminate();
}
