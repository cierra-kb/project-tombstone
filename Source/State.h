#pragma once

class PlayScene;

class State
{
public:
    static State* getInstance() {
        static State singleton;
        return &singleton;
    }

    void setPlayLayer(PlayScene* pl) {
        m_playScene = pl;
    }
    PlayScene* getPlayLayer() const {
        return m_playScene;
    }

private:
    PlayScene* m_playScene;
};
