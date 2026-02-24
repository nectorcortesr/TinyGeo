#include <iostream>
#include "TinyGeo/Matrix.h"
#include "TinyGeo/Vector.h"
#include "TinyGeo/Solver.h"

int main() {
    using namespace TinyGeo;
    std::cout << "--- DAY 21: Least Squares Linear Solver ---" << std::endl;

    // 1. Simulación de mediciones del "Sensor" (Cámara detectando carril)
    // El "Ground Truth" real que el robot no conoce es: y = 2.0*x + 1.0
    // Hemos medido 5 puntos con algo de ruido:
    // P1(1.0, 3.1), P2(2.0, 4.9), P3(3.0, 7.2), P4(4.0, 8.8), P5(5.0, 11.1)

    // Matriz A (5x2): Columna 0 son las 'x', Columna 1 son puros 1s.
    Matrix<float, 5, 2> A = {
        1.0f, 1.0f,
        2.0f, 1.0f,
        3.0f, 1.0f,
        4.0f, 1.0f,
        5.0f, 1.0f
    };

    // Vector b (5x1): Las mediciones 'y' observadas
    Vector<float, 5> b = { 3.1f, 4.9f, 7.2f, 8.8f, 11.1f };

    // 2. Las Ecuaciones Normales: H*x = v  ==>  (A^T * A) * x = (A^T * b)
    Matrix<float, 2, 5> A_T = A.transpose();
    
    // Matriz H (2x2)
    Matrix<float, 2, 2> H = A_T * A;
    
    // Vector v (2x1)
    Vector<float, 2> v = A_T * b;

    std::cout << "Solving system H*x = v..." << std::endl;

    // 3. Resolver usando nuestro motor SVD!
    auto svd_H = Solver<float, 2>::svd(H);
    
    // Calcular Pseudoinversa de H (H^+)
    Matrix<float, 2, 2> H_inv = Solver<float, 2>::pseudoInverse(svd_H);

    // Encontrar incógnitas: x = H^+ * v
    Vector<float, 2> x = H_inv * v;

    // 4. Resultados
    float m_est = x[0]; // Pendiente
    float c_est = x[1]; // Intersección (Offset)

    std::cout << "\n[RESULTS]" << std::endl;
    std::cout << "Ground Truth Line : y = 2.0x + 1.0" << std::endl;
    std::cout << "Estimated Line    : y = " << m_est << "x + " << c_est << std::endl;

    // Verificar el error de nuestro ajuste
    std::cout << "\n[RESIDUAL ANALYSIS]" << std::endl;
    float total_error = 0.0f;
    for(size_t i = 0; i < 5; ++i) {
        float x_val = A(i, 0);
        float y_obs = b[i];
        float y_pred = m_est * x_val + c_est; // Predicción del modelo
        float error = std::abs(y_pred - y_obs);
        total_error += error * error; // Error cuadrático
        std::cout << "Point " << i+1 << " -> Obs: " << y_obs << ", Pred: " << y_pred << " (Err: " << error << ")" << std::endl;
    }
    std::cout << "Sum of Squared Errors (SSE): " << total_error << std::endl;

    return 0;
}