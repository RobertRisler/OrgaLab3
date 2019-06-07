#include "LaboratorioOrga.h"

// ########## Variables globales ##########

// ##### Registros #####
// Todos los registros parten iniciados en cero, cambian sus valores durante la ejecucion.
// ## $zero ##
int zero = 0;
// ## $at ##
int at = 0;
// ## $v0 - $v1 ##
int v0 = 0;
int v1 = 0;
// ## $a0 - $a3
int a0 = 0;
int a1 = 0;
int a2 = 0;
int a3 = 0;
// ## $t0 - $t7 ##
int t0 = 0;
int t1 = 0;
int t2 = 0;
int t3 = 0;
int t4 = 0;
int t5 = 0;
int t6 = 0;
int t7 = 0;
// ## $s0 - $s7
int s0 = 0;
int s1 = 0;
int s2 = 0;
int s3 = 0;
int s4 = 0;
int s5 = 0;
int s6 = 0;
int s7 = 0;
// ## $t8 - $t9 ##
int t8 = 0;
int t9 = 0;
// ## $k0 - $k1 ##
int k0 = 0;
int k1 = 0;
// ## $gp ##
int gp = 0;
// ## $sp ##
int sp = 1024;
// ## $fp ##
int fp = 0;
// ## $ra ##
int ra = 0;

// ##### Memoria #####
// Arreglo utilizado como memoria para guardar o cargar datos con sw y lw respectivamente.
int memoria[2048];

// ##### Etapas pipeline #####
// Punteros para indicar que instrucción está en cual etapa.
nodo *IF = NULL;
nodo *ID = NULL;
nodo *EX1 = NULL;
nodo *EX2 = NULL;
nodo *MEM = NULL;
nodo *WB = NULL;

// ##### Bandera pipeline #####
// Bandera que sirve para indicar el momento en que se vació el pipeline, se cambia a
// 1 cuando el pipeline terminó.
int banderaPipeline = 0;

// ##### Buffers #####
// Datos que es necesario guardar entre etapas.
int IFID = 0;            // (Buffer IFID)
int IDEX1[3] = {0};      // Guarda rs/rt/0 o rs/rt/immediate o rs/rt/offset (Buffer IDEX1)
int EX1EX2[2] = {0};     // Guarda el resultado de la ALU y rt (Buffer EX1EX2)
nodo *EX2MEM_PTR = NULL; // Guarda la nueva dirección de contadorDePrograma (Buffer EX2MEM)
int EX2MEM[2] = {0};     // Guarda el resultado del calculo en EX1 y rt (Buffer EX2MEM)
int MEMWB[2] = {0};      // Guarda el dato leido de memoria (Buffer MEMWB)

// ##### Flush #####
// Señal que indica la necesidad de hacer un flush, 0 indica no y 1 indica si.
int flush = 0;

// ##### Contador de Programa #####
// Puntero que hace la función del contador de programa, indica que instrucción es la última
// en entrar al pipeline
nodo *contadorDePrograma = NULL;

// ########## Funciones ##########

// Menú principal donde se pide el nombre de entrada del archivo de instrucciones, también se
// hace el llamado a las funciones principales para la ejecución.
// Entrada: Vacío.
// Salida: Vacío.
void menu()
{
    printf("\n");
    printf("########################################\n");
    printf("#                                      #\n");
    printf("#  PROGRAMA DE SIMULACION DE PIPELINE  #\n");
    printf("#                                      #\n");
    printf("########################################\n");
    printf("\n");

    char nombreArchivoInstrucciones[50];
    printf("(No olvide la extension del archivo)\nIngrese el nombre del archivo de instrucciones:\n");
    scanf("%s", nombreArchivoInstrucciones);

    // Lista donde se guardan todas las instrucciones.
    lista *memoriaInstrucciones = (lista *)malloc(sizeof(lista));
    memoriaInstrucciones->largo = 0;
    memoriaInstrucciones->inicio = NULL;
    guardarInstrucciones(nombreArchivoInstrucciones, memoriaInstrucciones);

    // Ejecución general del programa.
    ejecucionPrograma(memoriaInstrucciones);

    // Se libera la memoria de cada nodo y de la lista.
    liberarMemoria(memoriaInstrucciones);
    free(memoriaInstrucciones);

    return;
}

