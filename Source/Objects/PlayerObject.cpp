#include "PlayerObject.h"

#include "Scenes/PlayLayer.h"
#include "Managers/AssetManager.h"
#include "State.h"

#include <2d/MotionStreak.h>
#include <2d/ActionEase.h>
#include <2d/SpriteFrameCache.h>
#include <2d/Layer.h>
#include <math/Vec2.h>
#include <base/Utils.h>


PlayerObject* PlayerObject::create(int iconID) {
    return ax::utils::createInstance<PlayerObject>(&PlayerObject::init, iconID);
}

void PlayerObject::update(float dt) {
    if (m_dead) {
        return;
    }

    m_previousPosition = getPosition();

    if (!m_locked) {
        float moddedDt = dt * 0.9f;
        updateJump(moddedDt);
        
        setPosition({
            (float)(getPositionX() + (m_velocity.x * moddedDt)),
            (float)(getPositionY() + (m_velocity.y * moddedDt))
        });
    }
}

void PlayerObject::updateJump(float dt)
{
    double var_38;
    double var_28;
    double var_30;

    if (m_flyMode)
    {
        // loc_14C620

        if (!m_buttonPushed)
        {
            // loc_14C6C8
            if (playerIsFalling() == false)
            {
                // loc_14C6D2
                var_38 = 1.2000000476837158;
            }
            else
            {
                // loc_14C822
                var_38 = 0.800000011920929;
            }
        }
        else
        {
            var_38 = -1.0;
        }

        if (m_buttonPushed != false && this->playerIsFalling() != false)
        {
            var_28 = 0.5;
        }
        else
        {
            // loc_14C634
            var_28 = 0.4000000059604645;
        }

        // loc_14C63C
        var_30         = this->m_velocity.y;
        double tmp     = static_cast<double>(dt) * this->m_gravity;
        double newYVel = var_30 - tmp * static_cast<double>(this->flipMod()) * var_38 * var_28;

        this->m_velocity.y = newYVel;

        // new

        if (m_gravityFlipped)
        {
            // loc_14C6FA
            if ((newYVel < -8.0) == false)  // note: check if -8.0 is correct
            {
                // loc_14C708
                if (newYVel > 6.400000095367432)
                    newYVel = 6.400000095367432;
            }
            else
            {
                // loc_14C806
                this->m_velocity.y = -8.0;
                newYVel           = this->m_velocity.y;
            }
        }
        else
        {
            if ((newYVel < -6.400000095367432) == false)
            {
                // loc_14C6B2
                if (newYVel > 8.0)
                    newYVel = 8.0;
            }
            else
            {
                // loc_14C814
                this->m_velocity.y = -6.400000095367432;
                newYVel           = this->m_velocity.y;
            }
        }

        // loc_14C71C
        m_velocity.y = newYVel;

        if (m_buttonPushed)
            m_onGround = false;

        return;
    }

    float f_v9;

    if (m_rollMode)
        f_v9 = 0.6;
    else
        f_v9 = 1.0;

    if (m_buttonPushed && m_canJump)
    {
        // loc_14C8B2
        m_onAir = true;
        m_onGround = false;
        m_canJump  = false;
        m_inputBuffered = false;

        this->m_velocity.y = m_jumpYStart * this->flipMod();
        m_unk20     = true;
        //this->m_unk26     = sub_14AE00();

        //if (m_bGotGameLayerFromGM_unk)
        {
            // INCOMPLETE. TODO: increment jumps
            //unk = true;
        }

        if (!m_rollMode)
            return runRotateAction();

        this->flipGravity(!this->m_gravityFlipped);
        this->m_velocity.y *= 0.6000000238418579;

        m_unk27             = false;
        this->m_buttonPushed = false;

        return;
    }

    if (m_onAir)
    {
        // loc_14C594
        this->m_velocity.y = m_velocity.y - (dt * m_gravity) * static_cast<double>(this->flipMod()) * f_v9;

        if (this->playerIsFalling())
        {
            m_onAir = false;
            m_unk19         = true;
            m_onGround = false;
        }

        return;
    }
    else
    {
        if (this->playerIsFalling())
            this->m_canJump = false;

        double new_y_velocity =
            this->m_velocity.y - (dt * m_gravity) * static_cast<double>(this->flipMod()) * f_v9;
        this->m_velocity.y = new_y_velocity;

        if (this->m_gravityFlipped && new_y_velocity > 15.0)
            new_y_velocity = 15.0;
        else if (new_y_velocity < -15.0)
            new_y_velocity = -15.0;

        this->m_velocity.y = new_y_velocity;

        if (this->playerIsFalling())
        {
            if (!this->m_rollMode && !this->getActionByTag(0))
                this->runRotateAction();

            bool on_ground = false;

            if (this->m_gravityFlipped)
            {
                on_ground = this->m_velocity.y > 4.0;
            }
            else
            {
                on_ground = this->m_velocity.y < -4.0;
            }

            if (on_ground)
                this->m_onGround = false;
        }

        return;
    }
}

