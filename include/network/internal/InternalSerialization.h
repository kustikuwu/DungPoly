//
// Created by Igor Engel on 30.03.2020.
//

#ifndef HSEPROJ_INTERNALSERIALIZATION_H
#define HSEPROJ_INTERNALSERIALIZATION_H

#include "MonopolyFwd.h"
#include "monopoly.pb.h"
namespace Monopoly::Serialization::Internal {

    Pb::PlayerRequest serializeRequest(const PlayerRequestData& req);
    PlayerRequest deserializeRequest(Pb::PlayerRequest&& ser);

    Pb::PlayerReply serializeReply(const PlayerReplyData& rep);
    PlayerReply deserializeReply(Pb::PlayerReply&& ser);

    Pb::FieldTileModel serializeFieldTile(const FieldTileModel& data);
    FieldTileModel deserializeFieldTile(Pb::FieldTileModel&& ser);

    Pb::Card serializeCard(const Card& card);
    Card* deserializeCard(Pb::Card&& data, Board&);

    Pb::PlayerData serializePlayerModel(const PlayerModel& data);
    PlayerModel deserializePlayerModel(Pb::PlayerData&& ser);

    Pb::BoardModel serializeBoardModel(const BoardModel& board);
    BoardModel deserializeBoardModel(Pb::BoardModel&& ser);

    Pb::InitializationMessage serializeInitializationMessage(const InitializationMessage& msg);
    InitializationMessage deserializeInitializationMessage(Pb::InitializationMessage&& ser);

}

#endif //HSEPROJ_INTERNALSERIALIZATION_H
