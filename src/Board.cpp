#include "Board.h"

#include <utility>
#include <iostream>
#include "Field.h"
#include "Game.h"

PlayerData::PlayerData(std::string name, Token token) : name(name), token(token) {}

int PlayerData::getMoney() const {
    return money;
}

int PlayerData::addMoney(int newMoney) {
    money += newMoney;
    return money;
}

void PlayerData::setLoser() {
    alive = false;
    money = 0;
    numberOfMortgagedProperty = 0;
    numberOfUtilities = 0;
    numberOfRailways = 0;
    numberOfHouses = 0;
    numberOfHotels = 0;
    cardToLeavePrison = false;
    prisoner = false;
}

void PlayerData::newPosition(int throwSum) {
    std::size_t newPos = (position + throwSum + Board::FIELD_SIZE) % Board::FIELD_SIZE;
    if (newPos < position) {
        (*this).addMoney(START_SUM);
    }
    position = (position + throwSum + Board::FIELD_SIZE) % Board::FIELD_SIZE;
}

void PlayerData::toPrison() {
    position = PRISON_POS;
    daysLeftInPrison = 3;
    doubleDice = 0;
    prisoner = true;
}

void PlayerData::outOfPrison() {
    prisoner = false;
    daysLeftInPrison = 0;
}

Board::Board(LocalGame& game) : game(game), field(), deck() {}

