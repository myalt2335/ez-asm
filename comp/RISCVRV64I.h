#pragma once
#include "../compiler/AsmDefinition.h"

inline AsmDefinition RISCVRV64I(
    "RISC-V",
    "RV64I",
    // RV64 has the same 32 integer regs x0..x31
    {"x0","x1","x2","x3","x4","x5","x6","x7","x8","x9","x10","x11","x12","x13","x14","x15",
     "x16","x17","x18","x19","x20","x21","x22","x23","x24","x25","x26","x27","x28","x29","x30","x31"},
    {
        // Arithmetic and logic (XLEN = 64) 
        {"add",   "{d} = {s1} + {s2};"},
        {"sub",   "{d} = {s1} - {s2};"},
        {"sll",   "{d} = {s1} << ({s2} & 0x3F);"},
        {"slt",   "{d} = ({s1} < {s2}) ? 1 : 0;"},
        {"sltu",  "{d} = ((uint64_t){s1} < (uint64_t){s2}) ? 1 : 0;"},
        {"xor",   "{d} = {s1} ^ {s2};"},
        {"srl",   "{d} = ((uint64_t){s1}) >> ({s2} & 0x3F);"},
        {"sra",   "{d} = {s1} >> ({s2} & 0x3F);"},
        {"or",    "{d} = {s1} | {s2};"},
        {"and",   "{d} = {s1} & {s2};"},

        // 64-bit "W" register forms (write sign-extended 32-bit result) 
        {"addw",  "{d} = (int64_t)(int32_t)({s1} + {s2});"},
        {"subw",  "{d} = (int64_t)(int32_t)({s1} - {s2});"},
        {"sllw",  "{d} = (int64_t)(int32_t)(((uint32_t){s1}) << ({s2} & 0x1F));"},
        {"srlw",  "{d} = (int64_t)(int32_t)(((uint32_t){s1}) >> ({s2} & 0x1F));"},
        {"sraw",  "{d} = (int64_t)(int32_t)(((int32_t){s1}) >> ({s2} & 0x1F));"},

        // Immediate arithmetic (XLEN = 64) 
        {"addi",  "{d} = {s1} + {imm};"},
        {"slti",  "{d} = ({s1} < {imm}) ? 1 : 0;"},
        {"sltiu", "{d} = ((uint64_t){s1} < (uint64_t){imm}) ? 1 : 0;"},
        {"xori",  "{d} = {s1} ^ {imm};"},
        {"ori",   "{d} = {s1} | {imm};"},
        {"andi",  "{d} = {s1} & {imm};"},
        {"slli",  "{d} = {s1} << ({imm} & 0x3F);"},
        {"srli",  "{d} = ((uint64_t){s1}) >> ({imm} & 0x3F);"},
        {"srai",  "{d} = {s1} >> ({imm} & 0x3F);"},
        // 64-bit "W" immediate forms 
        {"addiw", "{d} = (int64_t)(int32_t)({s1} + {imm});"},
        {"slliw", "{d} = (int64_t)(int32_t)(((uint32_t){s1}) << ({imm} & 0x1F));"},
        {"srliw", "{d} = (int64_t)(int32_t)(((uint32_t){s1}) >> ({imm} & 0x1F));"},
        {"sraiw", "{d} = (int64_t)(int32_t)(((int32_t){s1}) >> ({imm} & 0x1F));"},

        // Load / store 
        // Note: On RV64, LW sign-extends; LWU zero-extends; LD is 64-bit.
        {"lb",  "{d} = (int64_t)(int8_t)(*(int8_t*)({s1}));"},
        {"la",  "{d} = (uintptr_t){s1};"},
        {"lh",  "{d} = (int64_t)(int16_t)(*(int16_t*)({s1}));"},
        {"lw",  "{d} = (int64_t)(int32_t)(*(int32_t*)({s1}));"},
        {"lbu", "{d} = (uint64_t)(uint8_t)(*(uint8_t*)({s1}));"},
        {"lhu", "{d} = (uint64_t)(uint16_t)(*(uint16_t*)({s1}));"},
        {"lwu", "{d} = (uint64_t)(*(uint32_t*)({s1}));"},
        {"ld",  "{d} = *(uint64_t*)({s1});"},

        {"sb",  "*(uint8_t*)({s1})  = (uint8_t){s2};"},
        {"sh",  "*(uint16_t*)({s1}) = (uint16_t){s2};"},
        {"sw",  "*(uint32_t*)({s1}) = (uint32_t){s2};"},
        {"sd",  "*(uint64_t*)({s1}) = {s2};"},

        // Control flow 
        {"beq",   "if ({s1} == {s2}) goto {label};"},
        {"bne",   "if ({s1} != {s2}) goto {label};"},
        {"blt",   "if ({s1} < {s2}) goto {label};"},
        {"bge",   "if ({s1} >= {s2}) goto {label};"},
        {"bltu",  "if ((uint64_t){s1} < (uint64_t){s2}) goto {label};"},
        {"bgeu",  "if ((uint64_t){s1} >= (uint64_t){s2}) goto {label};"},
        {"jal",   "{d} = PC + 4; goto {label};"},
        {"jalr",  "{d} = PC + 4; PC = ({s1} + {imm}) & ~1;"},

        // Upper immediates 
        {"lui",   "{d} = (uint64_t){imm} << 12;"},
        {"auipc", "{d} = PC + ((uint64_t){imm} << 12);"},

        // System 
        {"ecall", "system_call();"},
        {"ebreak","debug_break();"},

        // Pseudo / convenience (compiler-level) 
        // Note: 'sext.w' is commonly a pseudo for ADDIW rd, rs, 0 on RV64
        {"sext.w","{d} = (int64_t)(int32_t){s1};"},
        {"print", "a7 = 4; a0 = (intptr_t){s1}; system_call();"},
        {"exit", "a0 = (uint64_t){s1}; a7 = 93; system_call();"},
        {"li",    "{d} = (uint64_t){imm};"},
        {"mv",    "{d} = {s1};"}
    }
);
