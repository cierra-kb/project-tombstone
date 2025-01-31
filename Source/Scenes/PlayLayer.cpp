#include "PlayLayer.h"

#include "Managers/AssetManager.h"
#include "Managers/GameManager.h"
#include "Objects/GroundLayer.h"
#include "Objects/PlayerObject.h"
#include "Objects/Level.h"
#include "Objects/LevelSettings.h"
#include "Objects/GameObject.h"
#include "Extensions/DirectorExt.h"
#include "Utils/SplitString.inl.h"
#include "State.h"

#include <base/EventDispatcher.h>
#include <base/EventListenerKeyboard.h>
#include <base/EventListenerTouch.h>
#include <2d/Camera.h>
#include <2d/SpriteBatchNode.h>
#include <2d/ActionInstant.h>
#include <2d/ActionEase.h>
#include <audio/AudioEngine.h>

#include <vector>

const char* getAudioFileName(int id) {
    const char* audioName = "";

    switch (id) {
        case 0:
            audioName = "StereoMadness.mp3";
            break;
        case 1:
            audioName = "BackOnTrack.mp3";
            break;
        case 2:
            audioName = "Polargeist.mp3";
            break;
        case 3:
            audioName = "DryOut.mp3";
            break;
        case 4:
            audioName = "BaseAfterBase.mp3";
            break;
        case 5:
            audioName = "CantLetGo.mp3";
            break;
        case 6:
            audioName = "Jumper.mp3";
            break;
        case 7:
            audioName = "TimeMachine.mp3";
            break;
        case 8:
            audioName = "Cycles.mp3";
            break;
        case 9:
            audioName = "xStep.mp3";
            break;
    }

    return audioName;
}

void PlayScene::onEnter() {
    ax::Scene::onEnter();
    ax::extension::Inspector::getInstance()->openForScene(this);
}

void PlayScene::onExit() {
    ax::extension::Inspector::getInstance()->close();
    ax::Scene::onExit();
}

void PlayScene::setupKeybinds() {
    ax::Director* director                      = ax::Director::getInstance();
    ax::EventListenerKeyboard* keyboardListener = ax::EventListenerKeyboard::create();

    static bool playerButtonHeld  = false;
    static bool unscheduled = false;

    keyboardListener->onKeyPressed = [this](ax::EventKeyboard::KeyCode key, ax::Event*) {
        switch (key) {
            case ax::EventKeyboard::KeyCode::KEY_ESCAPE:
                ax::Director::getInstance()->popScene();
                break;
            case ax::EventKeyboard::KeyCode::KEY_SPACE: {
                if (unscheduled) {
                    scheduleUpdate();
                    ax::AudioEngine::resumeAll();
                } else {
                    unscheduleUpdate();
                    ax::AudioEngine::pauseAll();
                }

                unscheduled ^= true;
                break;
            }
            case ax::EventKeyboard::KeyCode::KEY_UP_ARROW:
                if (playerButtonHeld) {
                    break;
                }

                playerButtonHeld = true;
                m_player->pushButton(PlayerButton::Unk);
                
                break;
            default:
                break;
        }
    };

    keyboardListener->onKeyReleased = [this](ax::EventKeyboard::KeyCode key, ax::Event*) {
        switch (key) {
            case ax::EventKeyboard::KeyCode::KEY_UP_ARROW:
                if (!playerButtonHeld) {
                    break;
                }

                playerButtonHeld = false;
                m_player->releaseButton(PlayerButton::Unk);
                break;
            default:
                break;
        }
    };

    director
        ->getEventDispatcher()
        ->addEventListenerWithSceneGraphPriority(keyboardListener, this);
}

void PlayScene::setupTouchControls() {
    ax::Director* director                        = ax::Director::getInstance();
    ax::EventListenerTouchOneByOne* touchListener = ax::EventListenerTouchOneByOne::create();

    touchListener->onTouchMoved = [this](ax::Touch* t, ax::Event*) {
        this->getDefaultCamera()->setPosition(this->getDefaultCamera()->getPosition() -
                                               (t->getLocation() - t->getPreviousLocation()));
    };

    touchListener->onTouchBegan = [this](ax::Touch*, ax::Event*) {
        m_player->pushButton(PlayerButton::Unk);
        return true;
    };

    touchListener->onTouchEnded = [this](ax::Touch*, ax::Event*) {
        m_player->releaseButton(PlayerButton::Unk);
        return true;
    };

    director->getEventDispatcher()
        ->addEventListenerWithSceneGraphPriority(touchListener, this);
}

PlayScene::~PlayScene() {
    if (m_level) {
        m_level->release();
    }

    if (m_levelSettings) {
        m_levelSettings->release();
    }

    ax::AudioEngine::stopAll();
}

