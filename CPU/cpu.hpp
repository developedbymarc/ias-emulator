#pragma once

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "instructions.hpp"

#define MEMORY_SIZE 4096

namespace IAS {
    enum DebugMode {
        CLI,
        IPC
    };

    class IASCPU
    {
    public:
        IASCPU();
        void loadProgram(const IASWord *program, size_t size);
        void dumpMemory(uint16_t start, uint16_t number_of_lines, std::ostream& out);
        void prettyDumpMemory(uint16_t start, uint16_t number_of_lines, std::ostream& out);
        void run();
        void debug(DebugMode mode = DebugMode::CLI);

    private:
        int64_t AC;  // Accumulator
        int64_t MQ;  // Multiplier-Quotient
        uint16_t PC; // Program Counter

        IASInstructionType IR;  // Instruction Register
        uint16_t MAR; // Memory Address Register
        
        IASInstruction* IBR; // Instruction Buffer Register
        IASWord MBR; // Memory Buffer Register

        int64_t getMemoryValue(uint16_t address);
        void setMemoryValue(uint16_t address, int64_t value);

        IASWord memory[MEMORY_SIZE]; // 4K words of memory

        void fetchInstruction(IASInstruction &instr);
        void decodeInstruction(const IASInstruction &instr);
        void executeInstruction(const IASInstruction &instr);

        void dumpRegisters(std::ostream &out) const;
        void dumpRAM(std::ostream &out, uint16_t start, uint16_t number_of_lines) const;
        
        void prettyDumpRegisters(std::ostream &out) const;
        void prettyDumpRAM(std::ostream &out, uint16_t start, uint16_t number_of_lines) const;

        void debugCLI();
        void debugIPC();
    };

} // namespace IAS;