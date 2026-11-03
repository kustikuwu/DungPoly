#include "network/MonopolyServer.h"
#include "MonopolyFwd.h"
#include "Manager.h"
#include <utility>
#include "network/internal/MessageFormat.h"
#include "network/MonopolySerialization.h"

namespace Monopoly::Network {

    ClientHandler::ClientHandler(ClientId id,
                                 const sockpp::inet_address& addr, sockpp::tcp_socket socket,
                                 std::function<void(ClientId)> onDeath,
                                 std::function<void(Token, std::string, ClientHandler&)> initialize)
            : id(id), addr(addr), socket(std::move(socket)),
            onDeath(std::move(onDeath)),
            initialize(std::move(initialize)) {}

    ClientHandler::ClientHandler(ClientHandler&& other) noexcept
            : id(other.id), th(std::move(other.th)), addr(other.addr), socket(std::move(other.socket)),
              model(std::move(other.model)),
              onDeath(std::move(other.onDeath)),
              initialize(std::move(other.initialize)) {}

    ClientHandler::~ClientHandler() {
        close(); //this may actually throw on attempt to lock mutex...
        if(th.joinable()) {
            th.join();
        }
    }

    void ClientHandler::sync(const BoardModel& board) {
        model.sync(board);
    }

    PlayerReply ClientHandler::sendRequest(Player& p, PlayerRequest request) {
        return model.processRequest(std::move(request));
    }

    void ClientHandler::close() {
        std::lock_guard g(socketMutex);
        socket.close();
    }

    void ClientHandler::mainLoop() {
        using Messages::MessageType;
        while (true) {
            std::lock_guard g(socketMutex);
            if (!socket.is_open()) break;
            if (model.isDirty())
                Messages::send(socket, MessageType::BOARD,
                               Serialization::serializeBoard(model.getBoard()));
            if (auto req = model.getRequest(); req.has_value())
                Messages::send(socket, MessageType::REQUEST,
                               Serialization::serializeRequest(req.value()));
            if (auto clientMsg = Messages::receive(socket, true); clientMsg.has_value()) {
                const auto&[type, content] = clientMsg.value();
                switch (type) {
                    case MessageType::INITIALIZE: {
                        auto msg = Serialization::deserializeInitialization(content);
                        initialize(msg.token, std::move(msg.name), *this);
                        break;
                    }
                    case MessageType::REPLY: {
                        model.sendReply(Serialization::deserializeReply(content));
                        break;
                    }
                    default:
                        throw Messages::InvalidMessageError("Неожиданный тип сообщения от клиента");
                }
            }
        }
        onDeath(id);
    }

    ClientId ClientHandler::getId() const {
        return id;
    }

    void ClientHandler::start() {
        th = std::thread([this]() { this->mainLoop(); });
    }

    MonopolyServer::~MonopolyServer() {
        close();
    }

    void MonopolyServer::close() {
        std::lock_guard g(acceptorMutex);
        for (auto&[id, handler] : clients) {
            handler.close();
        }
        acceptor.close();
    }

    void MonopolyServer::mainLoop() {
        while (acceptor.is_open()) {
            sockpp::inet_address addr;
            auto socket = acceptor.accept(&addr);
            if (!socket.is_open()) {
                if (!acceptor.is_open()) break;
                continue;
            }
            std::lock_guard g(acceptorMutex);
            if (!acceptor.is_open()) {
                socket.close();
                break;
            }
            ClientId id = curId++;
            clients.emplace(id, ClientHandler(id, addr, std::move(socket),
                                              [this](ClientId id) { this->clients.erase(id); },
                                              [this](Token token, std::string name, ClientHandler& h){
                manager.addPlayer(std::make_unique<NetworkPlayer>(
                        token, std::move(name), h
                        ));
            }));
            auto& handler = clients.at(id);
            handler.start();
        }
    }

    MonopolyServer::MonopolyServer(Manager& manager) : manager(manager) {
        acceptor.open(NETWORK_PORT);
    }
}