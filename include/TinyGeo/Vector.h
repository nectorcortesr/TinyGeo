#pragma once // Evita que el archivo se incluya múltiples veces

#include <array>
#include <iostream>
#include <initializer_list>
#include <cassert>
#include <cmath>

namespace TinyGeo {

    // T: Tipo de dato (float, double, int)
    // N: Dimensión (2, 3, 4...)
    template <typename T, size_t N>
    class Vector {
    public:
        // --- 0. ACCESORES NOMBRADOS (Quality of Life) ---
        // Solo tienen sentido si N es suficiente, pero por simplicidad
        // los dejaremos disponibles. Usamos 'if constexpr' o asserts en producción,
        // aquí confiaremos en el uso correcto o check de rango del array.
        
        T& x() { static_assert(N >= 1); return data[0]; }
        const T& x() const { static_assert(N >= 1); return data[0]; }

        T& y() { static_assert(N >= 2); return data[1]; }
        const T& y() const { static_assert(N >= 2); return data[1]; }

        T& z() { static_assert(N >= 3); return data[2]; }
        const T& z() const { static_assert(N >= 3); return data[2]; }
        
        // (Opcional) Para coordenadas homogéneas w
        T& w() { static_assert(N >= 4); return data[3]; }
        const T& w() const { static_assert(N >= 4); return data[3]; }

        // --- 1. ALMACENAMIENTO ---
        // Usamos std::array subyacente. Es stack-allocated y seguro.
        // Lo hacemos público (o accesible) para facilitar operaciones futuras,
        // pero por ahora lo envolvemos.
        std::array<T, N> data;

        // --- 2. CONSTRUCTORES ---
        
        // Constructor por defecto: Vector<float, 3> v; (Inicia en 0 o basura según el compilador/versión)
        // Forzamos inicialización a cero para seguridad.
        Vector() : data{} {}

        // Constructor con lista: Vector<float, 3> v = {1.0, 2.0, 3.0};
        Vector(std::initializer_list<T> list) {
            assert(list.size() == N && "Initializer list size mismatch");
            size_t i = 0;
            for (const auto& element : list) {
                data[i++] = element;
            }
        }

        // --- 3. ARITMÉTICA: ASIGNACIÓN COMPUESTA (Modifican *this) ---

        // Suma: v += other
        Vector& operator+=(const Vector& other) {
            for (size_t i = 0; i < N; ++i) {
                data[i] += other[i];
            }
            return *this; // Retornamos referencia para encadenar (a += b += c)
        }

        // Resta: v -= other
        Vector& operator-=(const Vector& other) {
            for (size_t i = 0; i < N; ++i) {
                data[i] -= other[i];
            }
            return *this;
        }

        // Multiplicación por Escalar: v *= scalar
        // Nota: No multiplicamos vectores entre sí (eso es producto punto/cruz)
        Vector& operator*=(T scalar) {
            for (size_t i = 0; i < N; ++i) {
                data[i] *= scalar;
            }
            return *this;
        }

        // División por Escalar: v /= scalar
        Vector& operator/=(T scalar) {
            // Check senior: Evitar división por cero es responsabilidad del usuario
            // por ahora en matemáticas de alto rendimiento, pero podríamos poner un assert.
            assert(scalar != 0 && "Division by zero");
            T inv_scalar = T(1) / scalar; // Optimización: 1 división, N multiplicaciones
            for (size_t i = 0; i < N; ++i) {
                data[i] *= inv_scalar;
            }
            return *this;
        }


        // --- 4. ACCESSORS (Lectura/Escritura) ---

        // Versión NO-CONST: Permite escribir (v[0] = 5.0)
        T& operator[](size_t index) {
            // En modo Debug (-g), .at() chequea límites. En Release, operator[] puro es más rápido.
            // Aquí usamos simple array access por performance, asumiendo responsabilidad.
            assert(index < N && "Index out of bounds");
            return data[index];
        }

        // Versión CONST: Solo lectura (x = v[0])
        // Esto se llama automáticamente cuando el objeto es const.
        const T& operator[](size_t index) const {
            assert(index < N && "Index out of bounds");
            return data[index];
        }

        // --- 5. UTILIDADES ---
        
