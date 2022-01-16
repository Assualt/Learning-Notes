#include "logging.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <map>
#include <memory>
#include <sstream>
namespace db {
using Func = std::function<void(const std::string &, const std::string &)>;
class LevelDBClient {
public:
    LevelDBClient()
        : m_DB(nullptr) {
    }

    bool open(const std::string &strDBPath, const leveldb::Options &options = leveldb::Options()) {
        leveldb::Status status = leveldb::DB::Open(options, strDBPath, &m_DB);
        if (status.ok())
            return true;
        logger.warning("try to repair db %s", strDBPath);
        status = leveldb::RepairDB(strDBPath, options);
        if (status.ok())
            return true;
        logger.warning("open db %s failed. errmsg:%s", strDBPath, status.ToString());
        return false;
    }

    std::string Get(const std::string &key, const leveldb::ReadOptions &option = leveldb::ReadOptions()) {
        if (m_DB == nullptr) {
            logger.warning("do such %s function before init leveldb ok.", __func__);
            return "";
        }
        std::string     resultString;
        leveldb::Status status = m_DB->Get(option, key, &resultString);
        if (!status.ok()) {
            logger.info("get key %s from leveldb failed. errmsg:%s", key, status.ToString());
            return "";
        }
        return resultString;
    }

    bool Delete(const std::string &key, const leveldb::WriteOptions &option = leveldb::WriteOptions()) {
        if (m_DB == nullptr) {
            logger.warning("do such %s function before init leveldb ok.", __func__);
            return false;
        }
        leveldb::Status status = m_DB->Delete(option, key);
        if (!status.ok()) {
            logger.info("get key %s from leveldb failed. errmsg:%s", key, status.ToString());
            return false;
        }
        return true;
    }

    bool Put(const std::string &key, const std::string &val, const leveldb::WriteOptions &option = leveldb::WriteOptions()) {
        if (m_DB == nullptr) {
            logger.warning("do such %s function before init leveldb ok.", __func__);
            return false;
        }
        leveldb::Status status = m_DB->Put(option, key, val);
        if (!status.ok()) {
            logger.info("put key %s with val %s from leveldb failed. errmsg:%s", key, val, status.ToString());
            return false;
        }
        return true;
    }

    bool Write(const std::map<std::string, std::string> &KeyMap, const leveldb::WriteOptions &option = leveldb::WriteOptions()) {
        if (m_DB == nullptr) {
            logger.warning("do such %s function before init leveldb ok.", __func__);
            return false;
        }
        leveldb::WriteBatch batch;
        std::for_each(KeyMap.begin(), KeyMap.end(), [&batch](std::pair<std::string, std::string> item) { batch.Put(item.first, item.second); });
        leveldb::Status status = m_DB->Write(option, &batch);
        if (!status.ok()) {
            logger.info("write batch kvmap size %d from leveldb failed. errmsg:%s", KeyMap.size(), status.ToString());
            return false;
        }
        return true;
    }

    std::string getDBStatus() const {
        if (m_DB == nullptr) {
            logger.warning("do such %s function before init leveldb ok.", __func__);
            return "";
        }
        std::stringstream ss;

        std::string value;
        if (m_DB->GetProperty("leveldb.num-files-at-level<N>", &value)) {
            ss << "leveldb.num-files-at-level<N>:" << value << std::endl;
            value.clear();
        }
        if (m_DB->GetProperty("leveldb.stats", &value)) {
            ss << "leveldb.stats:" << value << std::endl;
            value.clear();
        }
        if (m_DB->GetProperty("eveldb.sstables", &value)) {
            ss << "leveldb.sstables:" << value << std::endl;
        }
        return ss.str();
    }

    void Iterator(db::Func func, const leveldb::ReadOptions &option = leveldb::ReadOptions()) {
        if (m_DB == nullptr) {
            logger.warning("do such %s function before init leveldb ok.", __func__);
            return;
        }
        leveldb::Iterator *iter = m_DB->NewIterator(option);
        if (iter == nullptr) {
            logger.warning("create iter for leveldb failed.");
            return;
        }
        iter->SeekToFirst();
        while (iter->Valid()) {
            func(iter->key().ToString(), iter->value().ToString());
            iter->Next();
        }
    }

    ~LevelDBClient() {
        close();
    }

    void close() {
        if (m_DB)
            delete m_DB;
        m_DB = nullptr;
    }

protected:
    leveldb::DB *m_DB;
};

} // namespace db

int main(int argc, char const *argv[]) {

    db::LevelDBClient client;

    leveldb::Options options;
    options.create_if_missing = true;

    bool ok = client.open("test", options);

    if (!ok)
        return 0;

    client.Put("Hello", "World");
    client.Put("test", "OK");

    std::cout << client.Get("Hello") << std::endl;

    client.Iterator([](const std::string &key, const std::string &val) { std::cout << "Key:" << key << " Val:" << val << std::endl; });

    std::cout << client.getDBStatus() << std::endl;

    client.close();

    /* code */
    return 0;
}
