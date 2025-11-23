#ifndef CLI_INTERFACE_H
#define CLI_INTERFACE_H

#include "RegistryService.h" // Единственная точка входа в бизнес-логику
#include <string>
#include <vector>

class CLIInterface {
private:
    RegistryService service; // Владеем сервисом, сервис владеет шлюзами и БД

    // Вспомогательный метод вывода таблиц
    static void printTable(
        const std::string& title,
        int currentPage,
        int totalPages,
        const std::vector<std::string>& headers,
        const std::vector<std::vector<std::string>>& rows,
        int pageSize
    );

    // Меню и навигация
    void showMainMenu();
    
    // Подменю сущностей
    void manageEnterprises();
    void manageProducts();
    void manageAssortment();
    void manageSalesDepartments();
    void manageBankDetails();

    // Списки (вывод данных)
    void listEnterprises(int page = 1, int pageSize = 5);
    void listProducts(int page = 1, int pageSize = 5);
    void listAssortmentForEnterprise(int enterpriseId, int page = 1, int pageSize = 5);
    void listSalesDepartments(int initialPage = 1, int pageSize = 10);
    void listBankDetails(int initialPage = 1, int pageSize = 10);

    // Операции добавления (CRUD)
    void addEnterprise();
    void addProduct();
    void addSalesDepartment();
    void addBankDetail();
    void addProductToEnterprise(int enterpriseId); // Для ассортимента

    // Операции редактирования (CRUD)
    void editEnterprise();
    void editProduct();
    void editSalesDepartment();
    void editBankDetail();
    void updateWholesalePrice(int enterpriseId); // Для ассортимента

    // Операции удаления (CRUD)
    void deleteEnterprise();
    void deleteProduct();
    void deleteSalesDepartment();
    void deleteBankDetail();
    void removeProductFromEnterprise(int enterpriseId); // Для ассортимента

    // Утилиты ввода
    int getIntegerInput(const std::string& prompt);
    std::string getStringInput(const std::string& prompt);
    void pause();

public:
    CLIInterface(); 
    void run();
};

#endif