#include "CLIInterface.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <sstream>

// ==========================================
// Вспомогательные функции для UTF-8 (оставлены как были)
// ==========================================

size_t utf8_char_count(const std::string& str) {
    size_t count = 0;
    for (size_t i = 0; i < str.size(); ++i) {
        unsigned char c = str[i];
        if (c < 0x80) count++;
        else if (c >= 0xC0) count++;
    }
    return count;
}

std::string truncate_to_visual_width(const std::string& s, size_t maxChars) {
    if (s.empty()) return std::string(maxChars, ' ');

    std::string result;
    size_t charCount = 0;
    for (size_t i = 0; i < s.size() && charCount < maxChars; ) {
        unsigned char c = s[i];
        if (c < 0x80) { // ASCII
            result += c;
            ++charCount;
            ++i;
        } else if (c < 0xC0) { // Неверный UTF-8
            ++i;
        } else if (c < 0xE0) { // 2-байтный
            if (i + 1 >= s.size()) break;
            result += c; result += s[i + 1];
            ++charCount; i += 2;
        } else if (c < 0xF0) { // 3-байтный
            if (i + 2 >= s.size()) break;
            result += c; result += s[i + 1]; result += s[i + 2];
            ++charCount; i += 3;
        } else { // 4-байтный
            ++charCount; i += 4;
            if (i > s.size()) i = s.size();
        }
    }

    while (charCount < maxChars) {
        result += ' ';
        ++charCount;
    }

    if (charCount == maxChars && utf8_char_count(s) > maxChars && maxChars >= 3) {
        size_t pos = result.size();
        int removed = 0;
        while (pos > 0 && removed < 3) {
            unsigned char c = result[pos - 1];
            if (c < 0x80 || c >= 0xC0) removed++;
            pos--;
        }
        result.resize(pos);
        result += "...";
        while (utf8_char_count(result) < maxChars) result += ' ';
    }
    return result;
}

// ==========================================
// Реализация CLIInterface
// ==========================================

CLIInterface::CLIInterface() {
    // Конструктор пуст, сервис инициализируется автоматически
}

void CLIInterface::run() {
    // Вся логика подключения и создания таблиц теперь внутри сервиса
    if (!service.initialize()) {
        std::cerr << "Критическая ошибка: Сервис данных недоступен." << std::endl;
        return;
    }

    while (true) {
        showMainMenu();
        int choice = getIntegerInput("Выберите действие: ");
        switch (choice) {
            case 1: manageEnterprises(); break;
            case 2: manageProducts(); break;
            case 3: manageAssortment(); break;
            case 4: manageSalesDepartments(); break;
            case 5: manageBankDetails(); break;
            case 0: std::cout << "Выход из программы." << std::endl; return;
            default: std::cout << "Неверный выбор. Попробуйте снова." << std::endl;
        }
    }
}

void CLIInterface::showMainMenu() {
    std::cout << "\n=== Реестр предприятий ===\n";
    std::cout << "1. Управление предприятиями\n";
    std::cout << "2. Управление товарами\n";
    std::cout << "3. Управление ассортиментом предприятий\n";
    std::cout << "4. Управление отделами сбыта\n";
    std::cout << "5. Управление банковскими реквизитами\n";
    std::cout << "0. Выход\n";
}

