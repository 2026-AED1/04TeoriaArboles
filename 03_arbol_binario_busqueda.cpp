// 03_arbol_binario_busqueda.cpp
// Objetivo: anadir una PROPIEDAD DE ORDEN al arbol binario del fichero 02.
// Eso es lo unico que distingue un Arbol Binario de Busqueda (BST) de un
// arbol binario cualquiera:
//
//   PROPIEDAD BST: para todo nodo, TODO lo que cuelga de su subarbol
//   izquierdo es menor, y TODO lo que cuelga del derecho es mayor.
//
// La consecuencia es enorme: buscar deja de ser O(n). En cada nodo se
// descarta UNA MITAD del arbol entero sin mirarla, igual que la busqueda
// binaria sobre un array ordenado... pero aqui no hace falta que el array
// este contiguo, porque el orden vive en la FORMA del arbol, no en su
// posicion en memoria.
//
// Y de regalo: el inorden del fichero 02 (izquierdo, visitar, derecho),
// aplicado a un BST, recorre los elementos EN ORDEN. No es casualidad: es
// la propiedad BST leida al reves.
//
// EL AVISO IMPORTANTE, que se mide al final de este fichero: el coste
// O(log n) depende de que el arbol este "equilibrado". Si se insertan los
// datos YA ORDENADOS, cada nodo nuevo cuelga siempre del mismo lado y el
// arbol degenera en una CADENA: la altura pasa de log n a n, y buscar
// vuelve a ser O(n). Arreglar esto sin cambiar el contrato es exactamente
// lo que hace el AVL del fichero 06.
//
// CAMBIA EL TAD, NO SOLO LA ESTRUCTURA. En los ficheros 01 y 02 quien usaba
// el arbol NAVEGABA por el: raiz, hijo izquierdo, hijo derecho, padre...
// Aqui ya no: quien usa un BST pide cosas POR VALOR (insertar(v),
// buscar(v), borrar(v)) y no ve ni un nodo. El TAD que se ofrece es un
// CONJUNTO (o un Diccionario, si cada clave lleva un valor asociado); el
// arbol es solo su IMPLEMENTACION. Por eso la clase de abajo esconde Nodo y
// raiz_ en la parte privada: las posiciones ya no forman parte del
// contrato. Es lo mismo que hace std::set (fichero 08).
//
// Compilar:
//   g++ -std=c++17 -O2 -Wall -o 03_bst 03_arbol_binario_busqueda.cpp && ./03_bst

#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <algorithm>
#include <limits>

// =========================================================================
// LA ESPECIFICACION
// =========================================================================
//
//  insertar(v)   pre: -             post: v esta en la coleccion (si no
//                                         estaba ya; no admitimos repetidos)
//  buscar(v)     pre: -             post: devuelve si v esta
//  borrar(v)     pre: -             post: si estaba, lo quita y devuelve
//                                         true; el resto conserva la
//                                         propiedad BST
//  inorden()     pre: -             post: devuelve todos los elementos,
//                                         EN ORDEN ASCENDENTE

class ArbolBusqueda {
public:
    ArbolBusqueda() = default;
    ~ArbolBusqueda() { liberar(raiz_); }

    // Clonar un arbol entero no es gratis y casi nunca hace falta: en vez
    // de escribir un clone() que nadie va a usar, seguimos la otra mitad
    // legitima de la regla de los tres y PROHIBIMOS la copia.
    ArbolBusqueda(const ArbolBusqueda&) = delete;
    ArbolBusqueda& operator=(const ArbolBusqueda&) = delete;

    void insertar(int valor) { raiz_ = insertarEn(raiz_, valor); }

    bool buscar(int valor) const {
        Nodo* n = raiz_;
        while (n != nullptr) {
            if (valor == n->valor) return true;
            n = (valor < n->valor) ? n->izq : n->der;
        }
        return false;
    }

    // --- Ejercicio: borrar, el caso interesante del tema ------------------
    //
    // Los TRES casos, de menos a mas dificil:
    //   1. Nodo HOJA (sin hijos)        -> se quita sin mas: devolver nullptr.
    //   2. Nodo con UN SOLO hijo        -> el nodo se "salta": devolver ese
    //                                      hijo en su lugar.
    //   3. Nodo con DOS hijos           -> no se puede quitar sin mas (que
    //                                      hueco dejaria?). La solucion
    //                                      clasica: buscar el SUCESOR
    //                                      inorden (el MINIMO del subarbol
    //                                      derecho: el mas pequenio de los
    //                                      que son mayores que el nodo),
    //                                      copiar su valor a este nodo, y
    //                                      borrar recursivamente ese sucesor
    //                                      del subarbol derecho. Ese borrado
    //                                      recursivo cae SIEMPRE en el caso 1
    //                                      o el caso 2 (el minimo de un
    //                                      subarbol nunca tiene hijo
    //                                      izquierdo), asi que no hace falta
    //                                      un cuarto caso.
    // También se puede usar el PREDECESOR inorden (el MAXIMO del subarbol izquierdo) y el resultado es simetrico.
    //
    // Sigue el MISMO patron que insertarEn(): una funcion recursiva que
    // recibe una raiz de subarbol y DEVUELVE la nueva raiz de ese subarbol
    // (que puede ser la misma, otra, o nullptr).
    bool borrar(int valor) {
        bool borrado = false;
        raiz_ = borrarEn(raiz_, valor, borrado);
        return borrado;
    }

    std::vector<int> inorden() const {
        std::vector<int> salida;
        inordenEn(raiz_, salida);
        return salida;
    }

    int altura() const { return alturaDe(raiz_); }
    int tamanio() const { return tamanioDe(raiz_); }

