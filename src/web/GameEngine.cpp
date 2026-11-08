#include "LocalGame.h"
#include "Field.h"
#include "threads/BoardModel.h"
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

namespace {

struct EnginePlayer {
    std::string name;
    Token token;
};

std::string jsonString(const std::string& value) {
    std::ostringstream result;
    result << '"';
    for (unsigned char ch : value) {
        switch (ch) {
            case '"': result << "\\\""; break;
            case '\\': result << "\\\\"; break;
            case '\b': result << "\\b"; break;
            case '\f': result << "\\f"; break;
            case '\n': result << "\\n"; break;
            case '\r': result << "\\r"; break;
            case '\t': result << "\\t"; break;
            default:
                if (ch < 0x20) {
                    constexpr char hex[] = "0123456789abcdef";
                    result << "\\u00" << hex[ch >> 4] << hex[ch & 0x0f];
                } else {
                    result << static_cast<char>(ch);
                }
        }
    }
    result << '"';
    return result.str();
}

std::string tokenName(Token token) {
    switch (token) {
        case Token::FREE_FIELD: return "FREE_FIELD";
        case Token::DOG: return "DOG";
        case Token::HAT: return "HAT";
        case Token::BOOT: return "BOOT";
        case Token::CAT: return "CAT";
        case Token::CAR: return "CAR";
        case Token::SHIP: return "SHIP";
    }
    return "FREE_FIELD";
}

std::string actionName(PlayerAction action) {
    switch (action) {
        case PlayerAction::END_TURN: return "END_TURN";
        case PlayerAction::PAY_TAX: return "PAY_TAX";
        case PlayerAction::BUY_FIELD: return "BUY_FIELD";
        case PlayerAction::BUY_HOUSE: return "BUY_HOUSE";
        case PlayerAction::BUY_HOTEL: return "BUY_HOTEL";
        case PlayerAction::SELL_HOUSE: return "SELL_HOUSE";
        case PlayerAction::SELL_FIELD: return "SELL_FIELD";
        case PlayerAction::SELL_HOTEL: return "SELL_HOTEL";
        case PlayerAction::USE_CARD: return "USE_CARD";
        case PlayerAction::PAY_TO_OTHER_PLAYER: return "PAY_TO_OTHER_PLAYER";
        case PlayerAction::TAKE_CARD: return "TAKE_CARD";
        case PlayerAction::MORTGAGE_HOLDINGS: return "MORTGAGE_HOLDINGS";
        case PlayerAction::BUY_BACK_PROPERTY: return "BUY_BACK_PROPERTY";
        case PlayerAction::START_TRADE: return "START_TRADE";
        case PlayerAction::START_TRADE_NEW_FIELD: return "START_TRADE_NEW_FIELD";
        case PlayerAction::ROLL_DICE: return "ROLL_DICE";
        case PlayerAction::EXIT_GAME: return "EXIT_GAME";
        case PlayerAction::LOSE: return "LOSE";
        case PlayerAction::FINISH_GAME: return "FINISH_GAME";
    }
    throw std::invalid_argument("Unknown game action");
}

PlayerAction parseAction(const std::string& value) {
    static const std::map<std::string, PlayerAction> actions = {
        {"END_TURN", PlayerAction::END_TURN},
        {"PAY_TAX", PlayerAction::PAY_TAX},
        {"BUY_FIELD", PlayerAction::BUY_FIELD},
        {"BUY_HOUSE", PlayerAction::BUY_HOUSE},
        {"BUY_HOTEL", PlayerAction::BUY_HOTEL},
        {"SELL_HOUSE", PlayerAction::SELL_HOUSE},
        {"SELL_FIELD", PlayerAction::SELL_FIELD},
        {"SELL_HOTEL", PlayerAction::SELL_HOTEL},
        {"USE_CARD", PlayerAction::USE_CARD},
        {"PAY_TO_OTHER_PLAYER", PlayerAction::PAY_TO_OTHER_PLAYER},
        {"TAKE_CARD", PlayerAction::TAKE_CARD},
        {"MORTGAGE_HOLDINGS", PlayerAction::MORTGAGE_HOLDINGS},
        {"BUY_BACK_PROPERTY", PlayerAction::BUY_BACK_PROPERTY},
        {"START_TRADE", PlayerAction::START_TRADE},
        {"START_TRADE_NEW_FIELD", PlayerAction::START_TRADE_NEW_FIELD},
        {"ROLL_DICE", PlayerAction::ROLL_DICE},
        {"EXIT_GAME", PlayerAction::EXIT_GAME},
        {"LOSE", PlayerAction::LOSE},
        {"FINISH_GAME", PlayerAction::FINISH_GAME}
    };
    const auto action = actions.find(value);
    if (action == actions.end()) throw std::invalid_argument("Unknown action");
    return action->second;
}

std::string requestTypeName(RequestType type) {
    switch (type) {
        case RequestType::ACTION: return "ACTION";
        case RequestType::TRADE: return "TRADE";
        case RequestType::MESSAGE: return "MESSAGE";
        case RequestType::NUM: return "NUM";
        case RequestType::SUM: return "SUM";
        case RequestType::TOKEN: return "TOKEN";
    }
    throw std::invalid_argument("Unknown request type");
}

std::string colorJson(const std::optional<Color>& color) {
    if (!color) return "null";
    return std::to_string(static_cast<std::size_t>(*color));
}

std::string tokenJson(const std::optional<Token>& token) {
    if (!token) return "null";
    return std::to_string(static_cast<std::size_t>(*token));
}

template<typename T>
std::string optionalNumber(const std::optional<T>& value) {
    return value ? std::to_string(*value) : "null";
}

std::string boardJson(const BoardModel& board) {
    std::ostringstream out;
    out << "{\"currentPlayer\":" << static_cast<std::size_t>(board.curPlayer)
        << ",\"players\":[";
    for (std::size_t i = 0; i < board.players.size(); ++i) {
        const auto& player = board.players[i];
        if (i) out << ',';
        out << "{\"name\":" << jsonString(player.name)
            << ",\"token\":" << static_cast<std::size_t>(player.token)
            << ",\"tokenName\":" << jsonString(tokenName(player.token))
            << ",\"position\":" << player.position
            << ",\"money\":" << player.money
            << ",\"prisoner\":" << (player.prisoner ? "true" : "false")
            << ",\"alive\":" << (player.alive ? "true" : "false") << '}';
    }
    out << "],\"tiles\":[";
    for (std::size_t i = 0; i < board.field.size(); ++i) {
        const auto& tile = board.field[i];
        if (i) out << ',';
        out << "{\"position\":" << tile.position
            << ",\"name\":" << jsonString(tile.name)
            << ",\"shortName\":";
        if (tile.shortName) {
            out << jsonString(*tile.shortName);
        } else {
            out << "null";
        }
        out
            << ",\"mortgaged\":" << (tile.isMortgaged ? "true" : "false")
            << ",\"cost\":" << optionalNumber(tile.cost)
            << ",\"rent\":" << optionalNumber(tile.costOfParking)
            << ",\"color\":" << colorJson(tile.color)
            << ",\"owner\":" << tokenJson(tile.owner)
            << ",\"houses\":" << optionalNumber(tile.numberOfHouses)
            << ",\"hotels\":" << optionalNumber(tile.numberOfHotels)
            << ",\"houseCost\":" << optionalNumber(tile.costPerHouse)
            << ",\"hotelCost\":" << optionalNumber(tile.costPerHotel);
        if (tile.taxes) {
            const auto& taxes = *tile.taxes;
            out << ",\"taxes\":[" << taxes.startTax << ',' << taxes.taxOneHouse << ','
                << taxes.taxTwoHouses << ',' << taxes.taxThreeHouses << ','
                << taxes.taxFourHouses << ',' << taxes.taxHotel << ']';
        } else {
            out << ",\"taxes\":null";
        }
        out << '}';
    }
    out << "]}";
    return out.str();
}

class GameSession {
public:
    explicit GameSession(const std::vector<EnginePlayer>& players)
        : game(makePlayerData(players),
               [this](PlayerRequest request) { return handleRequest(std::move(request)); },
               [this](const BoardModel& board) { emit("STATE", boardJson(board)); }) {}

