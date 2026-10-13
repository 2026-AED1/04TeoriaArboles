# Tema 4 (i): Árboles

## Cómo usar esta guía

Igual que en las clases anteriores: cada bloque introduce una idea y termina
en un ejercicio con código ejecutable. Lee primero el planteamiento, intenta
anticipar el resultado y solo después despliega el código y ejecútalo para
comparar tu razonamiento con lo que mide la máquina.

Algunos ficheros tienen partes marcadas con `// TODO: implementar`: son el
ejercicio. Complétalas tú.

Para compilar y ejecutar cualquier fichero:

```bash
g++ -std=c++17 -Wall -o 01_arbol 01_arbol_general.cpp && ./01_arbol
```

En los ficheros que miden tiempos (`03`, `05`, `06`, `07`, `08`) conviene
compilar con optimización, que es como se compila el código de verdad:

```bash
g++ -std=c++17 -O2 -Wall -o 03_bst 03_arbol_binario_busqueda.cpp && ./03_bst
```

**Ficheros de esta carpeta:**

| Fichero                             | Contenido                                                                            | ¿Hay que completarlo? |
| ----------------------------------- | ------------------------------------------------------------------------------------ | ---------------------- |
| `01_arbol_general.cpp`            | El TAD Árbol y dos implementaciones del árbol n-ario; por qué una de ellas ES un árbol binario | No, está completo     |
| `02_arbol_binario_recorridos.cpp` | El árbol binario y sus cuatro recorridos                                            | **Sí**          |
| `03_arbol_binario_busqueda.cpp`   | El BST: orden total, y su caso patológico                                           | **Sí**          |
| `04_cola_prioridad_heap.cpp`      | El montículo binario: un árbol que vive dentro de un array                         | No, está completo     |
| `05_heapify_heapsort.cpp`         | Construcción en O(n), heapsort y el patrón*top-k*                                | No, está completo     |
| `06_arbol_avl.cpp`                | El AVL: reequilibrar con rotaciones tras cada inserción                             | **Sí**          |
| `07_arbol_b.cpp`                  | El B-tree: nodos con muchas claves, pensado para el disco                            | **Sí**          |
| `08_adaptadores_stl.cpp`          | `std::priority_queue`, `std::set` y `std::map`: los TADs del tema hechos tipo  | No, está completo     |

**Lo que se da por sabido de las clases anteriores:** todo el tema 2 (listas,
coste amortizado) y el tema 3 (Pila, Cola, Deque, y los adaptadores de la
STL). En concreto, la Pila reaparece aquí para convertir una recursión en un
bucle explícito, y la Cola para el recorrido por niveles.

---

## 0. El hilo conductor del tema

Cada bloque de este tema existe porque el anterior deja un problema abierto.
No es una lista de seis estructuras sueltas: es una única pregunta —***cómo
organizar datos** jerárquicamente **para que buscar sea barato***— respondida seis
veces, cada una con una restricción distinta.

| # | Bloque         | Lo que aporta                                                                | El problema que deja abierto                                          |
| - | -------------- | ---------------------------------------------------------------------------- | --------------------------------------------------------------------- |
| 1 | Árbol general | Jerarquía, sin más                                                         | Ninguna aridad fija: incómodo de programar                           |
| 2 | Árbol binario | Aridad 2, y los recorridos                                                   | Ningún orden: buscar sigue siendo O(n)                               |
| 3 | BST            | Orden total → buscar en O(log n)...                                         | ...si el árbol no degenera. Insertar ya ordenado → O(n)             |
| 4 | Montículo     | Garantiza O(log n)**por la forma**, no por el azar                     | Renuncia a la búsqueda por valor: solo sirve para el extremo         |
| 5 | AVL            | Recupera la búsqueda por valor, y repara el problema del BST con rotaciones | Sigue siendo binario: con millones de claves en disco, la altura pesa |
| 6 | B-tree         | Nodos con muchas claves: la altura se desploma                               | (cierra el tema: es lo que hay debajo de un índice de base de datos) |

Fíjate especialmente en el salto del 3 al 4: el montículo no es "el
siguiente paso lógico" del BST, es una respuesta **distinta** a la misma
pregunta ("¿cómo garantizo O(log n)?"), que sacrifica algo que el BST tenía
(la búsqueda por valor) a cambio de una garantía de forma que el BST no
podía dar. Y el salto del 4 al 5 es la respuesta contraria: en vez de
cambiar de estructura, se **repara** el BST con trabajo extra en cada
inserción. Los dos caminos son legítimos, y saber cuál elegir —cambiar de
estructura o reparar la que ya tienes— es una de las lecciones del tema.

---

## 1. El árbol general

Un **árbol** es la generalización de la Lista del tema 2: en vez de "el
siguiente", cada nodo tiene "hijos", y puede tener cualquier número de
ellos. La raíz no tiene padre; todos los demás tienen exactamente uno.

### 1.0 El TAD Árbol

En el tema 2 separamos el **qué** (el TAD Lista: qué operaciones hay y qué
significa cada una) del **cómo** (enlazada, array dinámico). Aquí hacemos lo
mismo. Un **Árbol** es una colección de elementos con una relación
**jerárquica**:

- hay un nodo distinguido, la **raíz**, que no tiene padre;
- todo nodo que no es la raíz tiene **exactamente un padre**;
- los hijos de un nodo están **ordenados**: hay un primero, un segundo...;
- un nodo sin hijos es una **hoja**.

**Vocabulario:**

- **Camino**: secuencia de nodos en la que cada uno es padre del siguiente.
- **Profundidad de un nodo**: número de aristas desde la raíz hasta él (la raíz tiene profundidad 0).
- **Altura del árbol**: la mayor profundidad de sus hojas. Un árbol con un solo nodo tiene altura 0 y, por convenio, el árbol vacío tiene altura −1.
- **Grado** de un nodo: su número de hijos.
- **Subárbol**: un nodo junto con todos sus descendientes.

**Especificación.** Se trabaja con **posiciones** (como la `Posicion` del
iterador de listas): una posición identifica un nodo, y la posición *nula*
significa "no hay tal nodo".

| Operación             | Pre        | Post                                                               |
| --------------------- | ---------- | ------------------------------------------------------------------ |
| `vacio()`           | —          | `true` si el árbol no tiene nodos                                |
| `raiz()`            | `!vacio()` | la posición de la raíz                                            |
| `padre(p)`          | `p` válida | el padre de `p`, o *nula* si `p` es la raíz                     |
| `primerHijo(p)`     | `p` válida | el primer hijo de `p`, o *nula* si es hoja                       |
| `hermanoDerecho(p)` | `p` válida | el siguiente hijo del padre de `p`, o *nula* si `p` es el último |
| `etiqueta(p)`       | `p` válida | el valor guardado en `p` (sin modificar el árbol)                 |
| `crearRaiz(v)`      | `vacio()`  | el árbol pasa a tener un único nodo, con valor `v`               |
| `insertarHijo(p, v)` | `p` válida | `p` tiene un hijo más, con valor `v`, en la **última** posición |

