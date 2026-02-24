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


# --- TOOLS ---
# Herramienta de diagnóstico de CPU
add_executable(cpu_check src/cpu_check.cpp)

# Flags de compilación:
# Aunque cpu_check solo consulta info, es buena práctica mantener los warnings.
if(MSVC)
    target_compile_options(cpu_check PRIVATE /W4)
else()
    target_compile_options(cpu_check PRIVATE -Wall -Wextra)
endif()

# --- BENCHMARKING ---

include(FetchContent)

# Desactivar tests de la librería benchmark para ahorrar tiempo
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

# 1. Definir dónde bajar Google Benchmark
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.8.3  # Versión estable reciente
)

# 2. Descargar y hacerlo disponible (puede tardar un poco la primera vez)
FetchContent_MakeAvailable(googlebenchmark)

# 3. Crear ejecutable de benchmark
add_executable(bench_runner benchmarks/bench_matrix.cpp)

# 4. Linkear con Google Benchmark y nuestra librería
target_link_libraries(bench_runner PRIVATE benchmark::benchmark)
# Nota: 'TinyGeo' aquí asume que es header-only (interface library), 
# así que solo necesitamos los includes.
target_include_directories(bench_runner PRIVATE include)

# Flags de optimización MÁXIMA para benchmarks (queremos medir el mejor caso)
if(MSVC)
    target_compile_options(bench_runner PRIVATE /O2)
else()
    target_compile_options(bench_runner PRIVATE -O3)
endif()


add_executable(linear_solver src/linear_solver.cpp)

target_include_directories(linear_solver PRIVATE include)