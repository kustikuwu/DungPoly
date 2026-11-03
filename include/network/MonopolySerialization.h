//
// Created by Igor Engel on 03.04.2020.
//

#ifndef HSEPROJ_MONOPOLYSERIALIZATION_H
#define HSEPROJ_MONOPOLYSERIALIZATION_H

#include "MonopolyFwd.h"
#include <string>
namespace Monopoly::Serialization {

    std::string serializeRequest(const PlayerRequest& req);
    PlayerRequest deserializeRequest(const std::string& ser);

    std::string serializeReply(const PlayerReply& rep);
    PlayerReply deserializeReply(const std::string& ser);

    std::string serializeBoard(const BoardModel& board);
    BoardModel deserializeBoard(const std::string& ser);

    struct InitializationMessage {
        Token token;
        std::string name;
    };

    std::string serializeInitialization(const InitializationMessage& msg);
    InitializationMessage deserializeInitialization(const std::string& ser);
}

#endif //HSEPROJ_MONOPOLYSERIALIZATION_H
