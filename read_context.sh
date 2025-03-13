#!/bin/bash

# Папка для сканирования
DIR="./main"  # Укажите нужную папку
OUTPUT_FILE="context.txt"

# Очистка файла перед записью
> "$OUTPUT_FILE"

# Записываем структуру папок и файлов
echo "Структура папок и файлов:" >> "$OUTPUT_FILE"
tree "$DIR" >> "$OUTPUT_FILE" 2>/dev/null || find "$DIR" >> "$OUTPUT_FILE"
echo -e "\n\n" >> "$OUTPUT_FILE"

# Рекурсивное чтение файлов и запись их содержимого в выходной файл
find "$DIR" -type f | while read -r file; do
    echo "=== $file ===" >> "$OUTPUT_FILE"
    cat "$file" >> "$OUTPUT_FILE"
    echo -e "\n" >> "$OUTPUT_FILE"
done

echo "Структура и содержимое всех файлов записаны в $OUTPUT_FILE"
