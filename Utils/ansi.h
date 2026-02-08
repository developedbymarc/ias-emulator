#pragma once

namespace ANSI {
    namespace Color
    {
        // Regular Colors
        inline auto BLACK = "\033[0;30m";
        inline auto RED = "\033[0;31m";
        inline auto GREEN = "\033[0;32m";
        inline auto YELLOW = "\033[0;33m";
        inline auto BLUE = "\033[0;34m";
        inline auto PURPLE = "\033[0;35m";
        inline auto CYAN = "\033[0;36m";
        inline auto WHITE = "\033[0;37m";

        // Background
        inline auto BG_BLACK = "\033[40m";
        inline auto BG_RED = "\033[41m";
        inline auto BG_GREEN = "\033[42m";
        inline auto BG_YELLOW = "\033[43m";
        inline auto BG_BLUE = "\033[44m";
        inline auto BG_PURPLE = "\033[45m";
        inline auto BG_CYAN = "\033[46m";
        inline auto BG_WHITE = "\033[47m";

        // Reset
        inline auto RESET = "\033[0m";
    } // namespace Colors

    namespace Cursor
    {
        inline auto UP = "\033[A";
        inline auto DOWN  = "\033[B";
        inline auto RIGHT = "\033[C";
        inline auto LEFT  = "\033[D";

        inline auto HIDE  = "\033[?25l";
        inline auto SHOW  = "\033[?25h";
    } // namespace Screen
    
    namespace Clear
    {
        inline auto LINE = "\033[2K";
        inline auto SCREEN = "\033[2J";
    } // namespace Clear
};
