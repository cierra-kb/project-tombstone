#include "GameObject.h"
#include "Utils/SplitString.inl.h"
#include "Scenes/PlayLayer.h"
#include "State.h"

#include <2d/ParticleSystemQuad.h>
#include <2d/SpriteBatchNode.h>
#include <2d/Layer.h>
#include <platform/FileUtils.h>
#include <base/Utils.h>

#include <nlohmann/json.hpp>

enum class ObjectPropertyID {
    Invalid      = 0,
    ObjectID     = 1,
    PosX         = 2,
    PosY         = 3,
    FlipX        = 4,
    FlipY        = 5,
    Rotation     = 6,
    TintR        = 7,
    TintG        = 8,
    TintB        = 9,
    TintDuration = 10
};

class GameObjectDictionary {
private:
    static GameObjectDictionary& getInstance() {
        static GameObjectDictionary singleton;
        return singleton;
    }
public:
    static const std::string& keyToFrame(int id) {
        return getInstance().m_keyToFrameMap[id];
    }
    static bool isKeyDefined(int id) {
        return getInstance().m_keyToFrameMap.contains(id);
    }

    bool initFromFile(std::string_view filePath);
private:
    std::unordered_map<int, std::string> m_keyToFrameMap;

    GameObjectDictionary();
    AX_DISALLOW_COPY_AND_ASSIGN(GameObjectDictionary);
};

GameObjectDictionary::GameObjectDictionary() {
    if (!GameObjectDictionary::initFromFile("tombstone/blocks.json")) {
        throw std::runtime_error("failed to parse blocks.json");
    }
}

bool GameObjectDictionary::initFromFile(std::string_view filePath) {
    nlohmann::basic_json<> data;

    {
        std::string buffer;
        ax::FileUtils::getInstance()->getContents(filePath, &buffer);
        data = nlohmann::json::parse(buffer);
    }

    if (!data.is_array()) {
        return false;
    }

    for (nlohmann::basic_json<>& entry : data) {
        if (!entry.is_object()) {
            continue;
        }

        auto key = entry.value<int>("idx", 0);
        auto texture = entry.value<std::string>("texture", "");

        m_keyToFrameMap[key] = texture;
    }

    return true;
}



GameObject::~GameObject() {
    if (m_glowSprite) {
        m_glowSprite->removeFromParent();
        m_glowSprite->release();
    }
}

GameObject* GameObject::createFromString(std::string_view str) {
    auto arr = split_string::split(str, ",");

    ObjectPropertyID propertyId = ObjectPropertyID::Invalid;
    int objectId                = -1;
    float xPos                  = 0;
    float yPos                  = 0;
    float rot                   = 0;
    bool flipX                  = false;
    bool flipY                  = false;
    ax::Color3B tint            = {};
    float tintDuration          = 0;

    for (const std::string& str : arr) {
        if (propertyId == ObjectPropertyID::Invalid) {
            if (str.empty() || str == "\n") {
                break;
            }
            propertyId = (ObjectPropertyID)std::stoi(str);
            continue;
        }

        switch (propertyId) {
            case ObjectPropertyID::ObjectID:
                objectId = std::stoi(str);
                break;
            case ObjectPropertyID::PosX:
                xPos = static_cast<float>(std::stoi(str));
                break;
            case ObjectPropertyID::PosY:
                yPos = static_cast<float>(std::stoi(str)) + 90;
                break;
            case ObjectPropertyID::Rotation:
                rot = static_cast<float>(std::stoi(str));
                break;
            case ObjectPropertyID::FlipX:
                flipX = (bool)std::stoi(str);
                break;
            case ObjectPropertyID::FlipY:
                flipY = (bool)std::stoi(str);
                break;
            case ObjectPropertyID::TintR:
                tint.r = std::stoi(str);
                break;
            case ObjectPropertyID::TintG:
                tint.g = std::stoi(str);
                break;
            case ObjectPropertyID::TintB:
                tint.b = std::stoi(str);
                break;
            case ObjectPropertyID::TintDuration:
                tintDuration = std::stof(str);
                break;
            default:
                break;
        }

        propertyId = ObjectPropertyID::Invalid;
    }

    if (!GameObjectDictionary::isKeyDefined(objectId)) {
        return nullptr;
    }

    auto obj = ax::utils::createInstance<GameObject>(
        &GameObject::init, GameObjectDictionary::keyToFrame(objectId));

    obj->setObjectKey(objectId);

    if (objectId == 9) {
        obj->setObjectZ(2);
    }

    obj->addGlow();

    obj->setPosition({xPos, yPos});
    obj->setStartPosition(obj->getPosition());

    obj->setRotation(rot);

    obj->setFlippedX(flipX);
    obj->setFlippedY(flipY);

    obj->customSetup();

    if (obj->getFrame() == "edit_eTintBGBtn_001.png" || obj->getFrame() == "edit_eTintGBtn_001.png") {
        obj->setTintColor(tint);
        obj->setTintDuration(tintDuration);
    }

    return obj;
}

