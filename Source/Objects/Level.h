#pragma once

#include <string>
#include <base/Object.h>

class Level : public ax::Object {
public:
    static Level* create();

    const std::string& getLevelData() const { return m_levelData; }
    void setLevelData(std::string_view sv) { m_levelData = sv; }
private:
    std::string m_levelData;
};
