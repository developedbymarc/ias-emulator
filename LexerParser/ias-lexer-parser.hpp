#pragma once

#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "instructions.hpp"

namespace IAS
{    
    enum class TokenType {
        TOK_EOL, // end of line

        TOK_IDENT,
        TOK_INT_LIT,
        
        TOK_LPAREN,    // `(`
        TOK_RPAREN,    // `)`
        TOK_COMMA,     // `,`
        TOK_COLON,     // `:`
        TOK_SEMICOLON, // `;`
        TOK_PIPE,      // `|`
        TOK_MINUS,     // `-`
        TOK_FWD_SLASH, // `/` for comment start

        TOK_UNKNOWN,
    };

    struct Token {
        TokenType type;
        std::string text; // for IDENT, INT_LIT and UNKNOW

        Token(TokenType t = TokenType::TOK_EOL, std::string s = "") {
            type = t;
            text = std::move(s);
        }
    };

    class LineLexer {
    public:
        LineLexer(const std::string &line = "") {
            s = line;
            pos = 0;
        }

        Token nextToken() {
            skipWhitespace();
            if (pos >= s.size()) return Token(TokenType::TOK_EOL, "");

            char c = s[pos];

            // Comment: starts with `//` and gets treated as EOL
            if (pos + 1 < s.size() && s[pos] == '/' && s[pos + 1] == '/') {
                pos = s.size(); // move to end of line
                return Token(TokenType::TOK_EOL, ""); // recursively get next token
            }

            // IDENT: starts with letter, followed by letters/digits
            if (std::isalpha((unsigned char)c)) {
                std::string ident;
                while (pos < s.size() && (std::isalnum((unsigned char)s[pos]) || s[pos] == '+')) {
                    ident += s[pos];
                    pos++;
                }
                return Token(TokenType::TOK_IDENT, ident);
            }

            // INT_LIT: starts with digit, followed by digits
            if (std::isdigit((unsigned char)c)) {
                std::string number;
                while (pos < s.size() && std::isdigit((unsigned char)s[pos])) {
                    number += s[pos];
                    pos++;
                }
                return Token(TokenType::TOK_INT_LIT, number);
            }

            // Single-character tokens
            pos++; // consume the character
            switch (c) {
                case '(': return Token(TokenType::TOK_LPAREN, "(");
                case ')': return Token(TokenType::TOK_RPAREN, ")");
                case ',': return Token(TokenType::TOK_COMMA, ",");
                case ':': return Token(TokenType::TOK_COLON, ":");
                case ';': return Token(TokenType::TOK_SEMICOLON, ";");
                case '|': return Token(TokenType::TOK_PIPE, "|");
                case '-': return Token(TokenType::TOK_MINUS, "-");
                default: return Token(TokenType::TOK_UNKNOWN, "");
            }
        }

        size_t getPos() {
            return pos;
        }

    private:
        std::string s;
        size_t pos;

        void skipWhitespace() {
            while (pos < s.size()) {
                char c = s[pos];
                if (! std::isspace((unsigned char)c)) break;
                pos++;
            }
        }
    };

    // recursive descent parser, inspired from the CSC239: Concepts of Programming Languages course :)
    class IASLineParser {
    public:
        IASLineParser() = delete;
        IASLineParser(const std::string &line) {
            lexer = LineLexer(line);
            token = lexer.nextToken();
            originalLine = line;
        }

        /// ```
        /// <line> ::= (<data_line> | <instr_line>) EOL | EOL
        /// ```
        IAS::IASWord line();

    private:
        LineLexer lexer;
        Token token;
        std::string originalLine;

        /// ```
        /// <data_line> ::= [ MINUS ] <integer>
        /// ```
        IAS::IASWord data_line();

        /// ```
        /// <instr_line> ::= <instr> [ SEMICOLON <instr> ]
        /// ```
        IAS::IASWord instr_line();

        /// ```
        /// <instr> ::= <symbolic_repr> <operand>
        /// ```
        IAS::IASInstruction instr();
        
        enum Range: uint8_t {
            RANGE_00_19,
            RANGE_08_19,
            RANGE_20_39,
            RANGE_28_39
        };

        struct Operand {
            bool isNegative = false;
            bool isAbsolute = false;
            bool isRanged   = false;
            enum Type: uint8_t {
                TYPE_MX,
                TYPE_MQ,
                TYPE_MQ_MX,
                TYPE_AC,
                TYPE_NONE, // for instructions that don't take operands
            } type;
            uint16_t address = 0; // only for TYPE_MX
            Range range = RANGE_00_19; // only for TYPE_MX with range
        };

        /// ```
        /// <symbolic_repr> ::= IDENT matched with the corresponding symbolic representation
        /// ```
        IASUniqueSymbolicReprKeyword symbolic_repr();

        /// ```
        /// <operand> ::= { MINUS } IDENT("M") LPAREN <address> RPAREN
        ///             | { MINUS } PIPE IDENT("M") LPAREN <address> RPAREN PIPE
        ///             | IDENT("M") LPAREN <address> COMMA <range> RPAREN
        ///             | IDENT("MQ") { COMMA IDENT("M") LPAREN <address> RPAREN }
        ///             | IDENT("AC")
        /// ```
        IASLineParser::Operand operand();

        /// ```
        /// <address> ::= <integer>
        /// ```
        /// Semantic rule: `0 <= <integer> <= 4095`
        uint16_t address();

        /// ```
        /// <range> ::= <integer>(0) COLON <integer>(19)
        ///           | <integer>(8) COLON <integer>(19)
        ///           | <integer>(20) COLON <integer>(39)
        ///           | <integer>(28) COLON <integer>(39)
        /// ```
        IASLineParser::Range range();

        /// ```
        /// <integer> ::= Parsed INT_LIT
        /// ```
        /// Semantic rule: `-549755813888 <= <integer> <= 549755813887`
        int64_t integer();
    };
} // namespace IAS
