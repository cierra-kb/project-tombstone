#pragma once

#include <Director.h>

class DirectorExt : public ax::Director {
public:
    static DirectorExt* getInstance() {
        return static_cast<DirectorExt*>(ax::Director::getInstance());
    }

    float getContentScaleFactorMax() const {
        float scaleX = _glView->getScaleX();
        float scaleY = _glView->getScaleY();
        
        return (scaleX > scaleY) ? scaleX : scaleY;
    }
};
