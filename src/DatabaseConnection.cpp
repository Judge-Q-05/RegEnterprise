#include "DatabaseConnection.h"

DatabaseConnection::DatabaseConnection() : hEnv(SQL_NULL_HANDLE), hDbc(SQL_NULL_HANDLE), connected(false) {}

DatabaseConnection::~DatabaseConnection() {
    disconnect();
}

bool DatabaseConnection::connect(const std::string& dsn, const std::string& user, const std::string& password) {
    SQLRETURN ret;

    // Инициализация среды
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Ошибка при выделении среды ODBC" << std::endl;
        return false;
    }

    ret = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Ошибка при установке версии ODBC" << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }

    // Выделение дескриптора соединения
    ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Ошибка при выделении дескриптора соединения" << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }

    // Подключение к базе данных
    SQLCHAR outConnStr[1024];
    SQLSMALLINT outConnStrLen;

    ret = SQLDriverConnect(hDbc, nullptr,
                           (SQLCHAR*)("DSN=" + dsn + ";UID=" + user + ";PWD=" + password).c_str(),
                           SQL_NTS, outConnStr, sizeof(outConnStr), &outConnStrLen,
                           SQL_DRIVER_NOPROMPT);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLCHAR sqlState[6], message[512];
        SQLINTEGER nativeError;
        SQLGetDiagRec(SQL_HANDLE_DBC, hDbc, 1, sqlState, &nativeError, message, sizeof(message), nullptr);
        std::cerr << "Ошибка подключения: " << message << " (SQLSTATE: " << sqlState << ")" << std::endl;
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }

    connected = true;
    std::cout << "Подключено к БД через ODBC." << std::endl;
    return true;
}

void DatabaseConnection::disconnect() {
    if (connected) {
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        connected = false;
        std::cout << "Отключено от БД." << std::endl;
    }
}

bool DatabaseConnection::executeQuery(const std::string& sql) {
    if (!connected) {
        std::cerr << "Не подключено к БД!" << std::endl;
        return false;
    }

    SQLHSTMT hStmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Ошибка при выделении оператора SQL" << std::endl;
        return false;
    }

    ret = SQLExecDirect(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLCHAR sqlState[6], message[512];
        SQLINTEGER nativeError;
        SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, 1, sqlState, &nativeError, message, sizeof(message), nullptr);
        std::cerr << "Ошибка выполнения запроса: " << message << " (SQLSTATE: " << sqlState << ")" << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return true;
}