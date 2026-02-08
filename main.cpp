#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cctype>

#include "instructions.hpp"
#include "cpu.hpp"
#include "ansi.h"

#include "ias-lexer-parser.hpp"

std::vector<IAS::IASWord>* parseProgram(std::istream& in) {
    std::vector<IAS::IASWord>* program = new std::vector<IAS::IASWord>;

    std::string line;
    uint16_t lineno = 1;
    while (std::getline(in, line)) {
        try {
            IAS::IASLineParser parser(line);
            IAS::IASWord word = parser.line();
            program->push_back(word);
        } catch (const std::exception& e) {
            delete program;
            std::cerr
                << ANSI::Color::RED
                << "Error parsing line "
                << lineno << ": " << e.what()
                << ANSI::Color::RESET
                << std::endl;
            exit(1);
        }
        lineno++;
    }

    // Pad with zeros if program is less than MEMORY_SIZE
    while (program->size() < MEMORY_SIZE) {
        program->push_back(IAS::IASWord(0));
    }

    return program;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <program_file> [--debug {CLI|IPC}]" << std::endl;
        return 1;
    }

    bool isDebugMode = (argc >= 3 && std::string(argv[2]) == "--debug");
    IAS::DebugMode mode = IAS::DebugMode::CLI;
        
    if (isDebugMode) {
        if (argc >= 4) {
            std::string modeStr = std::string(argv[3]);
            if (modeStr == "IPC") {
                mode = IAS::DebugMode::IPC;
            } else if (modeStr == "CLI") {
                mode = IAS::DebugMode::CLI;
            } else {
                std::cerr << "Invalid debug mode: " << modeStr << ". Use CLI or IPC." << std::endl;
                return 1;
            }
        }
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Unable to open file" << std::endl;
        return 1;
    }

    std::vector<IAS::IASWord>* program = parseProgram(file);

    IAS::IASCPU cpu = IAS::IASCPU();
    cpu.loadProgram(program->data(), program->size());

    if (isDebugMode) {
        cpu.debug(mode);
    } else {
        cpu.run();
    }
    
    delete program;
    return 0;
}