bool PlayScene::init(Level* level) {
    if (!ax::Scene::init()) {
        return false;
    }

    const DirectorExt* director     = DirectorExt::getInstance();
    const ax::Size     winSize      = director->getWinSize();
    AssetManager*      assetManager = AssetManager::getInstance();
    GameManager*       gameManager  = GameManager::singleton();

    level->retain();
    m_level = level;

    State::getInstance()->setPlayLayer(this);

#pragma region Background
    m_bgSprite = assetManager->createSprite("game_bg_01_001.png");

    m_bgSprite->getTexture()->setTexParameters({
        ax::backend::SamplerFilter::LINEAR,
        ax::backend::SamplerFilter::LINEAR,
        ax::backend::SamplerAddressMode::REPEAT,
        ax::backend::SamplerAddressMode::MIRROR_REPEAT,
    });

    m_bgSprite->setAnchorPoint({0, 0});
    m_bgSprite->setScale(director->getContentScaleFactorMax() / director->getContentScaleFactor());
    m_bgSprite->setColor({0x28, 0x7D, 0xFF});

    m_bgSprite->setBlendFunc({
        .src = ax::backend::BlendFactor::ONE,
        .dst = ax::backend::BlendFactor::ZERO
    });

    m_backgroundWidth = m_bgSprite->getContentSize().width * m_bgSprite->getScale();

    {
        ax::Rect bgTexRect = m_bgSprite->getTextureRect();
        bgTexRect.size.width *= 2;

        m_bgSprite->setTextureRect(bgTexRect);
    }

    this->addChild(m_bgSprite);
#pragma endregion Background


    m_gameLayer = ax::Layer::create();
    this->addChild(m_gameLayer, 1);


#pragma region Ground
    m_regularGround = GroundLayer::create();
    m_regularGround->setVisible(true);
    this->addChild(m_regularGround, 4);


    m_rollGround.top = GroundLayer::create();
    m_rollGround.top->setScaleY(-1.0);
    this->addChild(m_rollGround.top, 4);

    m_rollGround.bottom = GroundLayer::create();
    this->addChild(m_rollGround.bottom, 4);


    m_flyGround = {
        .top = ax::Layer::create(),
        .topDetail {
            .groundSprite = assetManager->createSpriteWithFrameName("whiteSquare60_001.png"),
            .floorLine = assetManager->createSpriteWithFrameName("floorLine_001.png")
        },

        .bottom = ax::Layer::create(),
        .bottomDetail {
            .groundSprite = assetManager->createSpriteWithFrameName("whiteSquare60_001.png"),
            .floorLine = assetManager->createSpriteWithFrameName("floorLine_001.png")
        }
    };

    this->addChild(m_flyGround.bottom, 5);
    this->addChild(m_flyGround.top, 5);

    m_flyGround.bottom->setName("Bottom Fly Ground");
    m_flyGround.top->setName("Top Fly Ground");

    m_flyGround.bottom->addChild(this->m_flyGround.bottomDetail.groundSprite, 5);
    m_flyGround.top->addChild(this->m_flyGround.topDetail.groundSprite, 5);

    m_flyGround.bottom->addChild(this->m_flyGround.bottomDetail.floorLine, 6);
    m_flyGround.top->addChild(this->m_flyGround.topDetail.floorLine, 6);

    m_flyGround.bottomDetail.groundSprite->setAnchorPoint({0, 1.0});
    m_flyGround.topDetail.groundSprite->setAnchorPoint({0, 0});

    {
        ax::Rect groundTextureRect = m_flyGround.bottomDetail.groundSprite->getTextureRect();

        ax::Vec2 groundScale(
            (winSize.width + 20.0f) / groundTextureRect.size.width,
            (((winSize.height - 320.0f) / 2) + 30.0f) / groundTextureRect.size.height
        );

        m_flyGround.bottomDetail.groundSprite->setScale(groundScale.x, groundScale.y);
        m_flyGround.topDetail.groundSprite->setScale(groundScale.x, groundScale.y);
    }

    m_flyGround.bottomDetail.groundSprite->setPosition({-10.0, 0});
    m_flyGround.topDetail.groundSprite->setPosition({-10.0, 0});

    m_flyGround.bottomDetail.groundSprite->setColor({0, 102, 255});
    m_flyGround.topDetail.groundSprite->setColor({0, 102, 255});


    m_flyGround.bottomDetail.floorLine->setPosition({winSize.width / 2, 0});
    m_flyGround.topDetail.floorLine->setPosition({winSize.width / 2, 0});

    // TODO: set blend func for floor lines

    m_flyGroundVisualPos = {
        .bottom = (float)((float)(winSize.height * 0.5) - 160.0) + 10.0f,
        .top    = (float)((winSize.height * 0.5) + 160.0) - 10.0f
    };
    m_flyGround.bottom->setPosition({0, m_flyGroundVisualPos.bottom});
    m_flyGround.top->setPosition({0, m_flyGroundVisualPos.top});
#pragma endregion Ground


#pragma region BatchNodes
    {
        std::string spriteSheetName = assetManager->getForwardedFileName("GJ_GameSheet.png");

        m_batchNode = ax::SpriteBatchNode::create(spriteSheetName);
        m_additiveBatchNode = ax::SpriteBatchNode::create(spriteSheetName);
        m_playerBatchNode = ax::SpriteBatchNode::create(spriteSheetName);
    }

    m_additiveBatchNode->setBlendFunc(ax::BlendFunc::ADDITIVE);

    m_gameLayer->addChild(m_batchNode, 1);
    m_gameLayer->addChild(m_additiveBatchNode, 0);
    m_gameLayer->addChild(m_playerBatchNode, 1);
#pragma endregion BatchNodes


#pragma region Player
    m_player = PlayerObject::create(0);
    m_player->setPosition(m_startPos);
    m_player->setColor(gameManager->colorForIdx(gameManager->getPlayerColor()));
    m_player->setSecondColor(gameManager->colorForIdx(gameManager->getPlayerColor2()));

    m_playerBatchNode->addChild(m_player, 10);
#pragma endregion Player


    createObjectsFromSetup(level->getLevelData());
    
    updateCamera(0);
    updateVisibility();

    animateOutRollGround(true);
    animateOutFlyGround(true);
    tintBackground(m_levelSettings->getStartBGColor(), 0);
    tintGround(m_levelSettings->getStartGColor(), 0);

    setupKeybinds();
    setupTouchControls();

    m_debugDrawNode = ax::DrawNode::create();
    m_gameLayer->addChild(m_debugDrawNode, 1000);
    m_debugDrawNode->clear();

    // Uncomment to see hitboxes
    /*for (auto object : m_objects) {
        if (object->getIsDisabled()) {
            continue;
        }

        ax::Rect rect = object->getObjectRect();

        m_debugDrawNode->drawRect(
            rect.origin,
            rect.origin + rect.size,
            ax::Color4B::RED
        );
    }*/

    this->runAction(ax::Sequence::create(
        ax::DelayTime::create(0.5),
        ax::CallFunc::create(std::bind(&PlayScene::startGame, this)),
        NULL
    ));

    return true;
}

