#!/usr/bin/env bash
# Сборка всех программ в ./build. Зависимостей, кроме компилятора с C++17, нет.
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p build

CXX=${CXX:-g++}
CXXFLAGS="-std=c++17 -O2"

echo "Куб (19 подзадач):"
for src in cube/experiments/*.cpp; do
    name=$(basename "$src" .cpp)
    printf '  %s\n' "$name"
    $CXX $CXXFLAGS -Icube/include -o "build/$name" "$src" cube/src/helpers.cpp
done

echo "Срезы (3 программы):"
for name in larman_sweep larman_ncolors; do
    printf '  %s\n' "$name"
    $CXX $CXXFLAGS -Islices -o "build/$name" "slices/$name.cpp"
done
printf '  %s\n' larman_perebor
$CXX $CXXFLAGS -Islices -pthread -o build/larman_perebor \
     slices/larman_perebor.cpp slices/sat.cpp

echo "Готово. Бинарники в ./build"
