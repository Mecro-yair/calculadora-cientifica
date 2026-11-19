#include "algebra.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>

#include "evaluador.h"  // ErrorCalculo, formatearNumero

namespace {

const double TOLERANCIA = 1e-12;

bool casiCero(double x) {
    return std::fabs(x) < TOLERANCIA;
}

}  // namespace

ResultadoCuadratica resolverCuadratica(double a, double b, double c) {
    ResultadoCuadratica r;

    if (casiCero(a)) {
        // No es de segundo grado: bx + c = 0
        if (casiCero(b)) {
            if (casiCero(c)) {
                r.tipo = TipoCuadratica::Infinitas;
            } else {
                r.tipo = TipoCuadratica::SinSolucion;
            }
        } else {
            r.tipo = TipoCuadratica::Lineal;
            r.x1 = -c / b;
        }
        return r;
    }

    r.discriminante = b * b - 4 * a * c;
    r.verticeX = -b / (2 * a);
    r.verticeY = a * r.verticeX * r.verticeX + b * r.verticeX + c;

    if (casiCero(r.discriminante)) {
        r.tipo = TipoCuadratica::UnaDoble;
        r.x1 = r.verticeX;
        r.x2 = r.verticeX;
    } else if (r.discriminante > 0) {
        r.tipo = TipoCuadratica::DosReales;
        // Fórmula estable: evita perder precisión al restar números casi iguales
        const double raiz = std::sqrt(r.discriminante);
        double q = 0.0;
        if (b >= 0) {
            q = -0.5 * (b + raiz);
        } else {
            q = -0.5 * (b - raiz);
        }
        r.x1 = q / a;
        r.x2 = c / q;
        if (r.x1 > r.x2) {
            std::swap(r.x1, r.x2);
        }
    } else {
        r.tipo = TipoCuadratica::Complejas;
        r.parteReal = -b / (2 * a);
        r.parteImaginaria = std::sqrt(-r.discriminante) / (2 * std::fabs(a));
    }
    return r;
}

ResultadoSistema resolverSistema2x2(double a1, double b1, double c1, double a2, double b2, double c2) {
    ResultadoSistema r;
    r.determinante = a1 * b2 - a2 * b1;

    if (!casiCero(r.determinante)) {
        r.tipo = TipoSistema::Unica;
        r.x = (c1 * b2 - c2 * b1) / r.determinante;
        r.y = (a1 * c2 - a2 * c1) / r.determinante;
        return r;
    }

    // Determinante 0: las rectas son paralelas (sin solución) o la misma recta (infinitas)
    const bool ecuacion1Imposible = casiCero(a1) && casiCero(b1) && !casiCero(c1);
    const bool ecuacion2Imposible = casiCero(a2) && casiCero(b2) && !casiCero(c2);
    if (ecuacion1Imposible || ecuacion2Imposible) {
        r.tipo = TipoSistema::SinSolucion;
        return r;
    }
    const double determinanteX = c1 * b2 - c2 * b1;
    const double determinanteY = a1 * c2 - a2 * c1;
    if (casiCero(determinanteX) && casiCero(determinanteY)) {
        r.tipo = TipoSistema::Infinitas;
    } else {
        r.tipo = TipoSistema::SinSolucion;
    }
    return r;
}

std::string convertirABase(unsigned long long valor, int base) {
    if (base < 2 || base > 16) {
        throw ErrorCalculo("La base debe estar entre 2 y 16");
    }
    if (valor == 0) {
        return "0";
    }
    const std::string digitos = "0123456789ABCDEF";
    std::string resultado;
    // Divisiones sucesivas: los residuos, leídos al revés, son los dígitos
    while (valor > 0) {
        resultado += digitos[valor % base];
        valor /= base;
    }
    std::reverse(resultado.begin(), resultado.end());
    return resultado;
}

unsigned long long convertirDesdeBase(const std::string& texto, int base) {
    if (base < 2 || base > 16) {
        throw ErrorCalculo("La base debe estar entre 2 y 16");
    }

    std::string limpio;
    for (char c : texto) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            limpio += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
    }
    if (limpio.empty()) {
        throw ErrorCalculo("Escribe un número");
    }
    if (limpio[0] == '-') {
        throw ErrorCalculo("Solo se pueden convertir números enteros no negativos");
    }

    const unsigned long long maximo = std::numeric_limits<unsigned long long>::max();
    unsigned long long valor = 0;
    for (char c : limpio) {
        int digito = -1;
        if (c >= '0' && c <= '9') {
            digito = c - '0';
        } else if (c >= 'A' && c <= 'F') {
            digito = c - 'A' + 10;
        }
        if (digito < 0 || digito >= base) {
            throw ErrorCalculo(std::string("El dígito '") + c + "' no es válido en base " + std::to_string(base));
        }
        // Método de Horner: valor = valor * base + dígito (comprobando que no se desborde)
        if (valor > (maximo - static_cast<unsigned long long>(digito)) / static_cast<unsigned long long>(base)) {
            throw ErrorCalculo("El número es demasiado grande (máximo 18446744073709551615)");
        }
        valor = valor * base + digito;
    }
    return valor;
}
