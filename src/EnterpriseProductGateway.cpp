#include "Gateways.h"
#include <sstream>

void EnterpriseProductGateway::createTableIfNotExists() {
    db->executeQuery(R"(
        CREATE TABLE IF NOT EXISTS enterprise_product (
            enterprise_id INTEGER NOT NULL REFERENCES enterprise(enterprise_id) ON DELETE CASCADE,
            product_id INTEGER NOT NULL REFERENCES product(product_id) ON DELETE CASCADE,
            wholesale_price NUMERIC(10,2),
            PRIMARY KEY (enterprise_id, product_id)
        );
    )");
}

std::vector<EnterpriseProduct> EnterpriseProductGateway::findByEnterprise(int enterprise_id) {
    std::vector<EnterpriseProduct> list;
    if (!db->isConnected()) return list;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, db->getHandle(), &hStmt);
    std::ostringstream oss;
    oss << "SELECT enterprise_id, product_id, wholesale_price FROM enterprise_product WHERE enterprise_id=" << enterprise_id;
    SQLExecDirect(hStmt, (SQLCHAR*)oss.str().c_str(), SQL_NTS);

    EnterpriseProduct ep;
    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLGetData(hStmt, 1, SQL_C_LONG, &ep.enterprise_id, 0, nullptr);
        SQLGetData(hStmt, 2, SQL_C_LONG, &ep.product_id, 0, nullptr);
        SQLGetData(hStmt, 3, SQL_C_DOUBLE, &ep.wholesale_price, 0, nullptr);
        list.push_back(ep);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return list;
}

std::vector<EnterpriseProduct> EnterpriseProductGateway::findByProduct(int product_id) {
    std::vector<EnterpriseProduct> list;
    
    // Проверка соединения
    if (!db->isConnected()) return list;

    SQLHSTMT hStmt;
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, db->getHandle(), &hStmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) return list;

    // Формируем запрос: выбираем все предприятия, у которых есть конкретный товар
    std::ostringstream oss;
    oss << "SELECT enterprise_id, product_id, wholesale_price "
        << "FROM enterprise_product WHERE product_id=" << product_id;

    SQLExecDirect(hStmt, (SQLCHAR*)oss.str().c_str(), SQL_NTS);

    EnterpriseProduct ep;
    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLGetData(hStmt, 1, SQL_C_LONG, &ep.enterprise_id, 0, nullptr);
        SQLGetData(hStmt, 2, SQL_C_LONG, &ep.product_id, 0, nullptr);
        SQLGetData(hStmt, 3, SQL_C_DOUBLE, &ep.wholesale_price, 0, nullptr);
        
        list.push_back(ep);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return list;
}

bool EnterpriseProductGateway::insert(const EnterpriseProduct& item) {
    std::ostringstream oss;
    oss << "INSERT INTO enterprise_product (enterprise_id, product_id, wholesale_price) VALUES ("
        << item.enterprise_id << ", " << item.product_id << ", " << item.wholesale_price << ")";
    return db->executeQuery(oss.str());
}

bool EnterpriseProductGateway::update(const EnterpriseProduct& item) {
    std::ostringstream oss;
    oss << "UPDATE enterprise_product SET wholesale_price=" << item.wholesale_price
        << " WHERE enterprise_id=" << item.enterprise_id << " AND product_id=" << item.product_id;
    return db->executeQuery(oss.str());
}

bool EnterpriseProductGateway::remove(int enterprise_id, int product_id) {
    std::ostringstream oss;
    oss << "DELETE FROM enterprise_product WHERE enterprise_id=" << enterprise_id << " AND product_id=" << product_id;
    return db->executeQuery(oss.str());
}