void PlayScene::update(float dt)
{
    float relativeDelta = dt * 60.0f;

    if (!m_player->getIsLocked()) {
        m_player->setPosition(m_lastPlayerPos);
    }

    m_player->setTouchedRing(nullptr);

    int steps = std::max(static_cast<int>(roundf(relativeDelta * 4.0f)), 4);
    float deltaPerStep = relativeDelta / static_cast<float>(steps);

    if (!m_onLevelEndAnimation && steps > 0) {
        for (int i = 0; i != steps && !m_onLevelEndAnimation; ++i) {
            m_player->update(deltaPerStep);
            checkCollisions(deltaPerStep);
        }
    }
    
    if (m_player->getFlyMode()) {
        m_player->updateShipRotation(relativeDelta);
    }

    if (!m_player->getIsLocked()) {
        m_lastPlayerPos = m_player->getPosition();
    }

    if (m_player->getIsLocked() || m_flipProgress == 1.0 || m_flipProgress == 0.0) {
        updateCamera(relativeDelta);
        updateVisibility();
        checkSpawnObjects();
    } else {
        float unk = m_flipProgress;

        if (m_flipScale== -1.0) {
            unk = 1.0 - m_flipProgress;
        }

        updateCamera(relativeDelta);
        updateVisibility();
        checkSpawnObjects();

        m_player->setPosition(m_player->getPosition() + ax::Vec2(unk * 150.0, 0));
    }

    //self->_clkTimer_290 = dt + self->_clkTimer_290;
    //self->_ael_1a4->audioStep(dt);
}

void PlayScene::updateTweenAction(float value, std::string_view key) {
    if (key == "cTY") {
        m_cameraPos.y = value;
    } else if (key == "fTX") {
        m_flipProgress = value;
    }
}

void PlayScene::tintBackground(ax::Color3B color, float t) {
    m_bgSprite->stopAllActions();
    m_activeBGColor = color;

    if (t <= 0) {
        return m_bgSprite->setColor(color);
    }

    m_bgSprite->runAction(ax::TintTo::create(t, color));
}

void PlayScene::tintGround(ax::Color3B color, float time) {
    std::array<ax::Node*, 5> nodesOfInterest = {
        m_regularGround->getGroundSprite(),

        m_flyGround.bottomDetail.groundSprite,
        m_flyGround.topDetail.groundSprite,

        m_rollGround.bottom,
        m_rollGround.top
    };

    // NOTE: Original code calls stopAllActions on regular ground sprite
    
    for (auto node : nodesOfInterest) {
        node->stopActionByTag(3);
    }

    m_activeGColor = color;

    if (time > 0) {
        for (auto node : nodesOfInterest) {
            ax::TintTo* tintAction = ax::TintTo::create(time, color);
            tintAction->setTag(3);

            node->runAction(tintAction);
        }
    } else {
        for (auto node : nodesOfInterest) {
            node->setColor(color);
        }
    }
}

const std::string PlayScene::getParticleKey(
    int objType, const char* particleName, int unk, ax::ParticleSystem::PositionType posType)
{
    return fmt::format("{}{}{}{}", objType, particleName, unk, static_cast<int>(posType));
}

const std::string PlayScene::getParticleKey2(const std::string particle) {
    return fmt::format("{}2", particle);
}

ax::ParticleSystemQuad* PlayScene::createParticle(int objType,
                                             const char* particleName,
                                             int unk,
                                             ax::ParticleSystem::PositionType posType)
{
    constexpr size_t MAX_PARTICLES_PER_KEY = 0x13;

    auto particleKey = getParticleKey(objType, particleName, unk, posType);

    if (!m_particleDictionary.contains(particleKey)) {
        m_particleDictionary[particleKey] = {};
        m_particleDictionary[getParticleKey2(particleKey)] = {};
    }

    auto& array = m_particleDictionary[particleKey];

    if (array.size() <= MAX_PARTICLES_PER_KEY) {
        auto particle = ax::ParticleSystemQuad::create(particleName);
        particle->setTag(unk);
        particle->setPositionType(posType);
        particle->stopSystem();

        array.pushBack(particle);
        m_gameLayer->addChild(particle, unk);

        return particle;
    }

    return nullptr;
}

ax::ParticleSystemQuad* PlayScene::claimParticle(std::string key) {
    if (!m_particleDictionary.contains(key) || !m_particleDictionary[key].size())
        return nullptr;

    auto& unkArr = m_particleDictionary[key];
    auto lastObj = unkArr.back();

    m_particleDictionary[getParticleKey2(key)].pushBack(lastObj);
    unkArr.eraseObject(unkArr.back());
    lastObj->setVisible(true);

    return lastObj;
}

void PlayScene::unclaimParticle(const char* key, ax::ParticleSystemQuad* particleSystem) {
    if (!particleSystem || !m_particleDictionary.contains(key)) {
        return;
    }

    m_particleDictionary[key].pushBack(particleSystem);
    m_particleDictionary[getParticleKey2(key)].eraseObject(particleSystem, false);
    particleSystem->setVisible(false);
}



