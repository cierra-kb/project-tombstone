#include "GameManager.h"

#include <array>
#include <math/Color.h>

static const std::array<ax::Color3B, 13> playerColors = {
    ax::Color3B{0  , 255, 125},
    ax::Color3B{0  , 255, 0  },
    ax::Color3B{125, 255, 0  },
    ax::Color3B{255, 255, 0  },
    ax::Color3B{255, 125, 0  },
    ax::Color3B{255, 0  , 0  },
    ax::Color3B{255, 0  , 125},
    ax::Color3B{255, 0  , 255},
    ax::Color3B{125, 0  , 255},
    ax::Color3B{0  , 0  , 255},
    ax::Color3B{0  , 125, 255},
    ax::Color3B{0  , 255, 255},
    ax::Color3B{255, 255, 255}
};

const ax::Color3B& GameManager::colorForIdx(int id)
{
    if (id > playerColors.size() || id < 0)
        abort();

    return playerColors[id];
}
