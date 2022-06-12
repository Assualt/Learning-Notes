#pragma once
#include <stdint.h>

namespace muduo {
namespace base {

enum DB_Type { Type_Null, Type_Sqlite, Type_LevelDB, Type_Mysql, Type_Redis };

class DBManager {
public:
    explicit DBManager(DB_Type type);

private:
    DB_Type m_type;
};

} // namespace base
} // namespace muduo