// 05_heapify_heapsort.cpp
// Objetivo: dos resultados que sorprenden, y un algoritmo que sale gratis.
//
// (A) CONSTRUIR un monticulo con n elementos cuesta O(n), no O(n log n),
//     si se hace bien. Es de los pocos sitios donde el analisis "obvio"
//     (n elementos x log n cada uno) da una cota CIERTA pero NO AJUSTADA.
//
// (B) HEAPSORT: ordenar con un monticulo, en O(n log n), sin memoria extra.
//
// (C) TOP-K: si solo quieres los k mayores de n, ordenar es un despilfarro.
//     Con un monticulo de tamanio k basta: O(n log k) y memoria O(k).
//
// Compilar:
//   g++ -std=c++17 -O2 -Wall -o 05_heapify 05_heapify_heapsort.cpp && ./05_heapify

#include <iostream>
#include <iomanip>
#include <vector>
#include <queue>
#include <random>
#include <algorithm>
#include <chrono>

// -------------------------------------------------------------------------
// Operaciones basicas sobre un vector visto como monticulo de maximos.
// -------------------------------------------------------------------------
long bajar(std::vector<int>& v, int i, int n) {
    long ops = 0;
    while (true) {
        int mayor = i, iz = 2 * i + 1, de = 2 * i + 2;
        if (iz < n) { ops++; if (v[iz] > v[mayor]) mayor = iz; }
        if (de < n) { ops++; if (v[de] > v[mayor]) mayor = de; }
        if (mayor == i) break;
        std::swap(v[i], v[mayor]);
        ops++;
        i = mayor;
    }
    return ops;
}

long subir(std::vector<int>& v, int i) {
    long ops = 0;
    while (i > 0) {
        int p = (i - 1) / 2;
        ops++;
        if (v[p] >= v[i]) break;
        std::swap(v[p], v[i]);
        ops++;
        i = p;
    }
    return ops;
}

bool esMonticulo(const std::vector<int>& v) {
    for (std::size_t i = 1; i < v.size(); i++)
        if (v[(i - 1) / 2] < v[i]) return false;
    return true;
}

// =========================================================================
// (A) DOS FORMAS DE CONSTRUIR EL MONTICULO
// =========================================================================

// Forma 1, la evidente: insertar uno a uno, subiendo cada vez.
// n inserciones x O(log n) = O(n log n).
long construirInsertando(std::vector<int>& v) {
    long ops = 0;
    std::vector<int> h;
    for (int x : v) {
        h.push_back(x);
        ops += subir(h, static_cast<int>(h.size()) - 1);
    }
    v = h;
    return ops;
}

// Forma 2, la de Floyd (1964): meter TODO el array de golpe (la forma
// completa ya la cumple, sin hacer nada) y despues arreglar la propiedad de
// monticulo bajando, desde el ultimo padre hacia la raiz.
//
// POR QUE ES O(n), y no O(n log n):
// el coste de bajar NO es log n para todos los nodos, sino la altura DE CADA
// NODO. Y en un arbol completo la inmensa mayoria de los nodos estan abajo,
// donde la altura es diminuta:
//
//      n/2  hojas          -> altura 0  (ni se tocan: el bucle ni las visita)
//      n/4  nodos          -> altura 1
//      n/8  nodos          -> altura 2
//      ...
//      1    raiz           -> altura log n
//
// El total es la suma  n * SUM(h / 2^(h+1))  para h = 0, 1, 2, ...
// Esa serie converge a 1 (no crece con n), asi que el total es <= 2n = O(n).
//
// La moraleja general: "n operaciones de coste O(log n) cada una" da O(n log n),
// pero si la mayoria de esas operaciones son mucho mas baratas que el peor
// caso, la cota se puede afinar. Es el mismo tipo de razonamiento que el
// analisis amortizado, aunque aqui no haya amortizacion de por medio.
long construirFloyd(std::vector<int>& v) {
    long ops = 0;
    int n = static_cast<int>(v.size());
    for (int i = n / 2 - 1; i >= 0; i--) ops += bajar(v, i, n);   // <-- de abajo arriba
    return ops;
}

// =========================================================================
// (B) HEAPSORT
// =========================================================================
//
// 1. Construir el monticulo de maximos (Floyd): O(n).
// 2. Repetir n-1 veces: intercambiar la raiz (el maximo) con el ultimo
//    elemento del trozo activo y encoger el trozo. El maximo queda colocado
//    en su sitio definitivo, al final. Reajustar con bajar: O(log n).
//
// Total O(n log n) SIEMPRE (no tiene peor caso malo, a diferencia de
// quicksort) y "in situ": memoria extra O(1).
//
// Por que entonces el estandar usa introsort y no heapsort: heapsort salta
// por todo el array (mala localidad de cache) y hace mas intercambios. Otra
// vez lo mismo: mismo orden de complejidad, constantes muy distintas.
long heapsort(std::vector<int>& v) {
    long ops = construirFloyd(v);
    for (int fin = static_cast<int>(v.size()) - 1; fin > 0; fin--) {
        std::swap(v[0], v[fin]);
        ops++;
        ops += bajar(v, 0, fin);
    }
    return ops;
}

