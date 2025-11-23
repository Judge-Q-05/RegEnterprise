#ifndef GATEWAYS_H
#define GATEWAYS_H

#include "DatabaseConnection.h"
#include "DomainEntities.h"
#include <vector>
#include <string>

// ==========================================
// Базовый класс TableGateway
// ==========================================
class TableGateway {
protected:
    DatabaseConnection* db;

public:
    TableGateway(DatabaseConnection* conn) : db(conn) {}
    virtual ~TableGateway() = default;

    // Метод создания таблицы (должен быть реализован в каждом шлюзе)
    virtual void createTableIfNotExists() = 0;

    // Вспомогательные статические методы для экранирования SQL
    // Реализацию можно оставить в TableGateway.cpp или сделать inline здесь, 
    // но согласно вашей структуре они были в хедере.
    static std::string quote(const std::string& str) {
        return "'" + str + "'";
    }

    static std::string escape(const std::string& str) {
        std::string escaped = str;
        size_t pos = 0;
        while ((pos = escaped.find("'", pos)) != std::string::npos) {
            escaped.replace(pos, 1, "''");
            pos += 2;
        }
        return escaped;
    }
};

// ==========================================
// Шлюз: Предприятия (Enterprise)
// ==========================================
class EnterpriseGateway : public TableGateway {
public:
    using TableGateway::TableGateway;

    void createTableIfNotExists() override;

    // Основной CRUD
    std::vector<Enterprise> findAll();
    Enterprise findById(int id);
    
    // Принимает DTO, возвращает ID созданной записи
    int insert(const Enterprise& ent); 
    
    // Принимает DTO, возвращает успех операции
    bool update(const Enterprise& ent);
    
    // Удаление по ID
    bool remove(int id);

    // Специфичные методы поиска
    Enterprise findByInn(const std::string& inn);
};

// ==========================================
// Шлюз: Товары (Product)
// ==========================================
class ProductGateway : public TableGateway {
public:
    using TableGateway::TableGateway;

    void createTableIfNotExists() override;

    std::vector<Product> findAll();
    Product findById(int id);
    
    // Возвращает ID нового товара
    int insert(const Product& prod);
    
    bool update(const Product& prod);
    bool remove(int id);

    // Поиск по названию
    Product findByName(const std::string& name);
};

// ==========================================
// Шлюз: Ассортимент (EnterpriseProduct)
// Таблица связи "Многие-ко-Многим"
// ==========================================
class EnterpriseProductGateway : public TableGateway {
public:
    using TableGateway::TableGateway;

    void createTableIfNotExists() override;

    // Поиск связей
    std::vector<EnterpriseProduct> findByEnterprise(int enterprise_id);
    std::vector<EnterpriseProduct> findByProduct(int product_id);

    // Добавление связи (товар в ассортимент предприятия)
    // Возвращает bool, так как ID составной
    bool insert(const EnterpriseProduct& item);

    // Обновление (например, изменение оптовой цены)
    bool update(const EnterpriseProduct& item);

    // Удаление связи (нужен составной ключ)
    bool remove(int enterprise_id, int product_id);
};

// ==========================================
// Шлюз: Отделы сбыта (SalesDepartment)
// ==========================================
class SalesDepartmentGateway : public TableGateway {
public:
    using TableGateway::TableGateway;

    void createTableIfNotExists() override;

    std::vector<SalesDepartment> findAll();
    SalesDepartment findById(int id);

    int insert(const SalesDepartment& dept);
    bool update(const SalesDepartment& dept);
    bool remove(int id);
};

// ==========================================
// Шлюз: Банковские реквизиты (BankDetails)
// ==========================================
class BankDetailsGateway : public TableGateway {
public:
    using TableGateway::TableGateway;

    void createTableIfNotExists() override;

    std::vector<BankDetails> findAll();
    BankDetails findById(int id);

    int insert(const BankDetails& details);
    bool update(const BankDetails& details);
    bool remove(int id);
};

#endif