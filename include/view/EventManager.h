#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <functional>
#include <unordered_map>
#include <vector>
#include <SFML/Graphics.hpp>

class EventManager {
public:
    void handleEvent(sf::Event &e);

    template<sf::Event::EventType type, typename F>
    void addHandler(F handler) {
        using T = sf::Event::EventType;
        static_assert(type != T::Count); //Not an actual event
        handlers[type].emplace_back([handler](sf::Event &e) {
            if constexpr (type == T::Resized) handler(e.size);
            else if constexpr (type == T::TextEntered) handler(e.text);
            else if constexpr (type == T::KeyPressed) handler(e.key);
            else if constexpr (type == T::KeyReleased) handler(e.key);
            else if constexpr (type == T::MouseWheelMoved) handler(e.mouseWheel);
            else if constexpr (type == T::MouseWheelScrolled) handler(e.mouseWheelScroll);
            else if constexpr (type == T::MouseButtonPressed) handler(e.mouseButton);
            else if constexpr (type == T::MouseButtonReleased) handler(e.mouseButton);
            else if constexpr (type == T::MouseMoved) handler(e.mouseMove);
            else if constexpr (type == T::JoystickButtonPressed) handler(e.joystickButton);
            else if constexpr (type == T::JoystickButtonReleased) handler(e.joystickButton);
            else if constexpr (type == T::JoystickMoved) handler(e.joystickMove);
            else if constexpr (type == T::JoystickConnected) handler(e.joystickConnect);
            else if constexpr (type == T::JoystickDisconnected) handler(e.joystickConnect);
            else if constexpr (type == T::TouchBegan) handler(e.touch);
            else if constexpr (type == T::TouchMoved)  handler(e.touch);
            else if constexpr (type == T::TouchEnded) handler(e.touch);
            else if constexpr (type == T::SensorChanged) handler(e.sensor);
            else handler();
        });
    }

private:
    using EventHandler = std::function<void(sf::Event &)>;
    std::unordered_map<sf::Event::EventType, std::vector<EventHandler>> handlers;
};

#endif //EVENTMANAGER_H