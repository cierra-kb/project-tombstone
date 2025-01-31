#pragma once

#include <2d/Layer.h>

namespace ax {
    class Sprite;
};

class GroundLayer : public ax::Layer {
public:
    static GroundLayer* create();
    bool init() override;
    void showGround();
    void fadeInGround(float t);
    void fadeOutGround(float t);
    ax::Sprite* getGroundSprite() const { return m_sprite; };
    float getGroundWidth() const { return m_width; }
private:
    ax::Sprite* m_sprite;
    float m_width;
};
