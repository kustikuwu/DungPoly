#ifndef FIELD_H
#define FIELD_H

#include "MonopolyFwd.h"
#include "PlayerRequests.h"
#include <string>
#include <vector>

static constexpr int PRISON_FINE = 50;
static constexpr int START_SUM = 200;

enum class Color : std::size_t {
    COL1,
    COL2,
    COL3,
    COL4,
    COL5,
    COL6,
    COL7,
    COL8,
    NO_COL
};

enum class TileType : std::size_t {
    START = 0,
    OWNABLE_TILE = 1,
    PRISON = 2,
    GOTO_PRISON = 3,
    CHANCE = 4,
    INCOME_TAX = 5,
    FREE_PARKING = 6,
    PUBLIC_TREASURY = 7,
    UNINITIALIZED = 1000
};

enum class OwnableType : std::size_t {
    RAILWAY = 0,
    STREET = 1,
    UTILITY = 2,
    UNINITIALIZED = 1000
};

struct Taxes {
    uint32_t startTax = 0;
    uint32_t taxOneHouse = 0;
    uint32_t taxTwoHouses = 0;
    uint32_t taxThreeHouses = 0;
    uint32_t taxFourHouses = 0;
    uint32_t taxHotel = 0;
    uint32_t get(int id) const;
};

class FieldTile {
public:
    explicit FieldTile(Board& board); // Creates uninitialized tile, for deserialization only
    FieldTile(Board& board, TileType type, int position, std::string name);

    virtual ~FieldTile() = default;

    Board& board;
    TileType type = TileType::UNINITIALIZED;
    uint32_t position = 0;
    std::string name;
    bool isMortgaged = false;

    virtual void onPlayerPass(Token token);

    virtual void onPlayerEntry(Token token);

    virtual void onPurchase(Token token) {};

    virtual std::vector<std::string> writeTileInfo();

    virtual void setMortgageCost(int val = 0) = 0;

    virtual bool MortgageTile(Token token) = 0;

    virtual Token getOwner() const { return Token::FREE_FIELD; }

    virtual Color getColor() const { return Color::NO_COL; }

    virtual uint32_t getMortgageCost() const { return 0; }

    virtual uint32_t getNumberOfHouses() const { return 0; }

    virtual uint32_t getNumberOfHotels() const { return 0; }

    virtual uint32_t getHouseCost() const { return 0; }

    virtual uint32_t getHotelCost() const { return 0; }

    virtual void setTaxes(Taxes taxes) {};

    virtual void addHouse() {}

    virtual void addHotel() {}

    virtual void decrHotelNum() {}

    virtual void decrHouseNum() {}

    virtual void setOwner(Token token) {}

    virtual int getFieldCost() { return 0; }

    virtual void decrPropertyNum(Token token) {};

    virtual bool canBeSold(Token token) const { return false; }
};

class OwnableTile : public FieldTile {
public:
    explicit OwnableTile(Board& board);

    OwnableTile(Board& board, int position, std::string name, std::string shortName,
                OwnableType ownableType, uint32_t cost, Color color);

    virtual uint32_t calculateTax(Token token) = 0;

    virtual void onPurchase(Token token) override = 0;

    void onPlayerEntry(Token token) override;

    void setMortgageCost(int val = 0) override { mortgageCost = val; }

    bool MortgageTile(Token token) override;

    virtual std::vector<std::string> writeTileInfo() override;

    Token getOwner() const override { return owner; }

    uint32_t getMortgageCost() const override { return mortgageCost; }

    void setOwner(Token token) override { owner = token; }

    int getFieldCost() override { return cost; }

    bool canBeSold(Token token) const override;

    std::string shortName;
    OwnableType ownableType = OwnableType::UNINITIALIZED;
    uint32_t cost = 0;
    uint32_t costOfParking = 0;
    Color color = Color::COL1;
    Token owner = Token::FREE_FIELD;
    int mortgageCost = 0;
};

class Railway final : public OwnableTile {
public:

    explicit Railway(Board& board);

    Railway(Board& board, int position, std::string name, std::string shortName,
            int cost, Color color);

    uint32_t calculateTax(Token token) override;

    void onPurchase(Token token) override;

    std::vector<std::string> writeTileInfo() override;

    void decrPropertyNum(Token token) override;
};

