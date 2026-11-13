#pragma once

// =============================================================================
// BosqueTriaje.hpp
// Bosque de 7 árboles de decisión (uno por categoría de symptoms.json).
//
// DECISIÓN DE DISEÑO: DeSiGNAR en el bosque de decisión
// Designar::Tree (tree.hpp) es un BST con semántica de conjunto: no modela
// un árbol n-ario de decisión. Se usa NodoArbol con Designar::DynArray para
// los hijos, y Designar::QuadTree para el enrutamiento espacial.
// =============================================================================

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <array.hpp>

#include "modelos_triaje.hpp"

template <typename T>
class NodoArbol
{
public:
    T           dato;
    std::string textoPregunta;
    bool        esHoja;
    Designar::DynArray<std::unique_ptr<NodoArbol<T>>> hijos;

    NodoArbol(T datoInicial, std::string texto, bool hoja)
        : dato(std::move(datoInicial)), textoPregunta(std::move(texto)), esHoja(hoja) {}

    NodoArbol<T>* agregarHijo(std::unique_ptr<NodoArbol<T>> hijo)
    {
        return hijos.append(std::move(hijo)).get();
    }

    void recorrer(const std::function<void(const NodoArbol<T>&, int)>& visitante,
                  int profundidad = 0) const
    {
        visitante(*this, profundidad);
        for (Designar::nat_t i = 0; i < hijos.size(); ++i)
        {
            const auto& hijo = hijos[i];
            hijo->recorrer(visitante, profundidad + 1);
        }
    }
};

struct DatoNodoTriaje
{
    int         id_opcion      = -1;
    int         base_points    = 0;
    NivelTriaje nivel_estimado = NivelTriaje::V;
};

class BosqueTriaje
{
public:
    using Nodo = NodoArbol<DatoNodoTriaje>;

    void construirDesdeCategorias(const std::vector<CategoriaTriaje>& categorias);
    int evaluarCategoria(int idCategoria, const std::vector<int>& opcionesSeleccionadas) const;
    bool tieneSintomasSeleccionados(const std::map<int, std::vector<int>>& seleccionPorCategoria) const;
    std::size_t numeroArboles() const { return arboles_.size(); }

    /// Devuelve el nombre de la categoría ("" si no existe). Necesario para
    /// que ResultadoTriaje.categoria_ganadora tenga un nombre legible en la UI.
    std::string nombreCategoria(int idCategoria) const;

private:
    std::map<int, std::unique_ptr<Nodo>> arboles_;
    std::map<int, std::string> nombres_;
};
