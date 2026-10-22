#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    void resetLevel() {
        PlayLayer::resetLevel();
        log::info("Level was reset!");
    }
};