Nada de esto habla de vectores ni de punteros. En `01_arbol_general.cpp` el
contrato es la clase abstracta `Arbol` (métodos `virtual ... = 0`, como
`ListaDeEnteros` en el tema 2), y hay dos **implementaciones** del mismo
contrato. Además, `preorden`, `altura` y `contarNodos` se escriben **una sola
vez** sobre la interfaz, igual que `imprimir()` en la Lista, y funcionan con las
dos.

**¿Puede un árbol estar vacío?** Según la definición clásica, un árbol tiene
*uno o más* nodos: siempre hay raíz. Lo que sí puede estar vacío es el
**contenedor**: un objeto `Arbol` recién creado todavía no tiene raíz
(`raiz_ == nullptr`), igual que `primero_ == nullptr` era la lista vacía.

### Las dos implementaciones

| Representación               | Cada nodo guarda                                                 | Coste por nodo                                               |
| ----------------------------- | ---------------------------------------------------------------- | ------------------------------------------------------------ |
| Lista de hijos                | Un `vector` de punteros a sus hijos, y un puntero al padre       | Variable: el propio `vector` reserva y crece                 |
| Primer-hijo / hermano-derecho | Dos punteros: al primer hijo, y al siguiente hermano             | **Fijo**: siempre dos punteros, sea cual sea la aridad       |

El contrato es el mismo, pero el coste de cada operación no:

| Operación          | A (vector de hijos)           | B (primer-hijo / hermano-derecho) |
| ------------------ | ----------------------------- | --------------------------------- |
| `primerHijo`     | O(1)                          | O(1)                              |
| `hermanoDerecho` | O(k): buscar `p` en el vector del padre | O(1)                    |
| `padre`          | O(1)                          | **O(n)**: buscarlo desde la raíz  |
| `insertarHijo`   | O(1) amortizado               | O(k): recorrer los hermanos       |

(k = número de hijos del padre, n = número de nodos del árbol.)

**Pregunta para pensar:** en B, `padre(p)` cuesta O(n). ¿Qué añadirías al
nodo para que fuera O(1), y qué pierde B a cambio?

<details>
<summary>Mostrar respuesta</summary>

Un tercer puntero `padre_` en cada nodo. `padre(p)` pasa a ser O(1), pero B
deja de ser "dos punteros por nodo": paga 8 bytes más por nodo, y además
`insertarHijo` tiene que mantener ese puntero. Es la misma decisión que en el
tema 2 entre la lista simple y la doble: un puntero más por nodo a cambio de
poder ir hacia atrás.

</details>

### Recorridos

Un árbol general puede recorrerse en **profundidad** de dos formas:

- **Preorden**: primero la raíz y después, de izquierda a derecha, el preorden de cada subárbol.
- **Postorden**: primero, de izquierda a derecha, el postorden de cada subárbol, y al final la raíz.

También puede recorrerse en **anchura** (por niveles), usando una Cola.

El **inorden** no está bien definido en un árbol general: con tres hijos, ¿la
raíz se visita después del primero o después del segundo? Aparece en el
bloque 2, en el árbol binario, donde solo hay un hijo a cada lado.

### Ejercicio 1 — El mismo árbol, dos representaciones (completo, para leer)

**Antes de mirar el código:** `preorden` está escrito sobre la interfaz y,
desde cada nodo, baja a `primerHijo` y sigue a `hermanoDerecho`. ¿En qué dirección se aumenta la profundidad, yendo hacia el hijo o hacia el hermano?.
¿Cuál es cuál, y por qué recorrer así sigue dando un preorden correcto del
árbol *general* original?

<details>
<summary>Mostrar código (fragmento; completo en <code>01_arbol_general.cpp</code>)</summary>

```cpp
class Arbol {                       // el contrato: solo la interfaz
public:
    virtual Posicion primerHijo(Posicion p) const = 0;
    virtual Posicion hermanoDerecho(Posicion p) const = 0;
    virtual const std::string& etiqueta(Posicion p) const = 0;
    // ... raiz, padre, crearRaiz, insertarHijo
};

// Escrito UNA vez; sirve para ArbolLista y para ArbolPHHD.
void preorden(const Arbol& a, Posicion p, int profundidad, std::string& salida) {
    if (p == nullptr) return;
    salida += std::string(profundidad * 2, ' ') + a.etiqueta(p) + "\n";
    preorden(a, a.primerHijo(p), profundidad + 1, salida);   // baja un nivel
    preorden(a, a.hermanoDerecho(p), profundidad, salida);   // se queda en este nivel
}
```

</details>

<details>
<summary>Mostrar resultado</summary>

```text
=== El mismo arbol, dos representaciones ===
  (un sistema de ficheros pequenio: '/' con tres hijos)

  recien creados: vacio? A=true B=true  altura=-1  nodos=0

  -- preorden, representacion A (vector de hijos) --
/
  home
    user1
    user2
  etc
  var
    log
  -- preorden, representacion B (primerHijo/hermanoDerecho) --
/
  home
    user1
    user2
  etc
  var
    log
  iguales? true

=== Altura y numero de nodos: deben coincidir ===
  A: altura=2  nodos=7
  B: altura=2  nodos=7

=== padre("log"): mismo resultado, distinto coste ===
  A: var   (O(1): el nodo guarda su padre)
  B: var   (O(n): hay que buscarlo desde la raiz)

=== El precio en memoria de cada representacion ===
  sizeof(NodoLista) = 56 bytes  (+ el vector<NodoLista*> de cada nodo tiene su
                         PROPIO bloque reservado en el heap: capacidad, no solo tamanio)
  sizeof(NodoPHHD)  = 40 bytes  (siempre, tenga el nodo 0 o 100 hijos: dos punteros,
                         cero bloques adicionales)
```

</details>

**La respuesta:** `primerHijo` baja de nivel (es un hijo de verdad);
`hermanoDerecho` se queda en el mismo nivel (es otro hijo del mismo padre,
no un descendiente). El truco es que un hermano "a la derecha" en esta
representación es, en el árbol general, **otro hijo del mismo padre**: por
eso no cambia la profundidad. Este resultado es el que hay que llevarse de
todo el bloque: **cualquier árbol n-ario se puede representar como un árbol
binario sin perder nada**. Es el motivo por el que todo lo que viene a
partir de aquí (recorridos, búsqueda, equilibrado) se explica sobre árboles
*binarios* y sigue sirviendo para el caso general.

