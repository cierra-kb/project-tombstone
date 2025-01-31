#pragma once

#include <string_view>
#include <base/Object.h>
#include <math/Color.h>

class LevelSettings : public ax::Object {
    ax::Color3B m_backgroundColor;
    ax::Color3B m_groundColor;
    int m_soundtrackId;
public:
    static LevelSettings* objectFromString(std::string_view);

    ax::Color3B getStartBGColor() const { return m_backgroundColor; }
    ax::Color3B getStartGColor() const { return m_groundColor; }
    int getAudiotrack() const { return m_soundtrackId; }
};
