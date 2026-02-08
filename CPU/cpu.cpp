#include <fstream>
#include <ctime>
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include <cmath>

#include "cpu.hpp"
#include "instructions.hpp"
#include "ansi.h"
#include "formatting.hpp"

using namespace IAS;

IASCPU::IASCPU()
{
    AC = 0;
    MQ = 0;
    PC = 0;
    for (size_t i = 0; i < MEMORY_SIZE; i++)
    {
        memory[i] = IASWord(0);
    }
}

void IASCPU::loadProgram(const IASWord *program, size_t size)
{
    for (size_t i = 0; i < size && i < MEMORY_SIZE; i++)
    {
        memory[i] = program[i];
    }
}

/// @brief Dumps the CPU registers to the given output stream in JSON format
/// @param out The output stream to write to
void IASCPU::dumpRegisters(std::ostream &out) const
{
    // FORMAT:
    //  {
    //      "REGISTERS": {
    //          "PC":  {"int": "...", "bits": "..."},
    //          "AC":  {"int": "...", "bits": "..."},
    //          "MQ":  {"int": "...", "bits": "..."},
    //          "IR":  {"int": "...", "bits": "...", "instr": "..."},
    //          "MAR": {"int": "...", "bits": "..."},
    //          "MBR": {"int": "...", "bits": "...", "instr": "..."}
    //      }
    //  }
    //

    out << "\"REGISTERS\":{";

    out << "\"PC\":{\"int\":\"" << PC << "\",\"bits\":\"" << Formatting::bitrepr(PC, 12) << "\"},";
    out << "\"AC\":{\"int\":\"" << AC << "\",\"bits\":\"" << Formatting::bitrepr(AC, 40) << "\"},";
    out << "\"MQ\":{\"int\":\"" << MQ << "\",\"bits\":\"" << Formatting::bitrepr(MQ, 40) << "\"},";
    out << "\"IR\":{\"int\":\"" << static_cast<int>(IR) << "\",\"bits\":\"" << Formatting::bitrepr(IR, 8) << "\",\"instr\":\"" << IASInstructionTypeToString(IR) << "\"},";
    out << "\"MAR\":{\"int\":\"" << MAR << "\",\"bits\":\"" << Formatting::bitrepr(MAR, 12) << "\"},";

    out << "\"MBR\":{\"int\":\"" << MBR.asSigned() << "\""
        << ",\"bits\":\"" << Formatting::bitrepr(MBR.asUnsigned(), 40) << "\""
        << ",\"instr\":\"" << Formatting::formatInstrPair(MBR) << "\"}"
        << "}" << std::flush;
}

/// @see `IASCPU::dumpRegisters`
void IASCPU::prettyDumpRegisters(std::ostream &out) const
{
    out << "\"REGISTERS\": {\n";

    out << '\t' << "\"PC\" : { \"dec\":\"" << PC << "\", \"bits\":\"" << Formatting::bitrepr(PC, 12) << "\" },\n";
    out << '\t' << "\"AC\" : { \"dec\":\"" << AC << "\", \"bits\":\"" << Formatting::bitrepr(AC, 40) << "\" },\n";
    out << '\t' << "\"MQ\" : { \"dec\":\"" << MQ << "\", \"bits\":\"" << Formatting::bitrepr(MQ, 40) << "\" },\n";
    out << '\t' << "\"IR\" : { \"dec\":\"" << static_cast<int>(IR) << "\", \"bits\":\"" << Formatting::bitrepr(IR, 8) << "\" },\n";
    out << '\t' << "\"MAR\": { \"dec\":\"" << MAR << "\", \"bits\":\"" << Formatting::bitrepr(MAR, 12) << "\" },\n";

    out << '\t' << "\"MBR\":{ \"int\":\"" << MBR.asSigned() << "\""
        << ", \"bits\":\"" << Formatting::bitrepr(MBR.asUnsigned(), 40) << "\""
        << ", \"instr\":\"" << Formatting::formatInstrPair(MBR) << "\"}\n"
        << "}";
}

/// @brief Dumps the CPU RAM to the given output stream in JSON format
/// @param out The output stream to write to
/// @param start The starting address of the memory range to dump
/// @param number_of_lines The number of lines to dump
void IASCPU::dumpRAM(std::ostream &out,
                     uint16_t start,
                     uint16_t number_of_lines) const
{
    // FORMAT:
    //
    // "RAM": {
    //   "range": {"start": "...", "end": "..."},
    //   "memory": [
    //     {"addr": "...", "raw": "...", "signed": "...", "instr": "..."},
    //     {"addr": "...", "raw": "...", "signed": "...", "instr": "..."},
    //     ...
    //   ]
    // }
    //

    uint16_t end = std::min<uint16_t>(
        start + number_of_lines,
        MEMORY_SIZE);

    out << "\"RAM\":{\"range\":{\"start\":" << start << ",\"end\":" << (end - 1) << "},\"memory\":[";

    for (auto i = start; i < end; i++)
    {
        const IASWord &word = memory[i];

        out << '{'
            << "\"addr\":\"" << std::setw(4) << std::setfill('0') << i << "\","
            << "\"raw\":\"" << Formatting::bitrepr(word.asUnsigned(), 40) << "\","
            << "\"signed\":\"" << std::setw(12) << std::setfill(' ') << word.asSigned() << "\","
            << "\"instr\":\"" << Formatting::formatInstrPair(word) << "\""
            << '}';

        if (i != end - 1)
        {
            out << "," << std::flush;
        }
    }

    out << "]}" << std::flush;
}