---

## 2. El árbol binario y sus recorridos

**El TAD Árbol Binario.** Es el TAD del bloque 1 con otra forma de navegar:
en lugar de `primerHijo(p)` y `hermanoDerecho(p)`, ahora `izquierdo(p)` y
`derecho(p)`. Lo que cambia de verdad es que el **árbol vacío** pasa a ser
obligatorio, porque forma parte de la definición, que es recursiva:

> Un árbol binario es, o bien el árbol **vacío**, o bien una **raíz** con dos
> subárboles binarios, izquierdo y derecho, cualquiera de ellos vacío.

Sin el árbol vacío no podríamos distinguir "50 con solo hijo izquierdo 80"
de "50 con solo hijo derecho 80", y son árboles **distintos**. En
`02_arbol_binario_recorridos.cpp` la posición es directamente un `Nodo*` y
la posición nula es `nullptr`, para centrarnos en los recorridos. Cada
`if (n == nullptr) return ...;` del código es el caso base de esa
definición. La clase que encapsula los nodos llega en el fichero 03.

Fijada la aridad a 2 (izquierdo y derecho, y son distintos), la primera
pregunta es cómo visitarlo entero. Hay tres formas en profundidad —misma
recursión, visita movida de sitio— y una en anchura:

| Recorrido         | Orden                       | Con qué se programa               |
| ----------------- | --------------------------- | ---------------------------------- |
| Preorden          | visitar, izquierdo, derecho | Recursión (o una pila explícita) |
| Inorden           | izquierdo, visitar, derecho | Recursión (o una pila explícita) |
| Postorden         | izquierdo, derecho, visitar | Recursión (o una pila explícita) |
| Por niveles (BFS) | nivel a nivel               | Una**cola** (tema 3)         |

**El malentendido más común del tema:** un árbol binario cualquiera **no**
tiene por qué estar ordenado. El árbol de ejemplo de `02_arbol_binario_recorridos.cpp`
no lo está a propósito, para que quede claro que el orden en el inorden no
es gratis: es una propiedad que se añade en el fichero 03, no algo que traiga
la forma "árbol binario" por sí sola.

### Ejercicio 2 — El inorden sin recursión

El tema 3 lo dijo sin demostrarlo con código: *"toda recursión se puede
convertir en un bucle con una pila explícita"*, porque una llamada recursiva
no es magia, es la pila de llamadas del programa. Aquí toca demostrarlo.

**Tu tarea:** en `02_arbol_binario_recorridos.cpp`, completar
`inordenIterativo()` con un `std::stack<Nodo*>`, sin ninguna llamada
recursiva para el siguiente árbol binario:

```c++
// El arbol de ejemplo (a proposito SIN ningun orden):
//
//                50
//              /    \
//            80      30
//           /  \        \
//         10    90       20
//                           \
//                            60
```

**Antes de escribir código:** el inorden recursivo baja por la izquierda
antes de visitar nada. ¿Qué papel juega la pila explícita en simular esa
bajada, y en qué momento exacto se "visita" un nodo (se guarda su valor)?

<details>
<summary>Mostrar código (fragmento; completo en <code>02_arbol_binario_recorridos.cpp</code>)</summary>

```cpp
void preorden(Nodo* n, std::vector<int>& salida) {
    if (n == nullptr) return;
    salida.push_back(n->valor);   // visitar...
    preorden(n->izq, salida);     // ...luego izquierdo...
    preorden(n->der, salida);     // ...luego derecho
}

// Por niveles: la Cola del tema 3, aplicada a un arbol.
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

// TU TAREA:
std::vector<int> inordenIterativo(Nodo* raiz) {
    std::vector<int> salida;
    // TODO: implementar con std::stack<Nodo*>, sin ninguna llamada recursiva.
    return salida;
}
```

</details>

<details>
<summary>Mostrar resultado (con la solución correcta)</summary>

```text
=== El arbol (SIN ningun orden todavia) ===
  altura=3  nodos=7

  preorden:   50 80 10 90 30 20 60 
  inorden:    10 80 90 50 30 20 60 
  postorden:  10 90 80 60 20 30 50 
  por niveles:50 80 30 10 90 20 60 

  Fijate en el inorden: NO sale ordenado (10 80 90 50 30 20 60).
  Es la prueba de que un arbol binario, por si solo, no tiene
  ninguna propiedad de orden. Eso es justo lo que anadimos en
  el fichero 03.

=== El mismo inorden, sin recursion ===
  inorden iterativo:10 80 90 50 30 20 60 
  coincide con el recursivo: true
```

</details>

**La idea, si te atascas:** baja siempre por la izquierda, apilando cada
nodo que dejas atrás (son visitas pendientes). Cuando ya no puedas bajar
más, desapila uno: ese es el siguiente en visitarse, porque ya no le queda
nada a la izquierda sin visitar. Visítalo, y repite el proceso partiendo de
su hijo **derecho**. 

---

## 3. El árbol binario de búsqueda (BST)

Añadimos una única propiedad al árbol binario: para todo nodo, **todo** lo
que cuelga de su subárbol izquierdo es menor, y **todo** lo que cuelga del
derecho es mayor. Es la generalización de la búsqueda binaria a una
estructura enlazada: en cada nodo se descarta la mitad del árbol sin
mirarla, aunque los datos no estén contiguos en memoria.

**Cambia el TAD, no solo la estructura.** En los bloques 1 y 2, quien usaba
el árbol **navegaba** por él: raíz, hijo izquierdo, padre... En el BST ya no.
Quien lo usa pide cosas **por valor** (`insertar(v)`, `buscar(v)`,
`borrar(v)`) y no ve ni un nodo. El TAD que se ofrece es un **Conjunto** (o
un **Diccionario**, si cada clave lleva un valor asociado), y el árbol es
solo su **implementación**. Por eso `ArbolBusqueda` esconde `Nodo` y `raiz_`
en su parte privada: las posiciones ya no forman parte del contrato. Es
exactamente lo que hace `std::set` (bloque 7).

| Operación      | Coste (árbol equilibrado)                                                                                                                          |
| --------------- | --------------------------------------------------------------------------------------------------------------------------------------------------- |
| `buscar(v)`   | O(log n)                                                                                                                                            |
| `insertar(v)` | O(log n)                                                                                                                                            |
| `borrar(v)`   | O(log n)                                                                                                                                            |
| `inorden()`   | O(n), y sale **ordenado**: es la propiedad BST leída al revés: todo lo del subárbol izquierdo  <  nodo  <  todo lo del subárbol derecho |

