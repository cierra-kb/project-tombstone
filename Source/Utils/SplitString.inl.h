#pragma once

#include <functional>
#include <vector>
#include <string>

namespace split_string {
    inline std::pair<std::string, std::string>
    split_to_pair(std::string_view str, std::string_view delim) {
        auto delimPoint = str.find(delim);

        if (delimPoint == std::string::npos) {
            return std::make_pair(std::string(str), std::string());
        }

        return std::make_pair(
            std::string(str.substr(0, delimPoint)),
            std::string(str.substr(delimPoint + delim.size()))
        );
    }

    inline void split_streamed(std::string_view str,
                               std::string_view delim,
                               std::function<void(std::string_view)> callback)
    {
        std::vector<std::string> res;
        size_t pos = 0;
        size_t n   = 0;

        do {
            n = str.find(delim, pos);
            callback(str.substr(pos, n - pos));

            if (n == std::string::npos)
                break;

            pos = n + delim.size();
        } while (true);
    }

    inline std::vector<std::string>
    split(std::string_view str, std::string_view delim, bool once = false) {
        std::vector<std::string> res;
        size_t pos = 0;
        size_t n   = 0;

        if (once) {
            n = str.find(delim, pos);
            res.emplace_back(str.substr(pos, n - pos));
            pos = n + delim.size();
            res.emplace_back(str.substr(pos));
            return res;
        }
        
        do {
            n = str.find(delim, pos);
            res.emplace_back(str.substr(pos, n - pos));

            if (n == std::string::npos)
                break;

            pos = n + delim.size();
        } while (true);

        return res;
    }
}
