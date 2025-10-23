#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <cstdlib>
#include <algorithm>
#include "compiler/architectures.h"

struct DataSymbol { std::string name, ctype, value; };

std::string readText(const std::string& path) {
    std::ifstream in(path);
    std::stringstream ss; ss << in.rdbuf();
    return ss.str();
}

static inline bool isIdentStart(char c){ return std::isalpha((unsigned char)c) || c=='_'; }
static inline bool isIdentChar(char c){ return std::isalnum((unsigned char)c) || c=='_'; }
static inline bool contains(const std::string& s, const char* sub){ return s.find(sub)!=std::string::npos; }

static inline std::string toLower(std::string s){
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

static inline std::string normalizeArchKey(std::string s) {
    s = toLower(std::move(s));
    s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char c){
        return c==' ' || c=='-' || c=='_';
    }), s.end());
    return s;
}

static inline bool archKeyMatches(const AsmDefinition* def, const std::string& specRaw) {
    const std::string spec = normalizeArchKey(specRaw);
    const std::string kFull = normalizeArchKey(def->fullName());
    const std::string kGT   = normalizeArchKey(def->GT);
    const std::string kSBST = normalizeArchKey(def->SBST);
    if (spec == kFull || spec == kGT || (!kSBST.empty() && spec == kSBST)) return true;
    if (!def->SBST.empty()) {
        std::string concat = kGT + kSBST;
        if (spec == concat) return true;
    }
    return false;
}

static inline AsmDefinition* findArchBySpec(const std::string& spec) {
    for (auto* def : architectures)
        if (archKeyMatches(def, spec)) return def;
    return nullptr;
}

AsmDefinition* guessArchitecture(const std::string& source) {
    std::unordered_map<const AsmDefinition*, int> scores;
    std::istringstream words(source);
    std::string tok;
    while (words >> tok) {
        for (auto* def : architectures) {
            if (def->dict.count(tok)) scores[def] += 3;
            for (auto& r : def->traits)
                if (tok == r) scores[def] += 1;
        }
    }
    AsmDefinition* best = nullptr;
    int bestScore = 0;
    for (auto* def : architectures) {
        int s = scores[def];
        if (s > bestScore) { bestScore = s; best = def; }
    }
    return best;
}

std::vector<DataSymbol> parseDataSection(const std::string& src, std::set<std::string>& dataLabels) {
    std::vector<DataSymbol> data;
    std::istringstream ss(src);
    std::string line;
    bool inData = false;
    while (std::getline(ss, line)) {
        size_t a = line.find_first_not_of(" \t"); if (a==std::string::npos) continue;
        size_t b = line.find_last_not_of(" \t\r\n"); line = line.substr(a, b-a+1);
        if (line.empty() || line[0]=='#') continue;
        if (line.find(".data")!=std::string::npos) { inData = true; continue; }
        if (line.find(".text")!=std::string::npos) { inData = false; break; }
        if (!inData) continue;
        size_t colon = line.find(':'); if (colon==std::string::npos) continue;
        std::string name = line.substr(0, colon);
        size_t n0 = name.find_first_not_of(" \t"); size_t n1 = name.find_last_not_of(" \t");
        name = name.substr(n0, n1-n0+1);
        dataLabels.insert(name);
        std::string rest = line.substr(colon+1);
        size_t r0 = rest.find_first_not_of(" \t"); if (r0==std::string::npos) continue;
        rest = rest.substr(r0);
        std::istringstream lr(rest);
        std::string directive; lr >> directive;
        std::string after; std::getline(lr, after);
        size_t a0 = after.find_first_not_of(" \t"); if (a0!=std::string::npos) after = after.substr(a0); else after.clear();
        if (directive==".word") {
            data.push_back({name,"uint32_t", after});
        } else if (directive==".dword") {
            data.push_back({name,"uint64_t", after});
        } else if (directive==".asciiz") {
            std::string value; 
            size_t q1 = after.find('"');
            if (q1!=std::string::npos) {
                size_t q2 = after.find_last_of('"');
                if (q2!=std::string::npos && q2>q1) value = after.substr(q1, q2-q1+1);
            }
            if (value.empty()) value = "\"" + after + "\"";
            data.push_back({name,"char[]", value});
        }
    }
    return data;
}

std::set<std::string> collectTextLabels(const std::string& src) {
    std::set<std::string> labels;
    std::istringstream ss(src);
    std::string line; bool inText=false;
    while (std::getline(ss, line)) {
        if (line.find(".text")!=std::string::npos) { inText=true; continue; }
        if (!inText) continue;
        size_t a = line.find_first_not_of(" \t"); if (a==std::string::npos) continue;
        size_t b = line.find_last_not_of(" \r\n"); std::string t = line.substr(a, b-a+1);
        if (!t.empty() && t.back()==':') {
            t.pop_back();
            size_t i=0; while (i<t.size() && (t[i]==' '||t[i]=='\t')) ++i;
            std::string name = t.substr(i);
            if (!name.empty()) labels.insert(name);
        }
    }
    return labels;
}

