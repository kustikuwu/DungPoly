#include "view/EventManager.h"

void EventManager::handleEvent(sf::Event &e) {
    for(const auto& handler : handlers[e.type]) {
        handler(e);
    }
}