GameObject* GameObject::create(std::string_view texture) {
    return ax::utils::createInstance<GameObject>(&GameObject::init, texture);
}

void GameObject::setRotation(float rot) {
    Sprite::setRotation(rot);
    m_rotated = (std::fabs(rot) == 90 || std::fabs(rot) == 270);

    if (m_glowSprite) {
        m_glowSprite->setRotation(rot);
    }
}

void GameObject::activateObject() {
    m_shouldHide = false;

    if (m_active) {
        return;
    }

    m_active = true;

    if (m_isInvisible) {
        return;
    }
    
    setVisible(true);

    if (m_isOrb) {
        //TODO: PlayLayer::registerStateObject
    }

    if (m_objectParent) {
        m_objectParent->addChild(this, m_objectZ);
    }

    if (m_hasGlow) {
        ax::SpriteBatchNode* batchNode = State::getInstance()->getPlayLayer()->getBatchNodeAdd();
        batchNode->addChild(m_glowSprite);
    }
}

void GameObject::deactivateObject() {
    if (!m_shouldHide) {
        m_shouldHide = true;
        return;
    }

    if (!m_active) {
        return;
    }

    m_active = false;

    setVisible(false);

    //TODO:PlayLayer::unregisterStateObject

    // removeFromParentAndCleanup crashes?
    if (this->getParent())
        if (this->getParent()->getChildren().contains(this))
            this->getParent()->removeChild(this, false);

    if (m_hasGlow && m_glowSprite->getParent()) {
        m_glowSprite->removeFromParentAndCleanup(false);
    }
}

bool GameObject::getShouldSpawn() {
    return m_shouldSpawn;
}

float GameObject::getSpawnXPos() {
    return m_spawnXPos;
}

void GameObject::triggerObject() {
    PlayScene* playLayer = State::getInstance()->getPlayLayer();

    switch (m_objectKey)
    {
    case 22:
        playLayer->setActiveEnterEffect(1);
        break;
    case 23:
        playLayer->setActiveEnterEffect(5);
        break;
    case 24:
        playLayer->setActiveEnterEffect(4);
        break;
    case 25:
        playLayer->setActiveEnterEffect(6);
        break;
    case 26:
        playLayer->setActiveEnterEffect(7);
        break;
    case 27:
        playLayer->setActiveEnterEffect(2);
        break;
    case 28:
        playLayer->setActiveEnterEffect(3);
        break;
    case 29:
        playLayer->tintBackground(m_tintColor, m_tintDuration);
        break;
    case 30:
        playLayer->tintGround(m_tintColor, m_tintDuration);
        break;
    case 32:
        /* v22 = GameManager::sharedState((GameManager*)this);
        v23  = ((int(__fastcall*)(GameManager*))v22->getPlayLayer)(v22);
        v24  = (PlayerObject*)(*(int(__fastcall**)(int))(*(_DWORD*)v23 + 352))(v23);
        this = PlayerObject::toggleGhostEffect(v24, 1);
        */
        break;
    case 33:
        /*
        v25  = GameManager::sharedState((GameManager*)this);
        v26  = ((int(__fastcall*)(GameManager*))v25->getPlayLayer)(v25);
        v27  = (PlayerObject*)(*(int(__fastcall**)(int))(*(_DWORD*)v26 + 352))(v26);
        this = PlayerObject::toggleGhostEffect(v27, 0);*/
        break;
    case 42:
        /*
        v28  = GameManager::sharedState((GameManager*)this);
        v29  = (PlayLayer*)((int(__fastcall*)(GameManager*))v28->getPlayLayer)(v28);
        this = (GameObject*)PlayLayer::toggleAudioRain(v29, 1);*/
        break;
    case 43:
        /*
        v30  = GameManager::sharedState((GameManager*)this);
        v31  = (PlayLayer*)((int(__fastcall*)(GameManager*))v30->getPlayLayer)(v30);
        this = (GameObject*)PlayLayer::toggleAudioRain(v31, 0);
        */
        break;
    case 55:
        playLayer->setActiveEnterEffect(10);
        break;
    case 56:
        playLayer->setActiveEnterEffect(9);
        break;
    case 57:
        playLayer->setActiveEnterEffect(8);
        break;
    case 58:
        playLayer->setActiveEnterEffect(11);
        break;
    case 59:
        playLayer->setActiveEnterEffect(12);
        break;
    }
    
}

