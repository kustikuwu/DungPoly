#include "view/TooltipManager.h"

void TooltipManager::addTooltip(Tooltip tooltip) {
    tooltips.push_back(std::move(tooltip));
}

void TooltipManager::clear() {
    tooltips.clear();
}

std::string_view TooltipManager::getTooltip(sf::Event::MouseMoveEvent e) const {
    for(const auto& t : tooltips) {
        if(t.x <= e.x && e.x <= t.x + t.w &&
           t.y <= e.y && e.y <= t.y + t.h) {
            return t.tooltip;
        }
    }
    return "";
}