void PlayerObject::hitGround(bool hitGround)
{
    //TODO: hitGround
    m_velocity.y = 0;
    m_onGround = true;
    m_canJump  = true;
    m_unk27           = true;

    m_lastGroundPos = this->getPosition();

    if (!m_rollMode && getActionByTag(0))
        stopRotation();
    else if (m_rollMode && !getActionByTag(0))
        runRotateAction();

    m_unk20 = false;
}

void PlayerObject::pushButton(PlayerButton btn)
{
    if (PlayerButton::Unk == btn && !m_locked)
    {
        m_buttonPushed = true;
        m_inputBuffered = true;

        if (m_rollMode)
        {
            if (m_touchedRing)
                return ringJump();
        }
        else
        {
            if (m_touchedRing)
                return ringJump();

            if (m_flyMode)
                return;
        }
        

        if (m_canJump)
            updateJump(0);
    }
}

void PlayerObject::releaseButton(PlayerButton btn)
{
    if (btn == PlayerButton::Unk && m_buttonPushed)
    {
        m_buttonPushed = false;
        m_inputBuffered = false;
    }
}

void PlayerObject::collidedWithObject(float dt, GameObject* object)
{
    ax::Rect playerRect   = getObjectRect();
    ax::Rect collidedRect = object->getObjectRect();

    double margin = (!m_flyMode) ? static_cast<float>(flipMod()) * 10.0f : static_cast<float>(flipMod()) * 6.0f;

    float currentYPos  = this->getPositionY();
    float previousYPos = m_previousPosition.y;

    double point1     = (currentYPos + ((playerRect.size.height * -0.5) * (float)flipMod())) + margin;
    double point1Prev = (previousYPos + ((playerRect.size.height * -0.5) * (float)flipMod())) + margin;
    double point2     = (currentYPos + ((playerRect.size.height * -0.5) * (float)-flipMod())) - margin;
    double point2Prev = (previousYPos + ((playerRect.size.height * -0.5) * (float)-flipMod())) - margin;

    float maxY = collidedRect.getMaxY();
    float minY = collidedRect.getMinY();

    if (m_gravityFlipped && !m_flyMode)
    {
        if (point1 <= minY || point1Prev <= minY)
        {
            if (m_velocity.y > 0)
            {
                setPositionY((playerRect.size.height * -0.5) + minY);
                // TODO: PlayerObject::checkSnapJumpToObject(this, a3);
                hitGround((!m_gravityFlipped) ? m_flyMode : false);
                // TODO: PlayerObject::touchedObject(this, a3);
            }

            return;
        }

        goto check_intersection;
    }

    {
        double comparePoint1 = (m_gravityFlipped) ? point2 : point1;
        double comparePoint2 = (m_gravityFlipped) ? point2Prev : point1Prev;

        if (comparePoint1 >= maxY || comparePoint2 >= maxY)
        {
            if (m_velocity.y < 0.0)
            {
                setPositionY((playerRect.size.height * 0.5) + maxY);
                // PlayerObject::checkSnapJumpToObject(this, a3);
                this->hitGround((m_gravityFlipped) ? m_flyMode : false);
                // PlayerObject::touchedObject(this, a3);

                return;
            }
        }
    }

    if (!m_gravityFlipped && !m_flyMode)
        goto check_intersection;

    if (!m_gravityFlipped)
    {
        point1Prev = point2Prev;
        point1     = point2;
    }

    if (point1 <= minY || point1Prev <= minY)
    {
        if (m_velocity.y > 0)
        {
            setPositionY((playerRect.size.height * -0.5) + minY);
            // TODO: PlayerObject::checkSnapJumpToObject(this, a3);
            hitGround((!m_gravityFlipped) ? m_flyMode : false);
            // TODO: PlayerObject::touchedObject(this, a3);
        }

        return;
    }
    
check_intersection:
    if (getObjectRect({0.3, 0.3}).intersectsRect(collidedRect)) {
        State::getInstance()->getPlayLayer()->destroyPlayer();
    };

    return;
}

