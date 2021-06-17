#include <iostream>
#include "db/leveldbcli.h"

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