// Se lee el archivo de instrucciones y se guardan en una lista enlazada.
// Esta lista es utilizada como memoria de instrucciones.
// Entrada: Char con el nombre del archivo, lista a ser rellenada con instrucciones.
// Salida: Vacío.
void guardarInstrucciones(char *nombre, lista *memoriaIns)
{
    FILE *pArchivo;
    pArchivo = fopen(nombre, "r");

    char linea[50];
    while (!feof(pArchivo))
    {
        fgets(linea, 50, pArchivo);
        limpiarLinea(linea);

        char *token1, *token2, *token3, *token4;
        token1 = strtok(linea, " ");

        if (strcmp(token1, "add") == 0 || strcmp(token1, "sub") == 0)
        {
            token2 = strtok(NULL, " ");
            token3 = strtok(NULL, " ");
            token4 = strtok(NULL, " ");
            ingresarInstruccion(memoriaIns, 1, token1, token2, token3, token4);
        }
        else if (strcmp(token1, "lw") == 0 || strcmp(token1, "sw") == 0)
        {
            token2 = strtok(NULL, " ");
            token3 = strtok(NULL, " ");
            token4 = strtok(NULL, " ");
            ingresarInstruccion(memoriaIns, 2, token1, token2, token3, token4);
        }
        else if (strcmp(token1, "addi") == 0 || strcmp(token1, "subi") == 0)
        {
            token2 = strtok(NULL, " ");
            token3 = strtok(NULL, " ");
            token4 = strtok(NULL, " ");
            ingresarInstruccion(memoriaIns, 3, token1, token2, token3, token4);
        }
        else if (strcmp(token1, "beq") == 0 || strcmp(token1, "bne") == 0)
        {
            token2 = strtok(NULL, " ");
            token3 = strtok(NULL, " ");
            token4 = strtok(NULL, " ");
            ingresarInstruccion(memoriaIns, 4, token1, token2, token3, token4);
        }
        else if (strcmp(token1, "j") == 0)
        {
            token2 = strtok(NULL, " ");
            ingresarInstruccion(memoriaIns, 5, token1, token2, "placeholder", "placeholder");
        }
        else if (strchr(token1, ':') != NULL)
        {
            ingresarInstruccion(memoriaIns, 6, token1, "placeholder", "placeholder", "placeholder");
        }
    }

    fclose(pArchivo);
    return;
}

// Ingresa una instrucción a la lista enlazada.
// Entrada: Lista enlazada, entero para indicar el tipo de instrucción, char token 1-4 son las partes de la instrucción.
// Salida: Vacío.
void ingresarInstruccion(lista *memoriaIns, int tipoIns, char *token1, char *token2, char *token3, char *token4)
{
    nodo *instruccion = (nodo *)malloc(sizeof(nodo));

    switch (tipoIns)
    {
    case 1: // add, sub
        strcpy(instruccion->ins, token1);
        strcpy(instruccion->rd, token2);
        strcpy(instruccion->rs, token3);
        strcpy(instruccion->rt, token4);
        break;
    case 2: // lw, sw
        strcpy(instruccion->ins, token1);
        strcpy(instruccion->rt, token2);
        strcpy(instruccion->offset, token3);
        strcpy(instruccion->rs, token4);
        break;
    case 3: // addi, subi
        strcpy(instruccion->ins, token1);
        strcpy(instruccion->rt, token2);
        strcpy(instruccion->rs, token3);
        strcpy(instruccion->immediate, token4);
        break;
    case 4: // beq, bne
        strcpy(instruccion->ins, token1);
        strcpy(instruccion->rt, token2);
        strcpy(instruccion->rs, token3);
        strcpy(instruccion->label, token4);
        break;
    case 5: // j
        strcpy(instruccion->ins, token1);
        strcpy(instruccion->label, token2);
        break;
    case 6: // label
        strcpy(instruccion->ins, token1);
        break;
    default:
        free(instruccion);
        return;
    }

    instruccion->sgte = NULL;
    if (memoriaIns->largo == 0)
    {
        memoriaIns->inicio = instruccion;
        memoriaIns->largo = 1;
    }
    else
    {
        nodo *aux = memoriaIns->inicio;
        int j;
        for (j = 0; j < memoriaIns->largo - 1; j++)
        {
            aux = aux->sgte;
        }
        aux->sgte = instruccion;
        memoriaIns->largo++;
    }
    return;
}

// Remueve los espacios y el \n de una linea de instrucción.
// Entrada: Char correspondiente a la linea de instrucción.
// Salida: Vacío.
void limpiarLinea(char *string)
{
    while (strchr(string, ',') != NULL)
    {
        removerComa(string, ',');
    }
    if (strchr(string, '(') != NULL)
    {
        char *aux;
        aux = strchr(string, '(');
        *aux = ' ';
    }
    if (strchr(string, ')') != NULL)
    {
        char *aux;
        aux = strchr(string, ')');
        *aux = ' ';
    }
    if (strchr(string, '\n') != NULL)
    {
        char *aux;
        aux = strchr(string, '\n');
        *aux = '\0';
    }
    return;
}