void CLIInterface::printTable(
    const std::string& title,
    int currentPage,
    int totalPages,
    const std::vector<std::string>& headers,
    const std::vector<std::vector<std::string>>& rows,
    int /*pageSize*/
) {
    if (headers.empty()) {
        std::cout << "\n[Ошибка] Отсутствуют заголовки таблицы.\n";
        return;
    }
    const size_t MAX_COL_WIDTH = 50;
    std::vector<size_t> colWidths(headers.size(), 0);

    for (size_t i = 0; i < headers.size(); ++i) 
        colWidths[i] = std::min(utf8_char_count(headers[i]), MAX_COL_WIDTH);

    for (const auto& row : rows) {
        for (size_t i = 0; i < headers.size(); ++i) {
            if (i < row.size()) {
                size_t cellWidth = std::min(utf8_char_count(row[i]), MAX_COL_WIDTH);
                if (cellWidth > colWidths[i]) colWidths[i] = cellWidth;
            }
        }
    }

    std::cout << "\n--- " << title << " (страница " << currentPage << " из " << totalPages << ") ---\n";
    for (size_t i = 0; i < headers.size(); ++i) {
        if (i > 0) std::cout << " | ";
        std::cout << truncate_to_visual_width(headers[i], colWidths[i]);
    }
    std::cout << '\n';

    size_t totalWidth = 0;
    for (size_t w : colWidths) totalWidth += w;
    if (colWidths.size() > 1) totalWidth += (colWidths.size() - 1) * 3;
    std::cout << std::string(totalWidth, '-') << '\n';

    if (rows.empty()) {
        std::cout << "(нет данных)\n";
    } else {
        for (const auto& row : rows) {
            for (size_t i = 0; i < headers.size(); ++i) {
                if (i > 0) std::cout << " | ";
                std::cout << ((i < row.size()) ? truncate_to_visual_width(row[i], colWidths[i]) : std::string(colWidths[i], ' '));
            }
            std::cout << '\n';
        }
    }
    std::cout << std::string(totalWidth, '-') << '\n';
    std::cout << "(" << rows.size() << " строк" << (rows.size() != 1 ? "и)" : ")") << '\n';
}

// ============ УПРАВЛЕНИЕ ПРЕДПРИЯТИЯМИ ============

void CLIInterface::manageEnterprises() {
    while (true) {
        std::cout << "\n--- Управление предприятиями ---\n";
        std::cout << "1. Просмотр всех предприятий\n";
        std::cout << "2. Добавить предприятие\n";
        std::cout << "3. Редактировать предприятие\n";
        std::cout << "4. Удалить предприятие\n";
        std::cout << "0. Назад\n";
        int choice = getIntegerInput("Выберите действие: ");
        switch (choice) {
            case 1: listEnterprises(); break;
            case 2: addEnterprise(); break;
            case 3: editEnterprise(); break;
            case 4: deleteEnterprise(); break;
            case 0: return;
            default: std::cout << "Неверный выбор.\n";
        }
    }
}

void CLIInterface::listEnterprises(int initialPage, int pageSize) {
    int page = initialPage;
    while (true) {
        // Получаем данные через сервис
        auto enterprises = service.getAllEnterprises();
        int total = enterprises.size();
        int totalPages = (total == 0) ? 1 : (total + pageSize - 1) / pageSize;

        if (page < 1) page = 1;
        if (page > totalPages) page = totalPages;

        std::vector<std::vector<std::string>> rows;
        if (total > 0) {
            int start = (page - 1) * pageSize;
            int end = std::min(start + pageSize, total);
            for (int i = start; i < end; ++i) {
                const auto& e = enterprises[i];
                rows.push_back({
                    std::to_string(i + 1),
                    e.name,
                    e.legal_form_name,
                    e.ownership_form_name,
                    e.inn,
                    e.postal_address
                });
            }
        }

        printTable("Предприятия", page, totalPages, 
            {"№", "Название", "ОПФ", "Форма собственности", "ИНН", "Адрес"}, rows, pageSize);

        std::cout << "\nНавигация: [q] выход";
        if (page > 1) std::cout << ", [p] предыдущая";
        if (total > 0 && page < totalPages) std::cout << ", [n] следующая";
        std::cout << ": ";
        
        char ch;
        std::cin >> ch;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (ch == 'q') return;
        else if (ch == 'p' && page > 1) page--;
        else if (ch == 'n' && total > 0 && page < totalPages) page++;
    }
}

void CLIInterface::addEnterprise() {
    Enterprise ent;
    ent.name = getStringInput("Название предприятия: ");
    ent.inn = getStringInput("ИНН: ");
    ent.postal_address = getStringInput("Почтовый адрес: ");
    ent.legal_form_id = getIntegerInput("ID организационно-правовой формы: ");
    ent.ownership_form_id = getIntegerInput("ID формы собственности: ");

    int id = service.createEnterprise(ent);
    if (id > 0) std::cout << "Предприятие успешно добавлено с ID: " << id << std::endl;
    else std::cout << "Ошибка при добавлении предприятия." << std::endl;
}

