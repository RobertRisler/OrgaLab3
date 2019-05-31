#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estructuras
typedef struct Nodo
{
    char ins[10];          // add, sub, and, etc...
    char rs[10];           // Primer operando
    char rt[10];           // Segundo operando / registro destino
    char rd[10];           // Registro destino
    char label[10];        // Etiqueta para instrucciones beq / j
    char offset[10];       // Dato para lw / sw
    char immediate[10];    // Valor para instrucción addi
    struct Nodo *sgte;
} nodo;

typedef struct Lista
{
    int largo;      // Largo total de la lista
    nodo *inicio;
} lista;

// Cabeceras
void menu();
void guardarInstrucciones(char *nombre, lista *memoriaIns);
void ingresarInstruccion(lista *memoriaIns, int tipoIns, char *token1, char *token2, char *token3, char *token4);
void limpiarLinea(char *string);
void removerComa(char *string, char basura);

void ejecucionPrograma(lista *memoriaIns, char *salida);
void ejecutarInstruccion(nodo *instruccion);

int *obtenerReferencia(char *string);
int obtenerDato(char *string);

void rellenarMemoria();

void escribirInstruccion(FILE **pArchivo, nodo *instruccion);
void escribirRegistros(FILE **pArchivo);

void imprimirInstruccion(nodo *instruccion);
void imprimirMemoriaInstrucciones(lista *memoriaIns);
void imprimirRegistros();
void liberarMemoria(lista *memoriaIns);