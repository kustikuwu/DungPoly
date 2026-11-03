#ifndef MONOPOLYFWD_H
#define MONOPOLYFWD_H

#include <cstddef>
#include <memory>
#include <map>
//#include "View.h"

struct PlayerData;

class Board;

class Card;

class CardPool;

class FieldTile;

class OwnableTile;

class Start;

class Prison;

class GoToPrison;

class Chance;

class IncomeTax;

class Railway;

class Street;

class Utility;

class FreeParking;

enum class Color : std::size_t;

class LocalGame;

class Manager;

enum class Token : std::size_t {
    FREE_FIELD,
    DOG,
    HAT,
    BOOT,
    CAT,
    CAR,
    SHIP
};

enum class TokenForMortgage : std::size_t {
    DOG_M,
    HAT_M,
    BOOT_M,
    CAT_M,
    CAR_M,
    SHIP_M,
};

enum class MessageType : std::size_t {
    INFO = 0,
    CHANCE = 1,
    PUBLIC_TREASURY = 2,
    DICE = 3
};

class Player;

class NetworkPlayer;

class BotPlayer;

class LocalPlayer;

struct PlayerModel;
struct FieldTileModel;
struct BoardModel;

struct PlayerRequestData;
struct PlayerReplyData;
using PlayerRequest = std::unique_ptr<PlayerRequestData>;
using PlayerReply = std::unique_ptr<PlayerReplyData>;

class RandomSource;

class AbstractView;

class MenuView;

class MonopolyView;

class NcursesView;

using ViewHolder = std::unique_ptr<AbstractView>;

class SFMLView;

class MonopolyThreadManager;

namespace Monopoly::Serialization {

    struct InitializationMessage;

}

constexpr size_t NETWORK_PORT = 6741;

#endif //MONOPOLYFWD_H
