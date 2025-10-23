#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct AsmDefinition {
    std::string GT;
    std::string SBST;

    std::vector<std::string> traits;
    std::unordered_map<std::string, std::string> dict;
    int definitionCount;

    AsmDefinition(std::string gt,
                  std::string sbst,
                  std::vector<std::string> t,
                  std::unordered_map<std::string, std::string> d)
        : GT(std::move(gt)),
          SBST(std::move(sbst)),
          traits(std::move(t)),
          dict(std::move(d)),
          definitionCount(static_cast<int>(dict.size())) {}

    std::string fullName() const {
        return SBST.empty() ? GT : (GT + " " + SBST);
    }
};