void GameObject::setTintColor(ax::Color3B color) {
    m_tintColor = color;
}

void GameObject::setTintDuration(float t) {
    m_tintDuration = t;
}

ax::ParticleSystemQuad* GameObject::createAndAddParticle(
    int type, const char* plist, int unk, ax::ParticleSystem::PositionType posType)
{
    PlayScene* playLayer = State::getInstance()->getPlayLayer();
    ax::ParticleSystemQuad* ret = playLayer->createParticle(type, plist, unk, posType);
    m_particleKey   = playLayer->getParticleKey(type, plist, unk, posType);
    m_addedParticle = true;

    return ret;
}

void GameObject::setVisible(bool visible)
{
    m_shouldHide = true;

    if (m_addedParticle) {
        if (isVisible() != visible) {
            if (m_particleSystem) {
                m_particleSystem->setVisible(visible);
                m_particleSystem->resetSystem();
            } else {
                auto state = State::getInstance();
                auto pl    = state->getPlayLayer();

                m_particleSystem = pl->claimParticle(m_particleKey);
                this->setPosition(getPosition());

                if (m_particleSystem) {
                    m_particleSystem->setScaleX((isFlippedX()) ? -1 : 1);
                    m_particleSystem->setScaleY((isFlippedY()) ? -1 : 1);

                    m_particleSystem->setRotation(getRotation());

                    m_particleSystem->setVisible(visible);
                    m_particleSystem->resetSystem();
                }
            }
        } else {
            auto state      = State::getInstance();
            auto pl         = state->getPlayLayer();
            pl->unclaimParticle(m_particleKey.c_str(), m_particleSystem);
            m_particleSystem = nullptr;
        }
    }

    Sprite::setVisible(visible);

    if (m_glowSprite) {
        m_glowSprite->setVisible(visible);
    }
}

void GameObject::setPosition(const ax::Vec2& pos) {
    Sprite::setPosition(pos);
    
    if (PlayScene* playLayer = State::getInstance()->getPlayLayer(); playLayer && m_particleSystem) {
        ax::Layer* gameLayer = playLayer->getGamelayer();
        ax::Point positionInGameLayerSpace = gameLayer->convertToNodeSpace(
            this->convertToWorldSpace(this->getTextureRect().size / 2)
        );
        
        m_particleSystem->setPosition(positionInGameLayerSpace);
    }

    if (m_glowSprite) {
        m_glowSprite->setPosition(pos);
    }
}

void GameObject::addGlow() {
    switch (m_objectKey) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 6:
        case 7:
        case 8:
        case 35:
        case 39:
        case 40:
        case 44:
        case 62:
        case 63:
        case 64:
        case 65:
        case 66:
        case 68:
        case 69:
        case 70:
        case 71:
        case 72:
        case 74:
        case 75:
        case 76:
        case 77:
        case 78:
        case 79:
        case 81:
        case 82:
            m_hasGlow = true;
            m_glowSprite = Sprite::createWithSpriteFrameName(
                m_frame.substr(0, m_frame.find("_001.png")).append("_glow_001.png"));
            
            if (m_glowSprite) {
                m_glowSprite->retain();
                m_glowSprite->setPosition(getPosition());
                m_glowSprite->setOpacity(255);
            }
            break;
        default:
            break;
    }
}