void PlayerObject::flipGravity(bool flip) {
    if (m_gravityFlipped == flip) {
        return;
    }

    m_gravityFlipped = flip;
    m_velocity.y *= 0.5;
    m_canJump = false;

    setScaleY((!m_flyMode) ? 1 : (m_gravityFlipped) ? -1 : 1);

    if (m_rollMode) {
        stopRotation();
        runBallRotation2();
    }
}

void PlayerObject::toggleFlyMode(bool toggle) {
    if (m_flyMode == toggle) {
        return;
    }
    
    m_flyMode = toggle;

    stopRotation();

    m_velocity.y *= 0.5;
    setRotation(0);
    m_onGround = false;
    m_canJump  = false;
    m_unk20    = false;

    if (toggle)
    {
        m_ship->setVisible(true);
        m_ship->setPositionY(-5);
        m_cubeParts[0]->setScale(0.55);
        m_cubeParts[0]->setPositionY(5);

        if (m_gravityFlipped)
            return this->setScaleY(-1);
    }
    else
    {
        runRotateAction();
        m_cubeParts[0]->setScale(1.0);
        m_cubeParts[0]->setPosition(ax::Vec2::ZERO);
        m_ship->setVisible(false);
        return this->setScaleY(1.0);
    }
}

void PlayerObject::propellPlayer(float force)
{
    m_onAir = true;
    m_onGround = false;
    m_canJump  = false;
    m_unk20      = true;

    m_velocity.y = (force * 16) * flipMod();

    if (this->m_rollMode)
        this->m_velocity.y = m_velocity.y * 0.600000024;

    runRotateAction();
}

void PlayerObject::ringJump()
{
    //AXLOGD("inputBuffered {}", _inputBuffered);

    if (m_touchedRing && m_inputBuffered && m_buttonPushed)
    {
        m_onAir         = true;
        m_onGround      = false;
        m_canJump       = false;
        m_inputBuffered = false;

        double jump_y = this->m_jumpYStart;

        if (this->m_touchedRing->getType() == GameObjectType::BlueOrb)
            jump_y *= 0.8;

        this->m_velocity.y = static_cast<double>(this->flipMod()) * jump_y;

        if (this->m_rollMode)
            this->runBallRotation2();
        else
            this->runRotateAction();

        m_lastPortalPos = m_touchedRing->getPosition(); // ???

        // INCOMPLETE. TODO: ORB CIRCLE EFFECT

        m_lastGroundPos = this->getPosition();
        //this->activateStreak();
        m_hasRingJumped = true;

        if (this->m_rollMode)
        {
            this->m_velocity.y *= 0.699999988079071;
        }

        if (this->m_touchedRing->getType() == GameObjectType::BlueOrb)
        {
            this->flipGravity(!this->m_gravityFlipped);
            //GameManager::sharedState()->getPlayLayer()->playGravityEffect(this->m_gravityFlipped);
        }

        m_touchedRing->triggerActivated();
        //this->m_touchedRing_454->powerOffObject();
        this->m_touchedRing = NULL;
    }
}

void PlayerObject::toggleRollMode(bool toggle)
{
    if (m_rollMode == toggle)
        return;

    m_rollMode = toggle;
    toggleFlyMode(false);

    if (toggle)
    {
        m_cubeParts[1]->setSpriteFrame(ax::SpriteFrameCache::getInstance()->getSpriteFrameByName("player_ball_01_2_001.png"));
        m_cubeParts[0]->setSpriteFrame(ax::SpriteFrameCache::getInstance()->getSpriteFrameByName("player_ball_01_001.png"));
        m_cubeParts[1]->setPosition(m_cubeParts[0]->getContentSize() / 2);
    }
    else
    {
        //TODO: Revert to cube
        updatePlayerFrame(1);
    }

    stopRotation();
}

