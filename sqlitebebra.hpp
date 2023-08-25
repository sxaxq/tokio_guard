#ifndef SQLITEBEBRA_HPP
#define SQLITEBEBRA_HPP

#include <sqlite3.h>
#include <string>

extern sqlite3* db;

void insertIP(std::string ip);
static int callback(void* data, int argc, char** argv, char** azColName);
bool existsInDb(std::string ip);
int data_base_init(const std::string& db_name);

#endif