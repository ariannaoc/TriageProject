#include <iostream>
#include <avltree.hpp>

using namespace Designar;

int main() {
    AVLTree<int_t> arbolPrueba;
    arbolPrueba.insert(10);
    arbolPrueba.insert(5);
    arbolPrueba.insert(20);

    std::cout << "¡DeSiGNAR funciona correctamente! Elementos en el arbol AVL: " << arbolPrueba.size() << std::endl;
    return 0;
}