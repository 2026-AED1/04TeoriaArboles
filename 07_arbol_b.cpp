// 07_arbol_b.cpp
// Objetivo: el ultimo escalon del tema. Hasta aqui, todo arbol ha sido
// BINARIO: cada nodo, dos hijos como mucho. El B-tree (Bayer y McCreight,
// 1972) rompe esa regla a proposito: cada nodo puede tener MUCHOS hijos
// (decenas, cientos), y guarda VARIAS claves ordenadas, no solo una.
//
// POR QUE MERECE LA PENA: un AVL con un millon de claves tiene una altura
// de log2(1000000) =~ 20 niveles. Eso es estupendo si el arbol vive en
// memoria RAM. Pero si vive en DISCO (un indice de base de datos, un
// sistema de ficheros), CADA nivel que se baja es un acceso a disco, y un
// acceso a disco es ORDENES DE MAGNITUD mas lento que una comparacion en
// memoria. Lo que hay que minimizar ya no es el numero de comparaciones:
// es el numero de NODOS VISITADOS. La solucion: que cada nodo sea grande
// (cientos de claves) para que la altura se desplome. Con "grado minimo"
// t = 100, ese mismo millon de claves cabe en una altura de 2 o 3 niveles.
//
// LA ESTRUCTURA. Con un "grado minimo" t (parametro del arbol):
//   - Cada nodo (salvo la raiz) tiene ENTRE t-1 y 2t-1 claves, ordenadas.
//   - Un nodo con k claves tiene, si no es hoja, EXACTAMENTE k+1 hijos.
//   - Las claves de un nodo SEPARAN los hijos: todo lo que cuelga del
//     hijo i esta entre la clave (i-1) y la clave i (BST, generalizado).
//   - Todas las hojas estan a la MISMA profundidad (a diferencia de un
//     AVL, que solo garantiza que las alturas de los dos lados difieran
//     como mucho en 1: aqui la garantia es mas fuerte).
//
//        ejemplo con t=2 (entre 1 y 3 claves por nodo):
//
//                    [ 30 | 60 ]
//                   /     |     \
//            [10|20]   [40|50]  [70|80|90]
//
// LA INSERCION Y EL "SPLIT". Un nodo NUNCA puede superar 2t-1 claves. Si al
// insertar se pasa (llega a 2t), se PARTE en dos: la clave del medio SUBE
// al padre como nueva clave separadora, y las claves de alrededor se
// reparten en dos nodos hermanos. Es la unica operacion nueva del tema:
//
//        nodo con 2t claves:      [ c0 c1 ... c(t-1) | ct | c(t+1) ... c(2t-1) ]
//                                                       ^
//                                              esta SUBE al padre
//
//        tras partir:   izquierda: [c0..c(t-1)]      derecha: [c(t+1)..c(2t-1)]
//                        (y si no era hoja, los hijos se reparten igual:
//                         los primeros t+1 a la izquierda, el resto a la derecha)
//
// Compilar:
//   g++ -std=c++17 -O2 -Wall -o 07_btree 07_arbol_b.cpp && ./07_btree

#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <climits>

// =========================================================================
// LA ESPECIFICACION (la misma de siempre: insertar, buscar, inorden)
// =========================================================================

class ArbolB {
public:
    explicit ArbolB(int gradoMinimo) : t_(gradoMinimo) {}
    ~ArbolB() { liberar(raiz_); }
    ArbolB(const ArbolB&) = delete;
    ArbolB& operator=(const ArbolB&) = delete;

    bool buscar(int valor) const { return buscarEn(raiz_, valor); }

    void insertar(int valor) {
        if (raiz_ == nullptr) {
            raiz_ = new Nodo(true);
            raiz_->claves.push_back(valor);
            return;
        }
        int claveQueSube;
        Nodo* hermanoDerecho;
        bool sePartio = insertarEn(raiz_, valor, claveQueSube, hermanoDerecho);
        if (sePartio) {
            // La raiz se partio: hace falta una raiz NUEVA, un nivel mas.
            // Es el UNICO sitio donde el arbol crece en altura.
            Nodo* nuevaRaiz = new Nodo(false);
            nuevaRaiz->claves.push_back(claveQueSube);
            nuevaRaiz->hijos.push_back(raiz_);
            nuevaRaiz->hijos.push_back(hermanoDerecho);
            raiz_ = nuevaRaiz;
        }
    }

    std::vector<int> inorden() const {
        std::vector<int> salida;
        inordenEn(raiz_, salida);
        return salida;
    }

