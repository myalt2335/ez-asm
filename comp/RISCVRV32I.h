#pragma once
#include "../compiler/AsmDefinition.h"

inline AsmDefinition RISCVRV32I(
    "RISC-V",
    "RV32I",
    {"x0","x1","x2","x3","x4","x5","x6","x7","x8","x9","x10","x11","x12","x13","x14","x15",
     "x16","x17","x18","x19","x20","x21","x22","x23","x24","x25","x26","x27","x28","x29","x30","x31"},
    {
        // Arithmetic and logic
        {"add",   "{d} = {s1} + {s2};"},
        {"sub",   "{d} = {s1} - {s2};"},
        {"sll",   "{d} = {s1} << ({s2} & 0x1F);"},
        {"slt",   "{d} = ({s1} < {s2}) ? 1 : 0;"},
        {"sltu",  "{d} = ((uint32_t){s1} < (uint32_t){s2}) ? 1 : 0;"},
        {"xor",   "{d} = {s1} ^ {s2};"},
        {"srl",   "{d} = ((uint32_t){s1}) >> ({s2} & 0x1F);"},
        {"sra",   "{d} = {s1} >> ({s2} & 0x1F);"},
        {"or",    "{d} = {s1} | {s2};"},
        {"and",   "{d} = {s1} & {s2};"},

        // Immediate arithmetic
        {"addi",  "{d} = {s1} + {imm};"},
        {"slti",  "{d} = ({s1} < {imm}) ? 1 : 0;"},
        {"sltiu", "{d} = ((uint32_t){s1} < (uint32_t){imm}) ? 1 : 0;"},
        {"xori",  "{d} = {s1} ^ {imm};"},
        {"ori",   "{d} = {s1} | {imm};"},
        {"andi",  "{d} = {s1} & {imm};"},
        {"slli",  "{d} = {s1} << ({imm} & 0x1F);"},
        {"srli",  "{d} = ((uint32_t){s1}) >> ({imm} & 0x1F);"},
        {"srai",  "{d} = {s1} >> ({imm} & 0x1F);"},

        // Load / store
        {"lb",    "{d} = (int8_t)mem[{addr}];"},
        {"lh",    "{d} = (int16_t)mem[{addr}];"},
        {"lw",    "{d} = mem[{addr}];"},
        {"lbu",   "{d} = (uint8_t)mem[{addr}];"},
        {"lhu",   "{d} = (uint16_t)mem[{addr}];"},
        {"sb",    "mem[{addr}] = (uint8_t){s};"},
        {"sh",    "mem[{addr}] = (uint16_t){s};"},
        {"sw",    "mem[{addr}] = {s};"},

        // Control flow
        {"beq",   "if ({s1} == {s2}) goto {label};"},
        {"bne",   "if ({s1} != {s2}) goto {label};"},
        {"blt",   "if ({s1} < {s2}) goto {label};"},
        {"bge",   "if ({s1} >= {s2}) goto {label};"},
        {"bltu",  "if ((uint32_t){s1} < (uint32_t){s2}) goto {label};"},
        {"bgeu",  "if ((uint32_t){s1} >= (uint32_t){s2}) goto {label};"},
        {"jal",   "{d} = PC + 4; goto {label};"},
        {"jalr",  "{d} = PC + 4; PC = ({s1} + {imm}) & ~1;"},

        // Upper immediates
        {"lui",   "{d} = {imm} << 12;"},
        {"auipc", "{d} = PC + ({imm} << 12);"},

        // System
        {"ecall", "system_call();"},
        {"ebreak","debug_break();"},

        // Macros a compiler would likely have? I hope
        {"print", "a7 = 4; a0 = (intptr_t){s1}; system_call();"},
        // RISC-V Linux ABI: exit is syscall 93. Default to status 0.
        {"exit",  "a0 = 0; a7 = 93; system_call();"},
        {"li",    "{d} = {imm};"},
        {"mv",    "{d} = {s1};"}
    }
);