// =========================================================================
// (C) TOP-K con un monticulo de MINIMOS de tamanio k
// =========================================================================
//
// Truco contraintuitivo: para quedarse con los k MAYORES se usa un monticulo
// de MINIMOS. La raiz es entonces "el peor de los que he guardado", y es
// justo el que hay que expulsar cuando llega alguien mejor.
//
// Coste O(n log k) y memoria O(k). Con n = 10^9 y k = 10 esto funciona;
// ordenar los 10^9 no.
std::vector<int> topK(const std::vector<int>& v, int k) {
    std::priority_queue<int, std::vector<int>, std::greater<int>> mejores;  // min-heap
    for (int x : v) {
        if (static_cast<int>(mejores.size()) < k) mejores.push(x);
        else if (x > mejores.top()) { mejores.pop(); mejores.push(x); }
    }
    std::vector<int> res;
    while (!mejores.empty()) { res.push_back(mejores.top()); mejores.pop(); }
    std::sort(res.rbegin(), res.rend());
    return res;
}

int main() {
    std::mt19937 gen(7);
    std::uniform_int_distribution<int> dist(0, 1000000);

    std::cout << "=== (A) Construir el monticulo: O(n log n) frente a O(n) ===\n";
    std::cout << "  " << std::left << std::setw(12) << "n"
              << std::setw(16) << "insertando"
              << std::setw(10) << "ops/n"
              << std::setw(16) << "Floyd"
              << "ops/n\n";
    for (int n : {10000, 20000, 40000, 80000, 160000}) {
        std::vector<int> base(n);
        for (int& x : base) x = dist(gen);

        std::vector<int> a = base, b = base;
        long opsA = construirInsertando(a);
        long opsB = construirFloyd(b);

        std::cout << "  " << std::left << std::setw(12) << n
                  << std::setw(16) << opsA
                  << std::setw(10) << std::fixed << std::setprecision(2) << double(opsA) / n
                  << std::setw(16) << opsB
                  << double(opsB) / n
                  << (esMonticulo(a) && esMonticulo(b) ? "" : "  [ERROR]") << '\n';
    }
    std::cout << "\n  La columna ops/n de Floyd es CONSTANTE (~2): coste lineal.\n"
              << "  La de insertando CRECE con n: ahi esta el factor log n.\n"
              << "  Las dos construyen un monticulo igual de valido. Con estos n la\n"
              << "  diferencia es solo de un 35%, pero no es una constante: sigue\n"
              << "  subiendo con n, porque son ordenes distintos. Prueba con 10^7.\n";

    std::cout << "\n=== (B) Heapsort ===\n";
    std::vector<int> peq(12);
    for (int& x : peq) x = dist(gen) % 100;
    std::cout << "  antes:   "; for (int x : peq) std::cout << std::setw(3) << x << ' '; std::cout << '\n';
    heapsort(peq);
    std::cout << "  despues: "; for (int x : peq) std::cout << std::setw(3) << x << ' '; std::cout << '\n';
    std::cout << "  ordenado: " << std::boolalpha
              << std::is_sorted(peq.begin(), peq.end()) << ", sin memoria extra\n";

    std::cout << "\n  Comparacion con std::sort (introsort), mismo O(n log n):\n";
    for (int n : {200000, 400000, 800000}) {
        std::vector<int> base(n);
        for (int& x : base) x = dist(gen);
        std::vector<int> a = base, b = base;

        auto t0 = std::chrono::steady_clock::now();
        heapsort(a);
        auto t1 = std::chrono::steady_clock::now();
        std::sort(b.begin(), b.end());
        auto t2 = std::chrono::steady_clock::now();

        double msH = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double msS = std::chrono::duration<double, std::milli>(t2 - t1).count();
        std::cout << "    n=" << std::left << std::setw(9) << n
                  << "heapsort " << std::fixed << std::setprecision(1) << std::setw(8) << msH
                  << "ms   std::sort " << std::setw(8) << msS << "ms   "
                  << "(x" << std::setprecision(1) << msH / msS << ")"
                  << (a == b ? "  [mismo resultado]" : "  [DISTINTO!]") << '\n';
    }
    std::cout << "    Mismo orden de complejidad, y sin embargo heapsort pierde\n"
              << "    claramente: salta por todo el array y destroza la cache.\n";

    std::cout << "\n=== (C) Top-K: no ordenes lo que no necesitas ===\n";
    {
        const int n = 5000000, k = 10;
        std::vector<int> grande(n);
        for (int& x : grande) x = dist(gen);

        auto t0 = std::chrono::steady_clock::now();
        std::vector<int> mejores = topK(grande, k);
        auto t1 = std::chrono::steady_clock::now();
        std::vector<int> copia = grande;
        std::sort(copia.rbegin(), copia.rend());
        auto t2 = std::chrono::steady_clock::now();

        std::cout << "  n = " << n << ", k = " << k << '\n';
        std::cout << "    monticulo de k:  "
                  << std::fixed << std::setprecision(1)
                  << std::chrono::duration<double, std::milli>(t1 - t0).count()
                  << " ms   (memoria extra: " << k << " enteros)\n";
        std::cout << "    ordenar todo:    "
                  << std::chrono::duration<double, std::milli>(t2 - t1).count()
                  << " ms   (memoria extra: " << n << " enteros)\n";
        std::cout << "    los 10 mayores: ";
        for (int x : mejores) std::cout << x << ' ';
        std::cout << '\n';
        std::cout << "    coinciden con ordenar: " << std::boolalpha
                  << std::equal(mejores.begin(), mejores.end(), copia.begin()) << '\n';
    }

    std::cout << "\n  Este es el patron de 'los 10 articulos mas vendidos' sobre un\n"
              << "  flujo de millones de eventos: O(n log k), memoria O(k), y\n"
              << "  funciona aunque los datos no quepan en memoria, porque cada\n"
              << "  elemento se mira UNA vez y se tira.\n";
}