Board::Board(const std::vector<std::pair<std::string_view, Token> >& playersList, LocalGame& game) : game(game), field(),
                                                                                                     deck(*this) {
    players.reserve(playersList.size());
    for (auto[name, token] : playersList) {
        players.emplace_back(std::string(name), token);
    }
    numOfAlivePlayers = playersList.size();
    //TODO: Fill field with tiles
    field[0] = new Start(*this, 0, "Старт");

    field[1] = new Street(*this, 1, "ПТУ", "ПТУ",
                          60, Color::COL1, 50);
    field[1]->setMortgageCost(30);
    field[1]->setTaxes({2, 10, 30, 90, 160, 250});

    field[2] = new PublicTreasury(*this, 2, "Донат стримеру");


    field[3] = new Street(*this, 3, "Шаурмечная", "Шаурмечная",
                          60, Color::COL1, 50);
    field[3]->setMortgageCost(30);
    field[3]->setTaxes({4, 20, 60, 180, 320, 450});

    field[4] = new IncomeTax(*this, 4, "Дань", 200);

    field[5] = new Railway(*this, 5, "Раменское", "Раменское",
                           200, Color::COL1);
    field[5]->setMortgageCost(100);

    field[6] = new Street(*this, 6, "Вкусно и точка", "Вкусно и точка",
                          100, Color::COL2, 50);
    field[6]->setMortgageCost(50);
    field[6]->setTaxes({6, 30, 90, 270, 400, 550});

    field[7] = new Chance(*this, 7, "Аукцион");

    field[8] = new Street(*this, 8, "Ростикс", "Ростикс",
                          100, Color::COL2, 50);
    field[8]->setMortgageCost(50);
    field[8]->setTaxes({6, 30, 90, 270, 400, 550});

    field[9] = new Street(*this, 9, "Додо Пицца", "Додо Пицца",
                          120, Color::COL2, 50);
    field[9]->setMortgageCost(60);
    field[9]->setTaxes({8, 40, 100, 300, 450, 600});

    field[10] = new Prison(*this, 10, "Доска Позора");

    field[11] = new Street(*this, 11, "Хоррор Гаммов", "Хоррор Гаммов",
                           140, Color::COL3, 100);
    field[11]->setMortgageCost(70);
    field[11]->setTaxes({10, 50, 150, 450, 625, 750});

    field[12] = new Utility(*this, 12, "Питание Компа Optimus1412_", "Питание Компа Optimus1412_",
                            150, Color::COL1);
    field[12]->setMortgageCost(75);

    field[13] = new Street(*this, 13, "Роблокская", "Роблокская",
                           140, Color::COL3, 100);
    field[13]->setMortgageCost(70);
    field[13]->setTaxes({10, 50, 150, 450, 625, 750});

    field[14] = new Street(*this, 14, "Дота 2", "Дота 2",
                           160, Color::COL3, 100);
    field[14]->setMortgageCost(80);
    field[14]->setTaxes({12, 60, 180, 500, 700, 900});

    field[15] = new Railway(*this, 15, "Китай Город", "Китай Город",
                            200, Color::COL1);
    field[15]->setMortgageCost(100);

    field[16] = new Street(*this, 16, "Даб Тугезерово", "Даб Тугезерово",
                           180, Color::COL4, 100);
    field[16]->setMortgageCost(90);
    field[16]->setTaxes({14, 70, 200, 550, 750, 950});

    field[17] = new PublicTreasury(*this, 17, "Баллы Стримера");

    field[18] = new Street(*this, 18, "Фортнайт стрит", "Фортнайт стрит",
                           180, Color::COL4, 100);
    field[18]->setMortgageCost(90);
    field[18]->setTaxes({14, 70, 200, 550, 750, 950});

    field[19] = new Street(*this, 19, "Майнкрафтоская", "Майнкрафтоская",
                           200, Color::COL4, 100);
    field[19]->setMortgageCost(100);
    field[19]->setTaxes({16, 80, 220, 600, 800, 1000});

    field[20] = new FreeParking(*this, 20, "АФК");

    field[21] = new Street(*this, 21, "Чаттерская", "Чаттерская",
                           220, Color::COL5, 150);
    field[21]->setMortgageCost(110);
    field[21]->setTaxes({18, 90, 250, 700, 875, 1050});

    field[22] = new Chance(*this, 22, "Аукцион");

    field[23] = new Street(*this, 23, "Донатерская", "Донатерская",
                           220, Color::COL5, 150);
    field[23]->setMortgageCost(110);
    field[23]->setTaxes({18, 90, 250, 700, 875, 1050});

    field[24] = new Street(*this, 24, "Виперово", "Виперово",
                           240, Color::COL5, 150);
    field[24]->setMortgageCost(120);
    field[24]->setTaxes({20, 100, 300, 750, 925, 1100});

    field[25] = new Railway(*this, 25, "Домодедово", "Домодедово",
                            200, Color::COL1);
    field[25]->setMortgageCost(100);

    field[26] = new Street(*this, 26, "Дрейково", "Дрейково",
                           260, Color::COL6, 150);
    field[26]->setMortgageCost(130);
    field[26]->setTaxes({22, 110, 330, 800, 975, 1150});

    field[27] = new Street(*this, 27, "Денжер Лёхов", "Денжер Лёхов",
                           260, Color::COL6, 150);
    field[27]->setMortgageCost(130);
    field[27]->setTaxes({22, 110, 330, 800, 975, 1150});

    field[28] = new Utility(*this, 28, "Завод Монстров", "Завод Монстров",
                            150, Color::COL1);
    field[28]->setMortgageCost(75);

    field[29] = new Street(*this, 29, "Егор Кридов", "Егор Кридов",
                           280, Color::COL6, 150);
    field[29]->setMortgageCost(140);
    field[29]->setTaxes({24, 120, 360, 850, 1025, 1200});

    field[30] = new GoToPrison(*this, 30, "Отправляйся в Доску Позора");

    field[31] = new Street(*this, 31, "Тимурити", "Тимурити",
                           300, Color::COL7, 200);
    field[31]->setMortgageCost(150);
    field[31]->setTaxes({26, 130, 390, 900, 1100, 1275});

    field[32] = new Street(*this, 32, "Анимешниковская", "Анимешниковская",
                           300, Color::COL7, 200);
    field[32]->setMortgageCost(150);
    field[32]->setTaxes({26, 130, 390, 900, 1100, 1275});

    field[33] = new PublicTreasury(*this, 33, "Донат стримеру");

    field[34] = new Street(*this, 34, "Манданилово", "Манданилово",
                           320, Color::COL7, 200);
    field[34]->setMortgageCost(160);
    field[34]->setTaxes({28, 150, 450, 1000, 1200, 1400});

    field[35] = new Railway(*this, 35, "Шереметьево", "Шереметьево",
                            200, Color::COL1);
    field[35]->setMortgageCost(100);

    field[36] = new Chance(*this, 36, "Аукцион");

    field[37] = new Street(*this, 37, "Ахегаовская", "Ахегаовская",
                           350, Color::COL8, 200);
    field[37]->setMortgageCost(175);
    field[37]->setTaxes({35, 175, 500, 1100, 1300, 1500});

    field[38] = new IncomeTax(*this, 38, "Дань", 100);

    field[39] = new Street(*this, 39, "Банькова", "Банькова",
                           400, Color::COL8, 200);
    field[39]->setMortgageCost(200);
    field[39]->setTaxes({50, 200, 600, 1400, 1700, 2000});

    getMortgageToken[Token::DOG] = TokenForMortgage::DOG_M;
    getMortgageToken[Token::CAR] = TokenForMortgage::CAR_M;
    getMortgageToken[Token::HAT] = TokenForMortgage::HAT_M;
    getMortgageToken[Token::BOOT] = TokenForMortgage::BOOT_M;
    getMortgageToken[Token::CAT] = TokenForMortgage::CAT_M;
    getMortgageToken[Token::SHIP] = TokenForMortgage::SHIP_M;
}

