cmake_minimum_required(VERSION 3.10)
project(Borsuk_plus_kissat_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Для debug — добавь флаги, если нужно
set(CMAKE_BUILD_TYPE Debug)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -Wall -Wextra)
endif()

# Найти Catch2 (убрал фиксированный путь; если не находит — укажи Catch2_DIR=... при cmake)
find_package(Catch2 3 REQUIRED)

# Библиотека для helpers (с исходниками)
add_library(borsuk_lib STATIC helpers_src/helpers.cpp)
target_include_directories(borsuk_lib PUBLIC src/include)

# Headers-only библиотека для bruteforce.h
add_library(bruteforce_lib INTERFACE)
target_include_directories(bruteforce_lib INTERFACE src/include)

# Тесты
add_executable(run_tests catch2_tests/test_helpers.cpp)
target_link_libraries(run_tests PRIVATE Catch2::Catch2WithMain borsuk_lib bruteforce_lib)

# Автоматически находим все файлы экспериментов
file(GLOB EXPERIMENT_SOURCES "experiments/*.cpp")

# Создаём отдельный исполняемый файл для каждого эксперимента
foreach(EXPERIMENT_SOURCE ${EXPERIMENT_SOURCES})
    # Извлекаем имя файла без пути и расширения
    get_filename_component(EXPERIMENT_NAME ${EXPERIMENT_SOURCE} NAME_WE)
    # Создаём исполняемый файл
    add_executable(${EXPERIMENT_NAME} ${EXPERIMENT_SOURCE})
    # Линкуем с библиотеками
    target_link_libraries(${EXPERIMENT_NAME} PRIVATE borsuk_lib bruteforce_lib)
endforeach()