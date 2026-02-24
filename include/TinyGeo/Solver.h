#pragma once

#include <cmath>
#include "Matrix.h"
#include "Vector.h"
#include <utility>

namespace TinyGeo {

    // ============================================================
    // Resultado de la descomposición SVD
    // A = U * diag(sigma) * V^T
    // ============================================================
    template <typename T, size_t N>
    struct SVDResult {
        Matrix<T, N, N> U;
        Vector<T, N> sigma;
        Matrix<T, N, N> V;

        // Reconstrucción de A para validación
        Matrix<T, N, N> reconstruct() const {
            Matrix<T, N, N> U_scaled = U;

            // Escalar columnas de U por sigma
            for (size_t j = 0; j < N; ++j) {
                for (size_t i = 0; i < N; ++i) {
                    U_scaled(i, j) *= sigma[j];
                }
            }

            return U_scaled * V.transpose();
        }
    };

    // ============================================================
    // Solver Jacobi SVD (en construcción)
    // ============================================================
    template <typename T, size_t N>
    class Solver {
    public:
        // ------------------------------------------------------------
        // Placeholder SVD (para compilar por ahora)
        // ------------------------------------------------------------
        static SVDResult<T, N> svd(const Matrix<T, N, N>& A) {
            SVDResult<T, N> res;
            
            // 1. Inicialización
            // Copiamos A en U. Al final, U contendrá (U_real * Sigma).
            // Trabajamos sobre "U" como nuestra matriz de trabajo.
            res.U = A; 
            
            // V comienza como identidad y acumulará todas las rotaciones.
            res.V = Matrix<T, N, N>::identity();

            // 2. Parámetros de iteración
            const int MAX_SWEEPS = 20; // 20 pasadas es más que suficiente para float
            const T EPSILON = T(1e-9);

            // 3. Bucle Principal (The Sweeps)
            for (int sweep = 0; sweep < MAX_SWEEPS; ++sweep) {
                bool changed = false; // Para detectar convergencia temprana

                // Iterar sobre todos los pares posibles (0,1), (0,2)... (i, j)
                for (size_t i = 0; i < N - 1; ++i) {
                    for (size_t j = i + 1; j < N; ++j) {
                        
                        // a) Extraer columnas como vectores para calcular el ángulo
                        Vector<T, N> col_i, col_j;
                        for(size_t k=0; k<N; ++k) col_i[k] = res.U(k, i);
                        for(size_t k=0; k<N; ++k) col_j[k] = res.U(k, j);

                        // b) Calcular rotación necesaria para este par
                        JacobiRotation rot = symSchur2(col_i, col_j);

                        // c) Optimización: Si s es casi 0, ya son ortogonales. Saltamos.
                        if (std::abs(rot.s) > EPSILON) {
                            changed = true;
                            // d) Aplicar rotación a la matriz de trabajo (U)
                            apply_jacobi(res.U, i, j, rot);
                            
                            // e) Aplicar LA MISMA rotación a V
                            // (V acumula las rotaciones a la derecha)
                            apply_jacobi(res.V, i, j, rot);
                        }
                    }
                }

                if (!changed) break; // Convergencia total alcanzada
            }

            // 4. Post-Procesamiento (Separar U de Sigma)
            // En este punto, las columnas de res.U son ortogonales, pero no unitarias.
            // Su longitud es el valor singular (Sigma).
            for (size_t i = 0; i < N; ++i) {
                // Calcular norma de la columna
                Vector<T, N> col;
                for(size_t k=0; k<N; ++k) col[k] = res.U(k, i);
                
                res.sigma[i] = col.norm();

                // Normalizar la columna en U para que sea una rotación pura
                // Si sigma es muy pequeño, cuidado con la división por cero
                if (res.sigma[i] > EPSILON) {
                    for(size_t k=0; k<N; ++k) {
                        res.U(k, i) /= res.sigma[i];
                    }
                } else {
                    // Valor singular nulo -> Columna a cero o arbitraria
                     res.sigma[i] = T(0);
                }
            }

            // 5. Ordenamiento (Sorting)
            // SVD estándar requiere que los valores singulares sean descendentes:
            // sigma_0 >= sigma_1 >= ... >= sigma_n
            for (size_t i = 0; i < N - 1; ++i) {
                size_t max_idx = i;
                for (size_t j = i + 1; j < N; ++j) {
                    if (res.sigma[j] > res.sigma[max_idx]) {
                        max_idx = j;
                    }
                }
                
                // Si encontramos un sigma mayor más adelante, intercambiamos
                if (max_idx != i) {
                    // a) Intercambiar el valor escalar en sigma
                    std::swap(res.sigma[i], res.sigma[max_idx]);
                    
                    // b) CRÍTICO: Intercambiar las COLUMNAS correspondientes en U
                    for (size_t k = 0; k < N; ++k) {
                        std::swap(res.U(k, i), res.U(k, max_idx));
                    }
                    
                    // c) CRÍTICO: Intercambiar las COLUMNAS correspondientes en V
                    for (size_t k = 0; k < N; ++k) {
                        std::swap(res.V(k, i), res.V(k, max_idx));
                    }
                }
            }

            return res;
        }

