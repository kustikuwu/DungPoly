#include "network/internal/InternalSerialization.h"
#include "PlayerRequests.h"
#include "Field.h"
#include "Cards.h"
#include "Board.h"
#include "include/threads/BoardModel.h"
#include <google/protobuf/port_def.inc>
#include "network/MonopolySerialization.h"
#include <functional>
namespace Monopoly::Serialization::Internal {

    Pb::PlayerRequest serializeRequest(const PlayerRequestData& req) {
        Pb::PlayerRequest ser;
        ser.set_type(Pb::RequestType(req.type));
        ser.set_player(Pb::Token(req.player));
        ser.set_msg(req.message.message);
        for (auto action : req.availableActions) {
            ser.add_availableactions(Pb::PlayerAction(action));
        }
        ser.set_msgtype(Pb::MessageType(req.message.type));
        return ser;
    }

    PlayerRequest deserializeRequest(Pb::PlayerRequest&& ser) {
        std::vector<PlayerAction> availableActions;
        availableActions.reserve(ser.availableactions_size());
        for (auto action : ser.availableactions()) {
            availableActions.push_back(PlayerAction(action));
        }
        return std::make_unique<PlayerRequestData>(RequestType(ser.type()),
                Token(ser.player()), std::move(*ser.mutable_msg()),
                std::move(availableActions), MessageType(ser.msgtype()));
    }

    Pb::PlayerReply serializeReply(const PlayerReplyData& rep) {
        Pb::PlayerReply ser;
        ser.set_type(Pb::RequestType(rep.type));
        ser.set_player(Pb::Token(rep.player));
        switch(rep.type) {
            case RequestType::ACTION: ser.set_action(Pb::PlayerAction(rep.data.action)); break;
            case RequestType::TRADE: ser.set_trade(Pb::PlayerTradeAction(rep.data.trade)); break;
            case RequestType::NUM: ser.set_num(rep.data.num); break;
            case RequestType::SUM: ser.set_sum(rep.data.sum); break;
            case RequestType::TOKEN: ser.set_token(Pb::Token(rep.data.token)); break;
            default: break;
        }
        return ser;
    }

    PlayerReply deserializeReply(Pb::PlayerReply&& ser) {
        auto type = RequestType(ser.type());
        auto player = Token(ser.player());
        switch(type) {
            case RequestType::ACTION: return std::make_unique<PlayerReplyData>(player, PlayerAction(ser.action()));
            case RequestType::TRADE: return std::make_unique<PlayerReplyData>(player, PlayerTradeAction(ser.trade()));
            case RequestType::NUM: return std::make_unique<PlayerReplyData>(player, std::size_t(ser.num()));
            case RequestType::SUM: return std::make_unique<PlayerReplyData>(player, ser.sum());
            case RequestType::TOKEN: return std::make_unique<PlayerReplyData>(player, Token(ser.token()));
            default: return nullptr;
        }
    }

    template<typename T, typename U>
    std::optional<U> apply(std::optional<T> inp, std::function<U(const T&)> func) {
        if (inp.has_value()) return func(inp.value());
        else return {};
    }

    Pb::Taxes serializeTaxes(const Taxes& data) {
        Pb::Taxes ser;
        ser.set_starttax(data.startTax);
        ser.set_taxonehouse(data.taxOneHouse);
        ser.set_taxtwohouses(data.taxTwoHouses);
        ser.set_taxthreehouses(data.taxThreeHouses);
        ser.set_taxfourhouses(data.taxFourHouses);
        ser.set_taxhotel(data.taxHotel);
        return ser;
    }

    Taxes deserializeTaxes(Pb::Taxes&& ser) {
        Taxes ret;
        ret.startTax = ser.starttax();
        ret.taxOneHouse = ser.taxonehouse();
        ret.taxTwoHouses = ser.taxtwohouses();
        ret.taxThreeHouses = ser.taxthreehouses();
        ret.taxFourHouses = ser.taxfourhouses();
        ret.taxHotel = ser.taxhotel();
        return ret;
    }

    template<typename I, typename O>
    std::function<O(const I&)> converter() {
        return [](const I& x) { return O(x); };
    }

