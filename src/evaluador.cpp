#include "evaluador.h"

#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace {

const double PI = std::acos(-1.0);
const double TOLERANCIA = 1e-12;

bool esEntero(double x) {
    return std::isfinite(x) && std::floor(x) == x;
}

// En grados, sin(180) da 1.2e-16 por el redondeo del computador: lo dejamos en 0.
double limpiarCero(double x) {
    if (std::fabs(x) < TOLERANCIA) {
        return 0.0;
    }
    return x;
}

}  // namespace

// ---------------------------------------------------------------------------
// Funciones matemáticas auxiliares
// ---------------------------------------------------------------------------
double factorial(double n) {
    if (!esEntero(n) || n < 0) {
        throw ErrorCalculo("El factorial solo existe para enteros no negativos (0, 1, 2, ...)");
    }
    if (n > 170) {
        throw ErrorCalculo("El factorial de " + formatearNumero(n) + " es demasiado grande (máximo 170!)");
    }
    double resultado = 1.0;
    for (int i = 2; i <= static_cast<int>(n); i++) {
        resultado *= i;
    }
    return resultado;
}

double combinaciones(double n, double r) {
    if (!esEntero(n) || !esEntero(r) || n < 0 || r < 0 || r > n) {
        throw ErrorCalculo("nCr(n, r) necesita enteros con 0 <= r <= n");
    }
    if (r > n - r) {
        r = n - r;  // C(n, r) = C(n, n - r): menos multiplicaciones
    }
    double resultado = 1.0;
    for (int i = 1; i <= static_cast<int>(r); i++) {
        resultado = resultado * (n - r + i) / i;
    }
    return std::round(resultado);
}

double permutaciones(double n, double r) {
    if (!esEntero(n) || !esEntero(r) || n < 0 || r < 0 || r > n) {
        throw ErrorCalculo("nPr(n, r) necesita enteros con 0 <= r <= n");
    }
    double resultado = 1.0;
    for (int i = 0; i < static_cast<int>(r); i++) {
        resultado *= (n - i);
    }
    return resultado;
}

std::string formatearNumero(double valor) {
    if (valor == 0.0) {
        return "0";  // evita mostrar "-0"
    }
    std::ostringstream salida;
    salida << std::setprecision(12) << valor;
    return salida.str();
}

// ---------------------------------------------------------------------------
// Configuración
// ---------------------------------------------------------------------------
void Evaluador::setModo(ModoAngular modo) {
    modo_ = modo;
}

ModoAngular Evaluador::modo() const {
    return modo_;
}

double Evaluador::ultimoResultado() const {
    return ans_;
}

double Evaluador::aRadianes(double angulo) const {
    if (modo_ == ModoAngular::Grados) {
        return angulo * PI / 180.0;
    }
    return angulo;
}

double Evaluador::desdeRadianes(double angulo) const {
    if (modo_ == ModoAngular::Grados) {
        return angulo * 180.0 / PI;
    }
    return angulo;
}

