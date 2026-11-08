#ifndef LOCAL_GAME_H
#define LOCAL_GAME_H

#include "Board.h"
#include "PlayerRequests.h"
#include "threads/BoardModel.h"
#include <functional>
#include <string_view>
#include <vector>
#include "Random.h"

struct GameBase {
    virtual ~GameBase() = default;
    virtual void run() = 0;
    virtual const BoardModel& getBoard() = 0;
    virtual void terminate() = 0;
};

class LocalGame final : public GameBase {
public:
    using RequestHandler = std::function<PlayerReply(PlayerRequest)>;
    using SyncHandler = std::function<void(const BoardModel&)>;

    LocalGame(const std::vector<std::pair<std::string_view, Token>>& players,
              RequestHandler requestHandler, SyncHandler syncHandler);
    void run() override;
    PlayerReply send(PlayerRequest request);
    const BoardModel& getBoard() override;
    void terminate() override;
    void sync();

private:
    RandomSource rng;
    Board board;
    BoardModel model;
    RequestHandler requestHandler;
    SyncHandler syncHandler;
};

#endif // LOCAL_GAME_H
