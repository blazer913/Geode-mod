#pragma once
#include "Timeline.hpp"
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace twa {

struct TTRLHeader {
    uint32_t magic = 0;
    uint8_t version = 0;
    uint8_t flags = 0;
    uint16_t headerSize = 0;
    uint32_t declaredSize = 0;
};

struct TTRLParseResult {
    bool ok = false;
    TTRLHeader header{};
    InputTimeline timeline;
    std::string error;
    size_t bytesConsumed = 0;
};

class TTRLReader {
public:
    static TTRLParseResult read(std::filesystem::path const& path);
    static TTRLParseResult readBytes(std::vector<uint8_t> const& bytes);
};

}