PlayerData& Board::getPlayer(Token token) {
    for (auto& player : players) {
        if (player.token == token) {
            return player;
        }
    }
    //TODO: что вернуть если не нашли.
    throw 1;
}

const PlayerData& Board::getPlayer(Token token) const {
    for (auto& player : players) {
        if (player.token == token) {
            return player;
        }
    }
    //TODO: что вернуть если не нашли.
    throw 1;
}

Token Board::getPlayerToken(std::size_t index) const {
    return players[index].token;
}

PlayerReply Board::send(PlayerRequest request) const {
    return game.send(std::move(request));
}


std::size_t Board::getPlayersNumber() const {
    return players.size();
}

std::vector<PlayerData>& Board::getPlayers() {
    return players;
}


const std::vector<PlayerData>& Board::getPlayers() const {
    return players;
}


CardPool& Board::getDeck() {
    return deck;
}

const CardPool& Board::getDeck() const {
    return deck;
}

auto Board::getField() -> std::array<FieldTile *, Board::FIELD_SIZE>& {
    return field;
}

auto Board::getField() const -> const std::array<FieldTile *, Board::FIELD_SIZE>& {
    return field;
}

void Board::decrNumOfOlayers() {
    numOfAlivePlayers--;
}

void Board::surrenderPlayer(Token token) {
    PlayerData& player = getPlayer(token);
    if (!player.alive) return;
    player.setLoser();
    decrNumOfOlayers();
    makePlayerLoser(token);
}

bool Board::isFinished() const {
    return (numOfAlivePlayers <= 1) || terminated.load();
}

int Board::getCurNumOfPlayers() const {
    return numOfAlivePlayers;
}

Token Board::getWinner() const {
    for (std::size_t i = 0; i < (*this).getPlayersNumber(); i++) {
        if (players[i].alive) {
            return players[i].token;
        }
    }

    //TODO: что вернуть если не нашли.
    throw 1;
}

FieldTile *Board::getTile(std::size_t pos) const {
    return field[pos];
}

void Board::sync() {
    game.sync();
}

void Board::terminate() {
    terminated.store(true);
}

std::size_t Board::getPlayerIndex() const {
    return index;
}

void Board::setPlayerIndex(std::size_t num) {
    index = num;
}

FieldTile *Board::getFieldTile(int ind) {
    if (ind < 0 || static_cast<size_t>(ind) > FIELD_SIZE) {
        throw std::out_of_range("Нет клетки поля с номером " + std::to_string(ind));
    }
    return field[ind];
}

bool Board::checkAllFieldsOfCurColor(Token token, int ind) const {
    if (ind < 0 || static_cast<size_t>(ind) > FIELD_SIZE) {
        throw std::out_of_range("Нет клетки поля с номером " + std::to_string(ind));
    }
    for (std::size_t i = 0; i < FIELD_SIZE; i++) {
        if (field[i]->getColor() == field[ind]->getColor() &&
            field[i]->getOwner() != field[ind]->getOwner()) {
            return false;
        }
    }
    return true;
}

bool Board::checkFieldWithoutBuildings(Token token) const {
    if (token == Token::FREE_FIELD) {
        throw std::invalid_argument("У клетки нет владельца");
    }
    for (std::size_t i = 0; i < FIELD_SIZE; i++) {
        if (field[i]->getOwner() == token && field[i]->getNumberOfHotels() == 0 &&
            field[i]->getNumberOfHouses() == 0 && !field[i]->isMortgaged) {
            return true;
        }
    }
    return false;
}

