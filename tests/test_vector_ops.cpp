#include <iostream>
#include <cassert>
#include <cmath>
#include "TinyGeo/Vector.h"

// Una macro simple para validar flotantes con tolerancia
#define ASSERT_NEAR(val1, val2, epsilon) \
    assert(std::abs((val1) - (val2)) < (epsilon) && "Value mismatch")

void test_arithmetic() {
    using namespace TinyGeo;
    Vector<float, 3> v1 = {1.0f, 2.0f, 3.0f};
    Vector<float, 3> v2 = {4.0f, 5.0f, 6.0f};
    
    Vector<float, 3> sum = v1 + v2;

    ASSERT_NEAR(sum.x(), 5.0f, 1e-5f);
    ASSERT_NEAR(sum.y(), 7.0f, 1e-5f);
    ASSERT_NEAR(sum.z(), 9.0f, 1e-5f);

    
    std::cout << "[PASS] Arithmetic" << std::endl;
}

void test_geometry() {
    using namespace TinyGeo;
    Vector<float, 3> x = {1.0f, 0.0f, 0.0f};
    Vector<float, 3> y = {0.0f, 1.0f, 0.0f};
    
    // Dot product ortogonal
    ASSERT_NEAR(dot(x, y), 0.0f, 1e-5f);
    
    // Cross product
    Vector<float, 3> z = cross(x, y);

    ASSERT_NEAR(z.z(), 1.0f, 1e-5f); // Debe ser (0,0,1)
    
    std::cout << "[PASS] Geometry" << std::endl;
}

int main() {
    test_arithmetic();
    test_geometry();
    // Si llegamos aquí, todo pasó. Retornar 0 es "Success" para CTest.
    return 0;
}