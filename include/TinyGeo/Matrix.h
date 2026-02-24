#pragma once
#include <array>
#include <iostream>
#include <cassert>
#include <initializer_list>
#include "Vector.h"

namespace TinyGeo {

    // T: Tipo de dato (float, double)
    // Rows: Número de filas
    // Cols: Número de columnas
    template <typename T, size_t Rows, size_t Cols>
    class Matrix {
    public:
        // --- ALMACENAMIENTO ---
        // Aplanamos la matriz 2D en un array 1D.
        // Esto garantiza memoria contigua (cache friendly).
        std::array<T, Rows * Cols> data;

        // --- CONSTRUCTORES ---
        
        // Default: Inicializa a cero (importante para evitar basura)
        Matrix() : data{} {}

        // Constructor de lista plana: Matrix m = {1,2,3,4...};
        Matrix(std::initializer_list<T> list) {
            assert(list.size() == Rows * Cols && "Initializer list size mismatch");
            size_t i = 0;
            for (const auto& val : list) {
                data[i++] = val;
            }
        }

        // --- ACCESO (La parte crítica) ---

        // Usamos operator() en lugar de [][] porque [][] requiere 
        // clases proxy complicadas en C++ para ser eficiente.
        // Uso: mat(1, 2) = 5.0f;
        
        T& operator()(size_t row, size_t col) {
            assert(row < Rows && "Row index out of bounds");
            assert(col < Cols && "Col index out of bounds");
            // Mapeo Row-Major
            return data[row * Cols + col];
        }

        const T& operator()(size_t row, size_t col) const {
            assert(row < Rows && "Row index out of bounds");
            assert(col < Cols && "Col index out of bounds");
            return data[row * Cols + col];
        }

        // --- UTILIDADES ---
        constexpr size_t rows() const { return Rows; }
        constexpr size_t cols() const { return Cols; }
        constexpr size_t size() const { return Rows * Cols; }


        // --- OPERACIONES: TRANSPUESTA ---
        
        // Retorna una NUEVA matriz donde filas y columnas están intercambiadas.
        // El tipo de retorno invierte las dimensiones template <Cols, Rows>.
        Matrix<T, Cols, Rows> transpose() const {
            Matrix<T, Cols, Rows> result;
            for (size_t i = 0; i < Rows; ++i) {
                for (size_t j = 0; j < Cols; ++j) {
                    // Leemos (i, j) -> Escribimos en (j, i)
                    // result(j, i) accede a la fila j, columna i de la nueva matriz
                    result(j, i) = (*this)(i, j);
                }
            }
            return result;
        }


        // --- FACTORY METHODS ---
        
        // Crea una matriz identidad. Solo válido si es cuadrada.
        static Matrix<T, Rows, Cols> identity() {
            static_assert(Rows == Cols, "Identity matrix must be square");
            Matrix<T, Rows, Cols> res; // Inicia en ceros por el constructor default
            for (size_t i = 0; i < Rows; ++i) {
                res(i, i) = T(1);
            }
            return res;
        }


        // --- ARITMÉTICA: ASIGNACIÓN COMPUESTA ---

        Matrix& operator+=(const Matrix& other) {
            for (size_t i = 0; i < Rows * Cols; ++i) { // Bucle plano 1D (Más rápido)
                data[i] += other.data[i];
            }
            return *this;
        }

        Matrix& operator-=(const Matrix& other) {
            for (size_t i = 0; i < Rows * Cols; ++i) {
                data[i] -= other.data[i];
            }
            return *this;
        }
        
        // Nota: La multiplicación (*) NO es elemento a elemento en matrices.
        // Eso lo veremos el Día 10.

        
        // --- OPTIMIZACIÓN DÍA 13 ---
        
