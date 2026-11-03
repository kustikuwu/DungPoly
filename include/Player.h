#ifndef PLAYER_H
#define PLAYER_H

#include "MonopolyFwd.h"
#include "PlayerRequests.h"
#include "network/MonopolyServer.h"
#include <string>
#include <vector>
#include <variant>
#include <memory>

class Player {
public:
    explicit Player(Token token, std::string name);
    virtual ~Player() = default;
    virtual PlayerReply send(PlayerRequest request) = 0;
    virtual void sync(const BoardModel& board) = 0;

    const Token token;
    const std::string name;
};

class NetworkPlayer final : public Player {
public:
    explicit NetworkPlayer(Token token, std::string name, Monopoly::Network::ClientHandler& handler);
    PlayerReply send(PlayerRequest request) override;
    void sync(const BoardModel& board) override;
private:
    Monopoly::Network::ClientHandler& handler;
};

class BotPlayer final : public Player {
public:
    PlayerReply send(PlayerRequest request) override;
    void sync(const BoardModel& board) override;
};

class LocalPlayer final : public Player {
public:
    explicit LocalPlayer(Token token, std::string name, SFMLView& view);
    PlayerReply send(PlayerRequest request) override;
    void sync(const BoardModel& board) override;

private:
    SFMLView& view;
    SFMLView& gameView();
};

#endif //PLAYER_H