// ---------------------------------------------------------------------------
// Análisis léxico
// ---------------------------------------------------------------------------
void Evaluador::tokenizar(const std::string& texto) {
    tokens_.clear();
    std::size_t i = 0;

    while (i < texto.size()) {
        const char c = texto[i];
        const std::size_t posicion = i + 1;

        if (std::isspace(static_cast<unsigned char>(c))) {
            i++;
        } else if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
            // número: 12, 3.5, .5, 1e3, 2.5E-4
            std::size_t fin = i;
            while (fin < texto.size() && std::isdigit(static_cast<unsigned char>(texto[fin]))) {
                fin++;
            }
            if (fin < texto.size() && texto[fin] == '.') {
                fin++;
                while (fin < texto.size() && std::isdigit(static_cast<unsigned char>(texto[fin]))) {
                    fin++;
                }
            }
            // exponente solo si después de la "e" viene un dígito (si no, es la constante e: 2e = 2*e)
            if (fin < texto.size() && (texto[fin] == 'e' || texto[fin] == 'E')) {
                std::size_t j = fin + 1;
                if (j < texto.size() && (texto[j] == '+' || texto[j] == '-')) {
                    j++;
                }
                if (j < texto.size() && std::isdigit(static_cast<unsigned char>(texto[j]))) {
                    fin = j;
                    while (fin < texto.size() && std::isdigit(static_cast<unsigned char>(texto[fin]))) {
                        fin++;
                    }
                }
            }
            const std::string numero = texto.substr(i, fin - i);
            if (numero == ".") {
                throw ErrorCalculo("Número mal escrito en la posición " + std::to_string(posicion));
            }
            tokens_.push_back({TipoToken::Numero, numero, std::stod(numero), posicion});
            i = fin;
        } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            std::size_t fin = i;
            while (fin < texto.size()
                   && (std::isalnum(static_cast<unsigned char>(texto[fin])) || texto[fin] == '_')) {
                fin++;
            }
            tokens_.push_back({TipoToken::Identificador, texto.substr(i, fin - i), 0.0, posicion});
            i = fin;
        } else if (c == '(') {
            tokens_.push_back({TipoToken::ParentesisAbre, "(", 0.0, posicion});
            i++;
        } else if (c == ')') {
            tokens_.push_back({TipoToken::ParentesisCierra, ")", 0.0, posicion});
            i++;
        } else if (c == ',') {
            tokens_.push_back({TipoToken::Coma, ",", 0.0, posicion});
            i++;
        } else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '%' || c == '!') {
            tokens_.push_back({TipoToken::Operador, std::string(1, c), 0.0, posicion});
            i++;
        } else {
            throw ErrorCalculo("Carácter no válido en la posición " + std::to_string(posicion));
        }
    }
    tokens_.push_back({TipoToken::Fin, "", 0.0, texto.size() + 1});
}

const Evaluador::Token& Evaluador::ver() const {
    return tokens_[actual_];
}

Evaluador::Token Evaluador::avanzar() {
    Token token = tokens_[actual_];
    if (token.tipo != TipoToken::Fin) {
        actual_++;
    }
    return token;
}

bool Evaluador::esOperador(const std::string& operador) const {
    return ver().tipo == TipoToken::Operador && ver().texto == operador;
}

// "2pi", "3(4+1)" y "(1+2)(3+4)" se entienden como multiplicaciones.
bool Evaluador::empiezaMultiplicacionImplicita() const {
    return ver().tipo == TipoToken::Identificador || ver().tipo == TipoToken::ParentesisAbre;
}

// ---------------------------------------------------------------------------
// Análisis sintáctico y cálculo
// ---------------------------------------------------------------------------
double Evaluador::evaluar(const std::string& texto) {
    tokenizar(texto);
    actual_ = 0;

    if (ver().tipo == TipoToken::Fin) {
        throw ErrorCalculo("Escribe una expresión, por ejemplo: 2 + 3 * 4");
    }

    const double resultado = expresion();

    if (ver().tipo != TipoToken::Fin) {
        if (ver().tipo == TipoToken::ParentesisCierra) {
            throw ErrorCalculo("Sobra un ')' en la posición " + std::to_string(ver().posicion));
        }
        throw ErrorCalculo("Falta un operador antes de '" + ver().texto + "' (posición "
                           + std::to_string(ver().posicion) + ")");
    }
    if (std::isnan(resultado)) {
        throw ErrorCalculo("El resultado no es un número real");
    }
    if (std::isinf(resultado)) {
        throw ErrorCalculo("El resultado es demasiado grande");
    }

    ans_ = resultado;
    return resultado;
}

double Evaluador::expresion() {
    double valor = termino();
    while (true) {
        if (esOperador("+")) {
            avanzar();
            valor += termino();
        } else if (esOperador("-")) {
            avanzar();
            valor -= termino();
        } else {
            break;
        }
    }
    return valor;
}

