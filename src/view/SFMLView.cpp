#include "view/SFMLView.h"
#include "Manager.h"
#include "Field.h"
#include <view/VectorOps.h>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <cmath>
#include <sstream>

using Vec::transpose;

sf::Color getColor(Token tok) {
    switch (tok) {
        case Token::DOG:
            return sf::Color::White;
        case Token::HAT:
            return sf::Color::Red;
        case Token::BOOT:
            return sf::Color::Blue;
        case Token::CAT:
            return sf::Color::Cyan;
        case Token::CAR:
            return sf::Color::Magenta;
        case Token::SHIP:
            return sf::Color::Green;
        case Token::FREE_FIELD:
            return sf::Color::Transparent;
    }
    return sf::Color::Transparent;
}

sf::Color getColor(Color color) {
    switch (color) {

        case Color::COL1:
            return sf::Color(105, 50, 50);
        case Color::COL2:
            return sf::Color(36, 194, 181);
        case Color::COL3:
            return sf::Color(213, 0, 112);
        case Color::COL4:
            return sf::Color(255, 135, 0);
        case Color::COL5:
            return sf::Color(182, 0, 0);
        case Color::COL6:
            return sf::Color(175, 165, 15);
        case Color::COL7:
            return sf::Color(30, 175, 15);
        case Color::COL8:
            return sf::Color(15, 109, 175);
        case Color::NO_COL:
            return sf::Color::Transparent;
    }
    return sf::Color::Transparent;
}

