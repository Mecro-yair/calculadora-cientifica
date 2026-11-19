#include "estadistica.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

#include "evaluador.h"  // ErrorCalculo

std::vector<double> leerListaNumeros(const std::string& texto) {
    std::string limpio = texto;
    for (char& c : limpio) {
        if (c == ',' || c == ';') {
            c = ' ';
        }
    }

    std::vector<double> numeros;
    std::istringstream entrada(limpio);
    std::string palabra;
    while (entrada >> palabra) {
        std::size_t leidos = 0;
        double valor = 0.0;
        try {
            valor = std::stod(palabra, &leidos);
        } catch (const std::exception&) {
            leidos = 0;
        }
        if (leidos != palabra.size() || !std::isfinite(valor)) {
            throw ErrorCalculo("'" + palabra + "' no es un número válido");
        }
        numeros.push_back(valor);
    }
    return numeros;
}

ResumenEstadistico calcularEstadisticas(std::vector<double> datos) {
    if (datos.empty()) {
        throw ErrorCalculo("Escribe al menos un número");
    }

    ResumenEstadistico r;
    std::sort(datos.begin(), datos.end());
    const std::size_t n = datos.size();

    r.cantidad = n;
    for (double x : datos) {
        r.suma += x;
    }
    r.media = r.suma / n;
    r.minimo = datos.front();
    r.maximo = datos.back();
    r.rango = r.maximo - r.minimo;

    // mediana: el valor del centro (o el promedio de los dos del centro)
    if (n % 2 == 1) {
        r.mediana = datos[n / 2];
    } else {
        r.mediana = (datos[n / 2 - 1] + datos[n / 2]) / 2.0;
    }

    // moda: como la lista está ordenada, los valores repetidos están juntos
    std::size_t mayorRepeticion = 1;
    std::size_t i = 0;
    while (i < n) {
        std::size_t j = i;
        while (j < n && datos[j] == datos[i]) {
            j++;
        }
        const std::size_t repeticiones = j - i;
        if (repeticiones > mayorRepeticion) {
            mayorRepeticion = repeticiones;
            r.modas.clear();
            r.modas.push_back(datos[i]);
        } else if (repeticiones == mayorRepeticion && mayorRepeticion > 1) {
            r.modas.push_back(datos[i]);
        }
        i = j;
    }

    // varianza: promedio de las distancias al cuadrado respecto a la media
    double sumaCuadrados = 0.0;
    for (double x : datos) {
        sumaCuadrados += (x - r.media) * (x - r.media);
    }
    r.varianzaPoblacional = sumaCuadrados / n;
    r.desviacionPoblacional = std::sqrt(r.varianzaPoblacional);
    if (n >= 2) {
        r.varianzaMuestral = sumaCuadrados / (n - 1);
        r.desviacionMuestral = std::sqrt(r.varianzaMuestral);
    } else {
        r.varianzaMuestral = std::numeric_limits<double>::quiet_NaN();
        r.desviacionMuestral = std::numeric_limits<double>::quiet_NaN();
    }
    return r;
}
