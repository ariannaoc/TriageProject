#include "GestorCentros.hpp"

#include <algorithm>
#include <cmath>

namespace
{
constexpr double kPi              = 3.14159265358979323846;
constexpr double kKmPorGrado      = 111.0;   // 1 grado ~ 111 km (aprox.)
constexpr double kRadioInicialKm  = 5.0;
constexpr double kRadioMaxKm      = 200.0;
constexpr double kMargenGrados    = 0.5;     // margen del boundary del QuadTree

/// Distancia euclidiana local (km) entre dos coordenadas. Aproximación
/// esférica: latitud y longitud escaladas a km; longitud corregida por cos(lat).
double distanciaKm(double lat1, double lng1, double lat2, double lng2)
{
    const double rad  = lat1 * kPi / 180.0;
    const double dlat = (lat2 - lat1) * kKmPorGrado;
    const double dlng = (lng2 - lng1) * kKmPorGrado * std::cos(rad);
    return std::sqrt(dlat * dlat + dlng * dlng);
}
}  // namespace

GestorCentros::GestorCentros() = default;

GestorCentros::~GestorCentros()
{
    delete quadtree_;
}

void GestorCentros::cargarCentros(const std::vector<CentroSalud>& centros)
{
    centros_.clear();
    delete quadtree_;
    quadtree_ = nullptr;

    if (centros.empty())
        return;

    // Boundary del QuadTree: debe contener todos los centros. Se calcula el
    // rectángulo mínimo y se le añade un margen para que ninguna inserción
    // caiga fuera del cuadrado raíz.
    double min_lat = centros.front().latitud;
    double max_lat = centros.front().latitud;
    double min_lng = centros.front().longitud;
    double max_lng = centros.front().longitud;
    for (const auto& c : centros)
    {
        min_lat = std::min(min_lat, c.latitud);
        max_lat = std::max(max_lat, c.latitud);
        min_lng = std::min(min_lng, c.longitud);
        max_lng = std::max(max_lng, c.longitud);
    }
    const double center_lat = (min_lat + max_lat) / 2.0;
    const double center_lng = (min_lng + max_lng) / 2.0;
    const double half_dim   = std::max(std::max((max_lat - min_lat) / 2.0,
                                                (max_lng - min_lng) / 2.0), 0.0)
                            + kMargenGrados;

    boundary_ = Designar::QuadBoundary<CentroPunto>(
        CentroPunto{Designar::Point2D(center_lat, center_lng), -1}, half_dim);
    quadtree_ = new Designar::QuadTree<CentroPunto>(boundary_);

    for (const auto& c : centros)
    {
        centros_[c.id] = c;
        CentroPunto cp;
        cp.punto     = Designar::Point2D(c.latitud, c.longitud);
        cp.id_centro = c.id;
        quadtree_->insert(cp);
    }
}

std::vector<CentroSalud> GestorCentros::obtenerCercanos(double lat, double lng,
                                                        int k,
                                                        int complejidadMinima) const
{
    std::vector<CentroSalud> resultado;
    if (!quadtree_ || k <= 0)
        return resultado;

    const double rad = lat * kPi / 180.0;

    // Radio expansivo: el QuadTree de DeSiGNAR solo ofrece query_range con
    // un cuadrado. Para emular K-NN se consulta con un cuadrado cada vez
    // mayor hasta reunir al menos K candidatos que cumplan la complejidad
    // mínima (o alcanzar el radio máximo).
    double radio_km = kRadioInicialKm;
    std::vector<std::pair<double, CentroSalud>> candidatos;

    while (true)
    {
        const double delta_lat = radio_km / kKmPorGrado;
        const double delta_lng = radio_km / (kKmPorGrado * std::cos(rad));
        const double half      = std::max(delta_lat, delta_lng);  // peor caso

        Designar::QuadBoundary<CentroPunto> query(
            CentroPunto{Designar::Point2D(lat, lng), -1}, half);

        Designar::DynArray<CentroPunto> found;
        quadtree_->query_range(query, found);

        candidatos.clear();
        for (Designar::nat_t i = 0; i < found.size(); ++i)
        {
            const auto& cp = found[i];
            const auto it  = centros_.find(cp.id_centro);
            if (it == centros_.end())
                continue;
            if (it->second.nivel_complejidad < complejidadMinima)
                continue;
            const double d = distanciaKm(lat, lng, it->second.latitud,
                                         it->second.longitud);
            candidatos.emplace_back(d, it->second);
        }

        if (static_cast<int>(candidatos.size()) >= k || radio_km >= kRadioMaxKm)
            break;
        radio_km = std::min(radio_km * 2.0, kRadioMaxKm);
    }

    std::sort(candidatos.begin(), candidatos.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    const int n = std::min<int>(k, static_cast<int>(candidatos.size()));
    for (int i = 0; i < n; ++i)
        resultado.push_back(candidatos[i].second);

    return resultado;
}
