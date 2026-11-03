#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include "BoardModel.h"

namespace Monopoly::Threads {
    class ModelThreadManager {
    public:

        ModelThreadManager() = default;
        ModelThreadManager(ModelThreadManager&& other) noexcept;

        void sync(BoardModel board);

        bool isDirty(); // set by sync, reset by getBoard

        bool hasRequest(RequestType type);

        PlayerReply processRequest(PlayerRequest req);

        void processRequestAsync(PlayerRequest req);

        std::optional<PlayerRequest> getRequest();

        std::optional<PlayerReply> getReply();

        void sendReply(PlayerReply reply);

        BoardModel getBoard();

        RequestMessage getMessage();

    private:
        std::mutex boardMutex, requestMutex;
        std::condition_variable requestCond;

        BoardModel model;
        bool dirty = false;

        std::optional<PlayerRequest> curRequest;
        PlayerReply curReply;
    };
}