bool needsPC(const std::string& src) {
    std::istringstream ss(src);
    std::string line, op;
    while (std::getline(ss, line)) {
        std::istringstream is(line);
        if (is >> op) if (op=="auipc"||op=="jal"||op=="jalr") return true;
    }
    return false;
}

std::set<std::string> collectSymbols(const std::string& src, const std::set<std::string>& allLabels) {
    std::set<std::string> syms;
    std::istringstream ss(src);
    std::string line; bool inText=false;
    while (std::getline(ss, line)) {
        if (line.find(".text")!=std::string::npos) { inText=true; continue; }
        if (!inText) continue;
        std::istringstream is(line);
        std::string op,d,s1,s2; is >> op >> d >> s1 >> s2;
        auto norm = [&](std::string& t){
            while (!t.empty() && (t.back()==',' || t.back()==' ' || t.back()=='\t'))
            t.pop_back();
        };
        norm(d); norm(s1); norm(s2);
        auto add=[&](const std::string& t){
            if (t.empty()) return;
            if (t[0]=='.') return;
            if (t.find('"')!=std::string::npos) return;
            if (allLabels.count(t)) return;
            if (!isIdentStart(t[0])) return;
            for(char c: t) if(!isIdentChar(c)) return;
            syms.insert(t);
        };
        add(d); add(s1); add(s2);
    }
    return syms;
}

std::string resolveOperand(std::string tok, const std::set<std::string>& allLabels) {
    if (tok.empty()) return tok;
    while (!tok.empty() && (tok.back()==',' || tok.back()==' ' || tok.back()=='\t'))
        tok.pop_back();

    size_t lp = tok.find('(');
    size_t rp = tok.find(')');
    if (lp != std::string::npos && rp != std::string::npos && rp > lp) {
        std::string imm = tok.substr(0, lp);
        std::string reg = tok.substr(lp + 1, rp - lp - 1);
        auto trim = [](std::string s){
            size_t a = s.find_first_not_of(" \t");
            size_t b = s.find_last_not_of(" \t");
            return (a==std::string::npos)?std::string():s.substr(a,b-a+1);
        };
        imm = trim(imm);
        reg = trim(reg);
        if (imm.empty()) imm = "0";
        return "(" + reg + " + " + imm + ")";
    }

    if (allLabels.count(tok))
        return "(uintptr_t)&" + tok;

    return tok;
}

std::string translateLine(const std::string& raw, const AsmDefinition* def, const std::set<std::string>& allLabels) {
    auto sanitize = [](std::string s) {
        for (char& c : s)
            if (!std::isalnum((unsigned char)c) && c != '_') c = '_';
        return s;
    };
    auto maybeSanitize = [&](std::string t) -> std::string {
        if (t.empty()) return t;
        if (std::isdigit((unsigned char)t[0])) return t;
        if (t[0] == 'x' && t.size() <= 4) return t;
        if (t.find("x") != std::string::npos) return t;
        if (t.find("(intptr_t)") != std::string::npos) return t;
        if (t.find("(uintptr_t)") != std::string::npos) return t;
        if (t.find("&") != std::string::npos) return t;
        if (t.find("+") != std::string::npos || t.find("-") != std::string::npos) return t;
        return sanitize(t);
    };
    std::string line = raw;
    size_t h=line.find('#'); if(h!=std::string::npos) line=line.substr(0,h);
    size_t sc=line.find(';'); if(sc!=std::string::npos) line=line.substr(0,sc);
    while(!line.empty() && (line.back()==' '||line.back()=='\t'||line.back()=='\r')) line.pop_back();
    size_t l=0; while(l<line.size() && (line[l]==' '||line[l]=='\t')) ++l;
    if (l>=line.size()) return "";
    std::string s = line.substr(l);
    if (s.back()==':') return "";
    std::istringstream iss(s);
    std::string opcode; iss >> opcode;
    if (opcode==".globl"||opcode==".section"||opcode==".text"||opcode==".data") return "";
    auto it = def->dict.find(opcode);
    if (it==def->dict.end()) return "";
    std::string tmpl = it->second;
    std::string a,b,c; iss >> a >> b >> c;
    // !!!Do not strip closing parenthesis here!!! resolveOperand needs the full "imm(base)" form (e.g., "0(x1)") to transform it into a valid C expression like "(x1 + 0)".
    auto norm=[&](std::string& t){
        if (!t.empty() && t.back()==',') t.pop_back();
    };
    if (opcode=="sb" || opcode=="sh" || opcode=="sw" || opcode=="sd") {
        if (c.empty() && !a.empty() && !b.empty()) {
            c = a;
            a.clear();
        }
    }
    norm(a); norm(b); norm(c);
    a=resolveOperand(a,allLabels);
    b=resolveOperand(b,allLabels);
    c=resolveOperand(c,allLabels);
    a = maybeSanitize(a);
    b = maybeSanitize(b);
    c = maybeSanitize(c);
    bool hasD   = contains(tmpl,"{d}");
    bool hasS1  = contains(tmpl,"{s1}");
    if (!hasD && hasS1 && a.size() && b.empty()) { b=a; a.clear(); }
    auto repl=[&](const std::string& key,const std::string& val){
        size_t p=0;
        while((p=tmpl.find(key,p))!=std::string::npos){
            tmpl.replace(p,key.size(),val);
            p+=val.size();
        }
    };
    if (tmpl.find("{imm}") != std::string::npos && c.empty() && !b.empty()) c = b;
    if (tmpl.find("{label}") != std::string::npos && c.empty() && !b.empty()) c = b;
    repl("{d}",  a);
    repl("{s1}", b);
    repl("{s2}", c);
    repl("{imm}", c);
    repl("{addr}", c);
    repl("{label}", c);
    return tmpl + "\n";
}