void PlayScene::checkCollisions(float dt) {
    if (m_player->getPositionY() < 105 && !m_player->getFlyMode()) {
        if (m_player->getGravityFlipped()) {
            destroyPlayer();
            return;
        }

        m_player->setPositionY(105);
        m_player->hitGround(false);
    } else if (m_player->getPositionY() > 1590) {
        destroyPlayer();
        return;
    }

    if (m_player->getFlyMode() || m_player->getRollMode()) {
        float topBoundary = m_gameModeGroundPos.top - 15.0f;
        float bottomBoundary = m_gameModeGroundPos.bottom + 15.0;

        if (m_player->getPositionY() > topBoundary) {
            m_player->setPositionY(topBoundary);
            m_player->hitGround(m_player->getGravityFlipped() ^ 1);
        } else if (m_player->getPosition().y < bottomBoundary) {
            m_player->setPositionY(bottomBoundary);
            m_player->hitGround(m_player->getGravityFlipped());
        }
    }

    int sectionId = sectionForPos(m_player->getPosition());

    for (int sectionIndex = sectionId - 1; sectionIndex <= sectionId + 1; sectionIndex++) {
        if (sectionIndex < 0 || m_sections.size() <= sectionIndex) {
            continue;
        }

        auto section = m_sections[sectionIndex];

        for (auto object : section) {
            if (object->getType() == GameObjectType::Hazard) {
                m_hazards.pushBack(object);
                continue;
            }

            if (object->getIsDisabled() || object->getHasBeenActivated() ||
                !m_player->getObjectRect().intersectsRect(object->getObjectRect()))
            {
                continue;
            }

            GameObjectType type = object->getType();

            switch (type)
            {
            case GameObjectType::InvertGravityPortal:
                if (!m_player->getGravityFlipped()) {
                    this->playGravityEffect(true);
                }

                m_player->setPortalP(object->getPosition());
                m_player->flipGravity(true);
                object->triggerActivated();

                break;

            case GameObjectType::NormalGravityPortal:
                if (m_player->getGravityFlipped()) {
                    this->playGravityEffect(false);
                }

                m_player->setPortalP(object->getPosition());
                m_player->flipGravity(false);
                object->triggerActivated();

                break;
            case GameObjectType::ShipPortal:
                switchToFlyMode(object, false);
                object->triggerActivated();

                break;
            case GameObjectType::CubePortal:
                m_player->setPortalP(object->getPosition());

                exitFlyMode();
                exitRollMode();

                object->triggerActivated();

                break;
            case GameObjectType::YellowPad:
                m_player->setPortalP({object->getPosition().x, object->getPosition().y - 10});
                object->triggerActivated();
                m_player->propellPlayer(1.0);

                break;
            case GameObjectType::GravityPad: {
                bool flipped = (std::fabs(object->getRotation()) == 180);

                if (!object->isFlippedY()) {
                    flipped ^= true;
                }

                if (flipped != m_player->getGravityFlipped()) {
                    //self->playGravityEffect(flipped);
                    auto objectPosition = object->getPosition();
                    m_player->setPortalP({objectPosition.x, objectPosition.y - 10});
                    object->triggerActivated();
                    m_player->propellPlayer(0.8f);
                    m_player->flipGravity(flipped);
                }

                break;
            }
            case GameObjectType::YellowOrb:
                [[fallthrough]];
            case GameObjectType::BlueOrb:
                m_player->setTouchedRing(object);
                //object->powerOnObject();
                m_player->ringJump();

                break;
            case GameObjectType::MirrorPortal:
                m_player->setPortalP(object->getPosition());
                m_player->setPortalObject(object);
                toggleFlipped(true, false);

                object->triggerActivated();

                break;
            case GameObjectType::CounterMirrorPortal:
                m_player->setPortalP(object->getPosition());
                m_player->setPortalObject(object);
                toggleFlipped(false, false);

                object->triggerActivated();

                break;
            case GameObjectType::BallPortal:
                switchToRollMode(object, false);
                object->triggerActivated();

                break;
                
            default:
                m_player->collidedWithObject(dt, object);
                break;
            }
        }
    }

    for (auto hazard : m_hazards) {
        if (m_player->getObjectRect().intersectsRect(hazard->getObjectRect())) {
            this->destroyPlayer();

            // - replace the `break` below with `return`
            // - change destroyPlayer to do an early return,
            // - run through a bunch of ground spikes,
            // - watch as the game starts running to a crawl :)
            // (I probably made a mistake while decompiling this but it also happens in the original game)

            break;
        }
    }

    m_hazards.clear();
}

void PlayScene::destroyPlayer() {
    if (m_player->getIsLocked() || m_onLevelEndAnimation) {
        return;
    }

    ax::AudioEngine::stopAll();

    m_onLevelEndAnimation = true;
    m_player->playerDestroyed();

    ax::AudioEngine::play2d("explode_11.ogg", false, 0.6);

    m_resetQueued = true;

    runAction(ax::Sequence::create(
        ax::DelayTime::create(1),
        ax::CallFunc::create(std::bind(&PlayScene::delayedResetLevel, this)),
        NULL
    ));
}

void PlayScene::delayedResetLevel() {
    if (m_resetQueued) {
        resetLevel();
    }
}

int PlayScene::sectionForPos(ax::Vec2 pos) {
    return static_cast<int>(floorf(pos.x / 100));
}

void PlayScene::switchToFlyMode(GameObject* portal, bool instantCamera) {
    exitRollMode();

    m_player->setPortalP(portal->getPosition());
    m_player->setPortalObject(portal);
    m_player->toggleFlyMode(true);

    ax::Vec2 realPortalPos = portal->getRealPosition();

    m_gameModeGroundPos.bottom = realPortalPos.y - 150;
    m_gameModeGroundPos.bottom = std::max(floorf(m_gameModeGroundPos.bottom / 30) * 30, 90.0f);
    m_gameModeGroundPos.top    = m_gameModeGroundPos.bottom + 300;

    m_unk13c = m_gameModeGroundPos.bottom + 150;

    animateInFlyGround(false);

    float camY = (ax::Director::getInstance()->getWinSize().height * -0.5) + m_unk13c;

    if (instantCamera) {
        m_isMovingCameraY = true;
        m_cameraPos.y = camY;
    } else {
        cameraMoveY(camY, 0.5, 1.4);
    }
}

void PlayScene::switchToRollMode(GameObject* portal, bool instantCamera) {
    const ax::Size& winSize = ax::Director::getInstance()->getWinSize();

    exitFlyMode();

    m_player->setPortalP(portal->getPosition());
    m_player->setPortalObject(portal);
    m_player->toggleRollMode(true);

    const ax::Vec2 realPortalPos        = portal->getRealPosition();

    m_gameModeGroundPos.bottom = realPortalPos.y - 120;
    m_gameModeGroundPos.bottom = std::max(floorf(m_gameModeGroundPos.bottom / 30) * 30, 90.0f);
    m_gameModeGroundPos.top    = m_gameModeGroundPos.bottom + 240;

    m_unk13c   = m_gameModeGroundPos.bottom + 120;

    m_rollGroundVisualPos.bottom    = (winSize.height / 2) - 120 - m_rollGround.bottom->getGroundSprite()->getPositionY();
    m_rollGroundVisualPos.top =
        (winSize.height / 2) + 120 - (winSize.height - m_rollGround.bottom->getGroundSprite()->getPositionY());

    animateInRollGround(false);

    float camY = (winSize.height * -0.5) + m_unk13c;
    if (instantCamera) {
        m_isMovingCameraY = true;
        m_cameraPos.y = camY;
    } else {
        cameraMoveY(camY, 0.5, 1.4);
    }
}