### Ejercicio 3 — Borrar: el caso que hay que pensar dos veces

**Tu tarea:** en `03_arbol_binario_busqueda.cpp`, completar `borrarEn()`.

**Antes de escribir código,** los tres casos, de menos a más difícil:

1. **Nodo hoja:** se quita sin más.
2. **Nodo con un solo hijo:** el nodo se "salta": ese hijo ocupa su lugar.
3. **Nodo con dos hijos:** no se puede quitar sin más (¿qué hueco dejaría?).
   Busca el **sucesor inorden** (el mínimo del subárbol derecho: el más
   pequeño de los que son mayores que el nodo), copia su valor a este nodo,
   y borra recursivamente ese sucesor de donde estaba.

**La pregunta que debes poder responder:** ¿por qué el borrado recursivo del
sucesor del caso 3 nunca vuelve a caer en el propio caso 3?

<details>
<summary>Mostrar código (fragmento; completo en <code>03_arbol_binario_busqueda.cpp</code>)</summary>

```cpp
static Nodo* insertarEn(Nodo* n, int valor) {
    if (n == nullptr) return new Nodo(valor);
    if (valor < n->valor)      n->izq = insertarEn(n->izq, valor);
    else if (valor > n->valor) n->der = insertarEn(n->der, valor);
    return n;
}

static Nodo* borrarEn(Nodo* n, int valor, bool& borrado) {
    // TODO: implementar los tres casos.
    return n;
}
```

</details>

<details>
<summary>Mostrar resultado (con la solución correcta)</summary>

```text
=== Insertar y buscar ===
  inorden (sale ordenado, gratis):20 30 40 50 60 70 80 
  buscar(40): true   buscar(45): false
  altura=2  nodos=7  es BST valido: true

=== Borrar: los tres casos ===
  borrar(20) [hoja]:             30 40 50 60 70 80 
  borrar(30) [un hijo]:          40 50 60 70 80 
  borrar(50) [dos hijos, raiz]:  40 60 70 80 
  altura=2  nodos=4  sigue siendo BST valido: true
```

</details>

**La respuesta:** el mínimo de un subárbol se encuentra bajando siempre por
la izquierda; por construcción, ese nodo **nunca tiene hijo izquierdo** (si
lo tuviera, ese hijo sería aún más pequeño). Así que al borrarlo, cae
siempre en el caso 1 (si tampoco tiene hijo derecho) o en el caso 2 (si
tiene solo el derecho): nunca hace falta un cuarto caso.

### El aviso: un BST solo es rápido si está equilibrado

`03_arbol_binario_busqueda.cpp` termina midiendo lo que pasa si los datos
llegan **ya ordenados**: cada nodo nuevo cuelga siempre del mismo lado, y el
árbol degenera en una cadena.

```text
=== El aviso: orden de insercion frente a altura ===
  n         altura (aleatorio)    altura (YA ordenado)
  100       11                    99
  1000      21                    999
  10000     30                    9999
  100000    39                    99999
```

Con datos al azar la altura crece proporcional a log n. Con datos ya
ordenados, la altura es **exactamente n − 1**: es una lista enlazada
disfrazada de árbol, y `buscar` vuelve a ser O(n). El fichero 05 (montículo)
resuelve el mismo problema de "garantizar log n" de otra forma; el fichero
06 (AVL) lo resuelve reparando directamente este árbol.

---

## 4. El montículo binario (la Cola de Prioridad)

Cambio de idea respecto a la búsqueda por valor. En la Cola de Prioridad el
orden de salida no lo decide la llegada ni una clave de búsqueda cualquiera:
lo decide el **valor de prioridad**, y sale siempre el más prioritario.

| Operación          | Precondición | Postcondición                                                 |
| ------------------- | ------------- | -------------------------------------------------------------- |
| `insertar(v)`     | —            | `v` está en la colección                                   |
| `maximo()`        | `!vacia()`  | Devuelve el mayor; no modifica                                 |
| `extraerMaximo()` | —            | Si no estaba vacía, elimina**una** aparición del mayor |

### Por qué las implementaciones evidentes no valen

| Implementación      | `insertar`   | `extraerMaximo` |
| -------------------- | -------------- | ----------------- |
| Lista sin ordenar    | O(1)           | **O(n)**    |
| Lista ordenada       | **O(n)** | O(1)              |
| **Montículo** | O(log n)       | O(log n)          |

El montículo no gana en ninguna columna por separado: gana cuando las dos
operaciones se usan mezcladas, que es siempre.

### La estructura: forma antes que orden

Un **montículo binario** es un árbol binario con dos propiedades:

1. **Completo:** todos los niveles llenos menos el último, que se rellena
   de izquierda a derecha. La altura es exactamente ⌊log₂ n⌋ — no es una
   esperanza estadística como en un árbol de búsqueda, **es una consecuencia
   de la forma**.
2. **Propiedad de montículo:** todo padre es ≥ que sus hijos. **No** es un
   árbol de búsqueda: entre hermanos no hay ningún orden, y buscar un valor
   cualquiera sigue costando O(n).

Al ser completo, el árbol no necesita punteros: cabe en un array, con
aritmética de índices.

```text
    padre(i)     = (i - 1) / 2
    izquierdo(i) = 2*i + 1
    derecho(i)   = 2*i + 2

          [50]                    array:  50  35  40  10  30  20
         /    \                   índice:  0   1   2   3   4   5
      [35]    [40]
      /  \      \
   [10] [30]   [20]
```

`insertar` coloca al final y **sube** mientras sea mayor que su padre
(*sift-up*, O(log n)). `extraerMaximo` trae el último elemento a la raíz y lo
deja **bajar** hasta su sitio, comparando con los dos hijos (*sift-down*,
O(log n)).

<details>
<summary>Mostrar resultado (<code>04_cola_prioridad_heap.cpp</code>)</summary>

```text
=== El array ES un arbol ===
  array:                [ 50 35 40 10 30 20 ]
    nivel 0: 50 
    nivel 1: 35 40 
    nivel 2: 10 30 20 
    es monticulo valido: true

=== Insertar 50 en un monticulo: el sift-up ===
  antes:                [ 40 30 35 10 20 ]
  tras insertar 50:     [ 50 30 40 10 20 35 ]
    50 entro al final y subio hasta la raiz: 2 intercambios,
    no 5 desplazamientos. Ahi esta el log n.

=== Monticulo frente a lista ordenada ===
  n         heap (comp+int)   ordenada (ops)    razon
  1000      15974             499500            31.3x
  2000      35928             1999000           55.6x
  4000      79834             7998000           100.2x
  8000      175644            31996000          182.2x
```

</details>

