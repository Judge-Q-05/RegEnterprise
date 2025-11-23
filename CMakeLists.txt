# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(RegistryEnterprise VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Пути к заголовкам
include_directories(${PROJECT_SOURCE_DIR}/include)

# Список исходных файлов
file(GLOB SOURCES "src/*.cpp")

# Исполняемый файл
add_executable(RegEnterprise ${SOURCES})

# Ссылка на системную библиотеку ODBC
find_library(ODBC_LIBRARY odbc REQUIRED)
target_link_libraries(RegEnterprise ${ODBC_LIBRARY})

# Создаём исполняемый файл в папке bin на уровне исходного кода (рядом с CMakeLists.txt)
set_target_properties(RegEnterprise PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)