void PlayScene::exitFlyMode() {
    m_player->toggleFlyMode(false);
    m_isMovingCameraY = false;

    animateOutFlyGround(false);
}

void PlayScene::exitRollMode() {
    m_player->toggleRollMode(false);
    animateOutRollGround(false);
}

void PlayScene::updateCamera(float dt) {
    ax::Director* const director = ax::Director::getInstance();
    const ax::Size winSize  = director->getWinSize();

    auto camOffset = (winSize.width * -0.5) + 75;
    auto camPos    = m_cameraPos;
    auto playerPos = m_player->getPosition();

    float y    = camPos.y;
    float rate = 0;

    if (m_player->getFlyMode()) {
        y = (winSize.height * -0.5) + m_unk13c;

        if (y <= 0) {
            y = 0;
        }

        rate = 30;
    } else {
        float lowThreshold  = (m_player->getGravityFlipped()) ? 90 : 120;
        float highThreshold = (m_player->getGravityFlipped()) ? 120 : 90;

        if (m_player->getPosition().y > (winSize.height + camPos.y) - highThreshold) {
            y = (m_player->getPosition().y - winSize.height) + highThreshold;
        } else if (m_player->getPosition().y < (lowThreshold + camPos.y)) {
            y = m_player->getPosition().y - lowThreshold;
        }

        if (m_player->getGravityFlipped()) {
            rate = 10;
        } else {
            if (m_player->getLastGroundPos().y == 105) {
                rate = 10;

                if (m_player->getPosition().y <= (camPos.y + winSize.height) - highThreshold) {
                    y = 0;
                }
            } else {
                rate = 10;
            }
        }
    }

    float v7 = (float)((float)(y - camPos.y) / (float)(rate / dt)) + camPos.y;
    if (v7 <= 0.0) {
        v7 = 0.0;
    }
    if (v7 >= (float)(1440.0 - winSize.height)) {
        v7 = 1440.0 - winSize.height;
    }
    camPos.y = v7;
    float v9 = camOffset + m_player->getPosition().x;

    camPos = {v9, camPos.y};

    if (m_firstStart) {
        if (camPos.x <= 15) {
            camPos.x = 15;
        }
    }

    if (!m_isMovingCameraX || !m_isMovingCameraY) {
        if (!m_isMovingCameraX) {
            m_cameraPos.x = camPos.x;
        }

        if (!m_isMovingCameraY) {
            m_cameraPos.y = camPos.y;
        }
    }

    if (m_flipProgress == 1) {
        m_flipScale = -1;
    } else if (m_flipProgress == 0) {
        m_flipScale = 1;
    }

    m_gameLayer->setScaleX(m_flipScale);

    if (m_flipScale == -1) {
        m_gameLayer->setPosition({m_cameraPos.x, -m_cameraPos.y});
    } else {
        m_gameLayer->setPosition({-m_cameraPos.x, -m_cameraPos.y});
    }

    // Background
    float unkBgFlipValue = 0;

    if (m_flipProgress <= 0.5) {
        unkBgFlipValue = (m_flipProgress * -2.0f) + 1;
    } else {
        unkBgFlipValue = (m_flipProgress - 0.5) * -2;
    }

    float bgPosXDiff;

    if (m_backgroundXPosOffset == 0) {
        bgPosXDiff = 0;
    } else {
        bgPosXDiff = (camOffset - m_cameraPos.x) - m_backgroundXPosOffset;
    }

    float unk = ((bgPosXDiff * 0.1) * unkBgFlipValue) + m_bgSprite->getPosition().x;

    ax::Vec2 bgPos = {unk, (-m_cameraPos.y) * 0.1f};
    float bgPosX = bgPos.x;

    if (bgPos.x < -m_backgroundWidth) {
        float newBgPosX;

        do {
             newBgPosX = bgPosX + m_backgroundWidth;
            bgPos.x   = bgPosX + m_backgroundWidth;
             bgPosX    = newBgPosX;
        } while (-m_backgroundWidth > newBgPosX);
    }

    if (bgPosX > 0) {
        do {
             bgPos.x = bgPosX - m_backgroundWidth;
             bgPosX  = bgPos.x;
        } while (bgPos.x > 0);
    }

    m_bgSprite->setPosition(bgPos);



    // ground

    ax::Vec2 gPos = {(bgPosXDiff * unkBgFlipValue) + m_regularGround->getGroundSprite()->getPositionX(), 90};

    float groundWidth = m_regularGround->getGroundWidth();
    float newGPosX = gPos.x;

    if (gPos.x < -groundWidth) {
        do {
             newGPosX += groundWidth;
        } while (-groundWidth > newGPosX);
        gPos.x = newGPosX;
    }
    
    if (newGPosX > 0) {
        do {
             newGPosX -= groundWidth;
        } while (newGPosX > 0);
        gPos.x = newGPosX;
    }

    m_rollGround.bottom->getGroundSprite()->setPosition(gPos);
    m_rollGround.top->getGroundSprite()->setPosition(gPos);
    m_regularGround->getGroundSprite()->setPosition(gPos);
    m_regularGround->setPosition({0, -m_cameraPos.y});


    m_backgroundXPosOffset = camOffset - m_cameraPos.x;

    return;
}

void PlayScene::cameraMoveX(float, float, float) {}

void PlayScene::cameraMoveY(float y, float t, float r) {
    stopActionByTag(1);
    m_isMovingCameraY = true;

    ax::ActionTween* tween = ax::ActionTween::create(t, "cTY", m_cameraPos.y, y);
    ax::EaseInOut* act   = ax::EaseInOut::create(tween, r);
    act->setTag(1);

    this->runAction(act);
}

