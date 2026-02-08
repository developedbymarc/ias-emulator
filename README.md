# IAS CPU Emulator

A C++ emulator for the IAS computer architecture.

## Building

### Prerequisites
- C++14 compatible compiler (g++, clang...)

### Build Steps

Compile the emulator using the provided Makefile by choosing a target for your platform and optionally setting custom build options:
```bash
# Linux
make linux LINUX_CC=g++ CFLAGS="-O2 -Wall" OUT=my_emulator.out

# Windows (using MinGW)
make win64 WIN_CC=g++ CFLAGS="-O2 -Wall"

# macOS
make macos MACOS_CC=clang++ CFLAGS="-O2 -Wall"
```

The executable will be generated in `bin/<target>`.

## Usage

Run the emulator:
```bash
<executable> <path_to_program> [--debug {IPC|CLI}]
```

## Editor

For an IDE to write and debug ias programs, see the [IAS Editor](https://github.com/developedbymarc/ias-editor) project.

## Syntax Reference

```
<line> ::= (<data_line> | <instr_line>) EOL | EOL

<data_line> ::= [ MINUS ] <integer>

<instr_line> ::= <instr> [ SEMICOLON <instr> ]

<instr> ::= <symbolic_repr> <operand>

<symbolic_repr> ::= IDENT matched with the corresponding symbolic representation unique keyword (LOAD, STOR, JUMP, etc.)

<operand> ::= { MINUS } IDENT("M") LPAREN <address> RPAREN
            | { MINUS } PIPE IDENT("M") LPAREN <address> RPAREN PIPE
            | IDENT("M") LPAREN <address> COMMA <range> RPAREN
            | IDENT("MQ") [ COMMA IDENT("M") LPAREN <address> RPAREN ]
            | IDENT("AC")

<address> ::= <integer>
<address> semantic rule: 0 <= <integer> <= 4095

<range> ::= <integer>(0) COLON <integer>(19)
          | <integer>(8) COLON <integer>(19)
          | <integer>(20) COLON <integer>(39)
          | <integer>(28) COLON <integer>(39)

<integer> ::= Parsed INT_LIT
<integer> semantic rule: -549755813888 <= <integer> <= 549755813887
```

## Instruction Set Manual

[Manual](T02-Instrucciones_IAS.pdf)

## License

MIT License - see [LICENSE](LICENSE) file for details
