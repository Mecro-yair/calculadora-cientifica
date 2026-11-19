// Calculadora científica de consola
// Autor: Danny Yair Luque Pari
//
// Menú principal y comunicación con el usuario. Los cálculos están en:
//   evaluador.cpp   -> expresiones, trigonometría, logaritmos, factorial...
//   estadistica.cpp -> media, mediana, moda, varianza, desviación estándar
//   algebra.cpp     -> ecuación de 2.º grado, sistemas 2x2 y bases numéricas
#include <cmath>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX  // evita que windows.h defina min() y max() como macros
#endif
#include <windows.h>
#endif

#include "algebra.h"
#include "estadistica.h"
#include "evaluador.h"

namespace {

const std::size_t MAXIMO_HISTORIAL = 50;

std::vector<std::string> historial;

void agregarAlHistorial(const std::string& linea) {
    historial.push_back(linea);
    if (historial.size() > MAXIMO_HISTORIAL) {
        historial.erase(historial.begin());
    }
}

void configurarConsola() {
#ifdef _WIN32
    // Para que las tildes y símbolos (², ±, √) se vean bien en la consola de Windows
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

// Lee una línea. Devuelve false si se cerró la entrada (Ctrl+Z / fin de archivo).
bool leerLinea(const std::string& mensaje, std::string& linea) {
    std::cout << mensaje;
    if (!std::getline(std::cin, linea)) {
        return false;
    }
    // quita espacios al inicio y al final
    const std::size_t inicio = linea.find_first_not_of(" \t\r");
    if (inicio == std::string::npos) {
        linea.clear();
    } else {
        const std::size_t fin = linea.find_last_not_of(" \t\r");
        linea = linea.substr(inicio, fin - inicio + 1);
    }
    return true;
}

// Pide un número. También acepta expresiones: "sqrt(2)", "1/3", "pi/4".
std::optional<double> leerNumero(const Evaluador& evaluador, const std::string& mensaje) {
    Evaluador calculo = evaluador;  // copia: así no cambia el "ans" de la calculadora
    std::string linea;
    while (leerLinea(mensaje, linea)) {
        if (linea.empty()) {
            std::cout << "  Escribe un valor.\n";
            continue;
        }
        try {
            return calculo.evaluar(linea);
        } catch (const ErrorCalculo& error) {
            std::cout << "  Error: " << error.what() << "\n";
        }
    }
    return std::nullopt;
}

std::string nombreModo(const Evaluador& evaluador) {
    if (evaluador.modo() == ModoAngular::Grados) {
        return "GRADOS";
    }
    return "RADIANES";
}

void titulo(const std::string& texto) {
    std::cout << "\n--- " << texto << " ---\n";
}

// Escribe la ecuación como la escribiríamos a mano: "x² - 3x + 2 = 0" (y no "1x² + -3x + 2 = 0").
std::string escribirEcuacion(double a, double b, double c) {
    const double coeficientes[] = {a, b, c};
    const char* variables[] = {"x²", "x", ""};
    std::string texto;
    for (int i = 0; i < 3; i++) {
        const double valor = coeficientes[i];
        if (valor == 0.0) {
            continue;
        }
        if (texto.empty()) {
            if (valor < 0) {
                texto += "-";
            }
        } else if (valor < 0) {
            texto += " - ";
        } else {
            texto += " + ";
        }
        const double absoluto = std::fabs(valor);
        if (absoluto != 1.0 || i == 2) {
            texto += formatearNumero(absoluto);
        }
        texto += variables[i];
    }
    if (texto.empty()) {
        texto = "0";
    }
    return texto + " = 0";
}

// ---------------------------------------------------------------------------
// Ayuda
// ---------------------------------------------------------------------------
void mostrarAyuda() {
    titulo("Ayuda");
    std::cout
        << "Operadores:   +  -  *  /  ^ (potencia)  % (residuo)  ! (factorial)\n"
        << "Constantes:   pi   e   ans (último resultado)\n"
        << "Trigonometría (según el modo grados/radianes):\n"
        << "              sin (o sen)  cos  tan  asin  acos  atan\n"
        << "Hiperbólicas: sinh  cosh  tanh\n"
        << "Raíces:       sqrt(x) o raiz(x)   raiz(x, n) = raíz n-ésima   cbrt(x)\n"
        << "Logaritmos:   ln(x)   log(x) en base 10   log(x, b) en base b   exp(x)\n"
        << "Otras:        abs  floor  ceil  round  fact(n)  nCr(n, r)  nPr(n, r)\n"
        << "\nEjemplos:\n"
        << "  2 + 3 * 4            -> 14\n"
        << "  (2 + 3) * 4          -> 20\n"
        << "  2^10                 -> 1024\n"
        << "  sin(30) + cos(60)    -> 1        (en grados)\n"
        << "  raiz(27, 3)          -> 3\n"
        << "  log(8, 2)            -> 3\n"
        << "  5!                   -> 120\n"
        << "  nCr(5, 2)            -> 10\n"
        << "  2pi                  -> 6.28318530718   (multiplicación implícita)\n"
        << "  ans * 2              -> el doble del resultado anterior\n";
}

// ---------------------------------------------------------------------------
// 1. Expresiones
// ---------------------------------------------------------------------------
bool modoExpresiones(Evaluador& evaluador) {
    titulo("Calcular expresión (modo " + nombreModo(evaluador) + ")");
    std::cout << "Escribe una expresión y pulsa Enter. 'ayuda' muestra las funciones.\n"
              << "Deja la línea vacía para volver al menú.\n";

    std::string linea;
    while (true) {
        if (!leerLinea("\n> ", linea)) {
            return false;
        }
        if (linea.empty()) {
            return true;
        }
        if (linea == "ayuda") {
            mostrarAyuda();
            continue;
        }
        try {
            const double resultado = evaluador.evaluar(linea);
            const std::string texto = formatearNumero(resultado);
            std::cout << "= " << texto << "\n";
            agregarAlHistorial(linea + " = " + texto);
        } catch (const ErrorCalculo& error) {
            std::cout << "Error: " << error.what() << "\n";
        }
    }
}

// ---------------------------------------------------------------------------
// 2. Ecuación de segundo grado
// ---------------------------------------------------------------------------
bool modoCuadratica(const Evaluador& evaluador) {
    titulo("Ecuación de 2.º grado: ax² + bx + c = 0");
    const std::optional<double> a = leerNumero(evaluador, "a = ");
    if (!a) {
        return false;
    }
    const std::optional<double> b = leerNumero(evaluador, "b = ");
    if (!b) {
        return false;
    }
    const std::optional<double> c = leerNumero(evaluador, "c = ");
    if (!c) {
        return false;
    }

    const ResultadoCuadratica r = resolverCuadratica(*a, *b, *c);
    const std::string ecuacion = escribirEcuacion(*a, *b, *c);
    std::cout << "\n" << ecuacion << "\n";

    std::string resumen;
    switch (r.tipo) {
        case TipoCuadratica::DosReales:
            std::cout << "Discriminante: " << formatearNumero(r.discriminante) << " (> 0, dos soluciones reales)\n"
                      << "x1 = " << formatearNumero(r.x1) << "\n"
                      << "x2 = " << formatearNumero(r.x2) << "\n";
            resumen = "x1 = " + formatearNumero(r.x1) + ", x2 = " + formatearNumero(r.x2);
            break;
        case TipoCuadratica::UnaDoble:
            std::cout << "Discriminante: 0 (una solución doble)\n"
                      << "x = " << formatearNumero(r.x1) << "\n";
            resumen = "x = " + formatearNumero(r.x1) + " (doble)";
            break;
        case TipoCuadratica::Complejas:
            std::cout << "Discriminante: " << formatearNumero(r.discriminante)
                      << " (< 0, soluciones complejas)\n"
                      << "x1 = " << formatearNumero(r.parteReal) << " + " << formatearNumero(r.parteImaginaria) << "i\n"
                      << "x2 = " << formatearNumero(r.parteReal) << " - " << formatearNumero(r.parteImaginaria) << "i\n";
            resumen = "x = " + formatearNumero(r.parteReal) + " ± " + formatearNumero(r.parteImaginaria) + "i";
            break;
        case TipoCuadratica::Lineal:
            std::cout << "Como a = 0, es una ecuación de primer grado.\n"
                      << "x = " << formatearNumero(r.x1) << "\n";
            resumen = "x = " + formatearNumero(r.x1);
            break;
        case TipoCuadratica::SinSolucion:
            std::cout << "La ecuación no tiene solución.\n";
            resumen = "sin solución";
            break;
        case TipoCuadratica::Infinitas:
            std::cout << "Todos los coeficientes son 0: cualquier x es solución.\n";
            resumen = "infinitas soluciones";
            break;
    }
    if (r.tipo == TipoCuadratica::DosReales || r.tipo == TipoCuadratica::UnaDoble
        || r.tipo == TipoCuadratica::Complejas) {
        std::cout << "Vértice de la parábola: (" << formatearNumero(r.verticeX) << ", "
                  << formatearNumero(r.verticeY) << ")\n";
    }
    agregarAlHistorial(ecuacion + "  ->  " + resumen);
    return true;
}

// ---------------------------------------------------------------------------
// 3. Sistema de ecuaciones 2x2
// ---------------------------------------------------------------------------
bool modoSistema(const Evaluador& evaluador) {
    titulo("Sistema 2x2:  a1·x + b1·y = c1   y   a2·x + b2·y = c2");
    const char* nombres[] = {"a1 = ", "b1 = ", "c1 = ", "a2 = ", "b2 = ", "c2 = "};
    double v[6] = {0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 6; i++) {
        if (i == 3) {
            std::cout << "Segunda ecuación:\n";
        }
        const std::optional<double> valor = leerNumero(evaluador, nombres[i]);
        if (!valor) {
            return false;
        }
        v[i] = *valor;
    }

    const ResultadoSistema r = resolverSistema2x2(v[0], v[1], v[2], v[3], v[4], v[5]);
    std::cout << "\nDeterminante: " << formatearNumero(r.determinante) << "\n";

    std::string resumen;
    if (r.tipo == TipoSistema::Unica) {
        std::cout << "x = " << formatearNumero(r.x) << "\n"
                  << "y = " << formatearNumero(r.y) << "\n";
        resumen = "x = " + formatearNumero(r.x) + ", y = " + formatearNumero(r.y);
    } else if (r.tipo == TipoSistema::Infinitas) {
        std::cout << "Las dos ecuaciones representan la misma recta: hay infinitas soluciones.\n";
        resumen = "infinitas soluciones";
    } else {
        std::cout << "Las rectas son paralelas: el sistema no tiene solución.\n";
        resumen = "sin solución";
    }
    agregarAlHistorial("Sistema 2x2 -> " + resumen);
    return true;
}

// ---------------------------------------------------------------------------
// 4. Estadística
// ---------------------------------------------------------------------------
// Alinea las etiquetas en columna. Cuenta letras y no bytes, porque en UTF-8 "á" ocupa 2 bytes.
std::string fila(const std::string& etiqueta, const std::string& valor) {
    std::size_t letras = 0;
    for (unsigned char c : etiqueta) {
        if ((c & 0xC0) != 0x80) {
            letras++;
        }
    }
    const std::size_t ancho = 30;
    std::string espacios;
    if (letras < ancho) {
        espacios = std::string(ancho - letras, ' ');
    }
    return etiqueta + espacios + valor + "\n";
}

std::string formatearOGuion(double valor) {
    if (std::isnan(valor)) {
        return "— (se necesitan al menos 2 datos)";
    }
    return formatearNumero(valor);
}

bool modoEstadistica() {
    titulo("Estadística");
    std::cout << "Escribe los datos separados por espacios o comas (decimales con punto).\n"
              << "Ejemplo: 12 15.5 9 20 15\n";

    std::string linea;
    while (true) {
        if (!leerLinea("Datos: ", linea)) {
            return false;
        }
        try {
            const ResumenEstadistico r = calcularEstadisticas(leerListaNumeros(linea));

            std::string modas;
            if (r.modas.empty()) {
                modas = "no hay (ningún valor se repite)";
            } else {
                for (std::size_t i = 0; i < r.modas.size(); i++) {
                    if (i > 0) {
                        modas += ", ";
                    }
                    modas += formatearNumero(r.modas[i]);
                }
            }

            std::cout << "\n"
                      << fila("Cantidad de datos:", std::to_string(r.cantidad))
                      << fila("Suma:", formatearNumero(r.suma))
                      << fila("Media (promedio):", formatearNumero(r.media))
                      << fila("Mediana:", formatearNumero(r.mediana))
                      << fila("Moda:", modas)
                      << fila("Mínimo / Máximo:", formatearNumero(r.minimo) + " / " + formatearNumero(r.maximo))
                      << fila("Rango:", formatearNumero(r.rango))
                      << fila("Varianza (población):", formatearNumero(r.varianzaPoblacional))
                      << fila("Desviación est. (población):", formatearNumero(r.desviacionPoblacional))
                      << fila("Varianza (muestra):", formatearOGuion(r.varianzaMuestral))
                      << fila("Desviación est. (muestra):", formatearOGuion(r.desviacionMuestral));
            agregarAlHistorial("Estadística de " + std::to_string(r.cantidad) + " datos -> media = "
                               + formatearNumero(r.media) + ", desviación = "
                               + formatearNumero(r.desviacionPoblacional));
            return true;
        } catch (const ErrorCalculo& error) {
            std::cout << "  Error: " << error.what() << "\n";
        }
    }
}

// ---------------------------------------------------------------------------
// 5. Bases numéricas
// ---------------------------------------------------------------------------
bool modoBases() {
    titulo("Conversión de bases");
    std::cout << "Base del número que vas a escribir: 2 (binario), 8 (octal), 10 (decimal) o 16 (hexadecimal)\n";

    std::string linea;
    int base = 0;
    while (base == 0) {
        if (!leerLinea("Base: ", linea)) {
            return false;
        }
        if (linea == "2" || linea == "8" || linea == "10" || linea == "16") {
            base = std::stoi(linea);
        } else {
            std::cout << "  Escribe 2, 8, 10 o 16.\n";
        }
    }

    while (true) {
        if (!leerLinea("Número en base " + std::to_string(base) + ": ", linea)) {
            return false;
        }
        try {
            const unsigned long long valor = convertirDesdeBase(linea, base);
            std::cout << "\nDecimal (10):      " << convertirABase(valor, 10) << "\n"
                      << "Binario (2):       " << convertirABase(valor, 2) << "\n"
                      << "Octal (8):         " << convertirABase(valor, 8) << "\n"
                      << "Hexadecimal (16):  " << convertirABase(valor, 16) << "\n";
            agregarAlHistorial(linea + " (base " + std::to_string(base) + ") = " + convertirABase(valor, 10)
                               + " (decimal) = " + convertirABase(valor, 2) + " (binario) = "
                               + convertirABase(valor, 16) + " (hex)");
            return true;
        } catch (const ErrorCalculo& error) {
            std::cout << "  Error: " << error.what() << "\n";
        }
    }
}

// ---------------------------------------------------------------------------
// 6 y 7. Modo angular e historial
// ---------------------------------------------------------------------------
void cambiarModo(Evaluador& evaluador) {
    if (evaluador.modo() == ModoAngular::Grados) {
        evaluador.setModo(ModoAngular::Radianes);
    } else {
        evaluador.setModo(ModoAngular::Grados);
    }
    std::cout << "\nAhora las funciones trigonométricas usan " << nombreModo(evaluador) << ".\n";
}

void mostrarHistorial() {
    titulo("Historial");
    if (historial.empty()) {
        std::cout << "Todavía no hay operaciones.\n";
        return;
    }
    for (std::size_t i = 0; i < historial.size(); i++) {
        std::cout << (i + 1) << ". " << historial[i] << "\n";
    }
}

void mostrarMenu(const Evaluador& evaluador) {
    std::cout << "\n=============================================\n"
              << "          CALCULADORA CIENTÍFICA\n"
              << "=============================================\n"
              << "  Modo angular: " << nombreModo(evaluador)
              << "    ans = " << formatearNumero(evaluador.ultimoResultado()) << "\n"
              << "---------------------------------------------\n"
              << "  1. Calcular expresión\n"
              << "  2. Ecuación de 2.º grado\n"
              << "  3. Sistema de ecuaciones 2x2\n"
              << "  4. Estadística\n"
              << "  5. Conversión de bases\n"
              << "  6. Cambiar a " << (evaluador.modo() == ModoAngular::Grados ? "radianes" : "grados") << "\n"
              << "  7. Historial\n"
              << "  8. Ayuda\n"
              << "  0. Salir\n"
              << "---------------------------------------------\n";
}

}  // namespace

int main() {
    configurarConsola();
    Evaluador evaluador;

    std::string opcion;
    bool continuar = true;
    while (continuar) {
        mostrarMenu(evaluador);
        if (!leerLinea("Elige una opción: ", opcion)) {
            break;
        }

        if (opcion == "1") {
            continuar = modoExpresiones(evaluador);
        } else if (opcion == "2") {
            continuar = modoCuadratica(evaluador);
        } else if (opcion == "3") {
            continuar = modoSistema(evaluador);
        } else if (opcion == "4") {
            continuar = modoEstadistica();
        } else if (opcion == "5") {
            continuar = modoBases();
        } else if (opcion == "6") {
            cambiarModo(evaluador);
        } else if (opcion == "7") {
            mostrarHistorial();
        } else if (opcion == "8") {
            mostrarAyuda();
        } else if (opcion == "0") {
            continuar = false;
        } else {
            std::cout << "Opción no válida. Escribe un número del 0 al 8.\n";
        }
    }

    std::cout << "\n¡Hasta luego!\n";
    return 0;
}
