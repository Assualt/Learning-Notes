#include <functional>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <map>

namespace db {
using Func = std::function<void(const std::string &, const std::string &)>;
class LevelDBClient {
public:
    LevelDBClient();

    bool open(const std::string &strDBPath, const leveldb::Options &options = leveldb::Options());

    std::string Get(const std::string &key, const leveldb::ReadOptions &option = leveldb::ReadOptions());

    bool Delete(const std::string &key, const leveldb::WriteOptions &option = leveldb::WriteOptions());

    bool Put(const std::string &key, const std::string &val,
             const leveldb::WriteOptions &option = leveldb::WriteOptions());

    bool Write(const std::map<std::string, std::string> &KeyMap,
               const leveldb::WriteOptions &             option = leveldb::WriteOptions());

    std::string getDBStatus() const;

    void Iterator(db::Func func, const leveldb::ReadOptions &option = leveldb::ReadOptions());

    ~LevelDBClient();

    void close();

protected:
    leveldb::DB *m_DB;
};

} // namespace db