**Lo que un montículo no sabe hacer:** buscar un valor cualquiera (O(n): no
es un árbol de búsqueda), recorrer en orden (hay que vaciarlo), o cambiar la
prioridad de un elemento concreto sin saber dónde está.

### Construcción en O(n), heapsort y top-k

El fichero `05_heapify_heapsort.cpp` aprovecha el montículo para tres cosas
distintas: construirlo de golpe a partir de un array, ordenar con él
(heapsort) y quedarse con los k mayores de una colección enorme (top-k). Las
tres usan solo las dos operaciones del apartado anterior: `subir` (*sift-up*)
y `bajar` (*sift-down*).

#### (A) Construir un montículo a partir de un array

Tenemos un array desordenado con n elementos y queremos convertirlo en
montículo. Hay dos formas de hacerlo.

**Forma 1: insertar uno a uno.** Se empieza con un montículo vacío y se
inserta cada elemento: se coloca al final y se hace `subir`. Cada inserción
cuesta como mucho la altura, log n, así que el total es **O(n log n)**.

**Forma 2: el método de Floyd (1964).** Se basa en dos observaciones:

1. **La forma ya está bien.** Un array de n elementos, leído con
   `izquierdo(i) = 2i+1` y `derecho(i) = 2i+2`, ya es un árbol completo. Lo
   único que falla es el **orden** entre padres e hijos.
2. **Las hojas ya son montículos.** Una hoja no tiene hijos, así que no
   incumple nada. En un árbol completo, las hojas son toda la segunda mitad
   del array (índices `n/2` a `n−1`), así que la mitad del trabajo ya está
   hecha sin mover nada.

Así que se recorren solo los nodos que tienen hijos, **desde el último padre
(`n/2 − 1`) hacia la raíz (0)**, y en cada uno se llama a `bajar`:

```cpp
for (int i = n / 2 - 1; i >= 0; i--) bajar(v, i, n);
```

**Un ejemplo a mano** con `[10 20 30 40 50 60 70]` (n = 7, padres en los
índices 2, 1 y 0):

```text
  inicial:          bajar(2): 30 baja      bajar(1): 20 baja      bajar(0): 10 baja dos veces

     10                 10                     10                     70
   /    \             /    \                 /    \                 /    \
  20     30          20     70              50     70              50     60
 / \    / \         / \    / \             / \    / \             / \    / \
40 50  60 70       40 50  60 30           40 20  60 30           40 20  10 30

  array: 10 20 30 40 50 60 70  ->  10 20 70 40 50 60 30  ->  10 50 70 40 20 60 30  ->  70 50 60 40 20 10 30
```

**Pregunta para pensar:** ¿por qué hay que recorrer de abajo arriba (de
`n/2 − 1` hacia 0) y no de la raíz hacia abajo?

<details>
<summary>Mostrar respuesta</summary>

`bajar(i)` solo funciona si los dos subárboles de `i` **ya son montículos**:
compara el nodo con sus dos hijos, se intercambia con el mayor y sigue
bajando, dando por hecho que por debajo todo está en orden. Recorriendo de
abajo arriba, cuando se llega a `i` sus dos hijos ya se han procesado (tienen
índices mayores) y la condición se cumple. Si se empezara por la raíz, sus
subárboles aún estarían desordenados y el resultado no sería un montículo.

</details>

**¿Por qué Floyd cuesta O(n) y no O(n log n)?** El coste de `bajar` desde un
nodo no es log n, sino **la altura de ese nodo**, es decir, cuántos niveles
tiene por debajo. En un árbol completo, casi todos los nodos están abajo:

| Nodos de esa altura | Altura (máximo de pasos al bajar) | Trabajo en ese nivel |
| ------------------- | --------------------------------- | -------------------- |
| n/2 (las hojas)     | 0                                 | 0                    |
| n/4                 | 1                                 | n/4                  |
| n/8                 | 2                                 | 2·n/8 = n/4          |
| n/16                | 3                                 | 3·n/16               |
| ...                 | ...                               | ...                  |
| 1 (la raíz)         | log n                             | log n                |

La suma es n · (1/4 + 2/8 + 3/16 + 4/32 + …). Esa serie converge a 1, sea
cual sea n, así que el total es **como mucho del orden de n**.

Con la inserción uno a uno pasa justo lo contrario. Lo que cuesta es `subir`,
y la distancia que recorre un nodo al subir es su **profundidad**. La mitad
de los nodos son hojas, con profundidad ≈ log n: los nodos que más pueden
subir son precisamente los más numerosos. Floyd deja el trabajo caro para los
pocos nodos de arriba, y la inserción se lo asigna a los muchos de abajo.

**Una advertencia al leer las medidas.** O(n log n) es el **peor caso** de
insertar uno a uno, y no siempre se ve. Por eso el programa mide dos tandas:

- **Datos aleatorios:** un elemento recién insertado casi nunca sube hasta
  arriba, así que la columna `ops/n` de "insertando" apenas cambia (≈ 3,5)
  y parece lineal.
- **Datos en orden creciente:** cada elemento nuevo es el mayor hasta el
  momento y sube hasta la raíz. Ahí `ops/n` de "insertando" **crece**: suma
  unas 4 operaciones cada vez que n se multiplica por 4 (log₂ 4 = 2 niveles
  más, con 2 operaciones por nivel). Ese es el factor log n.

En las dos tandas, Floyd se queda clavado (≈ 2,6 con datos al azar, 3 con
datos en orden): su coste es lineal sean cuales sean los datos.

#### (B) Heapsort: ordenar con el montículo

Una vez construido el montículo de máximos, el mayor está en `v[0]`. La idea
es ir sacando el máximo una y otra vez y colocarlo **al final del propio
array**, en el hueco que deja el montículo al encoger. El array queda partido
en dos zonas:

```text
  [ montículo (aún por ordenar) | ya ordenado, en su sitio definitivo ]
```

En cada paso: (1) se intercambia `v[0]` (el máximo) con el último elemento
del montículo, (2) el montículo encoge en uno, de modo que ese máximo pasa a
la zona ordenada, y (3) se hace `bajar(0)` para arreglar la raíz, que ahora
tiene un valor cualquiera. Siguiendo con el montículo del ejemplo:

```text
  70 50 60 40 20 10 30 |              intercambiar 70 <-> 30, bajar(0)
  60 50 30 40 20 10 | 70              intercambiar 60 <-> 10, bajar(0)
  50 40 30 10 20 | 60 70              ...
  ...
  | 10 20 30 40 50 60 70              ordenado
```

Por eso se usa un montículo de **máximos** para ordenar de menor a mayor: el
máximo es el que tiene que ir al final.