void PlayerObject::updatePlayerFrame(int iconID)
{
    auto firstFrame  = fmt::format("player_{:#02}_001.png", iconID);
    auto secondFrame = fmt::format("player_{:#02}_2_001.png", iconID);

    auto sfc = ax::SpriteFrameCache::getInstance();

    m_cubeParts[0]->setSpriteFrame(sfc->getSpriteFrameByName(firstFrame));
    m_cubeParts[1]->setSpriteFrame(sfc->getSpriteFrameByName(secondFrame));

    m_cubeParts[1]->setPosition(m_cubeParts[0]->getContentSize() / 2);
}


float SquareDistance(float a1, float a2, float a3, float a4)
{
    return ((a3 - a1) * (a3 - a1)) + ((a4 - a2) * (a4 - a2));
}

float Slerp2D(float a1, float a2, float a3)
{
    double half_a1 = a1 * 0.5;

    class Spinor
    {
    public:
        double p[2];

        void slerp(Spinor const& a2, Spinor const& a3, float angle)
        {
            float v5  = a3.p[0];
            float v10 = a3.p[1];

            float v11 = (a2.p[0] * a3.p[0]) + (a2.p[1] * a3.p[1]);
            if (v11 < 0.0)
            {
                v11 *= -1;
                v10 *= -1;
                v5 *= -1;
            }

            float v12{};

            if ((1.0 - v11) > 0.0001)
            {
                double v14 = acos(v11);
                float v16  = sinf(v14);
                v12        = sinf((1.0 - angle) * v14) / v16;
                angle      = sinf(angle * v14) / v16;
            }
            else
            {
                v12 = 1.0 - angle;
            }

            p[0] = (v12 * a2.p[0]) + (angle * v5);
            p[1] = (v12 * a2.p[1]) + (angle * v10);
        }
    };

    Spinor v17{};
    v17.p[0] = cos(half_a1);
    v17.p[1] = sin(half_a1);

    double half_a2 = a2 * 0.5;

    Spinor v16{};
    v16.p[0] = cos(half_a2);
    v16.p[1] = sin(half_a2);

    Spinor v15{};
    v15.slerp(v17, v16, a3);
    double v13 = atan2(v15.p[1], v15.p[0]);

    return v13 + v13;
}

void PlayerObject::updateShipRotation(float dt)
{
    auto c_pos = getPosition();
    float v5      = c_pos.x - m_previousPosition.x;
    float v6      = (c_pos.y - m_previousPosition.y) * -1;

    if (SquareDistance(0.0, 0.0, v5, v6) >= 1.2)
    {
        float rad = getRotation() * 0.017453;
        float v10 = dt * 0.15;
        if ((dt * 0.15) >= 1.0)
            float v10 = 1.0;
        float new_rot = Slerp2D(rad, atan2(v6, v5), v10) * 57.296;
        setRotation(new_rot);
    }
}

void PlayerObject::setPosition(const ax::Vec2& pos)
{
    GameObject::setPosition(pos);

    if (m_motionStreak) {
        m_motionStreak->setPosition(getPosition() + ax::Vec2{-5, 0});
    }
}

void PlayerObject::setColor(const ax::Color3B& color)
{
    Sprite::setColor(color);
    m_cubeParts[0]->setColor(color);
    m_ship->setColor({(uint8_t)((int)color.r * 0.8), (uint8_t)((int)color.g * 0.8), (uint8_t)((int)color.b * 0.8)});
}

void PlayerObject::setSecondColor(const ax::Color3B& color)
{
    m_cubeParts[1]->setColor(color);
    m_motionStreak->setColor(color);
}

void PlayerObject::playerDestroyed()
{
    m_dead = true;
    stopRotation();

    auto fade = ax::FadeTo::create(0.05, 0);
    fade->setTag(3);
    runAction(fade);

    auto particle = ax::ParticleSystemQuad::create("explodeEffect.plist");
    particle->setPosition(getPosition());
    particle->setPositionType(ax::ParticleSystem::PositionType::GROUPED);
    particle->setAutoRemoveOnFinish(true);
    particle->setStartColor(ax::Color4F(getColor()));
    m_gameLayer->addChild(particle, 3);
    particle->resetSystem();
}

