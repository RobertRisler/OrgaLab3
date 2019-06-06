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

// ##### Buffers #####
// Datos que es necesario guardar entre etapas.
int IFID = 0;            // (Buffer IFID)
int IDEX1[2] = {0};      // Guarda rs/rt o rs/immediate o rs/offset (Buffer IDEX1)
int EX1EX2 = 0;          // Guarda el resultado de la ALU (Buffer EX1EX2)
nodo *EX2MEM_PTR = NULL; // Guarda la nueva dirección de contadorDePrograma (Buffer EX2MEM)
int EX2MEM_DIR = 0;      // Guarda la dirección calculada en EX1 (Buffer EX2MEM)
int MEMWB = 0;           // Guarda el dato leido de memoria (Buffer MEMWB)

// ##### Flush #####
// Señal que indica la necesidad de hacer un flush, 0 indica no y 1 indica si.
int flush = 0;

// ########## Main ##########
int main()
{
    menu();
    return 0;
}

// ########## Funciones ##########
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
    printf("\n(No olvide la extension del archivo)\nIngrese el nombre del archivo de instrucciones:\n");
    scanf("%s", nombreArchivoInstrucciones);

    char nombreArchivoSalida[50];
    printf("\n(No olvide la extension del archivo)\nIngrese el nombre del archivo de salida:\n");
    scanf("%s", nombreArchivoSalida);

    // Lista donde se guardan todas las instrucciones.
    lista *memoriaInstrucciones = (lista *)malloc(sizeof(lista));
    memoriaInstrucciones->largo = 0;
    memoriaInstrucciones->inicio = NULL;
    //guardarInstrucciones(nombreArchivoInstrucciones, memoriaInstrucciones);
    guardarInstrucciones("instrucciones.txt", memoriaInstrucciones);

    // Ejecución general del programa.
    ejecucionPrograma(memoriaInstrucciones, nombreArchivoSalida);

    // Se libera la memoria de cada nodo y de la lista.
    liberarMemoria(memoriaInstrucciones);
    free(memoriaInstrucciones);

    return;
}

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

        if (strcmp(token1, "add") == 0 || strcmp(token1, "sub") == 0 || strcmp(token1, "and") == 0 ||
            strcmp(token1, "or") == 0 || strcmp(token1, "slt") == 0)
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
        else if (strcmp(token1, "addi") == 0)
        {
            token2 = strtok(NULL, " ");
            token3 = strtok(NULL, " ");
            token4 = strtok(NULL, " ");
            ingresarInstruccion(memoriaIns, 3, token1, token2, token3, token4);
        }
        else if (strcmp(token1, "beq") == 0)
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

