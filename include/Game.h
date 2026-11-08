#ifndef GAME_H
#define GAME_H

#include "LocalGame.h"
#include "MonopolyFwd.h"
#include "network/MonopolyClient.h"

class NetworkGame final : public GameBase {
public:
    using Client = Monopoly::Network::Client;
    explicit NetworkGame(Client client, Player& player);

    void run() override;
    const BoardModel& getBoard() override;
    void terminate() override;
private:
    Client client;
    Player& player;
};

#endif //GAME_H