        // Tamaño conocido en tiempo de compilación
        constexpr size_t size() const { return N; }


        // --- 6. GEOMETRÍA: PRODUCTO PUNTO Y NORMA ---

        // Producto Punto: Mide alineación.
        // Retorna T (escalar).
        T dot(const Vector& other) const {
            T sum = T(0);
            for (size_t i = 0; i < N; ++i) {
                sum += data[i] * other[i];
            }
            return sum;
        }

        // Producto Cruz: Solo definido para N=3 en este contexto.
        // Retorna un vector perpendicular al plano definido por *this y other.
        Vector<T, N> cross(const Vector<T, N>& other) const {
            static_assert(N == 3, "Cross product is only defined for 3D vectors (N=3)");

            // Fórmula expandida usando nuestros nuevos accesores
            return {
                y() * other.z() - z() * other.y(),
                z() * other.x() - x() * other.z(),
                x() * other.y() - y() * other.x()
            };
        }

        // Norma al Cuadrado (Magnitud^2)
        // Úsala siempre para comparaciones de distancia para evitar sqrt().
        T normSq() const {
            return dot(*this); // v . v = |v|^2
        }

        // Norma Euclidiana (Magnitud)
        // Incluimos <cmath> arriba si no está.
        T norm() const {
            return std::sqrt(normSq());
        }

        // Normalización: Retorna un NUEVO vector unitario (dirección pura).
        // No modifica el actual.
        Vector normalized() const {
            T len = norm();
            // Manejo de vector cero para evitar NaNs (Not a Number)
            // Usamos una tolerancia pequeña (epsilon)
            if (len < T(1e-8)) {
                return Vector(); // Retorna vector cero si la longitud es casi nula
            }
            return (*this) / len; // Usa nuestro operador / escalar
        }
        
        // Mutador: Normaliza el vector actual
        void normalize() {
            *this = normalized();
        }

    };

    // --- 7. ARITMÉTICA: OPERADORES BINARIOS (Crean nuevo Vector) ---

    // Suma: v3 = v1 + v2
    // Pasamos 'lhs' por valor (copia implícita) para reutilizarla
    template <typename T, size_t N>
    Vector<T, N> operator+(Vector<T, N> lhs, const Vector<T, N>& rhs) {
        lhs += rhs; // Reutilizamos el operador miembro
        return lhs;
    }

    // Resta: v3 = v1 - v2
    template <typename T, size_t N>
    Vector<T, N> operator-(Vector<T, N> lhs, const Vector<T, N>& rhs) {
        lhs -= rhs;
        return lhs;
    }

    // Multiplicación Escalar (Derecha): v2 = v1 * 2.0
    template <typename T, size_t N>
    Vector<T, N> operator*(Vector<T, N> lhs, T scalar) {
        lhs *= scalar;
        return lhs;
    }

    // Multiplicación Escalar (Izquierda): v2 = 2.0 * v1
    // Esta es la razón por la que estos operadores están fuera de la clase.
    // Si fuera miembro, 'double' no tiene un método .operator*(Vector).
    template <typename T, size_t N>
    Vector<T, N> operator*(T scalar, Vector<T, N> rhs) {
        rhs *= scalar;
        return rhs;
    }

    // División Escalar: v2 = v1 / 2.0
    template <typename T, size_t N>
    Vector<T, N> operator/(Vector<T, N> lhs, T scalar) {
        lhs /= scalar;
        return lhs;
    }

    // --- 8. VISUALIZACIÓN (Operator Overloading) ---
    // Permite hacer: std::cout << v << std::endl;
    template <typename T, size_t N>
    std::ostream& operator<<(std::ostream& os, const Vector<T, N>& v) {
        os << "[";
        for (size_t i = 0; i < N; ++i) {
            os << v[i];
            if (i < N - 1) os << ", ";
        }
        os << "]";
        return os;
    }

    // Función libre para producto punto
    template <typename T, size_t N>
    T dot(const Vector<T, N>& a, const Vector<T, N>& b) {
        return a.dot(b);
    }

    template <typename T, size_t N>
    Vector<T, N> cross(const Vector<T, N>& a, const Vector<T, N>& b) {
        return a.cross(b);
    }

} // namespace TinyGeo