void GameObject::resetObject() {
    m_hasBeenActivated = false;
}

void GameObject::setScale(float scale) {
    Sprite::setScale(scale);

    if (m_glowSprite) {
        m_glowSprite->setScale(scale);
    }
}

void GameObject::setScale(float x, float y) {
    Sprite::setScale(x, y);

    if (m_glowSprite) {
        m_glowSprite->setScale(x, y);
    }
}

void GameObject::setScaleX(float scale) {
    Sprite::setScaleX(scale);

    if (m_glowSprite) {
        m_glowSprite->setScaleX(scale);
    }
}

void GameObject::setScaleY(float scale) {
    Sprite::setScaleY(scale);

    if (m_glowSprite) {
        m_glowSprite->setScaleY(scale);
    }
}

void GameObject::setFlippedX(bool flipped) {
    Sprite::setFlippedX(flipped);

    if (m_glowSprite) {
        m_glowSprite->setFlippedX(flipped);
    }
}

void GameObject::setFlippedY(bool flipped) {
    Sprite::setFlippedY(flipped);

    if (m_glowSprite) {
        m_glowSprite->setFlippedY(flipped);
    }
}

void GameObject::setOpacity(uint8_t opacity) {
    Sprite::setOpacity(opacity);

    if (m_glowSprite) {
        m_glowSprite->setOpacity(opacity);
    }

    if (!m_particleSystem) {
        return;
    }

    if (opacity > 50) {
        if (isVisible() && !m_particleSystem->isActive()) {
            m_particleSystem->resetSystem();
        }
    } else {
        m_particleSystem->stopSystem();
    }
}

bool GameObject::init(std::string_view texture) {
    if (!Sprite::initWithSpriteFrameName(texture)) {
        return false;
    }

    m_frame    = texture;
    m_size     = this->getContentSize();
    m_scaleMod = {1.0, 1.0};
    m_startScale = {1.01, 1.01};

    this->setScale(1.01f);

    return true;
}

void GameObject::calculateSpawnXPos() {
    m_spawnXPos = m_startPosition.x;
}

