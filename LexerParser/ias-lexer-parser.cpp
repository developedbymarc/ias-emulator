#include "cpu.hpp"
#include "instructions.hpp"
#include "ias-lexer-parser.hpp"
#include "ansi.h"

namespace IAS
{
    IASWord IASLineParser::line() {
        IASWord word;
        if (token.type == TokenType::TOK_INT_LIT || token.type == TokenType::TOK_MINUS)
        {
            word = data_line();
        } else if (token.type == TokenType::TOK_IDENT)
        {
            word = instr_line();
        } else if (token.type == TokenType::TOK_EOL) {
            word = IASWord(0);
        } else {
            throw std::runtime_error(
                "Syntax Error: invalid start of line near \"" + originalLine + "\""
            );
        }


        if (token.type != TokenType::TOK_EOL) {
            throw std::runtime_error(
                "Syntax Error: unexpected token near \"" + originalLine
                + "\". (position "
                + std::to_string(lexer.getPos())
                + ")"
            );
        }

        return word;
    }

    IASWord IASLineParser::data_line() {
        if (token.type == TokenType::TOK_MINUS) {
            token = lexer.nextToken(); // consume the minus token
            return IASWord(- IASLineParser::integer());
        } else {
            return IASWord(IASLineParser::integer());
        }
    }
    
    int64_t IASLineParser::integer() {
        if (token.type != TokenType::TOK_INT_LIT) {
            throw std::runtime_error(
                "Syntax Error: unexpected token near" + originalLine
                + " (position "
                + std::to_string(lexer.getPos())
                + ")"
            );
        }

        constexpr int64_t MIN40 = - (1LL << 39);
        constexpr int64_t MAX40 =   (1LL << 39) - 1;

        int64_t i = std::stoll(token.text);

        if (MIN40 > i || i > MAX40) {
            throw std::runtime_error(
                "Semantic Error: data integer out of bounds. (position "
                + std::to_string(lexer.getPos())
                + ")"
            );
        }

        token = lexer.nextToken(); // consume the integer token
        return i;
    }

    IASWord IASLineParser::instr_line() {
        IAS::IASInstruction instr1 = instr();        
        IASWord word(0);
        
        if (token.type == TokenType::TOK_SEMICOLON) {
            token = lexer.nextToken();
            IAS::IASInstruction instr2 = instr();

            // leftmost 20 bits for instr1, rightmost 20 bits for instr2
            uint64_t left = (static_cast<uint32_t>(instr1.type) & IASWord::MASK8) << 12 | instr1.address;
            uint64_t right = (static_cast<uint32_t>(instr2.type) & IASWord::MASK8) << 12 | instr2.address;
            
            word.setRaw((left << 20) | right);
        } else {
            uint64_t left = (static_cast<uint32_t>(instr1.type) & IASWord::MASK8) << 12 | instr1.address;

            // leftmost 20 bits for instr1, rightmost 20 bits are 0
            word.setRaw((left << 20));
        }

        return word;
    }