    Pb::FieldTileModel serializeFieldTile(const FieldTileModel& data) {
        Pb::FieldTileModel ser;
        ser.set_position(data.position);
        ser.set_name(data.name);
        ser.set_cost(data.cost.value_or(-1));
        ser.set_costofparking(data.costOfParking.value_or(-1));
        ser.set_color(apply(
                data.color, converter<Color, Pb::Color>()
        ).value_or(Pb::Color::UNSPECIFIED_col));
        ser.set_owner(apply(
                data.owner, converter<Token, Pb::Token>()
        ).value_or(Pb::Token::UNSPECIFIED_tok));
        ser.set_numberofhouses(data.numberOfHouses.value_or(-1));
        ser.set_costperhouse(data.costPerHouse.value_or(-1));
        ser.set_costperhotel(data.costPerHotel.value_or(-1));
        if (data.taxes.has_value()) {
            *ser.mutable_taxes() = serializeTaxes(data.taxes.value());
        }
        return ser;
    }

    template<typename T>
    std::optional<T> loadIfPresent(const T& value, const T& notPresent) {
        if (value == notPresent) return {};
        else return value;
    }

    FieldTileModel deserializeFieldTile(Pb::FieldTileModel&& ser) {
        FieldTileModel ret;
        ret.position = ser.position();
        ret.name = ser.name();
        ret.cost = loadIfPresent(ser.cost(), -1);
        ret.costOfParking = loadIfPresent(ser.costofparking(), -1);
        ret.color = apply(loadIfPresent(ser.color(), Pb::Color::UNSPECIFIED_col),
                          converter<Pb::Color, Color>());
        ret.owner = apply(loadIfPresent(ser.owner(), Pb::Token::UNSPECIFIED_tok),
                          converter<Pb::Token, Token>());
        ret.numberOfHouses = loadIfPresent(ser.numberofhouses(), -1);
        ret.costPerHouse = loadIfPresent(ser.costperhouse(), -1);
        ret.costPerHotel = loadIfPresent(ser.costperhotel(), -1);
        if(ser.has_taxes()) ret.taxes = deserializeTaxes(std::move(*ser.mutable_taxes()));
        return ret;
    }

    Pb::Card serializeCard(const Card& card) {
        Pb::Card ser;
        ser.set_text(card.text);
        return ser;
    }

    Card *deserializeCard(Pb::Card&& data, Board&) {
        //TODO: Subclasses
        throw 1;
    }

    Pb::PlayerData serializePlayerModel(const PlayerModel& data) {
        Pb::PlayerData ser;
        ser.set_name(data.name);
        ser.set_token(Pb::Token(data.token));
        ser.set_position(data.position);
        ser.set_money(data.money);
        ser.set_prisoner(data.prisoner);
        ser.set_alive(data.alive);
        return ser;
    }

    PlayerModel deserializePlayerModel(Pb::PlayerData&& ser) {
        PlayerModel ret;
        ret.name = ser.name();
        ret.token = Token(ser.token());
        ret.position = ser.position();
        ret.money = ser.money();
        ret.prisoner = ser.prisoner();
        ret.alive = ser.alive();
        return ret;
    }

    template<typename T, typename C, typename F>
    //T - serialized type
    //C - container of type V
    void addAll(PROTOBUF_NAMESPACE_ID::RepeatedPtrField<T> *field, const C& cont, F conv) {
        field->Reserve(cont.size());
        for (const auto& x : cont) {
            field->Add(conv(x));
        }
    }

    Pb::BoardModel serializeBoardModel(const BoardModel& board) {
        Pb::BoardModel ser;
        addAll(ser.mutable_tiles(), board.field, serializeFieldTile);
        addAll(ser.mutable_players(), board.players, serializePlayerModel);
        ser.set_curplayer(Pb::Token(board.curPlayer));
        return ser;
    }

    BoardModel deserializeBoardModel(Pb::BoardModel&& ser) {
        BoardModel ret;
        for(auto&& tileSer : *ser.mutable_tiles()) {
            auto tile = deserializeFieldTile(std::move(tileSer));
            ret.field[tile.position] = std::move(tile);
        }
        ret.players.reserve(ser.players_size());
        for (auto&& dataSer : *ser.mutable_players()) {
            ret.players.push_back(deserializePlayerModel(std::move(dataSer)));
        }
        ret.curPlayer = Token(ser.curplayer());
        return ret;
    }

    Pb::InitializationMessage serializeInitializationMessage(const InitializationMessage& msg) {
        Pb::InitializationMessage ser;
        ser.set_token(Pb::Token(msg.token));
        ser.set_name(msg.name);
        return ser;
    }

    InitializationMessage deserializeInitializationMessage(Pb::InitializationMessage&& ser) {
        InitializationMessage msg;
        msg.token = Token(ser.token());
        msg.name = std::move(*ser.mutable_name());
        return msg;
    }
}