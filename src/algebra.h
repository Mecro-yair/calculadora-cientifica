// Ecuaciones (2.º grado y sistemas 2x2) y conversión entre bases numéricas.
#ifndef ALGEBRA_H
#define ALGEBRA_H

#include <string>

// ---- Ecuación de segundo grado: ax² + bx + c = 0 ----
enum class TipoCuadratica {
    DosReales,      // dos soluciones reales distintas
    UnaDoble,       // una solución real repetida
    Complejas,      // dos soluciones complejas: parteReal ± parteImaginaria·i
    Lineal,         // a = 0: ecuación de primer grado con una solución
    SinSolucion,    // a = b = 0 y c != 0
    Infinitas       // a = b = c = 0
};

struct ResultadoCuadratica {
    TipoCuadratica tipo = TipoCuadratica::SinSolucion;
    double x1 = 0.0;
    double x2 = 0.0;
    double parteReal = 0.0;
    double parteImaginaria = 0.0;
    double discriminante = 0.0;
    double verticeX = 0.0;  // vértice de la parábola (solo si a != 0)
    double verticeY = 0.0;
};

ResultadoCuadratica resolverCuadratica(double a, double b, double c);

// ---- Sistema 2x2:  a1·x + b1·y = c1   y   a2·x + b2·y = c2  (regla de Cramer) ----
enum class TipoSistema { Unica, SinSolucion, Infinitas };

struct ResultadoSistema {
    TipoSistema tipo = TipoSistema::SinSolucion;
    double x = 0.0;
    double y = 0.0;
    double determinante = 0.0;
};

ResultadoSistema resolverSistema2x2(double a1, double b1, double c1, double a2, double b2, double c2);

// ---- Bases numéricas (2 a 16) ----
// Convierte un entero no negativo a texto en la base indicada: convertirABase(255, 16) = "FF".
std::string convertirABase(unsigned long long valor, int base);

// Convierte un texto en la base indicada a número: convertirDesdeBase("FF", 16) = 255.
// Lanza ErrorCalculo si tiene dígitos no válidos o es demasiado grande.
unsigned long long convertirDesdeBase(const std::string& texto, int base);

#endif