void GameObject::customSetup() {
#define CREATE_PARTICLE(plist, tag) createAndAddParticle(static_cast<int>(m_type), plist, tag,\
ax::ParticleSystem::PositionType::GROUPED)

    switch (m_objectKey) {
        case 5:
        case 73:
        case 80:
            m_type = GameObjectType::UnknownType;
            m_objectZ = -2;
            m_disabled = true;
            break;
        case 8:
            m_size = {30, 30};
            [[fallthrough]];
        case 39:
            m_type    = GameObjectType::Hazard;
            m_scaleMod   = {0.2f, 0.4f};
            break;
        case 9:
        case 61:
            m_type              = GameObjectType::Hazard;
            m_scaleMod          = {0.4f, 0.3f};
            break;
        case 10:
            m_type = GameObjectType::NormalGravityPortal;
            m_objectZ = 10;
            CREATE_PARTICLE("portalEffect01.plist", 3);
            break;
        case 11:
            m_type  = GameObjectType::InvertGravityPortal;
            m_objectZ         = 10;
            CREATE_PARTICLE("portalEffect02.plist", 3);
            break;
        case 12:
            m_type            = GameObjectType::CubePortal;
            m_objectZ         = 10;
            CREATE_PARTICLE("portalEffect03.plist", 3);
            break;
        case 13:
            m_type            = GameObjectType::ShipPortal;
            m_objectZ         = 10;
            CREATE_PARTICLE("portalEffect04.plist", 3);
            break;
        case 15:
        case 16:
        case 17:
            m_type  = GameObjectType::UnknownType;
            m_objectZ = -1;
            m_disabled = true;
            break;
        case 18:
        case 19:
        case 20:
        case 21:
            m_blendAdditive = 1;
            m_usePCol1      = true;
            m_type          = GameObjectType::UnknownType;
            m_objectZ       = 0;
            m_disabled      = true;
            break;
        case 35:
            m_type    = GameObjectType::YellowPad;
            m_scaleMod          = {1.0, 1.0};
            CREATE_PARTICLE("bumpEffect.plist", 0);
            break;
        case 36:
            m_type        = GameObjectType::YellowOrb;
            m_isOrb                 = true;
            m_useAudioScale         = true;
            m_scaleMod              = {1.2, 1.2};
            CREATE_PARTICLE("ringEffect.plist", 3);
            break;
        case 37:
            m_usePCol1 = true;
            m_type = GameObjectType::UnknownType2;
            m_useAudioScale = true;
            m_disabled      = true;
            m_size          = {30, 30};
            break;
        case 38:
            m_type = GameObjectType::UnknownType;
            m_disabled       = true;
            break;
        case 41:
            m_usePCol1               = true;
            m_type                   = GameObjectType::UnknownType;
            m_blendAdditive  = true;
            m_disabled               = true;
            break;
        case 44:
            m_type               = GameObjectType::UnknownType;
            m_objectZ            = 2;
            m_disabled           = true;
            break;
        case 45:
            m_type = GameObjectType::MirrorPortal;
            m_objectZ = 10;

            if (ax::ParticleSystemQuad* particle = CREATE_PARTICLE("portalEffect02.plist", 3)) {
                particle->setStartColor({1.0f, 150.0f / 255, 0.0f, 1.0f});
                particle->setEndColor({1.0f, 150.0f / 255, 0.0f, 1.0f});
            }

            break;
        case 46:
            m_type  = GameObjectType::CounterMirrorPortal;
            m_objectZ = 10;
            CREATE_PARTICLE("portalEffect01.plist", 3);
            break;
        case 47:
            m_type = GameObjectType::BallPortal;
            m_objectZ = 10;

            if (ax::ParticleSystemQuad* particle = CREATE_PARTICLE("portalEffect02.plist", 3)) {
                particle->setStartColor({1.0f, 100.0f / 255, 0.0f, 1.0f});
                particle->setEndColor({1.0f, 100.0f / 255, 0.0f, 1.0f});
            }

            break;
        case 48:
        case 49:
            m_blendAdditive = true;
            m_type          = GameObjectType::UnknownType;
            m_objectZ       = 0;
            m_disabled      = true;
            m_usePCol2      = true;
            break;

        case 50:
        case 51:
        case 52:
        case 53:
        case 54:
        case 60:
            m_usePCol1 = 1;
            m_type = GameObjectType::UnknownType2;
            m_useAudioScale   = true;
            m_blendAdditive = true;
            m_disabled        = true;
            m_size            = {30, 30};
            break;
        case 67:
            m_type = GameObjectType::GravityPad;
            m_scaleMod          = {1, 1};

            if (ax::ParticleSystemQuad* particle = CREATE_PARTICLE("bumpEffect.plist", 0)) {
                particle->setStartColor({0.f, 1.f, 1.f, 1.f});
                particle->setEndColor({0.f, 1.f, 1.f, 1.f});
            }

            break;
        case 84:
            m_type = GameObjectType::BlueOrb;
            m_isOrb= 1;
            m_useAudioScale= 1;
            m_scaleMod              = {1.2, 1.2};
            
            if (ax::ParticleSystemQuad* particle = CREATE_PARTICLE("ringEffect.plist", 3)) {
                particle->setStartColor({0.f, 1.f, 1.f, 1.f});
                particle->setEndColor({0.f, 1.f, 1.f, 1.f});
            }

            break;
        default:
            m_type = GameObjectType::None;

            if (m_frame.starts_with("edit_e")) {
                m_type        = GameObjectType::UnknownType;
                m_shouldSpawn = true;
                m_disabled    = true;
                m_isInvisible = true;
            }
    };

    return;
#undef CREATE_PARTICLE
}

ax::Rect GameObject::getObjectRect(ax::Vec2 scale) const {
    ax::Size size(m_size * scale);
    ax::Vec2 position = getRealPosition();

    if (m_rotated) {
        std::swap(size.width, size.height);
    }
    
    position += size * -0.5f;

    return ax::Rect(position, size);
}