bool Board::checkAllFieldsOfCol(Token token) const {
    if (field[1]->getOwner() == token && field[3]->getOwner() == token) {
        return true;
    }
    if (field[6]->getOwner() == token && field[8]->getOwner() == token &&
        field[9]->getOwner() == token) {
        return true;
    }
    if (field[11]->getOwner() == token && field[13]->getOwner() == token &&
        field[14]->getOwner() == token) {
        return true;
    }
    if (field[16]->getOwner() == token && field[18]->getOwner() == token &&
        field[19]->getOwner() == token) {
        return true;
    }
    if (field[21]->getOwner() == token && field[23]->getOwner() == token &&
        field[24]->getOwner() == token) {
        return true;
    }
    if (field[26]->getOwner() == token && field[27]->getOwner() == token &&
        field[29]->getOwner() == token) {
        return true;
    }
    if (field[31]->getOwner() == token && field[32]->getOwner() == token &&
        field[34]->getOwner() == token) {
        return true;
    }
    if (field[37]->getOwner() == token && field[39]->getOwner() == token) {
        return true;
    }
    return false;
}

std::size_t Board::getPlayerNum(Token token) const {
    for (std::size_t i = 0; i < (*this).getPlayersNumber(); i++) {
        if (players[i].token == token) {
            return i;
        }
    }
    throw std::invalid_argument("Игрок с такой фишкой не найден");
}

std::string Board::numToTokenString(int ind) const {
    if (ind == -1) {
        return "Нет";
    }
    Token buyer = this->getPlayerToken(ind);
    return this->tokenToString(buyer);
}

std::string Board::tokenToString(Token token) const {
    if (token == Token::SHIP)
        return "Корабль";
    else if (token == Token::BOOT)
        return "Ботинок";
    else if (token == Token::CAR)
        return "Машина";
    else if (token == Token::CAT)
        return "Кот";
    else if (token == Token::DOG)
        return "Собака";
    else if (token == Token::HAT)
        return "Шляпа";
    throw std::invalid_argument("Неизвестная игровая фишка");
}

bool Board::atLeastOneBoughtFieldOtherToken(Token token) const {
    for (std::size_t i = 0; i < FIELD_SIZE; i++) {
        if ((field[i]->getOwner() != Token::FREE_FIELD) && (field[i]->getOwner() != token)) {
            return true;
        }
    }
    return false;
}

void Board::makePlayerLoser(Token token) {
    for (std::size_t i = 0; i < FIELD_SIZE; i++) {
        if (field[i]->getOwner() == token) {
            field[i]->setOwner(Token::FREE_FIELD);
            while (field[i]->getNumberOfHotels() > 0) {
                field[i]->decrHotelNum();
            }
            while (field[i]->getNumberOfHouses() > 0) {
                field[i]->decrHouseNum();
            }
            field[i]->isMortgaged = false;
        }
    }
}

int Board::countPlayerMoney(Token token) const {
    int sum = 0;
    const PlayerData& player = this->getPlayer(token);
    sum += player.getMoney();
    for (std::size_t i = 0; i < FIELD_SIZE; i++) {
        if ((field[i]->getOwner() != Token::FREE_FIELD) && (field[i]->getOwner() != token)) {
            return true;
        }
    }
    return sum;
}

int countPrevForColor(int ind, const Board& board) {
    if (ind < 0 || static_cast<size_t>(ind) > Board::FIELD_SIZE) {
        throw std::out_of_range("Нет клетки поля с номером " + std::to_string(ind));
    }
    int cnt = 0;
    for (int i = ind; i >= 0; i--) {
        if (board.getField()[i]->getColor() == board.getField()[ind]->getColor()) {
            cnt++;
        }
    }
    return cnt;
}


bool checkHousesNumForHotel(int ind, const Board& board) {
    if (ind < 0 || ind > (int) Board::FIELD_SIZE) {
        throw std::out_of_range("Нет клетки поля с номером " + std::to_string(ind));
    }
    for (std::size_t i = ind; i < Board::FIELD_SIZE; i++) {
        if (board.getField()[i]->getColor() == board.getField()[ind]->getColor() &&
            (board.getField()[i]->getNumberOfHouses() != 4 || board.getField()[i]->getNumberOfHotels() != 0)) {
            return false;
        }
    }
    return true;
}

