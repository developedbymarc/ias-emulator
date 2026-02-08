#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace IAS
{
    // Maps the symbolic representation to the opcode
    enum IASInstructionType: uint8_t
    {
        // Data Transfer
        LOAD_MQ_MX      = 0b0000'1001,
        LOAD_MQ         = 0b0000'1010,
        LOAD_MX         = 0b0000'0001,
        LOAD_NEG_MX     = 0b0000'0010,
        LOAD_ABS_MX     = 0b0000'0011,
        LOAD_NEG_ABS_MX = 0b0000'0100,
        STOR_MX         = 0b0010'0001,

        // Arithmetic Operations
        ADD_MX          = 0b0000'0101,
        SUB_MX          = 0b0000'0110,
        ADD_ABS_MX      = 0b0000'0111,
        SUB_ABS_MX      = 0b0000'1000,
        MUL_MX          = 0b0000'1011,
        DIV_MX          = 0b0000'1100,
        LSH             = 0b0001'0100,
        RSH             = 0b0001'0101,

        // Unconditional Branching
        JUMP_MX_L       = 0b0000'1101,
        JUMP_MX_R       = 0b0000'1110,

        // Conditional Branching
        JUMP_PLUS_MX_L  = 0b0000'1111,
        JUMP_PLUS_MX_R  = 0b0001'0000,

        // Address Modification
        STOR_MX_L       = 0b0001'0010,
        STOR_MX_R       = 0b0001'0011,

        // Extras
        PRINT_MX        = 0b1111'1111,
        PRINT_MQ        = 0b1111'1110,
        PRINT_AC        = 0b1111'1101,
        HALT            = 0b0000'0000,
    };

    enum class IASUniqueSymbolicReprKeyword: uint8_t
    {
        LOAD,
        STOR,
        ADD,
        SUB,
        MUL,
        DIV,
        LSH,
        RSH,
        JUMP,
        JUMP_PLUS,
        
        PRINT,
        HALT,
    };

    extern const std::unordered_set<uint8_t> instrOpcodeSet;
    extern const std::unordered_map<std::string, IASUniqueSymbolicReprKeyword> symbolicReprToUniqueKeyword;
    extern const std::unordered_map<IASInstructionType, std::string> InstructionTypeToStringMap;
    IASInstructionType opcodeToIASInstructionType(uint8_t opcode);
    std::string IASInstructionTypeToString(IASInstructionType type);

    class IASInstruction
    {
        public:
            IASInstruction() = default;
            IASInstruction(IASInstructionType t, uint16_t a) : type(t), address(a) {}

            IASInstructionType type = HALT;
            uint16_t address = 0;

            std::string toString() {
                switch(type) {
                    case HALT           : return "HALT";
                    case PRINT_AC       : return "PRINT AC";
                    case PRINT_MQ       : return "PRINT MQ";
                    case PRINT_MX       : return "PRINT M(" + std::to_string(address) + ")";

                    case LOAD_MQ        : return "LOAD MQ";
                    case LOAD_MX        : return "LOAD M(" + std::to_string(address) + ")";
                    case LOAD_MQ_MX     : return "LOAD MQ,M(" + std::to_string(address) + ")";
                    case LOAD_NEG_MX    : return "LOAD -M(" + std::to_string(address) + ")";
                    case LOAD_ABS_MX    : return "LOAD |M(" + std::to_string(address) + ")|";
                    case LOAD_NEG_ABS_MX: return "LOAD -|M(" + std::to_string(address) + ")|";

                    case STOR_MX        : return "STOR M(" + std::to_string(address) + ")";

                    case ADD_MX         : return "ADD M(" + std::to_string(address) + ")";
                    case SUB_MX         : return "SUB M(" + std::to_string(address) + ")";
                    case ADD_ABS_MX     : return "ADD |M(" + std::to_string(address) + ")|";
                    case SUB_ABS_MX     : return "SUB |M(" + std::to_string(address) + ")|";

                    case MUL_MX         : return "MUL M(" + std::to_string(address) + ")";
                    case DIV_MX         : return "DIV M(" + std::to_string(address) + ")";

                    case LSH            : return "LSH";
                    case RSH            : return "RSH";

                    case JUMP_MX_L      : return "JUMP M(" + std::to_string(address) + ",0:19)";
                    case JUMP_MX_R      : return "JUMP M(" + std::to_string(address) + ",20:39)";

                    case JUMP_PLUS_MX_L : return "JUMP+ M(" + std::to_string(address) + ",0:19)";
                    case JUMP_PLUS_MX_R : return "JUMP+ M(" + std::to_string(address) + ",20:39)";

                    case STOR_MX_L      : return "STOR M(" + std::to_string(address) + ",8:19)";
                    case STOR_MX_R      : return "STOR M(" + std::to_string(address) + ",28:39)";

                    default: return "INVALID";
                }
            };
    };

    class IASWord
    {
        public:
            static const uint64_t MASK40 = 0xFF'FF'FF'FF'FF;
            static const uint32_t MASK20 = 0xFF'FFF;
            static const uint16_t MASK12 = 0xFFF;
            static const uint8_t  MASK8  = 0xFF;
            
            IASWord() = default;
            explicit IASWord(int64_t v) { set(v); };

            void set(int64_t v)
            {
                raw = static_cast<uint64_t>(v) & MASK40;
            }

            void setRaw(uint64_t v)
            {
                raw = v & MASK40;
            }

            uint64_t get() const
            {
                return raw & MASK40;
            }

            int64_t asSigned() const {
                int64_t v = raw & MASK40;
                if (v & (1ULL << 39)) v |= ~MASK40; // sign-extend
                return v;
            }

            uint64_t asUnsigned() const {
                return raw & MASK40;
            }

            void storePartial(int64_t v, uint8_t lo, uint8_t hi)
            {
                int width = hi - lo + 1;
                if (width < 0 || width > 40) {
                    throw std::invalid_argument("Computed width is invalid");
                }
                
                v = v & IASWord::MASK40;      
                int64_t mask = ((1LL << width) - 1) << lo;
                raw = (raw & ~mask) | ((v << lo) & mask);
            }

            IASInstruction* decodeLeft() const {
                uint32_t raw_instruction = (raw >> 20) & MASK20;
                uint16_t address = raw_instruction & MASK12;
                uint8_t opcode = (raw_instruction >> 12) & MASK8;
                IASInstructionType type = opcodeToIASInstructionType(opcode);

                return new IASInstruction(type, address);
            }
            
            IASInstruction* decodeRight() const {
                uint32_t raw_instruction = raw & MASK20;
                uint16_t address = raw_instruction & MASK12;
                uint8_t opcode = (raw_instruction >> 12) & MASK8;
                IASInstructionType type = opcodeToIASInstructionType(opcode);

                return new IASInstruction(type, address);
            }

        private:
            uint64_t raw = 0;
    };
} // namespace IAS