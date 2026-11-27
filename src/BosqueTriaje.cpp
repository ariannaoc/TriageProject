#include "BosqueTriaje.hpp"

#include <set>

namespace
{
using Nodo = BosqueTriaje::Nodo;

int sumarSeleccionadas(const Nodo& nodo, const std::set<int>& ids)
{
    if (nodo.esHoja)
        return ids.count(nodo.dato.id_opcion) ? nodo.dato.base_points : 0;

    int total = 0;
    for (Designar::nat_t i = 0; i < nodo.hijos.size(); ++i)
    {
        const auto& hijo = nodo.hijos[i];
        total += sumarSeleccionadas(*hijo, ids);
    }
    return total;
}
<<<<<<< HEAD

void recolectarOpciones(const Nodo& nodo, std::vector<OpcionSintoma>& opciones)
{
    if (nodo.dato.id_opcion >= 0)
    {
        opciones.push_back({nodo.dato.id_opcion, nodo.textoPregunta,
                            nodo.dato.base_points, nodo.dato.nivel_estimado});
    }
    for (Designar::nat_t i = 0; i < nodo.hijos.size(); ++i)
    {
        recolectarOpciones(*nodo.hijos[i], opciones);
    }
}
} 
=======
}  // namespace
>>>>>>> main

void BosqueTriaje::construirDesdeCategorias(const std::vector<CategoriaTriaje>& categorias)
{
    arboles_.clear();
    nombres_.clear();

    for (const auto& categoria : categorias)
    {
        nombres_[categoria.id] = categoria.nombre;

        // Raíz: pregunta de filtro dicotómica.
        auto raiz = std::make_unique<Nodo>(
            DatoNodoTriaje{}, "¿Presenta " + categoria.nombre + "?", false);

        raiz->agregarHijo(std::make_unique<Nodo>(DatoNodoTriaje{}, "No", true));

        Nodo* opcionMultiple = raiz->agregarHijo(
            std::make_unique<Nodo>(DatoNodoTriaje{}, "Sí: seleccione los síntomas", false));

        for (const auto& opcion : categoria.opciones)
        {
            DatoNodoTriaje dato;
            dato.id_opcion      = opcion.id;
            dato.base_points    = opcion.base_points;
            dato.nivel_estimado = opcion.nivel_estimado;
            opcionMultiple->agregarHijo(std::make_unique<Nodo>(dato, opcion.etiqueta, true));
        }

        arboles_[categoria.id] = std::move(raiz);
    }
}

int BosqueTriaje::evaluarCategoria(int idCategoria,
                                   const std::vector<int>& opcionesSeleccionadas) const
{
    if (opcionesSeleccionadas.empty())
        return 0;

    const auto it = arboles_.find(idCategoria);
    if (it == arboles_.end())
        return 0;

    const std::set<int> ids(opcionesSeleccionadas.begin(), opcionesSeleccionadas.end());
    return sumarSeleccionadas(*it->second, ids);
}

bool BosqueTriaje::tieneSintomasSeleccionados(
    const std::map<int, std::vector<int>>& seleccionPorCategoria) const
{
    for (const auto& [idCategoria, opciones] : seleccionPorCategoria)
        if (!opciones.empty())
            return true;
    return false;
}

std::string BosqueTriaje::nombreCategoria(int idCategoria) const
{
    const auto it = nombres_.find(idCategoria);
    return it == nombres_.end() ? "" : it->second;
}
<<<<<<< HEAD

std::vector<int> BosqueTriaje::categorias() const
{
    std::vector<int> ids;
    ids.reserve(arboles_.size());
    for (const auto& [id, arbol] : arboles_)
    {
        (void)arbol;
        ids.push_back(id);
    }
    return ids;
}

std::vector<OpcionSintoma> BosqueTriaje::opcionesCategoria(int idCategoria) const
{
    std::vector<OpcionSintoma> opciones;
    const auto it = arboles_.find(idCategoria);
    if (it == arboles_.end())
        return opciones;

    recolectarOpciones(*it->second, opciones);
    return opciones;
}
=======
>>>>>>> main
