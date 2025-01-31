#include "Level.h"

Level* Level::create() {
    auto p = new Level();
    p->autorelease();
    return p;
}