void CLIInterface::editEnterprise() {
    int num = getIntegerInput("Введите номер предприятия (по списку): ");
    auto enterprises = service.getAllEnterprises();
    if (num < 1 || num > (int)enterprises.size()) {
        std::cout << "Неверный номер." << std::endl;
        return;
    }

    Enterprise e = enterprises[num - 1]; // Получили копию DTO
    std::cout << "Редактирование: " << e.name << " (ИНН: " << e.inn << ")\n";

    std::string input = getStringInput("Новое название (пусто для сохранения): ");
    if (!input.empty()) e.name = input;

    input = getStringInput("Новый ИНН (пусто для сохранения): ");
    if (!input.empty()) e.inn = input;

    input = getStringInput("Новый адрес (пусто для сохранения): ");
    if (!input.empty()) e.postal_address = input;

    int intInput = getIntegerInput("Новая форма (ID, 0 для сохранения): ");
    if (intInput != 0) e.legal_form_id = intInput;

    intInput = getIntegerInput("Новая собственность (ID, 0 для сохранения): ");
    if (intInput != 0) e.ownership_form_id = intInput;

    if (service.updateEnterprise(e)) std::cout << "Предприятие обновлено.\n";
    else std::cout << "Ошибка при обновлении.\n";
}

void CLIInterface::deleteEnterprise() {
    int num = getIntegerInput("Введите номер предприятия для удаления: ");
    auto enterprises = service.getAllEnterprises();
    if (num < 1 || num > (int)enterprises.size()) {
        std::cout << "Неверный номер.\n"; return;
    }

    Enterprise e = enterprises[num - 1];
    std::cout << "Удалить \"" << e.name << "\"? (y/n): ";
    char confirm; std::cin >> confirm;
    if (confirm == 'y' || confirm == 'Y') {
        if (service.deleteEnterprise(e.id)) std::cout << "Предприятие удалено.\n";
        else std::cout << "Ошибка при удалении.\n";
    }
}

// ============ УПРАВЛЕНИЕ ТОВАРАМИ ============

void CLIInterface::manageProducts() {
    while (true) {
        std::cout << "\n--- Управление товарами ---\n";
        std::cout << "1. Просмотр всех товаров\n";
        std::cout << "2. Добавить товар\n";
        std::cout << "3. Редактировать товар\n";
        std::cout << "4. Удалить товар\n";
        std::cout << "0. Назад\n";
        int choice = getIntegerInput("Выберите действие: ");
        switch (choice) {
            case 1: listProducts(); break;
            case 2: addProduct(); break;
            case 3: editProduct(); break;
            case 4: deleteProduct(); break;
            case 0: return;
            default: std::cout << "Неверный выбор.\n";
        }
    }
}

void CLIInterface::listProducts(int initialPage, int pageSize) {
    int page = initialPage;
    while (true) {
        auto products = service.getAllProducts();
        int total = products.size();
        int totalPages = (total == 0) ? 1 : (total + pageSize - 1) / pageSize;
        if (page < 1) page = 1;
        if (page > totalPages) page = totalPages;

        std::vector<std::vector<std::string>> rows;
        if (total > 0) {
            int start = (page - 1) * pageSize;
            int end = std::min(start + pageSize, total);
            for (int i = start; i < end; ++i) {
                const auto& p = products[i];
                std::ostringstream rS, pS;
                rS << std::fixed << std::setprecision(2) << p.retail_price;
                pS << std::fixed << std::setprecision(2) << p.purchase_price;
                rows.push_back({
                    std::to_string(i + 1), p.name, p.category_name,
                    std::to_string(p.shelf_life_days) + " дн.",
                    p.delivery_terms_description, rS.str(), pS.str()
                });
            }
        }
        printTable("Товары", page, totalPages, 
            {"№", "Наименование", "Категория", "Срок", "Поставка", "Розничная", "Закупочная"}, rows, pageSize);

        std::cout << "\nНавигация: [q] выход";
        if (page > 1) std::cout << ", [p] предыдущая";
        if (total > 0 && page < totalPages) std::cout << ", [n] следующая";
        std::cout << ": ";
        
        char ch; std::cin >> ch;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (ch == 'q') return;
        else if (ch == 'p' && page > 1) page--;
        else if (ch == 'n' && total > 0 && page < totalPages) page++;
    }
}

