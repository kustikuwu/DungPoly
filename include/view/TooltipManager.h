#ifndef TOOLTIPMANAGER_H
#define TOOLTIPMANAGER_H

#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

struct Tooltip {
    std::string tooltip;
    float x, y, w, h;
};

class TooltipManager {
public:
    void addTooltip(Tooltip tooltip);
    void clear();
    std::string_view getTooltip(sf::Event::MouseMoveEvent e) const;
private:
    std::vector<Tooltip> tooltips;
};

#endif //TOOLTIPMANAGER_H