void PlayerObject::setOpacity(uint8_t opacity)
{
    GameObject::setOpacity(opacity);
    m_cubeParts[0]->setOpacity(opacity);
    m_cubeParts[1]->setOpacity(opacity);
    m_ship->setOpacity(opacity);
}

void PlayerObject::resetObject()
{
    //TODO: this
    m_unk18 = true;
    m_portalObject = nullptr;
    m_locked       = false;
    m_unk20        = false;


    setPosition(State::getInstance()->getPlayLayer()->getStartPos());
    
    
    m_velocity.y = 0;
    flipGravity(false);
    toggleFlyMode(false);
    toggleRollMode(false);
    stopRotation();
    setRotation(0);
    m_dead  = false;
    stopActionByTag(3);
    setOpacity(255);

    

    m_unk18 = false;
}

bool PlayerObject::init(int iconID)
{
    iconID = std::min(std::max(iconID, 1), 0x1A);  // 0x1...0x1A
    auto firstFrame  = fmt::format("player_{:#02}_001.png", iconID);
    auto secondFrame = fmt::format("player_{:#02}_2_001.png", iconID);

    if (!GameObject::init(firstFrame))
        return false;

    //TODO: Add a way to use a layer that is provided via create
    auto playLayer = State::getInstance()->getPlayLayer();
    m_gameLayer = (playLayer) ? playLayer->getGamelayer() : nullptr;

    this->setTextureRect(ax::Rect::ZERO);

    m_cubeParts[0] = Sprite::createWithSpriteFrameName(firstFrame);
    this->addChild(m_cubeParts[0], 1);

    m_cubeParts[1] = Sprite::createWithSpriteFrameName(secondFrame);
    m_cubeParts[1]->setPosition(m_cubeParts[0]->convertToNodeSpace(ax::Vec2::ZERO));
    m_cubeParts[0]->addChild(m_cubeParts[1], -1);

    m_ship = Sprite::createWithSpriteFrameName("ship_01_001.png");
    this->addChild(m_ship, 2);
    m_ship->setVisible(false);

    m_velocity = {5.7700018882751465, 0};
    m_jumpYStart = 11.180031776428223;  // 0x40265C2D20000000
    m_gravity    = 0.9581990242004395;  // 0x3FEEA99100000000LL

    AssetManager* assetManager = AssetManager::getInstance();

    if (m_gameLayer) {
            m_motionStreak = ax::MotionStreak::create(
        0.3, 3.0, 10, ax::Color3B::WHITE, 
        "streak.png");
        m_motionStreak->setBlendFunc(ax::BlendFunc::ADDITIVE);


        m_gameLayer->addChild(m_motionStreak, 0);
    }

    return true;
}

bool PlayerObject::playerIsFalling()
{
    //TODO: Check if this is actually correct

    if (m_gravityFlipped)
    {
        return (m_velocity.y > (m_gravity + m_gravity));
    }
    else
    {
        return (m_velocity.y < (m_gravity + m_gravity));
    }
}

int PlayerObject::flipMod()
{
    return (m_gravityFlipped) ? -1 : 1;
}

void PlayerObject::stopRotation()
{
    this->stopActionByTag(0);
    this->stopActionByTag(1);

    if (getRotation() != 0 && !m_rollMode)
    {
        int modRot = (int)getRotation() % 360;
        float newRot = (flipMod() == 1) ? 90 * roundf((float)modRot / 90.0f) : -90 * roundf((float)modRot / -90.0f);
        this->setRotation(newRot);
    }
}

void PlayerObject::runRotateAction()
{
    if (!m_locked)
    {
        stopRotation();
        if (m_rollMode)
        {
            runBallRotation();
        }
        else
        {
            runNormalRotation();
        }
    }
}

void PlayerObject::runNormalRotation()
{
    if (!m_flyMode)
    {
        auto rotAct = ax::RotateBy::create(0.43333f, 180 * flipMod());
        rotAct->setTag(0);
        this->runAction(rotAct);
    }
}

void PlayerObject::runBallRotation()
{
    auto act = ax::RepeatForever::create(ax::RotateBy::create(0.2, 120 * flipMod()));
    act->setTag(0);
    this->runAction(act);
}

void PlayerObject::runBallRotation2()
{
    auto act = ax::EaseOut::create(ax::RotateBy::create(0.4, -200 * flipMod()), 1.6);
    act->setTag(1);
    runAction(act);
}