        // Multiplicación optimizada usando localidad de caché.
        // A (R1 x Common) * B (Common x C2) -> Result (R1 x C2)
        // Usamos templates distintos para A y B para ser genéricos.
        template <size_t R1, size_t Common, size_t C2>
        static Matrix<T, R1, C2> multiply_fast(const Matrix<T, R1, Common>& A, const Matrix<T, Common, C2>& B) {
            Matrix<T, R1, C2> result; // Ceros

            // PASO 1: Transponer B para lectura lineal.
            // Esto tiene un costo O(N^2), pero la multiplicación es O(N^3).
            // Para N grande, el ahorro en la multiplicación paga con creces este costo.
            Matrix<T, C2, Common> B_T = B.transpose();

            // PASO 2: Multiplicación con acceso lineal
            for (size_t i = 0; i < R1; ++i) {
                for (size_t j = 0; j < C2; ++j) {
                    
                    T sum = T(0);
                    
                    // Bucle crítico (Inner Loop)
                    // Ahora recorremos A por filas (k) y B_T por filas (k).
                    // Ambos punteros avanzan secuencialmente en memoria RAM.
                    for (size_t k = 0; k < Common; ++k) {
                        // Antes: A(i, k) * B(k, j)  <-- B saltaba memoria
                        // Ahora: A(i, k) * B_T(j, k) <-- B_T es lineal
                        sum += A(i, k) * B_T(j, k);
                    }
                    
                    result(i, j) = sum;
                }
            }
            return result;
        }



    };

    // --- VISUALIZACIÓN ---
    template <typename T, size_t Rows, size_t Cols>
    std::ostream& operator<<(std::ostream& os, const Matrix<T, Rows, Cols>& m) {
        os << "Matrix (" << Rows << "x" << Cols << "):\n";
        for (size_t i = 0; i < Rows; ++i) {
            os << "  [ ";
            for (size_t j = 0; j < Cols; ++j) {
                os << m(i, j);
                if (j < Cols - 1) os << ", ";
            }
            os << " ]\n";
        }
        return os;
    }


    // --- OPERADORES BINARIOS (Fuera de la clase) ---

    template <typename T, size_t R, size_t C>
    Matrix<T, R, C> operator+(Matrix<T, R, C> lhs, const Matrix<T, R, C>& rhs) {
        lhs += rhs;
        return lhs;
    }

    template <typename T, size_t R, size_t C>
    Matrix<T, R, C> operator-(Matrix<T, R, C> lhs, const Matrix<T, R, C>& rhs) {
        lhs -= rhs;
        return lhs;
    }


    // --- MULTIPLICACIÓN MATRICIAL (O(N^3)) ---
    
    // Matriz * Matriz
    // A(R1 x Common) * B(Common x C2) -> Result(R1 x C2)
    template <typename T, size_t R1, size_t Common, size_t C2>
    Matrix<T, R1, C2> operator*(const Matrix<T, R1, Common>& lhs, const Matrix<T, Common, C2>& rhs) {
        Matrix<T, R1, C2> result; // Inicializada a ceros por default

        for (size_t i = 0; i < R1; ++i) {           // Iterar sobre filas de A
            for (size_t j = 0; j < C2; ++j) {       // Iterar sobre columnas de B
                for (size_t k = 0; k < Common; ++k) { // Producto punto
                    // result(i, j) += A(i, k) * B(k, j)
                    result(i, j) += lhs(i, k) * rhs(k, j);
                }
            }
        }
        return result;
    }

    
    // --- MULTIPLICACIÓN MATRIZ-VECTOR ---
    
    // Matriz(Rows x Cols) * Vector(Cols) -> Vector(Rows)
    // Ejemplo: Proyección 3x3 * Vector3 -> Vector3
    template <typename T, size_t Rows, size_t Cols>
    Vector<T, Rows> operator*(const Matrix<T, Rows, Cols>& mat, const Vector<T, Cols>& vec) {
        Vector<T, Rows> result; // Inicia en ceros

        for (size_t i = 0; i < Rows; ++i) {
            // El elemento i del resultado es el dot product de la fila i con el vector
            for (size_t j = 0; j < Cols; ++j) {
                result[i] += mat(i, j) * vec[j];
            }
        }
        return result;
    }



} // namespace TinyGeo