**Coste:** la construcción es O(n) y luego hay n − 1 pasos de O(log n) cada
uno, así que el total es **O(n log n), siempre**. No tiene un peor caso malo
como quicksort, y no usa memoria extra: todo ocurre dentro del mismo array.

**¿Por qué entonces `std::sort` no es un heapsort?** Por la caché. `bajar`
salta de `i` a `2i+1`, luego a `4i+3`, y así sucesivamente: cada salto aterriza
en una zona lejana del array, y el procesador tiene que traerla de memoria.
Quicksort, en cambio, recorre el array de forma secuencial. Con el mismo
O(n log n), heapsort tarda entre 1,5 y 2 veces más. Aun así, el heapsort no se tira:
`std::sort` es un *introsort*, un quicksort que **cambia a heapsort** si
detecta que va camino de su peor caso O(n²), precisamente porque heapsort lo
garantiza.

#### (C) Top-k: los k mayores sin ordenarlo todo

**El problema:** de n elementos (pongamos 5 millones de ventas) se quieren
solo los k mayores (pongamos los 10 productos más vendidos). Ordenarlo todo
cuesta O(n log n) y exige tener los n elementos en memoria, solo para
quedarse con 10.

**La idea:** recorrer los datos una sola vez, guardando los **k mejores
vistos hasta ahora**. Cada vez que llega un elemento nuevo, la pregunta es:
¿es mejor que el **peor** de los que tengo guardados? Si lo es, el peor sale
y el nuevo entra. Para responder rápido hay que tener ese "peor de los
mejores" siempre a mano, y eso es exactamente la raíz de un **montículo de
mínimos**. De ahí la paradoja aparente: para quedarse con los **mayores** se
usa un montículo de **mínimos**. La raíz funciona como el listón para entrar.

```cpp
for (int x : v) {
    if (mejores.size() < k)        mejores.push(x);            // aún hay sitio
    else if (x > mejores.top()) {  mejores.pop(); mejores.push(x); }  // supera el listón
}
```

**Un ejemplo** con k = 3 y los datos `5 1 8 3 9 2 7`:

```text
  llega   montículo (raíz = listón)   qué pasa
  5       {5}                         hay sitio
  1       {1, 5}                      hay sitio
  8       {1, 5, 8}     listón 1      hay sitio; ya está lleno
  3       {3, 5, 8}     listón 3      3 > 1: sale el 1, entra el 3
  9       {5, 8, 9}     listón 5      9 > 3: sale el 3, entra el 9
  2       {5, 8, 9}     listón 5      2 < 5: se descarta sin tocar nada
  7       {7, 8, 9}     listón 7      7 > 5: sale el 5, entra el 7
                                      -> los 3 mayores: 9 8 7
```

Con un montículo de máximos, la raíz sería el **mejor** guardado, que es
justo el que no hay que expulsar, y encontrar el peor costaría O(k).

**Coste:** cada elemento cuesta como mucho O(log k), porque el montículo
nunca pasa de k elementos, así que el total es **O(n log k)**, y la memoria
es **O(k)**. Con k = 10, log k es poco más de 3. Además, cada dato se mira
una sola vez y se descarta, así que funciona aunque los datos lleguen como un
flujo y no quepan en memoria (un registro de eventos, una consulta a una
base de datos…).

<details>
<summary>Mostrar resultado (<code>05_heapify_heapsort.cpp</code>)</summary>

```text
=== (A) Construir el monticulo: O(n log n) frente a O(n) ===
  Datos ALEATORIOS (caso medio):
  n           insertando      ops/n     Floyd           ops/n
  10000       35286           3.53      26213           2.62
  40000       143436          3.59      105363          2.63
  160000      569971          3.56      419844          2.62
  640000      2275377         3.56      1679244         2.62
  2560000     9120127         3.56      6720599         2.63

  Datos en ORDEN CRECIENTE (peor caso de insertar):
  n           insertando      ops/n     Floyd           ops/n
  10000       227262          22.73     29974           3.00
  40000       1068962         26.72     119968          3.00
  160000      4915750         30.72     479962          3.00
  640000      22222890        34.72     1919956         3.00
  2560000     99131438        38.72     7679950         3.00

=== (B) Heapsort ===
  antes:   95  84  92  1   21  67  38  6   97  26  70  54
  despues: 1   6   21  26  38  54  67  70  84  92  95  97
  ordenado: true, sin memoria extra

  Comparacion con std::sort (introsort), mismo O(n log n):
    n=200000   heapsort 13.2    ms   std::sort 8.8     ms   (x1.5)  [mismo resultado]
    n=400000   heapsort 28.4    ms   std::sort 18.7    ms   (x1.5)  [mismo resultado]
    n=800000   heapsort 63.7    ms   std::sort 39.1    ms   (x1.6)  [mismo resultado]

=== (C) Top-K: no ordenes lo que no necesitas ===
  n = 5000000, k = 10
    monticulo de k:  2.7 ms   (memoria extra: 10 enteros)
    ordenar todo:    274.8 ms   (memoria extra: 5000000 enteros)
    coinciden con ordenar: true
```

</details>

Para quedarse con tres ideas:

- **(A)** "n operaciones de O(log n) cada una" da una cota **cierta**, pero
  no siempre **ajustada**. Si la mayoría de esas operaciones cuestan mucho
  menos que el peor caso, el total puede ser menor. La columna `ops/n` de
  Floyd se queda clavada aunque n se multiplique por 256, y da igual que
  los datos lleguen al azar o en orden.
- **(B)** Dos algoritmos con el mismo orden de complejidad pueden tener
  constantes muy distintas. La caché cuenta.
- **(C)** No hay que ordenar lo que no se necesita: O(n log k) con memoria
  O(k) frente a O(n log n) con memoria O(n). En el ejemplo, unas 100 veces más
  rápido.

---

## 5. El AVL: reparar el BST en vez de cambiar de estructura

El montículo garantiza O(log n) **renunciando** a la búsqueda por valor. El
AVL (Adelson-Velskii y Landis, 1962) elige la otra salida: mantener el orden
total del BST, y **reparar** el desequilibrio en el momento en que aparece,
con una rotación.

**El invariante:** para todo nodo, `|altura(der) − altura(izq)| ≤ 1`.

### Los cuatro casos, y por qué solo hacen falta dos rotaciones

| Caso | Diagnóstico                                                                     | Arreglo                                                                         |
| ---- | -------------------------------------------------------------------------------- | ------------------------------------------------------------------------------- |
| LL   | Pesa la izquierda, y el problema está a la izquierda de esa izquierda           | Una rotación simple a la derecha                                               |
| RR   | Simétrico                                                                       | Una rotación simple a la izquierda                                             |
| LR   | Pesa la izquierda, pero el problema está a la**derecha** de esa izquierda | Rotar el hijo izquierdo a la izquierda (se convierte en LL), y luego aplicar LL |
| RL   | Simétrico                                                                       | Rotar el hijo derecho a la derecha (se convierte en RR), y luego aplicar RR     |

