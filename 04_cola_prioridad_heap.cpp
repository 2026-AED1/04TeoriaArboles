// 04_cola_prioridad_heap.cpp
// Objetivo: el TAD Cola de Prioridad y su implementacion buena, el MONTICULO
// BINARIO (binary heap).
//
// Cambio de idea respecto a todo lo anterior. En la Pila y en la Cola, el
// orden de salida lo decide el orden de ENTRADA. En la Cola de Prioridad lo
// decide el VALOR: sale siempre el mas prioritario, haya llegado cuando haya
// llegado. Es el TAD del "urgencias de un hospital", no el del "turno de la
// carniceria".
//
// EL COMPROMISO. Las implementaciones evidentes son malas:
//
//    lista sin ordenar : insertar O(1)      , extraer maximo O(n)
//    lista ordenada    : insertar O(n)      , extraer maximo O(1)
//    MONTICULO         : insertar O(log n)  , extraer maximo O(log n)
//
// El monticulo no gana en ninguna de las dos columnas por separado: gana
// cuando las dos operaciones se usan mezcladas, que es siempre. Es el
// ejemplo perfecto de que "el mejor coste" no significa nada sin decir para
// que mezcla de operaciones.
//
// LA ESTRUCTURA. Un monticulo es un arbol binario con dos propiedades:
//   1. COMPLETO: todos los niveles llenos menos el ultimo, que se llena de
//      izquierda a derecha. Eso garantiza altura exactamente floor(log2 n).
//   2. PROPIEDAD DE MONTICULO: todo padre es >= que sus hijos (monticulo de
//      maximos). Ojo: NO es un arbol de busqueda. Entre hermanos no hay
//      ningun orden, y por eso buscar un valor cualquiera sigue siendo O(n).
//
// Y AQUI ESTA LO BONITO: al ser completo, el arbol no necesita punteros.
// Cabe en un array, con aritmetica de indices:
//
//        padre(i)      = (i - 1) / 2
//        izquierdo(i)  = 2*i + 1
//        derecho(i)    = 2*i + 2
//
//              [50]                 array: 50 30 40 10 20 35
//             /    \                index:  0  1  2  3  4  5
//          [30]    [40]
//          /  \      \
//       [10] [20]   [35]
//
// Cero memoria desperdiciada en enlaces, y recorrido con localidad de cache.
//
// Compilar:
//   g++ -std=c++17 -Wall -o 04_heap 04_cola_prioridad_heap.cpp && ./04_heap

#include <iostream>
#include <iomanip>
#include <vector>
#include <stdexcept>
#include <algorithm>

// =========================================================================
// LA ESPECIFICACION
// =========================================================================
//
//  insertar(v)      pre: -            post: v esta en la coleccion
//  maximo()         pre: !vacia()     post: devuelve el mayor; no modifica
//  extraerMaximo()  pre: -            post: si no estaba vacia, elimina UNA
//                                           aparicion del mayor -> true
//  tamanio()        pre: -            post: numero de elementos
//
// Que pasa con los EMPATES: el TAD no dice nada. Si dos elementos tienen la
// misma prioridad, el orden entre ellos es arbitrario (un monticulo NO es
// estable). Si el desempate importa, se mete en la propia prioridad, por
// ejemplo usando el par (prioridad, numero_de_llegada).

class ColaPrioridadMax {
public:
    ColaPrioridadMax() : comparaciones_(0), intercambios_(0) {}

    // --- insertar: colocar al final y SUBIR (sift-up) --------------------
    // El sitio unico donde puede ir un elemento nuevo sin romper la forma
    // completa es el final del array. Desde ahi sube mientras sea mayor que
    // su padre. Recorre como mucho la altura: O(log n).
    void insertar(int valor) {
        datos_.push_back(valor);
        subir(static_cast<int>(datos_.size()) - 1);
    }

