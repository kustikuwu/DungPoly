#ifndef BUTTONMANAGER_H
#define BUTTONMANAGER_H
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

class Button : public sf::Transformable {
public:
    bool isActive() const;
    void setActive(bool b);
    void activate();
    void deactivate();

    virtual bool isValidTarget(sf::Event::MouseButtonEvent mouse) const = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
private:
    bool active = true;
};

class SpriteButton final : public Button {
public:
    void setTexture(const std::string& filename);
    void setSpriteColor(sf::Color color);
    void setVisible(bool visible);
    void setHitbox(sf::FloatRect hitbox);

    sf::Vector2f getSize();

    bool isValidTarget(sf::Event::MouseButtonEvent mouse) const override;
    void draw(sf::RenderWindow& window) override;
private:

    void normalize();

    sf::Sprite buttonSprite;
    sf::Texture buttonTexture;
    sf::FloatRect hitbox;
    bool visible = true;
    bool hasHitbox = false;
};

class ButtonManager {
public:
    using Handler = std::function<bool(sf::Event::MouseButtonEvent, Button&)>;
    using SimpleHandler = std::function<bool(sf::Event::MouseButtonEvent)>;
    using Modifier = std::function<void(Button&)>;
    void addButton(std::unique_ptr<Button> button, Handler handler);
    void addButton(std::unique_ptr<Button> button, const SimpleHandler& handler);
    void apply(const Modifier& m);

    bool handle(sf::Event::MouseButtonEvent e);
    void draw(sf::RenderWindow& window) const;
private:
    std::vector<std::pair<std::unique_ptr<Button>, Handler>> buttons;
};

class FieldButton {
public:
    void setSize(int x, int y, int w, int h) {
        startX = x;
        startY = y;
        width = w;
        height = h;
    }

    bool isValidTarget(sf::Event::MouseButtonEvent mouse) const {
        auto[button, x, y] = mouse;
        return startX <= x && x <= startX + width &&
               startY <= y  && y <= startY + height;
    }
private:
    int startX, startY;
    int width, height;

};

#endif //BUTTONMANAGER_H