#pragma once

#include "GameObject.h"

namespace ax {
    class Sprite;
    class Vec2;
    class MotionStreak;
};

class PlayScene;
class GameObject;

enum class PlayerButton : uint8_t {
    Unk = 1
};

class PlayerObject : public GameObject {
public:
    static PlayerObject* create(int iconID);
    void update(float dt) override;
    void updateJump(float dt);
    bool getIsLocked() const { return m_locked; }
    void hitGround(bool hitGround);
    bool getFlyMode() const { return m_flyMode; }
    bool getRollMode() const { return m_rollMode; }
    bool getGravityFlipped() const { return m_gravityFlipped; }
    void pushButton(PlayerButton);
    void releaseButton(PlayerButton);
    void collidedWithObject(float, GameObject*);
    ax::Vec2 getRealPosition() const override { return this->getPosition(); }
    void flipGravity(bool);
    void setPortalP(ax::Vec2 p) { m_lastPortalPos = p; }
    void setPortalObject(GameObject* o) { m_portalObject = o; }
    void toggleFlyMode(bool toggle);
    void propellPlayer(float);
    void setTouchedRing(GameObject* obj) { m_touchedRing = obj; }
    void ringJump();
    void toggleRollMode(bool toggle);
    ax::Vec2 getLastGroundPos() const { return m_lastGroundPos; }
    void updatePlayerFrame(int);
    void updateShipRotation(float);
    void setPosition(const ax::Vec2&) override;
    void setColor(const ax::Color3B& color) override;
    void setSecondColor(const ax::Color3B& color);
    void playerDestroyed();
    void setOpacity(uint8_t) override;
    void resetObject() override;
private:
    bool init(int iconID);
    bool playerIsFalling();
    int flipMod();
    void stopRotation();
    void runRotateAction();
    void runNormalRotation();
    void runBallRotation();
    void runBallRotation2();
private:
    bool m_buttonPushed;
    bool m_onAir; ///< Offset (1.3): 0x365
    bool m_onGround;
    bool m_gravityFlipped;
    bool m_dead;
    bool m_locked;
    bool m_canJump;
    bool m_inputBuffered;
    bool m_hasRingJumped;

    GameObject* m_touchedRing;
    GameObject* m_portalObject;
    ax::Vec2 m_lastPortalPos;
    ax::Vec2 m_lastGroundPos;

    bool m_flyMode;
    bool m_rollMode;

    double m_gravity;
    double m_jumpYStart;

    struct {
        double x;
        double y;
    } m_velocity;
    ax::Vec2 m_previousPosition;

    ax::Sprite* m_cubeParts[2];
    ax::Sprite* m_ship;

    bool m_unk27;
    bool m_unk19;
    bool m_unk20;
    bool m_unk18;

    ax::MotionStreak* m_motionStreak;
    ax::Layer* m_gameLayer;
};