// Remueve las comas de una linea de instrucción.
// Entrada: Char correspondiente a la linea de instrucción y char a remover.
// Salida: Vacío.
void removerComa(char *string, char basura)
{
    char *aux1, *aux2;
    for (aux1 = aux2 = string; *aux1 != '\0'; aux1++)
    {
        *aux2 = *aux1;
        if (*aux2 != basura)
        {
            aux2++;
        }
    }
    *aux2 = '\0';
    return;
}

// Etapa principal de ejecución, se escriben los archivos solicitados y se llaman a las funciones que
// simulan al pipeline.
// Entrada: Lista enlazada con las instrucciones.
// Salida: Vacío.
void ejecucionPrograma(lista *memoriaIns)
{
    FILE *pArchivo;
    pArchivo = fopen("RIESGOS_DETECTADOS.txt", "w");
    fprintf(pArchivo, "Funcionalidad no implementada.");
    fclose(pArchivo);

    pArchivo = fopen("SOLUCION_RIESGOS.txt", "w");
    fprintf(pArchivo, "Funcionalidad no implementada.");
    fclose(pArchivo);

    pArchivo = fopen("REGISTROS.txt", "w");

    rellenarMemoria();
    contadorDePrograma = memoriaIns->inicio;

    int numeroCiclo = 0;

    while (banderaPipeline == 0)
    {
        numeroCiclo++;

        etapaIF(numeroCiclo, memoriaIns);

        if (WB != NULL) // Etapa WB -> Escribir Registro
        {
            etapaWB();
        }

        if (MEM != NULL) // Etapa MEM -> Escribir/Leer Memoria
        {
            etapaMEM();
        }

        if (EX2 != NULL) // Etapa EX2 -> Dirección
        {
            etapaEX2(memoriaIns);
        }

        if (EX1 != NULL) // Etapa EX1 -> ALU
        {
            etapaEX1();
        }

        if (ID != NULL) // Etapa ID -> Leer Registros
        {
            etapaID();
        }
    }

    escribirRegistros(&pArchivo);
    fclose(pArchivo);

    printf("Se escribieron los archivos solicitados.\n");
    return;
}

// Ejecución de la etapa Instruction Fetch. Se mueven los punteros que indican en que etapa se encuentra
// dicha instrucción.
// Entrada: Entero con el número del ciclo actual, lista enlazada con las instrucciones.
// Salida: Vacío.
void etapaIF(int numeroCiclo, lista *memoriaIns)
{
    if (IF == NULL && ID == NULL && EX1 == NULL && EX2 == NULL && MEM == NULL && WB == NULL && numeroCiclo == 1)
    {
        IF = memoriaIns->inicio;
    }
    else if (IF != NULL && ID == NULL && EX1 == NULL && EX2 == NULL && MEM == NULL && WB == NULL)
    {
        ID = IF;
        IF = IF->sgte;
    }
    else if (IF != NULL && ID != NULL && EX1 == NULL && EX2 == NULL && MEM == NULL && WB == NULL)
    {
        EX1 = ID;
        ID = IF;
        IF = IF->sgte;
    }
    else if (IF != NULL && ID != NULL && EX1 != NULL && EX2 == NULL && MEM == NULL && WB == NULL)
    {
        EX2 = EX1;
        EX1 = ID;
        ID = IF;
        IF = IF->sgte;
    }
    else if (IF != NULL && ID != NULL && EX1 != NULL && EX2 != NULL && MEM == NULL && WB == NULL)
    {
        MEM = EX2;
        EX2 = EX1;
        EX1 = ID;
        ID = IF;
        IF = IF->sgte;
    }
    else if (IF != NULL && ID != NULL && EX1 != NULL && EX2 != NULL && MEM != NULL && WB == NULL)
    {
        WB = MEM;
        MEM = EX2;
        EX2 = EX1;
        EX1 = ID;
        ID = IF;
        IF = IF->sgte;
    }
    else if (IF != NULL && ID != NULL && EX1 != NULL && EX2 != NULL && MEM != NULL && WB != NULL)
    {
        WB = MEM;
        MEM = EX2;
        EX2 = EX1;
        EX1 = ID;
        ID = IF;
        IF = IF->sgte;
    }
    else if (IF == NULL && ID != NULL && EX1 != NULL && EX2 != NULL && MEM != NULL && WB != NULL)
    {
        WB = MEM;
        MEM = EX2;
        EX2 = EX1;
        EX1 = ID;
        ID = IF;
    }
    else if (IF == NULL && ID == NULL && EX1 != NULL && EX2 != NULL && MEM != NULL && WB != NULL)
    {
        WB = MEM;
        MEM = EX2;
        EX2 = EX1;
        EX1 = ID;
    }
    else if (IF == NULL && ID == NULL && EX1 == NULL && EX2 != NULL && MEM != NULL && WB != NULL)
    {
        WB = MEM;
        MEM = EX2;
        EX2 = EX1;
    }
    else if (IF == NULL && ID == NULL && EX1 == NULL && EX2 == NULL && MEM != NULL && WB != NULL)
    {
        WB = MEM;
        MEM = EX2;
    }
    else if (IF == NULL && ID == NULL && EX1 == NULL && EX2 == NULL && MEM == NULL && WB != NULL)
    {
        WB = MEM;
        banderaPipeline = 1;
    }

    return;
}