void ingresarInstruccion(lista *memoriaIns, int tipoIns, char *token1, char *token2, char *token3, char *token4)
{
    nodo *instruccion = (nodo *)malloc(sizeof(nodo));

    switch (tipoIns)
    {
    case 1: // add, sub, and, or, slt
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
    case 3: // addi
        strcpy(instruccion->ins, token1);
        strcpy(instruccion->rt, token2);
        strcpy(instruccion->rs, token3);
        strcpy(instruccion->immediate, token4);
        break;
    case 4: // beq
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

void ejecucionPrograma(lista *memoriaIns, char *archivo)
{
    FILE *pArchivo;
    pArchivo = fopen(archivo, "w");
    fprintf(pArchivo, "- - - - - - - - - - INICIO DEL PROGRAMA - - - - - - - - - -\n");

    rellenarMemoria();
    nodo *contadorDePrograma = memoriaIns->inicio;

    int numeroCiclo = 0;
    int bandera = 1;

    do
    {
        numeroCiclo++;

        // Etapa IF -> Mover punteros

        // Etapa ID -> Leer Registros

        // Etapa EX1 -> ALU

        // Etapa EX2 -> Dirección

        // Etapa MEM -> Escribir/Leer Memoria

        // Etapa WB -> Escribir Registro

        if (contadorDePrograma != NULL)
        {
            contadorDePrograma = contadorDePrograma->sgte;
        }

    } while (bandera == 1);

    fclose(pArchivo);
    return;
}

void etapaIF(int numeroCiclo, lista *memoriaIns, int *bandera)
{
    // Inicio del programa
    if (IF == NULL && ID == NULL && EX1 == NULL && EX2 == NULL && MEM == NULL && WB == NULL && numeroCiclo == 1)
    {
        IF = memoriaIns->inicio;
    }
    // Primeras dos instrucciones
    else if (IF != NULL && ID == NULL && EX1 == NULL && EX2 == NULL && MEM == NULL && WB == NULL)
    {
        ID = IF;
        IF = IF->sgte;
    }
    // Primeras tres instrucciones
    else if (IF != NULL && ID != NULL && EX1 == NULL && EX2 == NULL && MEM == NULL && WB == NULL)
    {
        EX1 = ID;
        ID = IF;
        IF = IF->sgte;
    }
    // Primeras cuatro instrucciones
    else if (IF != NULL && ID != NULL && EX1 != NULL && EX2 == NULL && MEM == NULL && WB == NULL)
    {
        EX2 = EX1;
        EX1 = ID;
        ID = IF;
        IF = IF->sgte;
    }
    // Primeras cinco instrucciones
    else if (IF != NULL && ID != NULL && EX1 != NULL && EX2 != NULL && MEM == NULL && WB == NULL)
    {
        MEM = EX2;
        EX2 = EX1;
        EX1 = ID;
        ID = IF;
        IF = IF->sgte;
    }
    // Primeras seis instrucciones
    else if (IF != NULL && ID != NULL && EX1 != NULL && EX2 != NULL && MEM != NULL && WB == NULL)
    {
        WB = MEM;
        MEM = EX2;
        EX2 = EX1;
        EX1 = ID;
        ID = IF;
        IF = IF->sgte;
    }
    // Instrucciones siguientes
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
    }
    else if (IF == NULL && ID == NULL && EX1 == NULL && EX2 == NULL && MEM == NULL && WB == NULL && numeroCiclo != 1)
    {
        bandera = 0;
    }

    return;
}

void etapaID()
{
    if (strcmp(ID->ins, "add") == 0 || strcmp(ID->ins, "sub") == 0 || strcmp(ID->ins, "beq") == 0 || strcmp(ID->ins, "bne") == 0)
    {
        IDEX1[0] = obtenerDato(ID->rs);
        IDEX1[1] = obtenerDato(ID->rt);
    }
    else if (strcmp(ID->ins, "addi") == 0 || strcmp(ID->ins, "subi") == 0)
    {
        IDEX1[0] = obtenerDato(ID->rs);
        IDEX1[1] = atoi(ID->immediate);
    }
    else if (strcmp(ID->ins, "lw") == 0 || strcmp(ID->ins, "sw") == 0)
    {
        IDEX1[0] = obtenerDato(ID->rs);
        IDEX1[1] = atoi(ID->offset);
    }
    else if (strcmp(ID->ins, "j") == 0)
    {
        IDEX1[0] = 0;
        IDEX1[1] = 0;
    }

    return;
}

void etapaEX1()
{
    if (strcmp(EX1->ins, "add") == 0 || strcmp(EX1->ins, "addi") == 0)
    {
        EX1EX2 = IDEX1[0] + IDEX1[1];
    }
    else if (strcmp(EX1->ins, "sub") == 0 || strcmp(EX1->ins, "subi") == 0 || strcmp(EX1->ins, "beq") == 0 || strcmp(EX1->ins, "bne") == 0)
    {
        EX1EX2 = IDEX1[0] - IDEX1[1];
    }
    else if (strcmp(EX1->ins, "lw") == 0 || strcmp(EX1->ins, "sw") == 0)
    {
        EX1EX2 = (IDEX1[0] + IDEX1[1]) / 4;
    }
    else if (strcmp(EX1->ins, "lw") == 0)
    {
        EX1EX2 = 0;
    }

    return;
}

void etapaEX2(lista *memoriaIns)
{
    if (((strcmp(EX2->ins, "beq") == 0) && (EX1EX2 == 0)) || /* beq y la resta dio cero */
        ((strcmp(EX2->ins, "bne") == 0) && (EX1EX2 != 0)) || /* bne y la resta no dio cero */
        ((strcmp(EX2->ins, "j") == 0)))                      /* salto incondicional */
    {
        EX2MEM_DIR = 0;

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

    return;
}

void etapaMEM()
{
}

void etapaWB()
{
}

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

void rellenarMemoria()
{
    int i;
    for (i = 0; i < 1024; i++)
    {
        memoria[i] = 0;
    }
    return;
}

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
    fprintf(*pArchivo, "$ra = %d\n", ra);
    return;
}

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