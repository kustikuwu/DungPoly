#include <MonopolyFwd.h>
#include "PlayerRequests.h"

PlayerRequestData::PlayerRequestData(RequestType type, Token player, std::string message,
                                     std::vector<PlayerAction> availableActions,
                                     MessageType msgtype)
        : type(type), player(player), availableActions(std::move(availableActions)),
          message({std::move(message), msgtype}) {}

PlayerReplyData::PlayerReplyData(Token player, PlayerAction action)
        : type(RequestType::ACTION), player(player), data() { data.action = action; }

PlayerReplyData::PlayerReplyData(Token player, std::size_t num)
        : type(RequestType::NUM), player(player), data() { data.num = num; }

PlayerReplyData::PlayerReplyData(Token player, int sum)
        : type(RequestType::SUM), player(player), data() { data.sum = sum; }

PlayerReplyData::PlayerReplyData(Token player, Token token)
        : type(RequestType::TOKEN), player(player), data() { data.token = token; }

PlayerReplyData::PlayerReplyData(Token player, PlayerTradeAction trade)
        : type(RequestType::TRADE), player(player), data() { data.trade = trade; }

namespace Monopoly::Requests {

    PlayerRequest action(Token player, std::vector<PlayerAction> actions, std::string msg) {
        return std::make_unique<PlayerRequestData>(RequestType::ACTION, player, std::move(msg),
                                                   std::move(actions), MessageType::INFO);
    }

    PlayerRequest trade(Token player, std::string msg) {
        return std::make_unique<PlayerRequestData>(RequestType::TRADE, player, std::move(msg),
                                                   std::vector<PlayerAction>{}, MessageType::INFO);
    }

    PlayerRequest message(Token player, MessageType msgtype, std::string msg) {
        return std::make_unique<PlayerRequestData>(RequestType::MESSAGE, player, std::move(msg),
                                                   std::vector<PlayerAction>{}, msgtype);
    }

    PlayerRequest num(Token player, std::string msg) {
        return std::make_unique<PlayerRequestData>(RequestType::NUM, player, std::move(msg),
                                                   std::vector<PlayerAction>{}, MessageType::INFO);
    }

    PlayerRequest sum(Token player, std::string msg) {
        return std::make_unique<PlayerRequestData>(RequestType::SUM, player, std::move(msg),
                                                   std::vector<PlayerAction>{}, MessageType::INFO);
    }

    PlayerRequest token(Token player, std::string msg) {
        return std::make_unique<PlayerRequestData>(RequestType::TOKEN, player, std::move(msg),
                                                   std::vector<PlayerAction>{}, MessageType::INFO);
    }
}