// Ejecución de la etapa Instruction Decode. Se guardan los datos necesarios para la instrucción en el
// buffer IDEX1.
// Entrada: Vacío.
// Salida: Vacío.
void etapaID()
{
    if (strcmp(ID->ins, "add") == 0 || strcmp(ID->ins, "sub") == 0 || strcmp(ID->ins, "beq") == 0 || strcmp(ID->ins, "bne") == 0)
    {
        IDEX1[0] = obtenerDato(ID->rs);
        IDEX1[1] = obtenerDato(ID->rt);
        IDEX1[2] = 0;
    }
    else if (strcmp(ID->ins, "addi") == 0 || strcmp(ID->ins, "subi") == 0)
    {
        IDEX1[0] = obtenerDato(ID->rs);
        IDEX1[1] = obtenerDato(ID->rt);
        IDEX1[2] = atoi(ID->immediate);
    }
    else if (strcmp(ID->ins, "lw") == 0 || strcmp(ID->ins, "sw") == 0)
    {
        IDEX1[0] = obtenerDato(ID->rs);
        IDEX1[1] = obtenerDato(ID->rt);
        IDEX1[2] = atoi(ID->offset);
    }
    else if (strcmp(ID->ins, "j") == 0)
    {
        IDEX1[0] = 0;
        IDEX1[1] = 0;
        IDEX1[2] = 0;
    }

    if (ID->sgte == NULL)
    {
        IFID = 0;
    }
    
    return;
}

// Ejecución de la etapa Execution 1. Se hacen cálculos relacionados a la ALU y se guaran en el
// buffer EX1EX2.
// Entrada: Vacío.
// Salida: Vacío.
void etapaEX1()
{
    EX1EX2[0] = IDEX1[1];

    if (strcmp(EX1->ins, "add") == 0)
    {
        EX1EX2[1] = IDEX1[0] + IDEX1[1];
    }
    else if (strcmp(EX1->ins, "addi") == 0)
    {
        EX1EX2[1] = IDEX1[0] + IDEX1[2];
    }
    else if (strcmp(EX1->ins, "sub") == 0 || strcmp(EX1->ins, "beq") == 0 || strcmp(EX1->ins, "bne") == 0)
    {
        EX1EX2[1] = IDEX1[0] - IDEX1[1];
    }
    else if (strcmp(EX1->ins, "subi") == 0)
    {
        EX1EX2[1] = IDEX1[0] - IDEX1[2];
    }
    else if (strcmp(EX1->ins, "lw") == 0 || strcmp(EX1->ins, "sw") == 0)
    {
        EX1EX2[1] = (IDEX1[0] + IDEX1[1]) / 4;
    }
    else if (strcmp(EX1->ins, "j") == 0)
    {
        EX1EX2[1] = 0;
    }

    if (EX1->sgte == NULL)
    {
        IDEX1[0] = 0;
        IDEX1[1] = 0;
        IDEX1[2] = 0;
    }

    return;
}

// Ejecución de la etapa Execution 2. Se busca la etiqueta correspondiente al salto o branch.
// Si es necesario, se guarda en buffer el puntero hacía donde se debe mover el contador de programa.
// Entrada: Lista enlazada con las instrucciones.
// Salida: Vacío.
void etapaEX2(lista *memoriaIns)
{
    EX2MEM[0] = EX1EX2[0];
    EX2MEM[1] = EX1EX2[1];

    if (((strcmp(EX2->ins, "beq") == 0) && (EX1EX2[1] == 0)) || /* beq y la resta dio cero */
        ((strcmp(EX2->ins, "bne") == 0) && (EX1EX2[1] != 0)) || /* bne y la resta no dio cero */
        ((strcmp(EX2->ins, "j") == 0)))                         /* salto incondicional */
    {
        char *etiqueta = EX2->label;
        int bandera = 0;

        nodo *auxiliar = memoriaIns->inicio;
        strcat(etiqueta, ":");

        while (bandera != 1 && auxiliar != NULL)
        {
            if ((strchr(auxiliar->ins, ':') != NULL) && (strcmp(auxiliar->ins, etiqueta) == 0))
            {
                EX2MEM_PTR = auxiliar;
                bandera = 1;
                etiqueta = NULL;
            }
            else
            {
                auxiliar = auxiliar->sgte;
            }
        }
    }
    else
    {
        EX2MEM_PTR = NULL;
    }

    if (EX2->sgte == NULL)
    {
        EX1EX2[0] = 0;
        EX1EX2[1] = 0;
    }

    return;
}

