#include "Gateways.h"
#include <sstream>

void EnterpriseGateway::createTableIfNotExists() {
    // В базовом классе нет ensureTableExists, реализуем проверку здесь или просто пытаемся создать IF NOT EXISTS
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS enterprise (
            enterprise_id SERIAL PRIMARY KEY,
            name TEXT NOT NULL,
            legal_form_id INTEGER NOT NULL REFERENCES legal_form(legal_form_id),
            ownership_form_id INTEGER NOT NULL REFERENCES ownership_form(ownership_form_id),
            postal_address TEXT NOT NULL,
            inn TEXT NOT NULL UNIQUE
        );
    )";
    db->executeQuery(sql);
}

std::vector<Enterprise> EnterpriseGateway::findAll() {
    std::vector<Enterprise> list;
    if (!db->isConnected()) return list;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, db->getHandle(), &hStmt);

    std::string sql = R"(
        SELECT e.enterprise_id, e.name, e.legal_form_id, e.ownership_form_id, 
               e.postal_address, e.inn, lf.name, of.name
        FROM enterprise e
        LEFT JOIN legal_form lf ON e.legal_form_id = lf.legal_form_id
        LEFT JOIN ownership_form of ON e.ownership_form_id = of.ownership_form_id
        ORDER BY e.enterprise_id
    )";

    SQLExecDirect(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);

    Enterprise e;
    SQLCHAR name[256], addr[256], inn[64], lf_name[128], of_name[128];
    
    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLGetData(hStmt, 1, SQL_C_LONG, &e.id, 0, nullptr);
        SQLGetData(hStmt, 2, SQL_C_CHAR, name, sizeof(name), nullptr);
        SQLGetData(hStmt, 3, SQL_C_LONG, &e.legal_form_id, 0, nullptr);
        SQLGetData(hStmt, 4, SQL_C_LONG, &e.ownership_form_id, 0, nullptr);
        SQLGetData(hStmt, 5, SQL_C_CHAR, addr, sizeof(addr), nullptr);
        SQLGetData(hStmt, 6, SQL_C_CHAR, inn, sizeof(inn), nullptr);
        SQLGetData(hStmt, 7, SQL_C_CHAR, lf_name, sizeof(lf_name), nullptr);
        SQLGetData(hStmt, 8, SQL_C_CHAR, of_name, sizeof(of_name), nullptr);

        e.name = (char*)name;
        e.postal_address = (char*)addr;
        e.inn = (char*)inn;
        e.legal_form_name = (char*)lf_name;
        e.ownership_form_name = (char*)of_name;
        list.push_back(e);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return list;
}

Enterprise EnterpriseGateway::findById(int id) {
    Enterprise e;
    e.id = 0; // Flag as not found
    if (!db->isConnected()) return e;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, db->getHandle(), &hStmt);

    std::ostringstream oss;
    oss << "SELECT enterprise_id, name, legal_form_id, ownership_form_id, postal_address, inn FROM enterprise WHERE enterprise_id=" << id;
    SQLExecDirect(hStmt, (SQLCHAR*)oss.str().c_str(), SQL_NTS);

    SQLCHAR name[256], addr[256], inn[64];
    if (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLGetData(hStmt, 1, SQL_C_LONG, &e.id, 0, nullptr);
        SQLGetData(hStmt, 2, SQL_C_CHAR, name, sizeof(name), nullptr);
        SQLGetData(hStmt, 3, SQL_C_LONG, &e.legal_form_id, 0, nullptr);
        SQLGetData(hStmt, 4, SQL_C_LONG, &e.ownership_form_id, 0, nullptr);
        SQLGetData(hStmt, 5, SQL_C_CHAR, addr, sizeof(addr), nullptr);
        SQLGetData(hStmt, 6, SQL_C_CHAR, inn, sizeof(inn), nullptr);
        e.name = (char*)name;
        e.postal_address = (char*)addr;
        e.inn = (char*)inn;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return e;
}

int EnterpriseGateway::insert(const Enterprise& ent) {
    std::ostringstream oss;
    oss << "INSERT INTO enterprise (name, legal_form_id, ownership_form_id, postal_address, inn) VALUES ("
        << quote(escape(ent.name)) << ", " 
        << ent.legal_form_id << ", " 
        << ent.ownership_form_id << ", "
        << quote(escape(ent.postal_address)) << ", "
        << quote(escape(ent.inn)) << ")";
    
    // Для получения ID нужен RETURNING или отдельный запрос, упростим для ODBC (драйверы по разному поддерживают RETURNING)
    // Попробуем через RETURNING enterprise_id, если PostgreSQL
    oss << " RETURNING enterprise_id";

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

bool EnterpriseGateway::update(const Enterprise& ent) {
    std::ostringstream oss;
    oss << "UPDATE enterprise SET "
        << "name=" << quote(escape(ent.name)) << ", "
        << "legal_form_id=" << ent.legal_form_id << ", "
        << "ownership_form_id=" << ent.ownership_form_id << ", "
        << "postal_address=" << quote(escape(ent.postal_address)) << ", "
        << "inn=" << quote(escape(ent.inn))
        << " WHERE enterprise_id=" << ent.id;
    return db->executeQuery(oss.str());
}

bool EnterpriseGateway::remove(int id) {
    std::ostringstream oss;
    oss << "DELETE FROM enterprise WHERE enterprise_id=" << id;
    return db->executeQuery(oss.str());
}

Enterprise EnterpriseGateway::findByInn(const std::string& inn) {
    Enterprise e;
    e.id = 0;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, db->getHandle(), &hStmt);
    
    std::string q = "SELECT enterprise_id, name, inn FROM enterprise WHERE inn=" + quote(escape(inn));
    SQLExecDirect(hStmt, (SQLCHAR*)q.c_str(), SQL_NTS);
    
    SQLCHAR name[256], inn_buf[64];
    if (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLGetData(hStmt, 1, SQL_C_LONG, &e.id, 0, nullptr);
        SQLGetData(hStmt, 2, SQL_C_CHAR, name, sizeof(name), nullptr);
        SQLGetData(hStmt, 3, SQL_C_CHAR, inn_buf, sizeof(inn_buf), nullptr);
        e.name = (char*)name;
        e.inn = (char*)inn_buf;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return e;
}