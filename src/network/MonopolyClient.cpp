#include "network/MonopolyClient.h"
#include "network/internal/MessageFormat.h"
#include "network/MonopolySerialization.h"

namespace Monopoly::Network {


    Client::Client(const sockpp::inet_address& addr, std::unique_ptr<sockpp::stream_socket> socket,
                   std::function<void()> onDeath)
            : addr(addr), socket(std::move(socket)), onDeath(std::move(onDeath)) {}

    Client::Client(Client&& other) noexcept :
            th(std::move(other.th)), addr(other.addr), socket(std::move(other.socket)),
            model(std::move(other.model)), onDeath(std::move(other.onDeath)) {}

    void Client::mainLoop(Token token, std::string name) {
        using Messages::MessageType;
        auto& sock = *socket;
        {
            std::lock_guard g(socketMutex);
            if (!sock.is_open()) return;
            Messages::send(sock, MessageType::INITIALIZE,
                    Serialization::serializeInitialization({token, std::move(name)}));
        }
        while (true) {
            std::lock_guard g(socketMutex);
            if (!sock.is_open()) break;
            if (auto rep = model.getReply(); rep.has_value())
                Messages::send(sock, MessageType::REPLY, Serialization::serializeReply(rep.value()));
            if (auto msg = Messages::receive(sock, true); msg.has_value()) {
                const auto&[type, content] = msg.value();
                switch (type) {
                    case MessageType::BOARD: {
                        model.sync(Serialization::deserializeBoard(content));
                        break;
                    }
                    case MessageType::REQUEST: {
                        model.processRequestAsync(Serialization::deserializeRequest(content));
                        break;
                    }
                    default:
                        throw Messages::InvalidMessageError("Неожиданный тип сообщения от сервера");
                }
            }
        }
        onDeath();
    }

    const BoardModel& Client::getBoard() {
        if(model.isDirty()) {
            cachedModel = model.getBoard();
        }
        return cachedModel;
    }

    void Client::close() {
        std::lock_guard g(socketMutex);
        socket->close();
    }
}