// Ejecución de la etapa Memory Access. Se escribe o lee un dato de la memoria de datos y si es necesario
// se escribe en el buffer MEMWB el dato leido.
// Entrada: Vacío.
// Salida: Vacío.
void etapaMEM()
{
    if (strcmp(MEM->ins, "lw") == 0)
    {
        MEMWB[0] = 0;
        MEMWB[1] = memoria[EX2MEM[1]];
    }
    else if (strcmp(MEM->ins, "sw") == 0)
    {
        MEMWB[0] = 0;
        MEMWB[1] = 0;
        memoria[EX2MEM[1]] = EX2MEM[0];
    }
    else if (strcmp(MEM->ins, "beq") == 0 || strcmp(MEM->ins, "bne") == 0 || strcmp(MEM->ins, "j") == 0)
    {
        MEMWB[0] = 0;
        MEMWB[1] = 0;
        contadorDePrograma = EX2MEM_PTR;
        // Hacer función flush
    }
    else
    {
        MEMWB[0] = EX2MEM[1];
        MEMWB[1] = 0;
    }

    if (MEM->sgte == NULL)
    {
        EX2MEM[0] = 0;
        EX2MEM[1] = 0;
    }

    return;
}

// Ejecución de la etapa Write Back. Escribe sobre un registro un dato guardado en el buffer MEMWB.
// Entrada: Vacío.
// Salida: Vacío.
void etapaWB()
{
    if (strcmp(WB->ins, "add") == 0 || strcmp(WB->ins, "sub") == 0)
    {
        *obtenerReferencia(WB->rd) = MEMWB[0];
    }
    else if (strcmp(WB->ins, "addi") == 0 || strcmp(WB->ins, "subi") == 0)
    {
        *obtenerReferencia(WB->rt) = MEMWB[0];
    }
    else if (strcmp(WB->ins, "lw") == 0)
    {
        *obtenerReferencia(WB->rt) = MEMWB[1];
    }

    if (contadorDePrograma != NULL)
    {
        contadorDePrograma = contadorDePrograma->sgte;
    }

    if (WB->sgte == NULL)
    {
        MEMWB[0] = 0;
        MEMWB[1] = 0;
    }

    return;
}

// Se obtiene la referencia de donde se guarda el dato del registro solicitado.
// Entrada: Char que indica el registro al que se refiere.
// Salida: Referencia a entero.
int *obtenerReferencia(char *string)
{
    if (strcmp(string, "$zero") == 0)
    {
        return &zero;
    }
    else if (strcmp(string, "$at") == 0)
    {
        return &at;
    }
    else if (strcmp(string, "$v0") == 0)
    {
        return &v0;
    }
    else if (strcmp(string, "$v1") == 0)
    {
        return &v1;
    }
    else if (strcmp(string, "$a0") == 0)
    {
        return &a0;
    }
    else if (strcmp(string, "$a1") == 0)
    {
        return &a1;
    }
    else if (strcmp(string, "$a2") == 0)
    {
        return &a2;
    }
    else if (strcmp(string, "$a3") == 0)
    {
        return &a3;
    }
    else if (strcmp(string, "$t0") == 0)
    {
        return &t0;
    }
    else if (strcmp(string, "$t1") == 0)
    {
        return &t1;
    }
    else if (strcmp(string, "$t2") == 0)
    {
        return &t2;
    }
    else if (strcmp(string, "$t3") == 0)
    {
        return &t3;
    }
    else if (strcmp(string, "$t4") == 0)
    {
        return &t4;
    }
    else if (strcmp(string, "$t5") == 0)
    {
        return &t5;
    }
    else if (strcmp(string, "$t6") == 0)
    {
        return &t6;
    }
    else if (strcmp(string, "$t7") == 0)
    {
        return &t7;
    }
    else if (strcmp(string, "$s0") == 0)
    {
        return &s0;
    }
    else if (strcmp(string, "$s1") == 0)
    {
        return &s1;
    }
    else if (strcmp(string, "$s2") == 0)
    {
        return &s2;
    }
    else if (strcmp(string, "$s3") == 0)
    {
        return &s3;
    }
    else if (strcmp(string, "$s4") == 0)
    {
        return &s4;
    }
    else if (strcmp(string, "$s5") == 0)
    {
        return &s5;
    }
    else if (strcmp(string, "$s6") == 0)
    {
        return &s6;
    }
    else if (strcmp(string, "$s7") == 0)
    {
        return &s7;
    }
    else if (strcmp(string, "$t8") == 0)
    {
        return &t8;
    }
    else if (strcmp(string, "$t9") == 0)
    {
        return &t9;
    }
    else if (strcmp(string, "$k0") == 0)
    {
        return &k0;
    }
    else if (strcmp(string, "$k1") == 0)
    {
        return &k1;
    }
    else if (strcmp(string, "$gp") == 0)
    {
        return &gp;
    }
    else if (strcmp(string, "$sp") == 0)
    {
        return &sp;
    }
    else if (strcmp(string, "$fp") == 0)
    {
        return &fp;
    }
    else if (strcmp(string, "$ra") == 0)
    {
        return &ra;
    }
    else
    {
        return 0;
    }
}

