# Compilar con:  make            (crea bin/calculadora)
# Pruebas:       make pruebas
CXX      ?= g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2
CALCULO  := src/evaluador.cpp src/estadistica.cpp src/algebra.cpp

bin/calculadora: src/main.cpp $(CALCULO) src/*.h
	mkdir -p bin
	$(CXX) $(CXXFLAGS) src/main.cpp $(CALCULO) -o $@

bin/pruebas: tests/pruebas.cpp $(CALCULO) src/*.h
	mkdir -p bin
	$(CXX) $(CXXFLAGS) tests/pruebas.cpp $(CALCULO) -o $@

.PHONY: pruebas limpiar
pruebas: bin/pruebas
	./bin/pruebas

limpiar:
	rm -rf bin
