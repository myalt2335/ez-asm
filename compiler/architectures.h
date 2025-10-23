#pragma once
#include "../comp/RISCVRV32I.h"
#include "../comp/RISCVRV64I.h"
#include "../comp/MIPS32.h"

inline std::vector<AsmDefinition*> architectures = {
    &RISCVRV32I,
    &RISCVRV64I,
    &MIPS32
};