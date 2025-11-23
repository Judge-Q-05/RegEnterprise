#ifndef DATABASE_CONNECTION_H
#define DATABASE_CONNECTION_H

#include <sql.h>
#include <sqlext.h>
#include <iostream>
#include <string>
#include <stdexcept>

class DatabaseConnection {
private:
    SQLHENV hEnv;
    SQLHDBC hDbc;
    bool connected;

public:
    DatabaseConnection();
    ~DatabaseConnection();

    bool connect(const std::string& dsn = "rab_dsn", 
                 const std::string& user = "rab", 
                 const std::string& password = "1111");

    bool isConnected() const { return connected; }
    SQLHDBC getHandle() const { return hDbc; }

    void disconnect();
    bool executeQuery(const std::string& sql);
};

#endif