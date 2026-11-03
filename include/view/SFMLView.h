#ifndef SFMLVIEW_H
#define SFMLVIEW_H
#include "MonopolyFwd.h"
#include "PlayerRequests.h"
#include "EventManager.h"
#include "Board.h"
#include "include/threads/BoardModel.h"
#include "ButtonManager.h"
#include "TooltipManager.h"
#include "threads/ModelThreadManager.h"
#include <SFML/Graphics.hpp>

//#include <pthread/pthread.h>

#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>

inline sf::String fromUtf8(const std::string& text) {
    return sf::String::fromUtf8(text.begin(), text.end());
}

inline unsigned int scaleUi(unsigned int size) {
    return (size * 5 + 2) / 4;
}


struct ShapeHolder {

    std::array<sf::RectangleShape, Board::FIELD_SIZE> fieldRects;

};

struct ButtonHolder {

    std::array<FieldButton, Board::FIELD_SIZE> fieldButtons;

};

class ButtonText {
public:
    ButtonText() = default;
    ButtonText(const std::string &buttonText, sf::Vector2f buttonSize, int charSize, sf::Color bgColor, sf::Color textColor) {
        button.setSize(buttonSize);
        button.setFillColor(bgColor);

        buttonWidth = buttonSize.x;
        buttonHeight = buttonSize.y;

        text.setString(fromUtf8(buttonText));
        text.setCharacterSize(scaleUi(charSize));
        text.setFillColor(textColor);
    }

    void setFont(sf::Font &fonts) {
        text.setFont(fonts);
    }

    void setBackColor(sf::Color color) {
        button.setFillColor(color);
    }

    void setTextColor(sf::Color color) {
        text.setFillColor(color);
    }

    void setPosition(sf::Vector2f point) {
        button.setPosition(point - sf::Vector2f(buttonWidth / 2, buttonHeight / 2));

        // Center text on button:
        float xPos = (point.x) - (text.getLocalBounds().width / 2);
        float yPos = (point.y) - (text.getLocalBounds().height / 2);
        text.setPosition(xPos, yPos);
    }

    void drawTo(sf::RenderWindow &window) {
        window.draw(button);
        window.draw(text);
    }

    // Check if the mouse is within the bounds of the button:
    bool isMouseOver(sf::Event::MouseButtonEvent e) const {
        int mouseX = e.x;
        int mouseY = e.y;

        int buttonPosX = button.getPosition().x;
        int buttonPosY = button.getPosition().y;

        int buttonxPosWidth = button.getPosition().x + buttonWidth;
        int buttonyPosHeight = button.getPosition().y + buttonHeight;

        return mouseX < buttonxPosWidth && mouseX > buttonPosX && mouseY < buttonyPosHeight && mouseY > buttonPosY;
    }

private:
    sf::RectangleShape button;
    sf::Text text;

    int buttonWidth;
    int buttonHeight;
};

class ButtonImage {
public:
    ButtonImage() = default;

    void setTexture(const std::string& textureName) {
        if (!buttonTexture.loadFromFile(textureName)) {
            std::cout << "Не удалось загрузить кнопку завершения хода!\n";
        }
        buttonTexture.setSmooth(true);
        buttonSprite.setTexture(buttonTexture);
        buttonWidth = buttonSprite.getTexture()->getSize().x * buttonSprite.getScale().x;
        buttonHeight = buttonSprite.getTexture()->getSize().y * buttonSprite.getScale().y;
    }

    void setSpriteColor(sf::Color color) {
        buttonSprite.setColor(color);
    }

    void setPosition(sf::Vector2f point) {
        buttonSprite.setPosition(point);
    }

    void setScale(sf::Vector2f scale) {
        buttonSprite.setScale(scale);
        buttonWidth = buttonSprite.getTexture()->getSize().x * buttonSprite.getScale().x;
        buttonHeight = buttonSprite.getTexture()->getSize().y * buttonSprite.getScale().y;
    }

    sf::Vector2f getSize() const {
        return sf::Vector2f(buttonWidth, buttonHeight);
    }

    bool getClickability() const {
        return isClickable;
    }

    void setActive(bool b) {
        isClickable = b;
    }

    void activate() {
        setActive(true);
    }

    void deactivate() {
        setActive(false);
    }

    bool getVisibilty() const {
        return isVisible;
    }

    void setVisible(bool b) {
        isVisible = b;
    }

    void makeVisible() {
        setVisible(true);
    }

    void makeInvisible() {
        setVisible(false);
    }

