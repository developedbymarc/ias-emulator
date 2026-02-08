#include <unordered_map>
#include <unordered_set>

#include "instructions.hpp"

namespace IAS
{    
    const std::unordered_set<uint8_t> instrOpcodeSet = {
        0b0000'1001, // LOAD MQ,M(X)
        0b0000'1010, // LOAD MQ
        0b0000'0001, // LOAD M(X)
        0b0000'0010, // LOAD -M(X)
        0b0000'0011, // LOAD |M(X)|
        0b0000'0100, // LOAD -|M(X)|
        
        0b0010'0001, // STOR M(X)

        0b0000'0101, // ADD M(X)
        0b0000'0110, // SUB M(X)
        0b0000'0111, // ADD |M(X)|
        0b0000'1000, // SUB |M(X)|
        0b0000'1011, // MUL M(X)
        0b0000'1100, // DIV M(X)
        0b0001'0100, // LSH
        0b0001'0101, // RSH

        0b0000'1101, // JUMP M(X,0:19)
        0b0000'1110, // JUMP M(X,20:39)

        0b0000'1111, // JUMP+ M(X,0:19)
        0b0001'0000, // JUMP+ M(X,20:39)

        0b0001'0010, // STOR M(X,8:19)
        0b0001'0011, // STOR M(X,28:39)

        0b1111'1111, // PRINT MX
        0b1111'1110, // PRINT MQ
        0b1111'1101, // PRINT AC
        0b0000'0000, // HALT
    };

    const std::unordered_map<std::string, IASUniqueSymbolicReprKeyword> symbolicReprToUniqueKeyword = {
        {"LOAD", IASUniqueSymbolicReprKeyword::LOAD},
        {"STOR", IASUniqueSymbolicReprKeyword::STOR},
        {"ADD", IASUniqueSymbolicReprKeyword::ADD},
        {"SUB", IASUniqueSymbolicReprKeyword::SUB},
        {"MUL", IASUniqueSymbolicReprKeyword::MUL},
        {"DIV", IASUniqueSymbolicReprKeyword::DIV},
        {"LSH",  IASUniqueSymbolicReprKeyword::LSH},
        {"RSH",  IASUniqueSymbolicReprKeyword::RSH},
        {"JUMP", IASUniqueSymbolicReprKeyword::JUMP},
        {"JUMP+", IASUniqueSymbolicReprKeyword::JUMP_PLUS},
        {"PRINT", IASUniqueSymbolicReprKeyword::PRINT},
        {"HALT",  IASUniqueSymbolicReprKeyword::HALT},
    };

    const std::unordered_map<IASInstructionType, std::string> InstructionTypeToStringMap = {
        {IASInstructionType::LOAD_MQ_MX      , "LOAD MQ,M(X)"       },
        {IASInstructionType::LOAD_MQ         , "LOAD MQ"            },
        {IASInstructionType::LOAD_MX         , "LOAD M(X)"          },
        {IASInstructionType::LOAD_NEG_MX     , "LOAD -M(X)"         },
        {IASInstructionType::LOAD_ABS_MX     , "LOAD |M(X)|"        },
        {IASInstructionType::LOAD_NEG_ABS_MX , "LOAD -|M(X)|"       },
        {IASInstructionType::STOR_MX         , "STOR M(X)"          },

        {IASInstructionType::ADD_MX          , "ADD M(X)"           },
        {IASInstructionType::SUB_MX          , "SUB M(X)"           },
        {IASInstructionType::ADD_ABS_MX      , "ADD |M(X)|"         },
        {IASInstructionType::SUB_ABS_MX      , "SUB |M(X)|"         },
        {IASInstructionType::MUL_MX          , "MUL M(X)"           },
        {IASInstructionType::DIV_MX          , "DIV M(X)"           },
        {IASInstructionType::LSH             , "LSH"                },
        {IASInstructionType::RSH             , "RSH"                },

        {IASInstructionType::JUMP_MX_L       , "JUMP M(X,0:19)"     },
        {IASInstructionType::JUMP_MX_R       , "JUMP M(X,20:39)"    },

        {IASInstructionType::JUMP_PLUS_MX_L  , "JUMP+ M(X,0:19)"    },
        {IASInstructionType::JUMP_PLUS_MX_R  , "JUMP+ M(X,20:39)"   },

        {IASInstructionType::STOR_MX_L       , "STOR M(X,8:19)"     },
        {IASInstructionType::STOR_MX_R       , "STOR M(X,28:39)"    },

        {IASInstructionType::PRINT_MX        , "PRINT MX"           },
        {IASInstructionType::PRINT_MQ        , "PRINT MQ"           },
        {IASInstructionType::PRINT_AC        , "PRINT AC"           },
        {IASInstructionType::HALT            , "HALT"               },
    };

    IASInstructionType opcodeToIASInstructionType(uint8_t opcode) {
        if (instrOpcodeSet.find(opcode) != instrOpcodeSet.end()) {
            return static_cast<IASInstructionType>(opcode);
        }
        throw std::invalid_argument("Invalid instruction opcode");
    }

    std::string IASInstructionTypeToString(IASInstructionType type) {
        auto it = InstructionTypeToStringMap.find(type);
        if (it != InstructionTypeToStringMap.end()) {
            return it->second;
        }
        return "INVALID";
    }
}; // namespace IAS
