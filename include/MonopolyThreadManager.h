#ifndef MONOPOLYTHREADMANAGER_H
#define MONOPOLYTHREADMANAGER_H
#include <thread>
#include "view/SFMLView.h"
#include "unordered_map"

class MonopolyThreadManager {
public:

    void run();
    void startGame();
    void addPlayer(std::unique_ptr<Player> player);
private:

    void runLogic();
    void runUIOnMain(); //UI can only be used on main thread because OSX

    std::mutex mutex;
    std::thread logicThread;

    std::shared_ptr<SFMLView> view;
    std::unordered_map<Token, std::unique_ptr<Player>> players;
};

#endif //MONOPOLYTHREADMANAGER_H