#include "MenuScene.h"
#include "2d/MenuItem.h"
#include "Objects/Level.h"
#include "Scenes/PlayLayer.h"

#include <base/Utils.h>
#include <base/Director.h>
#include <2d/Sprite.h>
#include <2d/Menu.h>
#include <2d/TransitionProgress.h>
#include <math/Vec2.h>
#include <platform/FileUtils.h>

Level* createSampleLevel() {
    ax::FileUtils* const fileUtils = ax::FileUtils::getInstance();
    std::string data;

    fileUtils->getContents("tombstone/level.txt", &data);

    Level* level = Level::create();
    level->setLevelData(data);
    
    return level;
}

bool MenuScene::init() {
    if (!Scene::init()) {
        return false;
    }

    ax::Director* const  director = ax::Director::getInstance();
    const ax::Vec2& winSize = director->getWinSize();

    ax::Sprite* logo = ax::Sprite::createWithSpriteFrameName("GJ_logo_001.png");
    logo->setPosition({winSize.width / 2, winSize.height - 50});
    this->addChild(logo);

    ax::Menu* menu = ax::Menu::create();
    menu->setPosition({winSize.width / 2, (winSize.height / 2) + 10});

    ax::Sprite* playBtn = ax::Sprite::createWithSpriteFrameName("GJ_playBtn_001.png");
    menu->addChild(playBtn);

    ax::Sprite* garageBtn = ax::Sprite::createWithSpriteFrameName("GJ_garageBtn_001.png");
    garageBtn->setPosition(playBtn->getPosition() + ax::Vec2{-110, 0});
    menu->addChild(garageBtn);

    ax::Sprite* creatorBtnSpr = ax::Sprite::createWithSpriteFrameName("GJ_creatorBtn_001.png");
    ax::MenuItemSprite* creatorBtn = ax::MenuItemSprite::create(
        creatorBtnSpr, creatorBtnSpr, [director](Object*) -> void {
            director->pushScene(ax::TransitionFade::create(0.5,
                ax::utils::createInstance<PlayScene>(&PlayScene::init, ::createSampleLevel()))
            );
    });
    creatorBtn->setPosition(playBtn->getPosition() + ax::Vec2{110, 0});
    menu->addChild(creatorBtn);

    this->addChild(menu);

    return true;
}
