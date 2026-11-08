#include <utility>
#include "Game.h"
#include "Player.h"

NetworkGame::NetworkGame(NetworkGame::Client client, Player& player)
: client(std::move(client)), player(player) {}

void NetworkGame::run() {
    client.mainLoop(player.token, player.name);
}

const BoardModel& NetworkGame::getBoard() {
    return client.getBoard();
}

void NetworkGame::terminate() {
    client.close();
}
