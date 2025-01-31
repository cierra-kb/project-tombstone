#pragma once

#include <2d/Scene.h>
#include <2d/ActionTween.h>
#include <2d/ParticleSystem.h>
#include <Inspector/Inspector.h>

#include "Objects/GameObject.h" // not forward declared because of ax::Vector

namespace ax {
    class ParticleSystemQuad;
    class ParticleSystem;
    class Sprite;
    class SpriteBatchNode;
    class DrawNode;
}

class Level;
class LevelSettings;
class GroundLayer;
class PlayerObject;

class PlayScene : public ax::Scene, public ax::ActionTweenDelegate {
public:
    ~PlayScene();

    bool init(Level*);

    void update(float) override;
    void onEnter() override;
    void onExit() override;

    void updateTweenAction(float value, std::string_view key) override;

    void tintBackground(ax::Color3B color, float duration);
    void tintGround(ax::Color3B color, float duration);

    const std::string getParticleKey(
        int type, const char* plist, int unk, ax::ParticleSystem::PositionType positionType);
    const std::string getParticleKey2(const std::string key);

    ax::ParticleSystemQuad* createParticle(
        int type, const char* plist, int unk, ax::ParticleSystem::PositionType positionType);
    
    ax::ParticleSystemQuad* claimParticle(std::string key);
    void unclaimParticle(const char* key, ax::ParticleSystemQuad* particleSystem);

    void destroyPlayer();
    void delayedResetLevel();

    void setActiveEnterEffect(int effectId) {
        m_activeEnterEffect = effectId;
    }

    ax::Layer* getGamelayer() const {
        return m_gameLayer;
    }

    ax::SpriteBatchNode* getBatchNodeAdd() const {
        return m_additiveBatchNode;
    }

    ax::DrawNode* getDrawNode() const {
        return m_debugDrawNode;
    }

    ax::Vec2 getStartPos() const {
        return m_startPos;
    }
private:
    void setupKeybinds();
    void setupTouchControls();
private:
    void checkCollisions(float);
    int sectionForPos(ax::Vec2);
    void switchToFlyMode(GameObject* portal, bool instantCamera);
    void switchToRollMode(GameObject* object, bool instantCamera);
    void exitFlyMode();
    void exitRollMode();
    void updateCamera(float);
    void cameraMoveX(float, float, float);
    void cameraMoveY(float, float, float);
    void updateVisibility();
    void toggleFlipped(bool,bool);
    bool isFlipping() const;
    float getRelativeMod(ax::Vec2, float, float, float);
    void animateInFlyGround(bool);
    void animateOutFlyGround(bool);
    void createObjectsFromSetup(std::string);
    void addToSection(GameObject*);
    void resetLevel();
    void checkSpawnObjects();
    void applyEnterEffect(GameObject*);
    void startGame();
    void playGravityEffect(bool);
    void animateInRollGround(bool instant);
    void animateOutRollGround(bool instant);
    void animateOutRollGroundFinished();
private:
	int m_activeEnterEffect;

    std::unordered_map<std::string, ax::Vector<ax::ParticleSystemQuad*>> m_particleDictionary;

    LevelSettings* m_levelSettings;
    ax::Color3B m_activeBGColor;
    ax::Color3B m_activeGColor;

    ax::Sprite* m_bgSprite;

    /**
     * Scaled width of the background sprite.
     *
     * Offset (1.3): 0x17C
     */
    float m_backgroundWidth;
    
    float m_unk13c; ///< Offset (1.3): 0x13C
    bool m_isMovingCameraX;
    bool m_isMovingCameraY;

    bool m_isFlipped;

    /**
     * A progressive value ranging from `0` to `1` that represents the current state of the mirror effect.
     * `1` being completely mirrored, and `0` with no mirror effect applied.
     *
     * Offset (1.3): 0x220
     */
    float m_flipProgress;

    /**
     * `-1` when completely mirrored, `1` otherwise.
     *
     * Offset (1.3): 0x214
     */
    float m_flipScale; 
    
    int m_previousSection = 0; ///< Offset (1.3): 0x1C8
    int m_nextSection = 0; ///< Offset (1.3): 0x1CC

    float m_backgroundXPosOffset;

    float m_maxObjectXPos; ///< Offset (1.3): 0x1DC
    float m_levelSize;
    
    ax::Layer* m_gameLayer;

    std::vector<ax::SpriteBatchNode*> m_batchNodes;
    ax::SpriteBatchNode* m_additiveBatchNode;
    ax::SpriteBatchNode* m_batchNode;

    std::vector<ax::Vector<GameObject*>> m_sections; ///< Offset (1.3): 0x184
    ax::Vector<GameObject*> m_hazards; ///< Offset (1.3): 0x188
    ax::Vector<GameObject*> m_objects; ///< Offset (1.3): 0x198
    ax::Vector<GameObject*> m_spawnObjects; ///< Offset (1.3): 0x190
    ax::Vector<GameObject*> m_spawnQueue; ///< All queued objects to be spawned. See `resetLevel` and `checkSpawnObjects`.
    
    ax::Vec2 m_cameraPos = ax::Vec2::ZERO;
    bool m_firstStart = true; //< Offset (1.3): 0x1C1

    struct {
        float top; //< Offset (1.3): 0x148
        float bottom; //< Offset (1.3): 0x144
    } m_gameModeGroundPos; ///< Real (Non-Visual) position for top and bottom grounds for ship and ball.

    struct {
        struct GJFlyGroundDetail {
            ax::Sprite* groundSprite;
            ax::Sprite* floorLine;
        };
    
        ax::Layer* top; //< Offset (1.3): 0x178
        GJFlyGroundDetail topDetail;

        ax::Layer* bottom; //< Offset (1.3): 0x174
        GJFlyGroundDetail bottomDetail;
    } m_flyGround;

    
    struct {
        float bottom; //< Offset (1.3): 0x1BC
        float top; //< Offset (1.3): 0x1B8
    } m_flyGroundVisualPos {0, 0}; ///< Visual position of top and bottom grounds for ship.

    bool m_flyGroundActive;
   
    struct {
        float top;
        float bottom;
    } m_rollGroundVisualPos {0, 0};  ///< Visual position of top and bottom grounds for ball.

    struct {
        GroundLayer* top;
        GroundLayer* bottom;
    } m_rollGround {nullptr, nullptr}; ///< Grounds used when in roll mode.

    bool m_rollGroundActive = false;

    GroundLayer* m_regularGround = nullptr;
    PlayerObject* m_player = nullptr;
    ax::SpriteBatchNode* m_playerBatchNode = nullptr;
    ax::Vec2 m_lastPlayerPos = ax::Vec2::ZERO;
    ax::Vec2 m_startPos = {0, 105};

    bool m_onLevelEndAnimation = false;
    bool m_resetQueued = false;
    bool m_cleanReset = false;

    Level* m_level = nullptr;

    bool m_testMode = true;

    std::vector<GameObject*> m_debugDrawObjects;
    ax::DrawNode* m_debugDrawNode = nullptr;
};