bool checkPrevHotel(int ind, const Board& board) {
    int i = ind - 1;
    while (i >= 0) {
        if (board.getField()[i]->getColor() == board.getField()[ind]->getColor()
            && board.getField()[i]->getNumberOfHotels() == 1) {
            return true;
        }
        i--;
    }
    return true;
}

bool checkPrevHouse(int ind, const Board& board) {
    int i = ind - 1;
    while (i >= 0) {
        if (board.getField()[i]->getColor() == board.getField()[ind]->getColor()
            && board.getField()[i]->getNumberOfHouses() == board.getField()[ind]->getNumberOfHouses() + 1) {
            return true;
        }
        i--;
    }
    return true;
}

bool checkHousesNumForHouse(int ind, const Board& board) {
    for (std::size_t i = ind; i < Board::FIELD_SIZE; i++) {
        if (board.getField()[i]->getColor() == board.getField()[ind]->getColor() &&
            (board.getField()[i]->getNumberOfHouses() != board.getField()[i]->getNumberOfHotels())) {
            return false;
        }
    }
    return true;
}

bool checkPrevForHotel(int ind, const Board& board) {
    int pos = countPrevForColor(ind, board);
    if (pos == 0) {
        return checkHousesNumForHotel(ind, board);
    }
    return checkPrevHotel(ind, board);
}

bool checkPrevForHouse(int ind, const Board& board) {
    int pos = countPrevForColor(ind, board);
    if (pos == 0) {
        return checkHousesNumForHouse(ind, board);
    }
    return checkPrevHouse(ind, board);
}

bool checkNextForHotel(int ind, const Board& board) {
    std::size_t pos = ind + 1;
    while (pos < Board::FIELD_SIZE && board.getField()[pos]->getColor() != board.getField()[ind]->getColor()) {
        pos++;
    }
    return pos == Board::FIELD_SIZE || board.getField()[pos]->getNumberOfHotels() == 0;
}

bool checkNextForHouse(int ind, const Board& board) {
    std::size_t pos = ind + 1;
    while (pos < Board::FIELD_SIZE && board.getField()[pos]->getColor() != board.getField()[ind]->getColor()) {
        pos++;
    }
    return pos == Board::FIELD_SIZE || (board.getField()[pos]->getNumberOfHotels() == 0 &&
                                        board.getField()[pos]->getNumberOfHouses() + 1 ==
                                        board.getField()[ind]->getNumberOfHouses());
}

bool checkNextForBuyHouse(int ind, const Board& board) {
    std::size_t pos = ind + 1;
    while (pos < Board::FIELD_SIZE && board.getField()[pos]->getColor() != board.getField()[ind]->getColor()) {
        pos++;
    }
    std::cout << (pos == Board::FIELD_SIZE ? "Нет\n" : board.getField()[pos]->name + '\n');
    return pos == Board::FIELD_SIZE || (board.getField()[pos]->getNumberOfHotels() == 0 &&
                                        board.getField()[pos]->getNumberOfHouses() ==
                                        board.getField()[ind]->getNumberOfHouses());
}

bool checkNextForBuyHotel(int ind, const Board& board) {
    std::size_t pos = ind + 1;
    while (pos < Board::FIELD_SIZE && board.getField()[pos]->getColor() != board.getField()[ind]->getColor()) {
        pos++;
    }
    return (pos == Board::FIELD_SIZE || board.getField()[pos]->getNumberOfHotels() == 0);
}

bool checkToSellField(int ind, const Board& board) {
    if (ind < 0 || ind > (int) Board::FIELD_SIZE) {
        throw std::out_of_range("Нет клетки поля с номером " + std::to_string(ind));
    }
    for (std::size_t i = 0; i < Board::FIELD_SIZE; i++) {
        if ((int) i != ind && board.getField()[i]->getColor() == board.getField()[ind]->getColor() &&
            board.getField()[i]->getOwner() == board.getField()[ind]->getOwner() &&
            (board.getField()[i]->getNumberOfHouses() != 0 ||
             board.getField()[i]->getNumberOfHotels() != 0)) {
            return false;
        }
    }
    return true;
}
