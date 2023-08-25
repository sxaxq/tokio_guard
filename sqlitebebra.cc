#include "sqlitebebra.hpp"
#include <iostream>

sqlite3* db;

void insertIP(std::string ip)
{
    std::string sql = "INSERT INTO banned_ip (ban_ip) VALUES (?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if(rc != SQLITE_OK) {
        std::cout << "failed insert ip to db" << std::endl;
        return;
    }
    sqlite3_bind_text(stmt, 1, ip.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE) {
        std::cout << "failed insert #2" << std::endl;
        return;
    }
    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", nullptr, 0, nullptr);
}

static int callback(void* data, int argc, char** argv, char** azColName) 
{
    int* count = (int*)data;
    *count = atoi(argv[0]);
    return 0;
}

bool existsInDb(std::string ip) 
{
    std::string sql = "SELECT COUNT(id) FROM banned_ip WHERE ban_ip = '" + ip + "';";
    int count = 0;
    sqlite3_exec(db, sql.c_str(), callback, &count, nullptr);
    return (count > 0);
}

int data_base_init(const std::string& db_name)
{
    char* zErrMsg = 0;
    int rc = sqlite3_open(db_name.c_str(), &db);
    if(rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    else {
        fprintf(stderr, "opened database successfully\n");
    }

    std::string sql = "CREATE TABLE IF NOT EXISTS banned_ip ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT," 
                      "ban_ip TEXT NOT NULL);";
    
    rc = sqlite3_exec(db, sql.c_str(), nullptr, 0, &zErrMsg);

    if(rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
    }
    else {
        fprintf(stdout, "table created successfully\n");
    }

    insertIP("212323");

    return 0;
}