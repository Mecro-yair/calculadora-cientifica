// Pruebas automáticas de la calculadora.
// Compilar y ejecutar:  compilar.bat pruebas   (o: make pruebas)
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "../src/algebra.h"
#include "../src/estadistica.h"
#include "../src/evaluador.h"

namespace {

int aprobadas = 0;
int fallidas = 0;

void comprobar(bool condicion, const std::string& descripcion) {
    if (condicion) {
        aprobadas++;
    } else {
        fallidas++;
        std::cout << "FALLA: " << descripcion << "\n";
    }
}

bool cerca(double a, double b) {
    return std::fabs(a - b) < 1e-9 * std::max(1.0, std::fabs(b));
}

void esperarResultado(Evaluador& evaluador, const std::string& expresion, double esperado) {
    try {
        const double obtenido = evaluador.evaluar(expresion);
        comprobar(cerca(obtenido, esperado),
                  expresion + " debería dar " + formatearNumero(esperado) + " y dio " + formatearNumero(obtenido));
    } catch (const ErrorCalculo& error) {
        comprobar(false, expresion + " lanzó un error: " + error.what());
    }
}

void esperarError(Evaluador& evaluador, const std::string& expresion) {
    try {
        evaluador.evaluar(expresion);
        comprobar(false, expresion + " debería dar error");
    } catch (const ErrorCalculo&) {
        comprobar(true, "");
    }
}

void probarEvaluador() {
    Evaluador ev;

    // precedencia y paréntesis
    esperarResultado(ev, "2 + 3 * 4", 14);
    esperarResultado(ev, "(2 + 3) * 4", 20);
    esperarResultado(ev, "10 - 4 - 3", 3);
    esperarResultado(ev, "100 / 10 / 5", 2);
    esperarResultado(ev, "2^3^2", 512);
    esperarResultado(ev, "-2^2", -4);
    esperarResultado(ev, "(-2)^2", 4);
    esperarResultado(ev, "2^-1", 0.5);
    esperarResultado(ev, "17 % 5", 2);
    esperarResultado(ev, "--3", 3);
    esperarResultado(ev, "1.5e3 + .5", 1500.5);

    // multiplicación implícita y constantes
    esperarResultado(ev, "2pi", 2 * std::acos(-1.0));
    esperarResultado(ev, "3(4 + 1)", 15);
    esperarResultado(ev, "(1 + 2)(3 + 4)", 21);
    esperarResultado(ev, "2e", 2 * std::exp(1.0));
    esperarResultado(ev, "ln(e)", 1);

    // ans
    ev.evaluar("6 * 7");
    esperarResultado(ev, "ans / 2", 21);

    // trigonometría en grados
    ev.setModo(ModoAngular::Grados);
    esperarResultado(ev, "sin(30)", 0.5);
    esperarResultado(ev, "cos(60)", 0.5);
    esperarResultado(ev, "sin(180)", 0);
    esperarResultado(ev, "tan(45)", 1);
    esperarResultado(ev, "sen(90)", 1);
    esperarResultado(ev, "asin(1)", 90);
    esperarResultado(ev, "acos(0)", 90);
    esperarError(ev, "tan(90)");
    esperarError(ev, "asin(2)");

    // trigonometría en radianes
    ev.setModo(ModoAngular::Radianes);
    esperarResultado(ev, "sin(pi / 2)", 1);
    esperarResultado(ev, "cos(pi)", -1);
    esperarResultado(ev, "atan(1)", std::acos(-1.0) / 4);
    ev.setModo(ModoAngular::Grados);

    // raíces, logaritmos y otras
    esperarResultado(ev, "sqrt(16)", 4);
    esperarResultado(ev, "raiz(27, 3)", 3);
    esperarResultado(ev, "raiz(-8, 3)", -2);
    esperarResultado(ev, "cbrt(-27)", -3);
    esperarResultado(ev, "log(1000)", 3);
    esperarResultado(ev, "log(8, 2)", 3);
    esperarResultado(ev, "exp(0)", 1);
    esperarResultado(ev, "abs(-7.5)", 7.5);
    esperarResultado(ev, "floor(2.7) + ceil(2.1) + round(2.5)", 8);
    esperarResultado(ev, "5!", 120);
    esperarResultado(ev, "0!", 1);
    esperarResultado(ev, "3!!", 720);
    esperarResultado(ev, "nCr(5, 2)", 10);
    esperarResultado(ev, "nCr(52, 5)", 2598960);
    esperarResultado(ev, "nPr(5, 2)", 20);

    // errores que deben detectarse
    esperarError(ev, "");
    esperarError(ev, "1 / 0");
    esperarError(ev, "5 % 0");
    esperarError(ev, "sqrt(-4)");
    esperarError(ev, "ln(0)");
    esperarError(ev, "log(-1)");
    esperarError(ev, "log(8, 1)");
    esperarError(ev, "(-8)^0.5");
    esperarError(ev, "0^-1");
    esperarError(ev, "2.5!");
    esperarError(ev, "(-3)!");
    esperarError(ev, "171!");
    esperarError(ev, "(2 + 3");
    esperarError(ev, "2 + 3)");
    esperarError(ev, "2 +");
    esperarError(ev, "2 3");
    esperarError(ev, "2 $ 3");
    esperarError(ev, "foo(2)");
    esperarError(ev, "x + 1");
    esperarError(ev, "sin(1, 2)");
    esperarError(ev, "10^400");

    // formato
    comprobar(formatearNumero(0.1 + 0.2) == "0.3", "0.1 + 0.2 debería mostrarse como 0.3");
    comprobar(formatearNumero(-0.0) == "0", "-0 debería mostrarse como 0");
    comprobar(formatearNumero(1.0 / 3.0) == "0.333333333333", "1/3 con 12 cifras");
}

void probarEstadistica() {
    const ResumenEstadistico r = calcularEstadisticas(leerListaNumeros("2, 4, 4, 4, 5, 5, 7, 9"));
    comprobar(r.cantidad == 8, "cantidad 8");
    comprobar(cerca(r.suma, 40), "suma 40");
    comprobar(cerca(r.media, 5), "media 5");
    comprobar(cerca(r.mediana, 4.5), "mediana 4.5");
    comprobar(r.modas.size() == 1 && cerca(r.modas[0], 4), "moda 4");
    comprobar(cerca(r.varianzaPoblacional, 4), "varianza poblacional 4");
    comprobar(cerca(r.desviacionPoblacional, 2), "desviación poblacional 2");
    comprobar(cerca(r.varianzaMuestral, 32.0 / 7.0), "varianza muestral 32/7");
    comprobar(cerca(r.rango, 7), "rango 7");

    const ResumenEstadistico impar = calcularEstadisticas(leerListaNumeros("3 1 2"));
    comprobar(cerca(impar.mediana, 2), "mediana de 3 datos");
    comprobar(impar.modas.empty(), "sin moda si nada se repite");

    const ResumenEstadistico dosModas = calcularEstadisticas(leerListaNumeros("1;1;2;2;3"));
    comprobar(dosModas.modas.size() == 2, "dos modas");

    const ResumenEstadistico uno = calcularEstadisticas({5.0});
    comprobar(std::isnan(uno.varianzaMuestral), "varianza muestral de 1 dato no existe");

    bool error = false;
    try {
        calcularEstadisticas(leerListaNumeros("1 dos 3"));
    } catch (const ErrorCalculo&) {
        error = true;
    }
    comprobar(error, "'dos' no es un número");

    error = false;
    try {
        calcularEstadisticas({});
    } catch (const ErrorCalculo&) {
        error = true;
    }
    comprobar(error, "lista vacía");
}

void probarAlgebra() {
    ResultadoCuadratica c = resolverCuadratica(1, -3, 2);  // (x - 1)(x - 2)
    comprobar(c.tipo == TipoCuadratica::DosReales && cerca(c.x1, 1) && cerca(c.x2, 2), "x² - 3x + 2 = 0");
    comprobar(cerca(c.verticeX, 1.5) && cerca(c.verticeY, -0.25), "vértice (1.5, -0.25)");

    c = resolverCuadratica(1, 2, 1);
    comprobar(c.tipo == TipoCuadratica::UnaDoble && cerca(c.x1, -1), "x² + 2x + 1 = 0");

    c = resolverCuadratica(1, 2, 5);
    comprobar(c.tipo == TipoCuadratica::Complejas && cerca(c.parteReal, -1) && cerca(c.parteImaginaria, 2),
              "x² + 2x + 5 = 0 -> -1 ± 2i");

    c = resolverCuadratica(0, 2, -6);
    comprobar(c.tipo == TipoCuadratica::Lineal && cerca(c.x1, 3), "2x - 6 = 0");
    comprobar(resolverCuadratica(0, 0, 5).tipo == TipoCuadratica::SinSolucion, "0 = 5");
    comprobar(resolverCuadratica(0, 0, 0).tipo == TipoCuadratica::Infinitas, "0 = 0");

    c = resolverCuadratica(1, -1e8, 1);  // caso difícil de precisión
    comprobar(cerca(c.x1, 1e-8) && cerca(c.x2, 1e8), "raíces muy distintas sin perder precisión");

    ResultadoSistema s = resolverSistema2x2(2, 3, 8, 1, -1, -1);
    comprobar(s.tipo == TipoSistema::Unica && cerca(s.x, 1) && cerca(s.y, 2), "sistema 2x + 3y = 8, x - y = -1");
    comprobar(resolverSistema2x2(1, 1, 2, 2, 2, 4).tipo == TipoSistema::Infinitas, "misma recta");
    comprobar(resolverSistema2x2(1, 1, 2, 1, 1, 5).tipo == TipoSistema::SinSolucion, "rectas paralelas");
    comprobar(resolverSistema2x2(0, 0, 5, 0, 0, 0).tipo == TipoSistema::SinSolucion, "0 = 5 en el sistema");

    comprobar(convertirABase(255, 16) == "FF", "255 -> FF");
    comprobar(convertirABase(10, 2) == "1010", "10 -> 1010");
    comprobar(convertirABase(0, 2) == "0", "0 -> 0");
    comprobar(convertirABase(8, 8) == "10", "8 -> 10 en octal");
    comprobar(convertirDesdeBase("ff", 16) == 255, "ff -> 255");
    comprobar(convertirDesdeBase("1010", 2) == 10, "1010 -> 10");
    comprobar(convertirDesdeBase("18446744073709551615", 10) == 18446744073709551615ULL, "máximo permitido");

    const std::vector<std::pair<std::string, int>> invalidos = {
        {"102", 2}, {"G1", 16}, {"9", 8}, {"", 10}, {"-5", 10}, {"18446744073709551616", 10}};
    for (const auto& caso : invalidos) {
        bool error = false;
        try {
            convertirDesdeBase(caso.first, caso.second);
        } catch (const ErrorCalculo&) {
            error = true;
        }
        comprobar(error, "'" + caso.first + "' en base " + std::to_string(caso.second) + " debería dar error");
    }
}

}  // namespace

int main() {
    probarEvaluador();
    probarEstadistica();
    probarAlgebra();

    std::cout << "\nPruebas aprobadas: " << aprobadas << "\n"
              << "Pruebas fallidas:  " << fallidas << "\n";
    if (fallidas > 0) {
        return 1;
    }
    return 0;
}