void CLIInterface::addProduct() {
    Product p;
    p.name = getStringInput("Наименование товара: ");
    p.category_id = getIntegerInput("ID категории товара: ");
    p.shelf_life_days = getIntegerInput("Срок реализации (дней): ");
    p.delivery_terms_id = getIntegerInput("ID условий поставки: ");
    p.retail_price = std::stod(getStringInput("Розничная цена: "));
    p.purchase_price = std::stod(getStringInput("Закупочная цена: "));

    if (service.createProduct(p) > 0) std::cout << "Товар успешно добавлен.\n";
    else std::cout << "Ошибка при добавлении товара.\n";
}

void CLIInterface::editProduct() {
    int num = getIntegerInput("Введите номер товара (по списку): ");
    auto products = service.getAllProducts();
    if (num < 1 || num > (int)products.size()) { std::cout << "Неверный номер.\n"; return; }

    Product p = products[num - 1];
    std::cout << "Редактирование: " << p.name << "\n";

    std::string input = getStringInput("Новое наименование (пусто для сохранения): ");
    if (!input.empty()) p.name = input;

    int intInput = getIntegerInput("Новая категория (0 — оставить): ");
    if (intInput != 0) p.category_id = intInput;

    intInput = getIntegerInput("Новый срок реализации (0 — оставить): ");
    if (intInput != 0) p.shelf_life_days = intInput;

    input = getStringInput("Новая розничная цена (пусто для сохранения): ");
    if (!input.empty()) p.retail_price = std::stod(input);

    if (service.updateProduct(p)) std::cout << "Товар обновлён.\n";
    else std::cout << "Ошибка при обновлении.\n";
}

void CLIInterface::deleteProduct() {
    int num = getIntegerInput("Введите номер товара: ");
    auto products = service.getAllProducts();
    if (num < 1 || num > (int)products.size()) { std::cout << "Неверный номер.\n"; return; }
    
    if (service.deleteProduct(products[num - 1].id)) std::cout << "Товар удалён.\n";
    else std::cout << "Ошибка при удалении.\n";
}

// ============ УПРАВЛЕНИЕ АССОРТИМЕНТОМ ============

void CLIInterface::manageAssortment() {
    std::cout << "\n--- Выбор предприятия ---\n";
    listEnterprises(1, 10);
    int entNum = getIntegerInput("Введите номер предприятия: ");
    auto enterprises = service.getAllEnterprises();
    if (entNum < 1 || entNum > (int)enterprises.size()) { std::cout << "Неверный номер.\n"; return; }
    
    Enterprise selectedEnt = enterprises[entNum - 1];
    std::cout << "\nПредприятие: " << selectedEnt.name << "\n";

    while (true) {
        std::cout << "\n--- Ассортимент ---\n1. Просмотр\n2. Добавить товар\n3. Удалить товар\n4. Изменить цену\n0. Назад\n";
        int choice = getIntegerInput("Выбор: ");
        switch (choice) {
            case 1: listAssortmentForEnterprise(selectedEnt.id); break;
            case 2: addProductToEnterprise(selectedEnt.id); break;
            case 3: removeProductFromEnterprise(selectedEnt.id); break;
            case 4: updateWholesalePrice(selectedEnt.id); break;
            case 0: return;
            default: std::cout << "Неверный выбор.\n";
        }
    }
}

void CLIInterface::listAssortmentForEnterprise(int enterpriseId, int initialPage, int pageSize) {
    int page = initialPage;
    while (true) {
        auto assortment = service.getAssortmentForEnterprise(enterpriseId);
        int total = assortment.size();
        int totalPages = (total == 0) ? 1 : (total + pageSize - 1) / pageSize;
        if (page < 1) page = 1; if (page > totalPages) page = totalPages;

        std::vector<std::vector<std::string>> rows;
        if (total > 0) {
            int start = (page - 1) * pageSize;
            int end = std::min(start + pageSize, total);
            for (int i = start; i < end; ++i) {
                const auto& [product, wholesale] = assortment[i];
                rows.push_back({ std::to_string(i + 1), product.name, std::to_string(wholesale) });
            }
        }
        printTable("Ассортимент", page, totalPages, {"№", "Товар", "Оптовая цена"}, rows, pageSize);
        
        std::cout << "\n[q] выход, [p] назад, [n] вперед: ";
        char ch; std::cin >> ch; std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (ch == 'q') return; else if (ch == 'p' && page > 1) page--; else if (ch == 'n' && total > 0 && page < totalPages) page++;
    }
}

