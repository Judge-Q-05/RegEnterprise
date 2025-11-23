#include "Gateways.h"
#include <sstream>

// Реализация методов для работы с таблицей sales_department

void SalesDepartmentGateway::createTableIfNotExists() {
    // Создаем таблицу отделов сбыта.
    // ON DELETE CASCADE означает, что если удалить предприятие, отдел удалится сам.
    db->executeQuery(R"(
        CREATE TABLE IF NOT EXISTS sales_department (
            depart_id SERIAL PRIMARY KEY,
            enterprise_id INTEGER NOT NULL UNIQUE REFERENCES enterprise(enterprise_id) ON DELETE CASCADE,
            phone TEXT,
            fax TEXT,
            email TEXT,
            contact_last_name TEXT NOT NULL,
            contact_first_name TEXT NOT NULL,
            contact_patronymic TEXT
        );
    )");
}

std::vector<SalesDepartment> SalesDepartmentGateway::findAll() {
    std::vector<SalesDepartment> list;
    if (!db->isConnected()) return list;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, db->getHandle(), &hStmt);

    // JOIN с таблицей enterprise, чтобы получить название предприятия для отображения
    std::string sql = R"(
        SELECT 
            sd.depart_id, sd.enterprise_id, e.name as ent_name,
            sd.phone, sd.fax, sd.email,
            sd.contact_last_name, sd.contact_first_name, sd.contact_patronymic
        FROM sales_department sd
        JOIN enterprise e ON sd.enterprise_id = e.enterprise_id
        ORDER BY sd.depart_id
    )";

    SQLExecDirect(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);

    SalesDepartment sd;
    // Буферы для строк (размер с запасом)
    SQLCHAR ent_name[256], phone[128], fax[128], email[256];
    SQLCHAR last[256], first[256], patr[256];
    
    // Инициализация буферов нулями (на случай NULL из БД)
    // Примечание: драйвер ODBC может вернуть NULL, для надежности лучше проверять индикатор длины,
    // но для учебного примера допустимо полагаться на инициализацию.
    
    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        // Сброс буферов перед чтением новой строки
        ent_name[0] = phone[0] = fax[0] = email[0] = last[0] = first[0] = patr[0] = '\0';

        SQLGetData(hStmt, 1, SQL_C_LONG, &sd.id, 0, nullptr);
        SQLGetData(hStmt, 2, SQL_C_LONG, &sd.enterprise_id, 0, nullptr);
        SQLGetData(hStmt, 3, SQL_C_CHAR, ent_name, sizeof(ent_name), nullptr);
        SQLGetData(hStmt, 4, SQL_C_CHAR, phone, sizeof(phone), nullptr);
        SQLGetData(hStmt, 5, SQL_C_CHAR, fax, sizeof(fax), nullptr);
        SQLGetData(hStmt, 6, SQL_C_CHAR, email, sizeof(email), nullptr);
        SQLGetData(hStmt, 7, SQL_C_CHAR, last, sizeof(last), nullptr);
        SQLGetData(hStmt, 8, SQL_C_CHAR, first, sizeof(first), nullptr);
        SQLGetData(hStmt, 9, SQL_C_CHAR, patr, sizeof(patr), nullptr);

        sd.enterprise_name = (char*)ent_name;
        sd.phone = (char*)phone;
        sd.fax = (char*)fax;
        sd.email = (char*)email;
        sd.contact_last_name = (char*)last;
        sd.contact_first_name = (char*)first;
        sd.contact_patronymic = (char*)patr;

        list.push_back(sd);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return list;
}

SalesDepartment SalesDepartmentGateway::findById(int id) {
    SalesDepartment sd; 
    sd.id = 0; // Маркер "не найдено"
    
    if (!db->isConnected()) return sd;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, db->getHandle(), &hStmt);

    std::ostringstream oss;
    oss << R"(
        SELECT 
            sd.depart_id, sd.enterprise_id, e.name as ent_name,
            sd.phone, sd.fax, sd.email,
            sd.contact_last_name, sd.contact_first_name, sd.contact_patronymic
        FROM sales_department sd
        JOIN enterprise e ON sd.enterprise_id = e.enterprise_id
        WHERE sd.depart_id = )" << id;

    SQLExecDirect(hStmt, (SQLCHAR*)oss.str().c_str(), SQL_NTS);

    SQLCHAR ent_name[256], phone[128], fax[128], email[256];
    SQLCHAR last[256], first[256], patr[256];
    ent_name[0] = phone[0] = fax[0] = email[0] = last[0] = first[0] = patr[0] = '\0';

    if (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLGetData(hStmt, 1, SQL_C_LONG, &sd.id, 0, nullptr);
        SQLGetData(hStmt, 2, SQL_C_LONG, &sd.enterprise_id, 0, nullptr);
        SQLGetData(hStmt, 3, SQL_C_CHAR, ent_name, sizeof(ent_name), nullptr);
        SQLGetData(hStmt, 4, SQL_C_CHAR, phone, sizeof(phone), nullptr);
        SQLGetData(hStmt, 5, SQL_C_CHAR, fax, sizeof(fax), nullptr);
        SQLGetData(hStmt, 6, SQL_C_CHAR, email, sizeof(email), nullptr);
        SQLGetData(hStmt, 7, SQL_C_CHAR, last, sizeof(last), nullptr);
        SQLGetData(hStmt, 8, SQL_C_CHAR, first, sizeof(first), nullptr);
        SQLGetData(hStmt, 9, SQL_C_CHAR, patr, sizeof(patr), nullptr);

        sd.enterprise_name = (char*)ent_name;
        sd.phone = (char*)phone;
        sd.fax = (char*)fax;
        sd.email = (char*)email;
        sd.contact_last_name = (char*)last;
        sd.contact_first_name = (char*)first;
        sd.contact_patronymic = (char*)patr;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return sd;
}

int SalesDepartmentGateway::insert(const SalesDepartment& dept) {
    std::ostringstream oss;
    // Формируем SQL INSERT. Для пустых строк используем NULL или пустую строку.
    // Здесь используем экранирование для защиты от SQL-инъекций.
    oss << "INSERT INTO sales_department ("
        << "enterprise_id, phone, fax, email, "
        << "contact_last_name, contact_first_name, contact_patronymic) VALUES ("
        << dept.enterprise_id << ", "
        << quote(escape(dept.phone)) << ", "
        << quote(escape(dept.fax)) << ", "
        << quote(escape(dept.email)) << ", "
        << quote(escape(dept.contact_last_name)) << ", "
        << quote(escape(dept.contact_first_name)) << ", "
        << quote(escape(dept.contact_patronymic))
        << ") RETURNING depart_id";

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

bool SalesDepartmentGateway::update(const SalesDepartment& dept) {
    std::ostringstream oss;
    oss << "UPDATE sales_department SET "
        << "enterprise_id=" << dept.enterprise_id << ", "
        << "phone=" << quote(escape(dept.phone)) << ", "
        << "fax=" << quote(escape(dept.fax)) << ", "
        << "email=" << quote(escape(dept.email)) << ", "
        << "contact_last_name=" << quote(escape(dept.contact_last_name)) << ", "
        << "contact_first_name=" << quote(escape(dept.contact_first_name)) << ", "
        << "contact_patronymic=" << quote(escape(dept.contact_patronymic))
        << " WHERE depart_id=" << dept.id;

    return db->executeQuery(oss.str());
}

bool SalesDepartmentGateway::remove(int id) {
    std::ostringstream oss;
    oss << "DELETE FROM sales_department WHERE depart_id=" << id;
    return db->executeQuery(oss.str());
}