    int maximo() const {                          // O(1): esta en la raiz
        if (datos_.empty()) throw std::out_of_range("maximo() sobre cola vacia");
        return datos_[0];
    }

    // --- extraer: traer el ultimo a la raiz y BAJAR (sift-down) ----------
    // No se puede "borrar la raiz" sin mas: quedaria un hueco. Se tapa con
    // el ULTIMO elemento (el unico que se puede quitar sin romper la forma)
    // y se le deja caer hasta su sitio. O(log n).
    bool extraerMaximo() {
        if (datos_.empty()) return false;
        datos_[0] = datos_.back();
        datos_.pop_back();
        if (!datos_.empty()) bajar(0);
        return true;
    }

    int  tamanio() const { return static_cast<int>(datos_.size()); }
    bool vacia() const { return datos_.empty(); }

    long comparaciones() const { return comparaciones_; }
    long intercambios() const { return intercambios_; }
    void reiniciarContadores() { comparaciones_ = 0; intercambios_ = 0; }

    // Comprobacion del invariante: todo padre >= sus hijos.
    bool esMonticulo() const {
        for (std::size_t i = 1; i < datos_.size(); i++)
            if (datos_[(i - 1) / 2] < datos_[i]) return false;
        return true;
    }

    void imprimirArray(const char* etiqueta) const {
        std::cout << "  " << std::left << std::setw(22) << etiqueta << "[ ";
        for (int x : datos_) std::cout << x << ' ';
        std::cout << "]\n";
    }

    // Dibuja el arbol por niveles, para ver que el array ES un arbol.
    void imprimirNiveles() const {
        int nivel = 0, desde = 0;
        while (desde < tamanio()) {
            int cuantos = 1 << nivel;
            std::cout << "    nivel " << nivel << ": ";
            for (int i = desde; i < desde + cuantos && i < tamanio(); i++)
                std::cout << datos_[i] << ' ';
            std::cout << '\n';
            desde += cuantos;
            nivel++;
        }
    }

private:
    std::vector<int> datos_;
    mutable long comparaciones_;
    long intercambios_;

    // --- Invariante de la representacion ---------------------------------
    //  El array esta SIEMPRE lleno de 0 a n-1 (forma completa, gratis).
    //  Para todo i en [1, n):   datos_[(i-1)/2] >= datos_[i]
    //  Consecuencia: datos_[0] es el maximo. No se dice nada mas: el resto
    //  del array puede estar en cualquier orden compatible con eso.
    // ---------------------------------------------------------------------

    static int padre(int i)     { return (i - 1) / 2; }
    static int izquierdo(int i) { return 2 * i + 1; }
    static int derecho(int i)   { return 2 * i + 2; }

    void subir(int i) {
        while (i > 0) {
            comparaciones_++;
            if (datos_[padre(i)] >= datos_[i]) break;   // ya esta en su sitio
            std::swap(datos_[padre(i)], datos_[i]);
            intercambios_++;
            i = padre(i);
        }
    }

    void bajar(int i) {
        int n = tamanio();
        while (true) {
            int mayor = i;
            int iz = izquierdo(i), de = derecho(i);
            // Hay que comparar con LOS DOS hijos y bajar por el mayor. Si se
            // baja por un hijo cualquiera, el nuevo padre puede quedar menor
            // que su otro hijo y el invariante se rompe.
            if (iz < n) { comparaciones_++; if (datos_[iz] > datos_[mayor]) mayor = iz; }
            if (de < n) { comparaciones_++; if (datos_[de] > datos_[mayor]) mayor = de; }
            if (mayor == i) break;
            std::swap(datos_[i], datos_[mayor]);
            intercambios_++;
            i = mayor;
        }
    }
};