### Ejercicio 4 — La rotación doble que falta

**Tu tarea:** en `06_arbol_avl.cpp`, el caso RL está completo y sirve de
plantilla. Completa el caso **LR**, simétrico.

**Antes de escribir código:** dibuja a mano el resultado de insertar
`30, 10, 20` (en ese orden) sin ninguna rotación. ¿Qué nodo queda
desequilibrado, y hacia qué lado pesa? ¿Basta una rotación simple a la
derecha del nodo raíz para arreglarlo?

<details>
<summary>Mostrar código (fragmento; completo en <code>06_arbol_avl.cpp</code>)</summary>

```cpp
static Nodo* reequilibrar(Nodo* n) {
    int fe = factorEquilibrio(n);

    if (fe < -1) {
        if (factorEquilibrio(n->izq) <= 0) {
            return rotarDerecha(n);          // caso LL: completo
        } else {
            // TODO: caso LR. Dos lineas, simetricas al caso RL de abajo:
            //   n->izq = rotarIzquierda(n->izq);
            //   return rotarDerecha(n);
            return n;
        }
    }
    if (fe > 1) {
        if (factorEquilibrio(n->der) >= 0) {
            return rotarIzquierda(n);        // caso RR: completo
        } else {
            n->der = rotarDerecha(n->der);   // caso RL: completo (la plantilla)
            return rotarIzquierda(n);
        }
    }
    return n;
}
```

</details>

<details>
<summary>Mostrar resultado (con la solución correcta)</summary>

```text
=== Los cuatro casos de desequilibrio ===
  LL:   insertar 30 20 10  -> altura=1  valido=true   inorden: 10 20 30 
  RR:   insertar 10 20 30  -> altura=1  valido=true   inorden: 10 20 30 
  RL:   insertar 10 30 20  -> altura=1  valido=true   inorden: 10 20 30 
  LR:   insertar 30 10 20  -> altura=1  valido=true   inorden: 10 20 30 

=== El aviso del fichero 03, arreglado ===
  n         altura AVL    cota teorica 1.44*log2(n)
  100       6             9.6
  1000      9             14.4
  10000     13            19.1
  100000    16            23.9
```

</details>

**La respuesta:** insertar `30, 10, 20` deja el nodo raíz (30) con factor de
equilibrio −2 (pesa la izquierda), pero el 20 —el que rompe el equilibrio—
cuelga a la **derecha** del 10, no a su izquierda. Una sola rotación a la
derecha de 30 subiría al 10 pero dejaría al 20 colgando del lado que no
toca: por eso hace falta primero enderezar esa rama (rotando el 10 a la
izquierda) antes de aplicar la rotación de siempre. Con el ejercicio
completo, insertando **en orden** —el peor caso posible del fichero 03,
donde la altura se disparaba a n— la altura del AVL se queda siempre por
debajo de 1,44 log₂(n): es la cota que demostraron Adelson-Velskii y Landis,
y nunca se supera, insertes en el orden que insertes.

---

## 6. El B-tree: cuando el acceso es caro

Todo lo anterior asume que comparar dos claves es barato: memoria RAM,
nanosegundos. Si el árbol vive en **disco** (un índice de base de datos, un
sistema de ficheros), cada nivel que se baja es un acceso a disco, órdenes
de magnitud más lento que una comparación. Lo que hay que minimizar ya no es
el número de comparaciones: es el número de **nodos visitados**.

La solución del B-tree (Bayer y McCreight, 1972): romper la aridad 2. Cada
nodo guarda **muchas** claves ordenadas y tiene **muchos** hijos. Con un
"grado mínimo" t, cada nodo tiene entre t−1 y 2t−1 claves; con t = 100, un
millón de claves cabe en 2 o 3 niveles.

```text
        ejemplo con t=2 (entre 1 y 3 claves por nodo):

                    [ 30 | 60 ]
                   /     |     \
            [10|20]   [40|50]  [70|80|90]
```

### Ejercicio 5 — El split

Un nodo nunca puede superar 2t−1 claves. Si al insertar se pasa (llega a
2t), se **parte** en dos: la clave del medio sube al padre como nueva clave
separadora.

**Tu tarea:** en `07_arbol_b.cpp`, completar `dividirSiHaceFalta()`.

**Antes de escribir código:** un nodo con 2t claves, `[c0 ... c(2t-1)]`. ¿Qué
índice es "el del medio"? ¿Cuántas claves quedan a su izquierda, y cuántas a
su derecha, y coincide eso con los límites `t-1` y `2t-1` que exige el
invariante?

<details>
<summary>Mostrar código (fragmento; completo en <code>07_arbol_b.cpp</code>)</summary>

```cpp
bool insertarEn(Nodo* n, int valor, int& claveQueSube, Nodo*& hermanoDerecho) {
    if (n->esHoja) {
        // insertar 'valor' en su sitio ordenado dentro de n->claves
    } else {
        // bajar al hijo que toca; si ese hijo se partio, insertar la
        // clave promovida y el nuevo hermano en ESTE nodo
    }
    return dividirSiHaceFalta(n, claveQueSube, hermanoDerecho);
}

bool dividirSiHaceFalta(Nodo* n, int& claveQueSube, Nodo*& hermanoDerecho) {
    if (static_cast<int>(n->claves.size()) < 2 * t_) return false;
    // TODO: la clave del indice t_ sube; el resto se reparte en dos nodos
    // (y sus hijos, si n no es hoja) y se devuelve true.
    return false;
}
```

</details>

<details>
<summary>Mostrar resultado (con la solución correcta)</summary>

```text
=== Insertar y buscar, con grado minimo t=2 (1 a 3 claves por nodo) ===
  inorden (sale ordenado, gratis):10 20 30 40 50 60 70 80 90 
  buscar(50): true   buscar(55): false
  niveles=1  claves totales=9  valido=true

=== Un ejemplo algo mayor, para ver crecer los niveles ===
  40 claves con t=2: niveles=3  claves totales=40  valido=true
```

</details>

**La respuesta:** con `n->claves.size() == 2*t_`, el índice del medio es
`t_` (0-indexado). A su izquierda quedan `t_` claves y a su derecha
`t_ - 1`: las dos cantidades caen dentro de `[t_-1, 2*t_-1]`, así que el
invariante se mantiene en los dos nodos resultantes sin ningún caso
especial. Si el nodo no es hoja, los hijos se reparten con el mismo corte,
desplazado una posición (`t_+1` a la izquierda, el resto a la derecha),
porque un nodo con k claves siempre tiene k+1 hijos.