/// @see `IASCPU::dumpRam`
void IASCPU::prettyDumpRAM(std::ostream &out,
                           uint16_t start,
                           uint16_t number_of_lines) const
{
    uint16_t end = std::min<uint16_t>(
        start + number_of_lines,
        MEMORY_SIZE);

    out << "\"RAM\": {\n\t\"range\": {\"start\":" << start << ",\"end\":" << (end - 1) << "},\n\t\"memory\": [";

    for (auto i = start; i < end; i++)
    {
        if (i == start) out << '\n';

        const IASWord &word = memory[i];

        out << '{'
            << " \"addr\": " << std::setw(4) << std::setfill('0') << i << ','
            << " \"raw\": " << Formatting::bitrepr(word.asUnsigned(), 40) << ','
            << " \"signed\": " << std::setw(12) << std::setfill(' ') << word.asSigned() << ','
            << " \"instr\": \"" << Formatting::formatInstrPair(word) << "\" "
            << '}';

        if (i != end - 1)
        {
            out << ",\n";
        }
    }

    out << "]\n}";
}

void IASCPU::dumpMemory(uint16_t start, uint16_t number_of_lines, std::ostream &out)
{
    if (start < 0 || start >= MEMORY_SIZE)
        throw std::invalid_argument("Invalid starting memory address");

    out << "{\n";
    dumpRegisters(out);
    out << ",\n";
    dumpRAM(out, start, number_of_lines);
    out << "\n}\n";
}

void IASCPU::prettyDumpMemory(uint16_t start, uint16_t number_of_lines, std::ostream &out)
{
    if (start < 0 || start >= MEMORY_SIZE)
        throw std::invalid_argument("Invalid starting memory address");

    out << "{\n";
    prettyDumpRegisters(out);
    out << ",\n";
    prettyDumpRAM(out, start, number_of_lines);
    out << "\n}\n";
}

void IASCPU::run()
{
    while (true)
    {
        IASInstruction instr;
        fetchInstruction(instr);
        decodeInstruction(instr);
        executeInstruction(instr);
        if (IR == HALT)
            break;
    }
}

void IASCPU::debug(DebugMode mode)
{
    if (mode == DebugMode::CLI)
    {
        debugCLI();
    }
    else if (mode == DebugMode::IPC)
    {
        debugIPC();
    }
    else
    {
        throw std::invalid_argument("Invalid debug mode");
    }
}

void IASCPU::debugIPC()
{
    std::cout << "Entering IPC debug mode. Waiting for commands on stdin..." << std::endl;
    std::string command;
    while (std::getline(std::cin, command))
    {
        if (command == "step")
        {
            IASInstruction instr;
            fetchInstruction(instr);
            decodeInstruction(instr);
            executeInstruction(instr);

            std::cout << "{\"type\":\"step\",";
            dumpRegisters(std::cout);
            std::cout << "}" << std::endl;

            if (IR == HALT)
                break;
        }
        else if (command.rfind("dump ", 0) == 0)
        {
            // Command format: dump <start> <number_of_lines>
            std::istringstream iss(command.substr(5));
            uint16_t start, number_of_lines;
            if (!(iss >> start >> number_of_lines))
            {
                std::cerr << "Invalid dump command format. Use: dump <start> <number_of_lines>" << std::endl;
                continue;
            }

            std::cout << "{\"type\":\"dump\",";
            dumpRAM(std::cout, start, number_of_lines);
            std::cout << "}" << std::endl;
        }
        else if (command == "exit")
        {
            break;
        }
        else
        {
            std::cerr << "Unknown command. Available commands: step, dump <start> <number_of_lines>, exit" << std::endl;
        }
    }
    return;
}

void IASCPU::debugCLI()
{
    std::cout
            << ANSI::Color::YELLOW
            << "Entering CLI debug mode. Press "
            << ANSI::Color::WHITE << ANSI::Color::BG_YELLOW
            << "[Enter]"
            << ANSI::Color::RESET
            << ANSI::Color::YELLOW
            << " to continue, or "
            << ANSI::Color::WHITE << ANSI::Color::BG_YELLOW
            << "[M] + [Enter]"
            << ANSI::Color::RESET
            << ANSI::Color::YELLOW
            << " to request a snapshot of the CPU registers and memory state\n\n"
            << ANSI::Color::RESET
            << std::flush;

    while (true)
    {
        std::string command;

        if (!std::getline(std::cin, command)) break; // EOF -> exit

        if (command == "")
        {
            // step
            IASInstruction instr;
            std::cout << "PC: " << PC << ", ";
            fetchInstruction(instr);
            std::cout
                << ANSI::Color::GREEN
                << instr.toString()
                << ANSI::Color::RESET;
            decodeInstruction(instr);
            executeInstruction(instr);
        }
        else if (command == "M" || command == "m")
        {
            std::cout << "Input start address [0-4095]: " << std::flush;

            uint16_t start;
            std::cin >> start;

            std::cout << "Input number of lines to output [0-4096]: " << std::flush;

            uint16_t number_of_lines;
            std::cin >> number_of_lines;

            std::ofstream outfile(Formatting::makeTimestampedFilename());
            if (!outfile)
            {
                std::cerr << "Unable to open memory dump file" << std::endl;
                return;
            }
            prettyDumpMemory(start, number_of_lines, outfile);
        }

        if (IR == HALT)
            break;
    }
}

