#ifndef GAME_H
#define GAME_H

#include "MonopolyFwd.h"
#include "Board.h"
#include "PlayerRequests.h"
#include "threads/BoardModel.h"
#include "network/MonopolyClient.h"
#include <string_view>
#include <memory>
#include <vector>
#include "Random.h"

struct GameBase {
    virtual ~GameBase() = default;
    virtual void run() = 0;
    virtual const BoardModel& getBoard() = 0;
    virtual void terminate() = 0;
};

class LocalGame final : public GameBase {
public:
    LocalGame(const std::vector<std::pair<std::string_view, Token>>& players, Manager&);
    void run() override;
    PlayerReply send(PlayerRequest request);
    const BoardModel& getBoard() override;
    void terminate() override;
    void sync();

private:
    RandomSource rng;
    Board board;
    BoardModel model;
    Manager& manager;
};

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
