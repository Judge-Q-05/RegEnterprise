#include "Gateways.h"
#include <sstream>

void ProductGateway::createTableIfNotExists() {
    db->executeQuery(R"(
        CREATE TABLE IF NOT EXISTS product (
            product_id SERIAL PRIMARY KEY,
            category_id INTEGER NOT NULL REFERENCES product_category(category_id),
            name TEXT NOT NULL,
            shelf_life_days INTEGER,
            delivery_terms_id INTEGER NOT NULL REFERENCES delivery_terms(delivery_terms_id),
            retail_price NUMERIC(10,2),
            purchase_price NUMERIC(10,2)
        );
    )");
}

std::vector<Product> ProductGateway::findAll() {
    std::vector<Product> list;
    if (!db->isConnected()) return list;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, db->getHandle(), &hStmt);

    std::string sql = R"(
        SELECT p.product_id, p.category_id, p.name, p.shelf_life_days, 
               p.delivery_terms_id, p.retail_price, p.purchase_price,
               pc.name as cat_name, dt.description as dt_desc
        FROM product p
        LEFT JOIN product_category pc ON p.category_id = pc.category_id
        LEFT JOIN delivery_terms dt ON p.delivery_terms_id = dt.delivery_terms_id
        ORDER BY p.product_id
    )";
    SQLExecDirect(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);

    Product p;
    SQLCHAR name[256], cat_name[128], dt_desc[256];
    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLGetData(hStmt, 1, SQL_C_LONG, &p.id, 0, nullptr);
        SQLGetData(hStmt, 2, SQL_C_LONG, &p.category_id, 0, nullptr);
        SQLGetData(hStmt, 3, SQL_C_CHAR, name, sizeof(name), nullptr);
        SQLGetData(hStmt, 4, SQL_C_LONG, &p.shelf_life_days, 0, nullptr);
        SQLGetData(hStmt, 5, SQL_C_LONG, &p.delivery_terms_id, 0, nullptr);
        SQLGetData(hStmt, 6, SQL_C_DOUBLE, &p.retail_price, 0, nullptr);
        SQLGetData(hStmt, 7, SQL_C_DOUBLE, &p.purchase_price, 0, nullptr);
        SQLGetData(hStmt, 8, SQL_C_CHAR, cat_name, sizeof(cat_name), nullptr);
        SQLGetData(hStmt, 9, SQL_C_CHAR, dt_desc, sizeof(dt_desc), nullptr);
        
        p.name = (char*)name;
        p.category_name = (char*)cat_name;
        p.delivery_terms_description = (char*)dt_desc;
        list.push_back(p);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return list;
}

Product ProductGateway::findById(int id) {
    Product p; p.id = 0;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, db->getHandle(), &hStmt);
    std::ostringstream oss;
    oss << "SELECT product_id, name, retail_price FROM product WHERE product_id=" << id;
    SQLExecDirect(hStmt, (SQLCHAR*)oss.str().c_str(), SQL_NTS);
    SQLCHAR name[256];
    if (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLGetData(hStmt, 1, SQL_C_LONG, &p.id, 0, nullptr);
        SQLGetData(hStmt, 2, SQL_C_CHAR, name, sizeof(name), nullptr);
        SQLGetData(hStmt, 3, SQL_C_DOUBLE, &p.retail_price, 0, nullptr);
        p.name = (char*)name;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return p;
}

int ProductGateway::insert(const Product& prod) {
    std::ostringstream oss;
    oss << "INSERT INTO product (category_id, name, shelf_life_days, delivery_terms_id, retail_price, purchase_price) VALUES ("
        << prod.category_id << ", "
        << quote(escape(prod.name)) << ", "
        << prod.shelf_life_days << ", "
        << prod.delivery_terms_id << ", "
        << prod.retail_price << ", "
        << prod.purchase_price << ") RETURNING product_id";
    
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, db->getHandle(), &hStmt);
    SQLExecDirect(hStmt, (SQLCHAR*)oss.str().c_str(), SQL_NTS);
    int newId = -1;
    if (SQLFetch(hStmt) == SQL_SUCCESS) SQLGetData(hStmt, 1, SQL_C_LONG, &newId, 0, nullptr);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return newId;
}

bool ProductGateway::update(const Product& prod) {
    std::ostringstream oss;
    oss << "UPDATE product SET "
        << "category_id=" << prod.category_id << ", "
        << "name=" << quote(escape(prod.name)) << ", "
        << "shelf_life_days=" << prod.shelf_life_days << ", "
        << "delivery_terms_id=" << prod.delivery_terms_id << ", "
        << "retail_price=" << prod.retail_price << ", "
        << "purchase_price=" << prod.purchase_price
        << " WHERE product_id=" << prod.id;

    return db->executeQuery(oss.str());
}
bool ProductGateway::remove(int id) {
    std::ostringstream oss;
    oss << "DELETE FROM product WHERE product_id=" << id;
    return db->executeQuery(oss.str());
}

Product ProductGateway::findByName(const std::string& name) {
    Product p; p.id = 0;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, db->getHandle(), &hStmt);
    std::string q = "SELECT product_id, name FROM product WHERE name=" + quote(escape(name));
    SQLExecDirect(hStmt, (SQLCHAR*)q.c_str(), SQL_NTS);
    SQLCHAR n[256];
    if (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLGetData(hStmt, 1, SQL_C_LONG, &p.id, 0, nullptr);
        SQLGetData(hStmt, 2, SQL_C_CHAR, n, sizeof(n), nullptr);
        p.name = (char*)n;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return p;
}