void CLIInterface::addProductToEnterprise(int enterpriseId) {
    listProducts(1, 10);
    int prodNum = getIntegerInput("Введите номер товара: ");
    auto products = service.getAllProducts();
    if (prodNum < 1 || prodNum > (int)products.size()) return;

    double price = std::stod(getStringInput("Оптовая цена: "));
    if (service.addProductToAssortment(enterpriseId, products[prodNum - 1].id, price))
        std::cout << "Добавлено.\n";
    else std::cout << "Ошибка.\n";
}

void CLIInterface::removeProductFromEnterprise(int enterpriseId) {
    // В реальном приложении лучше переиспользовать listAssortmentForEnterprise для выбора
    auto assortment = service.getAssortmentForEnterprise(enterpriseId);
    if (assortment.empty()) { std::cout << "Ассортимент пуст.\n"; return; }

    std::cout << "--- Список ---\n";
    for(size_t i=0; i<assortment.size(); ++i) 
        std::cout << i+1 << ". " << assortment[i].first.name << "\n";

    int num = getIntegerInput("Номер для удаления: ");
    if (num < 1 || num > (int)assortment.size()) return;

    if (service.removeProductFromAssortment(enterpriseId, assortment[num-1].first.id))
        std::cout << "Удалено.\n";
    else std::cout << "Ошибка.\n";
}

void CLIInterface::updateWholesalePrice(int enterpriseId) {
    auto assortment = service.getAssortmentForEnterprise(enterpriseId);
    if (assortment.empty()) { std::cout << "Ассортимент пуст.\n"; return; }
    
    // Упрощенный вывод списка для выбора
    for(size_t i=0; i<assortment.size(); ++i) 
        std::cout << i+1 << ". " << assortment[i].first.name << " (" << assortment[i].second << ")\n";

    int num = getIntegerInput("Номер товара: ");
    if (num < 1 || num > (int)assortment.size()) return;

    double newPrice = std::stod(getStringInput("Новая цена: "));
    if (service.updateProductPriceInAssortment(enterpriseId, assortment[num-1].first.id, newPrice))
        std::cout << "Обновлено.\n";
    else std::cout << "Ошибка.\n";
}

// ============ ОТДЕЛЫ СБЫТА ============

void CLIInterface::manageSalesDepartments() {
    while (true) {
        std::cout << "\n--- Отделы сбыта ---\n1. Просмотр\n2. Добавить\n3. Редактировать\n4. Удалить\n0. Назад\n";
        switch (getIntegerInput("Выбор: ")) {
            case 1: listSalesDepartments(); break;
            case 2: addSalesDepartment(); break;
            case 3: editSalesDepartment(); break;
            case 4: deleteSalesDepartment(); break;
            case 0: return;
        }
    }
}

void CLIInterface::listSalesDepartments(int initialPage, int pageSize) {
    int page = initialPage;
    while (true) {
        auto deps = service.getAllSalesDepartments();
        int total = deps.size();
        int totalPages = (total == 0) ? 1 : (total + pageSize - 1) / pageSize;
        if (page < 1) page = 1; if (page > totalPages) page = totalPages;

        std::vector<std::vector<std::string>> rows;
        if (total > 0) {
            int start = (page - 1) * pageSize;
            int end = std::min(start + pageSize, total);
            for (int i = start; i < end; ++i) {
                const auto& d = deps[i];
                rows.push_back({
                    std::to_string(i + 1), d.enterprise_name, d.phone, d.fax, d.email, 
                    d.contact_last_name + " " + d.contact_first_name
                });
            }
        }
        printTable("Отделы сбыта", page, totalPages, {"№", "Предприятие", "Телефон", "Факс", "Email", "Контакт"}, rows, pageSize);
        
        std::cout << "\n[q] выход, [p] назад, [n] вперед: ";
        char ch; std::cin >> ch; std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (ch == 'q') return; else if (ch == 'p' && page > 1) page--; else if (ch == 'n' && total > 0 && page < totalPages) page++;
    }
}

