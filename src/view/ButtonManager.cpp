#include "view/ButtonManager.h"

bool Button::isActive() const {
    return active;
}

void Button::setActive(bool b) {
    active = b;
}

void Button::activate() {
    setActive(true);
}

void Button::deactivate() {
    setActive(false);
}


void SpriteButton::setTexture(const std::string& filename) {
    if (!buttonTexture.loadFromFile(filename)) {
        throw std::runtime_error("Не удалось загрузить текстуру"); //TODO: throw more specific type
    }
    buttonTexture.setSmooth(true);
    buttonSprite.setTexture(buttonTexture);
}

void SpriteButton::setSpriteColor(sf::Color color) {
    buttonSprite.setColor(color);
}

void SpriteButton::setVisible(bool visible) {
    this->visible = visible;
}

void SpriteButton::setHitbox(sf::FloatRect hitbox) {
    this->hitbox = hitbox;
    hasHitbox = true;
}

void SpriteButton::draw(sf::RenderWindow& window) {
    if (!visible) return;
    setSpriteColor(isActive() ? sf::Color::White : sf::Color(155, 155, 155));
    window.draw(buttonSprite);
}

bool SpriteButton::isValidTarget(sf::Event::MouseButtonEvent mouse) const {
    if (!visible) return false;
    if (hasHitbox) {
        return hitbox.contains(static_cast<float>(mouse.x), static_cast<float>(mouse.y)) && isActive();
    }
    auto[bx, by] = buttonSprite.getPosition();
    auto[bx_, by_, bw, bh] = buttonSprite.getLocalBounds();
    auto[button, x, y] = mouse;
    return bx <= x && x <= bx + bw &&
           by <= y  && y <= by + bh &&
           isActive();
}

sf::Vector2f SpriteButton::getSize() {
    normalize();
    auto [w, h] = buttonSprite.getTexture()->getSize();
    auto [sw, sh] = buttonSprite.getScale();
    return sf::Vector2f(w * sw, h * sh);
}

void SpriteButton::normalize() {
    buttonSprite.setOrigin(getOrigin());
    buttonSprite.setPosition(getPosition());
    buttonSprite.setRotation(getRotation());
    buttonSprite.setScale(getScale());
}


void ButtonManager::addButton(std::unique_ptr<Button> button, ButtonManager::Handler handler) {
    buttons.emplace_back(std::move(button), std::move(handler));
}

void ButtonManager::addButton(std::unique_ptr<Button> button, const ButtonManager::SimpleHandler& handler) {
    addButton(std::move(button), [handler](sf::Event::MouseButtonEvent e, Button& _){ return handler(e); });
}

void ButtonManager::apply(const ButtonManager::Modifier& m) {
    for(auto& [btn, handler] : buttons) {
        m(*btn);
    }
}

bool ButtonManager::handle(sf::Event::MouseButtonEvent e) {
    for(auto& [btn, handler] : buttons) {
        if(btn->isValidTarget(e) && handler(e, *btn)) {
            return true;
        }
    }
    return false;
}

void ButtonManager::draw(sf::RenderWindow& window) const {
    for (const auto& [btn, handler] : buttons) {
        btn->draw(window);
    }
}
