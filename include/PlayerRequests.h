#ifndef PLAYERREQUESTS_H
#define PLAYERREQUESTS_H

#include <vector>
#include <string>
#include <memory>
#include <variant>

enum class RequestType : std::size_t {
    ACTION = 0,
    TRADE = 1,
    MESSAGE = 2,
    NUM = 3,
    SUM = 4,
    TOKEN = 5
};

enum class PlayerAction : std::size_t {
    END_TURN = 0,
    PAY_TAX = 1,
    BUY_FIELD = 2,
    BUY_HOUSE = 3,
    BUY_HOTEL = 4,
    SELL_HOUSE = 5,
    SELL_FIELD = 6,
    SELL_HOTEL = 7,
    USE_CARD = 8,
    PAY_TO_OTHER_PLAYER = 9,
    TAKE_CARD = 10,
    MORTGAGE_HOLDINGS = 11,
    BUY_BACK_PROPERTY = 12,
    START_TRADE = 13,
    START_TRADE_NEW_FIELD = 14,
    ROLL_DICE = 15,
    EXIT_GAME = 16,
    LOSE = 17,
    FINISH_GAME = 18
};

enum class PlayerTradeAction {
    REFUSE = 0,
    PARTICIPATE = 1
};

struct RequestMessage {
    std::string message = "";
    MessageType type = MessageType::INFO;
};

struct PlayerRequestData final {
    PlayerRequestData(RequestType type, Token player, std::string message,
                      std::vector<PlayerAction> availableActions, MessageType msgtype);

    RequestType type;
    Token player;
    std::vector<PlayerAction> availableActions;
    RequestMessage message;
};

struct PlayerReplyData final {
    explicit PlayerReplyData(Token player, PlayerAction action);
    explicit PlayerReplyData(Token player, std::size_t num);
    explicit PlayerReplyData(Token player, int sum);
    explicit PlayerReplyData(Token player, Token token);
    explicit PlayerReplyData(Token player, PlayerTradeAction trade);

    const RequestType type;
    const Token player;
    union {
        PlayerAction action;
        std::size_t num;
        int sum;
        Token token;
        PlayerTradeAction trade;
    } data;
};

namespace Monopoly::Requests {

    PlayerRequest action(Token player, std::vector<PlayerAction> actions, std::string msg = "");
    PlayerRequest trade(Token player, std::string msg = "");
    PlayerRequest message(Token player, MessageType msgtype, std::string msg = "");
    PlayerRequest num(Token player, std::string msg = "");
    PlayerRequest sum(Token player, std::string msg = "");
    PlayerRequest token(Token player, std::string msg = "");

}

#endif //PLAYERREQUESTS_H
