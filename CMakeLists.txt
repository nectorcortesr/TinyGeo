cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto y versión
project(TinyGeo VERSION 1.0 LANGUAGES CXX)

# Estandar C++ 17 (Requerido para features modernas como std::optional, std::variant)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Queremos ISO C++ estándar, no extensiones de GCC

# FLAGS SENIOR: Tratamos los warnings como errores.
# Esto te obliga a escribir código limpio desde el día 1.
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Definimos dónde están nuestros headers (.h)
include_directories(include)

# Definimos el ejecutable principal
add_executable(tinygeo_runner src/main.cpp)

# (Opcional por ahora) Linkear librerías si las tuviéramos
# target_link_libraries(tinygeo_runner PRIVATE ...)

# Habilitar testing (Standard CTest)
enable_testing()

# Agregar el ejecutable de test
add_executable(unit_tests tests/test_vector_ops.cpp)

# Linkear include directories (importante para encontrar Vector.h)
target_include_directories(unit_tests PRIVATE include)

# Flags estrictos también para los tests
if(MSVC)
    target_compile_options(unit_tests PRIVATE /W4 /WX)
else()
    target_compile_options(unit_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Registrar el test en CTest
# Nombre del test: "VectorOps"
# Comando a correr: unit_tests
add_test(NAME VectorOps COMMAND unit_tests)