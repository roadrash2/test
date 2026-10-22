#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

static void showBufferedIndicator() {
    if (!Mod::get()->getSettingValue<bool>("indicator")) return;

    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene) return;

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto label = CCLabelBMFont::create("CLICK ASSIST", "bigFont.fnt");
    label->setScale(0.5f);
    label->setColor({0, 255, 120});
    label->setPosition({winSize.width / 2.f, winSize.height - 40.f});
    label->runAction(CCSequence::create(
        CCDelayTime::create(0.15f),
        CCFadeOut::create(0.35f),
        CCRemoveSelf::create(),
        nullptr
    ));
    scene->addChild(label, 1000);
}

class $modify(AssistPlayer, PlayerObject) {
    struct Fields {
        float sinceClick = 9999.f;
        bool pending = false;     // an early click is waiting for its moment
        bool held = false;        // the real button state (what you are doing)
        bool simulated = false;   // the mod is holding the button down for you
        int releaseIn = 0;        // frames until the mod lets go after firing
    };

    // Player 1, and player 2 in dual mode
    bool isTrackedPlayer() {
        auto pl = PlayLayer::get();
        return pl && (this == pl->m_player1 || this == pl->m_player2);
    }

    // Cube, ball, robot and spider act on landing / orbs.
    // Ship, UFO, wave and swing are hold/tap modes with nothing to time, so skip them.
    bool needsGround() {
        return !(m_isShip || m_isBird || m_isDart || m_isSwing);
    }

    void stopAssist() {
        if (m_fields->simulated && !m_fields->held) {
            PlayerObject::releaseButton(PlayerButton::Jump);
        }
        m_fields->simulated = false;
        m_fields->pending = false;
    }

    bool pushButton(PlayerButton btn) {
        if (btn == PlayerButton::Jump && this->isTrackedPlayer()) {
            m_fields->held = true;
            if (Mod::get()->getSettingValue<bool>("enabled")
                && this->needsGround()
                && !m_isOnGround) {
                m_fields->pending = true;
                m_fields->sinceClick = 0.f;
            }
        }
        return PlayerObject::pushButton(btn);
    }

    bool releaseButton(PlayerButton btn) {
        if (btn == PlayerButton::Jump && this->isTrackedPlayer()) {
            m_fields->held = false;
            // while the mod is holding the click for you, keep it held
            if (m_fields->simulated) return true;
        }
        return PlayerObject::releaseButton(btn);
    }

    void update(float dt) {
        PlayerObject::update(dt);
        if (!this->isTrackedPlayer()) return;

        if (m_fields->releaseIn > 0) {
            m_fields->releaseIn--;
            if (m_fields->releaseIn == 0 && !m_fields->held) {
                PlayerObject::releaseButton(PlayerButton::Jump);
            }
        }

        if (!m_fields->pending) return;

        // mode changed (e.g. portal into ship) while a click was waiting
        if (!this->needsGround()) {
            this->stopAssist();
            return;
        }

        m_fields->sinceClick += dt;
        float windowSec = static_cast<float>(Mod::get()->getSettingValue<double>("window")) / 1000.f;

        // the click was too early, give up
        if (m_fields->sinceClick > windowSec) {
            this->stopAssist();
            return;
        }

        // landed inside the window: the held click makes the jump this frame
        if (m_isOnGround) {
            bool wasSimulated = m_fields->simulated;
            m_fields->pending = false;
            m_fields->simulated = false;
            if (wasSimulated) {
                m_fields->releaseIn = 3;
                log::info("Click assist fired");
                showBufferedIndicator();
            }
            return;
        }

        // still in the air: hold the click for you so the first valid
        // moment (landing, or touching an orb) uses it
        if (!m_fields->held && !m_fields->simulated) {
            PlayerObject::pushButton(PlayerButton::Jump);
            m_fields->simulated = true;
        }
    }
};

class $modify(AssistPauseLayer, PauseLayer) {
    void onToggleBuffer(CCObject* sender) {
        auto toggler = static_cast<CCMenuItemToggler*>(sender);
        // the toggler reports its state from before the click
        Mod::get()->setSettingValue<bool>("enabled", !toggler->isToggled());
    }

    void customSetup() {
        PauseLayer::customSetup();

        auto toggler = CCMenuItemToggler::createWithStandardSprites(
            this, menu_selector(AssistPauseLayer::onToggleBuffer), 0.7f
        );
        toggler->toggle(Mod::get()->getSettingValue<bool>("enabled"));
        toggler->setPosition({30.f, 30.f});

        auto label = CCLabelBMFont::create("Click Assist", "bigFont.fnt");
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