static void printArchitecturesGrouped() {
    std::unordered_map<std::string, std::vector<const AsmDefinition*>> byGT;
    for (auto* def : architectures)
        byGT[def->GT].push_back(def);
    for (auto& [gt, defs] : byGT) {
        std::cout << gt << "\n";
        for (auto* def : defs) {
            std::string subset = def->SBST.empty() ? "(generic)" : def->SBST;
            std::cout << " - " << subset << " (" << def->definitionCount << " defs)\n";
        }
        std::cout << "\n";
    }
}

bool requiresRuntime(const AsmDefinition* def) {
    for (auto& kv : def->dict) {
        const std::string& t = kv.second;
        if (t.find("system_call")!=std::string::npos || t.find("debug_break")!=std::string::npos) return true;
    }
    return false;
}

std::string getOutputName(const std::string& inputPath) {
    size_t dot = inputPath.find_last_of('.');
    size_t slash = inputPath.find_last_of("/\\");
    std::string stem = (slash==std::string::npos) ? inputPath.substr(0,dot) : inputPath.substr(slash+1, dot - slash - 1);
    return stem + ".exe";
}

std::string detectArchHint(const std::string& source) {
    std::istringstream ss(source);
    std::string line;
    while (std::getline(ss, line)) {
        size_t start = line.find_first_not_of(" \t\r");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        std::string trimmed = line.substr(start, end - start + 1);
        if (trimmed.empty()) continue;
        if (trimmed.rfind(";!", 0) == 0 && trimmed.find("!;") != std::string::npos) {
            size_t a = 2;
            size_t b = trimmed.find("!;", a);
            std::string inner = trimmed.substr(a, b - a);
            size_t i0 = inner.find_first_not_of(" \t");
            size_t i1 = inner.find_last_not_of(" \t");
            if (i0 != std::string::npos)
                inner = inner.substr(i0, i1 - i0 + 1);
            return inner;
        }
        break;
    }
    return "";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "EZM 1.A018.22.251023\n"
                  << "Usage: ezm [options] <file.ezm>\n\n"
                  << "Options:\n"
                  << "  -arch <name>   Force architecture (e.g. \"RISC-V RV32I\")\n"
                  << "  -k             Keep temp.c after compilation\n\n"
                  << "Architectures:\n";
            printArchitecturesGrouped();
        return 0;
    }
    bool keepTemp = false;
    bool runAfter = false;
    std::string archName, filePath;
    for (int i=1; i<argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-k") { keepTemp = true; continue; }
        if (arg == "-r") { runAfter = true; continue; }
        if (arg == "-arch" && i+1 < argc) { archName = argv[++i]; continue; }
        if (arg[0] != '-') { filePath = arg; }
    }
    if (filePath.empty()) { std::cerr << "No input file.\n"; return 1; }
    std::string source = readText(filePath);
    AsmDefinition* arch = nullptr;
    if (!archName.empty()) {
        arch = findArchBySpec(archName);
        if (!arch) {
            std::cerr << "Unknown architecture: " << archName << "\n";
            return 1;
        }
    } else {
        std::string hintedArch = detectArchHint(source);
        if (!hintedArch.empty()) {
            arch = findArchBySpec(hintedArch);
            if (!arch)
                std::cerr << "Warning: Unknown architecture hint \"" << hintedArch << "\" — ignoring.\n";
        }
        if (!arch)
            arch = guessArchitecture(source);
    }
    if (!arch) {
        std::cerr << "Could not determine architecture from syntax.\n";
        printArchitecturesGrouped();
        return 1;
    }
    std::set<std::string> dataLabels;
    auto data = parseDataSection(source, dataLabels);
    auto textLabels = collectTextLabels(source);
    std::set<std::string> allLabels = dataLabels; allLabels.insert(textLabels.begin(), textLabels.end());
    auto symbols = collectSymbols(source, allLabels);
    bool runtime = requiresRuntime(arch);
    if (runtime) {
        std::string gtLower = toLower(arch->GT);
        if (gtLower.find("mips") != std::string::npos) {
            symbols.insert("$v0");
            symbols.insert("$a0");
        } else if (gtLower.find("risc") != std::string::npos) {
            symbols.insert("a0");
            symbols.insert("a7");
        }
    }
    std::vector<std::string> translatedBody;
    translatedBody.reserve(256);
    bool usesMem   = false;
    bool usesMem64 = false;
    bool usesPCVar = needsPC(source);
    {
        std::istringstream src1(source);
        std::string line; bool inText=false;
        while (std::getline(src1, line)) {
            if (line.find(".text")!=std::string::npos) { inText=true; continue; }
            if (!inText) continue;
            std::string emitted = translateLine(line, arch, allLabels);
            if (!emitted.empty()) {
                if (emitted.find("mem[")   != std::string::npos) usesMem = true;
                if (emitted.find("mem64[") != std::string::npos) usesMem64 = true;
                if (emitted.find("PC")     != std::string::npos) usesPCVar = true;
                translatedBody.push_back(std::move(emitted));
            }
        }
    }
    std::string outputName = getOutputName(filePath);
    std::ofstream out("temp.c");
    out << "#include <stdio.h>\n#include <stdlib.h>\n#include <stdint.h>\n\n";
    for (auto& d : data) {
        if (d.ctype=="uint32_t") out << "uint32_t " << d.name << " = " << d.value << ";\n";
        else if (d.ctype=="uint64_t") out << "uint64_t " << d.name << " = " << d.value << ";\n";
        else out << "char " << d.name << "[] = " << d.value << ";\n";
    }
    auto sanitize = [](std::string s) {
        for (char& c : s)
            if (!std::isalnum((unsigned char)c) && c != '_') c = '_';
        return s;
    };
    for (auto& s : symbols)
        out << "intptr_t " << sanitize(s) << " = 0;\n";
    if (usesMem || usesMem64) {
        out << "\n#ifndef MEM_SIZE\n#define MEM_SIZE 65536\n#endif\n";
        out << "uint8_t  mem[MEM_SIZE];\n";
        out << "uint32_t mem32[MEM_SIZE / 4];\n";
        out << "uint64_t mem64[MEM_SIZE / 8];\n";
    }
    if (usesPCVar) {
        out << "intptr_t PC = 0;\n";
    }
    if (runtime) {
        std::string gtLower = toLower(arch->GT);
        if (gtLower.find("mips") != std::string::npos) {
            out 
            << "\nvoid system_call(){\n"
            << "    switch(_v0){\n"
            << "        case 4: printf(\"%s\", (char*)_a0); break;\n"
            << "        case 10: exit(0); break;\n"
            << "        default: printf(\"[unknown syscall %d]\\n\", (int)_v0); break;\n"
            << "    }\n}\n\n";
        } else {
            out 
            << "\nvoid system_call(){\n"
            << "    switch(a7){\n"
            << "        case 4:  printf(\"%s\", (char*)a0); break;\n"
            << "        case 93: exit((int)a0); break;\n"
            << "        default: printf(\"[unknown syscall %d]\\n\", (int)a7); break;\n"
            << "    }\n}\n\n";
        }
    }
    out << "int main(){\n";
    for (const auto& ln : translatedBody) {
        out << "    " << ln;
    }
    out << "    return 0;\n}\n";
    out.close();
    std::cout << "Architecture: " << arch->fullName() << " (" << arch->definitionCount << " defs)\n";
    if (!runAfter)
        std::cout << "Compiling temp.c -> " << outputName << " ...\n";
    std::string cmd = "gcc temp.c -o \"" + outputName + "\"";
    system(cmd.c_str());
    if (!keepTemp) std::remove("temp.c");
    if (runAfter) {
    #ifdef _WIN32
        std::string runCmd = outputName;
    #else
        std::string runCmd = "./" + outputName;
    #endif
        system(runCmd.c_str());
        std::remove(outputName.c_str());
    } else {
        std::cout << "Done. Run ./" << outputName << "\n";
    }
    return 0;
}

