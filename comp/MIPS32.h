#pragma once
#include "../compiler/AsmDefinition.h"
#include <stdint.h>
#include <stddef.h>

inline AsmDefinition MIPS32(
    "MIPS",
    "32",
    // Registers (GPRs)
    {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
     "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
     "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
     "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"},
    {
        //  Arithmetic 
        {"add",   "{d} = {s1} + {s2};"},
        {"addu",  "{d} = (int32_t)((uint32_t){s1} + (uint32_t){s2});"},
        {"sub",   "{d} = {s1} - {s2};"},
        {"subu",  "{d} = (int32_t)((uint32_t){s1} - (uint32_t){s2});"},

        // multiply (signed/unsigned)
        {"mult",  "{ int64_t prod=(int64_t){s1}*(int64_t){s2}; LO=(int32_t)prod; HI=(int32_t)(prod>>32); }"},
        {"multu", "{ uint64_t prod=(uint64_t)(uint32_t){s1}*(uint64_t)(uint32_t){s2}; LO=(int32_t)(uint32_t)prod; HI=(int32_t)(uint32_t)(prod>>32); }"},
        {"div",   "if((int32_t){s2}!=0){ LO = (int32_t){s1} / (int32_t){s2}; HI = (int32_t){s1} % (int32_t){s2}; }"},
        {"divu",  "if((uint32_t){s2}!=0){ LO = (int32_t)((uint32_t){s1} / (uint32_t){s2}); HI = (int32_t)((uint32_t){s1} % (uint32_t){s2}); }"},

        // move from/to HI/LO
        {"mflo",  "{d} = LO;"},
        {"mfhi",  "{d} = HI;"},
        {"mtlo",  "LO = {s1};"},
        {"mthi",  "HI = {s1};"},

        // set-on-less-than
        {"slt",   "{d} = ((int32_t){s1} < (int32_t){s2}) ? 1 : 0;"},
        {"sltu",  "{d} = ((uint32_t){s1} < (uint32_t){s2}) ? 1 : 0;"},

        //  Logical 
        {"and",   "{d} = {s1} & {s2};"},
        {"or",    "{d} = {s1} | {s2};"},
        {"xor",   "{d} = {s1} ^ {s2};"},
        {"nor",   "{d} = ~({s1} | {s2});"},

        //  Shifts 
        {"sll",   "{d} = (int32_t)((uint32_t){s1} << ({imm} & 0x1F));"},
        {"srl",   "{d} = (int32_t)((uint32_t){s1} >> ({imm} & 0x1F));"},
        {"sra",   "{d} = (int32_t)((int32_t){s1} >> ({imm} & 0x1F));"},
        {"sllv",  "{d} = (int32_t)((uint32_t){s1} << ((uint32_t){s2} & 0x1F));"},
        {"srlv",  "{d} = (int32_t)((uint32_t){s1} >> ((uint32_t){s2} & 0x1F));"},
        {"srav",  "{d} = (int32_t)((int32_t){s1} >> ((uint32_t){s2} & 0x1F));"},

        // Immediates 
        {"addi",  "{d} = (int32_t)((int32_t){s1} + (int32_t)(int16_t){imm});"},
        {"addiu", "{d} = (int32_t)((uint32_t){s1} + (uint32_t)(int16_t){imm});"},
        {"slti",  "{d} = ((int32_t){s1} < (int32_t)(int16_t){imm}) ? 1 : 0;"},
        {"sltiu", "{d} = ((uint32_t){s1} < (uint32_t)(int16_t){imm}) ? 1 : 0;"},
        {"andi",  "{d} = (int32_t)((uint32_t){s1} & (uint32_t)(uint16_t){imm});"},
        {"ori",   "{d} = (int32_t)((uint32_t){s1} | (uint32_t)(uint16_t){imm});"},
        {"xori",  "{d} = (int32_t)((uint32_t){s1} ^ (uint32_t)(uint16_t){imm});"},
        {"lui",   "{d} = (int32_t)((uint32_t)(uint16_t){imm} << 16);"},

        // Memory (sign/zero extension explicit) 
        {"lw",    "{d} = *(int32_t*)((intptr_t){s1} + (int32_t)(int16_t){imm});"},
        {"sw",    "*(int32_t*)((intptr_t){s1} + (int32_t)(int16_t){imm}) = {s2};"},

        {"lb",    "{d} = (int32_t)(int8_t)(*(int8_t*)((intptr_t){s1} + (int32_t)(int16_t){imm}));"},
        {"lbu",   "{d} = (int32_t)(uint32_t)(*(uint8_t*)((intptr_t){s1} + (int32_t)(int16_t){imm}));"},
        {"sb",    "*(int8_t*)((intptr_t){s1} + (int32_t)(int16_t){imm}) = (int8_t){s2};"},

        {"lh",    "{d} = (int32_t)(int16_t)(*(int16_t*)((intptr_t){s1} + (int32_t)(int16_t){imm}));"},
        {"lhu",   "{d} = (int32_t)(uint32_t)(*(uint16_t*)((intptr_t){s1} + (int32_t)(int16_t){imm}));"},
        {"sh",    "*(int16_t*)((intptr_t){s1} + (int32_t)(int16_t){imm}) = (int16_t){s2};"},

        // Branching & Jumps
        {"beq",   "if ({s1} == {s2}) goto {label};"},
        {"bne",   "if ({s1} != {s2}) goto {label};"},
        {"bgtz",  "if ((int32_t){s1} >  0) goto {label};"},
        {"bltz",  "if ((int32_t){s1} <  0) goto {label};"},
        {"bgez",  "if ((int32_t){s1} >= 0) goto {label};"},
        {"blez",  "if ((int32_t){s1} <= 0) goto {label};"},

        {"j",     "goto {label};"},
        {"jal",   "$ra = (intptr_t)&&ret_label; goto {label}; ret_label:"},
        {"jalr",  "{d} = (intptr_t)&&ret_label; goto *(void*)(intptr_t){s1}; ret_label:"},
        {"jr",    "goto *(void*)(intptr_t){s1};"},

        // System / Misc 
        {"syscall", "system_call();"},
        {"break",   "/* breakpoint */"},
        {"nop",     "/* nop */"},

        // Pseudos / Convenience 
        {"li",    "{d} = (int32_t){imm};"},
        {"la",    "{d} = (intptr_t){s1};"},
        {"move",  "{d} = {s1};"},

        // Demo I/O helpers (convention-based)
        {"print", "$v0 = 4; $a0 = (intptr_t){s1}; system_call();"},
        {"exit",  "$v0 = 10; system_call();"}
    }
);
