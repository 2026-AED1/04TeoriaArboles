// 08_adaptadores_stl.cpp
// Objetivo: cerrar el tema viendo como la biblioteca estandar implementa de
// verdad los arboles que hemos construido a mano: la cola de prioridad
// (fichero 04) y el arbol de busqueda (ficheros 03 y 06).
//
// PARTE 1: std::priority_queue. Por que el COMPARADOR es tambien un
// parametro de plantilla, igual que el contenedor lo era en std::stack y
// std::queue:
//
//     template <class T, class Container = std::vector<T>,
//               class Compare = std::less<T>>
//     class priority_queue;
//
// No es una estructura nueva: por debajo es el monticulo binario del
// fichero 04_cola_prioridad_heap.cpp, montado sobre un std::vector.
//
// PARTE 2: std::set y std::map. Por debajo NO son un arbol binario de
// busqueda simple ni un AVL: son un ARBOL ROJO-NEGRO, un arbol de busqueda
// equilibrado por construccion (ver 2_profesor/11_arbol_rojinegro.md). La
// consecuencia que se ve desde fuera: insertar/buscar/borrar son SIEMPRE
// O(log n), sin el caso patologico O(n) del BST simple del fichero 03, y
// ademas se puede recorrer en orden con un iterador normal, cosa que
// priority_queue no permite.
//
// Compilar:
//   g++ -std=c++17 -O2 -Wall -o 08_stl 08_adaptadores_stl.cpp && ./08_stl

#include <iostream>
#include <iomanip>
#include <queue>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <functional>
#include <random>
#include <chrono>
#include <algorithm>
#include <cmath>

int main() {
    std::cout << "=== 1) priority_queue: el comparador tambien es un parametro ===\n";
    {
        std::priority_queue<int> maximos;                                   // por defecto: max
        std::priority_queue<int, std::vector<int>, std::greater<int>> minimos;
        for (int v : {5, 1, 9, 3, 7}) { maximos.push(v); minimos.push(v); }

        std::cout << "  max-heap (por defecto): ";
        while (!maximos.empty()) { std::cout << maximos.top() << ' '; maximos.pop(); }
        std::cout << "\n  min-heap (greater<>):   ";
        while (!minimos.empty()) { std::cout << minimos.top() << ' '; minimos.pop(); }
        std::cout << '\n';
    }

    std::cout << "\n  Con prioridades reales, y el desempate resuelto a mano:\n";
    {
        // El monticulo NO es estable. Si dos tareas empatan en prioridad, el
        // orden entre ellas es arbitrario. Se arregla metiendo el orden de
        // llegada DENTRO de la clave: asi nunca hay empates de verdad.
        struct Tarea {
            int prioridad;
            int llegada;
            std::string nombre;
            bool operator<(const Tarea& o) const {
                if (prioridad != o.prioridad) return prioridad < o.prioridad;
                return llegada > o.llegada;            // llegada menor -> "mayor"
            }
        };

        std::priority_queue<Tarea> cola;
        int t = 0;
        cola.push({1, t++, "backup nocturno"});
        cola.push({5, t++, "caida de produccion"});
        cola.push({3, t++, "ticket de soporte"});
        cola.push({5, t++, "alerta de seguridad"});
        cola.push({3, t++, "otro ticket"});

        std::cout << "    orden de atencion:\n";
        while (!cola.empty()) {
            const Tarea& x = cola.top();
            std::cout << "      prioridad " << x.prioridad
                      << " (llego " << x.llegada << ")  " << x.nombre << '\n';
            cola.pop();
        }
        std::cout << "    Las dos de prioridad 5 salen en orden de llegada porque\n"
                  << "    lo hemos pedido explicitamente. Sin ese desempate, el\n"
                  << "    orden entre ellas no estaria definido por el TAD.\n";
    }

    std::cout << "\n=== 2) set/map: el arbol de busqueda que NO degenera ===\n";
    {
        // Lo que en el fichero 03 rompia el BST simple: insertar ya ordenado.
        std::set<int> s;
        for (int i = 0; i < 15; i++) s.insert(i);           // 0,1,2,...,14: EN ORDEN
        std::cout << "  insertados 0..14 EN ORDEN (el peor caso del BST simple)\n";
        std::cout << "  recorrido con iterador (siempre en orden, gratis): ";
        for (int x : s) std::cout << x << ' ';
        std::cout << "\n  y sigue siendo O(log n) buscar, insertar y borrar: el arbol\n"
                  << "  rojo-negro que hay debajo se reequilibra solo (fichero 06 es\n"
                  << "  la version AVL de esa misma idea, mas sencilla de programar).\n";
    }

    std::cout << "\n  Medido: BST simple (fichero 03) frente a std::set, insertando EN ORDEN\n";
    {
        std::cout << "  " << std::left << std::setw(10) << "n"
                  << std::setw(18) << "set::insert (ms)"
                  << "altura simulada\n";
        for (int n : {1000, 5000, 20000, 80000}) {
            std::set<int> s;
            auto t0 = std::chrono::steady_clock::now();
            for (int i = 0; i < n; i++) s.insert(i);
            auto t1 = std::chrono::steady_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            std::cout << "  " << std::left << std::setw(10) << n
                      << std::setw(18) << std::fixed << std::setprecision(3) << ms
                      << "~" << (int)(2 * std::log2(n + 1)) << " (cota rojo-negro: 2*log2(n))\n";
        }
        std::cout << "  Nunca se dispara, a diferencia del BST simple del fichero 03:\n"
                  << "  ahi insertar en orden daba una CADENA de altura n.\n";
    }

    std::cout << "\n=== 3) std::map es std::set de pares (clave, valor) ===\n";
    {
        std::map<std::string, int> edad;
        edad["Ada"] = 36;
        edad["Alan"] = 41;
        edad["Grace"] = 85;
        std::cout << "  recorrido de un map (SIEMPRE por clave, en orden):\n";
        for (const auto& [nombre, anios] : edad)
            std::cout << "    " << nombre << " -> " << anios << '\n';
    }

    std::cout << "\n=== Tabla final del tema, con todos los TADs sobre arboles ===\n";
    std::cout << "  TAD                 STL                  por debajo\n"
              << "  -------------------------------------------------------------\n"
              << "  Cola de prioridad   std::priority_queue  monticulo binario (array)\n"
              << "  Conjunto ordenado   std::set             arbol rojo-negro\n"
              << "  Diccionario         std::map             arbol rojo-negro de pares\n"
              << "  Con repeticiones    std::multiset/map    igual, sin exigir claves unicas\n\n"
              << "  priority_queue usa vector porque el monticulo vive de la aritmetica\n"
              << "  i -> 2i+1 sobre memoria contigua. set/map usan un arbol enlazado por\n"
              << "  punteros porque necesitan insertar/borrar SIN mover el resto y\n"
              << "  mantener el recorrido en orden barato. Cada defecto responde a lo\n"
              << "  que la estructura necesita de verdad.\n\n"
              << "  Y ninguno de los dos es un B-tree (fichero 07): el B-tree solo\n"
              << "  compensa cuando cada nodo es un acceso a disco, no a memoria.\n";
}
