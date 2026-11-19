// Evaluador de expresiones matemáticas.
// Convierte un texto como "2*sin(30) + sqrt(16)^2" en un número,
// respetando la precedencia de operadores y los paréntesis.
#ifndef EVALUADOR_H
#define EVALUADOR_H

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

// Error de cálculo o de escritura, con un mensaje pensado para el usuario.
class ErrorCalculo : public std::runtime_error {
public:
    explicit ErrorCalculo(const std::string& mensaje) : std::runtime_error(mensaje) {
    }
};

enum class ModoAngular { Grados, Radianes };

class Evaluador {
public:
    // Evalúa la expresión y guarda el resultado en "ans". Lanza ErrorCalculo si hay un error.
    double evaluar(const std::string& expresion);

    void setModo(ModoAngular modo);
    ModoAngular modo() const;
    double ultimoResultado() const;

private:
    enum class TipoToken { Numero, Identificador, Operador, ParentesisAbre, ParentesisCierra, Coma, Fin };

    struct Token {
        TipoToken tipo;
        std::string texto;
        double valor;
        std::size_t posicion;  // posición en el texto (empezando en 1), para los mensajes de error
    };

    std::vector<Token> tokens_;
    std::size_t actual_ = 0;
    ModoAngular modo_ = ModoAngular::Grados;
    double ans_ = 0.0;

    // Análisis léxico: separa el texto en números, nombres y operadores.
    void tokenizar(const std::string& texto);
    const Token& ver() const;
    Token avanzar();
    bool esOperador(const std::string& operador) const;
    bool empiezaMultiplicacionImplicita() const;

    // Análisis sintáctico (descenso recursivo). De menor a mayor precedencia:
    //   expresion := termino (('+' | '-') termino)*
    //   termino   := unario (('*' | '/' | '%') unario | multiplicación implícita)*
    //   unario    := ('-' | '+') unario | potencia
    //   potencia  := postfijo ('^' unario)?          (asociativa por la derecha)
    //   postfijo  := primario '!'*
    //   primario  := número | constante | función '(' argumentos ')' | '(' expresion ')'
    double expresion();
    double termino();
    double unario();
    double potencia();
    double postfijo();
    double primario();

    double llamarFuncion(const std::string& nombre, const std::vector<double>& argumentos, std::size_t posicion);
    double aRadianes(double angulo) const;
    double desdeRadianes(double angulo) const;
};

// Muestra un número con hasta 12 cifras significativas, sin ceros de más.
std::string formatearNumero(double valor);

// n! para enteros entre 0 y 170. Lanza ErrorCalculo si n no es válido.
double factorial(double n);

// Combinaciones C(n, r) y permutaciones P(n, r).
double combinaciones(double n, double r);
double permutaciones(double n, double r);

#endif