void CLIInterface::addSalesDepartment() {
    auto enterprises = service.getAllEnterprises();
    // Логика фильтрации доступных предприятий такая же, как раньше, 
    // но теперь мы заполняем структуру.
    std::cout << "Доступные предприятия:\n";
    for(size_t i=0; i<enterprises.size(); ++i) std::cout << i+1 << ". " << enterprises[i].name << "\n";
    
    int entNum = getIntegerInput("Номер предприятия: ");
    if (entNum < 1 || entNum > (int)enterprises.size()) return;

    SalesDepartment sd;
    sd.enterprise_id = enterprises[entNum - 1].id;
    sd.phone = getStringInput("Телефон: ");
    sd.fax = getStringInput("Факс: ");
    sd.email = getStringInput("Email: ");
    sd.contact_last_name = getStringInput("Фамилия: ");
    sd.contact_first_name = getStringInput("Имя: ");
    sd.contact_patronymic = getStringInput("Отчество: ");

    if (service.createSalesDepartment(sd) > 0) std::cout << "Добавлено.\n";
    else std::cout << "Ошибка.\n";
}

void CLIInterface::editSalesDepartment() {
    int num = getIntegerInput("Номер отдела для редактирования (из списка): ");
    auto deps = service.getAllSalesDepartments();
    
    if (num < 1 || num > (int)deps.size()) {
        std::cout << "Неверный номер.\n";
        return;
    }

    // Получаем копию объекта для редактирования
    SalesDepartment sd = deps[num - 1];
    
    std::cout << "\n--- Редактирование отдела (Enterprise: " << sd.enterprise_name << ") ---\n";
    std::cout << "(Оставьте поле пустым и нажмите Enter, чтобы сохранить текущее значение)\n";

    // 1. Телефон
    std::cout << "Текущий телефон: " << (sd.phone.empty() ? "-" : sd.phone) << "\n";
    std::string input = getStringInput("Новый телефон: ");
    if (!input.empty()) sd.phone = input;

    // 2. Факс
    std::cout << "Текущий факс: " << (sd.fax.empty() ? "-" : sd.fax) << "\n";
    input = getStringInput("Новый факс: ");
    if (!input.empty()) sd.fax = input;

    // 3. Email
    std::cout << "Текущий Email: " << (sd.email.empty() ? "-" : sd.email) << "\n";
    input = getStringInput("Новый Email: ");
    if (!input.empty()) sd.email = input;

    // 4. Фамилия
    std::cout << "Текущая Фамилия: " << sd.contact_last_name << "\n";
    input = getStringInput("Новая Фамилия: ");
    if (!input.empty()) sd.contact_last_name = input;

    // 5. Имя
    std::cout << "Текущее Имя: " << sd.contact_first_name << "\n";
    input = getStringInput("Новое Имя: ");
    if (!input.empty()) sd.contact_first_name = input;

    // 6. Отчество
    std::cout << "Текущее Отчество: " << (sd.contact_patronymic.empty() ? "-" : sd.contact_patronymic) << "\n";
    input = getStringInput("Новое Отчество: ");
    if (!input.empty()) sd.contact_patronymic = input;

    // Сохраняем изменения
    if (service.updateSalesDepartment(sd)) {
        std::cout << "Отдел сбыта успешно обновлён.\n";
    } else {
        std::cout << "Ошибка при обновлении (проверьте данные).\n";
    }
}

void CLIInterface::deleteSalesDepartment() {
    int num = getIntegerInput("Номер отдела для удаления: ");
    auto deps = service.getAllSalesDepartments();
    if (num < 1 || num > (int)deps.size()) return;

    if (service.deleteSalesDepartment(deps[num - 1].id)) std::cout << "Удалено.\n";
    else std::cout << "Ошибка.\n";
}

// ============ БАНКОВСКИЕ РЕКВИЗИТЫ ============

void CLIInterface::manageBankDetails() {
    while (true) {
        std::cout << "\n--- Банковские реквизиты ---\n1. Просмотр\n2. Добавить\n3. Редактировать\n4. Удалить\n0. Назад\n";
        switch (getIntegerInput("Выбор: ")) {
            case 1: listBankDetails(); break;
            case 2: addBankDetail(); break;
            case 3: editBankDetail(); break;
            case 4: deleteBankDetail(); break;
            case 0: return;
        }
    }
}