La comparación a gran escala (por qué un grado grande compensa frente a un
AVL cuando cada nivel es un acceso a disco) está medida en
`2_profesor/07_arbol_b_resuelta.cpp`, con el ejercicio ya resuelto: en el
fichero del estudiante, ejecutarla sin el split implementado no mediría
nada (sin partir nunca ningún nodo, todo el árbol es un único nodo
gigante).

---

## 7. Los adaptadores de la STL: los TADs del tema, hechos tipo

Igual que en el tema 3 `std::stack`/`std::queue` eran la Pila y la Cola
convertidas en mecanismo del lenguaje, aquí `std::priority_queue` es el
montículo del fichero 04, y `std::set`/`std::map` son un árbol de búsqueda
equilibrado —un **árbol rojo-negro**, no un AVL— convertido en tipo.

<details>
<summary>Mostrar resultado (<code>08_adaptadores_stl.cpp</code>)</summary>

```text
=== 2) set/map: el arbol de busqueda que NO degenera ===
  insertados 0..14 EN ORDEN (el peor caso del BST simple)
  recorrido con iterador (siempre en orden, gratis): 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 

  Medido: BST simple (fichero 03) frente a std::set, insertando EN ORDEN
  n         set::insert (ms)  altura simulada
  1000      0.110             ~19 (cota rojo-negro: 2*log2(n))
  80000     7.645             ~32 (cota rojo-negro: 2*log2(n))
  Nunca se dispara, a diferencia del BST simple del fichero 03:
  ahi insertar en orden daba una CADENA de altura n.
```

</details>

### La tabla final del tema

| TAD               | En la STL                   | Contenedor/estructura por debajo         | Por qué                                                                                                                                                                          |
| ----------------- | --------------------------- | ---------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Cola de prioridad | `std::priority_queue`     | Montículo binario, sobre`std::vector` | Necesita acceso indexado`i -> 2i+1` y memoria contigua                                                                                                                          |
| Conjunto ordenado | `std::set`                | Árbol rojo-negro                        | Recorrido en orden barato + O(log n) garantizado sin cota tan estricta como un AVL                                                                                                |
| Diccionario       | `std::map`                | Árbol rojo-negro de pares               | Igual que`set`, con clave y valor                                                                                                                                               |
| B-tree            | (no está en la STL de C++) | —                                       | Solo compensa cuando cada nodo es un acceso a disco, no a memoria: es cosa de motores de bases de datos y sistemas de ficheros, no de una biblioteca de propósito general en RAM |

El porqué de que `std::set`/`std::map` usen un árbol rojo-negro y no un AVL
—una cota de altura algo más floja a cambio de rotaciones acotadas en
inserción y borrado— está explicado en `2_profesor/11_arbol_rojinegro.md`.

### Y fuera de clase: cómo se programan los árboles en C++

En este tema hemos usado `new`, `delete` y punteros crudos a propósito,
igual que en el tema 2: así se ve el mecanismo. En código real se hace de
otra manera:

1. **La STL no trae un tipo "árbol" genérico.** Trae **TADs implementados
   con árboles**, que es la idea del bloque 3. `std::set` y `std::map` son
   árboles rojinegros. En la implementación de GCC, cada nodo guarda padre,
   hijo izquierdo, hijo derecho y color, y el árbol tiene un **nodo
   cabecera** que hace de centinela, como el de la lista circular del tema 2.
   Las "posiciones" son los **iteradores**: `++it` avanza en inorden
   subiendo por el puntero al padre. `std::priority_queue` es el montículo
   del bloque 4 sobre un `std::vector`.
2. **Árbol general con propiedad automática:** cada nodo guarda
   `std::vector<std::unique_ptr<Nodo>> hijos` y, si hace falta, un
   `Nodo* padre` que *no* es dueño del padre. Los `unique_ptr` liberan el
   árbol solos, así que no hace falta escribir `liberar`. Es la
   representación A del bloque 1 con la memoria gestionada automáticamente,
   y es como se programan los árboles sintácticos de un compilador, el DOM
   de un navegador o el grafo de escena de un videojuego.
3. **Árbol en un array, con índices:** todos los nodos en un
   `std::vector<Nodo>`, y cada nodo guarda los **índices** (`int`) de sus
   hijos en vez de punteros. No hace un `new` por nodo y aprovecha la caché.
   Es la generalización de la idea del montículo (bloque 4), y la usan
   motores de juego, compiladores y bases de datos.
4. **Bibliotecas:** Boost.PropertyTree para jerarquías tipo JSON/XML, y
   Boost.Graph cuando la estructura deja de ser un árbol.

---

## Resumen

- El **TAD Árbol** describe una jerarquía (raíz, padre, hijos ordenados) sin
  decir cómo se guarda. El árbol general tiene al menos la raíz; en el árbol
  binario, el árbol vacío forma parte de la definición.
- Un **árbol n-ario** se puede representar siempre como un árbol binario
  (primer-hijo/hermano-derecho): por eso todo el tema, a partir de aquí, se
  explica sobre árboles binarios sin perder generalidad.
- Un **árbol binario** por sí solo no tiene ningún orden: los recorridos son
  propiedades de la **forma**, no del contenido. El inorden solo da una
  secuencia ordenada cuando, además, el árbol cumple la propiedad BST.
- El **BST** hace la búsqueda O(log n)... si está equilibrado. Insertar ya
  ordenado lo degenera en una cadena de altura n: la garantía depende del
  orden de inserción, y eso es justo lo que los dos bloques siguientes
  arreglan, cada uno a su manera.
- El **montículo** garantiza O(log n) por **construcción de la forma**
  (árbol siempre completo), renunciando a la búsqueda por valor. Construirlo
  cuesta O(n), no O(n log n): la mayoría de los nodos tienen altura
  diminuta.
- El **AVL** repara el mismo problema que rompía al BST, con rotaciones
  acotadas por inserción: nunca más de 1,44 log₂(n) de altura, insertes en
  el orden que insertes.
- El **B-tree** generaliza a nodos con muchas claves: cuando cada nivel es
  un acceso a disco, minimizar el número de niveles importa más que
  minimizar comparaciones. Es la estructura real detrás de los índices de
  bases de datos y los sistemas de ficheros.
- Los **adaptadores de la STL** convierten cada TAD del tema en un tipo del
  lenguaje: `priority_queue` es el montículo; `set`/`map` son un árbol
  rojo-negro, no un AVL, porque la STL prioriza rotaciones acotadas en la
  modificación sobre la cota de altura más ajustada.