// Se obtiene el dato dentro de un registro solicitado.
// Entrada: Char que indica el registro al que se refiere.
// Salida. Entero.
int obtenerDato(char *string)
{
    if (strcmp(string, "$zero") == 0)
    {
        return zero;
    }
    else if (strcmp(string, "$at") == 0)
    {
        return at;
    }
    else if (strcmp(string, "$v0") == 0)
    {
        return v0;
    }
    else if (strcmp(string, "$v1") == 0)
    {
        return v1;
    }
    else if (strcmp(string, "$a0") == 0)
    {
        return a0;
    }
    else if (strcmp(string, "$a1") == 0)
    {
        return a1;
    }
    else if (strcmp(string, "$a2") == 0)
    {
        return a2;
    }
    else if (strcmp(string, "$a3") == 0)
    {
        return a3;
    }
    else if (strcmp(string, "$t0") == 0)
    {
        return t0;
    }
    else if (strcmp(string, "$t1") == 0)
    {
        return t1;
    }
    else if (strcmp(string, "$t2") == 0)
    {
        return t2;
    }
    else if (strcmp(string, "$t3") == 0)
    {
        return t3;
    }
    else if (strcmp(string, "$t4") == 0)
    {
        return t4;
    }
    else if (strcmp(string, "$t5") == 0)
    {
        return t5;
    }
    else if (strcmp(string, "$t6") == 0)
    {
        return t6;
    }
    else if (strcmp(string, "$t7") == 0)
    {
        return t7;
    }
    else if (strcmp(string, "$s0") == 0)
    {
        return s0;
    }
    else if (strcmp(string, "$s1") == 0)
    {
        return s1;
    }
    else if (strcmp(string, "$s2") == 0)
    {
        return s2;
    }
    else if (strcmp(string, "$s3") == 0)
    {
        return s3;
    }
    else if (strcmp(string, "$s4") == 0)
    {
        return s4;
    }
    else if (strcmp(string, "$s5") == 0)
    {
        return s5;
    }
    else if (strcmp(string, "$s6") == 0)
    {
        return s6;
    }
    else if (strcmp(string, "$s7") == 0)
    {
        return s7;
    }
    else if (strcmp(string, "$t8") == 0)
    {
        return t8;
    }
    else if (strcmp(string, "$t9") == 0)
    {
        return t9;
    }
    else if (strcmp(string, "$k0") == 0)
    {
        return k0;
    }
    else if (strcmp(string, "$k1") == 0)
    {
        return k1;
    }
    else if (strcmp(string, "$gp") == 0)
    {
        return gp;
    }
    else if (strcmp(string, "$sp") == 0)
    {
        return sp;
    }
    else if (strcmp(string, "$fp") == 0)
    {
        return fp;
    }
    else if (strcmp(string, "$ra") == 0)
    {
        return ra;
    }
    else
    {
        return 0;
    }
}

// Se rellena la memoria de datos con 0.
// Entrada: Vacío.
// Salida: Vacío.
void rellenarMemoria()
{
    int i;
    for (i = 0; i < 1024; i++)
    {
        memoria[i] = 0;
    }
    return;
}

