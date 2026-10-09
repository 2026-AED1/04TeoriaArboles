// 06_arbol_avl.cpp
// Objetivo: arreglar el problema que el fichero 03 dejo pendiente. Un BST
// simple garantiza O(log n) SOLO si esta "equilibrado", y el orden de
// insercion puede romper eso sin avisar (insertar ya ordenado degenera en
// una cadena de altura n).
//
// LA IDEA DEL AVL (Adelson-Velskii y Landis, 1962): tras CADA insercion,
// comprobar si algun antepasado del nodo nuevo se ha desequilibrado, y si
// es asi, arreglarlo en el momento con una ROTACION. El invariante que se
// mantiene siempre:
//
//   FACTOR DE EQUILIBRIO: para todo nodo, |altura(der) - altura(izq)| <= 1.
//
// Esto es DISTINTO del monticulo del fichero 04: alli la forma se
// garantizaba por CONSTRUCCION (arbol siempre completo) y se pagaba
// renunciando a la busqueda por valor. Aqui se mantiene el orden total del
// BST y se paga con un poco de trabajo extra en cada insercion: unas pocas
// rotaciones, nunca mas de O(log n) de ellas.
//
// LOS CUATRO CASOS DE DESEQUILIBRIO, y por que solo hacen falta dos
// rotaciones simples (no cuatro cosas distintas):
//
//   LL  (pesa la izquierda, y el problema esta a la izquierda de esa
//        izquierda)         -> UNA rotacion simple a la derecha.
//   RR  (simetrico)         -> UNA rotacion simple a la izquierda.
//   LR  (pesa la izquierda, pero el problema esta a la DERECHA de esa
//        izquierda)         -> DOS rotaciones: primero el hijo izquierdo
//                               gira a la izquierda (eso lo convierte en
//                               un caso LL), y LUEGO se aplica LL.
//   RL  (simetrico)         -> primero el hijo derecho gira a la derecha
//                               (lo convierte en RR), y luego se aplica RR.
//
//        LL:                          tras rotar a la derecha:
//            z                              y
//           /                              / \
//          y              -->             x   z
//         /
//        x
//
// Compilar:
//   g++ -std=c++17 -O2 -Wall -o 06_avl 06_arbol_avl.cpp && ./06_avl

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <climits>
#include <algorithm>

// =========================================================================
// LA ESPECIFICACION (la misma que el BST del fichero 03: insertar, buscar,
// inorden. La diferencia entera esta en COMO se mantiene el invariante.)
// =========================================================================

class ArbolAVL {
public:
    ~ArbolAVL() { liberar(raiz_); }
    ArbolAVL() = default;
    ArbolAVL(const ArbolAVL&) = delete;
    ArbolAVL& operator=(const ArbolAVL&) = delete;

    void insertar(int valor) { raiz_ = insertarEn(raiz_, valor); }

    bool buscar(int valor) const {
        Nodo* n = raiz_;
        while (n != nullptr) {
            if (valor == n->valor) return true;
            n = (valor < n->valor) ? n->izq : n->der;
        }
        return false;
    }

    std::vector<int> inorden() const {
        std::vector<int> salida;
        inordenEn(raiz_, salida);
        return salida;
    }

    int altura() const { return alturaDe(raiz_); }

    // Comprueba el invariante AVL completo: que siga siendo BST (bounds) Y
    // que este equilibrado (|der - izq| <= 1) en TODOS los nodos.
    bool esValido() const {
        return esBSTValido(raiz_, INT_MIN, INT_MAX) && esEquilibrado(raiz_);
    }

private:
    struct Nodo {
        int valor;
        int altura = 0;             // altura de ESTE nodo (hoja = 0)
        Nodo* izq = nullptr;
        Nodo* der = nullptr;
        explicit Nodo(int v) : valor(v) {}
    };

    Nodo* raiz_ = nullptr;

    // --- Invariante de la representacion -----------------------------------
    //  Es un BST (fichero 03) Y ADEMAS, para todo nodo n:
    //      |alturaDe(n->der) - alturaDe(n->izq)| <= 1
    //  n->altura se mantiene coherente con sus hijos en todo momento.
    // -------------------------------------------------------------------------

    static int alturaDe(Nodo* n) { return (n == nullptr) ? -1 : n->altura; }

    static void actualizarAltura(Nodo* n) {
        n->altura = 1 + std::max(alturaDe(n->izq), alturaDe(n->der));
    }

    // Positivo: pesa el subarbol DERECHO. Negativo: pesa el IZQUIERDO.
    static int factorEquilibrio(Nodo* n) { return alturaDe(n->der) - alturaDe(n->izq); }

    // --- Las dos rotaciones simples: completas -----------------------------
    //
    // rotarDerecha arregla un desequilibrio a la izquierda subiendo al hijo
    // izquierdo. Su hijo derecho (lo que "sobraba" a la derecha de ese
    // hijo) pasa a colgar de la izquierda de n: sigue siendo BST porque ese
    // subarbol tenia valores entre el hijo y n.
    static Nodo* rotarDerecha(Nodo* n) {
        Nodo* nuevaRaiz = n->izq;
        n->izq = nuevaRaiz->der;
        nuevaRaiz->der = n;
        actualizarAltura(n);          // OJO: n primero (es el mas hondo ahora)
        actualizarAltura(nuevaRaiz);
        return nuevaRaiz;
    }

    static Nodo* rotarIzquierda(Nodo* n) {
        Nodo* nuevaRaiz = n->der;
        n->der = nuevaRaiz->izq;
        nuevaRaiz->izq = n;
        actualizarAltura(n);
        actualizarAltura(nuevaRaiz);
        return nuevaRaiz;
    }