    void start() {
        emit("STATE", boardJson(game.getBoard()));
        gameThread = std::thread([this]() {
            try {
                game.run();
                emit("DONE", "{}");
            } catch (const std::exception& error) {
                emit("ERROR", jsonString(error.what()));
            } catch (...) {
                emit("ERROR", jsonString("Unknown game engine error"));
            }
        });
    }

    void stop() {
        game.terminate();
        {
            std::lock_guard lock(requestMutex);
            stopping = true;
        }
        requestCondition.notify_all();
    }

    void join() {
        if (gameThread.joinable()) gameThread.join();
    }

    bool submitReply(const std::string& id, const std::string& type, const std::string& value) {
        std::lock_guard lock(requestMutex);
        if (!pendingRequest || pendingId != id) return false;
        try {
            const Token player = pendingRequest->player;
            PlayerReply reply;
            if (pendingRequest->type == RequestType::ACTION && type == "ACTION") {
                const PlayerAction action = parseAction(value);
                if (action == PlayerAction::FINISH_GAME) {
                    throw std::invalid_argument("Finishing a game manually is not available");
                }
                bool allowed = false;
                for (const auto candidate : pendingRequest->availableActions) {
                    if (candidate == action) allowed = true;
                }
                if (!allowed) throw std::invalid_argument("Action is not available");
                reply = std::make_unique<PlayerReplyData>(player, action);
            } else if (pendingRequest->type == RequestType::TRADE && type == "TRADE") {
                if (value != "PARTICIPATE" && value != "REFUSE") {
                    throw std::invalid_argument("Invalid trade choice");
                }
                reply = std::make_unique<PlayerReplyData>(player,
                    value == "PARTICIPATE" ? PlayerTradeAction::PARTICIPATE : PlayerTradeAction::REFUSE);
            } else if (pendingRequest->type == RequestType::NUM && type == "NUM") {
                std::size_t consumed = 0;
                const unsigned long position = std::stoul(value, &consumed);
                if (consumed != value.size() || position >= Board::FIELD_SIZE) {
                    throw std::invalid_argument("Invalid field position");
                }
                reply = std::make_unique<PlayerReplyData>(player, static_cast<std::size_t>(position));
            } else if (pendingRequest->type == RequestType::SUM && type == "SUM") {
                std::size_t consumed = 0;
                const long sum = std::stol(value, &consumed);
                if (consumed != value.size() || sum < 0 || sum > 1000000000L) {
                    throw std::invalid_argument("Invalid offer");
                }
                reply = std::make_unique<PlayerReplyData>(player, static_cast<int>(sum));
            } else if (pendingRequest->type == RequestType::TOKEN && type == "TOKEN") {
                std::size_t consumed = 0;
                const unsigned long token = std::stoul(value, &consumed);
                if (consumed != value.size() || token < 1 || token > 6 ||
                    token == static_cast<unsigned long>(player)) {
                    throw std::invalid_argument("Invalid player token");
                }
                reply = std::make_unique<PlayerReplyData>(player, static_cast<Token>(token));
            } else {
                throw std::invalid_argument("Reply type does not match the request");
            }
            pendingReply = std::move(reply);
            requestCondition.notify_one();
            return true;
        } catch (const std::exception& error) {
            emit("REPLY_ERROR", jsonString(error.what()));
            return false;
        }
    }

private:
    static std::vector<std::pair<std::string_view, Token>> makePlayerData(
        const std::vector<EnginePlayer>& players) {
        std::vector<std::pair<std::string_view, Token>> data;
        data.reserve(players.size());
        for (const auto& player : players) data.emplace_back(player.name, player.token);
        return data;
    }

