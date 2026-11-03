#ifndef MANAGER_H
#define MANAGER_H

#include "MonopolyFwd.h"
#include "Player.h"
#include "Game.h"
#include "network/MonopolyClient.h"
#include "network/MonopolyServer.h"
#include <atomic>
#include <vector>
#include <memory>
#include <thread>
#include <future>

class Manager {
public:
    ~Manager();
    void addPlayer(std::unique_ptr<Player> player);
    void run();
    void startGame();
    void requestShutdown();
    void startRemoteGame(const sockpp::inet_address& addr);
    bool isGameStarted();
    PlayerReply sendRequest(PlayerRequest request);
    void sync(const BoardModel& board);
private:

    void createView();
    void createGame();
    void createServer();
private:

    std::thread gameThread, serverThread;
    std::atomic_bool shuttingDown = false;
    std::mutex serverMutex, playerMutex;

    std::shared_ptr<SFMLView> view;
    std::shared_ptr<GameBase> game;
    std::shared_ptr<Monopoly::Network::MonopolyServer> server;
    std::vector<std::unique_ptr<Player>> players;
};

#endif //MANAGER_H
