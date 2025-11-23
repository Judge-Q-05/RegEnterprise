#include "RegistryService.h"
#include <iostream>

// ==========================================
// Конструктор и Деструктор
// ==========================================

RegistryService::RegistryService() {
    // Инициализируем шлюзы, передавая им указатель на (пока еще закрытое) соединение.
    // std::make_unique создает экземпляры классов и управляет памятью.
    enterpriseGateway = std::make_unique<EnterpriseGateway>(&db);
    productGateway = std::make_unique<ProductGateway>(&db);
    enterpriseProductGateway = std::make_unique<EnterpriseProductGateway>(&db);
    salesDepartmentGateway = std::make_unique<SalesDepartmentGateway>(&db);
    bankDetailsGateway = std::make_unique<BankDetailsGateway>(&db);
}

RegistryService::~RegistryService() {}

// ==========================================
// Инициализация
// ==========================================

bool RegistryService::initialize() {
    // 1. Подключение к БД
    // Используем параметры по умолчанию из вашего старого кода
    if (!db.connect("rab_dsn", "rab", "1111")) {
        std::cerr << "Критическая ошибка: Не удалось подключиться к БД." << std::endl;
        return false;
    }

    // 2. Создание справочников (Словари)
    // Эти таблицы не имеют собственных шлюзов, так как они статичны, 
    // но они нужны для Foreign Keys основных таблиц.
    
    // Справочник организационно-правовых форм
    db.executeQuery(R"(
        CREATE TABLE IF NOT EXISTS legal_form (
            legal_form_id SERIAL PRIMARY KEY,
            name TEXT NOT NULL UNIQUE
        );
    )");

    // Справочник форм собственности
    db.executeQuery(R"(
        CREATE TABLE IF NOT EXISTS ownership_form (
            ownership_form_id SERIAL PRIMARY KEY,
            name TEXT NOT NULL UNIQUE
        );
    )");

    // Справочник категорий товаров
    db.executeQuery(R"(
        CREATE TABLE IF NOT EXISTS product_category (
            category_id SERIAL PRIMARY KEY,
            name TEXT NOT NULL UNIQUE
        );
    )");

    // Справочник условий поставки
    db.executeQuery(R"(
        CREATE TABLE IF NOT EXISTS delivery_terms (
            delivery_terms_id SERIAL PRIMARY KEY,
            description TEXT NOT NULL UNIQUE
        );
    )");

    // 3. Создание основных таблиц через шлюзы
    // Порядок важен из-за внешних ключей (Foreign Keys)
    
    enterpriseGateway->createTableIfNotExists();         // Зависит от legal_form, ownership_form
    productGateway->createTableIfNotExists();            // Зависит от product_category, delivery_terms
    enterpriseProductGateway->createTableIfNotExists();  // Зависит от enterprise, product
    salesDepartmentGateway->createTableIfNotExists();    // Зависит от enterprise
    bankDetailsGateway->createTableIfNotExists();        // Зависит от enterprise

    std::cout << "Сервис данных инициализирован успешно." << std::endl;
    return true;
}

// ==========================================
// Предприятия (Enterprise)
// ==========================================

std::vector<Enterprise> RegistryService::getAllEnterprises() {
    return enterpriseGateway->findAll();
}

Enterprise RegistryService::getEnterpriseById(int id) {
    return enterpriseGateway->findById(id);
}

int RegistryService::createEnterprise(const Enterprise& ent) {
    // Бизнес-валидация
    if (ent.name.empty() || ent.inn.empty()) {
        std::cerr << "Ошибка: Название предприятия и ИНН обязательны." << std::endl;
        return -1;
    }
    // Можно добавить проверку на уникальность ИНН
    Enterprise existing = enterpriseGateway->findByInn(ent.inn);
    if (existing.id != 0) {
        std::cerr << "Ошибка: Предприятие с таким ИНН уже существует." << std::endl;
        return -1;
    }

    return enterpriseGateway->insert(ent);
}

bool RegistryService::updateEnterprise(const Enterprise& ent) {
    if (ent.id <= 0) return false;
    if (ent.name.empty() || ent.inn.empty()) return false;
    return enterpriseGateway->update(ent);
}

bool RegistryService::deleteEnterprise(int id) {
    // В базе настроен ON DELETE CASCADE, поэтому удаление предприятия
    // автоматически удалит отделы сбыта, банковские реквизиты и связи ассортимента.
    return enterpriseGateway->remove(id);
}

// ==========================================
// Товары (Product)
// ==========================================

std::vector<Product> RegistryService::getAllProducts() {
    return productGateway->findAll();
}

Product RegistryService::getProductById(int id) {
    return productGateway->findById(id);
}

