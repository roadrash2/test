#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

class $modify(BufferedPlayer, PlayerObject) {
    struct Fields {
        float sinceClick = 9999.f;
        bool pending = false;
        bool held = false;
        bool releaseNext = false;
    };

    bool isMainPlayer() {
        auto pl = PlayLayer::get();
        return pl && this == pl->m_player1;
    }

    bool pushButton(PlayerButton btn) {
        if (btn == PlayerButton::Jump && this->isMainPlayer()) {
            m_fields->held = true;
            if (Mod::get()->getSettingValue<bool>("enabled") && !m_isOnGround) {
                m_fields->pending = true;
                m_fields->sinceClick = 0.f;
            }
        }
        return PlayerObject::pushButton(btn);
    }

    bool releaseButton(PlayerButton btn) {
        if (btn == PlayerButton::Jump && this->isMainPlayer()) {
            m_fields->held =
