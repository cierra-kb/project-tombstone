#include "LevelSettings.h"
#include "Utils/SplitString.inl.h"

LevelSettings* LevelSettings::objectFromString(std::string_view str) {
    auto settings = new LevelSettings();
    settings->autorelease();

    auto arr = split_string::split(str, ",");
    std::string key;

    for (auto& item : arr) {
        if (key.empty()) {
            key = item;
            continue;
        }

        if (key == "kS1") {
            settings->m_backgroundColor.r = std::stoi(item);
        }
        else if (key == "kS2") {
            settings->m_backgroundColor.g = std::stoi(item);
        }
        else if (key == "kS3") {
            settings->m_backgroundColor.b = std::stoi(item);
        }
        else if (key == "kS4") {
            settings->m_groundColor.r = std::stoi(item);
        }
        else if (key == "kS5") {
            settings->m_groundColor.g = std::stoi(item);
        }
        else if (key == "kS6") {
            settings->m_groundColor.b = std::stoi(item);
        }
        else if (key == "kA1") {
            settings->m_soundtrackId = std::stoi(item);
        }

        key = "";
    }

    return settings;
}
