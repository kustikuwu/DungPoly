#pragma once
#include <SFML/System.hpp>


namespace Vec {
    using sf::Vector2;

    template<typename T>
    Vector2<T> transpose(Vector2<T> v) {
        return {v.y, v.x};
    }
}

