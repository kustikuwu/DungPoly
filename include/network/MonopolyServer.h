#pragma once

#include <functional>
#include <unordered_map>
#include <thread>
#include <sockpp/tcp_acceptor.h>
#include "include/threads/BoardModel.h"
#include "threads/ModelThreadManager.h"

namespace Monopoly::Network {

    using ClientId = std::size_t;

    class ClientHandler {
    public:
        ClientHandler(ClientId id,
                      const sockpp::inet_address& addr,sockpp::tcp_socket socket,
                      std::function<void(ClientId)> onDeath,
                      std::function<void(Token, std::string, ClientHandler&)> initialize);

        ClientHandler(ClientHandler&& other) noexcept;

        ~ClientHandler();

        void sync(const BoardModel& board);

        PlayerReply sendRequest(Player& p, PlayerRequest request);

        void close();

        void mainLoop();

        ClientId getId() const;

        void start();

    private:

        ClientId id;
        std::thread th;
        std::mutex socketMutex;
        sockpp::inet_address addr;
        sockpp::tcp_socket socket;
        Threads::ModelThreadManager model;
        std::function<void(ClientId)> onDeath;
        std::function<void(Token, std::string, ClientHandler&)> initialize;
    };

    class MonopolyServer {
    public:
        MonopolyServer(Manager& manager);
        ~MonopolyServer();

        void close();

        void mainLoop();

    private:
        Manager& manager;
        ClientId curId = 0;
        std::mutex acceptorMutex;
        sockpp::tcp_acceptor acceptor;
        std::unordered_map<ClientId, ClientHandler> clients;
    };

}