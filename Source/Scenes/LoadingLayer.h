#pragma once

#include <2d/Scene.h>
#include <functional>

namespace ax {
    class Sprite;
};

class LoadingLayer : public ax::Scene {
public:
    bool init() override;
    void onEnterTransitionDidFinish() override;
private:
    void setBarProgress(float f);
    void loadAssets();
    void assetsLoaded();
private:
    ax::Sprite* m_sliderGrooveSprite;
    ax::Sprite* m_sliderBarSprite;

    std::vector<std::function<void()>> m_loadBatches;
    size_t m_currentLoadBatchId;
};
