#include "LoadingLayer.h"

#include "Extensions/DirectorExt.h"
#include "Managers/AssetManager.h"
#include "Scenes/MenuScene.h"

#include <2d/Sprite.h>
#include <2d/ActionInstant.h>
#include <2d/ActionInterval.h>
#include <2d/Label.h>
#include <base/Utils.h>

bool LoadingLayer::init() {
    if (!Scene::init()) {
        return false;
    }

    DirectorExt* director                  = DirectorExt::getInstance();
    ax::Size winSize                       = director->getWinSize();
    AssetManager* assetManager             = AssetManager::getInstance();

    assetManager->addSpriteFramesWithFile("GJ_LaunchSheet.plist");

    ax::Sprite* bgSprite = assetManager->createSprite("game_bg_01_001.png");
    bgSprite->setPosition({winSize.width / 2, winSize.height / 2});
    bgSprite->setScale(director->getContentScaleFactorMax() / director->getContentScaleFactor());
    bgSprite->setColor({45, 125, 255});
    this->addChild(bgSprite);

    ax::Sprite* logoSprite = assetManager->createSpriteWithFrameName("GJ_logo_001.png");
    logoSprite->setPosition({winSize.width / 2, winSize.height / 2});
    this->addChild(logoSprite);

    ax::Sprite* robtopLogoSprite = assetManager->createSpriteWithFrameName("RobTopLogoBig_001.png");
    robtopLogoSprite->setPosition(logoSprite->getPosition() + ax::Vec2(0, 80));
    this->addChild(robtopLogoSprite);

    ax::Label* splashLabel = ax::Label::createWithBMFont(
        assetManager->getForwardedFileName("goldFont.fnt"),
        "Unlock new icons and colors by completing achievements!",
        ax::TextHAlignment::CENTER);
    splashLabel->setMaxLineWidth(440.f);
    splashLabel->setLineBreakWithoutSpace(true);
    splashLabel->setPosition({winSize.width / 2, (winSize.height / 2) - 100.0f});

    if (float sc = 300 / splashLabel->getContentSize().width; sc <= 1) {
        splashLabel->setScale(sc);
    }
    this->addChild(splashLabel);

    ax::Sprite* sliderGrooveSprite = assetManager->createSprite("slidergroove.png");
    sliderGrooveSprite->setPosition(splashLabel->getPosition() + ax::Vec2(0, 40));

    ax::Sprite* sliderBarSprite = assetManager->createSprite("sliderBar.png");
    sliderBarSprite->setPosition({2, 4});
    sliderBarSprite->setAnchorPoint({0, 0});
    sliderBarSprite->getTexture()->setTexParameters(
        ax::Texture2D::TexParams(
            ax::backend::SamplerFilter::LINEAR, ax::backend::SamplerFilter::LINEAR,
            ax::backend::SamplerAddressMode::REPEAT, ax::backend::SamplerAddressMode::CLAMP_TO_EDGE
        )
    );
    sliderGrooveSprite->addChild(sliderBarSprite, -1);

    this->addChild(sliderGrooveSprite);

    m_sliderGrooveSprite = sliderGrooveSprite;
    m_sliderBarSprite    = sliderBarSprite;

    this->setBarProgress(0);

    return true;
}

void LoadingLayer::onEnterTransitionDidFinish() {
    m_currentLoadBatchId                = 0;
    AssetManager* assetManager = AssetManager::getInstance();

    m_loadBatches = {
        [assetManager]() {
            assetManager->addSpriteFramesWithFile("GJ_GameSheet.plist");
        },
        [assetManager]() {
            assetManager->addSpriteFramesWithFile("CCControlColourPickerSpriteSheet.plist");
        },
        [assetManager]() {
            assetManager->addTextureToCache("gravityOverlay.png");
        }};

    ax::Action* act = ax::Sequence::create({
        ax::DelayTime::create(1.0),
        ax::CallFunc::create([this]() {
            this->loadAssets();
        })
    });
    this->runAction(act);
}

void LoadingLayer::setBarProgress(float f) {
    float maxSliderWidth = m_sliderGrooveSprite->getContentSize().width - 4;

    m_sliderBarSprite->setTextureRect({m_sliderBarSprite->getTextureRect().origin,
                                      ax::Size(maxSliderWidth * f, m_sliderBarSprite->getTextureRect().size.height)});
}

void LoadingLayer::loadAssets() {
    if (m_currentLoadBatchId >= m_loadBatches.size()) {
        return assetsLoaded();
    }

    m_loadBatches[m_currentLoadBatchId]();

    ax::Action* act = ax::Sequence::create({
        ax::DelayTime::create(0.1f),
        ax::CallFunc::create([this]() {
            loadAssets();
        })
    });
    this->runAction(act);

    m_currentLoadBatchId++;
    this->setBarProgress((float)m_currentLoadBatchId / (float)m_loadBatches.size());
}

void LoadingLayer::assetsLoaded() {
    Scene* nextScene = ax::utils::createInstance<MenuScene>(&MenuScene::init);
    ax::Director::getInstance()->replaceScene(nextScene);
}
