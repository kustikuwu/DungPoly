#include "network/MonopolySerialization.h"
#include "network/internal/InternalSerialization.h"
#include "PlayerRequests.h"
#include "include/threads/BoardModel.h"
#include "monopoly.pb.h"
namespace Monopoly::Serialization {

    template<typename T, typename F>
    std::string serialize(const T& t, F conv) {
        return conv(t).SerializeAsString();
    }

    template<typename S, typename F>
    auto deserialize(const std::string& ser, F conv) {
        S data;
        data.ParseFromString(ser);
        return conv(std::move(data));
    }

    std::string serializeRequest(const PlayerRequest& req) {
        return serialize(*req, Internal::serializeRequest);
    }

    PlayerRequest deserializeRequest(const std::string& ser) {
        return deserialize<Pb::PlayerRequest>(ser, Internal::deserializeRequest);
    }

    std::string serializeReply(const PlayerReply& rep) {
        return serialize(*rep, Internal::serializeReply);
    }

    PlayerReply deserializeReply(const std::string& ser) {
        return deserialize<Pb::PlayerReply>(ser, Internal::deserializeReply);
    }

    std::string serializeBoard(const BoardModel& board) {
        return serialize(board, Internal::serializeBoardModel);
    }

    BoardModel deserializeBoard(const std::string& ser) {
        return deserialize<Pb::BoardModel>(ser, Internal::deserializeBoardModel);
    }

    std::string serializeInitialization(const InitializationMessage& msg) {
        return serialize(msg, Internal::serializeInitializationMessage);
    }
    InitializationMessage deserializeInitialization(const std::string& ser) {
        return deserialize<Pb::InitializationMessage>(ser, Internal::deserializeInitializationMessage);
    }

}