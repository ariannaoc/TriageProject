#pragma once

// =============================================================================
// GestorCentros.hpp
// Carga centros de salud en un Designar::QuadTree y resuelve consultas K-NN
// por radio expansivo (el QuadTree de DeSiGNAR no tiene K-NN nativo, solo
// query_range con un cuadrado).
// =============================================================================

#include <map>
#include <vector>

#include <array.hpp>
#include <point2D.hpp>
#include <quadtree.hpp>

#include "modelos_triaje.hpp"

/// Wrapper para Designar::QuadTree: asocia el id del centro a un Point2D.
/// QuadTree<PointT> requiere get_x(), get_y() y el typedef NumberType.
struct CentroPunto
{
    Designar::Point2D punto;
    int id_centro = -1;

    using NumberType = Designar::real_t;

    // QuadTree::subdivide() construye los centros de los cuadrantes hijos
    // con PointT(x, y); por eso es OBLIGATORIO tener un constructor de dos
    // NumberType. El id_centro de esos puntos de frontera es -1 (no son
    // datos reales, solo centros geométricos del árbol).
    CentroPunto() = default;
    CentroPunto(NumberType x, NumberType y) : punto(x, y), id_centro(-1) {}
    CentroPunto(const Designar::Point2D& p, int id = -1) : punto(p), id_centro(id) {}

    Designar::real_t get_x() const { return punto.get_x(); }
    Designar::real_t get_y() const { return punto.get_y(); }
};

class GestorCentros
{
public:
    GestorCentros();
    ~GestorCentros();

    // No copiable: posee el QuadTree con puntero crudo.
    GestorCentros(const GestorCentros&) = delete;
    GestorCentros& operator=(const GestorCentros&) = delete;

    /// Construye el QuadTree con todos los centros (boundary calculado a partir
    /// de sus coordenadas + margen) y guarda el mapa id -> CentroSalud.
    void cargarCentros(const std::vector<CentroSalud>& centros);

    /// Devuelve hasta K centros con nivel_complejidad >= complejidadMinima,
    /// ordenados por distancia euclidiana al paciente. Usa radio expansivo:
    /// 5 km, 10 km, 20 km, ..., 200 km.
    std::vector<CentroSalud> obtenerCercanos(double lat, double lng, int k,
                                            int complejidadMinima) const;

private:
    std::map<int, CentroSalud>            centros_;
    Designar::QuadTree<CentroPunto>*      quadtree_ = nullptr;
    Designar::QuadBoundary<CentroPunto>   boundary_;
};
