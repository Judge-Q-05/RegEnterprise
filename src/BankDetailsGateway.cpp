#include "Gateways.h"
#include <sstream>

// Реализация методов для работы с таблицей bank_details

void BankDetailsGateway::createTableIfNotExists() {
    db->executeQuery(R"(
        CREATE TABLE IF NOT EXISTS bank_details (
            bank_id SERIAL PRIMARY KEY,
            enterprise_id INTEGER NOT NULL UNIQUE REFERENCES enterprise(enterprise_id) ON DELETE CASCADE,
            bank_name TEXT NOT NULL,
            bank_city TEXT NOT NULL,
            account_number TEXT NOT NULL
        );
    )");
}

std::vector<BankDetails> BankDetailsGateway::findAll() {
    std::vector<BankDetails> list;
    if (!db->isConnected()) return list;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, db->getHandle(), &hStmt);

    std::string sql = R"(
        SELECT 
            bd.bank_id, bd.enterprise_id, e.name as ent_name,
            bd.bank_name, bd.bank_city, bd.account_number
        FROM bank_details bd
        JOIN enterprise e ON bd.enterprise_id = e.enterprise_id
        ORDER BY bd.bank_id
    )";

    SQLExecDirect(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);

    BankDetails bd;
    SQLCHAR ent_name[256], b_name[256], b_city[256], acc_num[256];

    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        // Очистка буферов
        ent_name[0] = b_name[0] = b_city[0] = acc_num[0] = '\0';

        SQLGetData(hStmt, 1, SQL_C_LONG, &bd.id, 0, nullptr);
        SQLGetData(hStmt, 2, SQL_C_LONG, &bd.enterprise_id, 0, nullptr);
        SQLGetData(hStmt, 3, SQL_C_CHAR, ent_name, sizeof(ent_name), nullptr);
        SQLGetData(hStmt, 4, SQL_C_CHAR, b_name, sizeof(b_name), nullptr);
        SQLGetData(hStmt, 5, SQL_C_CHAR, b_city, sizeof(b_city), nullptr);
        SQLGetData(hStmt, 6, SQL_C_CHAR, acc_num, sizeof(acc_num), nullptr);

        bd.enterprise_name = (char*)ent_name;
        bd.bank_name = (char*)b_name;
        bd.bank_city = (char*)b_city;
        bd.account_number = (char*)acc_num;

        list.push_back(bd);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return list;
}

BankDetails BankDetailsGateway::findById(int id) {
    BankDetails bd;
    bd.id = 0;
    if (!db->isConnected()) return bd;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, db->getHandle(), &hStmt);

    std::ostringstream oss;
    oss << R"(
        SELECT 
            bd.bank_id, bd.enterprise_id, e.name as ent_name,
            bd.bank_name, bd.bank_city, bd.account_number
        FROM bank_details bd
        JOIN enterprise e ON bd.enterprise_id = e.enterprise_id
        WHERE bd.bank_id = )" << id;

    SQLExecDirect(hStmt, (SQLCHAR*)oss.str().c_str(), SQL_NTS);

    SQLCHAR ent_name[256], b_name[256], b_city[256], acc_num[256];
    ent_name[0] = b_name[0] = b_city[0] = acc_num[0] = '\0';

    if (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLGetData(hStmt, 1, SQL_C_LONG, &bd.id, 0, nullptr);
        SQLGetData(hStmt, 2, SQL_C_LONG, &bd.enterprise_id, 0, nullptr);
        SQLGetData(hStmt, 3, SQL_C_CHAR, ent_name, sizeof(ent_name), nullptr);
        SQLGetData(hStmt, 4, SQL_C_CHAR, b_name, sizeof(b_name), nullptr);
        SQLGetData(hStmt, 5, SQL_C_CHAR, b_city, sizeof(b_city), nullptr);
        SQLGetData(hStmt, 6, SQL_C_CHAR, acc_num, sizeof(acc_num), nullptr);

        bd.enterprise_name = (char*)ent_name;
        bd.bank_name = (char*)b_name;
        bd.bank_city = (char*)b_city;
        bd.account_number = (char*)acc_num;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return bd;
}

int BankDetailsGateway::insert(const BankDetails& details) {
    std::ostringstream oss;
    oss << "INSERT INTO bank_details (enterprise_id, bank_name, bank_city, account_number) VALUES ("
        << details.enterprise_id << ", "
        << quote(escape(details.bank_name)) << ", "
        << quote(escape(details.bank_city)) << ", "
        << quote(escape(details.account_number))
        << ") RETURNING bank_id";

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, db->getHandle(), &hStmt);
    SQLExecDirect(hStmt, (SQLCHAR*)oss.str().c_str(), SQL_NTS);

    int newId = -1;
    if (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLGetData(hStmt, 1, SQL_C_LONG, &newId, 0, nullptr);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return newId;
}

bool BankDetailsGateway::update(const BankDetails& details) {
    std::ostringstream oss;
    oss << "UPDATE bank_details SET "
        << "enterprise_id=" << details.enterprise_id << ", "
        << "bank_name=" << quote(escape(details.bank_name)) << ", "
        << "bank_city=" << quote(escape(details.bank_city)) << ", "
        << "account_number=" << quote(escape(details.account_number))
        << " WHERE bank_id=" << details.id;

    return db->executeQuery(oss.str());
}

bool BankDetailsGateway::remove(int id) {
    std::ostringstream oss;
    oss << "DELETE FROM bank_details WHERE bank_id=" << id;
    return db->executeQuery(oss.str());
}