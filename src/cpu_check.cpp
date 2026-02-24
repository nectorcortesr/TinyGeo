#include <iostream>
#include <vector>
#include <string>
#include <array>

// Esta librería es específica de GCC/Clang en Linux/Mac.
// Nos da acceso a la instrucción de ensamblador 'cpuid'.
#include <cpuid.h> 

class CPUID {
public:
    // Registros del CPU donde cpuid guarda la respuesta: EAX, EBX, ECX, EDX
    // Son enteros de 32 bits.
    unsigned int regs[4];

    // Constructor: Llama a la instrucción __cpuid
    // func_id: Es la pregunta que le hacemos al CPU.
    // 0 = Vendor ID (Intel/AMD)
    // 1 = Features básicas (SSE, etc)
    // 7 = Features extendidas (AVX2)
    explicit CPUID(unsigned int func_id, unsigned int sub_func_id = 0) {
        // __cpuid_count es una macro de <cpuid.h>
        // Pone func_id en EAX, sub_func_id en ECX, y ejecuta la instrucción 'cpuid'
        // Los resultados quedan en regs[0]...regs[3]
        __cpuid_count(func_id, sub_func_id, regs[0], regs[1], regs[2], regs[3]);
    }

    const unsigned int& EAX() const { return regs[0]; }
    const unsigned int& EBX() const { return regs[1]; }
    const unsigned int& ECX() const { return regs[2]; }
    const unsigned int& EDX() const { return regs[3]; }
};

int main() {
    std::cout << "--- TinyGeo Hardware Diagnostic ---" << std::endl;

    // 1. Obtener Vendor (Fabricante)
    // Pregunta 0: El CPU responde con strings en EBX, EDX, ECX
    CPUID cpuID0(0);
    std::string vendor;
    vendor += std::string((const char*)&cpuID0.EBX(), 4);
    vendor += std::string((const char*)&cpuID0.EDX(), 4);
    vendor += std::string((const char*)&cpuID0.ECX(), 4);

    std::cout << "CPU Vendor: " << vendor << std::endl;

    // 2. Chequear SSE y AVX (Pregunta 1)
    CPUID cpuID1(1);
    
    // Los bits específicos están documentados en manuales de Intel/AMD.
    // Bit 25 de ECX = SSE3
    // Bit 28 de ECX = AVX (Advanced Vector Extensions)
    bool has_sse3 = (cpuID1.ECX() & (1 << 0)); 
    bool has_sse41 = (cpuID1.ECX() & (1 << 19));
    bool has_avx   = (cpuID1.ECX() & (1 << 28));

    std::cout << "Instruction Sets Support:" << std::endl;
    std::cout << "  SSE3:   " << (has_sse3 ? "Yes" : "No") << std::endl;
    std::cout << "  SSE4.1: " << (has_sse41 ? "Yes" : "No") << std::endl;
    std::cout << "  AVX:    " << (has_avx ? "Yes (128/256 bit capable)" : "No") << std::endl;

    // 3. Chequear AVX2 (Pregunta 7, Sub-pregunta 0)
    // AVX2 añade instrucciones para manejar enteros en vectores de 256 bits, muy útil.
    CPUID cpuID7(7, 0);
    bool has_avx2 = (cpuID7.EBX() & (1 << 5));

    std::cout << "  AVX2:   " << (has_avx2 ? "Yes (Optimized Integers)" : "No") << std::endl;

    // 4. Chequear FMA (Fused Multiply-Add)
    // FMA permite hacer (a * b + c) en UN solo paso. Crítico para matrices.
    bool has_fma = (cpuID1.ECX() & (1 << 12));
    std::cout << "  FMA3:   " << (has_fma ? "Yes (Critical for Matrix Mul)" : "No") << std::endl;

    std::cout << "-----------------------------------" << std::endl;
    if (has_avx && has_fma) {
        std::cout << "VERDICT: Your system is ready for High-Performance Optimization." << std::endl;
    } else {
        std::cout << "VERDICT: Hardware limited. Optimizations will be less effective." << std::endl;
    }

    return 0;
}