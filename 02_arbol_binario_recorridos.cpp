// 02_arbol_binario_recorridos.cpp
// Objetivo: el Arbol Binario como estructura (cada nodo tiene A LO SUMO dos
// hijos, izquierdo y derecho, y hay diferencia entre ellos), y las cuatro
// formas de recorrerlo.
//
// EL TAD ARBOL BINARIO. A diferencia del arbol general del fichero 01, aqui
// el arbol vacio NO es opcional: forma parte de la definicion, que es
// recursiva:
//
//   un arbol binario es, o bien el arbol VACIO, o bien una RAIZ con dos
//   subarboles binarios (izquierdo y derecho), cualquiera de ellos vacio.
//
// Sin el arbol vacio no se podria distinguir "solo hijo izquierdo" de
// "solo hijo derecho", que son arboles DISTINTOS:
//
//        50            50
//       /                \       <- mismos nodos, arboles distintos
//     80                  80
//
// Las operaciones son las del fichero 01 con otra navegacion: en vez de
// primerHijo(p) / hermanoDerecho(p), ahora izquierdo(p) / derecho(p), y
// cualquiera de las dos puede devolver la posicion NULA (subarbol vacio).
// Para centrarnos en los recorridos, en este fichero la posicion es
// directamente un Nodo* y la nula es nullptr: cada "if (n == nullptr)" del
// codigo es el caso base de la definicion recursiva. La clase que
// encapsula los nodos llega en el fichero 03.
//
// OJO CON EL MALENTENDIDO MAS COMUN DEL TEMA: un arbol binario CUALQUIERA
// no tiene por que estar ordenado. El arbol de este fichero NO lo esta:
// recorrerlo en inorden NO da la secuencia ordenada. Eso solo pasara cuando,
// en el fichero 03, anadamos una propiedad extra (arbol de BUSQUEDA). Aqui
// solo hay FORMA, todavia sin ORDEN.
//
// LOS TRES RECORRIDOS EN PROFUNDIDAD son la misma recursion con la visita
// movida de sitio:
//
//   preorden:  visitar, IZQUIERDO, DERECHO      (raiz primero)
//   inorden:   IZQUIERDO, visitar, DERECHO      (raiz en medio)
//   postorden: IZQUIERDO, DERECHO, visitar      (raiz al final)
//
// El recorrido POR NIVELES (BFS) es distinto de raiz: no es profundidad,
// es anchura, y por eso necesita una COLA en vez de la pila de llamadas.
// Es literalmente el mismo bucle BFS del tema 3 (3_practicas/04), aplicado
// a un arbol en vez de a un grafo general.
//
// El arbol de ejemplo (a proposito SIN ningun orden):
//
//                50
//              /    \
//            80      30
//           /  \        \
//         10    90       20
//                           \
//                            60
//
// Compilar:
//   g++ -std=c++17 -Wall -o 02_arbol 02_arbol_binario_recorridos.cpp && ./02_arbol

#include <iostream>
#include <vector>
#include <queue>
#include <stack>

struct Nodo {
    int valor;
    Nodo* izq = nullptr;
    Nodo* der = nullptr;
    explicit Nodo(int v) : valor(v) {}
};

// --- Los tres recorridos en profundidad: misma forma, visita movida -------

void preorden(Nodo* n, std::vector<int>& salida) {
    if (n == nullptr) return;
    salida.push_back(n->valor);   // visitar...
    preorden(n->izq, salida);     // ...luego izquierdo...
    preorden(n->der, salida);     // ...luego derecho
}

void inorden(Nodo* n, std::vector<int>& salida) {
    if (n == nullptr) return;
    inorden(n->izq, salida);
    salida.push_back(n->valor);   // visitar EN MEDIO
    inorden(n->der, salida);
}

void postorden(Nodo* n, std::vector<int>& salida) {
    if (n == nullptr) return;
    postorden(n->izq, salida);
    postorden(n->der, salida);
    salida.push_back(n->valor);   // visitar AL FINAL
}