// Se escribe una instruccion dentro de un archivo.
// Entrada: Puntero a archivo a escribir, puntero a nodo que contiene la instrucción.
// Salida: Vacío.
void escribirInstruccion(FILE **pArchivo, nodo *instruccion)
{
    if (strcmp(instruccion->ins, "add") == 0 || strcmp(instruccion->ins, "sub") == 0 || strcmp(instruccion->ins, "and") == 0 ||
        strcmp(instruccion->ins, "or") == 0 || strcmp(instruccion->ins, "slt") == 0)
    {
        fprintf(*pArchivo, "%s %s, %s, %s\n", instruccion->ins, instruccion->rd, instruccion->rs, instruccion->rt);
    }
    else if (strcmp(instruccion->ins, "lw") == 0 || strcmp(instruccion->ins, "sw") == 0)
    {
        fprintf(*pArchivo, "%s %s, %s(%s)\n", instruccion->ins, instruccion->rt, instruccion->offset, instruccion->rs);
    }
    else if (strcmp(instruccion->ins, "addi") == 0)
    {
        fprintf(*pArchivo, "%s %s, %s, %s\n", instruccion->ins, instruccion->rt, instruccion->rs, instruccion->immediate);
    }
    else if (strcmp(instruccion->ins, "beq") == 0)
    {
        fprintf(*pArchivo, "%s %s, %s, %s\n", instruccion->ins, instruccion->rt, instruccion->rs, instruccion->label);
    }
    else if (strcmp(instruccion->ins, "j") == 0)
    {
        fprintf(*pArchivo, "%s %s\n", instruccion->ins, instruccion->label);
    }
    else if (strchr(instruccion->ins, ':') != NULL)
    {
        fprintf(*pArchivo, "%s\n", instruccion->ins);
    }
    return;
}

// Se escriben los valores de los registros dentro de un archivo.
// Entrada: Puntero a archivo a escribir.
// Salida: Vacío.
void escribirRegistros(FILE **pArchivo)
{
    fprintf(*pArchivo, "$zero = %d\n", zero);
    fprintf(*pArchivo, "$at = %d\n", at);
    fprintf(*pArchivo, "$v0 = %d\n", v0);
    fprintf(*pArchivo, "$v1 = %d\n", v1);
    fprintf(*pArchivo, "$a0 = %d\n", a0);
    fprintf(*pArchivo, "$a1 = %d\n", a1);
    fprintf(*pArchivo, "$a2 = %d\n", a2);
    fprintf(*pArchivo, "$a3 = %d\n", a3);
    fprintf(*pArchivo, "$t0 = %d\n", t0);
    fprintf(*pArchivo, "$t1 = %d\n", t1);
    fprintf(*pArchivo, "$t2 = %d\n", t2);
    fprintf(*pArchivo, "$t3 = %d\n", t3);
    fprintf(*pArchivo, "$t4 = %d\n", t4);
    fprintf(*pArchivo, "$t5 = %d\n", t5);
    fprintf(*pArchivo, "$t6 = %d\n", t6);
    fprintf(*pArchivo, "$t7 = %d\n", t7);
    fprintf(*pArchivo, "$s0 = %d\n", s0);
    fprintf(*pArchivo, "$s1 = %d\n", s1);
    fprintf(*pArchivo, "$s2 = %d\n", s2);
    fprintf(*pArchivo, "$s3 = %d\n", s3);
    fprintf(*pArchivo, "$s4 = %d\n", s4);
    fprintf(*pArchivo, "$s5 = %d\n", s5);
    fprintf(*pArchivo, "$s6 = %d\n", s6);
    fprintf(*pArchivo, "$s7 = %d\n", s7);
    fprintf(*pArchivo, "$t8 = %d\n", t8);
    fprintf(*pArchivo, "$t9 = %d\n", t9);
    fprintf(*pArchivo, "$k0 = %d\n", k0);
    fprintf(*pArchivo, "$k1 = %d\n", k1);
    fprintf(*pArchivo, "$gp = %d\n", gp);
    fprintf(*pArchivo, "$sp = %d\n", sp);
    fprintf(*pArchivo, "$fp = %d\n", fp);
    fprintf(*pArchivo, "$ra = %d", ra);
    return;
}