std::string SFMLView::tokenToString(Token token) {
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

SFMLView::SFMLView(Manager& manager) : manager(manager) {
    window.create(sf::VideoMode(sf::VideoMode::getDesktopMode().width * 20 / 21,
                                sf::VideoMode::getDesktopMode().height * 9 / 10),
                  "DungPoly", sf::Style::Titlebar | sf::Style::Close);
    window.setPosition(sf::Vector2i(0, 0));
    window.setFramerateLimit(60);
    mainFont.loadFromFile("resources/Ubuntu-R.ttf");

    enteredT.setFont(mainFont);
    enteredT.setCharacterSize(scaleUi(15));
    enteredT.setFillColor(sf::Color::White);
    box.setFont(mainFont);
    box.setCharacterSize(scaleUi(15));
    box.setFillColor(sf::Color::White);
    message.setFont(mainFont);
    message.setCharacterSize(scaleUi(20));
    message.setFillColor(sf::Color::White);


    addActionButton(PlayerAction::END_TURN,
                    "resources/END_TURN.png",
                    "Завершить ход");
    addActionButton(PlayerAction::ROLL_DICE,
                    "resources/ROLL_DICE.png",
                    "Бросить кубики");
    addActionButton(PlayerAction::PAY_TAX,
                    "resources/PAY_TAX_RU.png",
                    "Уплатить налог банку");
    addActionButton(PlayerAction::PAY_TO_OTHER_PLAYER,
                    "resources/PAY_TO_OTHER_PLAYER.png",
                    "Заплатить другому игроку");
    addActionButton(PlayerAction::BUY_FIELD,
                    "resources/BUY_FIELD.png",
                    "Купить эту собственность");
    addActionButton(PlayerAction::SELL_FIELD,
                    "resources/SELL_FIELD_RU.png",
                    "Продать эту собственность");
    addActionButton(PlayerAction::BUY_HOUSE,
                    "resources/BUY_HOUSE.png",
                    "Построить дом");
    addActionButton(PlayerAction::SELL_HOUSE,
                    "resources/SELL_HOUSE.png",
                    "Продать дом");
    addActionButton(PlayerAction::BUY_HOTEL,
                    "resources/BUY_HOTEL_RU.png",
                    "Построить гостиницу");
    addActionButton(PlayerAction::SELL_HOTEL,
                    "resources/SELL_HOTEL_RU.png",
                    "Продать гостиницу");
    addActionButton(PlayerAction::TAKE_CARD,
                    "resources/TAKE_CARD.png",
                    "Взять карточку");
    addActionButton(PlayerAction::USE_CARD,
                    "resources/USE_CARD.png",
                    "Использовать карточку");
    addActionButton(PlayerAction::START_TRADE_NEW_FIELD,
                    "resources/START_TRADE_NEW_FIELD.png",
                    "Отказаться от покупки и начать аукцион");
    addActionButton(PlayerAction::START_TRADE,
                    "resources/START_TRADE.png",
                    "Начать обмен с другим игроком");
    addActionButton(PlayerAction::MORTGAGE_HOLDINGS,
                    "resources/MORTGAGE_HOLDINGS.png",
                    "Заложить собственность");
    addActionButton(PlayerAction::BUY_BACK_PROPERTY,
                    "resources/BUY_BACK_PROPERTY.png",
                    "Выкупить заложенную собственность");
    addActionButton(PlayerAction::FINISH_GAME,
                    "resources/EXIT_GAME.png",
                    "Завершить игру для всех игроков");
    addActionButton(PlayerAction::EXIT_GAME,
                    "resources/FINISH_GAME.png",
                    "Выйти из игры");


    tokenButtons[Token::DOG].setTexture("resources/DOG.png");
    tokenButtons[Token::BOOT].setTexture("resources/BOOT.png");
    tokenButtons[Token::CAR].setTexture("resources/CAR.png");
    tokenButtons[Token::CAT].setTexture("resources/CAT.png");
    tokenButtons[Token::HAT].setTexture("resources/HAT.png");
    tokenButtons[Token::SHIP].setTexture("resources/SHIP.png");

    events.addHandler<sf::Event::Closed>([this]() { dispose(); });
    events.addHandler<sf::Event::Resized>([this](sf::Event::SizeEvent e) {
        window.setView(sf::View(sf::FloatRect(0, 0, e.width, e.height)));
        onResize(e);
    });
    events.addHandler<sf::Event::MouseMoved>([this](sf::Event::MouseMoveEvent e) {
        if (!isMenu) {
            box.setString(fromUtf8(std::string(this->tooltips.getTooltip(e))));
            box.setPosition(sf::Vector2f(20, window.getSize().y - box.getLocalBounds().height * 2));
        }
    });
    events.addHandler<sf::Event::TextEntered>([this](sf::Event::TextEvent e) {
        if (model.hasRequest(RequestType::SUM) || isEnteringIP) {
            if (e.unicode == 0x08 && !enteredText.empty()) {
                enteredText.pop_back();
            } else if (e.unicode < 0x80) {
                if (std::isdigit((char) e.unicode) || (isEnteringIP && e.unicode == '.')) {
                    enteredText.push_back((char) e.unicode);
                }
            }
        }
    });
    events.addHandler<sf::Event::KeyPressed>([this](sf::Event::KeyEvent e) {
        lastKeyIsReturn = e.code == sf::Keyboard::Return;
    });


    onResize({window.getSize().x, window.getSize().y});

    events.addHandler<sf::Event::MouseButtonPressed>([this](sf::Event::MouseButtonEvent e) {
        if (!isMenu) {
            buttons.handle(e);
            if (e.button == sf::Mouse::Left) {
                if (handlePropertyActionClick(e)) return;
                if (model.hasRequest(RequestType::NUM)) {
                    for (std::size_t i = 0; i < fieldButtons.fieldButtons.size(); i++) {
                        if (fieldButtons.fieldButtons[i].isValidTarget(e)) {
                            if (auto request = model.getRequest(); request.has_value()) {
                                assert(request.value()->type == RequestType::NUM);
                                takeMessage(request.value()->message);
                                model.sendReply(std::make_unique<PlayerReplyData>(request.value()->player, i));
                            }
                            break;
                        }
                    }
                } else if (model.hasRequest(RequestType::TRADE)) {
                    if (participateInTrade[0].isMouseOver(e)) {
                        if (auto request = model.getRequest(); request.has_value()) {
                            assert(request.value()->type == RequestType::TRADE);
                            takeMessage(request.value()->message);
                            model.sendReply(std::make_unique<PlayerReplyData>(request.value()->player,
                                                                              PlayerTradeAction::PARTICIPATE));
                        }
                    } else if (participateInTrade[1].isMouseOver(e)) {
                        if (auto request = model.getRequest(); request.has_value()) {
                            assert(request.value()->type == RequestType::TRADE);
                            takeMessage(request.value()->message);
                            model.sendReply(std::make_unique<PlayerReplyData>(request.value()->player,
                                                                              PlayerTradeAction::REFUSE));
                        }
                    }
                } else if (model.hasRequest(RequestType::TOKEN)) {
                    for (auto& t: tokenButtons) {
                        if (t.second.isMouseOver(e)) {
                            if (auto request = model.getRequest(); request.has_value()) {
                                assert(request.value()->type == RequestType::TOKEN);
                                takeMessage(request.value()->message);
                                model.sendReply(std::make_unique<PlayerReplyData>(request.value()->player,
                                                                                  t.first));
                            }
                        }
                    }
                }
                if (!model.hasRequest(RequestType::NUM) &&
                    !model.hasRequest(RequestType::TRADE) &&
                    !model.hasRequest(RequestType::TOKEN)) {
                    for (std::size_t i = 0; i < fieldButtons.fieldButtons.size(); i++) {
                        if (fieldButtons.fieldButtons[i].isValidTarget(e)) {
                            curCardIndex = i;
                            break;
                        }
                    }
                }
            } else if (e.button == sf::Mouse::Right) {
                for (std::size_t i = 0; i < fieldButtons.fieldButtons.size(); i++) {
                    if (fieldButtons.fieldButtons[i].isValidTarget(e)) {
                        curCardIndex = i;
                        break;
                    }
                }
            }
        }
        if (isMenu) {
            if (e.button == sf::Mouse::Left) {
                if (numOfAddedPlayers >= 2 && menuButtons[0].isMouseOver(e)) {
                    isMenu = false;
                    this->manager.startGame();
                } else if (numOfAddedPlayers < 6 && menuButtons[1].isMouseOver(e)) {
                    isTokenDraw = true;
                } else if (menuButtons[2].isMouseOver(e)) {

                } else if (menuButtons[3].isMouseOver(e)) {
                    dispose();
                } else if (menuButtons[4].isMouseOver(e) && numOfAddedPlayers == 1) {
                    isEnteringIP = true;
                }

                if (isTokenDraw) {
                    for (auto& t : tokenButtons) {
                        if (t.second.getClickability() && t.second.isMouseOver(e)) {
                            std::string name = "Игрок " + std::to_string(numOfAddedPlayers + 1);
                            this->manager.addPlayer(std::make_unique<LocalPlayer>(t.first, name, *this));
                            t.second.deactivate();
                            isTokenDraw = false;
                            ++numOfAddedPlayers;
                        }
                    }
                }

            }
        }
    });
}

void SFMLView::dispose() {
    std::optional<RequestType> requestType;
    Token requestPlayer = Token::FREE_FIELD;
    {
        std::lock_guard g(windowMutex);
        if (shouldClose) return;
        shouldClose = true;
        requestType = pendingRequestType;
        requestPlayer = pendingRequestPlayer;
    }
    manager.requestShutdown();
    if (requestType && *requestType != RequestType::MESSAGE) {
        model.sendReply(shutdownReply(*requestType, requestPlayer));
    }
}


void SFMLView::mainLoop() {
    while (window.isOpen()) {
        {
            std::lock_guard m(windowMutex);
            if (shouldClose) {
                window.close();
                break;
            }
        }
        handleActionRequest();
        handleSelectedPropertyRequest();
        handleSumRequest();
        sf::Event e{};
        while (window.pollEvent(e)) {
            events.handleEvent(e);
        }
        window.clear();
        monopolyDrawer();
        if (isMenu) {
            menuDrawer();
        }

        window.display();
    }
}

bool doesFit(const sf::Text& t, float width) {
    return t.getLocalBounds().width < width;
}

void SFMLView::drawField(const BoardModel& board) {
    for (const auto& fieldTile : board.field) {
        int i = fieldTile.position;
        auto& viewTile = shapes.fieldRects[i];
        viewTile.setOutlineColor(curCardIndex == static_cast<std::size_t>(i)
                                 ? sf::Color(255, 215, 100)
                                 : sf::Color::White);
        bool isSide = (i / 10) % 2 == 0;
        auto[x, y] = viewTile.getPosition();
        auto[w, h] = isSide ? viewTile.getSize() : transpose(viewTile.getSize());
        const float shift = (w / 4);
        window.draw(viewTile);

        if (fieldTile.color.has_value()) {
            const float size = w / 6;
            sf::RectangleShape colorRect(sf::Vector2f(size, h));
            colorRect.setFillColor(getColor(fieldTile.color.value()));
            if (!isSide) colorRect.rotate(90);
            switch (i / 10) {
                case 0:
                    colorRect.setPosition(viewTile.getPosition() + sf::Vector2f(w - size, 0));
                    break;
                case 1:
                    colorRect.setPosition(viewTile.getPosition() + sf::Vector2f(h, w - size));
                    break;
                case 2:
                    colorRect.setPosition(viewTile.getPosition());
                    break;
                case 3:
                    colorRect.setPosition(viewTile.getPosition() + sf::Vector2f(h, 0));
                    break;
            }
            window.draw(colorRect);
        }

        if (fieldTile.owner.has_value()) {
            sf::Color color = getColor(fieldTile.owner.value());
            const float size = w / 6;
            sf::RectangleShape ownerLabel(sf::Vector2f(size, size));
            ownerLabel.setFillColor(color);
            switch (i / 10) {
                case 0:
                    ownerLabel.setPosition(viewTile.getPosition());
                    break;
                case 1:
                    ownerLabel.setPosition(viewTile.getPosition() + sf::Vector2f(h - size, 0));
                    break;
                case 2:
                    ownerLabel.setPosition(viewTile.getPosition() + sf::Vector2f(w - size, h - size));
                    break;
                case 3:
                    ownerLabel.setPosition(viewTile.getPosition() + sf::Vector2f(0, w - size));
                    break;
            }
            window.draw(ownerLabel);
            auto[ow, oh] = ownerLabel.getSize();
            if (fieldTile.isMortgaged) {
                sf::Color mortgageColor = sf::Color(150, 150, 150);
                const float mortgageSize = w / 20;
                sf::CircleShape mortgageLabel(mortgageSize);
                mortgageLabel.setOrigin(mortgageLabel.getRadius(), mortgageLabel.getRadius());
                mortgageLabel.setFillColor(mortgageColor);
                switch (i / 10) {
                    case 0:
                        mortgageLabel.setPosition(viewTile.getPosition() + sf::Vector2f(ow / 2, oh / 2));
                        break;
                    case 1:
                        mortgageLabel.setPosition(
                                viewTile.getPosition() + sf::Vector2f(h - size, 0) + sf::Vector2f(ow / 2, oh / 2));
                        break;
                    case 2:
                        mortgageLabel.setPosition(viewTile.getPosition() + sf::Vector2f(w - size, h - size) +
                                                  sf::Vector2f(ow / 2, oh / 2));
                        break;
                    case 3:
                        mortgageLabel.setPosition(
                                viewTile.getPosition() + sf::Vector2f(0, w - size) + sf::Vector2f(ow / 2, oh / 2));
                        break;
                }
                window.draw(mortgageLabel);
            }
        }

        std::string s;
        if (fieldTile.shortName.has_value())
            s = std::string(fieldTile.shortName.value());
        else
            s = std::string(fieldTile.name);
        sf::Text name(fromUtf8(s), mainFont);
        int fsize = 10;
        name.setCharacterSize(fsize);
        while (!doesFit(name, h * 0.7f)) {
            auto lastSpace = s.rfind(' ');
            if (lastSpace == std::string::npos) break;
            s.replace(lastSpace, 1, "\n");
            name.setString(fromUtf8(s));
        }
        int j = 0;
        while (doesFit(name, h * 0.7f) && j++ < 10) {
            name.setCharacterSize(++fsize);
        }
        auto[nx, ny, nw, nh] = name.getLocalBounds();
        const float align = ((h - nw) / 2);
        if (i % 10 == 0) {
            name.setOrigin(nx + nw / 2, ny + nh / 2);
            name.setPosition(x + h / 2, y + w / 2);
            name.rotate(isSide ? 45 : -45);
        } else {
            switch (i / 10) {
                case 0:
                    if (i == 2 || i == 4 || i == 5 || i == 7) {
                        name.rotate(+90);
                        name.setPosition(x + w, y + align);
                    } else {
                        name.rotate(+90);
                        name.setPosition(x + w - shift, y + align);
                    }
                    break;
                case 1:
                    if (i == 12 || i == 15 || i == 17) {
                        name.setPosition(x + align, y + h - nh / 2);
                    } else {
                        name.setPosition(x + align, y + h - shift - nh / 2);
                    }
                    break;
                case 2:
                    name.rotate(-90);
                    if (i == 22 || i == 25 || i == 28) {
                        name.setPosition(x, y + h - align);
                    } else {
                        name.setPosition(x + shift, y + h - align);
                    }
                    break;
                case 3:
                    if (i == 33 || i == 35 || i == 36 || i == 38) {
                        name.setPosition(x + align, y);
                    } else {
                        name.setPosition(x + align, y + shift);
                    }
                    break;
            }
        }

        window.draw(name);
    }
    {
        sf::Text curTurn(fromUtf8("Ход игрока: "), mainFont);
        curTurn.setPosition(shapes.fieldRects[0].getPosition() + sf::Vector2f(0, shapes.fieldRects[0].getSize().y + 5));
        auto[x, y, w, h] = curTurn.getLocalBounds();
        const float pulse = 2.0f + std::abs(std::sin(turnPulseClock.getElapsedTime().asSeconds() * 4.0f)) * 2.0f;
        sf::CircleShape p(h / 2 + pulse);
        p.setPosition(curTurn.getPosition() + sf::Vector2f(x + w + 20 - pulse, y - pulse));
        p.setFillColor(getColor(curTurnBy));
        p.setOutlineColor(sf::Color(255, 220, 80));
        p.setOutlineThickness(2);
        window.draw(curTurn);
        window.draw(p);
    }
}

void moveTo(sf::Shape& s, const sf::RectangleShape& to) {
    auto[x, y] = to.getPosition();
    auto[w, h] = to.getSize();
    s.move(x + w / 2, y + h / 2);
}

void SFMLView::drawPlayers(const BoardModel& board) {
    curTurnBy = board.curPlayer;
    for (const auto& player : board.players) {
        const std::size_t i = player.position;
        auto [lastPosition, inserted] = displayedPlayerPositions.emplace(player.token, i);
        if (!inserted && lastPosition->second != i) {
            if (player.token == curTurnBy) {
                movingPlayer = player.token;
                moveStartPosition = lastPosition->second;
                moveDestination = i;
                isJailTransfer = player.prisoner && i == PRISON_POS;
                hasPendingMove = true;
                pendingMoveClock.restart();
            } else if (player.token == movingPlayer) {
                hasPendingMove = false;
                isPlayerMoving = false;
            }
            lastPosition->second = i;
        }

        if (hasPendingMove && movingPlayer == player.token) {
            const int diceElapsedMs = diceAnimationClock.getElapsedTime().asMilliseconds();
            if (hasDiceResult && movingPlayer == curTurnBy && diceElapsedMs >= 1500) {
                isPlayerMoving = true;
                hasPendingMove = false;
                playerMoveClock.restart();
            } else if (!hasDiceResult && pendingMoveClock.getElapsedTime().asMilliseconds() >= 350) {
                hasPendingMove = false;
            }
        }

        int row = i / 10;
        float k = 6;
        bool isSide = row % 2 == 0;
        auto& tile = shapes.fieldRects[i];
        auto[w, h] = isSide ? tile.getSize() : transpose(tile.getSize());
        sf::CircleShape p(w / 20);
        p.setOrigin(p.getRadius(), p.getRadius());
        p.setFillColor(getColor(player.token));
        if (i % 10 == 0) {
            float a = w / k, b = a / 2, c = h / k, d = c / 2;
            switch (player.token) {
                case Token::DOG:
                    p.move(-a + b, c + d);
                    break;
                case Token::HAT:
                    p.move(a + b, -c + d);
                    break;
                case Token::BOOT:
                    p.move(a - b, -c - d);
                    break;
                case Token::CAT:
                    p.move(-a - b, c - d);
                    break;
                case Token::CAR:
                    p.move(a + b, -c - d);
                    break;
                case Token::SHIP:
                    p.move(-a - b, c + d);
                    break;
                case Token::FREE_FIELD:
                    assert(false);
            }
            auto[px, py] = p.getPosition();
            switch (row) {
                case 0:
                    p.setPosition(px, py);
                    break;
                case 1:
                    p.setPosition(-px, py);
                    break;
                case 2:
                    p.setPosition(-px, -py);
                    break;
                case 3:
                    p.setPosition(px, -py);
                    break;
            }
        } else {
            p.setPosition(-w / k, 0);
            switch (player.token) {
                case Token::DOG:
                    p.move(0, h / (2 * k));
                    break;
                case Token::HAT:
                    p.move(0, -h / (2 * k));
                    break;
                case Token::BOOT:
                    p.move(-w / k, -h / (2 * k));
                    break;
                case Token::CAT:
                    p.move(-w / k, h / (2 * k));
                    break;
                case Token::CAR:
                    p.move(-w / (2 * k), -h / k);
                    break;
                case Token::SHIP:
                    p.move(-w / (2 * k), h / k);
                    break;
                case Token::FREE_FIELD:
                    assert(false);
            }
            auto[px, py] = p.getPosition();
            switch (row) {
                case 0:
                    p.setPosition(px, py);
                    break;
                case 1:
                    p.setPosition(py, px);
                    break;
                case 2:
                    p.setPosition(-px, -py);
                    break;
                case 3:
                    p.setPosition(-py, -px);
                    break;
            }
        }
        moveTo(p, tile);

        if ((hasPendingMove || isPlayerMoving) && player.token == movingPlayer) {
            const auto tileCenter = [this](std::size_t position) {
                const auto& field = shapes.fieldRects[position];
                return field.getPosition() + field.getSize() / 2.0f;
            };
            const sf::Vector2f playerOffset = p.getPosition() - tileCenter(i);

            if (hasPendingMove) {
                p.setPosition(tileCenter(moveStartPosition) + playerOffset);
            } else {
                const int totalSteps = static_cast<int>((moveDestination + Board::FIELD_SIZE -
                                                         moveStartPosition) % Board::FIELD_SIZE);
                const int durationMs = isJailTransfer
                        ? 450
                        : std::clamp(totalSteps * 65, 260, 780);
                const float progress = std::clamp(
                        static_cast<float>(playerMoveClock.getElapsedTime().asMilliseconds()) / durationMs,
                        0.0f, 1.0f);
                const sf::Vector2f start = tileCenter(moveStartPosition);
                const sf::Vector2f finish = tileCenter(moveDestination);
                sf::Vector2f position;

                if (isJailTransfer) {
                    position = start + (finish - start) * progress;
                } else {
                    const float routePosition = progress * static_cast<float>(totalSteps);
                    const int completedSteps = std::min(static_cast<int>(routePosition), totalSteps - 1);
                    const float stepProgress = routePosition - static_cast<float>(completedSteps);
                    const float easedProgress = stepProgress * stepProgress * (3.0f - 2.0f * stepProgress);
                    const std::size_t from = (moveStartPosition + completedSteps) % Board::FIELD_SIZE;
                    const std::size_t to = (from + 1) % Board::FIELD_SIZE;
                    const sf::Vector2f segmentStart = tileCenter(from);
                    const sf::Vector2f segmentFinish = tileCenter(to);
                    position = segmentStart + (segmentFinish - segmentStart) * easedProgress;
                }
                p.setPosition(position + playerOffset);

                if (progress >= 1.0f) {
                    isPlayerMoving = false;
                    isJailTransfer = false;
                }
            }
        }

        if (player.token == curTurnBy) {
            const float pulse = 2.0f + std::abs(std::sin(turnPulseClock.getElapsedTime().asSeconds() * 4.0f)) * 2.0f;
            sf::CircleShape highlight(p.getRadius() + pulse);
            highlight.setOrigin(highlight.getRadius(), highlight.getRadius());
            highlight.setPosition(p.getPosition());
            highlight.setFillColor(sf::Color::Transparent);
            highlight.setOutlineColor(sf::Color(255, 220, 80));
            highlight.setOutlineThickness(2);
            window.draw(highlight);
        }
        window.draw(p);
    }
}

void SFMLView::drawMessage() {
    takeMessage(model.getMessage());
    if (hasDiceResult && diceAnimationClock.getElapsedTime().asMilliseconds() >= 1500 &&
        !hasPendingMove && !isPlayerMoving) {
        hasDiceResult = false;
    }
    if (hasDiceResult) {
        curMessageType = MessageType::DICE;
    }
    auto[W, H] = window.getSize();
    switch (curMessageType) {
        case MessageType::CHANCE: {
            message.setCharacterSize(scaleUi(20));
            auto[nx, ny, nw, nh] = message.getLocalBounds();
            message.setOrigin(nx + nw / 2, ny + nh / 2);
            message.setPosition(sf::Vector2f(float(W) / 2, float(H) / 2));
            break;
        }
        case MessageType::PUBLIC_TREASURY: {
            message.setCharacterSize(scaleUi(20));
            auto[nx, ny, nw, nh] = message.getLocalBounds();
            message.setOrigin(nx + nw / 2, ny + nh / 2);
            message.setPosition(sf::Vector2f(float(W) / 2, float(H) / 2));
            break;
        }
        case MessageType::INFO: {
            const auto& baseRect = shapes.fieldRects[0];
            message.setCharacterSize(scaleUi(15));
            message.setPosition(baseRect.getPosition() +
                                sf::Vector2f(message.getLocalBounds().width, -message.getLocalBounds().height));
            break;
        }
        case MessageType::DICE: {
            if (hasDiceResult) {
                constexpr int animationDurationMs = 1100;
                const int elapsedMs = diceAnimationClock.getElapsedTime().asMilliseconds();
                const bool isRolling = elapsedMs < animationDurationMs;
                const float centerX = static_cast<float>(W) / 2;
                const float centerY = static_cast<float>(H) / 2;
                const float diceSize = static_cast<float>(scaleUi(54));
                const float gap = static_cast<float>(scaleUi(18));
                const float panelWidth = diceSize * 2 + gap + static_cast<float>(scaleUi(60));
                const float panelHeight = static_cast<float>(scaleUi(160));
                sf::RectangleShape panel({panelWidth, panelHeight});
                panel.setOrigin(panelWidth / 2, panelHeight / 2);
                panel.setPosition(centerX, centerY);
                panel.setFillColor(sf::Color(15, 15, 15, 235));
                panel.setOutlineColor(sf::Color(230, 230, 230));
                panel.setOutlineThickness(2);
                window.draw(panel);

                sf::Text heading(fromUtf8(isRolling ? "Бросаем кубики..." : "Результат броска"), mainFont);
                heading.setCharacterSize(scaleUi(20));
                auto[headingX, headingY, headingWidth, headingHeight] = heading.getLocalBounds();
                heading.setOrigin(headingX + headingWidth / 2, headingY + headingHeight / 2);
                heading.setPosition(centerX, centerY - panelHeight * 0.32f);
                window.draw(heading);

                const int frame = elapsedMs / 65;
                const float bounce = isRolling
                        ? std::abs(std::sin(static_cast<float>(elapsedMs) / 75.0f)) * scaleUi(8)
                        : 0.0f;
                const float firstX = centerX - diceSize - gap / 2;
                const float diceY = centerY - diceSize / 2 + bounce;
                const float dotRadius = diceSize / 13;
                for (int die = 0; die < 2; ++die) {
                    const int face = isRolling
                            ? (frame * (die + 2) + die * 3) % 6 + 1
                            : diceResult[die];
                    sf::RectangleShape dice({diceSize, diceSize});
                    dice.setPosition(firstX + die * (diceSize + gap), diceY);
                    dice.setFillColor(sf::Color(250, 250, 245));
                    dice.setOutlineColor(sf::Color(30, 30, 30));
                    dice.setOutlineThickness(2);
                    window.draw(dice);

                    const float left = dice.getPosition().x + diceSize * 0.25f;
                    const float middle = dice.getPosition().x + diceSize * 0.5f;
                    const float right = dice.getPosition().x + diceSize * 0.75f;
                    const float top = diceY + diceSize * 0.25f;
                    const float center = diceY + diceSize * 0.5f;
                    const float bottom = diceY + diceSize * 0.75f;
                    const sf::Vector2f pips[] = {
                            {left, top}, {middle, top}, {right, top},
                            {left, center}, {middle, center}, {right, center},
                            {left, bottom}, {middle, bottom}, {right, bottom}
                    };
                    const bool visible[] = {
                            face == 2 || face == 3 || face == 4 || face == 5 || face == 6,
                            face == 6,
                            face == 4 || face == 5 || face == 6,
                            face == 6,
                            face == 1 || face == 3 || face == 5,
                            face == 6,
                            face == 4 || face == 5 || face == 6,
                            face == 6,
                            face == 2 || face == 3 || face == 4 || face == 5 || face == 6
                    };
                    for (std::size_t pip = 0; pip < std::size(visible); ++pip) {
                        if (!visible[pip]) continue;
                        sf::CircleShape dot(dotRadius);
                        dot.setOrigin(dotRadius, dotRadius);
                        dot.setPosition(pips[pip]);
                        dot.setFillColor(sf::Color(25, 25, 25));
                        window.draw(dot);
                    }
                }

                if (!isRolling) {
                    sf::Text result(fromUtf8("Выпало: " + std::to_string(diceResult[0]) +
                                             " + " + std::to_string(diceResult[1])), mainFont);
                    result.setCharacterSize(scaleUi(18));
                    auto[resultX, resultY, resultWidth, resultHeight] = result.getLocalBounds();
                    result.setOrigin(resultX + resultWidth / 2, resultY + resultHeight / 2);
                    result.setPosition(centerX, centerY + panelHeight * 0.34f);
                    window.draw(result);
                }
                return;
            }
            return;
        }
    }
    window.draw(message);
}

void SFMLView::monopolyDrawer() {
    if (!isMenu) {
        BoardModel m = model.getBoard();
        if(!m.isInitialized) return;
        drawField(m);
        drawPlayers(m);
        drawMoney(m);
        drawTurnIndicator(m);
        drawMessage();
        drawCardInfo(m, curCardIndex);
        drawActionPanel();
        if (model.hasRequest(RequestType::TRADE)) {
            drawTradeButtons();
        }
        drawEnteredT();
        buttons.draw(window);
        window.draw(box);
    }
}

void SFMLView::menuDrawer() {
    drawMenuButtons();
    if (isTokenDraw) {
        drawTokenButtons();
    }
    drawEnteredT();
    if(isEnteringIP && lastKeyIsReturn) {
        std::string ip = std::move(enteredText);
        enteredText.clear();
        manager.startRemoteGame(sockpp::inet_address(ip, NETWORK_PORT));
        isMenu = false;
        isEnteringIP = false;
        lastKeyIsReturn = false;
    }
}

void SFMLView::redraw(const BoardModel& board) {
    model.sync(board);
}

PlayerReply SFMLView::processRequest(Player& p, PlayerRequest req) {
    {
        std::lock_guard g(windowMutex);
        if (shouldClose) return shutdownReply(req->type, req->player);
        pendingRequestType = req->type;
        pendingRequestPlayer = req->player;
    }
    PlayerReply reply = model.processRequest(std::move(req));
    {
        std::lock_guard g(windowMutex);
        pendingRequestType.reset();
        pendingRequestPlayer = Token::FREE_FIELD;
    }
    return reply;
}

PlayerReply SFMLView::shutdownReply(RequestType type, Token player) const {
    switch (type) {
        case RequestType::ACTION:
            return std::make_unique<PlayerReplyData>(player, PlayerAction::EXIT_GAME);
        case RequestType::TRADE:
            return std::make_unique<PlayerReplyData>(player, PlayerTradeAction::REFUSE);
        case RequestType::NUM:
            return std::make_unique<PlayerReplyData>(player, std::size_t{0});
        case RequestType::SUM:
            return std::make_unique<PlayerReplyData>(player, 0);
        case RequestType::TOKEN:
            return std::make_unique<PlayerReplyData>(player, Token::FREE_FIELD);
        case RequestType::MESSAGE:
            return nullptr;
    }
    return nullptr;
}

void SFMLView::drawEnteredT() {
    if (model.hasRequest(RequestType::SUM)) {
        enteredT.setString(fromUtf8("Введите цену: " + enteredText));
    } else if (model.hasRequest(RequestType::NUM)) {
        enteredT.setString(fromUtf8("Выберите клетку поля щелчком мыши"));
    } else if (model.hasRequest(RequestType::TOKEN)) {
        enteredT.setString(fromUtf8("Выберите игрока для обмена"));
    } else if(isEnteringIP) {
        enteredT.setString(fromUtf8("Введите IP-адрес для подключения: " + enteredText));
    } else {
        enteredT.setString(sf::String());
    }
    auto[nx, ny, nw, nh] = enteredT.getLocalBounds();
    auto[x, y] = shapes.fieldRects[35].getPosition();

    enteredT.setOrigin(nx + nw / 2, 0);
    const float maxTextY = static_cast<float>(window.getSize().y) - nh - 10;
    enteredT.setPosition(x, std::min(y + shapes.fieldRects[35].getSize().y * 2, maxTextY));
    window.draw(enteredT);
}


void SFMLView::onResize(sf::Event::SizeEvent e) {
    auto[m, M] = e;
    if (m > M) std::swap(m, M);

    window.setView(sf::View(sf::FloatRect(0, 0, e.width, e.height)));
    const size_t side = (200 * m) / 2400;
    const size_t longSide = (250 * m) / 2400;
    const auto ss = static_cast<float>(side);
    const auto ls = static_cast<float>(longSide);
    const size_t longSideDiff = longSide - side;
    const size_t rowLen = 10;
    const size_t totalSide = side * rowLen + longSideDiff;
    const size_t totalLen = totalSide + longSide;
    const size_t left = (e.width - totalLen) / 2;
    const size_t bot = e.height - (e.height - totalLen) / 2 - longSide - longSide / 5;
    const size_t right = left + totalSide;
    const size_t top = bot - totalSide;

    for (size_t i = 0; i < Board::FIELD_SIZE; i++) {
        auto& tile = shapes.fieldRects[i];
        tile.setFillColor(sf::Color::Transparent);
        tile.setOutlineColor(sf::Color::White);
        tile.setOutlineThickness(1);

        const size_t k = i % rowLen;
        if (k == 0) {
            tile.setSize({ls, ls});
            switch (i / rowLen) {
                case 0:
                    tile.setPosition(left, bot);
                    break;
                case 1:
                    tile.setPosition(left, top);
                    break;
                case 2:
                    tile.setPosition(right, top);
                    break;
                case 3:
                    tile.setPosition(right, bot);
                    break;
            }
        } else {
            const auto shift = static_cast<float>(side * k);
            switch (i / rowLen) {
                case 0:
                    tile.setPosition(left, bot - shift);
                    break;
                case 1:
                    tile.setPosition(left + shift + longSideDiff, top);
                    break;
                case 2:
                    tile.setPosition(right, top + shift + longSideDiff);
                    break;
                case 3:
                    tile.setPosition(right - shift, bot);
                    break;
            }
            if ((i / rowLen) % 2 == 0) {
                tile.setSize({ls, ss});
            } else {
                tile.setSize({ss, ls});
            }
        }
        auto[x, y] = shapes.fieldRects[i].getPosition();
        auto[w, h] = shapes.fieldRects[i].getSize();
        fieldButtons.fieldButtons[i].setSize(x, y, w, h);
    }

    const auto& baseRect = shapes.fieldRects[0];
    switch (curMessageType) {
        case MessageType::INFO:
            message.setPosition(baseRect.getPosition() + baseRect.getPoint(1) +
                                sf::Vector2f(5, -message.getLocalBounds().height * 2));
            break;
        case MessageType::DICE: {
            auto[W, H] = window.getSize();
            message.setPosition(sf::Vector2f(float(W) / 2, float(H) / 2));
        }
            break;
        case MessageType::CHANCE: {
            auto[W, H] = window.getSize();
            message.setPosition(sf::Vector2f(float(W) / 2, float(H) / 2));
        }
            break;
        case MessageType::PUBLIC_TREASURY: {
            auto[W, H] = window.getSize();
            message.setPosition(sf::Vector2f(float(W) / 2, float(H) / 2));
        }
            break;
    }
    createMenuButtons();
}

void SFMLView::drawMoney(const BoardModel& board) {
    drawCenterDashboard(board);
}

void SFMLView::drawActionPanel() {
    struct ActionEntry {
        PlayerAction action;
        const char* group;
        const char* label;
    };
    static const ActionEntry actions[] = {
            {PlayerAction::ROLL_DICE, "ХОД", "Бросить кубики"},
            {PlayerAction::END_TURN, "ХОД", "Завершить ход"},
            {PlayerAction::BUY_FIELD, "НЕДВИЖИМОСТЬ", "Купить поле"},
            {PlayerAction::SELL_FIELD, "НЕДВИЖИМОСТЬ", "Продать поле"},
            {PlayerAction::BUY_HOUSE, "НЕДВИЖИМОСТЬ", "Построить дом"},
            {PlayerAction::SELL_HOUSE, "НЕДВИЖИМОСТЬ", "Продать дом"},
            {PlayerAction::BUY_HOTEL, "НЕДВИЖИМОСТЬ", "Построить гостиницу"},
            {PlayerAction::SELL_HOTEL, "НЕДВИЖИМОСТЬ", "Продать гостиницу"},
            {PlayerAction::MORTGAGE_HOLDINGS, "НЕДВИЖИМОСТЬ", "Заложить владение"},
            {PlayerAction::BUY_BACK_PROPERTY, "НЕДВИЖИМОСТЬ", "Выкупить из залога"},
            {PlayerAction::PAY_TAX, "ПЛАТЕЖИ", "Заплатить банку"},
            {PlayerAction::PAY_TO_OTHER_PLAYER, "ПЛАТЕЖИ", "Заплатить игроку"},
            {PlayerAction::START_TRADE, "ОБМЕН И КАРТЫ", "Предложить обмен"},
            {PlayerAction::START_TRADE_NEW_FIELD, "ОБМЕН И КАРТЫ", "Начать аукцион"},
            {PlayerAction::TAKE_CARD, "ОБМЕН И КАРТЫ", "Взять карточку"},
            {PlayerAction::USE_CARD, "ОБМЕН И КАРТЫ", "Использовать карточку"},
            {PlayerAction::EXIT_GAME, "ИГРА", "Выйти из игры"},
            {PlayerAction::FINISH_GAME, "ИГРА", "Завершить партию"}
    };

    const bool waitingForInput = model.hasRequest(RequestType::NUM) ||
                                 model.hasRequest(RequestType::SUM) ||
                                 model.hasRequest(RequestType::TOKEN) ||
                                 model.hasRequest(RequestType::TRADE);
    std::vector<const ActionEntry*> available;
    if (!waitingForInput) {
        for (const auto& entry : actions) {
            const auto button = actionButtons.find(entry.action);
            if (button != actionButtons.end() && button->second.isActive()) {
                available.push_back(&entry);
            }
        }
    }

    const auto& boardLeftTile = shapes.fieldRects[0];
    const float boardLeft = boardLeftTile.getPosition().x;
    const auto windowHeight = window.getSize().y;
    const float panelWidth = std::min(280.0f, std::max(190.0f, boardLeft - 20.0f));
    const float panelX = std::max(8.0f, boardLeft - panelWidth - 10.0f);
    const float panelY = 62.0f;
    const float panelBottom = static_cast<float>(windowHeight) - 14.0f;
    const float panelHeight = std::max(120.0f, panelBottom - panelY);
    const float padding = 12.0f;
    const float iconSize = std::min(30.0f, panelWidth * 0.15f);
    const float rowHeight = available.empty()
            ? 38.0f
            : std::clamp((panelHeight - 54.0f - 76.0f) / static_cast<float>(available.size()),
                         30.0f, 42.0f);

    sf::RectangleShape panel({panelWidth, panelHeight});
    panel.setPosition(panelX, panelY);
    panel.setFillColor(sf::Color(22, 27, 38, 238));
    panel.setOutlineColor(sf::Color(92, 102, 122));
    panel.setOutlineThickness(1);
    window.draw(panel);

    sf::Text heading(fromUtf8("ДЕЙСТВИЯ"), mainFont);
    heading.setCharacterSize(scaleUi(14));
    heading.setFillColor(sf::Color(220, 225, 235));
    heading.setPosition(panelX + padding, panelY + padding);
    window.draw(heading);

    tooltips.clear();
    for (const auto& [action, button] : actionButtons) {
        button.setVisible(button.isActive() && !waitingForInput);
    }

    float y = panelY + 40.0f;
    std::string currentGroup;
    if (available.empty()) {
        const char* waitingMessage = model.hasRequest(RequestType::NUM)
                ? "Выберите клетку на поле"
                : model.hasRequest(RequestType::SUM) ? "Введите сумму"
                : model.hasRequest(RequestType::TOKEN) ? "Выберите игрока"
                : model.hasRequest(RequestType::TRADE) ? "Ответьте на предложение"
                : "Ожидайте следующего действия";
        sf::Text waiting(fromUtf8(waitingMessage), mainFont);
        waiting.setCharacterSize(scaleUi(12));
        waiting.setFillColor(sf::Color(180, 190, 205));
        waiting.setPosition(panelX + padding, y);
        window.draw(waiting);
        return;
    }

    for (const auto* entry : available) {
        if (currentGroup != entry->group) {
            currentGroup = entry->group;
            sf::Text groupTitle(fromUtf8(currentGroup), mainFont);
            groupTitle.setCharacterSize(scaleUi(10));
            groupTitle.setFillColor(sf::Color(155, 175, 205));
            groupTitle.setPosition(panelX + padding, y);
            window.draw(groupTitle);
            y += scaleUi(17);
        }

        auto& button = actionButtons.at(entry->action);
        const sf::Vector2f currentSize = button.getSize();
        const sf::Vector2f currentScale = button.getScale();
        button.setScale({currentScale.x * iconSize / std::max(currentSize.x, 1.0f),
                         currentScale.y * iconSize / std::max(currentSize.y, 1.0f)});
        button.setPosition(panelX + padding, y + (rowHeight - iconSize) / 2);
        button.setHitbox({panelX + padding, y, panelWidth - padding * 2, rowHeight});

        sf::RectangleShape row({panelWidth - padding * 2, rowHeight - 2});
        row.setPosition(panelX + padding, y);
        row.setFillColor(entry->action == PlayerAction::ROLL_DICE
                         ? sf::Color(46, 83, 112)
                         : sf::Color(34, 41, 54));
        row.setOutlineColor(entry->action == PlayerAction::ROLL_DICE
                            ? sf::Color(100, 175, 230)
                            : sf::Color(62, 72, 88));
        row.setOutlineThickness(1);
        window.draw(row);

        sf::Text label(fromUtf8(entry->label), mainFont);
        label.setCharacterSize(scaleUi(12));
        const float labelWidth = panelWidth - padding * 3 - iconSize;
        while (label.getLocalBounds().width > labelWidth && label.getCharacterSize() > scaleUi(9)) {
            label.setCharacterSize(label.getCharacterSize() - 1);
        }
        label.setPosition(panelX + padding + iconSize + 8,
                          y + (rowHeight - label.getLocalBounds().height) / 2 -
                          label.getLocalBounds().top);
        label.setFillColor(sf::Color::White);
        window.draw(label);

        const auto tooltip = actionTooltips.find(entry->action);
        if (tooltip != actionTooltips.end()) {
            tooltips.addTooltip({tooltip->second, panelX + padding, y,
                                 panelWidth - padding * 2, rowHeight});
        }
        y += rowHeight;
    }
}

void SFMLView::drawCenterDashboard(const BoardModel& board) {
    const auto& bottomLeft = shapes.fieldRects[0];
    const auto& topRight = shapes.fieldRects[20];
    const auto[boardLeft, boardBottom] = bottomLeft.getPosition();
    const auto[cornerWidth, cornerHeight] = bottomLeft.getSize();
    const auto[boardRightX, boardTop] = topRight.getPosition();
    const float boardRight = boardRightX + topRight.getSize().x;
    const float centerX = (boardLeft + boardRight) / 2.0f;
    const float centerY = (boardTop + boardBottom + cornerHeight) / 2.0f;
    const float panelWidth = std::min(boardRight - boardLeft - cornerWidth * 2.4f, 520.0f);
    const float panelHeight = std::min(bottomLeft.getPosition().y - boardTop - cornerHeight * 1.8f, 330.0f);
    if (panelWidth <= 0 || panelHeight <= 0) return;

    sf::RectangleShape panel({panelWidth, panelHeight});
    panel.setOrigin(panelWidth / 2, panelHeight / 2);
    panel.setPosition(centerX, centerY);
    panel.setFillColor(sf::Color(22, 27, 38, 238));
    panel.setOutlineColor(sf::Color(92, 102, 122));
    panel.setOutlineThickness(2);
    window.draw(panel);

    const float left = centerX - panelWidth / 2 + 16;
    const float top = centerY - panelHeight / 2 + 12;
    const auto activePlayer = std::find_if(board.players.begin(), board.players.end(),
                                           [&board](const PlayerModel& player) {
                                               return player.token == board.curPlayer;
                                           });
    const auto fitText = [](sf::Text& text, float width, unsigned int minSize) {
        while (text.getLocalBounds().width > width && text.getCharacterSize() > minSize) {
            text.setCharacterSize(text.getCharacterSize() - 1);
        }
    };

    sf::Text title(fromUtf8("ЦЕНТР ПАРТИИ"), mainFont);
    title.setCharacterSize(scaleUi(14));
    title.setFillColor(sf::Color(175, 190, 215));
    title.setPosition(left, top);
    window.draw(title);

    if (activePlayer != board.players.end()) {
        sf::CircleShape badge(scaleUi(11));
        badge.setPosition(left, top + scaleUi(25));
        badge.setFillColor(getColor(activePlayer->token));
        badge.setOutlineColor(sf::Color::White);
        badge.setOutlineThickness(2);
        window.draw(badge);

        sf::Text activeName(fromUtf8(activePlayer->name), mainFont);
        activeName.setCharacterSize(scaleUi(20));
        fitText(activeName, panelWidth - scaleUi(80), scaleUi(12));
        activeName.setPosition(left + scaleUi(32), top + scaleUi(20));
        window.draw(activeName);

        std::size_t propertyCount = 0;
        for (const auto& field : board.field) {
            if (field.owner == activePlayer->token) ++propertyCount;
        }
        std::string turnStatus;
        if (!activePlayer->alive) {
            turnStatus = "Банкрот";
        } else if (activePlayer->prisoner) {
            turnStatus = "В Доске Позора";
        } else if (model.hasRequest(RequestType::SUM)) {
            turnStatus = "Введите сумму платежа";
        } else if (model.hasRequest(RequestType::NUM)) {
            turnStatus = "Выберите клетку на поле";
        } else if (model.hasRequest(RequestType::TOKEN)) {
            turnStatus = "Выберите игрока";
        } else if (model.hasRequest(RequestType::TRADE)) {
            turnStatus = "Ожидает решения по сделке";
        } else if (actionButtons.at(PlayerAction::ROLL_DICE).isActive()) {
            turnStatus = "Ваш ход — бросьте кубики";
        } else if (actionButtons.at(PlayerAction::END_TURN).isActive()) {
            turnStatus = "Завершите ход, когда будете готовы";
        } else {
            turnStatus = "Ожидание следующего действия";
        }

        sf::Text summary(fromUtf8("Баланс: " + std::to_string(activePlayer->money) +
                                  " М   •   Владений: " + std::to_string(propertyCount)), mainFont);
        summary.setCharacterSize(scaleUi(14));
        summary.setFillColor(sf::Color(220, 225, 235));
        summary.setPosition(left + scaleUi(32), top + scaleUi(48));
        window.draw(summary);

        const auto position = std::min(activePlayer->position, Board::FIELD_SIZE - 1);
        sf::Text status(fromUtf8(turnStatus + "   •   " + board.field[position].name), mainFont);
        status.setCharacterSize(scaleUi(13));
        fitText(status, panelWidth - 32, scaleUi(10));
        status.setFillColor(sf::Color(255, 215, 125));
        status.setPosition(left, top + scaleUi(78));
        window.draw(status);
    }

    const float cardsTop = top + scaleUi(108);
    const float cardGap = 8;
    const float cardWidth = (panelWidth - 32 - cardGap) / 2;
    const float cardHeight = scaleUi(30);
    for (std::size_t i = 0; i < board.players.size(); ++i) {
        const auto& player = board.players[i];
        const std::size_t row = i / 2;
        const std::size_t column = i % 2;
        const float x = left + static_cast<float>(column) * (cardWidth + cardGap);
        const float y = cardsTop + static_cast<float>(row) * (cardHeight + cardGap);

        sf::RectangleShape playerCard({cardWidth, cardHeight});
        playerCard.setPosition(x, y);
        playerCard.setFillColor(player.token == board.curPlayer
                                ? sf::Color(53, 62, 79)
                                : sf::Color(35, 40, 51));
        playerCard.setOutlineColor(player.token == board.curPlayer
                                   ? getColor(player.token)
                                   : sf::Color(70, 77, 91));
        playerCard.setOutlineThickness(player.token == board.curPlayer ? 2 : 1);
        window.draw(playerCard);

        sf::CircleShape playerBadge(scaleUi(7));
        playerBadge.setPosition(x + 8, y + (cardHeight - scaleUi(14)) / 2);
        playerBadge.setFillColor(getColor(player.token));
        window.draw(playerBadge);

        std::size_t propertyCount = 0;
        for (const auto& field : board.field) {
            if (field.owner == player.token) ++propertyCount;
        }
        const std::string playerStatus = !player.alive ? " • банкрот"
                                         : player.prisoner ? " • тюрьма" : "";
        sf::Text cardText(fromUtf8(player.name + playerStatus), mainFont);
        cardText.setCharacterSize(scaleUi(12));
        fitText(cardText, cardWidth - scaleUi(32), scaleUi(9));
        cardText.setPosition(x + scaleUi(25), y + 3);
        window.draw(cardText);

        sf::Text details(fromUtf8(std::to_string(player.money) + " М  •  владений: " +
                                  std::to_string(propertyCount)), mainFont);
        details.setCharacterSize(scaleUi(11));
        fitText(details, cardWidth - scaleUi(32), scaleUi(9));
        details.setFillColor(sf::Color(195, 205, 220));
        details.setPosition(x + scaleUi(25), y + cardHeight / 2);
        window.draw(details);
    }

    if (!recentEvents.empty()) {
        const float eventY = centerY + panelHeight / 2 - scaleUi(28);
        sf::Text eventLabel(fromUtf8("ПОСЛЕДНЕЕ СОБЫТИЕ"), mainFont);
        eventLabel.setCharacterSize(scaleUi(11));
        eventLabel.setFillColor(sf::Color(175, 190, 215));
        eventLabel.setPosition(left, eventY);
        window.draw(eventLabel);

        sf::Text eventText(fromUtf8(recentEvents.back()), mainFont);
        eventText.setCharacterSize(scaleUi(12));
        fitText(eventText, panelWidth - scaleUi(165), scaleUi(9));
        eventText.setFillColor(sf::Color::White);
        eventText.setPosition(left + scaleUi(130), eventY);
        window.draw(eventText);
    }
}

void SFMLView::drawTurnIndicator(const BoardModel& board) {
    const auto player = std::find_if(board.players.begin(), board.players.end(),
                                     [&board](const PlayerModel& item) {
                                         return item.token == board.curPlayer && item.alive;
                                     });
    if (player == board.players.end()) return;

    auto[windowWidth, windowHeight] = window.getSize();
    sf::Text label(fromUtf8("ХОД ИГРОКА: " + player->name), mainFont);
    label.setCharacterSize(scaleUi(20));
    const auto bounds = label.getLocalBounds();
    const float padding = static_cast<float>(scaleUi(16));
    const float badgeRadius = static_cast<float>(scaleUi(12));
    const float bannerWidth = bounds.width + padding * 2 + badgeRadius * 2 + 12;
    const float bannerHeight = std::max(bounds.height + padding, badgeRadius * 2 + padding);
    const float centerX = static_cast<float>(windowWidth) / 2;
    const float margin = static_cast<float>(scaleUi(12));

    sf::RectangleShape banner({bannerWidth, bannerHeight});
    banner.setOrigin(bannerWidth / 2, 0);
    banner.setPosition(centerX, margin);
    banner.setFillColor(sf::Color(25, 25, 25, 230));
    banner.setOutlineColor(getColor(player->token));
    banner.setOutlineThickness(3);
    window.draw(banner);

    sf::CircleShape tokenBadge(badgeRadius);
    tokenBadge.setOrigin(badgeRadius, badgeRadius);
    tokenBadge.setPosition(centerX - bannerWidth / 2 + padding + badgeRadius, margin + bannerHeight / 2);
    tokenBadge.setFillColor(getColor(player->token));
    tokenBadge.setOutlineColor(sf::Color::White);
    tokenBadge.setOutlineThickness(2);
    window.draw(tokenBadge);

    label.setPosition(centerX - bannerWidth / 2 + padding + badgeRadius * 2 + 12,
                      margin + (bannerHeight - bounds.height) / 2 - bounds.top);
    window.draw(label);
}

void SFMLView::drawToken(const BoardModel& board) {

}

void SFMLView::drawCardInfo(const BoardModel& board, std::optional<std::size_t> index) {
    cardActionHitTargets.clear();
    if (!index) {
        const auto activePlayer = std::find_if(board.players.begin(), board.players.end(),
                                               [&board](const PlayerModel& player) {
                                                   return player.token == board.curPlayer;
                                               });
        if (activePlayer != board.players.end()) {
            index = std::min(activePlayer->position, Board::FIELD_SIZE - 1);
        }
    }

    auto baseRect = shapes.fieldRects[0];
    auto[w, h] = baseRect.getSize();
    w *= 3, h *= 5;
    baseRect.setSize(sf::Vector2f(w, h));
    auto[wx, wy] = window.getSize();
    baseRect.setPosition(sf::Vector2f(float(wx), float(wy)) - sf::Vector2f(w + 1, h + 1));
    if (index.has_value()) {
        baseRect.setFillColor(sf::Color(22, 27, 38, 245));
        baseRect.setOutlineColor(sf::Color(220, 225, 235));
        baseRect.setOutlineThickness(2);
        const auto& fieldTile = board.field[index.value()];
        const sf::Color color = fieldTile.color.has_value()
                ? getColor(fieldTile.color.value())
                : sf::Color(100, 110, 125);
        window.draw(baseRect);

        const float size = h / 8;
        float detailsY = static_cast<float>(wy - h + size);
        sf::RectangleShape colorRect(sf::Vector2f(w, size));
        colorRect.setFillColor(color);
        colorRect.setPosition(sf::Vector2f(float(wx), float(wy)) - sf::Vector2f(w + 1, h + 1));
        window.draw(colorRect);

        std::string titleString = fieldTile.name;
        sf::Text title(fromUtf8(titleString), mainFont);
        title.setCharacterSize(scaleUi(18));
        while (title.getLocalBounds().width > w - 20 && title.getCharacterSize() > scaleUi(12)) {
            title.setCharacterSize(title.getCharacterSize() - 1);
        }
        auto[nx, ny, nw, nh] = title.getLocalBounds();
        auto[x, y] = baseRect.getPosition();
        title.setOrigin(nx + nw / 2, 0);
        title.setPosition(x + w / 2, y + size + 8);
        window.draw(title);
        detailsY = std::max(detailsY, y + size + nh + 20);

        if (fieldTile.cost.has_value()) {
            sf::Text price(fromUtf8("ЦЕНА  " + std::to_string(fieldTile.cost.value()) + " М"), mainFont);
            price.setCharacterSize(scaleUi(14));
            const float tagHeight = static_cast<float>(scaleUi(28));
            const float tagWidth = std::min(price.getLocalBounds().width + scaleUi(38), w - 24);
            const float tagX = x + (w - tagWidth) / 2;
            const float tagY = y + size + nh + 16;

            sf::ConvexShape priceTag(5);
            priceTag.setPoint(0, {0, 0});
            priceTag.setPoint(1, {tagWidth - tagHeight * 0.4f, 0});
            priceTag.setPoint(2, {tagWidth, tagHeight / 2});
            priceTag.setPoint(3, {tagWidth - tagHeight * 0.4f, tagHeight});
            priceTag.setPoint(4, {0, tagHeight});
            priceTag.setPosition(tagX, tagY);
            priceTag.setFillColor(sf::Color(235, 190, 85));
            priceTag.setOutlineColor(sf::Color(255, 225, 145));
            priceTag.setOutlineThickness(1);
            window.draw(priceTag);

            const float holeRadius = tagHeight * 0.09f;
            sf::CircleShape tagHole(holeRadius);
            tagHole.setOrigin(holeRadius, holeRadius);
            tagHole.setPosition(tagX + tagHeight * 0.22f, tagY + tagHeight / 2);
            tagHole.setFillColor(sf::Color(110, 82, 35));
            window.draw(tagHole);

            price.setFillColor(sf::Color(38, 40, 45));
            price.setPosition(tagX + tagHeight * 0.42f,
                              tagY + (tagHeight - price.getLocalBounds().height) / 2 -
                              price.getLocalBounds().top);
            window.draw(price);
            detailsY = std::max(detailsY, tagY + tagHeight + 10);
        }

        const bool hasOwner = fieldTile.owner.has_value() && fieldTile.owner != Token::FREE_FIELD;
        std::string cardText = hasOwner ? "Статус: В собственности\n" : "Статус: Свободно\n";
        if (fieldTile.costOfParking.has_value()) {
            cardText += "Базовая аренда: " + std::to_string(fieldTile.costOfParking.value()) + " М\n";
        }
        if (hasOwner) {
            const auto owner = std::find_if(board.players.begin(), board.players.end(),
                                            [&fieldTile](const PlayerModel& player) {
                                                return player.token == fieldTile.owner;
                                            });
            cardText += "Владелец: " + (owner == board.players.end() ? "неизвестен" : owner->name) + "\n";
        }
        cardText += fieldTile.isMortgaged ? "Залог: Да\n" : "Залог: Нет\n";
        if (fieldTile.numberOfHouses.has_value()) {
            cardText += "Дома: " + std::to_string(fieldTile.numberOfHouses.value()) +
                        "   Гостиницы: " + std::to_string(fieldTile.numberOfHotels.value()) + "\n";
        }
        if (fieldTile.costPerHouse.has_value()) {
            cardText += "Стоимость дома: " + std::to_string(fieldTile.costPerHouse.value()) + " М\n";
        }
        if (fieldTile.costPerHotel.has_value()) {
            cardText += "Стоимость гостиницы: " + std::to_string(fieldTile.costPerHotel.value()) + " М\n";
        }
        if (fieldTile.taxes.has_value()) {
            const auto& taxes = fieldTile.taxes.value();
            cardText += "Аренда с постройками: " + std::to_string(taxes.startTax) + " / " +
                        std::to_string(taxes.taxOneHouse) + " / " + std::to_string(taxes.taxTwoHouses) +
                        " / " + std::to_string(taxes.taxThreeHouses) + " / " +
                        std::to_string(taxes.taxFourHouses) + " / " + std::to_string(taxes.taxHotel) + " М\n";
        }

        sf::Text text(fromUtf8(cardText), mainFont);
        text.setCharacterSize(scaleUi(13));
        text.setPosition(x + 12, detailsY);
        window.draw(text);

        if (fieldTile.cost.has_value()) {
            const auto activePlayer = std::find_if(board.players.begin(), board.players.end(),
                                                   [&board](const PlayerModel& player) {
                                                       return player.token == board.curPlayer;
                                                   });
            const bool isOwnProperty = activePlayer != board.players.end() &&
                                       fieldTile.owner == activePlayer->token;
            const bool isUnownedLanding = activePlayer != board.players.end() &&
                                          fieldTile.owner == Token::FREE_FIELD &&
                                          activePlayer->position == index.value();

            struct PropertyAction {
                PlayerAction action;
                const char* label;
                bool applies;
            };
            const std::size_t houses = fieldTile.numberOfHouses.value_or(0);
            const std::size_t hotels = fieldTile.numberOfHotels.value_or(0);
            const PropertyAction propertyActions[] = {
                    {PlayerAction::BUY_FIELD, "Купить", isUnownedLanding},
                    {PlayerAction::BUY_HOUSE, "Построить дом", isOwnProperty &&
                            fieldTile.color.has_value() && !fieldTile.isMortgaged &&
                            houses < 4 && hotels == 0},
                    {PlayerAction::BUY_HOTEL, "Построить гостиницу", isOwnProperty &&
                            fieldTile.color.has_value() && !fieldTile.isMortgaged &&
                            houses >= 4 && hotels == 0},
                    {PlayerAction::SELL_HOUSE, "Продать дом", isOwnProperty &&
                            houses > 0 && hotels == 0},
                    {PlayerAction::SELL_HOTEL, "Продать гостиницу", isOwnProperty &&
                            hotels > 0},
                    {PlayerAction::SELL_FIELD, "Продать владение", isOwnProperty &&
                            houses == 0 && hotels == 0 && !fieldTile.isMortgaged},
                    {PlayerAction::MORTGAGE_HOLDINGS, "Заложить", isOwnProperty &&
                            houses == 0 && hotels == 0 && !fieldTile.isMortgaged},
                    {PlayerAction::BUY_BACK_PROPERTY, "Выкупить из залога", isOwnProperty &&
                            fieldTile.isMortgaged}
            };

            const float actionsTop = std::max(detailsY + text.getLocalBounds().height + 8,
                                              y + h * 0.68f);
            const float actionsBottom = y + h - 10;
            const float actionGap = 6;
            const float actionHeight = scaleUi(24);
            const float actionWidth = (w - 24 - actionGap) / 2;
            sf::Text actionsHeading(fromUtf8("ДЕЙСТВИЯ С ПОЛЕМ"), mainFont);
            actionsHeading.setCharacterSize(scaleUi(10));
            actionsHeading.setFillColor(sf::Color(175, 190, 215));
            actionsHeading.setPosition(x + 12, actionsTop);
            window.draw(actionsHeading);

            float actionY = actionsTop + scaleUi(16);
            std::size_t visibleActionCount = 0;
            for (const auto& action : propertyActions) {
                if (!action.applies) continue;

                const auto availableButton = actionButtons.find(action.action);
                const bool inputPending = model.hasRequest(RequestType::NUM) ||
                                          model.hasRequest(RequestType::SUM) ||
                                          model.hasRequest(RequestType::TOKEN) ||
                                          model.hasRequest(RequestType::TRADE);
                const bool enabled = !inputPending && availableButton != actionButtons.end() &&
                                     availableButton->second.isActive();
                const std::size_t actionColumn = visibleActionCount % 2;
                const float actionX = x + 12 + actionColumn *
                                      (actionWidth + actionGap);
                if (actionY + actionHeight > actionsBottom) break;

                sf::RectangleShape actionButton({actionWidth, actionHeight});
                actionButton.setPosition(actionX, actionY);
                actionButton.setFillColor(enabled ? sf::Color(46, 83, 112) : sf::Color(42, 46, 55));
                actionButton.setOutlineColor(enabled ? sf::Color(100, 175, 230) : sf::Color(68, 74, 84));
                actionButton.setOutlineThickness(1);
                window.draw(actionButton);

                sf::Text actionLabel(fromUtf8(action.label), mainFont);
                actionLabel.setCharacterSize(scaleUi(10));
                while (actionLabel.getLocalBounds().width > actionWidth - 8 &&
                       actionLabel.getCharacterSize() > scaleUi(8)) {
                    actionLabel.setCharacterSize(actionLabel.getCharacterSize() - 1);
                }
                actionLabel.setPosition(actionX + 4,
                                        actionY + (actionHeight - actionLabel.getLocalBounds().height) / 2 -
                                        actionLabel.getLocalBounds().top);
                actionLabel.setFillColor(enabled ? sf::Color::White : sf::Color(145, 150, 160));
                window.draw(actionLabel);

                if (enabled) {
                    cardActionHitTargets.emplace_back(
                            sf::FloatRect(actionX, actionY, actionWidth, actionHeight), action.action);
                }
                ++visibleActionCount;
                if (visibleActionCount % 2 == 0) {
                    actionY += actionHeight + 4;
                }
            }

            if (isOwnProperty && cardActionHitTargets.empty()) {
                sf::Text hint(fromUtf8("Действия доступны в ваш ход"), mainFont);
                hint.setCharacterSize(scaleUi(10));
                hint.setFillColor(sf::Color(155, 165, 180));
                hint.setPosition(x + 12, actionY + scaleUi(16));
                window.draw(hint);
            }
        }
    }
}

bool SFMLView::handlePropertyActionClick(sf::Event::MouseButtonEvent event) {
    if (model.hasRequest(RequestType::NUM) || model.hasRequest(RequestType::SUM) ||
        model.hasRequest(RequestType::TOKEN) || model.hasRequest(RequestType::TRADE)) {
        return false;
    }
    for (const auto& [hitbox, action] : cardActionHitTargets) {
        if (!hitbox.contains(static_cast<float>(event.x), static_cast<float>(event.y))) continue;
        const auto button = actionButtons.find(action);
        if (button == actionButtons.end() || !button->second.isActive() || !curCardIndex) return true;

        if (action != PlayerAction::BUY_FIELD) {
            pendingPropertyField = curCardIndex;
        }
        model.sendReply(std::make_unique<PlayerReplyData>(curTurnBy, action));
        return true;
    }
    return false;
}

void SFMLView::handleSelectedPropertyRequest() {
    if (!pendingPropertyField || !model.hasRequest(RequestType::NUM)) return;
    if (auto request = model.getRequest(); request.has_value()) {
        assert(request.value()->type == RequestType::NUM);
        takeMessage(request.value()->message);
        model.sendReply(std::make_unique<PlayerReplyData>(request.value()->player, *pendingPropertyField));
        pendingPropertyField.reset();
    }
}

void SFMLView::handleActionRequest() {
    if (model.hasRequest(RequestType::ACTION)) {
        if (auto req = model.getRequest(); req.has_value()) {
            takeMessage(req.value()->message);
            assert(req.value()->type == RequestType::ACTION);
            for (auto&[act, btn] : actionButtons) {
                btn.deactivate();
            }
            for (auto action : req.value()->availableActions) {
                if (actionButtons.count(action)) {
                    actionButtons.at(action).activate();
                }
            }
        }
    }
}

void SFMLView::addActionButton(PlayerAction action,
                               const std::string& texture,
                               std::string tooltip) {
    auto btn = std::make_unique<SpriteButton>();
    btn->setPosition(10, 10);
    btn->setScale({1.25f, 1.25f});
    btn->setTexture(texture);
    btn->setVisible(false);
    actionButtons.emplace(action, *btn);
    actionTooltips.emplace(action, std::move(tooltip));
    buttons.addButton(std::move(btn), [this, action](auto e) {
        model.sendReply(std::make_unique<PlayerReplyData>(curTurnBy, action));
        return true;
    });
}

void SFMLView::createMenuButtons() {
    auto[w, h] = window.getSize();
    sf::Vector2f buttonSize = sf::Vector2f(float(w) * 5 / 12, float(h) / 8);
    auto grey = sf::Color(130, 130, 130);
    menuButtons[0] = ButtonText("НАЧАТЬ ИГРУ", buttonSize, 18, grey, sf::Color::White);
    menuButtons[1] = ButtonText("ДОБАВИТЬ ИГРОКА", buttonSize, 18, grey, sf::Color::White);
    menuButtons[2] = ButtonText("ОБ ИГРЕ", buttonSize, 18, grey, sf::Color::White);
    menuButtons[3] = ButtonText("ВЫХОД", buttonSize, 18, grey, sf::Color::White);
    menuButtons[4] = ButtonText("ПОДКЛЮЧИТЬСЯ", buttonSize, 18, grey, sf::Color::White);
}

void SFMLView::drawMenuButtons() {
    auto[w, h] = window.getSize();
    float shift = float(h) / NUM_OF_BUTTONS;
    for (auto& menuButton : menuButtons)
        menuButton.setFont(mainFont);
    float start = shift / 2;

    for(size_t i = 0; i < NUM_OF_BUTTONS; i++) {
        menuButtons[i].setPosition(sf::Vector2f(float(w / 2), start + shift * i));
    }

    for (auto& menuButton : menuButtons)
        menuButton.drawTo(window);
}

void SFMLView::drawTokenButtons() {
    auto[w, h] = window.getSize();
    float shift = h / 3;
    float start = shift / 4;
    float x1 = w / 6;
    float x2 = w * 5 / 6;
    tokenButtons[Token::DOG].setPosition(sf::Vector2f(x1, start));
    tokenButtons[Token::DOG].setScale(sf::Vector2f(0.875, 0.875));
    tokenButtons[Token::BOOT].setPosition(sf::Vector2f(x1, start + shift));
    tokenButtons[Token::BOOT].setScale(sf::Vector2f(0.875, 0.875));
    tokenButtons[Token::CAR].setPosition(sf::Vector2f(x1, start + shift * 2));
    tokenButtons[Token::CAR].setScale(sf::Vector2f(0.875, 0.875));

    tokenButtons[Token::CAT].setPosition(sf::Vector2f(x2, start));
    tokenButtons[Token::CAT].setScale(sf::Vector2f(0.875, 0.875));
    tokenButtons[Token::HAT].setPosition(sf::Vector2f(x2, start + shift));
    tokenButtons[Token::HAT].setScale(sf::Vector2f(0.875, 0.875));
    tokenButtons[Token::SHIP].setPosition(sf::Vector2f(x2, start + shift * 2));
    tokenButtons[Token::SHIP].setScale(sf::Vector2f(0.875, 0.875));

    for (auto& t : tokenButtons) {
        t.second.drawTo(window);
    }
}

void SFMLView::drawTradeButtons() {
    auto[w, h] = window.getSize();
    sf::Vector2f buttonSize = sf::Vector2f(float(w) / 8, float(h) / 16);
    participateInTrade[0] = ButtonText("УЧАСТВОВАТЬ", buttonSize, 15, sf::Color::Green, sf::Color::White);
    participateInTrade[1] = ButtonText("ОТКАЗАТЬСЯ", buttonSize, 15, sf::Color::Red, sf::Color::White);

    for (auto& button : participateInTrade)
        button.setFont(mainFont);

    float y = shapes.fieldRects[0].getPosition().y + shapes.fieldRects[0].getSize().y * 2;
    participateInTrade[0].setPosition(sf::Vector2f(shapes.fieldRects[0].getPosition().x + buttonSize.x / 2, y));
    participateInTrade[1].setPosition(sf::Vector2f(
            shapes.fieldRects[30].getPosition().x + shapes.fieldRects[30].getSize().x - buttonSize.x / 2, y));

    for (auto& button : participateInTrade)
        button.drawTo(window);
}

void SFMLView::handleSumRequest() {
    if (model.hasRequest(RequestType::SUM) && lastKeyIsReturn) {
        lastKeyIsReturn = false;
        if (auto request = model.getRequest(); request) {
            assert(request.value()->type == RequestType::SUM);
            takeMessage(request.value()->message);
            model.sendReply(std::make_unique<PlayerReplyData>(request.value()->player, std::stoi(enteredText)));
            enteredText.clear();
        }
    }
}

void SFMLView::takeMessage(const RequestMessage& request) {
    if(!request.message.empty()) {
        message.setString(fromUtf8(request.message));
        curMessageType = request.type;
        if (request.type != MessageType::DICE) {
            recentEvents.push_back(request.message);
            if (recentEvents.size() > 3) recentEvents.erase(recentEvents.begin());
        }
        if (request.type == MessageType::DICE) {
            std::istringstream result(request.message);
            hasDiceResult = static_cast<bool>(result >> diceResult[0] >> diceResult[1]);
            if (hasDiceResult) diceAnimationClock.restart();
        }
    }
}