void PlayScene::updateVisibility() {
    auto cameraPos = this->m_cameraPos;
    auto director          = ax::Director::getInstance();

    int previousSection = floorf(cameraPos.x / 100.0f) - 1;
    int nextSection     = ceilf((cameraPos.x + ax::Director::getInstance()->getWinSize().width) / 100.0f) + 1;

    bool isFlipping = this->isFlipping();
    float audioScale       = 1;

    auto apply_flip_effect = [director, this](GameObject* object) {
        auto position = object->getPosition();

        float invertValue = (m_flipScale == -1.0) ? 1.0f - m_flipProgress : m_flipProgress;
        float relativeX   = position.x - m_cameraPos.x;
        float screenRight = director->getWinSize().width;
        float screenLeft  = 0;

        object->setPosition(
            {(((screenRight + screenLeft) + (relativeX * -2.0f)) * invertValue) + position.x, position.y});

        float abs_rot = std::abs(object->getRotation());
        bool flipped  = (abs_rot != 90.0) ? abs_rot == 270.0 : true;

        if (object->getUseAudioScale()) {
            return;
        }

        auto flip = [](GameObject* object, bool vertical) {
            if (!vertical) {
                object->setScaleX(-object->getScaleX());
            } else {
                object->setScaleY(-object->getScaleY());
            }
        };

        if (m_flipScale != 1.0) {
            if (this->m_flipProgress > 0.5) {
                return;
            }

            flip(object, flipped);
        } else if (m_flipProgress >= 0.5) {
            flip(object, flipped);
        }
    };

    if (previousSection <= nextSection) {
        for (int i = previousSection; i < nextSection; i++) {
            if (i < 0 || i >= m_sections.size()) {
                continue;
            }

            // Process each section up until the furthest on the screen
            ax::Vector<GameObject*> section = m_sections[i];

            for (auto object : section) {
                object->activateObject();

                if (object->getUseAudioScale()) {
                    object->setScale(audioScale);
                }

                float sc = (object->getType() == GameObjectType::UnknownType)
                               ? (object->getObjectRect().size.width * object->getScaleX()) * 0.4
                               : 0;

                if (!object->getDontTransform()) {
                    object->setOpacity(this->getRelativeMod(object->getRealPosition(), 70, 70, sc) * 255);
                    this->applyEnterEffect(object);

                    if (isFlipping) {
                        apply_flip_effect(object);
                    }
                }
            }
        }
    }

    if (this->m_nextSection >= this->m_previousSection) {
        for (int i = m_previousSection; i < this->m_nextSection; i++) {
            if (i < 0 || i >= m_sections.size()) {
                continue;
            }

            for (GameObject* object : m_sections[i]) {
                object->deactivateObject();
            }
        }
    }

    m_previousSection = previousSection;
    m_nextSection     = nextSection;
}

void PlayScene::toggleFlipped(bool flipped, bool instant)
{
    if (m_isFlipped == flipped) {
        return;
    }

    m_isFlipped        = flipped;

    float endFlipProg = (flipped) ? 1.0 : 0;

    if (instant) {
        m_flipProgress = endFlipProg;
        return;
    }

    //TODO: the flip callback (i forgot)
    auto act = ax::EaseInOut::create(ax::ActionTween::create(0.5, "fTX", m_flipProgress, endFlipProg), 1.2);
    act->setTag(4);
    this->runAction(act);
}

bool PlayScene::isFlipping() const
{
    if (m_flipProgress != 0 && m_flipProgress != 1) {
        return true;
    }

    return false;
}

float PlayScene::getRelativeMod(ax::Vec2 pos, float a2, float a3, float a4) {
    float screenWidth = ax::Director::getInstance()->getWinSize().width / 2;
    float camX        = m_cameraPos.x;

    float unk, unk2;

    if (pos.x <= (camX + screenWidth)) {
        unk = a3;
        unk2 = ((camX + screenWidth) - pos.x) - a4;
    } else {
        unk = a2;
        unk2 = ((pos.x - a4) - camX) - screenWidth;
    }

    if (unk < 1.0) {
        unk = 1.0;
    }

    float res = (screenWidth - unk2) / unk;
    return std::clamp(res, 0.0f, 1.0f);
}

void PlayScene::animateInFlyGround(bool instant) {
    if (m_flyGroundActive) {
        return;
    }

    m_flyGround.top->stopAllActions();
    m_flyGround.bottom->stopAllActions();
    m_flyGround.top->setVisible(true);
    m_flyGround.bottom->setVisible(true);

    if (instant) {

    } else {
        m_flyGround.top->runAction(ax::EaseInOut::create(ax::MoveTo::create(0.5, {0, m_flyGroundVisualPos.top}), 2));
        m_flyGround.bottom->runAction(ax::EaseInOut::create(ax::MoveTo::create(0.5, {0, m_flyGroundVisualPos.bottom}), 2));

        m_flyGround.top->setCascadeOpacityEnabled(true);
        m_flyGround.bottom->setCascadeOpacityEnabled(true);

        m_flyGround.top->runAction(ax::FadeIn::create(0.5));
        m_flyGround.bottom->runAction(ax::FadeIn::create(0.5));
    }
}

void PlayScene::animateOutFlyGround(bool instant) {
    m_flyGroundActive = false;

    const ax::Vec2& winSize = ax::Director::getInstance()->getWinSize();
    ax::Vec2 endBottom = {0, -2};
    ax::Vec2 endTop    = {0, winSize.height + 2};

    if (instant) {
        m_flyGround.bottom->setPosition(endBottom);
        m_flyGround.top->setPosition(endTop);
        return;
    }

    auto actBot = ax::EaseInOut::create(ax::MoveTo::create(0.4, endBottom), 1.5);
    auto actTop = ax::EaseInOut::create(ax::MoveTo::create(0.4, endTop), 1.5);

    m_flyGround.bottom->runAction(actBot);
    m_flyGround.top->runAction(actTop);

    //TODO: animateOutFlyGroundFinished

    m_flyGround.bottom->setCascadeOpacityEnabled(true);
    m_flyGround.top->setCascadeOpacityEnabled(true);

    m_flyGround.bottom->runAction(ax::FadeOut::create(0.4));
    m_flyGround.top->runAction(ax::FadeOut::create(0.4));
}

