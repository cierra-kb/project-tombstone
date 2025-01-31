#pragma once

namespace ax {
    class Color3B;
}

class GameManager {
public:
    static GameManager* singleton() {
        static GameManager singleton;
        return &singleton;
    }

    const ax::Color3B& colorForIdx(int id);

    int getPlayerColor() const {
        return m_playerColor;
    }
    int getPlayerColor2() const {
        return m_playerColor2;
    }
private:
    int m_playerColor = 0;
    int m_playerColor2 = 0;
};
