#pragma once

#include <cstdint>
#include <string_view>

#include <2d/ParticleSystemQuad.h>
#include <2d/Sprite.h>

enum class GameObjectType : int32_t {
    None                = 0,
    Hazard              = 2,
    InvertGravityPortal = 3,
    NormalGravityPortal = 4,
    ShipPortal          = 5,
    CubePortal          = 6,
    UnknownType         = 7,
    UnknownType2        = 8,
    YellowPad           = 9,
    GravityPad          = 10,
    YellowOrb           = 11,
    BlueOrb             = 12,
    MirrorPortal        = 13,
    CounterMirrorPortal = 14,
    BallPortal          = 15
};

class GameObject : public ax::Sprite {
public:
    ~GameObject();

    static GameObject* createFromString(std::string_view);
    static GameObject* create(std::string_view);

    void setRotation(float) override;

    ax::Rect getObjectRect() const { return getObjectRect(m_scaleMod); };
    ax::Rect getObjectRect(ax::Vec2 scale) const;

    virtual ax::Vec2 getRealPosition() const { return m_startPosition; };
    ax::Vec2 getStartPosition() const { return m_startPosition; };

    int getObjectKey() const { return m_objectKey; };
    GameObjectType getType() const { return m_type; }
    bool getIsDisabled() const { return m_disabled; }
    bool getHasBeenActivated() const { return m_hasBeenActivated; }
    virtual void triggerActivated() { m_hasBeenActivated = true; }
    bool getDontTransform() const { return m_dontTransform; }
    bool getUseAudioScale() const { return m_useAudioScale; }
    bool getBlendAdditive() const { return m_blendAdditive; }
    void setObjectParent(ax::Node* node) { m_objectParent = node; }
    bool getShouldSpawn() const { return m_shouldSpawn; }
    void calculateSpawnXPos();
    std::string getFrame() const { return m_frame; }
    void setObjectKey(int id) { m_objectKey = id; };

    void setStartPosition(ax::Vec2 position)
    {
        m_startPosition = position;
        setPosition(position);
    };

    void customSetup();
    void setObjectZ(int z) { m_objectZ = z; }
    void setStartRotation(float rot) { m_startRotation = rot; }
    void setSectionIdx(int idx) { m_sectionIdx = idx; }
    void activateObject();
    void deactivateObject();
    bool getShouldSpawn();
    float getSpawnXPos();
    virtual void triggerObject();
    void setTintColor(ax::Color3B);
    void setTintDuration(float);
    void setEnterEffect(int effectIdx) { m_enterEffect = effectIdx; }
    int getEnterEffect() const { return m_enterEffect; }

    float getStartScaleX() const { return m_startScale.x; }
    float getStartScaleY() const { return m_startScale.y; }

    // NOTE: This doesn't exist in the original code but
    // it makes things easier.
    ax::Vec2 getStartScale() const { return m_startScale; }

    void setEnterAngle(float aangle) { m_enterAngle = aangle; }
    float getEnterAngle() const { return m_enterAngle; }
    bool getUsePlayerColor() const { return m_usePCol1; }
    bool getUsePlayerColor2() const { return m_usePCol2; }

    ax::ParticleSystemQuad* createAndAddParticle(int type,
                                                 const char* plist,
                                                 int unk,
                                                 ax::ParticleSystem::PositionType posType);
    void setVisible(bool) override;
    void setPosition(const ax::Vec2& pos) override;
    void addGlow();
    virtual void resetObject();
    void setScale(float) override;
    void setScale(float, float) override;
    void setScaleX(float) override;
    void setScaleY(float) override;
    
    // note: unlike in the actual code, this is not a virtual function
    void setFlippedX(bool);

    // note: unlike in the actual code, this is not a virtual function
    void setFlippedY(bool);

    void setOpacity(uint8_t opacity) override;

protected:
    bool init(std::string_view texture);

private:
    int m_objectKey; ///< The object or block id.

    /**
     * The "Real" position of the object. This is the position/origin of the hitbox rect.
     * This is also unaffected by visual changes such as the 'flip' effect caused by mirror portals, hence the "Real" part.
     */
    ax::Vec2 m_startPosition;

    ax::Vec2 m_scaleMod; ///< Scales the hitbox size

    /**
     * The (content) size of the object. This is the raw/unscaled size of the hitbox rect.
     */
    ax::Size m_size;

    /**
     * Initial scale of the object. Needed by enter effects to keep track of the original scale.
     */
    ax::Vec2 m_startScale;

    bool m_rotated; ///< States if the object is rotated sideways (90 or 270 degrees).
    GameObjectType m_type; ///< The object's type.

    int m_objectZ;
    bool m_disabled;
    bool m_hasBeenActivated;
    bool m_blendAdditive;
    bool m_usePCol1;
    bool m_usePCol2;
    bool m_isOrb;
    bool m_useAudioScale;
    bool m_dontTransform;
    bool m_shouldHide;
    bool m_active;
    bool m_isInvisible;
    bool m_hasGlow;
    int m_sectionIdx;
    float m_startRotation;
    float m_spawnXPos;
    bool m_shouldSpawn;
    ax::Node* m_objectParent;
    std::string m_frame;
    ax::Color3B m_tintColor;
    float m_tintDuration;
    int m_enterEffect;
    float m_enterAngle;
    std::string m_particleKey;
    bool m_addedParticle;
    ax::ParticleSystemQuad* m_particleSystem;
    ax::Sprite* m_glowSprite;
};
