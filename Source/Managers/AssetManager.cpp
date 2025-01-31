#include "AssetManager.h"

#include <2d/Sprite.h>
#include <2d/SpriteFrameCache.h>
#include <renderer/TextureCache.h>

#include <FileUtils.h>
#include <Director.h>

ax::Sprite* AssetManager::createSprite(std::string_view path, bool forwardPath) {
    if (!forwardPath || !needTextureQualitySuffix()) {
        return ax::Sprite::create(path);
    }
    return ax::Sprite::create(getForwardedFileName(std::string(path)));
}

ax::Sprite* AssetManager::createSpriteWithFrameName(std::string_view frame) {
    return ax::Sprite::createWithSpriteFrameName(frame);
}

void AssetManager::addSpriteFramesWithFile(std::string_view path, bool forwardPath) {
    ax::SpriteFrameCache* spriteFrameCache = ax::SpriteFrameCache::getInstance();

    if (!forwardPath || !needTextureQualitySuffix()) {
        spriteFrameCache->addSpriteFramesWithFile(path);
    }

    spriteFrameCache->addSpriteFramesWithFile(
        getForwardedFileName(std::string(path))
    );
}

ax::Texture2D* AssetManager::addTextureToCache(std::string_view filePath, bool forwardPath) {
    auto textureCache = ax::Director::getInstance()->getTextureCache();

    if (!forwardPath || !needTextureQualitySuffix()) {
        textureCache->addImage(filePath);
    }

    return textureCache->addImage(
        getForwardedFileName(std::string(filePath))
    );
}

std::string AssetManager::getForwardedFileName(std::string path) {
    std::string suffixed = appendTextureQualitySuffix(path);

    if (doesFileExist(suffixed)) {
        return suffixed;
    }

    return path;
}

bool AssetManager::doesFileExist(std::string_view path) {
    return ax::FileUtils::getInstance()->isFileExist(path);
}

std::string& AssetManager::appendTextureQualitySuffix(std::string& str) const {
    if (!needTextureQualitySuffix()) {
        return str;
    }

    const char* suffix = getTextureQualitySuffix();
    
    auto lastPositionOfDot = str.find_last_of(".");
    bool stringHasNoDot = lastPositionOfDot == str.npos;

    if (stringHasNoDot) {
        str.append(suffix);
    } else {
        str.insert(lastPositionOfDot, suffix);
    }

    return str;
}
