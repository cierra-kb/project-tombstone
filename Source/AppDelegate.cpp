/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 Copyright (c) 2019-present Axmol Engine contributors (see AUTHORS.md).

 https://axmol.dev/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "AppDelegate.h"
#include "Managers/AssetManager.h"
#include "Scenes/LoadingLayer.h"

#include <base/Director.h>
#include <platform/GLView.h>
#include <platform/GLViewImpl.h>

#define USE_AUDIO_ENGINE 1

#if USE_AUDIO_ENGINE
#    include "audio/AudioEngine.h"
#endif

using namespace ax;

static ax::Size designResolutionSize = ax::Size(480, 320);

AppDelegate::AppDelegate() {}

AppDelegate::~AppDelegate() {}

// if you want a different context, modify the value of glContextAttrs
// it will affect all platforms
void AppDelegate::initGLContextAttrs() {
    // set OpenGL context attributes: red,green,blue,alpha,depth,stencil,multisamplesCount
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0};
    // since axmol-2.2 vsync was enabled in engine by default
    glContextAttrs.vsync = false;

    GLView::setGLContextAttrs(glContextAttrs);
}

bool AppDelegate::applicationDidFinishLaunching() {
    ax::Director* director     = Director::getInstance();
    ax::GLView*   glView       = director->getGLView();
    AssetManager* assetManager = AssetManager::getInstance();

    if (glView == nullptr) {
#ifdef AX_PLATFORM_PC
        glView = GLViewImpl::createWithRect(
            "project-tombstone",
            ax::Rect(ax::Vec2::ZERO, {1280, 720}),
            1.0, false
        );
#else
        glView = GLViewImpl::create("project-tombstone");
#endif
        director->setGLView(glView);

        ((GLViewImpl*)glView)->setFullscreen();
    }

    director->setStatsDisplay(true);

#ifdef AX_PLATFORM_PC
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    director->setAnimationInterval(1.0f / mode->refreshRate);
#endif

    // Set the design resolution
    ax::Vec2 screenRatio = glView->getFrameSize() / designResolutionSize;
    ResolutionPolicy resolutionPolicy = (screenRatio.width > screenRatio.height)
        ? ResolutionPolicy::FIXED_HEIGHT : ResolutionPolicy::FIXED_WIDTH;

    glView->setDesignResolutionSize(
        designResolutionSize.width,
        designResolutionSize.height,
        resolutionPolicy
    );

    director->setContentScaleFactor(assetManager->getAppropriateScaleFactor());

    director->runWithScene(utils::createInstance<LoadingLayer>());

    return true;
}

// This function will be called when the app is inactive. Note, when receiving a phone call it is invoked.
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::pauseAll();
#endif
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::resumeAll();
#endif
}