int RegistryService::createProduct(const Product& prod) {
    if (prod.name.empty()) {
        std::cerr << "Ошибка: У товара должно быть название." << std::endl;
        return -1;
    }
    if (prod.retail_price < 0 || prod.purchase_price < 0) {
        std::cerr << "Ошибка: Цена не может быть отрицательной." << std::endl;
        return -1;
    }
    
    // Проверка на дубликат имени (бизнес-логика)
    Product existing = productGateway->findByName(prod.name);
    if (existing.id != 0) {
        std::cerr << "Ошибка: Товар с таким названием уже существует." << std::endl;
        return -1;
    }

    return productGateway->insert(prod);
}

bool RegistryService::updateProduct(const Product& prod) {
    if (prod.id <= 0) return false;
    return productGateway->update(prod);
}

bool RegistryService::deleteProduct(int id) {
    return productGateway->remove(id);
}

// ==========================================
// Ассортимент (Assortment)
// ==========================================

std::vector<std::pair<Product, double>> RegistryService::getAssortmentForEnterprise(int enterpriseId) {
    std::vector<std::pair<Product, double>> result;
    
    // 1. Получаем связи из таблицы связей
    auto links = enterpriseProductGateway->findByEnterprise(enterpriseId);

    // 2. Для каждой связи подтягиваем полную информацию о товаре
    for (const auto& link : links) {
        Product p = productGateway->findById(link.product_id);
        if (p.id != 0) {
            result.push_back({p, link.wholesale_price});
        }
    }
    return result;
}

bool RegistryService::addProductToAssortment(int enterpriseId, int productId, double wholesalePrice) {
    if (wholesalePrice < 0) {
        std::cerr << "Ошибка: Оптовая цена не может быть отрицательной." << std::endl;
        return false;
    }

    // Формируем объект связи DTO
    EnterpriseProduct link;
    link.enterprise_id = enterpriseId;
    link.product_id = productId;
    link.wholesale_price = wholesalePrice;

    // Пытаемся вставить. Если связь уже есть — БД может вернуть ошибку (PK constraint),
    // либо можно предварительно проверить наличие.
    // В данном примере полагаемся на то, что insert вернет false при дубликате.
    if (!enterpriseProductGateway->insert(link)) {
        std::cerr << "Ошибка: Не удалось добавить товар (возможно, он уже в ассортименте)." << std::endl;
        return false;
    }
    return true;
}

bool RegistryService::removeProductFromAssortment(int enterpriseId, int productId) {
    return enterpriseProductGateway->remove(enterpriseId, productId);
}

bool RegistryService::updateProductPriceInAssortment(int enterpriseId, int productId, double newPrice) {
    if (newPrice < 0) return false;

    EnterpriseProduct link;
    link.enterprise_id = enterpriseId;
    link.product_id = productId;
    link.wholesale_price = newPrice;

    return enterpriseProductGateway->update(link);
}

// ==========================================
// Отделы сбыта (Sales Department)
// ==========================================

std::vector<SalesDepartment> RegistryService::getAllSalesDepartments() {
    return salesDepartmentGateway->findAll();
}

SalesDepartment RegistryService::getSalesDepartmentById(int id) {
    return salesDepartmentGateway->findById(id);
}

int RegistryService::createSalesDepartment(const SalesDepartment& dept) {
    if (dept.contact_last_name.empty() || dept.contact_first_name.empty()) {
        std::cerr << "Ошибка: Фамилия и Имя контакта обязательны." << std::endl;
        return -1;
    }
    return salesDepartmentGateway->insert(dept);
}

bool RegistryService::updateSalesDepartment(const SalesDepartment& dept) {
    if (dept.id <= 0) return false;
    return salesDepartmentGateway->update(dept);
}

bool RegistryService::deleteSalesDepartment(int id) {
    return salesDepartmentGateway->remove(id);
}

// ==========================================
// Банковские реквизиты (Bank Details)
// ==========================================

std::vector<BankDetails> RegistryService::getAllBankDetails() {
    return bankDetailsGateway->findAll();
}

BankDetails RegistryService::getBankDetailsById(int id) {
    return bankDetailsGateway->findById(id);
}

int RegistryService::createBankDetails(const BankDetails& details) {
    if (details.bank_name.empty() || details.account_number.empty()) {
        std::cerr << "Ошибка: Название банка и номер счета обязательны." << std::endl;
        return -1;
    }
    return bankDetailsGateway->insert(details);
}

bool RegistryService::updateBankDetails(const BankDetails& details) {
    if (details.id <= 0) return false;
    return bankDetailsGateway->update(details);
}

bool RegistryService::deleteBankDetails(int id) {
    return bankDetailsGateway->remove(id);
}