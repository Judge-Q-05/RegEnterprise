#ifndef REGISTRY_SERVICE_H
#define REGISTRY_SERVICE_H

#include "DatabaseConnection.h"
#include "Gateways.h"
#include "DomainEntities.h"
#include <vector>
#include <memory>
#include <utility> // для std::pair

class RegistryService {
private:
    DatabaseConnection db;
    
    // Шлюзы (владеем ими приватно, UI о них не знает)
    std::unique_ptr<EnterpriseGateway> enterpriseGateway;
    std::unique_ptr<ProductGateway> productGateway;
    std::unique_ptr<EnterpriseProductGateway> enterpriseProductGateway;
    std::unique_ptr<SalesDepartmentGateway> salesDepartmentGateway;
    std::unique_ptr<BankDetailsGateway> bankDetailsGateway;

public:
    RegistryService();
    ~RegistryService();

    // Инициализация (подключение к БД, создание всех таблиц и справочников)
    bool initialize(); 

    // ==========================================
    // Методы для работы с Предприятиями
    // ==========================================
    std::vector<Enterprise> getAllEnterprises();
    Enterprise getEnterpriseById(int id);
    // Возвращает ID созданного предприятия или -1 при ошибке
    int createEnterprise(const Enterprise& ent);
    bool updateEnterprise(const Enterprise& ent);
    bool deleteEnterprise(int id);

    // ==========================================
    // Методы для работы с Товарами
    // ==========================================
    std::vector<Product> getAllProducts();
    Product getProductById(int id);
    // Возвращает ID созданного товара или -1 при ошибке
    int createProduct(const Product& prod);
    bool updateProduct(const Product& prod);
    bool deleteProduct(int id);

    // ==========================================
    // Методы Ассортимента (Связь M:N)
    // ==========================================
    
    // Получает список товаров конкретного предприятия с их оптовыми ценами.
    // Возвращает пару: {Товар, Оптовая цена}
    std::vector<std::pair<Product, double>> getAssortmentForEnterprise(int enterpriseId);

    // Добавить товар в ассортимент предприятия
    bool addProductToAssortment(int enterpriseId, int productId, double wholesalePrice);

    // Удалить товар из ассортимента предприятия
    bool removeProductFromAssortment(int enterpriseId, int productId);

    // Изменить оптовую цену товара в ассортименте
    bool updateProductPriceInAssortment(int enterpriseId, int productId, double newPrice);

    // ==========================================
    // Методы для работы с Отделами сбыта
    // ==========================================
    std::vector<SalesDepartment> getAllSalesDepartments();
    SalesDepartment getSalesDepartmentById(int id);
    // Возвращает ID созданного отдела или -1 при ошибке
    int createSalesDepartment(const SalesDepartment& dept);
    bool updateSalesDepartment(const SalesDepartment& dept);
    bool deleteSalesDepartment(int id);

    // ==========================================
    // Методы для работы с Банковскими реквизитами
    // ==========================================
    std::vector<BankDetails> getAllBankDetails();
    BankDetails getBankDetailsById(int id);
    // Возвращает ID созданной записи или -1 при ошибке
    int createBankDetails(const BankDetails& details);
    bool updateBankDetails(const BankDetails& details);
    bool deleteBankDetails(int id);
};

#endif