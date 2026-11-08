#ifndef BOARD_H
#define BOARD_H

#include "MonopolyFwd.h"
#include "Cards.h"
#include "PlayerRequests.h"
#include <atomic>
#include <string_view>
#include <array>
#include <vector>

static constexpr int START_POS = 0;
static constexpr int PRISON_POS = 10;
static constexpr int RAMENSKOE_POS = 5;
static constexpr int HORROR_GAMMOV_POS = 11;
static constexpr int VIPEROVO_POS = 24;
static constexpr int BANKOVA_POS = 39;

struct PlayerData {
    PlayerData(std::string name, Token token);
    PlayerData(const PlayerData&) = delete;
    virtual ~PlayerData() = default;
    void operator=(PlayerData) = delete;
    PlayerData(PlayerData&&) noexcept = default;
    PlayerData& operator=(PlayerData&&) noexcept = default;
    std::string name;
    Token token;
    uint32_t position = 0;
    uint32_t money = 1500; //TODO: Negative money?
    uint32_t doubleDice = 0;
    uint32_t daysLeftInPrison = 0;
    uint32_t numberOfRailways = 0;
    uint32_t numberOfUtilities = 0;
    uint32_t lastTrow = 0;
    uint32_t numberOfHouses = 0;
    uint32_t numberOfHotels = 0;
    uint32_t numberOfMortgagedProperty = 0;

    bool prisoner = false;
    bool alive = true;
    bool cardToLeavePrison = false;
    std::vector<std::unique_ptr<Card>> cards;

    int getMoney() const ;
    int addMoney(int newMoney);
    void setLoser();

    void newPosition(int throwSum);
    void toPrison();
    void outOfPrison();
};

class Board {
public:
    explicit Board(LocalGame& game); // Creates uninitialized board, for deserialization only
    Board(const std::vector<std::pair<std::string_view, Token>>& players, LocalGame& game);
    static constexpr size_t FIELD_SIZE = 40;

    void sync();

    virtual PlayerData& getPlayer(Token token);
    const PlayerData& getPlayer(Token token) const;
    FieldTile *getFieldTile(int ind);
    Token getPlayerToken(std::size_t index) const;
    std::size_t getPlayerNum(Token token) const;

    virtual PlayerReply send(PlayerRequest request) const;

    std::size_t getPlayersNumber() const;
    void decrNumOfOlayers();
    void surrenderPlayer(Token token);
    bool isFinished() const;
    void terminate();
    int getCurNumOfPlayers() const;
    Token getWinner() const;
    FieldTile* getTile(std::size_t pos) const;
    bool checkAllFieldsOfCurColor(Token token, int ind) const;
    bool checkFieldWithoutBuildings(Token token) const;
    bool checkAllFieldsOfCol(Token token) const;

    std::string numToTokenString(int ind) const;
    std::string tokenToString(Token token) const;

    bool atLeastOneBoughtFieldOtherToken(Token token) const;
    int countPlayerMoney(Token token) const;
    Token findWinner() const;
    void makePlayerLoser(Token token);

    std::size_t getPlayerIndex() const;
    void setPlayerIndex(std::size_t num);
    std::map<Token, TokenForMortgage> getMortgageToken;

    std::array<FieldTile*, FIELD_SIZE>& getField();
    const std::array<FieldTile*, FIELD_SIZE>& getField() const;

    std::vector<PlayerData>& getPlayers();
    const std::vector<PlayerData>& getPlayers() const;

    CardPool& getDeck();
    const CardPool& getDeck() const;

private:
    LocalGame& game;
    std::array<FieldTile*, FIELD_SIZE> field; //TODO: raw pointer?
    std::vector<PlayerData> players;
    CardPool deck;
    int numOfAlivePlayers = 0;
    std::atomic_bool terminated = false;
    std::size_t index = 0;
};

bool checkPrevForHotel(int ind, const Board& board);
bool checkPrevForHouse(int ind, const Board& board);

bool checkNextForHotel(int ind, const Board& board);
bool checkNextForHouse(int ind, const Board& board);

int countPrevForColor(int ind, const Board& board);
bool checkHousesNumForHotel(int ind, const Board& board);
bool checkHousesNumForHouse(int ind, const Board& board);
bool checkPrevHotel(int ind, const Board& board);
bool checkPrevHouse(int ind, const Board& board);

bool checkToSellField(int ind, const Board& board);

bool checkNextForBuyHouse(int ind, const Board& board);
bool checkNextForBuyHotel(int ind, const Board& board);

#endif //BOARD_H
