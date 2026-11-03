#include "threads/ModelThreadManager.h"
#include <cassert>

namespace Monopoly::Threads {

    void ModelThreadManager::sync(BoardModel board) {
        std::lock_guard g(boardMutex);
        model = std::move(board);
        dirty = true;
    }

    PlayerReply ModelThreadManager::processRequest(PlayerRequest req) {
        std::unique_lock g(requestMutex);
        if(req->message.message.empty() && curRequest) {
            req->message = std::move(curRequest.value()->message);
            curRequest.value()->message.message.clear();
        }
        curRequest = std::move(req);
        if(curRequest.value()->type != RequestType::MESSAGE) {
            requestCond.wait(g, [this]() { return bool(this->curReply); });
            assert(curReply);
            return std::move(curReply);
        } else {
            return nullptr;
        }
    }

    void ModelThreadManager::processRequestAsync(PlayerRequest req) {
        std::unique_lock g(requestMutex);
        curRequest = std::move(req);
    }

    std::optional<PlayerRequest> ModelThreadManager::getRequest() {
        std::lock_guard g(requestMutex);
        if (!curRequest.has_value()) return {};
        std::optional<PlayerRequest> ret(std::move(curRequest.value()));
        curRequest.reset();
        return ret;
    }

    std::optional<PlayerReply> ModelThreadManager::getReply() {
        std::lock_guard g(requestMutex);
        if (curReply) return {std::move(curReply)};
        else return {};
    }

    void ModelThreadManager::sendReply(PlayerReply reply) {
        std::lock_guard g(requestMutex);
        curReply = std::move(reply);
        requestCond.notify_all();
    }

    BoardModel ModelThreadManager::getBoard() {
        std::lock_guard g(boardMutex);
        dirty = false;
        return model; //copy
    }

    bool ModelThreadManager::isDirty() {
        std::lock_guard g(boardMutex);
        return dirty;
    }

    bool ModelThreadManager::hasRequest(RequestType type) {
        std::lock_guard g(requestMutex);
        return curRequest.has_value() && curRequest.value()->type == type;
    }

    ModelThreadManager::ModelThreadManager(ModelThreadManager&& other) noexcept
            : model(std::move(other.model)), dirty(other.dirty), curRequest(std::move(other.curRequest)),
              curReply(std::move(other.curReply)) {}

    RequestMessage ModelThreadManager::getMessage() {
        std::lock_guard g(requestMutex);
        RequestMessage msg;
        if(curRequest) {
            msg = std::move(curRequest.value()->message);
            curRequest.value()->message.message.clear();
        }
        return msg;
    }



}