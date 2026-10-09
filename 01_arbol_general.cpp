// 01_arbol_general.cpp
// Objetivo: el TAD Arbol general (n-ario): cada nodo puede tener CUALQUIER
// numero de hijos. Es la generalizacion de la Lista de la clase 2: en vez de
// "el siguiente", cada nodo tiene "los hijos".
//
// Igual que en la clase 2 (01_tad_interfaz.cpp), primero el CONTRATO (que
// operaciones hay y que significan) y despues DOS IMPLEMENTACIONES:
//
//   A) NODO CON LISTA DE HIJOS. La evidente: cada nodo guarda un vector con
//      punteros a sus hijos. Facil de leer, cara en memoria: cada nodo carga
//      con un vector propio (con SU capacidad, SU reserva, SU posible
//      redimensionamiento: lo que costaba el array dinamico de la clase 2,
//      multiplicado por cada nodo del arbol).
//
//   B) PRIMER-HIJO / HERMANO-DERECHO. La misma informacion con SOLO DOS
//      punteros por nodo, sea cual sea el numero de hijos:
//
//          primerHijo_       -> el primero de mis hijos
//          hermanoDerecho_   -> el siguiente hijo de MI padre
//
//      Y aqui esta lo que hay que llevarse de este fichero: un nodo con dos
//      punteros con nombres distintos... es un ARBOL BINARIO. Cualquier
//      arbol n-ario se puede representar como un arbol binario sin perder
//      nada. Es el puente hacia el fichero 02.
//
//               A                    A
//             / | \                  \
//            B  C  D        <->       B
//           /|      \                / \
//          E F       G              C   E
//                                    \    \
//                                     D    F
//                                      \
//                                       G
//     (arbol general: A tiene         (primerHijo/hermanoDerecho:
//      3 hijos, B tiene 2)              bajar = primer hijo,
//                                       ir a la derecha = siguiente hermano)
//
// ¿PUEDE UN ARBOL GENERAL ESTAR VACIO?
//   Definicion clasica (Knuth): un arbol es un conjunto finito de UNO o mas
//   nodos -> siempre hay raiz. Una hoja es un nodo cuyo conjunto de hijos
//   esta vacio; el arbol no.
//   - Lo que SI puede estar vacio es el CONTENEDOR: un objeto Arbol recien
//     creado todavia no tiene raiz (raiz_ == nullptr), igual que en la
//     clase 2 primero_ == nullptr era la lista vacia.
//   - Los nullptr de B en primerHijo_ / hermanoDerecho_ NO significan
//     "arbol vacio": significan "no hay primer hijo" o "no quedan mas
//     hermanos". Es la secuencia de hijos (un BOSQUE) la que puede estar
//     vacia, igual que el vector de A, solo que escrita con punteros.
//   Convenio: altura(vacio) = -1, para que una hoja salga con
//   1 + max(-1) = 0 sin caso especial. En el fichero 02 (binarios) el
//   arbol vacio sera OBLIGATORIO: sin el no se distingue "solo hijo
//   izquierdo" de "solo hijo derecho".
//
// Compilar:
//   g++ -std=c++17 -Wall -o 01_arbol 01_arbol_general.cpp && ./01_arbol

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// =========================================================================
// ESPECIFICACION del TAD Arbol (de etiquetas string)
// -------------------------------------------------------------------------
// Un Arbol es una coleccion de nodos con una relacion JERARQUICA:
//   - hay un nodo distinguido, la RAIZ, que no tiene padre;
//   - todo nodo que no es la raiz tiene EXACTAMENTE un padre;
//   - los hijos de un nodo estan ORDENADOS (primero, segundo, ...);
//   - un nodo sin hijos es una HOJA.
//
// Se trabaja con POSICIONES (como la Posicion del iterador de listas): una
// posicion identifica un nodo, y la posicion NULA (nullptr) significa
// "no hay tal nodo".
//
//   vacio()             pre : ninguna
//                       post: true si el arbol no tiene nodos
//   raiz()              pre : !vacio()
//                       post: devuelve la posicion de la raiz
//   padre(p)            pre : p valida
//                       post: devuelve el padre de p, o NULA si p es la raiz
//   primerHijo(p)       pre : p valida
//                       post: devuelve el primer hijo de p, o NULA si es hoja
//   hermanoDerecho(p)   pre : p valida
//                       post: devuelve el siguiente hijo del padre de p,
//                             o NULA si p es el ultimo
//   etiqueta(p)         pre : p valida
//                       post: devuelve el valor guardado en p
//   crearRaiz(v)        pre : vacio()
//                       post: el arbol tiene un unico nodo, con valor v
//   insertarHijo(p, v)  pre : p valida
//                       post: p tiene un hijo mas, con valor v, en la
//                             ULTIMA posicion; devuelve su posicion
//
// Como en la Lista: NADA de esto habla de vectores ni de punteros.
// =========================================================================

