#pragma once

#include <bitset>
#include <stdexcept>
#include <cstdint>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cmath>

#include "cpu.hpp"
#include "instructions.hpp"

namespace Formatting
{
    inline std::string bitrepr(uint64_t value, int width)
    {
        if (width < 1 || width > 64)
            throw std::invalid_argument("Width must be between 1 and 64");

        // Mask to width
        uint64_t masked = (width == 64) ? value : (value & ((1ULL << width) - 1));

        // Use std::bitset
        return std::bitset<64>(masked).to_string().substr(64 - width);
    }

    inline std::string makeTimestampedFilename(const std::string& fileext = ".json")
    {
        std::time_t now = std::time(nullptr);
        std::tm local{};
    #if defined(_WIN32)
        localtime_s(&local, &now);
    #else
        localtime_r(&now, &local);
    #endif

        std::ostringstream oss;
        oss << "mem_snapshot_"
            << std::put_time(&local, "%Y-%m-%d_%H-%M-%S")
            << fileext;
        return oss.str();
    }

    inline std::string formatInstrPair(const IAS::IASWord& word)
    {
        try {
            auto L = word.decodeLeft();
            auto R = word.decodeRight();

            return L->toString() + " ; " + R->toString();
        } catch (const std::exception&) {
            return "INVALID ; INVALID";
        }
    }

} // namespace Formatting