        // ------------------------------------------------------------
        // Estructura ligera de rotación de Jacobi
        // ------------------------------------------------------------
        struct JacobiRotation {
            T c;
            T s;
        };

        // ------------------------------------------------------------
        // Calcula la rotación de Jacobi para ortogonalizar
        // dos columnas p y q (One-Sided Jacobi)
        // ------------------------------------------------------------
        static JacobiRotation symSchur2(
            const Vector<T, N>& p,
            const Vector<T, N>& q
        ) {
            T p_dot_p = p.dot(p);
            T q_dot_q = q.dot(q);
            T p_dot_q = p.dot(q);

            // Si ya son casi ortogonales, no rotamos
            if (std::abs(p_dot_q) < T(1e-9)) {
                return { T(1), T(0) };
            }

            // Cálculo robusto de la tangente
            T tau = (q_dot_q - p_dot_p) / (T(2) * p_dot_q);
            T t;

            if (tau >= T(0)) {
                t = T(1) / (tau + std::sqrt(T(1) + tau * tau));
            } else {
                t = T(-1) / (-tau + std::sqrt(T(1) + tau * tau));
            }

            T c = T(1) / std::sqrt(T(1) + t * t);
            T s = t * c;

            return { c, s };
        }

        // ------------------------------------------------------------
        // Aplica la rotación de Jacobi (ROTACIÓN POR LA DERECHA)
        // p' =  c*p + s*q
        // q' = -s*p + c*q
        // ------------------------------------------------------------
        static void apply_jacobi(Matrix<T, N, N>& M, size_t p, size_t q, const JacobiRotation& rot) {
            for (size_t i = 0; i < N; ++i) {
                T mp = M(i, p); // Copia temporal del valor en columna p
                T mq = M(i, q); // Copia temporal del valor en columna q
                
                // Usamos TU fórmula validada:
                M(i, p) = mp * rot.c - mq * rot.s;
                M(i, q) = mp * rot.s + mq * rot.c;
            }
        }

        // ------------------------------------------------------------
        // Calcula la Pseudoinversa de Moore-Penrose (A^+)
        // A^+ = V * Sigma^+ * U^T
        // ------------------------------------------------------------
        static Matrix<T, N, N> pseudoInverse(const SVDResult<T, N>& svd_res, T tolerance = T(1e-5)) {
            Matrix<T, N, N> Sigma_inv = Matrix<T, N, N>::identity(); // Iniciamos vacía
            for(size_t i=0; i<N; ++i) {
                for(size_t j=0; j<N; ++j) {
                    Sigma_inv(i,j) = T(0); // Limpiar
                }
            }

            // Invertir solo los valores singulares "sanos" (Thresholding)
            for (size_t i = 0; i < N; ++i) {
                if (svd_res.sigma[i] > tolerance) {
                    Sigma_inv(i, i) = T(1) / svd_res.sigma[i];
                }
            }

            // A^+ = V * Sigma_inv * U^T
            // Usamos los operadores de nuestra clase Matrix
            return svd_res.V * Sigma_inv * svd_res.U.transpose();
        }
    };

} // namespace TinyGeo
