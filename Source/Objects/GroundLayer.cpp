#include "GroundLayer.h"
#include "Managers/AssetManager.h"
#include "Extensions/DirectorExt.h"

#include <2d/ActionInterval.h>
#include <Utils.h>

GroundLayer* GroundLayer::create() {
    return ax::utils::createInstance<GroundLayer>(&GroundLayer::init);
}

bool GroundLayer::init() {
    if (!Layer::initLayer()) {
        return false;
    }

    DirectorExt* director = DirectorExt::getInstance();
    ax::Size winSize          = director->getWinSize();
    AssetManager* assetManager = AssetManager::getInstance();

    m_sprite = assetManager->createSprite("groundSquare_001.png");
    m_sprite->getTexture()->setTexParameters({
        ax::backend::SamplerFilter::LINEAR,
        ax::backend::SamplerFilter::LINEAR,
        ax::backend::SamplerAddressMode::REPEAT,
        ax::backend::SamplerAddressMode::REPEAT
    });
    m_sprite->setAnchorPoint({0, 1});
    m_sprite->setScale(director->getContentScaleFactorMax() / director->getContentScaleFactor());

    {
        ax::Rect spriteTexRect = m_sprite->getTextureRect();

        m_width = spriteTexRect.size.width * m_sprite->getScale();
        spriteTexRect.size.width *= ceil(winSize.width / m_width) + 1;

        m_sprite->setTextureRect(spriteTexRect);
    }

    m_sprite->setPosition({0, 90});
    m_sprite->setColor({0, 103, 255});
    this->addChild(m_sprite, 2);

    ax::Sprite* floorLine = assetManager->createSpriteWithFrameName("floorLine_001.png");
    floorLine->setPosition({winSize.width / 2, 90});
    this->addChild(floorLine, 3);

    ax::Sprite* shadowLeft = assetManager->createSpriteWithFrameName("groundSquareShadow_001.png");
    shadowLeft->setAnchorPoint({0, 1});
    shadowLeft->setPosition({-1.0, 90});
    shadowLeft->setOpacity(100);
    shadowLeft->setScaleX(0.7f);
    this->addChild(shadowLeft, 3);

    ax::Sprite* shadowRight = assetManager->createSpriteWithFrameName("groundSquareShadow_001.png");
    shadowRight->setAnchorPoint({1, 1});
    shadowRight->setPosition({winSize.width + 1, 90});
    shadowRight->setOpacity(100);
    shadowRight->setScaleX(0.7f);
    shadowRight->setFlippedX(true);
    this->addChild(shadowRight, 3);

    this->setCascadeOpacityEnabled(true);

    return true;
}

void GroundLayer::showGround() {
    this->setOpacity(255);
}

void GroundLayer::fadeInGround(float t) {
    this->runAction(ax::FadeIn::create(t));
}

void GroundLayer::fadeOutGround(float t) {
    this->runAction(ax::FadeOut::create(t));
}