    // Tras insertar por la izquierda o la derecha y actualizar la altura,
    // reequilibra ESTE nodo si hace falta. Se llama en cada nivel al volver
    // de la recursion, asi que arregla siempre el desequilibrio MAS BAJO
    // (el mas cercano al nodo insertado) primero.
    static Nodo* reequilibrar(Nodo* n) {
        int fe = factorEquilibrio(n);

        if (fe < -1) {                                  // pesa la izquierda
            if (factorEquilibrio(n->izq) <= 0) {
                return rotarDerecha(n);                  // caso LL: completo
            } else {
                // --- Ejercicio: caso LR ---------------------------------
                // El hijo izquierdo pesa hacia la DERECHA: una sola rotacion
                // a la derecha de n NO basta (compruebalo a mano con el
                // ejemplo del main: insertar 30, 10, 20). Hace falta
                // convertir esto en un caso LL primero.
                //
                // TODO: dos lineas, simetricas al caso RL de abajo:
                //   1. n->izq = rotarIzquierda(n->izq);
                //   2. return rotarDerecha(n);
                return n;   // <-- de momento no reequilibra nada: a completar
            }
        }

        if (fe > 1) {                                    // pesa la derecha
            if (factorEquilibrio(n->der) >= 0) {
                return rotarIzquierda(n);                 // caso RR: completo
            } else {
                // caso RL: completo, para que sirva de plantilla del LR.
                n->der = rotarDerecha(n->der);
                return rotarIzquierda(n);
            }
        }

        return n;   // ya estaba equilibrado
    }

    static Nodo* insertarEn(Nodo* n, int valor) {
        if (n == nullptr) return new Nodo(valor);
        if (valor < n->valor)      n->izq = insertarEn(n->izq, valor);
        else if (valor > n->valor) n->der = insertarEn(n->der, valor);
        else return n;                                    // sin repetidos

        actualizarAltura(n);
        return reequilibrar(n);
    }

    static void inordenEn(Nodo* n, std::vector<int>& salida) {
        if (n == nullptr) return;
        inordenEn(n->izq, salida);
        salida.push_back(n->valor);
        inordenEn(n->der, salida);
    }

    static bool esBSTValido(Nodo* n, int minimo, int maximo) {
        if (n == nullptr) return true;
        if (n->valor < minimo || n->valor > maximo) return false;
        return esBSTValido(n->izq, minimo, n->valor - 1) &&
               esBSTValido(n->der, n->valor + 1, maximo);
    }

    static bool esEquilibrado(Nodo* n) {
        if (n == nullptr) return true;
        if (std::abs(factorEquilibrio(n)) > 1) return false;
        return esEquilibrado(n->izq) && esEquilibrado(n->der);
    }

    static void liberar(Nodo* n) {
        if (n == nullptr) return;
        liberar(n->izq);
        liberar(n->der);
        delete n;
    }
};

void imprimir(const char* etiqueta, const std::vector<int>& v) {
    std::cout << "  " << etiqueta;
    for (int x : v) std::cout << x << ' ';
    std::cout << '\n';
}

void probarCaso(const char* nombre, std::initializer_list<int> secuencia) {
    ArbolAVL a;
    for (int v : secuencia) a.insertar(v);
    std::cout << "  " << std::left << std::setw(6) << nombre << "insertar ";
    for (int v : secuencia) std::cout << v << ' ';
    std::cout << " -> altura=" << a.altura()
              << "  valido=" << std::boolalpha << a.esValido() << "   inorden: ";
    for (int x : a.inorden()) std::cout << x << ' ';
    std::cout << '\n';
}

int main() {
    std::cout << "=== Los cuatro casos de desequilibrio ===\n";
    probarCaso("LL:", {30, 20, 10});     // pesa izq, problema a la izq del izq
    probarCaso("RR:", {10, 20, 30});     // simetrico
    probarCaso("RL:", {10, 30, 20});     // pesa der, problema a la izq del der
    probarCaso("LR:", {30, 10, 20});     // pesa izq, problema a la der del izq (TU EJERCICIO)
    std::cout << "\n  Si 'LR' sale con valido=false, es que reequilibrar() todavia\n"
              << "  no tiene el caso LR completado: el arbol se queda desequilibrado\n"
              << "  en vez de rotar dos veces.\n";

    std::cout << "\n=== El aviso del fichero 03, arreglado ===\n";
    std::cout << "  " << std::left << std::setw(10) << "n"
              << std::setw(14) << "altura AVL"
              << "cota teorica 1.44*log2(n)\n";
    for (int n : {100, 1000, 10000, 100000}) {
        ArbolAVL a;
        for (int i = 0; i < n; i++) a.insertar(i);     // EL PEOR CASO del BST simple
        double cota = 1.44 * std::log2(n + 2);
        std::cout << "  " << std::left << std::setw(10) << n
                  << std::setw(14) << a.altura()
                  << std::fixed << std::setprecision(1) << cota
                  << "   valido=" << std::boolalpha << a.esValido() << '\n';
    }
    std::cout << "\n  Insertando EN ORDEN -el peor caso posible del fichero 03, donde\n"
              << "  la altura se disparaba a n- el AVL se queda siempre por debajo\n"
              << "  de 1.44*log2(n): es la cota que demostraron Adelson-Velskii y\n"
              << "  Landis, y nunca se supera, insertes en el orden que insertes.\n";
}
