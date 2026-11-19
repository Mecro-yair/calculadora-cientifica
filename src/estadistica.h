// Estadística descriptiva de una lista de números.
#ifndef ESTADISTICA_H
#define ESTADISTICA_H

#include <cstddef>
#include <string>
#include <vector>

struct ResumenEstadistico {
    std::size_t cantidad = 0;
    double suma = 0.0;
    double media = 0.0;
    double mediana = 0.0;
    std::vector<double> modas;      // vacía si ningún valor se repite
    double minimo = 0.0;
    double maximo = 0.0;
    double rango = 0.0;
    double varianzaPoblacional = 0.0;
    double desviacionPoblacional = 0.0;
    double varianzaMuestral = 0.0;  // NaN si hay un solo dato
    double desviacionMuestral = 0.0;
};

// Convierte "4 8, 15; 16 23 42" en una lista de números.
// Acepta espacios, comas o punto y coma como separadores (los decimales van con punto).
std::vector<double> leerListaNumeros(const std::string& texto);

// Calcula todas las medidas. Lanza ErrorCalculo si la lista está vacía.
ResumenEstadistico calcularEstadisticas(std::vector<double> datos);

#endif