    // Numero de NIVELES del arbol (0 si esta vacio o tiene solo la raiz).
    // Todas las hojas estan a la misma profundidad: basta bajar por hijos[0].
    int altura() const {
        int h = 0;
        for (Nodo* n = raiz_; n != nullptr && !n->esHoja; n = n->hijos[0]) h++;
        return raiz_ == nullptr ? -1 : h;
    }

    int tamanio() const { return tamanioDe(raiz_); }

    bool esValido() const {
        if (raiz_ == nullptr) return true;
        return esValidoEn(raiz_, INT_MIN, INT_MAX, /*esRaiz=*/true);
    }

private:
    struct Nodo {
        bool esHoja;
        std::vector<int> claves;
        std::vector<Nodo*> hijos;     // vacio si esHoja
        explicit Nodo(bool hoja) : esHoja(hoja) {}
    };

    int t_;              // grado minimo: entre t-1 y 2t-1 claves por nodo (salvo raiz)
    Nodo* raiz_ = nullptr;

    // --- Invariante de la representacion -------------------------------------
    //  raiz_ == nullptr  <=>  arbol vacio.
    //  Todo nodo (salvo la raiz) tiene entre t_-1 y 2*t_-1 claves, ORDENADAS.
    //  Un nodo con k claves y que no es hoja tiene EXACTAMENTE k+1 hijos.
    //  BST generalizado: todo lo que cuelga de hijos[i] esta estrictamente
    //  entre claves[i-1] y claves[i] (con -inf y +inf en los extremos).
    //  Todas las hojas estan a la MISMA profundidad.
    // ---------------------------------------------------------------------------

    bool buscarEn(Nodo* n, int valor) const {
        if (n == nullptr) return false;
        std::size_t i = 0;
        while (i < n->claves.size() && valor > n->claves[i]) i++;
        if (i < n->claves.size() && valor == n->claves[i]) return true;
        if (n->esHoja) return false;
        return buscarEn(n->hijos[i], valor);
    }

    // Inserta 'valor' en el subarbol de n (n YA tiene sitio: se comprueba
    // ANTES de bajar a el, ver mas abajo). Si al insertar n se pasa de
    // claves, se parte: devuelve true, y por referencia la clave que sube
    // al padre y el nuevo nodo hermano (a la derecha del propio n).
    bool insertarEn(Nodo* n, int valor, int& claveQueSube, Nodo*& hermanoDerecho) {
        if (n->esHoja) {
            std::size_t pos = std::lower_bound(n->claves.begin(), n->claves.end(), valor)
                               - n->claves.begin();
            n->claves.insert(n->claves.begin() + pos, valor);
        } else {
            std::size_t i = 0;
            while (i < n->claves.size() && valor > n->claves[i]) i++;
            int claveHijoSube;
            Nodo* hermanoHijo;
            bool hijoSePartio = insertarEn(n->hijos[i], valor, claveHijoSube, hermanoHijo);
            if (hijoSePartio) {
                n->claves.insert(n->claves.begin() + i, claveHijoSube);
                n->hijos.insert(n->hijos.begin() + i + 1, hermanoHijo);
            }
        }
        return dividirSiHaceFalta(n, claveQueSube, hermanoDerecho);
    }

    // --- Ejercicio: el split ------------------------------------------------
    //
    // n puede tener, como mucho, DEMASIADAS claves de golpe: 2*t_ (una mas
    // de las 2*t_-1 permitidas), porque acaba de recibir una insercion o
    // una clave promovida de un hijo. Si es asi, hay que partirlo en dos:
    //
    //   1. La clave del medio (indice t_ en n->claves, con n->claves.size()
    //      == 2*t_) es la que SUBE: guardala en claveQueSube y quitala de
    //      n->claves.
    //   2. Crea un nodo nuevo (hermanoDerecho), del mismo tipo (hoja o no)
    //      que n, y muevele las claves que quedan a la derecha del medio.
    //   3. Si n NO es hoja, muevele tambien a hermanoDerecho los hijos que
    //      corresponden (los t_+1 ultimos de n->hijos).
    //   4. n se queda solo con sus primeras t_ claves (indices 0..t_-1) y,
    //      si no es hoja, sus primeros t_+1 hijos.
    //   5. Devuelve true.
    //
    // Si n no llego a tener 2*t_ claves, no hay nada que hacer: devuelve
    // false (y claveQueSube/hermanoDerecho no se usan).
    bool dividirSiHaceFalta(Nodo* n, int& claveQueSube, Nodo*& hermanoDerecho) {
        if (static_cast<int>(n->claves.size()) < 2 * t_) return false;
        // TODO: implementar los cinco pasos de arriba.
        return false;
    }