void IASCPU::fetchInstruction(IASInstruction &instr)
{
    MAR = PC;
    MBR = memory[MAR];

    if (IBR != nullptr) // there exists a buffered instruction
    {
        instr = *IBR;
        IBR = nullptr; // Clear IBR after fetching
        PC++;
    }
    else
    {
        instr = *MBR.decodeLeft();
        IBR = MBR.decodeRight();
    }
}

void IASCPU::decodeInstruction(const IASInstruction &instr)
{
    IR = instr.type;
    MAR = instr.address;
}

void IASCPU::executeInstruction(const IASInstruction &instr)
{
    switch (IR)
    {
        case LOAD_MQ:
        {
            AC = MQ;
            break;
        }
        case LOAD_MQ_MX:
        {
            MQ = getMemoryValue(MAR);
            break;
        }
        case LOAD_MX:
        {
            AC = getMemoryValue(MAR);
            break;
        }
        case LOAD_NEG_MX:
        {
            AC = -getMemoryValue(MAR);
            break;
        }
        case LOAD_ABS_MX:
        {
            AC = std::abs(getMemoryValue(MAR));
            break;
        }
        case LOAD_NEG_ABS_MX:
        {
            AC = -std::abs(getMemoryValue(MAR));
            break;
        }
        case STOR_MX:
        {
            setMemoryValue(MAR, AC);
            break;
        }
        case ADD_MX:
        {
            AC += getMemoryValue(MAR);
            break;
        }
        case ADD_ABS_MX:
        {
            AC += std::abs(getMemoryValue(MAR));
            break;
        }
        case SUB_MX:
        {
            AC -= getMemoryValue(MAR);
            break;
        }
        case SUB_ABS_MX:
        {
            AC -= std::abs(getMemoryValue(MAR));
            break;
        }
        case MUL_MX:
        {
            int64_t product = (getMemoryValue(MAR) * MQ) & IASWord::MASK40;
            AC = (product >> 20) & IASWord::MASK20;
            MQ = product & IASWord::MASK20;
            break;
        }
        case DIV_MX:
        {
            int64_t quotient = (AC / getMemoryValue(MAR)) & IASWord::MASK40;
            int64_t remainder = (AC % getMemoryValue(MAR)) & IASWord::MASK40;
            AC = remainder;
            MQ = quotient;
            break;
        }
        case LSH:
        {
            AC = (AC << 1) & IASWord::MASK40;
            break;
        }
        case RSH:
        {
            AC = (AC >> 1) & IASWord::MASK40;
            break;
        }
        case JUMP_MX_L:
        {
            PC = MAR;
            IBR = nullptr;
            break;
        }
        case JUMP_MX_R:
        {
            PC = MAR;
            IBR = memory[PC].decodeRight();
            break;
        }
        case JUMP_PLUS_MX_L:
        {
            if (AC >= 0)
            {
                PC = MAR;
                IBR = nullptr;
            }
            break;
        }
        case JUMP_PLUS_MX_R:
        {
            if (AC >= 0)
            {
                PC = MAR;
                IBR = memory[PC].decodeRight();
            }
            break;
        }
        case STOR_MX_L:
        {
            // 8:19 on IAS => 20:31 here
            memory[MAR].storePartial(AC & IASWord::MASK12, 20, 31);
            break;
        }
        case STOR_MX_R:
        {
            // 28:39 on IAS => 0:11 here
            memory[MAR].storePartial(AC & IASWord::MASK12, 0, 11);
            break;
        }
        case PRINT_MX:
        {
            std::cout << '\n' << getMemoryValue(MAR) << std::endl;
            break;
        }
        case PRINT_AC:
        {
            std::cout << '\n' << AC << std::endl;
            break;
        }
        case PRINT_MQ:
        {
            std::cout << '\n' << MQ << std::endl;
            break;
        }
        case HALT:
        {
            std::cout << '\n'
                    << ANSI::Color::YELLOW << "Machine halted" << ANSI::Color::RESET << std::endl;
            break;
        }
        default:
        {
            std::cerr << "Unimplemented instruction: " << static_cast<uint8_t>(IR) << std::endl;
            break;
        }
    }
}

int64_t IASCPU::getMemoryValue(uint16_t address)
{
    return memory[address].asSigned();
}

void IASCPU::setMemoryValue(uint16_t address, int64_t value)
{
    memory[address].set(value);
}