void PlayScene::createObjectsFromSetup(std::string setup) {
    auto [headerSetup, dataSetup] = split_string::split_to_pair(setup, ";");

    m_levelSettings = LevelSettings::objectFromString(headerSetup);
    m_levelSettings->retain();

    split_string::split_streamed(dataSetup, ";", [this](std::string_view setup) {
        GameObject* object = GameObject::createFromString(setup);

        if (!object) {
            return;
        }

        object->setVisible(false);

        if (object->getStartPosition().x > m_maxObjectXPos) {
            m_maxObjectXPos = object->getStartPosition().x;
        }

        if (object->getBlendAdditive()) {
            if (object->getUsePlayerColor()) {
                object->setColor(m_player->getColor());
            } else if (object->getUsePlayerColor2()) {
                GameManager* gm = GameManager::singleton();
                object->setColor(gm->colorForIdx(gm->getPlayerColor2()));
            }
            object->setObjectParent(m_additiveBatchNode);
        } else {
            object->setObjectParent(m_batchNode);
        }

        addToSection(object);
        m_objects.pushBack(object);

        if (object->getShouldSpawn()) {
            m_spawnObjects.pushBack(object);
            object->calculateSpawnXPos();
        }

        auto frame = object->getFrame();

        if (object->getObjectKey() == 31) {
            if (object->getPosition().x > m_startPos.x) {
                m_startPos = object->getPosition();
                m_testMode = true;
            }
        } else if (frame.starts_with("rod_0")) {
            //TODO: The pulse things
        } else if (frame.starts_with("portal_0")) {
            std::string backPortalTexture;

            if (frame == "portal_01_front_001.png")
                backPortalTexture = "portal_01_back_001.png";
            else if (frame == "portal_02_front_001.png")
                backPortalTexture = "portal_02_back_001.png";
            else if (frame == "portal_03_front_001.png")
                backPortalTexture = "portal_03_back_001.png";
            else if (frame == "portal_04_front_001.png")
                backPortalTexture = "portal_04_back_001.png";
            else if (frame == "portal_05_front_001.png")
                backPortalTexture = "portal_05_back_001.png";
            else if (frame == "portal_06_front_001.png")
                backPortalTexture = "portal_06_back_001.png";
            else if (frame == "portal_07_front_001.png")
                backPortalTexture = "portal_07_back_001.png";
            else
                backPortalTexture = "portal_01_back_001.png";

            GameObject* back = GameObject::create(backPortalTexture);
            back->setObjectKey(38);
            back->customSetup();

            back->setStartPosition(object->getPosition());
            back->setObjectParent(m_batchNode);
            back->setObjectZ(-1);

            back->setFlippedX(object->isFlippedX());
            back->setFlippedY(object->isFlippedY());

            back->setRotation(object->getRotation());
            back->setStartRotation(object->getRotation());

            addToSection(back);
        }
    });

    ax::Director* const director = ax::Director::getInstance();
    float screenLeft = director->getWinSize().width;

    float max = m_maxObjectXPos + 340;

    if (screenLeft + 300 >= max) {
        max = screenLeft + 300;
    }

    m_levelSize = max;

    //TODO: End portal object
}

void PlayScene::addToSection(GameObject* obj) {
    auto section = sectionForPos(obj->getPosition());

    if (m_sections.size() < (section + 1)) {
        while (m_sections.size() < (section + 1)) {
            m_sections.emplace_back();
        }
    }

    m_sections[section].pushBack(obj);
    obj->setSectionIdx(section);
}

void PlayScene::resetLevel() {
    //TODO: this

    m_resetQueued = false;
    m_activeEnterEffect = 1;
    stopActionByTag(0);
    stopActionByTag(1);
    m_isMovingCameraX = false;
    m_isMovingCameraY = false;
    m_firstStart          = m_cleanReset;
    m_onLevelEndAnimation = false;

    for (auto obj : m_objects) {
        obj->resetObject();
        obj->setEnterEffect(1);
    }

    m_isFlipped = false;
    m_flipProgress = 0;
    m_backgroundXPosOffset = 0;
    m_flipScale    = 1;
    stopActionByTag(4);
    m_player->resetObject();
    animateOutFlyGround(true);
    animateOutRollGround(true);

    m_spawnQueue.clear();
   
    for (auto obj : m_spawnObjects) {
        m_spawnQueue.pushBack(obj);
    }
    std::stable_sort(m_spawnQueue.begin(), m_spawnQueue.end(),
                     [](GameObject* lhs, GameObject* rhs) {return lhs->getSpawnXPos() < rhs->getSpawnXPos();});

    ax::Director* const director = ax::Director::getInstance();
    const ax::Vec2& winSize = director->getWinSize();

    m_cameraPos.y = (m_player->getPositionY() - winSize.height) + 90;
    updateCamera(0);
    updateVisibility();
    m_cleanReset = false;

    tintBackground(m_levelSettings->getStartBGColor(), 0);
    tintGround(m_levelSettings->getStartGColor(), 0);

    m_lastPlayerPos = m_player->getPosition();
    updateCamera(0);
    updateVisibility();

    ax::AudioPlayerSettings aps{};
    aps.volume = 0.9;

    ax::AudioEngine::play2d(::getAudioFileName(m_levelSettings->getAudiotrack()), aps);
}

void PlayScene::checkSpawnObjects() {
    if (!m_spawnQueue.size()) {
        return;
    }

    GameObject* obj = m_spawnQueue[0];

    if (m_player->getPositionX() >= obj->getSpawnXPos()) {
        obj->triggerObject();
        m_spawnQueue.erase(0);
    }
}