// La posicion es OPACA: quien usa el TAD no ve lo que hay dentro. Cada
// implementacion hereda su nodo de NodoBase y, por dentro, convierte la
// posicion a SU nodo con static_cast. NodoBase esta vacio, asi que no
// anade ni un byte a los nodos (optimizacion de base vacia).
struct NodoBase {};
using Posicion = NodoBase*;

class Arbol {
public:
    virtual ~Arbol() = default;

    virtual bool     vacio() const = 0;
    virtual Posicion raiz() const = 0;
    virtual Posicion padre(Posicion p) const = 0;
    virtual Posicion primerHijo(Posicion p) const = 0;
    virtual Posicion hermanoDerecho(Posicion p) const = 0;
    virtual const std::string& etiqueta(Posicion p) const = 0;

    virtual Posicion crearRaiz(const std::string& v) = 0;
    virtual Posicion insertarHijo(Posicion p, const std::string& v) = 0;

    virtual const char* nombre() const = 0;
};

// =========================================================================
// IMPLEMENTACION A: nodo con VECTOR de hijos.
// Para poder dar hermanoDerecho(p) hay que saber quien es el padre de p y
// en que lugar de su vector esta p: por eso este nodo guarda tambien padre.
// =========================================================================
struct NodoLista : NodoBase {
    std::string etiqueta;
    NodoLista* padre = nullptr;
    std::vector<NodoLista*> hijos;
    explicit NodoLista(std::string e) : etiqueta(std::move(e)) {} // move es una optimización: evita copiar la cadena, la "roba" del argumento
};

class ArbolLista : public Arbol {
public:
    ArbolLista() = default;
    ~ArbolLista() override { destruir(raiz_); }
    ArbolLista(const ArbolLista&) = delete;              // como en la clase 2:
    ArbolLista& operator=(const ArbolLista&) = delete;   // sin copias

    bool     vacio() const override { return raiz_ == nullptr; }
    Posicion raiz() const override {
        if (vacio()) throw std::logic_error("raiz() de un arbol vacio");
        return raiz_;
    }
    Posicion padre(Posicion p) const override { return nodo(p)->padre; }   // O(1)
    Posicion primerHijo(Posicion p) const override {                       // O(1)
        NodoLista* n = nodo(p);
        return n->hijos.empty() ? nullptr : n->hijos.front();
    }
    // O(k), k = hijos del padre: hay que localizar p en el vector del padre.
    Posicion hermanoDerecho(Posicion p) const override {
        NodoLista* n = nodo(p);
        if (n->padre == nullptr) return nullptr;            // la raiz no tiene hermanos
        const std::vector<NodoLista*>& h = n->padre->hijos;
        auto it = std::find(h.begin(), h.end(), n);
        return (it + 1 == h.end()) ? nullptr : *(it + 1);
    }
    const std::string& etiqueta(Posicion p) const override { return nodo(p)->etiqueta; }

    Posicion crearRaiz(const std::string& v) override {
        if (!vacio()) throw std::logic_error("crearRaiz() con raiz ya creada");
        raiz_ = new NodoLista(v);
        return raiz_;
    }
    Posicion insertarHijo(Posicion p, const std::string& v) override {
        NodoLista* nuevo = new NodoLista(v);
        nuevo->padre = nodo(p);
        nodo(p)->hijos.push_back(nuevo);   // O(1) amortizado: es un push_back de la clase 2
        return nuevo;
    }

    const char* nombre() const override { return "A (vector de hijos)"; }

private:
    static NodoLista* nodo(Posicion p) { return static_cast<NodoLista*>(p); }
    static void destruir(NodoLista* n) {
        if (n == nullptr) return;
        for (NodoLista* h : n->hijos) destruir(h);
        delete n;
    }
    NodoLista* raiz_ = nullptr;   // Invariante: raiz_ == nullptr <=> arbol vacio
};

// =========================================================================
// IMPLEMENTACION B: PRIMER-HIJO / HERMANO-DERECHO. Dos punteros, siempre.
// =========================================================================
struct NodoPHHD : NodoBase {
    std::string etiqueta;
    NodoPHHD* primerHijo_     = nullptr;
    NodoPHHD* hermanoDerecho_ = nullptr;
    explicit NodoPHHD(std::string e) : etiqueta(std::move(e)) {}
};