    PlayerReply handleRequest(PlayerRequest request) {
        if (request->type == RequestType::MESSAGE) {
            std::ostringstream event;
            event << "{\"playerToken\":" << static_cast<std::size_t>(request->player)
                  << ",\"type\":" << jsonString(requestTypeName(request->type))
                  << ",\"messageType\":" << static_cast<std::size_t>(request->message.type)
                  << ",\"message\":" << jsonString(request->message.message) << '}';
            emit("MESSAGE", event.str());
            return nullptr;
        }

        std::unique_lock lock(requestMutex);
        pendingId = std::to_string(nextRequestId++);
        pendingRequest = std::move(request);
        pendingReply.reset();
        std::ostringstream event;
        event << "{\"id\":" << jsonString(pendingId)
              << ",\"playerToken\":" << static_cast<std::size_t>(pendingRequest->player)
              << ",\"type\":" << jsonString(requestTypeName(pendingRequest->type))
              << ",\"message\":" << jsonString(pendingRequest->message.message)
              << ",\"actions\":[";
        bool firstAction = true;
        for (const auto action : pendingRequest->availableActions) {
            if (action == PlayerAction::FINISH_GAME) continue;
            if (!firstAction) event << ',';
            event << jsonString(actionName(action));
            firstAction = false;
        }
        event << "]}";
        emit("REQUEST", event.str());

        requestCondition.wait(lock, [this]() { return pendingReply || stopping; });
        if (stopping) {
            PlayerReply fallback;
            if (pendingRequest->type == RequestType::ACTION) {
                fallback = std::make_unique<PlayerReplyData>(pendingRequest->player, PlayerAction::END_TURN);
            } else if (pendingRequest->type == RequestType::TRADE) {
                fallback = std::make_unique<PlayerReplyData>(pendingRequest->player, PlayerTradeAction::REFUSE);
            } else if (pendingRequest->type == RequestType::NUM) {
                fallback = std::make_unique<PlayerReplyData>(pendingRequest->player, std::size_t{0});
            } else if (pendingRequest->type == RequestType::SUM) {
                fallback = std::make_unique<PlayerReplyData>(pendingRequest->player, 0);
            } else if (pendingRequest->type == RequestType::TOKEN) {
                fallback = std::make_unique<PlayerReplyData>(pendingRequest->player, Token::DOG);
            }
            pendingRequest.reset();
            return fallback;
        }
        PlayerReply result = std::move(pendingReply);
        pendingRequest.reset();
        pendingId.clear();
        return result;
    }