void PlayScene::applyEnterEffect(GameObject* object)
{
    auto realPos = object->getRealPosition();

    if (!object->getEnterEffect())
    {
        const ax::Vec2& winSize = ax::Director::getInstance()->getWinSize();

        object->setEnterEffect(m_activeEnterEffect);

        switch (m_activeEnterEffect) {
            case 8:
                if ((winSize.height / 2) + m_cameraPos.y >= realPos.y) {
                    object->setEnterAngle(45);
                } else {
                    object->setEnterAngle(135);
                }

                break;
            case 9:
                if ((winSize.height / 2) + m_cameraPos.y >= realPos.y) {
                    object->setEnterAngle(-45);
                } else {
                    object->setEnterAngle(-135);
                }

                break;
            case 10: {
                float randVal = ax::rand_0_1();
                object->setEnterAngle(180 * ((randVal + randVal) - 1));

                break;
            }
            case 11:
                if ((winSize.height / 2) + m_cameraPos.y < realPos.y) {
                    object->setEnterAngle(180);
                } else {
                    object->setEnterAngle(0);
                }

                break;
            case 12:
                if ((winSize.height / 2) + m_cameraPos.y < realPos.y) {
                    object->setEnterAngle(180);
                } else {
                    object->setEnterAngle(0);
                }
                object->setEnterAngle(object->getEnterAngle() + 180);

                break;
            default:
                break;
        }
    }

    
    float relativeMod = getRelativeMod(realPos, 60, 60, 0);

    switch (object->getEnterEffect()) {
        case 2:
            if (!object->getUseAudioScale()) {
                auto newScale = object->getStartScale() * relativeMod;
                object->setScale(newScale.x, newScale.y);
            }
            object->setPosition(realPos);

            break;
        case 3:
            if (!object->getUseAudioScale()) {
                ax::Vec2 newScale = object->getStartScale() * (((1.0f - relativeMod) * 0.75) + 1.0);
                object->setScale(newScale.x, newScale.y);
            }
            object->setPosition(realPos);

            break;
        case 4:
            object->setPosition(realPos + ax::Vec2{0, (1.0f - relativeMod) * 100.0f});
            if (!object->getUseAudioScale()) {
                object->setScale(object->getStartScaleX(), object->getStartScaleY());
            }

            break;
        case 5:
            object->setPosition(realPos + ax::Vec2{0, (1.0f - relativeMod) * -100.0f});
            if (!object->getUseAudioScale()) {
                object->setScale(object->getStartScaleX(), object->getStartScaleY());
            }

            break;
        case 6:
            object->setPosition(realPos + ax::Vec2{(1.0f - relativeMod) * -100.0f, 0});
            if (!object->getUseAudioScale()) {
                object->setScale(object->getStartScaleX(), object->getStartScaleY());
            }

            break;
        case 7:
            object->setPosition(realPos + ax::Vec2{(1.0f - relativeMod) * 100.0f, 0});
            if (!object->getUseAudioScale()) {
                object->setScale(object->getStartScaleX(), object->getStartScaleY());
            }

            break;
        case 8:
        case 9:
        case 10:
        case 11:
        case 12: {
            float angle = (object->getEnterAngle() - 90.0) * 0.017453;
            auto ccpForAngle = [](float angle) {
                return ax::Vec2{cos(angle), sin(angle)}; };
            ax::Vec2 point       = ccpForAngle(angle);

            ax::Vec2 newPos = ax::Vec2{
                ((1.0f - relativeMod) * 100.0f) * point.x,
                ((1.0f - relativeMod) * 100.0f) * point.y}
                + realPos;
            
            object->setPosition(newPos);

            if (!object->getUseAudioScale()) {
                object->setScale(object->getStartScaleX(), object->getStartScaleY());
            }

            break;
        }
        default:
            object->setPosition(realPos);

            if (!object->getUseAudioScale()) {
                object->setScale(object->getStartScaleX(), object->getStartScaleY());
            }
    }

    if (relativeMod == 1 || relativeMod == 0) {
        object->setEnterEffect(0);
    }
}

void PlayScene::startGame() {
    this->scheduleUpdate();

    m_cleanReset = true;
    m_player->setVisible(true);

    resetLevel();
}

void PlayScene::animateInRollGround(bool instant) {
    if (!m_rollGroundActive) {
        m_rollGroundActive = true;

        m_rollGround.top->stopAllActions();
        m_rollGround.bottom->stopAllActions();

        m_rollGround.top->setVisible(true);
        m_rollGround.bottom->setVisible(true);

        if (instant) {
            //TODO: implement instant
        } else {
            ax::EaseInOut* actBot = ax::EaseInOut::create(
                ax::MoveTo::create(0.5, {0, m_rollGroundVisualPos.bottom}), 2);
            ax::EaseInOut* actTop = ax::EaseInOut::create(
                ax::MoveTo::create(0.5, {0, m_rollGroundVisualPos.top}), 2);

            m_rollGround.bottom->runAction(actBot);
            m_rollGround.top->runAction(actTop);

            m_rollGround.bottom->fadeInGround(0.5);
            m_rollGround.top->fadeInGround(0.5);
        }
    }
}

void PlayScene::animateOutRollGround(bool instant) {
    const ax::Size& winSize      = ax::Director::getInstance()->getWinSize();

    m_rollGroundActive = false;

    float endBottom = -2.0f - m_rollGround.bottom->getGroundSprite()->getPositionY();
    float endTop    = (winSize.height + 2.0f) - (winSize.height - m_rollGround.bottom->getGroundSprite()->getPositionY());

    if (instant) {
        m_rollGround.bottom->stopAllActions();
        m_rollGround.top->stopAllActions();

        animateOutRollGroundFinished();

        m_rollGround.bottom->setPositionY(endBottom);
        m_rollGround.top->setPositionY(endTop);

        return;
    }

    auto topAct = ax::EaseInOut::create(
        ax::MoveTo::create(0.4f, {0, endTop, 0}), 1.5);
    auto bottomAct = ax::EaseInOut::create(
        ax::MoveTo::create(0.4f, {0, endBottom, 0}), 1.5);


    m_rollGround.bottom->runAction(bottomAct);
    m_rollGround.top->runAction(topAct);
    m_rollGround.top->runAction(ax::Sequence::create(
        ax::DelayTime::create(0.6f),
        ax::CallFunc::create([this]() { animateOutRollGroundFinished(); }),
        NULL
    ));

    m_rollGround.bottom->fadeOutGround(0.4f);
    m_rollGround.top->fadeOutGround(0.4f);
}

void PlayScene::animateOutRollGroundFinished() {
    m_rollGround.bottom->setVisible(false);
    m_rollGround.top->setVisible(false);
}

void PlayScene::playGravityEffect(bool) {

}