class ArbolPHHD : public Arbol {
public:
    ArbolPHHD() = default;
    ~ArbolPHHD() override { destruir(raiz_); }
    ArbolPHHD(const ArbolPHHD&) = delete;
    ArbolPHHD& operator=(const ArbolPHHD&) = delete;

    bool     vacio() const override { return raiz_ == nullptr; }
    Posicion raiz() const override {
        if (vacio()) throw std::logic_error("raiz() de un arbol vacio");
        return raiz_;
    }
    // O(n): el nodo no sabe quien es su padre, asi que hay que buscarlo
    // recorriendo el arbol desde la raiz. (Ver la pregunta del README.)
    Posicion padre(Posicion p) const override { return buscarPadre(raiz_, nodo(p)); }
    Posicion primerHijo(Posicion p) const override { return nodo(p)->primerHijo_; }         // O(1)
    Posicion hermanoDerecho(Posicion p) const override { return nodo(p)->hermanoDerecho_; } // O(1)
    const std::string& etiqueta(Posicion p) const override { return nodo(p)->etiqueta; }

    Posicion crearRaiz(const std::string& v) override {
        if (!vacio()) throw std::logic_error("crearRaiz() con raiz ya creada");
        raiz_ = new NodoPHHD(v);
        return raiz_;
    }
    // O(k), k = hijos que ya tenia el padre: hay que recorrer la cadena de
    // hermanos hasta el final. (La clase 2 ya nos enseno la solucion si esto
    // importara: guardar tambien un puntero al ULTIMO hermano. Aqui se deja
    // O(k) a proposito, para no esconder el precio real de cambiar de
    // representacion.)
    Posicion insertarHijo(Posicion p, const std::string& v) override {
        NodoPHHD* padre = nodo(p);
        NodoPHHD* nuevo = new NodoPHHD(v);
        if (padre->primerHijo_ == nullptr) { padre->primerHijo_ = nuevo; return nuevo; }
        NodoPHHD* h = padre->primerHijo_;
        while (h->hermanoDerecho_ != nullptr) h = h->hermanoDerecho_;
        h->hermanoDerecho_ = nuevo;
        return nuevo;
    }

    const char* nombre() const override { return "B (primerHijo/hermanoDerecho)"; }

private:
    static NodoPHHD* nodo(Posicion p) { return static_cast<NodoPHHD*>(p); }
    // Busca, entre los descendientes de n, al padre de objetivo.
    static NodoPHHD* buscarPadre(NodoPHHD* n, NodoPHHD* objetivo) {
        for (NodoPHHD* h = n->primerHijo_; h != nullptr; h = h->hermanoDerecho_) {
            if (h == objetivo) return n;
            if (NodoPHHD* r = buscarPadre(h, objetivo)) return r;
        }
        return nullptr;   // objetivo es la raiz (o no esta en este subarbol)
    }
    // Liberar es la misma recursion doble que el preorden: hijo y hermano.
    static void destruir(NodoPHHD* n) {
        if (n == nullptr) return;
        destruir(n->primerHijo_);
        destruir(n->hermanoDerecho_);
        delete n;
    }
    NodoPHHD* raiz_ = nullptr;    // Invariante: raiz_ == nullptr <=> arbol vacio
};

// =========================================================================
// OPERACIONES escritas UNA SOLA VEZ, sobre la interfaz (como imprimir() en
// la Lista). Sirven para A, para B y para cualquier implementacion futura.
// Ojo al precio: sobre A, cada hermanoDerecho cuesta O(k), asi que estas
// funciones son mas lentas en A que si se escribieran mirando el vector.
// Es lo que se paga por no ver la representacion.
// =========================================================================

// La recursion doble que hace que esto sea un preorden CORRECTO:
//   - bajar a primerHijo      -> es un HIJO      -> mas profundidad
//   - ir a hermanoDerecho     -> es un HERMANO   -> MISMA profundidad
void preorden(const Arbol& a, Posicion p, int profundidad, std::string& salida) {
    if (p == nullptr) return;
    salida += std::string(profundidad * 2, ' ') + a.etiqueta(p) + "\n"; //Aquí se multiplica la profundidad por 2 para crear una indentación visual en la salida, lo que ayuda a representar la estructura del árbol de manera más clara.
    preorden(a, a.primerHijo(p), profundidad + 1, salida);   // baja un nivel
    preorden(a, a.hermanoDerecho(p), profundidad, salida);   // se queda en este nivel
}