class Street final : public OwnableTile {
public:
    explicit Street(Board& board);

    Street(Board& board, int position, std::string name, std::string shortName,
           int cost, Color color, int costPerHouse, int costPerHotel = 0);

    uint32_t calculateTax(Token token) override;

    void onPurchase(Token token) override;

    std::vector<std::string> writeTileInfo() override;

    uint32_t getNumberOfHouses() const override { return numberOfHouses; }

    uint32_t getNumberOfHotels() const override { return numberOfHotels; }

    uint32_t getHotelCost() const override { return costPerHotel; }

    uint32_t getHouseCost() const override { return costPerHouse; }

    void addHouse() override { numberOfHouses++; }

    void addHotel() override { numberOfHotels++; }

    void setTaxes(Taxes taxes) override;

    uint32_t numberOfHouses = 0;
    uint32_t numberOfHotels = 0;
    uint32_t costPerHouse = 0;
    uint32_t costPerHotel = 0;

    void decrHotelNum() override { numberOfHotels--; }

    void decrHouseNum() override { numberOfHouses--; }

    Color getColor() const override { return color; }

    Taxes taxes;
};

class Utility final : public OwnableTile {
public:

    explicit Utility(Board& board);

    Utility(Board& board, int position, std::string name, std::string shortName,
            int cost, Color color);

    std::vector<std::string> writeTileInfo() override;

    uint32_t calculateTax(Token token) override;

    void onPurchase(Token token) override;

    void decrPropertyNum(Token token) override;
};

class Start final : public FieldTile {
public:
    explicit Start(Board& board);

    Start(Board& board, int position, std::string name);

    std::vector<std::string> writeTileInfo() override;

    void onPlayerPass(Token token) override;

    void onPlayerEntry(Token token) override;

    void setMortgageCost(int val = 0) override {};

    bool MortgageTile(Token token) override { return false; };
};

class Prison final : public FieldTile {
public:
    explicit Prison(Board& board);

    Prison(Board& board, int position, std::string name);

    std::vector<std::string> writeTileInfo() override;

    void onPlayerEntry(Token token) override;

    const static int TAX;

    void setMortgageCost(int val = 0) override {};

    bool MortgageTile(Token token) override { return false; };
};

//Considered harmful
class GoToPrison final : public FieldTile {
public:
    explicit GoToPrison(Board& board);

    GoToPrison(Board& board, int position, std::string name);

    std::vector<std::string> writeTileInfo() override;

    void onPlayerEntry(Token token) override;

    void setMortgageCost(int val = 0) override {};

    bool MortgageTile(Token token) override { return false; };
};

class Chance final : public FieldTile {
public:
    explicit Chance(Board& board);

    Chance(Board& board, int position, std::string name);

    std::vector<std::string> writeTileInfo() override;

    void onPlayerEntry(Token token) override;

    void setMortgageCost(int val = 0) override {};

    bool MortgageTile(Token token) override { return false; };

    std::vector<Card *> cards;
};

class PublicTreasury final : public FieldTile {
public:
    explicit PublicTreasury(Board& board);

    PublicTreasury(Board& board, int position, std::string name);

    void onPlayerEntry(Token token) override;

    std::vector<std::string> writeTileInfo() override;

    void setMortgageCost(int val = 0) override {};

    bool MortgageTile(Token token) override { return false; };

    std::vector<Card *> cards;
};

class IncomeTax final : public FieldTile {
public:
    explicit IncomeTax(Board& board);

    IncomeTax(Board& board, int position, std::string name,
              uint32_t tax);

    uint32_t tax = 0;

    std::vector<std::string> writeTileInfo() override;

    void onPlayerEntry(Token token) override;

    void setMortgageCost(int val = 0) override {};

    bool MortgageTile(Token token) override { return false; };

    int getTax() const;

};

class FreeParking final : public FieldTile {
public:
    explicit FreeParking(Board& board);

    FreeParking(Board& board, int position, std::string name);

    std::vector<std::string> writeTileInfo() override;

    void setMortgageCost(int val = 0) override {};

    bool MortgageTile(Token token) override { return false; };
};

PlayerRequest makeDefaultRequest(Token token, Board& board);

bool handleGenericActions(Token tok, const FieldTile& tile, PlayerAction action);

#endif //FIELD_H