// --- Por niveles (BFS): la Cola del tema 3, aplicada a un arbol ----------
//
// Se mete la raiz en una cola. En cada vuelta se saca un nodo, se visita, y
// se meten sus dos hijos (si existen) al FINAL de la cola. Como la cola es
// FIFO, todos los nodos de un nivel salen antes que ninguno del siguiente.
std::vector<int> porNiveles(Nodo* raiz) {
    std::vector<int> salida;
    if (raiz == nullptr) return salida;
    std::queue<Nodo*> pendientes;
    pendientes.push(raiz);
    while (!pendientes.empty()) {
        Nodo* actual = pendientes.front();
        pendientes.pop();
        salida.push_back(actual->valor);
        if (actual->izq != nullptr) pendientes.push(actual->izq);
        if (actual->der != nullptr) pendientes.push(actual->der);
    }
    return salida;
}

// --- Ejercicio: el inorden SIN recursion --------------------------------
//
// El tema 3 lo dijo y aqui toca demostrarlo: "toda recursion se puede
// convertir en un bucle con una pila explicita", porque una llamada
// recursiva no es magia, es la pila de llamadas del programa.
//
// La idea del inorden iterativo:
//   1. Bajar SIEMPRE por la izquierda, apilando cada nodo que se deja atras
//      (son visitas pendientes: su "izquierdo, visitar" aun no ha pasado).
//   2. Cuando ya no se puede bajar mas, desapilar UNO: ESE es el siguiente
//      en visitarse (ya no le queda nada a la izquierda sin visitar).
//   3. Despues de visitarlo, repetir el paso 1 pero partiendo de su hijo
//      DERECHO (todavia no visitado).
//   4. Terminar cuando la pila esta vacia Y no queda nodo actual.
//
// TU TAREA: completar el cuerpo. El resultado, comparado con inorden()
// (la version recursiva de arriba), debe salir identico.
std::vector<int> inordenIterativo(Nodo* raiz) {
    std::vector<int> salida;
    // TODO: implementar con std::stack<Nodo*>, sin ninguna llamada recursiva.
    return salida;
}

int altura(Nodo* n) {
    if (n == nullptr) return -1;
    return 1 + std::max(altura(n->izq), altura(n->der));
}

int contarNodos(Nodo* n) {
    if (n == nullptr) return 0;
    return 1 + contarNodos(n->izq) + contarNodos(n->der);
}

// Liberar el arbol es un POSTORDEN: hay que borrar los dos hijos ANTES que
// el nodo, porque despues de "delete n" ya no se puede leer n->izq ni n->der.
void liberar(Nodo* n) {
    if (n == nullptr) return;
    liberar(n->izq);
    liberar(n->der);
    delete n;
}

void imprimir(const char* etiqueta, const std::vector<int>& v) {
    std::cout << "  " << etiqueta;
    for (int x : v) std::cout << x << ' ';
    std::cout << '\n';
}

int main() {
    // Construimos el arbol de ejemplo a mano, enlazando los punteros.
    Nodo* n50 = new Nodo(50);
    Nodo* n80 = new Nodo(80);
    Nodo* n30 = new Nodo(30);
    Nodo* n10 = new Nodo(10);
    Nodo* n90 = new Nodo(90);
    Nodo* n20 = new Nodo(20);
    Nodo* n60 = new Nodo(60);

    n50->izq = n80;  n50->der = n30;
    n80->izq = n10;  n80->der = n90;
    n30->der = n20;
    n20->der = n60;

    std::cout << "=== El arbol (SIN ningun orden todavia) ===\n";
    std::cout << "  altura=" << altura(n50) << "  nodos=" << contarNodos(n50) << "\n\n";

    std::vector<int> pre, in, post;
    preorden(n50, pre);
    inorden(n50, in);
    postorden(n50, post);

    imprimir("preorden:   ", pre);
    imprimir("inorden:    ", in);
    imprimir("postorden:  ", post);
    imprimir("por niveles:", porNiveles(n50));

    std::cout << "\n  Fijate en el inorden: NO sale ordenado (10 80 90 50 30 20 60).\n"
              << "  Es la prueba de que un arbol binario, por si solo, no tiene\n"
              << "  ninguna propiedad de orden. Eso es justo lo que anadimos en\n"
              << "  el fichero 03.\n";

    std::cout << "\n=== El mismo inorden, sin recursion (tu ejercicio) ===\n";
    std::vector<int> inIter = inordenIterativo(n50);
    imprimir("inorden iterativo:", inIter);
    std::cout << "  coincide con el recursivo: " << std::boolalpha << (inIter == in) << '\n';
    if (inIter != in)
        std::cout << "  (todavia no, porque inordenIterativo() esta sin completar)\n";

    liberar(n50);   // cada new tiene su delete
}
