#ifndef BOARDMODEL_H
#define BOARDMODEL_H
#include <string_view>
#include <optional>
#include "MonopolyFwd.h"
#include "Board.h"
#include "Field.h"

struct PlayerModel {
    std::string name;
    Token token;
    std::size_t position;
    int money;
    bool prisoner;
    bool alive;

    void update(const PlayerData& player);
};

struct FieldTileModel {
    int position;
    std::string name;

   // std::optional<int> tax;
    bool isMortgaged;

    std::optional<int> cost;
    std::optional<std::string> shortName;
    std::optional<int> costOfParking;
    std::optional<Color> color;
    std::optional<Token> owner;
    std::optional<int> numberOfHouses;
    std::optional<int> numberOfHotels;
    std::optional<int> costPerHouse;
    std::optional<Taxes> taxes;
    std::optional<int> costPerHotel;

    void update(const FieldTile& tile);
};

struct BoardModel {
    constexpr static size_t FIELD_SIZE = Board::FIELD_SIZE;
    std::array<FieldTileModel, FIELD_SIZE> field;
    std::vector<PlayerModel> players;
    Token curPlayer = Token::FREE_FIELD;

    void update(const Board& board);
    bool isInitialized = false;
};

#endif //BOARDMODEL_H