    // Comprobacion explicita del invariante: para cada nodo, todo el
    // subarbol izquierdo < nodo < todo el subarbol derecho. Se hace
    // arrastrando una cota (minimo, maximo) valida hacia abajo.
    bool esValido() const {
        return esValidoEn(raiz_, std::numeric_limits<int>::min(),
                                  std::numeric_limits<int>::max());
    }

private:
    struct Nodo {
        int valor;
        Nodo* izq = nullptr;
        Nodo* der = nullptr;
        explicit Nodo(int v) : valor(v) {}
    };

    Nodo* raiz_ = nullptr;

    // --- Invariante de la representacion ----------------------------------
    //  Para todo nodo n:  n->izq (y todo lo que cuelga de el) < n->valor
    //                      n->der (y todo lo que cuelga de el) > n->valor
    //  Sin repetidos.
    // ------------------------------------------------------------------------

    // insertarEn devuelve la raiz del subarbol TRAS insertar: si n era
    // nullptr, la nueva raiz es el nodo recien creado; si no, sigue siendo n.
    static Nodo* insertarEn(Nodo* n, int valor) {
        if (n == nullptr) return new Nodo(valor);
        if (valor < n->valor)      n->izq = insertarEn(n->izq, valor);
        else if (valor > n->valor) n->der = insertarEn(n->der, valor);
        // valor == n->valor: ya esta, no se hace nada (sin repetidos).
        return n;
    }

    static Nodo* borrarEn(Nodo* n, int valor, bool& borrado) {
        // TODO: implementar los tres casos descritos arriba.
        // Pistas:
        //   - Si n == nullptr, no estaba: devolver nullptr (borrado se queda false).
        //   - Si valor < n->valor, seguir buscando por la izquierda:
        //         n->izq = borrarEn(n->izq, valor, borrado); return n;
        //   - Si valor > n->valor, simetrico por la derecha.
        //   - Si valor == n->valor, AQUI hay que borrar n: mira los tres casos.
        //     Para el caso 3, una funcion auxiliar que encuentre el minimo de
        //     un subarbol (bajar siempre por la izquierda) es lo que hace falta.
        return n;
    }

    static void inordenEn(Nodo* n, std::vector<int>& salida) {
        if (n == nullptr) return;
        inordenEn(n->izq, salida);
        salida.push_back(n->valor);
        inordenEn(n->der, salida);
    }

    static int alturaDe(Nodo* n) {
        if (n == nullptr) return -1;
        return 1 + std::max(alturaDe(n->izq), alturaDe(n->der));
    }

    static int tamanioDe(Nodo* n) {
        if (n == nullptr) return 0;
        return 1 + tamanioDe(n->izq) + tamanioDe(n->der);
    }

    static bool esValidoEn(Nodo* n, int minimo, int maximo) {
        if (n == nullptr) return true;
        if (n->valor < minimo || n->valor > maximo) return false;
        return esValidoEn(n->izq, minimo, n->valor - 1) &&
               esValidoEn(n->der, n->valor + 1, maximo);
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

int main() {
    std::cout << "=== Insertar y buscar ===\n";
    ArbolBusqueda a;
    for (int v : {50, 30, 70, 20, 40, 60, 80}) a.insertar(v);
    imprimir("inorden (sale ordenado, gratis):", a.inorden());
    std::cout << "  buscar(40): " << std::boolalpha << a.buscar(40)
              << "   buscar(45): " << a.buscar(45) << '\n';
    std::cout << "  altura=" << a.altura() << "  nodos=" << a.tamanio()
              << "  es BST valido: " << a.esValido() << '\n';

    std::cout << "\n=== Borrar: tu ejercicio, los tres casos ===\n";
    std::cout << "  borrar(20) [hoja]:           ";
    a.borrar(20); imprimir("", a.inorden());
    std::cout << "  borrar(30) [un hijo]:        ";
    a.borrar(30); imprimir("", a.inorden());
    std::cout << "  borrar(50) [dos hijos, raiz]:";
    a.borrar(50); imprimir("", a.inorden());
    std::cout << "  sigue siendo BST valido: " << std::boolalpha << a.esValido() << '\n';
    std::cout << "  (si borrar() no esta implementado, el inorden no habra cambiado\n"
              << "   y 'es BST valido' puede seguir dando true sin que se haya\n"
              << "   borrado nada: comprueba tambien tamanio())\n";

    std::cout << "\n=== El aviso: orden de insercion frente a altura ===\n";
    std::cout << "  " << std::left << std::setw(10) << "n"
              << std::setw(22) << "altura (aleatorio)"
              << "altura (YA ordenado)\n";
    std::mt19937 gen(7);
    for (int n : {100, 1000, 10000, 100000}) {
        std::vector<int> datos(n);
        for (int i = 0; i < n; i++) datos[i] = i;

        ArbolBusqueda ordenado;
        for (int x : datos) ordenado.insertar(x);       // YA vienen ordenados

        std::shuffle(datos.begin(), datos.end(), gen);
        ArbolBusqueda aleatorio;
        for (int x : datos) aleatorio.insertar(x);

        std::cout << "  " << std::left << std::setw(10) << n
                  << std::setw(22) << aleatorio.altura()
                  << ordenado.altura() << '\n';
    }
    std::cout << "\n  La columna aleatoria crece PROPORCIONAL a log n (con datos al\n"
              << "  azar, la altura esperada de un BST es unas cuantas veces log2(n),\n"
              << "  nunca n). La columna ordenada crece como n EXACTO: es literalmente\n"
              << "  una lista enlazada disfrazada de arbol. Mismo codigo, mismos\n"
              << "  datos, orden de insercion distinto: la diferencia de altura pasa\n"
              << "  de un factor de 2 (n=100) a un factor de mas de 2500 (n=100000).\n";
}