    static void inordenEn(Nodo* n, std::vector<int>& salida) {
        if (n == nullptr) return;
        for (std::size_t i = 0; i < n->claves.size(); i++) {
            if (!n->esHoja) inordenEn(n->hijos[i], salida);
            salida.push_back(n->claves[i]);
        }
        if (!n->esHoja) inordenEn(n->hijos.back(), salida);
    }

    static int tamanioDe(Nodo* n) {
        if (n == nullptr) return 0;
        int total = static_cast<int>(n->claves.size());
        if (!n->esHoja) for (Nodo* h : n->hijos) total += tamanioDe(h);
        return total;
    }

    bool esValidoEn(Nodo* n, int minimo, int maximo, bool esRaiz) const {
        int k = static_cast<int>(n->claves.size());
        if (!esRaiz && (k < t_ - 1 || k > 2 * t_ - 1)) return false;
        if (esRaiz && k > 2 * t_ - 1) return false;
        for (int i = 0; i < k; i++) {
            if (n->claves[i] < minimo || n->claves[i] > maximo) return false;
            if (i > 0 && n->claves[i] <= n->claves[i - 1]) return false;
        }
        if (!n->esHoja) {
            if (static_cast<int>(n->hijos.size()) != k + 1) return false;
            for (int i = 0; i <= k; i++) {
                int lo = (i == 0) ? minimo : n->claves[i - 1] + 1;
                int hi = (i == k) ? maximo : n->claves[i] - 1;
                if (!esValidoEn(n->hijos[i], lo, hi, false)) return false;
            }
        }
        return true;
    }

    static void liberar(Nodo* n) {
        if (n == nullptr) return;
        if (!n->esHoja) for (Nodo* h : n->hijos) liberar(h);
        delete n;
    }
};

void imprimir(const char* etiqueta, const std::vector<int>& v) {
    std::cout << "  " << etiqueta;
    for (int x : v) std::cout << x << ' ';
    std::cout << '\n';
}

int main() {
    std::cout << "=== Insertar y buscar, con grado minimo t=2 (1 a 3 claves por nodo) ===\n";
    ArbolB b(2);
    for (int v : {10, 20, 30, 40, 50, 60, 70, 80, 90}) b.insertar(v);
    imprimir("inorden (sale ordenado, gratis):", b.inorden());
    std::cout << "  buscar(50): " << std::boolalpha << b.buscar(50)
              << "   buscar(55): " << b.buscar(55) << '\n';
    std::cout << "  niveles=" << b.altura() << "  claves totales=" << b.tamanio()
              << "  valido=" << b.esValido() << '\n';
    std::cout << "  (si 'valido' da false o el inorden no sale completo, es que\n"
              << "   dividirSiHaceFalta() todavia esta sin completar: en cuanto un\n"
              << "   nodo llega a 2*t claves, hay que partirlo)\n";

    std::cout << "\n=== Un ejemplo algo mayor, para ver crecer los niveles ===\n";
    {
        ArbolB medio(2);
        for (int i = 0; i < 40; i++) medio.insertar(i);
        std::cout << "  40 claves con t=2: niveles=" << medio.altura()
                  << "  claves totales=" << medio.tamanio()
                  << "  valido=" << std::boolalpha << medio.esValido() << '\n';
        std::cout << "  (con el split funcionando, 40 claves y t=2 deberian caber en\n"
                  << "   3 o 4 niveles como mucho, nunca en una cadena de 40)\n";
    }
    std::cout << "\n  La comparacion de verdad -por que un grado grande compensa frente\n"
              << "  a un AVL cuando hay millones de claves y cada nivel es un acceso\n"
              << "  a disco- esta medida con el ejercicio ya resuelto en\n"
              << "  2_profesor/07_arbol_b_resuelta.cpp.\n";

    std::cout << "\n=== Y ninguno de los TADs de este tema sustituye a los demas ===\n";
    std::cout << "  arbol general      -> jerarquias (sistemas de ficheros, DOM)\n"
              << "  arbol binario      -> la base de todo lo demas\n"
              << "  BST                -> busqueda ordenada EN MEMORIA, caso tipico\n"
              << "  monticulo          -> SOLO el extremo (colas de prioridad)\n"
              << "  AVL                -> busqueda ordenada con garantia dura de O(log n)\n"
              << "  B-tree             -> busqueda ordenada cuando el acceso es caro (disco)\n";
}