// Se muestra por pantalla una instrucción.
// Entrada: Puntero a nodo con la instrucción.
// Salida: Vacío.
void imprimirInstruccion(nodo *instruccion)
{
    if (strcmp(instruccion->ins, "add") == 0 || strcmp(instruccion->ins, "sub") == 0 || strcmp(instruccion->ins, "and") == 0 ||
        strcmp(instruccion->ins, "or") == 0 || strcmp(instruccion->ins, "slt") == 0)
    {
        printf("%s %s, %s, %s\n", instruccion->ins, instruccion->rd, instruccion->rs, instruccion->rt);
    }
    else if (strcmp(instruccion->ins, "lw") == 0 || strcmp(instruccion->ins, "sw") == 0)
    {
        printf("%s %s, %s(%s)\n", instruccion->ins, instruccion->rt, instruccion->offset, instruccion->rs);
    }
    else if (strcmp(instruccion->ins, "addi") == 0)
    {
        printf("%s %s, %s, %s\n", instruccion->ins, instruccion->rt, instruccion->rs, instruccion->immediate);
    }
    else if (strcmp(instruccion->ins, "beq") == 0)
    {
        printf("%s %s, %s, %s\n", instruccion->ins, instruccion->rt, instruccion->rs, instruccion->label);
    }
    else if (strcmp(instruccion->ins, "j") == 0)
    {
        printf("%s %s\n", instruccion->ins, instruccion->label);
    }
    else if (strchr(instruccion->ins, ':') != NULL)
    {
        printf("%s\n", instruccion->ins);
    }
    return;
}

// Se muestra por pantalla la memoria de instrucciones.
// Entrada: Lista enlazada con las instrucciones.
// Salida: Vacío.
void imprimirMemoriaInstrucciones(lista *memoriaIns)
{
    int i;
    printf("largo memoria: %d\n", memoriaIns->largo);
    nodo *aux = memoriaIns->inicio;
    for (i = 0; i < memoriaIns->largo; i++)
    {
        imprimirInstruccion(aux);
        aux = aux->sgte;
    }
    return;
}

// Se muestran por pantalla los registros y sus valores.
// Entrada: Vacío.
// Salida: Vacío.
void imprimirRegistros()
{
    printf("$zero = %d\n", zero);
    printf("$at = %d\n", at);
    printf("$v0 = %d\n", v0);
    printf("$v1 = %d\n", v1);
    printf("$a0 = %d\n", a0);
    printf("$a1 = %d\n", a1);
    printf("$a2 = %d\n", a2);
    printf("$a3 = %d\n", a3);
    printf("$t0 = %d\n", t0);
    printf("$t1 = %d\n", t1);
    printf("$t2 = %d\n", t2);
    printf("$t3 = %d\n", t3);
    printf("$t4 = %d\n", t4);
    printf("$t5 = %d\n", t5);
    printf("$t6 = %d\n", t6);
    printf("$t7 = %d\n", t7);
    printf("$s0 = %d\n", s0);
    printf("$s1 = %d\n", s1);
    printf("$s2 = %d\n", s2);
    printf("$s3 = %d\n", s3);
    printf("$s4 = %d\n", s4);
    printf("$s5 = %d\n", s5);
    printf("$s6 = %d\n", s6);
    printf("$s7 = %d\n", s7);
    printf("$t8 = %d\n", t8);
    printf("$t9 = %d\n", t9);
    printf("$k0 = %d\n", k0);
    printf("$k1 = %d\n", k1);
    printf("$gp = %d\n", gp);
    printf("$sp = %d\n", sp);
    printf("$fp = %d\n", fp);
    printf("$ra = %d\n", ra);
    return;
}

// Se muestra por pantalla el pipeline y que instrucción se encuentra en que etapa.
// Entrada: Vacío.
// Salida: Vacío.
void imprimirPipeline()
{
    printf("IF -> %s\n", IF->ins);
    printf("ID -> %s\n", ID->ins);
    printf("EX1 -> %s\n", EX1->ins);
    printf("EX2 -> %s\n", EX2->ins);
    printf("MEM -> %s\n", MEM->ins);
    printf("WB -> %s\n\n", WB->ins);

    return;
}

// Se muestra por pantalla los valores que se encuentran en los distintos buffers.
// Entrada: Vacío.
// Salida: Vacío.
void imprimirBuffer()
{
    printf("IFID -> |%d|\n", IFID);
    printf("IDEX1 -> |%d|%d|%d|\n", IDEX1[0], IDEX1[1], IDEX1[2]);
    printf("EX1EX2 -> |%d|%d|\n", EX1EX2[0], EX1EX2[1]);
    printf("EX2MEM -> |%d|%d|\n", EX2MEM[0], EX2MEM[1]);
    printf("MEMWB -> |%d|%d|\n\n", MEMWB[0], MEMWB[1]);

    return;
}

// Se libera la memoria solicitada para los nodos y la lista enlaraza.
// Entrada: Lista enlazada con las instrucciones.
// Salida: Vacío.
void liberarMemoria(lista *memoriaIns)
{
    nodo *aux1 = memoriaIns->inicio;
    nodo *aux2;

    while (aux1 != NULL)
    {
        aux2 = aux1;
        aux1 = aux1->sgte;
        free(aux2);
    }

    return;
}