double Evaluador::termino() {
    double valor = unario();
    while (true) {
        if (esOperador("*")) {
            avanzar();
            valor *= unario();
        } else if (esOperador("/")) {
            avanzar();
            const double divisor = unario();
            if (divisor == 0.0) {
                throw ErrorCalculo("No se puede dividir entre cero");
            }
            valor /= divisor;
        } else if (esOperador("%")) {
            avanzar();
            const double divisor = unario();
            if (divisor == 0.0) {
                throw ErrorCalculo("No se puede calcular el residuo de dividir entre cero");
            }
            valor = std::fmod(valor, divisor);
        } else if (empiezaMultiplicacionImplicita()) {
            valor *= potencia();
        } else {
            break;
        }
    }
    return valor;
}

double Evaluador::unario() {
    if (esOperador("-")) {
        avanzar();
        return -unario();
    }
    if (esOperador("+")) {
        avanzar();
        return unario();
    }
    return potencia();
}

double Evaluador::potencia() {
    const double base = postfijo();
    if (!esOperador("^")) {
        return base;
    }
    avanzar();
    const double exponente = unario();  // permite 2^-1 y 2^3^2 = 2^(3^2)

    if (base == 0.0 && exponente < 0) {
        throw ErrorCalculo("0 elevado a un número negativo es una división entre cero");
    }
    if (base < 0 && !esEntero(exponente)) {
        throw ErrorCalculo("Una base negativa con exponente decimal no da un número real");
    }
    return std::pow(base, exponente);
}

double Evaluador::postfijo() {
    double valor = primario();
    while (esOperador("!")) {
        avanzar();
        valor = factorial(valor);
    }
    return valor;
}

double Evaluador::primario() {
    const Token token = avanzar();

    if (token.tipo == TipoToken::Numero) {
        return token.valor;
    }

    if (token.tipo == TipoToken::ParentesisAbre) {
        const double valor = expresion();
        if (ver().tipo != TipoToken::ParentesisCierra) {
            throw ErrorCalculo("Falta cerrar el paréntesis abierto en la posición " + std::to_string(token.posicion));
        }
        avanzar();
        return valor;
    }

    if (token.tipo == TipoToken::Identificador) {
        const std::string& nombre = token.texto;

        if (ver().tipo == TipoToken::ParentesisAbre) {
            avanzar();
            std::vector<double> argumentos;
            if (ver().tipo != TipoToken::ParentesisCierra) {
                argumentos.push_back(expresion());
                while (ver().tipo == TipoToken::Coma) {
                    avanzar();
                    argumentos.push_back(expresion());
                }
            }
            if (ver().tipo != TipoToken::ParentesisCierra) {
                throw ErrorCalculo("Falta cerrar el paréntesis de " + nombre + "(...)");
            }
            avanzar();
            return llamarFuncion(nombre, argumentos, token.posicion);
        }

        if (nombre == "pi") {
            return PI;
        }
        if (nombre == "e") {
            return std::exp(1.0);
        }
        if (nombre == "ans") {
            return ans_;
        }
        throw ErrorCalculo("No conozco '" + nombre + "'. Escribe 'ayuda' para ver las funciones disponibles");
    }

    if (token.tipo == TipoToken::Fin) {
        throw ErrorCalculo("La expresión está incompleta");
    }
    throw ErrorCalculo("No se esperaba '" + token.texto + "' en la posición " + std::to_string(token.posicion));
}