    IASInstruction IASLineParser::instr() {
        IASUniqueSymbolicReprKeyword sym_repr = symbolic_repr();
        Operand op = operand();

        IASInstructionType type;

        switch (sym_repr) {
            case IASUniqueSymbolicReprKeyword::LOAD: {
                if (op.type == Operand::TYPE_MQ) { type = IASInstructionType::LOAD_MQ; }
                else if (op.type == Operand::TYPE_MQ_MX) { type = IASInstructionType::LOAD_MQ_MX; }
                else if (op.type == Operand::TYPE_MX) {
                    if (op.isNegative && op.isAbsolute) { type = IASInstructionType::LOAD_NEG_ABS_MX; }
                    else if (op.isNegative) { type = IASInstructionType::LOAD_NEG_MX; }
                    else if (op.isAbsolute) { type = IASInstructionType::LOAD_ABS_MX; }
                    else { type = IASInstructionType::LOAD_MX; }
                } else { // Invalid Operand
                    throw std::runtime_error(
                        "Semantic Error: AC cannot be loaded from. (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }

                break;
            }
            case IASUniqueSymbolicReprKeyword::STOR: {
                if (op.type == Operand::TYPE_MX) {
                    if (op.isRanged) {
                        if (op.range == Range::RANGE_08_19) { type = IASInstructionType::STOR_MX_L; }
                        else if (op.range == Range::RANGE_28_39) { type = IASInstructionType::STOR_MX_R; }
                        else {
                            throw std::runtime_error(
                                "Semantic Error: Invalid range for STOR instruction. (position "
                                + std::to_string(lexer.getPos())
                                + ")"
                            );
                        }
                    } else { type = IASInstructionType::STOR_MX; }
                } else { // Invalid Operand
                    throw std::runtime_error(
                        "Semantic Error: Only M(X) can be stored into. (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }

                break;
            }
            case IASUniqueSymbolicReprKeyword::ADD: {
                if (op.isNegative) {
                    throw std::runtime_error(
                        "Semantic Error: Operand cannot be negative for ADD instruction. (position "
                        + std::to_string(lexer.getPos())
                    );
                }

                if (op.type == Operand::TYPE_MX) {
                    if (op.isAbsolute) { type = IASInstructionType::ADD_ABS_MX; }
                    else { type = IASInstructionType::ADD_MX; }
                } else { // Invalid Operand
                    throw std::runtime_error(
                        "Semantic Error: ADD only supports M(X). (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }

                break;
            }
            case IASUniqueSymbolicReprKeyword::SUB: {
                if (op.isNegative) {
                    throw std::runtime_error(
                        "Semantic Error: Operand cannot be negative for SUB instruction. (position "
                        + std::to_string(lexer.getPos())
                    );
                }

                if (op.type == Operand::TYPE_MX) {
                    if (op.isAbsolute) { type = IASInstructionType::SUB_ABS_MX; }
                    else { type = IASInstructionType::SUB_MX; }
                } else { // Invalid Operand
                    throw std::runtime_error(
                        "Semantic Error: SUB only supports M(X). (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }

                break;
            }
            case IASUniqueSymbolicReprKeyword::MUL: {
                if (op.type == Operand::TYPE_MX) {
                    type = IASInstructionType::MUL_MX;
                } else { // Invalid Operand
                    throw std::runtime_error(
                        "Semantic Error: MUL only supports M(X). (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }

                break;
            }
            case IASUniqueSymbolicReprKeyword::DIV: {
                if (op.type == Operand::TYPE_MX) {
                    type = IASInstructionType::DIV_MX;
                } else { // Invalid Operand
                    throw std::runtime_error(
                        "Semantic Error: DIV only supports M(X). (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }

                break;
            }
            case IASUniqueSymbolicReprKeyword::LSH: {
                if (op.type != Operand::TYPE_NONE) {
                    throw std::runtime_error(
                        "Semantic Error: LSH does not take operands. (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }

                type = IASInstructionType::LSH;
                break;
            }
            case IASUniqueSymbolicReprKeyword::RSH: {
                // RSH does not take operands
                if (op.type != Operand::TYPE_NONE) {
                    throw std::runtime_error(
                        "Semantic Error: RSH does not take operands. (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }

                type = IASInstructionType::RSH;
                break;
            }
            case IASUniqueSymbolicReprKeyword::JUMP: {
                if (op.type == Operand::TYPE_MX) {
                    if (op.isRanged) {
                        if (op.range == Range::RANGE_00_19) { type = IASInstructionType::JUMP_MX_L; }
                        else if (op.range == Range::RANGE_20_39) { type = IASInstructionType::JUMP_MX_R; }
                        else {
                            throw std::runtime_error(
                                "Semantic Error: Invalid range for JUMP instruction. (position "
                                + std::to_string(lexer.getPos())
                                + ")"
                            );
                        }
                    } else {
                        throw std::runtime_error(
                            "Semantic Error: JUMP instruction requires a range. (position "
                            + std::to_string(lexer.getPos())
                            + ")"
                        );
                    }
                } else { // Invalid Operand
                    throw std::runtime_error(
                        "Semantic Error: JUMP only supports M(X,<range>). (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }

                break;
            }
            case IASUniqueSymbolicReprKeyword::JUMP_PLUS: {
                if (op.type == Operand::TYPE_MX) {
                    if (op.isRanged) {
                        if (op.range == Range::RANGE_00_19) { type = IASInstructionType::JUMP_PLUS_MX_L; }
                        else if (op.range == Range::RANGE_20_39) { type = IASInstructionType::JUMP_PLUS_MX_R; }
                        else {
                            throw std::runtime_error(
                                "Semantic Error: Invalid range for JUMP+ instruction. (position "
                                + std::to_string(lexer.getPos())
                                + ")"
                            );
                        }
                    } else {
                        throw std::runtime_error(
                            "Semantic Error: JUMP+ instruction requires a range. (position "
                            + std::to_string(lexer.getPos())
                            + ")"
                        );
                    }
                } else { // Invalid Operand
                    throw std::runtime_error(
                        "Semantic Error: JUMP+ only supports M(X,<range>). (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }

                break;
            }
            case IASUniqueSymbolicReprKeyword::PRINT: {
                if (op.type == Operand::TYPE_AC) { type = IASInstructionType::PRINT_AC; }
                else if (op.type == Operand::TYPE_MQ) { type = IASInstructionType::PRINT_MQ; }
                else if (op.type == Operand::TYPE_MX) { type = IASInstructionType::PRINT_MX; }
                else { // Invalid Operand
                    throw std::runtime_error(
                        "Semantic Error: PRINT only supports AC, MQ, or M(X). (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }

                break;
            }
            case IASUniqueSymbolicReprKeyword::HALT: {
                if (op.type != Operand::TYPE_NONE) {
                    throw std::runtime_error(
                        "Semantic Error: HALT does not take operands. (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }

                type = IASInstructionType::HALT;
                break;
            }
            default: {
                throw std::runtime_error(
                    "Semantic Error: Unrecognized instruction. (position "
                    + std::to_string(lexer.getPos())
                    + ")"
                );
            }
        }

        
        return IASInstruction(type, op.address);
    }

    IASUniqueSymbolicReprKeyword IASLineParser::symbolic_repr() {
        if (token.type != TokenType::TOK_IDENT) {
            throw std::runtime_error(
                "Syntax Error: expected instruction symbolic representation near \"" + originalLine
                + "\". (position "
                + std::to_string(lexer.getPos())
                + ")"
            );
        }

        std::string ident = token.text;
        token = lexer.nextToken();

        auto it = symbolicReprToUniqueKeyword.find(ident);
        if (it == symbolicReprToUniqueKeyword.end()) {
            throw std::runtime_error(
                "Syntax Error: unrecognized instruction symbolic representation \"" + ident
                + "\" near \"" + originalLine
                + "\". (position "
                + std::to_string(lexer.getPos())
                + ")"
            );
        }

        return it->second;
    }

    /// ```
    /// <operand> ::= { MINUS } IDENT("M") LPAREN <address> RPAREN
    ///             | { MINUS } PIPE IDENT("M") LPAREN <address> RPAREN PIPE
    ///             | IDENT("M") LPAREN <address> COMMA <range> RPAREN
    ///             | IDENT("MQ") { COMMA IDENT("M") LPAREN <address> RPAREN }
    ///             | IDENT("AC")
    /// ```
    IASLineParser::Operand IASLineParser::operand() {
        Operand op;

        

        // optional minus sign
        if (token.type == TokenType::TOK_MINUS) {
            op.isNegative = true;
            token = lexer.nextToken();
        }

        // optional absolute value pipe
        if (token.type == TokenType::TOK_PIPE) {
            // expecting operand of form |M(<address>)| in this branch
            token = lexer.nextToken();

            // check not M
            if (token.type != TokenType::TOK_IDENT || token.text != "M") {
                throw std::runtime_error(
                    "Syntax Error: expected M after | near \"" + originalLine
                    + "\". (position "
                    + std::to_string(lexer.getPos())
                    + ")"
                );
            }

            token = lexer.nextToken();

            // check not (
            if (token.type != TokenType::TOK_LPAREN) {
                throw std::runtime_error(
                    "Syntax Error: expected ( after M near \"" + originalLine
                    + "\". (position "
                    + std::to_string(lexer.getPos())
                    + ")"
                );
            }

            token = lexer.nextToken();

            // parse address
            op.address = address();

            // check not )
            if (token.type != TokenType::TOK_RPAREN) {
                throw std::runtime_error(
                    "Syntax Error: expected ) after address near \"" + originalLine
                    + "\". (position "
                    + std::to_string(lexer.getPos())
                    + ")"
                );
            }

            token = lexer.nextToken();

            // check for missing closing pipe
            if (token.type != TokenType::TOK_PIPE) {
                throw std::runtime_error(
                    "Syntax Error: expected | after operand near \"" + originalLine
                    + "\". (position "
                    + std::to_string(lexer.getPos())
                    + ")"
                );
            }
            
            op.isAbsolute = true;
            op.type = Operand::TYPE_MX;
            
            token = lexer.nextToken();
        } else if (token.type == TokenType::TOK_IDENT && token.text == "M") {
            // expecting operand of form M(<address>) or M(<address>, <range>) in this branch

            token = lexer.nextToken();

            if (token.type != TokenType::TOK_LPAREN) {
                throw std::runtime_error(
                    "Syntax Error: expected ( after M near \"" + originalLine
                    + "\". (position "
                    + std::to_string(lexer.getPos())
                    + ")"
                );
            }
            
            token = lexer.nextToken();
            op.address = address();
            
            if (token.type == TokenType::TOK_COMMA) {
                // expecting <range> after comma

                if (op.isNegative) {
                    // M(<address>, <range>) cannot be preceded by a minus sign,
                    // that type of operand does not exist
                    throw std::runtime_error(
                        "Syntax Error: Unexpected token near \"" + originalLine
                        + "\". (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }

                token = lexer.nextToken();
                op.isRanged = true;
                op.range = range();

                if (token.type != TokenType::TOK_RPAREN) {
                    throw std::runtime_error(
                        "Syntax Error: expected ) after range near \"" + originalLine
                        + "\". (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }

                token = lexer.nextToken();
                
            } else if (token.type == TokenType::TOK_RPAREN) {
                op.isRanged = false;
                token = lexer.nextToken(); // consume RPAREN
            } else {
                throw std::runtime_error(
                    "Syntax Error: expected \',\' or \')\' after address near \"" + originalLine
                    + "\". (position "
                    + std::to_string(lexer.getPos())
                    + ")"
                );
            }

            op.type = Operand::TYPE_MX;
        } else if (token.type == TokenType::TOK_IDENT && token.text == "MQ") {
            // expecting operand of form MQ or MQ,M(<address>)
            if (op.isNegative) {
                // MQ cannot be preceded by a minus sign, that type of operand does not exist
                throw std::runtime_error(
                    "Syntax Error: Unexpected token near \"" + originalLine
                    + "\". (position "
                    + std::to_string(lexer.getPos())
                    + ")"
                );
            }

            token = lexer.nextToken();

            if (token.type == TokenType::TOK_COMMA) {
                // expecting M(<address>) after comma
                
                token = lexer.nextToken(); // consume COMMA
                
                if (token.type != TokenType::TOK_IDENT || token.text != "M") {
                    throw std::runtime_error(
                        "Syntax Error: expected M after \'MQ,\' near \"" + originalLine
                        + "\". (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }
                
                token = lexer.nextToken(); // consume M
                
                if (token.type != TokenType::TOK_LPAREN) {
                    throw std::runtime_error(
                        "Syntax Error: expected ( after \'MQ, M\' near \"" + originalLine
                        + "\". (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }
                
                token = lexer.nextToken(); // consume LPAREN
                op.address = address();
                
                if (token.type != TokenType::TOK_RPAREN) {
                    throw std::runtime_error(
                        "Syntax Error: unmatched open parenthesis near \"" + originalLine
                        + "\". (position "
                        + std::to_string(lexer.getPos())
                        + ")"
                    );
                }
                
                token = lexer.nextToken(); // consume RPAREN
                op.type = Operand::TYPE_MQ_MX;
            } else {
                // operand of form MQ
                op.type = Operand::TYPE_MQ;
            }
        } else if (token.type == TokenType::TOK_IDENT && token.text == "AC") {
            // operand of form AC
            if (op.isNegative) {
                // AC cannot be preceded by a minus sign, that type of operand does not exist
                throw std::runtime_error(
                    "Syntax Error: Unexpected token near \"" + originalLine
                    + "\". (position "
                    + std::to_string(lexer.getPos())
                    + ")"
                );
            }

            op.type = Operand::TYPE_AC;
            token = lexer.nextToken(); // consume AC
        } else {
            // no operand
            if (op.isNegative) {
                // if there's a minus sign, there must be an operand after it, so this is an error
                throw std::runtime_error(
                    "Syntax Error: Unexpected token near \"" + originalLine
                    + "\". (position "
                    + std::to_string(lexer.getPos())
                    + ")"
                );
            }

            op.type = Operand::TYPE_NONE;
        }

        return op;
    }

    uint16_t IASLineParser::address() {
        if (token.type != TokenType::TOK_INT_LIT) {
            throw std::runtime_error(
                "Syntax Error: expected integer for address near \"" + originalLine
                + "\". (position "
                + std::to_string(lexer.getPos())
                + ")"
            );
        }

        int64_t addr = std::stoll(token.text);

        if (addr < 0 || addr > 4095) {
            throw std::runtime_error(
                "Semantic Error: address out of bounds. (position "
                + std::to_string(lexer.getPos())
                + ")"
            );
        }

        token = lexer.nextToken(); // consume the integer token
        return static_cast<uint16_t>(addr);
    }

    IASLineParser::Range IASLineParser::range() {
        if (token.type != TokenType::TOK_INT_LIT) {
            throw std::runtime_error(
                "Syntax Error: expected integer for range near \"" + originalLine
                + "\". (position "
                + std::to_string(lexer.getPos())
                + ")"
            );
        }
        
        int64_t start = std::stoll(token.text);
        token = lexer.nextToken();

        if (token.type != TokenType::TOK_COLON) {
            throw std::runtime_error(
                "Syntax Error: expected colon for range near \"" + originalLine
                + "\". (position "
                + std::to_string(lexer.getPos())
                + ")"
            );
        }

        token = lexer.nextToken();

        if (token.type != TokenType::TOK_INT_LIT) {
            throw std::runtime_error(
                "Syntax Error: expected integer for range near \"" + originalLine
                + "\". (position "
                + std::to_string(lexer.getPos())
                + ")"
            );
        }

        int64_t end = std::stoll(token.text);

        token = lexer.nextToken();

        if (start == 0 && end == 19) {
            return Range::RANGE_00_19;
        } else if (start == 8 && end == 19) {
            return Range::RANGE_08_19;
        } else if (start == 20 && end == 39) {
            return Range::RANGE_20_39;
        } else if (start == 28 && end == 39) {
            return Range::RANGE_28_39;
        } else {
            throw std::runtime_error(
                "Semantic Error: invalid range. (position "
                + std::to_string(lexer.getPos())
                + ")"
            );
        }
    }
} // namespace IAS