    void emit(const std::string& kind, const std::string& payload) {
        std::lock_guard lock(outputMutex);
        std::cout << kind << '\t' << payload << '\n' << std::flush;
    }

    LocalGame game;
    std::thread gameThread;
    std::mutex requestMutex;
    std::condition_variable requestCondition;
    std::mutex outputMutex;
    PlayerRequest pendingRequest;
    PlayerReply pendingReply;
    std::string pendingId;
    std::size_t nextRequestId = 1;
    bool stopping = false;
};

std::string base64Decode(const std::string& input) {
    static const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string output;
    unsigned int buffer = 0;
    int bits = -8;
    for (unsigned char character : input) {
        if (character == '=') break;
        const auto value = alphabet.find(static_cast<char>(character));
        if (value == std::string::npos) throw std::invalid_argument("Invalid base64 player name");
        buffer = (buffer << 6) | static_cast<unsigned int>(value);
        bits += 6;
        if (bits >= 0) {
            output.push_back(static_cast<char>((buffer >> bits) & 0xff));
            bits -= 8;
        }
    }
    return output;
}

Token parseToken(const std::string& input) {
    std::size_t consumed = 0;
    const unsigned long value = std::stoul(input, &consumed);
    if (consumed != input.size() || value < 1 || value > 6) {
        throw std::invalid_argument("Invalid player token");
    }
    return static_cast<Token>(value);
}

std::vector<std::string> splitTabs(const std::string& line) {
    std::vector<std::string> result;
    std::size_t start = 0;
    while (true) {
        const std::size_t separator = line.find('\t', start);
        result.push_back(line.substr(start, separator == std::string::npos
                                              ? separator : separator - start));
        if (separator == std::string::npos) break;
        start = separator + 1;
    }
    return result;
}

}

int main() {
    try {
        std::string line;
        if (!std::getline(std::cin, line)) return 0;
        const auto start = splitTabs(line);
        if (start.size() < 5 || start[0] != "START" || start.size() % 2 != 1) {
            throw std::invalid_argument("Expected START followed by 2 to 6 player/token pairs");
        }
        const std::size_t playerCount = (start.size() - 1) / 2;
        if (playerCount < 2 || playerCount > 6) {
            throw std::invalid_argument("A game requires between 2 and 6 players");
        }
        std::vector<EnginePlayer> players;
        players.reserve(playerCount);
        for (std::size_t i = 0; i < playerCount; ++i) {
            const Token token = parseToken(start[1 + i * 2]);
            const std::string name = base64Decode(start[2 + i * 2]);
            if (name.empty() || name.size() > 96) {
                throw std::invalid_argument("Invalid player name");
            }
            for (const auto& player : players) {
                if (player.token == token) throw std::invalid_argument("Duplicate player token");
            }
            players.push_back({name, token});
        }

        GameSession session(players);
        session.start();
        while (std::getline(std::cin, line)) {
            const auto command = splitTabs(line);
            if (command[0] == "STOP") {
                session.stop();
                break;
            }
            if (command.size() == 4 && command[0] == "REPLY") {
                session.submitReply(command[1], command[2], command[3]);
            } else {
                std::cerr << "Ignoring malformed engine command\n";
            }
        }
        session.stop();
        session.join();
    } catch (const std::exception& error) {
        std::cout << "ERROR\t" << jsonString(error.what()) << '\n' << std::flush;
        return 1;
    }
    return 0;
}
