#pragma once

#include <functional>
#include <unordered_map>
#include <thread>
#include <sockpp/tcp_socket.h>
#include "include/threads/BoardModel.h"
#include "threads/ModelThreadManager.h"

namespace Monopoly::Network {

    class Client {
    public:
        Client(const sockpp::inet_address& addr, std::unique_ptr<sockpp::stream_socket> socket,
               std::function<void()> onDeath);
        Client(Client&& other) noexcept;

        void mainLoop(Token token, std::string name);
        const BoardModel& getBoard();
        void close();
    private:
        std::thread th;
        std::mutex socketMutex;
        sockpp::inet_address addr;
        std::unique_ptr<sockpp::stream_socket> socket;
        Threads::ModelThreadManager model;
        BoardModel cachedModel;
        std::function<void()> onDeath;
    };

}