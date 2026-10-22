#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

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
            m_fields->held = false;
        }
        return PlayerObject::releaseButton(btn);
    }

    void update(float dt) {
        PlayerObject::update(dt);
        if (!this->isMainPlayer()) return;

        if (m_fields->releaseNext) {
            m_fields->releaseNext = false;
            if (!m_fields->held) PlayerObject::releaseButton(PlayerButton::Jump);
        }

        if (!m_fields->pending) return;

        m_fields->sinceClick += dt;
        float windowSec = static_cast<float>(Mod::get()->getSettingValue<double>("window")) / 1000.f;

        if (m_fields->sinceClick > windowSec) {
            m_fields->pending = false;
            return;
        }

        if (m_isOnGround) {
            m_fields->pending = false;
            if (!m_fields->held) {
                PlayerObject::pushButton(PlayerButton::Jump);
                m_fields->releaseNext = true;
            }
        }
    }
};