// --- Implementacion alternativa, para comparar ---------------------------
// Lista ordenada: insertar busca el sitio y desplaza -> O(n).
class ColaPrioridadOrdenada {
public:
    ColaPrioridadOrdenada() : operaciones_(0) {}
    void insertar(int valor) {                    // O(n)
        std::size_t i = datos_.size();
        datos_.push_back(valor);
        while (i > 0 && datos_[i - 1] < valor) { datos_[i] = datos_[i - 1]; i--; operaciones_++; }
        datos_[i] = valor;                        // orden decreciente
    }
    int  maximo() const { return datos_.front(); }        // O(1)
    bool extraerMaximo() {                                // O(1) por el frente
        if (datos_.empty()) return false;
        datos_.erase(datos_.begin());
        operaciones_ += static_cast<long>(datos_.size());  // el erase desplaza
        return true;
    }
    bool vacia() const { return datos_.empty(); }
    long operaciones() const { return operaciones_; }
private:
    std::vector<int> datos_;
    long operaciones_;
};

int main() {
    std::cout << "=== El array ES un arbol ===\n";
    ColaPrioridadMax h;
    for (int v : {10, 20, 35, 30, 40, 50}) h.insertar(v);
    h.imprimirArray("array:");
    h.imprimirNiveles();
    std::cout << "    es monticulo valido: " << std::boolalpha << h.esMonticulo() << '\n';
    std::cout << "    (30 esta antes que 35 en el array y no pasa nada:\n"
              << "     entre elementos de ramas distintas NO hay ningun orden)\n";

    std::cout << "\n=== Insertar 50 en un monticulo: el sift-up ===\n";
    ColaPrioridadMax t;
    for (int v : {40, 30, 35, 10, 20}) t.insertar(v);
    t.imprimirArray("antes:");
    t.insertar(50);
    t.imprimirArray("tras insertar 50:");
    std::cout << "    50 entro al final y subio hasta la raiz: 2 intercambios,\n"
              << "    no 5 desplazamientos. Ahi esta el log n.\n";

    std::cout << "\n=== Extraccion repetida: sale ordenado ===\n";
    std::cout << "  ";
    while (!h.vacia()) { std::cout << h.maximo() << ' '; h.extraerMaximo(); }
    std::cout << "\n  (extraer n veces ordena el vector: eso es el heapsort,\n"
              << "   y lo vemos en el fichero 05)\n";

    std::cout << "\n=== Monticulo frente a lista ordenada ===\n";
    std::cout << "  " << std::left << std::setw(10) << "n"
              << std::setw(18) << "heap (comp+int)"
              << std::setw(18) << "ordenada (ops)" << "razon\n";
    for (int n : {1000, 2000, 4000, 8000}) {
        ColaPrioridadMax a;
        ColaPrioridadOrdenada b;
        // Peor caso para la lista ordenada: insertar en orden creciente
        // obliga a desplazar todo cada vez.
        for (int i = 0; i < n; i++) { a.insertar(i); b.insertar(i); }
        long opsHeap = a.comparaciones() + a.intercambios();
        std::cout << "  " << std::left << std::setw(10) << n
                  << std::setw(18) << opsHeap
                  << std::setw(18) << b.operaciones()
                  << std::fixed << std::setprecision(1)
                  << double(b.operaciones()) / double(opsHeap) << "x\n";
    }
    std::cout << "\n  Al doblar n, el heap poco mas que dobla (n log n) y la lista\n"
              << "  ordenada se multiplica por cuatro (n^2). La razon crece sin\n"
              << "  limite: no es una constante, es un orden de complejidad distinto.\n";

    std::cout << "\n=== Lo que un monticulo NO sabe hacer ===\n";
    std::cout << "  buscar un valor cualquiera -> O(n): no es un arbol de busqueda\n"
              << "  recorrer en orden           -> hay que vaciarlo, O(n log n)\n"
              << "  cambiar la prioridad de un elemento concreto -> hace falta\n"
              << "     saber donde esta; se resuelve guardando aparte un indice\n"
              << "     posicion->elemento. Es lo que necesita Dijkstra de verdad.\n";
}