    void drawTo(sf::RenderWindow &window) {
        if (!isVisible) {
            setSpriteColor(sf::Color::Transparent);
        } else if (!isClickable)
            setSpriteColor(sf::Color(100, 100, 100));
        else
            setSpriteColor(sf::Color::White);
        window.draw(buttonSprite);
    }

    bool isMouseOver(sf::Event::MouseButtonEvent e) const {
        int mouseX = e.x;
        int mouseY = e.y;

        int buttonPosX = buttonSprite.getPosition().x;
        int buttonPosY = buttonSprite.getPosition().y;

        int buttonxPosWidth = buttonSprite.getPosition().x + buttonWidth;
        int buttonyPosHeight = buttonSprite.getPosition().y + buttonHeight;

        return mouseX < buttonxPosWidth && mouseX > buttonPosX && mouseY < buttonyPosHeight && mouseY > buttonPosY;
    }

private:
    sf::Sprite buttonSprite;
    sf::Texture buttonTexture;
    int buttonWidth;
    int buttonHeight;
    bool isClickable = true;
    bool isVisible = true;
};


class SFMLView {
public:
    explicit SFMLView(Manager& manager);
    void dispose();
    void mainLoop();

    void redraw(const BoardModel& board);
    PlayerReply processRequest(Player& p, PlayerRequest req);
private:
    static std::string tokenToString(Token token);
    void onResize(sf::Event::SizeEvent e);

    void drawField(const BoardModel& board);
    void drawPlayers(const BoardModel& board);
    void drawMoney(const BoardModel& board);
    void drawCenterDashboard(const BoardModel& board);
    void drawActionPanel();
    void drawTurnIndicator(const BoardModel& board);
    void drawToken(const BoardModel& board);
    void drawCardInfo(const BoardModel &board, std::optional<std::size_t> index);
    bool handlePropertyActionClick(sf::Event::MouseButtonEvent event);
    void handleSelectedPropertyRequest();
    void drawMessage();
    void drawEnteredT();
    void monopolyDrawer();
    void menuDrawer();
    void handleActionRequest();
    void handleSumRequest();
    void takeMessage(const RequestMessage& req);
    PlayerReply shutdownReply(RequestType type, Token player) const;

    void addActionButton(PlayerAction action, const std::string& texture,
                         std::string tooltip);

    int tmp = 0;

    Manager& manager;

    EventManager events;
    ButtonManager buttons;
    TooltipManager tooltips;

    std::unordered_map<PlayerAction, SpriteButton&> actionButtons;
    std::unordered_map<PlayerAction, std::string> actionTooltips;
    std::vector<std::pair<sf::FloatRect, PlayerAction>> cardActionHitTargets;
    std::optional<std::size_t> pendingPropertyField;
    ShapeHolder shapes;
    ButtonHolder fieldButtons;
    sf::RenderWindow window;
    sf::Font mainFont;

    bool lastKeyIsReturn = false;

    std::mutex windowMutex;
    Monopoly::Threads::ModelThreadManager model;
    std::string enteredText;
    std::vector<std::string> recentEvents;
    sf::Text enteredT;
    ButtonText participateInTrade[2];
    void drawTradeButtons();


    sf::Text box;
    sf::Text message;
    MessageType curMessageType;
    sf::Clock diceAnimationClock;
    sf::Clock turnPulseClock;
    sf::Clock playerMoveClock;
    sf::Clock pendingMoveClock;
    int diceResult[2] = {1, 1};
    bool hasDiceResult = false;
    bool hasPendingMove = false;
    bool isPlayerMoving = false;
    bool isJailTransfer = false;
    Token movingPlayer = Token::FREE_FIELD;
    std::size_t moveStartPosition = 0;
    std::size_t moveDestination = 0;
    std::map<Token, std::size_t> displayedPlayerPositions;

    Token curTurnBy;
    std::optional<std::size_t> curCardIndex;
    bool shouldClose = false;
    std::optional<RequestType> pendingRequestType;
    Token pendingRequestPlayer = Token::FREE_FIELD;

    //
    static constexpr int NUM_OF_BUTTONS = 5;
    ButtonText menuButtons[NUM_OF_BUTTONS];
    bool isTokenDraw = false;
    int numOfAddedPlayers = 0;
    bool isMenu = true;
    bool isEnteringIP = false;
    std::map<Token, ButtonImage> tokenButtons;
    void createMenuButtons();
    void drawMenuButtons();
    void drawTokenButtons();
};

#endif //SFMLVIEW_H