int altura(const Arbol& a, Posicion p) {
    int maxima = -1;
    for (Posicion h = a.primerHijo(p); h != nullptr; h = a.hermanoDerecho(h))
        maxima = std::max(maxima, altura(a, h));
    return maxima + 1;                                  // hoja: -1 + 1 = 0
}
int altura(const Arbol& a) { return a.vacio() ? -1 : altura(a, a.raiz()); }

int contarNodos(const Arbol& a, Posicion p) {
    int total = 1;
    for (Posicion h = a.primerHijo(p); h != nullptr; h = a.hermanoDerecho(h))
        total += contarNodos(a, h);
    return total;
}
int contarNodos(const Arbol& a) { return a.vacio() ? 0 : contarNodos(a, a.raiz()); }

// Construye el mismo sistema de ficheros pequenio usando SOLO la interfaz.
// Devuelve la posicion de "log", para usarla despues con padre().
Posicion construirEjemplo(Arbol& a) {
    Posicion raiz = a.crearRaiz("/");
    Posicion home = a.insertarHijo(raiz, "home");
    a.insertarHijo(home, "user1");
    a.insertarHijo(home, "user2");
    a.insertarHijo(raiz, "etc");
    Posicion var = a.insertarHijo(raiz, "var");
    return a.insertarHijo(var, "log");
}

int main() {
    std::cout << "=== El mismo arbol, dos representaciones ===\n";
    std::cout << "  (un sistema de ficheros pequenio: '/' con tres hijos)\n\n";

    ArbolLista arbolA;
    ArbolPHHD  arbolB;

    std::cout << "  recien creados: vacio? A=" << std::boolalpha << arbolA.vacio()
              << " B=" << arbolB.vacio() << "  altura=" << altura(arbolA)
              << "  nodos=" << contarNodos(arbolA) << "\n\n";

    Posicion logA = construirEjemplo(arbolA);    // el MISMO codigo construye los dos
    Posicion logB = construirEjemplo(arbolB);

    std::string salidaA, salidaB;
    preorden(arbolA, arbolA.raiz(), 0, salidaA);
    preorden(arbolB, arbolB.raiz(), 0, salidaB);

    std::cout << "  -- preorden, representacion " << arbolA.nombre() << " --\n" << salidaA;
    std::cout << "  -- preorden, representacion " << arbolB.nombre() << " --\n" << salidaB;
    std::cout << "  iguales? " << (salidaA == salidaB) << '\n';

    std::cout << "\n=== Altura y numero de nodos: deben coincidir ===\n";
    std::cout << "  A: altura=" << altura(arbolA) << "  nodos=" << contarNodos(arbolA) << '\n';
    std::cout << "  B: altura=" << altura(arbolB) << "  nodos=" << contarNodos(arbolB) << '\n';

    // padre(): el contrato es el mismo, el coste no.
    std::cout << "\n=== padre(\"log\"): mismo resultado, distinto coste ===\n";
    std::cout << "  A: " << arbolA.etiqueta(arbolA.padre(logA)) << "   (O(1): el nodo guarda su padre)\n";
    std::cout << "  B: " << arbolB.etiqueta(arbolB.padre(logB)) << "   (O(n): hay que buscarlo desde la raiz)\n";

    std::cout << "\n=== El precio en memoria de cada representacion ===\n";
    std::cout << "  sizeof(NodoLista) = " << sizeof(NodoLista) // sizeof(std::vector<NodoLista*>) = 24 bytes (en 64 bits) porque vector tiene tres punteros internos: begin, end, capacity. Cada NodoLista tiene su propio vector, por lo que cada nodo tiene un bloque de memoria adicional para su vector de hijos.
              << " bytes  (+ el vector<NodoLista*> de cada nodo tiene su\n"
              << "                         PROPIO bloque reservado en el heap:"
              << " capacidad, no solo tamanio)\n";
    std::cout << "  sizeof(NodoPHHD)  = " << sizeof(NodoPHHD)
              << " bytes  (siempre, tenga el nodo 0 o 100 hijos: dos punteros,\n"
              << "                         cero bloques adicionales)\n";
    std::cout << "\n  La representacion B es exactamente un arbol binario. Con eso\n"
              << "  se cierra el argumento: todo lo que aprendamos sobre arboles\n"
              << "  BINARIOS (fichero 02 en adelante) sirve tambien para arboles\n"
              << "  generales, sin representar nada distinto.\n";
}
