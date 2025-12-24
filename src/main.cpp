#include <iostream>
#include "TinyGeo/Vector.h"

int main() {
    using namespace TinyGeo;

    Vector<float, 3> right   = {1, 0, 0}; // Eje X
    Vector<float, 3> forward = {0, 1, 0}; // Eje Y
    
    // Regla mano derecha: X cross Y = Z
    Vector<float, 3> up = cross(right, forward);

    std::cout << "Right:   " << right << std::endl;
    std::cout << "Forward: " << forward << std::endl;
    std::cout << "Up (RxF):" << up << " (Expected: [0, 0, 1])" << std::endl;

    // Prueba de Anticommutatividad
    Vector<float, 3> down = cross(forward, right);
    std::cout << "Down(FxR):" << down << " (Expected: [0, 0, -1])" << std::endl;

    // PRUEBA DE SEGURIDAD (Descomenta para ver el error de compilación)
    /*
    Vector<float, 2> v2a = {1, 2};
    Vector<float, 2> v2b = {3, 4};
    auto v2c = cross(v2a, v2b); // ERROR: Static assertion failed
    */

    return 0;
}