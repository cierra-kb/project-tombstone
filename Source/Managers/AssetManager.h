#pragma once

#include <cstdint>
#include <string_view>

namespace ax {
    class Sprite;
    class Texture2D;
};

class AssetManager {
public:
    enum class TextureQuality : uint8_t {
        Low = 0,
        Medium = 1,
    };
public:
    static AssetManager* getInstance() {
        static AssetManager singleton;
        return &singleton;
    }

    float getAppropriateScaleFactor() const {
        return (m_textureQuality == TextureQuality::Medium) ? 2.0 : 1.0;
    }

    ax::Sprite* createSprite(std::string_view filePath, bool forwardPath = true);
    ax::Sprite* createSpriteWithFrameName(std::string_view frame);
    void addSpriteFramesWithFile(std::string_view filePath, bool forwardPath = true);
    ax::Texture2D* addTextureToCache(std::string_view filePath, bool forwardPath = true);

    std::string getForwardedFileName(std::string filePath);
    bool doesFileExist(std::string_view path);
private:
    bool needTextureQualitySuffix() const {
        return m_textureQuality != TextureQuality::Low;
    }

    const char* getTextureQualitySuffix() const {
        return (m_textureQuality == TextureQuality::Medium) ? "-hd" : "";
    }

    std::string& appendTextureQualitySuffix(std::string& str) const;

private:
    TextureQuality m_textureQuality = TextureQuality::Medium;
};