double Evaluador::llamarFuncion(const std::string& nombre, const std::vector<double>& args, std::size_t posicion) {
    auto pedir = [&](std::size_t minimo, std::size_t maximo) {
        if (args.size() < minimo || args.size() > maximo) {
            std::string cantidad = std::to_string(minimo);
            if (maximo != minimo) {
                cantidad += " o " + std::to_string(maximo);
            }
            throw ErrorCalculo("La función " + nombre + " necesita " + cantidad
                               + (maximo == 1 ? " valor" : " valores") + " (posición "
                               + std::to_string(posicion) + ")");
        }
    };

    // --- Trigonométricas (usan el modo grados/radianes) ---
    if (nombre == "sin" || nombre == "sen") {
        pedir(1, 1);
        return limpiarCero(std::sin(aRadianes(args[0])));
    }
    if (nombre == "cos") {
        pedir(1, 1);
        return limpiarCero(std::cos(aRadianes(args[0])));
    }
    if (nombre == "tan") {
        pedir(1, 1);
        const double radianes = aRadianes(args[0]);
        if (std::fabs(std::cos(radianes)) < TOLERANCIA) {
            throw ErrorCalculo("La tangente no está definida en ese ángulo");
        }
        return limpiarCero(std::tan(radianes));
    }
    if (nombre == "asin" || nombre == "asen" || nombre == "acos") {
        pedir(1, 1);
        if (args[0] < -1.0 || args[0] > 1.0) {
            throw ErrorCalculo(nombre + " solo acepta valores entre -1 y 1");
        }
        if (nombre == "acos") {
            return desdeRadianes(std::acos(args[0]));
        }
        return desdeRadianes(std::asin(args[0]));
    }
    if (nombre == "atan") {
        pedir(1, 1);
        return desdeRadianes(std::atan(args[0]));
    }

    // --- Hiperbólicas ---
    if (nombre == "sinh" || nombre == "senh") {
        pedir(1, 1);
        return std::sinh(args[0]);
    }
    if (nombre == "cosh") {
        pedir(1, 1);
        return std::cosh(args[0]);
    }
    if (nombre == "tanh") {
        pedir(1, 1);
        return std::tanh(args[0]);
    }

    // --- Raíces, potencias y logaritmos ---
    if (nombre == "sqrt" || nombre == "raiz") {
        pedir(1, 2);
        const double x = args[0];
        const double indice = args.size() == 2 ? args[1] : 2.0;
        if (indice == 0.0) {
            throw ErrorCalculo("El índice de una raíz no puede ser 0");
        }
        if (x < 0) {
            if (esEntero(indice) && static_cast<long long>(indice) % 2 != 0) {
                return -std::pow(-x, 1.0 / indice);  // raíz impar de un negativo: raiz(-8, 3) = -2
            }
            throw ErrorCalculo("La raíz par de un número negativo no es un número real");
        }
        if (indice == 2.0) {
            return std::sqrt(x);
        }
        return std::pow(x, 1.0 / indice);
    }
    if (nombre == "cbrt") {
        pedir(1, 1);
        return std::cbrt(args[0]);
    }
    if (nombre == "exp") {
        pedir(1, 1);
        return std::exp(args[0]);
    }
    if (nombre == "ln") {
        pedir(1, 1);
        if (args[0] <= 0) {
            throw ErrorCalculo("El logaritmo solo existe para números mayores que 0");
        }
        return std::log(args[0]);
    }
    if (nombre == "log") {
        pedir(1, 2);
        if (args[0] <= 0) {
            throw ErrorCalculo("El logaritmo solo existe para números mayores que 0");
        }
        if (args.size() == 1) {
            return std::log10(args[0]);
        }
        if (args[1] <= 0 || args[1] == 1.0) {
            throw ErrorCalculo("La base del logaritmo debe ser mayor que 0 y distinta de 1");
        }
        return std::log(args[0]) / std::log(args[1]);
    }

    // --- Otras ---
    if (nombre == "abs") {
        pedir(1, 1);
        return std::fabs(args[0]);
    }
    if (nombre == "floor" || nombre == "piso") {
        pedir(1, 1);
        return std::floor(args[0]);
    }
    if (nombre == "ceil" || nombre == "techo") {
        pedir(1, 1);
        return std::ceil(args[0]);
    }
    if (nombre == "round" || nombre == "redondear") {
        pedir(1, 1);
        return std::round(args[0]);
    }
    if (nombre == "fact") {
        pedir(1, 1);
        return factorial(args[0]);
    }
    if (nombre == "nCr" || nombre == "comb") {
        pedir(2, 2);
        return combinaciones(args[0], args[1]);
    }
    if (nombre == "nPr" || nombre == "perm") {
        pedir(2, 2);
        return permutaciones(args[0], args[1]);
    }

    throw ErrorCalculo("La función '" + nombre + "' no existe. Escribe 'ayuda' para ver la lista");
}
