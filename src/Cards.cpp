#include "Cards.h"
#include "Field.h"

#include <algorithm>
#include <random>
#include <Board.h>
#include <iostream>

Card::Card(Board& board, std::string text) : board(board), text(std::move(text)) {}

LeavePrisonForFree::LeavePrisonForFree(Board& board, std::string text)
        : Card(board, std::move(text)) {}

GetMoney::GetMoney(Board& board, std::string text)
        : Card(board, std::move(text)) {}

PayMoney::PayMoney(Board& board, std::string text)
        : Card(board, std::move(text)) {}

GetMoneyFromOtherPlayers::GetMoneyFromOtherPlayers(Board& board, std::string text)
        : Card(board, std::move(text)) {}

PayMoneyToOtherPlayers::PayMoneyToOtherPlayers(Board& board, std::string text)
        : Card(board, std::move(text)) {}


Teleport::Teleport(Board& board, std::string text)
        : Card(board, std::move(text)) {}

TeleportToPrison::TeleportToPrison(Board& board, std::string text)
        : Card(board, std::move(text)) {}

void LeavePrisonForFree::apply(Token token) {
    PlayerData& player = board.getPlayer(token);
    player.cardToLeavePrison = true;
}

void GetMoney::apply(Token token) {
    PlayerData& player = board.getPlayer(token);
    player.addMoney(amount);
}

void PayMoney::apply(Token token) {
    PlayerData& player = board.getPlayer(token);
    FieldTile *curField = board.getTile(player.position);
    bool payTax = false;
    int tmp = amount;
    if (countTax) {
        amount = countPlayerTax(player);
    }
    PlayerRequest request = makeDefaultRequest(token, board);
    while (true) {
        if (!payTax) {
            request->availableActions.push_back(PlayerAction::PAY_TAX);
        }
        PlayerAction action = board.send(std::move(request))->data.action;
        request = makeDefaultRequest(token, board);
        board.sync();
        if (action == PlayerAction::END_TURN) {
            if (!payTax) {
                request->message.message = "Сейчас нельзя завершить ход";
                continue;
            }
        }
        if (action == PlayerAction::PAY_TAX) {
            if (player.getMoney() >= amount) {
                player.addMoney(-amount);
                amount = tmp;
                payTax = true;
                return;
            } else {
                request->message.message = "Недостаточно денег";
                continue;
            }
        }
        if (!handleGenericActions(token, *curField, action)) {
            amount = tmp;
            return;
        }
    }
}

int PayMoney::countPlayerTax(const PlayerData& player) const {
    return player.numberOfHotels * HOTEL_REPAIR_COST + player.numberOfHouses * HOUSE_REPAIR_COST;
}

void GetMoneyFromOtherPlayers::apply(Token token) {
    PayMoney payMoney(board);
    PlayerData& player = board.getPlayer(token);
    for (std::size_t i = 0; i < board.getPlayers().size(); i++) {
        if (!board.getPlayers()[i].alive) {
            continue;
        }
        if (board.getPlayers()[i].token == token) {
            continue;
        }
        board.send(Monopoly::Requests::message(board.getPlayers()[i].token, MessageType::INFO,
                                               "Заплатите " + std::to_string(amount)
                                               + " игроку " + std::string(board.getPlayers()[i].name)));
        board.sync();
        payMoney.apply(board.getPlayers()[i].token);
        player.addMoney(amount);
    }
}

void PayMoneyToOtherPlayers::apply(Token token) {
    PlayerData& player = board.getPlayer(token);
    FieldTile *curField = board.getTile(player.position);
    bool payTax = false;
    int tmp = amount;
    amount = board.getCurNumOfPlayers() * tmp;
    PlayerRequest request = makeDefaultRequest(token, board);
    request->availableActions.push_back(PlayerAction::PAY_TAX);
    while (true) {
        PlayerAction action = board.send(std::move(request))->data.action;
        request = makeDefaultRequest(token, board);
        request->availableActions.push_back(PlayerAction::PAY_TAX);
        board.sync();
        if (action == PlayerAction::END_TURN && !payTax) {
            request->message.message = "Сейчас нельзя завершить ход";
            continue;
        }
        if (action == PlayerAction::PAY_TAX) {
            if (player.getMoney() >= amount) {
                player.addMoney(-amount);
                amount = tmp;
                for (std::size_t i = 0; i < board.getPlayers().size(); i++) {
                    if (!board.getPlayers()[i].alive) {
                        continue;
                    }
                    if (board.getPlayers()[i].token == token) {
                        continue;
                    }
                    board.getPlayer(board.getPlayers()[i].token).addMoney(tmp);
                    board.sync();
                }
                return;
            } else {
                request->message.message = "Недостаточно денег";
                continue;
            }
        }
        if (!handleGenericActions(token, (*curField), action)) {
            amount = tmp;
            return;
        }
    }
}

void Teleport::apply(Token token) {
    PlayerData& player = board.getPlayer(token);
    if (fl) {
        player.newPosition(deltaPos);
    } else {
        player.newPosition(newPosition - (int) player.position);
    }
    player.addMoney(amount);
}

void TeleportToPrison::apply(Token token) {
    PlayerData& player = board.getPlayer(token);
    player.toPrison();
}

void GetMoney::setAmount(uint32_t val) {
    amount = val;
}

void PayMoney::setAmount(uint32_t val) {
    amount = val;
}

void GetMoneyFromOtherPlayers::setAmount(uint32_t val) {
    amount = val;
}

void PayMoneyToOtherPlayers::setAmount(uint32_t val) {
    amount = val;
}

void Teleport::setAmount(uint32_t val) {
    amount = val;
}

CardPool::CardPool() {

}

CardPool::CardPool(Board& board) {
    //pool.push_back(std::make_unique(...))
}

std::unique_ptr<Card> CardPool::takeCard() {
    auto card = std::move(pool.back());
    pool.pop_back();
    return card;
}

void CardPool::returnCard(std::unique_ptr<Card> card) {
    pool.push_back(std::move(card));
}

void CardPool::shuffle() {
    static std::random_device g_rd;
    static std::mt19937 g_gen(g_rd()); //TODO: rewrite with Random.h
    std::shuffle(pool.begin(), pool.end(), g_gen);
}

std::vector<std::unique_ptr<Card>>& CardPool::getCards() {
    return pool;
}

const std::vector<std::unique_ptr<Card>>& CardPool::getCards() const {
    return pool;
}