void CLIInterface::listBankDetails(int initialPage, int pageSize) {
    int page = initialPage;
    while (true) {
        auto details = service.getAllBankDetails();
        int total = details.size();
        int totalPages = (total == 0) ? 1 : (total + pageSize - 1) / pageSize;
        if (page < 1) page = 1; if (page > totalPages) page = totalPages;

        std::vector<std::vector<std::string>> rows;
        if (total > 0) {
            int start = (page - 1) * pageSize;
            int end = std::min(start + pageSize, total);
            for (int i = start; i < end; ++i) {
                const auto& bd = details[i];
                rows.push_back({ std::to_string(i + 1), bd.enterprise_name, bd.bank_name, bd.bank_city, bd.account_number });
            }
        }
        printTable("Реквизиты", page, totalPages, {"№", "Предприятие", "Банк", "Город", "Счет"}, rows, pageSize);
        
        std::cout << "\n[q] выход, [p] назад, [n] вперед: ";
        char ch; std::cin >> ch; std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (ch == 'q') return; else if (ch == 'p' && page > 1) page--; else if (ch == 'n' && total > 0 && page < totalPages) page++;
    }
}

void CLIInterface::addBankDetail() {
    auto enterprises = service.getAllEnterprises();
    std::cout << "Доступные предприятия:\n";
    for(size_t i=0; i<enterprises.size(); ++i) std::cout << i+1 << ". " << enterprises[i].name << "\n";

    int entNum = getIntegerInput("Номер предприятия: ");
    if (entNum < 1 || entNum > (int)enterprises.size()) return;

    BankDetails bd;
    bd.enterprise_id = enterprises[entNum - 1].id;
    bd.bank_name = getStringInput("Банк: ");
    bd.bank_city = getStringInput("Город: ");
    bd.account_number = getStringInput("Счет: ");

    if (service.createBankDetails(bd) > 0) std::cout << "Добавлено.\n";
    else std::cout << "Ошибка.\n";
}

void CLIInterface::editBankDetail() {
    int num = getIntegerInput("Номер записи реквизитов (из списка): ");
    auto details = service.getAllBankDetails();
    
    if (num < 1 || num > (int)details.size()) {
        std::cout << "Неверный номер.\n";
        return;
    }

    BankDetails bd = details[num - 1];

    std::cout << "\n--- Редактирование реквизитов (Enterprise: " << bd.enterprise_name << ") ---\n";
    std::cout << "(Оставьте поле пустым и нажмите Enter, чтобы сохранить текущее значение)\n";

    // 1. Название банка
    std::cout << "Текущий банк: " << bd.bank_name << "\n";
    std::string input = getStringInput("Новый банк: ");
    if (!input.empty()) bd.bank_name = input;

    // 2. Город банка
    std::cout << "Текущий город: " << bd.bank_city << "\n";
    input = getStringInput("Новый город: ");
    if (!input.empty()) bd.bank_city = input;

    // 3. Расчетный счет
    std::cout << "Текущий счет: " << bd.account_number << "\n";
    input = getStringInput("Новый счет: ");
    if (!input.empty()) bd.account_number = input;

    // Сохраняем
    if (service.updateBankDetails(bd)) {
        std::cout << "Банковские реквизиты успешно обновлены.\n";
    } else {
        std::cout << "Ошибка при обновлении.\n";
    }
}

void CLIInterface::deleteBankDetail() {
    int num = getIntegerInput("Номер записи: ");
    auto details = service.getAllBankDetails();
    if (num < 1 || num > (int)details.size()) return;

    if (service.deleteBankDetails(details[num - 1].id)) std::cout << "Удалено.\n";
    else std::cout << "Ошибка.\n";
}

// ============ ОБЩИЕ ============

int CLIInterface::getIntegerInput(const std::string& prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        } else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Ошибка ввода.\n";
        }
    }
}

std::string CLIInterface::getStringInput(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return input;
}

void CLIInterface::pause() {
    std::cout << "\nНажмите Enter...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}