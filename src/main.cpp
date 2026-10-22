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

class $modify(BufferPauseLayer, PauseLayer) {
    void onToggleBuffer(CCObject* sender) {
        auto toggler = static_cast<CCMenuItemToggler*>(sender);
        // the toggler reports its state from before the click
        Mod::get()->setSettingValue<bool>("enabled", !toggler->isToggled());
    }

    void customSetup() {
        PauseLayer::customSetup();

        auto toggler = CCMenuItemToggler::createWithStandardSprites(
            this, menu_selector(BufferPauseLayer::onToggleBuffer), 0.7f
        );
        toggler->toggle(Mod::get()->getSettingValue<bool>("enabled"));
        toggler->setPosition({30.f, 30.f});

        auto label = CCLabelBMFont::create("Input Buffer", "bigFont.fnt");
        label->setScale(0.4f);
        label->setAnchorPoint({0.f, 0.5f});
        label->setPosition({52.f, 30.f});

        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});
        menu->addChild(toggler);
        menu->addChild(label);
        this->addChild(menu);
    }
};
