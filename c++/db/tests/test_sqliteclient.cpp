#include "db/sqlitecli.h"
int main(int argc, char const *argv[]) {
    db::SqlPrepareStatement st("insert into test(id,name,desc) values(%,'%','%')");

    st.assignValue(12).assignValue("xhou").assignValue("description ok...");
    std::cout << st.getFullExecuteSql() << std::endl;

    db::SqliteClient client;

    client.opendb("./test.db");

    // std::cout << "Insert Row :" << client.exec(st) << std::endl;

    db::SqlPrepareStatement st1("select * from test");

    auto result = client.query<3>(st1);

    for (auto item : result) {
        for (auto val : item) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
    return 0;
}
