@echo off
REM Compila la calculadora. Uso:  compilar.bat          (crea bin\calculadora.exe y la abre)
REM                              compilar.bat pruebas  (compila y ejecuta las pruebas)
cd /d "%~dp0"
set "DEVCPP=C:\Program Files (x86)\Embarcadero\Dev-Cpp\TDM-GCC-64\bin\g++.exe"
set "GPP=g++"
where g++ >nul 2>nul
if not errorlevel 1 goto compilar
set "GPP=%DEVCPP%"
if exist "%GPP%" goto compilar
echo No se encontro g++. Instala Dev-C++ o MinGW y vuelve a intentarlo.
pause
exit /b 1

:compilar
if not exist bin mkdir bin
set "OPCIONES=-std=c++17 -Wall -Wextra -pedantic -O2 -static"
set "CALCULO=src\evaluador.cpp src\estadistica.cpp src\algebra.cpp"
if /i "%~1"=="pruebas" goto pruebas

"%GPP%" %OPCIONES% src\main.cpp %CALCULO% -o bin\calculadora.exe
if errorlevel 1 goto error
echo Compilado: bin\calculadora.exe
bin\calculadora.exe
exit /b 0

:pruebas
"%GPP%" %OPCIONES% tests\pruebas.cpp %CALCULO% -o bin\pruebas.exe
if errorlevel 1 goto error
bin\pruebas.exe
exit /b %errorlevel%

:error
echo Hubo un error al compilar.
pause
exit /b 1
