#ifndef DOMAIN_ENTITIES_H
#define DOMAIN_ENTITIES_H

#include <string>

struct Enterprise {
    int id;
    std::string name;
    int legal_form_id;
    int ownership_form_id;
    std::string legal_form_name;
    std::string ownership_form_name;
    std::string postal_address;
    std::string inn;
};

struct Product {
    int id;
    int category_id;
    std::string name;
    int shelf_life_days;
    int delivery_terms_id;
    double retail_price;
    double purchase_price;
    std::string category_name;
    std::string delivery_terms_description;
};

struct EnterpriseProduct {
    int enterprise_id;
    int product_id;
    double wholesale_price;
};

struct SalesDepartment {
    int id;
    int enterprise_id;
    std::string enterprise_name;
    std::string phone;
    std::string fax;
    std::string email;
    std::string contact_last_name;
    std::string contact_first_name;
    std::string contact_patronymic;
};

struct BankDetails {
    int id;
    int enterprise_id;
    std::string enterprise_name;
    std::string bank_name;
    std::string bank_city;
    std::string account_number;
};

#endif