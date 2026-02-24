#include <benchmark/benchmark.h>
#include "TinyGeo/Matrix.h"
#include <string>

using namespace TinyGeo;

// --- DEFINICIÓN DE LOS CASOS ---

// Caso Ingenuo (Naive)
template <size_t N>
static void BM_Naive(benchmark::State& state) {
    // Etiqueta para saber cuánto ocupa en memoria
    size_t bytes = N * N * sizeof(float);
    std::string label = std::to_string(bytes / 1024) + " KB";
    state.SetLabel(label);

    auto A = Matrix<float, N, N>::identity();
    auto B = Matrix<float, N, N>::identity();

    for (auto _ : state) {
        auto C = A * B;
        benchmark::DoNotOptimize(C);
    }
}

// Caso Transpuesta (Fast?)
template <size_t N>
static void BM_Transposed(benchmark::State& state) {
    size_t bytes = N * N * sizeof(float);
    std::string label = std::to_string(bytes / 1024) + " KB";
    state.SetLabel(label);

    auto A = Matrix<float, N, N>::identity();
    auto B = Matrix<float, N, N>::identity();

    for (auto _ : state) {
        auto C = Matrix<float, N, N>::multiply_fast(A, B);
        benchmark::DoNotOptimize(C);
    }
}

// --- INSTANCIACIÓN DE TAMAÑOS ---

// 1. Pequeño (Cabe en L1 - 48KB) -> Naive debería ganar
// 64x64 floats = 16 KB
BENCHMARK_TEMPLATE(BM_Naive, 64);
BENCHMARK_TEMPLATE(BM_Transposed, 64);

// 2. Mediano (Cabe en L2 - 2MB) -> Zona de guerra
// 256x256 floats = 256 KB
BENCHMARK_TEMPLATE(BM_Naive, 256);
BENCHMARK_TEMPLATE(BM_Transposed, 256);

// 3. Grande (Rompe L2 en algunas CPUs o empieza a saturar ancho de banda)
// 512x512 floats = 1 MB (4 MB totales A+B+C + Overhead transposición)
BENCHMARK_TEMPLATE(BM_Naive, 512);
BENCHMARK_TEMPLATE(BM_Transposed, 512);

// 4. Masivo (Para tu CPU bestia, vamos directo a 1024)
// 1024x1024 floats = 4 MB por matriz.
// A(4MB) + B(4MB) + C(4MB) = 12 MB de "Working Set".
// Esto cabe en tu L3 (30MB), pero ya es lo suficientemente grande 
// para que los patrones de acceso importen mucho más.
BENCHMARK_TEMPLATE(BM_Naive, 1024);
BENCHMARK_TEMPLATE(BM_Transposed, 1024);

BENCHMARK_MAIN();