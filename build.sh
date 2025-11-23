#!/bin/bash

# build.sh — скрипт для сборки проекта "Реестр предприятий"

set -e  # завершить при первой ошибке

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$PROJECT_ROOT/src"
INCLUDE_DIR="$PROJECT_ROOT/include"
BIN_DIR="$PROJECT_ROOT/bin"
BUILD_DIR="$PROJECT_ROOT/build"  # временная папка для объектных файлов (опционально)

# Создаём папку bin
mkdir -p "$BIN_DIR"

# Находим все .h файлы в include/
HEADERS=("$INCLUDE_DIR"/*.h)
# Находим все .cpp файлы в src/
SOURCES=("$SRC_DIR"/*.cpp)

# Проверяем, что файлы .h найдены
if [ ! -f "${HEADERS[0]}" ]; then
    echo "Ошибка: не найдены исходные файлы в $INCLUDE_DIR/"
    exit 1
fi
# Проверяем, что файлы .cpp найдены
if [ ! -f "${SOURCES[0]}" ]; then
    echo "Ошибка: не найдены исходные файлы в $SRC_DIR/"
    exit 1
fi

echo "Сборка проекта..."
echo "Исходники:"
echo "      .h:"
printf '%s\n' "${HEADERS[@]}"
echo "      .cpp:"
printf '%s\n' "${SOURCES[@]}"

# Компиляция и линковка одной командой
g++ -std=c++17 \
    -I"$INCLUDE_DIR" \
    -o "$BIN_DIR/RegEnterprise" \
    "${SOURCES[@]}" \
    -lodbc

echo "✅ Сборка завершена. Исполняемый файл: $BIN_DIR/RegEnterprise"
echo "Запуск (пример):"
echo "  ODBCINI=\"./.odbc.ini\" ./bin/RegEnterprise"