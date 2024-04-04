//
// Created by 侯鑫 on 2024/1/1.
//

#ifndef SSP_TOOLS_CONFIG_H
#define SSP_TOOLS_CONFIG_H

#include <string>
#include <map>
#include <iostream>
#include <vector>
#include "object.h"

namespace ssp::base {
using cstring = const std::string &;
using vstring = std::vector<std::string>;
using Section = std::map<std::string, std::string>;

class ConfigureManager : public Object {
public:
    explicit ConfigureManager(cstring confPath, cstring suffix = ".cf");

    void Init();

    void Dump(std::ostream &os) override;

public:
    void ChangeAccessPath(cstring confPath);

    [[nodiscard]] std::string GetString(cstring strDefault, cstring prefix) const;
    [[nodiscard]] bool        GetBool(bool nDefault, cstring prefix) const;
    [[nodiscard]] int         GetInt(int nDefault, cstring prefix) const;
    [[nodiscard]] float       GetFloat(float fDefault, cstring prefix) const;
    [[nodiscard]] double      GetDouble(double dDefault, cstring prefix) const;
    [[nodiscard]] long        GetLong(long lDefault, cstring prefix) const;
    [[nodiscard]] Section     GetSection(cstring prefix) const;

protected:
    void GetAllFiles(cstring path, cstring suffix);
    void InitFile(cstring file);

protected:
    vstring                            allConfFiles_;
    std::map<std::string, std::string> configKeyValMapper_;
    std::string                        prefix_;
    std::string                        suffix_;
};

} // namespace ssp::base

#endif //SSP_TOOLS_CONFIG_H
