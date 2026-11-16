/*** LIBRERÍAS ***/
#include <stdio.h> // Permite usar funciones de entrada y salida, como printf, getc, fopen, fclose y fwrite.
#include <stdlib.h> // Transforma cadenas de texto en números mediante funciones como atoi, atol, strtod y strtol.
#include <ctype.h> // Permite clasificar caracteres. 
// Por ejemplo, isalpha(c) pregunta si un carácter es una letra, isdigit(c) si es un dígito e isgraph(c) si es un carácter imprimible que no es un espacio en blanco.
#include <string.h> // Permite manipular cadenas de caracteres con funciones como strlen, strcpy, strcmp, strcat y strrchr.

/*** CONSTANTES ***/
#define MAX_CANTIDAD_IDENT 1024 // Hay espacio para un máximo de 1024 entradas: nombres de variables, constantes y procedimientos.
#define MAX_LONGITUD_CADENA 128  // Es a longitud neta, sin los apóstrofos ni el cero final.
// El número 128 proviene del sistema ASCII original, que utiliza 7 bits para representar cada carácter (2⁷ = 128).
#define MEM 8192 // MEM determina la capacidad del arreglo memoria: 8192 elementos de tipo byte.
// Como byte se define después como unsigned char, en esta implementación cada elemento ocupa un byte.
// Kibibyte (KiB): son 1.024 bytes (2¹⁰ bytes). Entonces: 8192 bytes = 8 KiB.

/*** NUEVOS TIPOS ***/
// Una declaración typedef es una declaración con typedef como clase de almacenamiento. 
// El declarador se convierte en un nuevo tipo. 
// Puede utilizar declaraciones typedef para construir nombres más cortos o más significativos para tipos ya definidos por C o para tipos que haya declarado.
// Los nombres de typedef permiten encapsular detalles de la implementación que pueden cambiar.
// Una declaración typedef se interpreta igual que una declaración de variable o de función, pero el identificador, en lugar de suponer el tipo especificado por la declaración, se convierte en un sinónimo del tipo.
// En ocasiones puede ser útil definir nombres para tipos de datos, 'alias' que nos hagan más fácil declarar variables y parámetros.

typedef unsigned char byte; // Será un tipo equivalente a 8 bits, sus valores van de 0 a 255.
// unsigned indica que el tipo representa valores sin signo.
// Cada posición de memoria ocupa tendrá una instrucción en hexa que ocupa un byte.

// Un enum es un conjunto de constantes enteras con nombre.
typedef enum {ASIGNACION, NULO, FIN_DE_ARCHIVO, BEGIN, END,
ABREPAREN, MAYOR, MENOR, IGUAL, DISTINTO, MAYORIGUAL, MENORIGUAL,
CIERRAPAREN, READLN, WRITE, WRITELN, VAR, CADENA, IDENT, NUMERO,
PUNTO, COMA, PTOCOMA, MAS, CONSTANTE, PROCEDURE, CALL, IF, THEN,
WHILE, DO, ODD, MENOS, BARRA, ASTERISCO} tTerminal; // En este caso se usa para representar todos los términos de los simbolos terminales.
// Es decir las palabras reservadas, numeros, identificadores, simbolos, indicación de fin de archivo y nulo.

typedef struct { // Define un nuevo tipo partiendo de una estructura.
    tTerminal tipo; // Si el identificador es una constante, una variable o un procedimiento.
    char nombre [MAX_LONGITUD_CADENA+3]; // El nombre del identificador más los apóstrofos y el cero final.
    int valor; // Constante: guarda el valor numérico de la constante.
               // Variable: guarda su posición de almacenamiento, utilizada para calcular dónde está su valor en memoria.
               // Procedimiento: guarda la posición en la que comienza su código generado.
} tEntradaTabla; // Representa un identificador.

typedef struct { // Define un nuevo tipo partiendo de una estructura.
    tTerminal simbolo; // simbolo guarda el tipo de terminal que es.
    char cadena[MAX_LONGITUD_CADENA + 3];  // cadena es el token leido más los apóstrofos y el cero final.
} tSimbolo; // Representa un simbolo terminal.

typedef tEntradaTabla tablaDeIdent[MAX_CANTIDAD_IDENT]; // Representa el espacio de memoria reservado para identificadores.
// MAX_CANTIDAD_IDENT vale 1024, por lo que tablaDeIdent es un arreglo de hasta 1024 entradas.

/*** NUEVAS FUNCIONES (PROTOTIPOS) ***/
// Son prototipos o etiquetas: le informan al compilador el nombre, el tipo de retorno y los parámetros de cada función antes de que aparezca su implementación.
tSimbolo aLex(FILE*); // Analizador léxico.

void imprimir(tSimbolo);
void concatenar(char*, char);
void uppercase(char*);
void error (int, char *);

// Análisis sintáctico y semántico.
tSimbolo programa (tSimbolo, FILE *, byte[], int *);
tSimbolo bloque (tSimbolo, FILE *, tablaDeIdent, int, byte[], int *, int *);
tSimbolo proposicion (tSimbolo, FILE *, tablaDeIdent, int, byte[], int *);
tSimbolo condicion (tSimbolo, FILE *, tablaDeIdent, int, byte[], int *);
tSimbolo expresion (tSimbolo, FILE *, tablaDeIdent, int, byte[], int *);
tSimbolo termino (tSimbolo, FILE *, tablaDeIdent, int, byte[], int *);
tSimbolo factor (tSimbolo, FILE *, tablaDeIdent, int, byte[], int *);

int buscarIdent (char *, tablaDeIdent, int, int);
void inicializarMemoria (byte[], int *);

void Crear (byte[], int, char *);
void CargarInt (byte[] , int *, int);
void CargarIntEnMem (byte[], int , int);
int leerIntDeMem (byte[], int);
void CargarByte(byte[],int *,byte);
void CargarByteEnMem (byte[], int, byte);
byte leerByte (byte[], int );
void cargarPopEAX(byte[], int*);

/*** MAIN ***/
// argc y argv son los parámetros que el sistema operativo le entrega al programa cuando se ejecuta desde la línea de comandos.
// argc: cantidad de argumentos recibidos.
// argv: arreglo de cadenas de caracteres, con un texto por argumento.
// El nombre del ejecutable también cuenta como argumento.
// Por eso, si ejecutás el compilador así: compiBase.exe ejemplo.pl0
// Los argumentos son: argc = 2, argv[0] = "compiBase.exe", argv[1] = "ejemplo.pl0".
// argv[0] suele contener el nombre o la ruta con la que se ejecutó el compilador.
// argv[1] contiene el nombre del archivo fuente que se quiere procesar.
int main(int argc, char* argv[]) {
    byte memoria[MEM]; // Crea la variable memoria de tipo array de byte, su índice válido va de 0 a 8191.
    // Sirve para ir poniendo todas las intrucciones de lenguaje de máquina en el array.
    int topememoria=0; // Inicializa topememoria en 0, me va a servir para saber hasta donde ocupé del array.
    // Representa la posición en la que se agregaría el próximo byte del código generado.
    // Al comenzar vale cero, por lo que el primer byte se escribiría en memoria[0].
    // No es el índice del último byte escrito, sino la posición siguiente.
    inicializarMemoria(memoria, &topememoria); // LLama a la función, esta tiene todo el encabezado que figura en la documentación.
    // La función recibe: 
    // memoria: el arreglo donde se va a construir el código.
    // &topememoria: la dirección de la variable topememoria (puntero) puede modificar el contador original.
    if (argc != 2) { // El programa espera exactamente dos argumentos: el nombre del propio compilador y un archivo fuente.
        printf("Uso: lexer FUENTE.PL0\n"); // Le muestra al usuario que debe proporcionar un archivo fuente.
    }
    else { // Sino entonces los argumentos están bien.
        FILE* archivo; // Declara un puntero que representa el archivo abierto.
        archivo = fopen(argv[1], "r"); // Intenta abrir el archivo en modo lectura.
        if (archivo == NULL) { // Si la apertura falla, fopen devuelve NULL.
            printf("Error de apertura del archivo [%s].\n", argv[1]);
        } else { // Si no, entonces apertura exitosa.
            tSimbolo s; // Declara s, que almacena un símbolo terminal.
            s = aLex(archivo); // LLama al analizador léxico, lee el primer símbolo del archivo fuente.
            s = programa (s, archivo, memoria, &topememoria); // Llama al análisis del programa completo. Se le pasa el símbolo actual, el archivo, el arreglo que recibirá el código generado y la dirección del contador (puede modificar topememoria).
            if (s.simbolo==FIN_DE_ARCHIVO) // Comprueba si el símbolo devuelto indica el fin del archivo.
                printf("GOOD JOB!! El programa estaba bien"); // Muestra el mensaje de éxito.
            else // Si no, llama a la función de error.
                error(0, s.cadena);
            fclose(archivo); // Cierra el archivo fuente que se había abierto.
        }
    Crear(memoria,topememoria,argv[1]); // Llama a la función Crear, escribe los bytes generados en el archivo binario.
    // Crear recibe:
    // memoria: el arreglo que contiene los bytes generados.
    // topememoria: la cantidad de bytes que se escribieron.
    // argv[1]: el nombre del archivo fuente, que la función usa para construir el nombre del archivo de salida.
    // La implementación también modifica el nombre recibido para cambiar su extensión a .exe.
    }
    return 0;
}

/*** NUEVAS FUNCIONES (DEFINICIONES) ***/
// Esta función es el analizador léxico, el objetivo es ir leyendo y cortando los caracteres del archivo e asignarles un tTerminal (tokenizar).
tSimbolo aLex(FILE* fp) { // Recibe el puntero al archivo prueba.pl0.
    tSimbolo a; // Declara un tSimbolo.
    a.cadena[0] = '\0'; // Inicializa la cadena como vacía (\0 es el carácter nulo y se utiliza para marcar el final de una cadena de texto).
    char c; // Declara un char.
    do { // Este bloque lee el archivo hasta encontrar algo.
        c = getc(fp); // Lee un caracter del archivo.
    } while (c != EOF && !isgraph(c));  /* corta cuando c==EOF o cuando isgraph(c)==true */ 
    //La función isgraph() en C verifica si un carácter tiene representación gráfica (es imprimible y no es un espacio en blanco).

    if (c == EOF){ // Si leyó un END OF FILE.
        a.simbolo = FIN_DE_ARCHIVO; // Se le asigna como simbolo el tTerminal FIN_DE_ARCHIVO.
    } else { // Sino significa que era un caracter imprimible...
        concatenar(a.cadena, c); // Concatena el caracter a la cadena, así va armando el token.
        if (isalpha(c)) { // Verifica si el carácter contiene únicamente letras.
            do { // Este bloque lee hasta encontrar algo que no sea letra o número (porque el token puede ser alphanumérico).
                c = getc(fp); 
                if (isalpha(c) || isdigit(c)) concatenar(a.cadena, c); // Si es un número o letra lo concatena y sigue leyendo.
            } while (c != EOF && (isalpha(c) || isdigit(c))); // Corta cuando c==EOF o cuando c no es letra ni dígito.
            ungetc(c, fp); // El char que provocó el fin de la cadena debe volver a leerse en el próximo llamado a aLex.
            // La función ungetc() en C sirve para devolver un carácter (un "empuje hacia atrás") a un flujo de entrada (FILE *stream), de modo que sea el próximo en leerse.
            // Se podría decir que el puntero retrocede una posición en la lectura del archivo.
            
            // Declara una cadena auxiliar.
            char cadenaAux[MAX_LONGITUD_CADENA + 3];  // Más los apóstrofos y el cero final.
            // Se reserva espacio para la longitud máxima, los dos apóstrofos y el cero final.
            // Contenido de la cadena: 128, Apóstrofo de apertura: 1, Apóstrofo de cierre: 1, Terminador '\0': 1, Total: 131.
            // Por eso, MAX_LONGITUD_CADENA + 3 permite reservar 131 posiciones para cadenaAux.
            strcpy(cadenaAux, a.cadena); // Copia la cadena que acaba de tokenizar a la auxiliar.
            uppercase(cadenaAux); // La pasa a mayúsculas.
            // En todos los casos se compara la cadena auxiliar con las palabras reservadas del código y si coinciden se le asigna el tTerminal correspondiente.
            if (strcmp(cadenaAux, "BEGIN") == 0) a.simbolo = BEGIN; // El strcmp da 0 si las cadenas son iguales.
            else if (strcmp(cadenaAux, "END") == 0) a.simbolo = END;
            else if (strcmp(cadenaAux, "READLN") == 0) a.simbolo = READLN;
            else if (strcmp(cadenaAux, "VAR") == 0) a.simbolo = VAR;
            else if (strcmp(cadenaAux, "CONST") == 0) a.simbolo = CONSTANTE;
            else if (strcmp(cadenaAux, "WRITE") == 0) a.simbolo = WRITE;
            else if (strcmp(cadenaAux, "WRITELN") == 0) a.simbolo = WRITELN;
            else if (strcmp(cadenaAux, "CALL") == 0) a.simbolo = CALL;
            else if (strcmp(cadenaAux, "NUMERO") == 0) a.simbolo = NUMERO; // Esta linea me parece que está totalmente de más
            else if (strcmp(cadenaAux, "PROCEDURE") == 0) a.simbolo = PROCEDURE;
            else if (strcmp(cadenaAux, "IF") == 0) a.simbolo = IF;
            else if (strcmp(cadenaAux, "THEN") == 0) a.simbolo = THEN;
            else if (strcmp(cadenaAux, "WHILE") == 0) a.simbolo = WHILE;
            else if (strcmp(cadenaAux, "DO") == 0) a.simbolo = DO;
            else if (strcmp(cadenaAux, "ODD") == 0) a.simbolo = ODD;
            else a.simbolo = IDENT; // En caso que no sea ninguna palabra reservada entonces es un identificador de variable o procedimiento.
        } else if (isdigit(c)) { // Si no es una letra, entonces comprueba si el carácter es un dígito numérico (del 0 al 9).
            do { // Este bloque lee hasta encontrar algo que no sea número.
                c = getc(fp);
                    if (isdigit(c)) concatenar(a.cadena, c); // Si es un número, lo concatena y sigue leyendo.
            } while (c != EOF && isdigit(c)); // Corta cuando c==EOF o cuando c no es dígito.
            ungetc(c, fp); // El char que provocó el fin de la cadena debe volver a leerse en el próximo llamado a aLex.
            a.simbolo = NUMERO; // Termina de armar el token numérico y le asigna al simbolo el tTerminal NUMERO.
        } else switch (c) { // Si no era letra ni número, entonces entra al switch de los posibles símbolos.
            // En C, la barra invertida \ sirve para representar caracteres especiales dentro de una constante de carácter o una cadena.
            // La barra invertida no es el carácter que se está buscando: es un mecanismo de escape para que C interprete correctamente el apóstrofo, sin confundirlo con el que delimita la constante de carácter.
            // Esto sirve por ejemplo, si el programa tiene: WRITE('Hola mundo') para poder leer la cadena.
            case '\'': // Este caso es si el caracter es apóstrofo.
                do { // Este bloque lee desde el comienzo de un apóstrofo hasta el cierre de otro.
                    c = getc(fp);
                    if (c != EOF && c != '\n') concatenar(a.cadena, c); // Si no es fin de archivo o salto de línea lo concatena.
                } while (c != EOF && c != '\n' && c != '\''); // Corta cuando c==EOF o c=='\n' o cuando c es un apóstrofo.
                // Porque si es un apóstrofo estaríamos ante el cierre.
                if (c == EOF || c == '\n') { // Si cortó el while porque se encontró con un fin de archivo o un \n.
                    a.simbolo = NULO; // Le asigna el tTerminal NULO.
                    ungetc(c, fp); 
                } else a.simbolo = CADENA; // Si el while se cortó porque llegó al cierre del apóstrofo, entonces le asigna el tTerminal CADENA a todo lo leido.
                break;
            // El resto de los casos según el símbolo del caracter se le asigna el tTerminal correspondiente.    
            case '.': a.simbolo = PUNTO;
                break;
            case ',': a.simbolo = COMA;
                break;
            case ';': a.simbolo = PTOCOMA;
                break;
            case '+': a.simbolo = MAS;
                break;
            case '(': a.simbolo = ABREPAREN;
                break;
            case ')': a.simbolo = CIERRAPAREN;
                break;
            case '<': // En el caso del menor quiero seguir leyendo a ver que le sigue.
                c = getc(fp); 
                switch(c){ 
                    case '=': a.simbolo = MENORIGUAL; // Si sigue un = entonces el token es <=.
                        concatenar(a.cadena, c); // Lo concatena.
                        break;
                    case '>': a.simbolo = DISTINTO; // Si sigue un > entonces el token es <>.
                        concatenar(a.cadena, c); // Lo concatena.
                        break;
                    default: a.simbolo = MENOR; // Si no, era cualquier otro símbolo, me quedo con el < solo.
                        ungetc(c, fp); // Lo deslee.
                        break;
                }
                break;
            // Antes usa switch porque eran más de dos opciones, ahora usa if porque son sólo dos pero es lo mismo.
            case '>': // Acá se repite la lógica que en el caso del menor, pero el distinto ya no hay que tenerlo en cuenta.
                c = getc(fp);
                if (c=='=')
                {
                    a.simbolo = MAYORIGUAL;
                    concatenar(a.cadena, c);
                }
                else
                {
                    a.simbolo = MAYOR;
                    ungetc(c, fp);
                }
                break;
            case ':': // En el caso de : sí o sí debe estar seguido de =.
                c = getc(fp);
                if (c=='=')
                {
                    a.simbolo = ASIGNACION;
                    concatenar(a.cadena, c);
                }
                else // Si no, es un token inválido y se le asigna el tTerminal NULO.
                {
                    a.simbolo = NULO;
                    ungetc(c, fp);
                }
                break;
            case '-': a.simbolo = MENOS;
                break;
            case '=': a.simbolo = IGUAL;
                break;
            case '/': a.simbolo = BARRA;
                break;
            case '*': a.simbolo = ASTERISCO;
                break;
            default: // Cualquier otro símbolo que no sea los analizados será error 18.
                error(18,a.cadena);
        }
    }
    imprimir(a); // Llama a la función imprimir mandando el simbolo completo (cadena y simbolo terminal).
    return a; // Retorna el simbolo.
}

void imprimir(tSimbolo simb) { // La función imprimir muestra lo que se acaba de tokenizar y qué tTerminal se le asignó.
    char j;
    printf("Cadena leida: \"%s\"\t Simbolo correspondiente: ", simb.cadena);
    switch(simb.simbolo){
        case NULO: printf("NULO");
            break;
        case FIN_DE_ARCHIVO: printf("FIN_DE_ARCHIVO");
            break;
        case NUMERO: printf("NUMERO");
            break;
        case MAYOR: printf("MAYOR");
            break;
        case CALL: printf("CALL");
            break;
        case MENOR: printf("MENOR");
            break;
        case IGUAL: printf("IGUAL");
            break;
        case BEGIN: printf("BEGIN");
            break;
        case IF: printf("IF");
            break;
        case THEN: printf("THEN");
            break;
        case END: printf("END");
            break;
        case READLN: printf("READLN");
            break;
        case WRITE: printf("WRITE");
            break;
        case WRITELN: printf("WRITELN");
            break;
        case VAR: printf("VAR");
            break;
        case CADENA: printf("CADENA");
            break;
        case IDENT: printf("IDENT");
            break;
        case PUNTO: printf("PUNTO");
            break;
        case COMA: printf("COMA");
            break;
        case ABREPAREN: printf("ABREPAREN");
            break;
        case DO: printf("DO");
            break;
        case CIERRAPAREN: printf("CIERRAPAREN");
            break;
        case PTOCOMA: printf("PTOCOMA");
            break;
        case MAS: printf("MAS");
            break;
        case MENOS: printf("MENOS");
            break;
        case DISTINTO: printf("DISTINTO");
            break;
        case MAYORIGUAL: printf("MAYORIGUAL");
            break;
        case MENORIGUAL: printf("MENORIGUAL");
            break;
        case ASIGNACION: printf("ASIGNACION");
            break;
        case CONSTANTE: printf("CONSTANTE");
            break;
        case WHILE: printf("WHILE");
            break;
        case PROCEDURE: printf("PROCEDURE");
            break;
        default:
            break;
    }
    printf ("\n");
}
 
void concatenar(char* s, char c) { // char* s es un puntero a char. En esta función se espera que apunte al comienzo de una cadena de caracteres terminada en '\0'.
    // Este if sirve como chequeo para evitar desbordamiento.
    if (strlen(s) < MAX_LONGITUD_CADENA + 2){  // La función permite que la cadena llegue a tener hasta 130 caracteres visibles o almacenados antes del terminador.
        // El for hace avanzar el puntero hasta el fin de línea.
        for (; *s; s++); // En este caso:
                         // Inicialización: está vacía. El puntero s ya apunta al comienzo de la cadena.
                         // Condición: *s. Se interpreta como verdadero mientras el carácter apuntado no sea '\0'.
                         // Actualización: s++. Avanza el puntero una posición.
                         // Cuerpo: está vacío, porque después del for hay un punto y coma.                  
        *s++ = c; // En el fin de línea escribe el nuevo caracter y luego avanza uno el puntero.
        *s++ = '\0'; // Escribe el fin de línea y luego avanza uno el puntero.
    }
    // También podría escribirse así:
    /* void concatenar(char* s, char c) {
        if (strlen(s) < MAX_LONGITUD_CADENA + 2) {
            while (*s != '\0') {
                s++;
            }

            *s = c;
            s++;
            *s = '\0';
        }
    }*/
}

void uppercase(char* s) {
    for (; *s; s++) // Recorre toda la cadena hasta el fin de línea.
        *s = toupper(*s); // Convierte un carácter en minúscula a su letra mayúscula correspondiente.
}

void error(int n, char *s) {
    printf("\nERROR: "); // Muestra el mensaje de error que corresponde asociado a un número de error.
    switch(n)
    {
    case 0:
        printf("Error 0: Se esperaba FIN DE ARCHIVO y se recibio '%s'", s);
        break;
    case 1:
        printf("Error 1: Se esperaba PUNTO y se recibio '%s'", s);
        break;
    case 2:
        printf("Error 2: Se esperaba IDENT y se recibio '%s'",s);
        break;
    case 3:
        printf("Error 3: Se esperaba IGUAL y se recibio '%s'",s);
        break;
    case 4:
        printf("Error 4: Se esperaba NUMERO y se recibio '%s'",s);
        break;
    case 5:
        printf("Error 5: Se esperaba PTOCOMA y se recibio '%s'",s);
        break;
    case 6:
        printf("Error 6: Se esperaba ASIGNACION y se recibio '%s'",s);
        break;
    case 7:
        printf("Error 7: Se esperaba END y se recibio '%s'",s);
        break;
    case 8:
        printf("Error 8: Se esperaba THEN y se recibio '%s'",s);
        break;
    case 9:
        printf("Error 9: Se esperaba DO y se recibio '%s'",s);
        break;
    case 10:
        printf("Error 10: Se esperaba ABREPARENTESIS y se recibio '%s'",s);
        break;
    case 11:
        printf("Error 11: Se esperaba CIERRAPARENTESIS y se recibio '%s'",s);
        break;
    case 12:
        printf("Error 12: Se esperaba OPERADOR RELACIONAL  y se recibio '%s'",s);
        break;
    case 13:
        printf("Error 13: Se esperaba IDENT, NUMERO O ABREPARENTESIS y se recibio '%s'",s);
        break;
    case 14:
        printf("Error 14: Identificador duplicado '%s'",s);
        break;
    case 15:
        printf("Error 15: Variable no declarada '%s'",s);
        break;
    case 16:
        printf("Error 16: Identificador no declarado como variable '%s'",s);
        break;
    case 17:
        printf("Error 17: Identificador no declarado como constante ni variable '%s'",s);
        break;
    default:
        printf(" %d \n",n);
    }
    exit(1); // Sirve para terminar el programa de forma inmediata y devolver un código de estado de error al sistema operativo.
}
 
tSimbolo programa (tSimbolo s, FILE *arch, byte memoria[], int *topememoria) {
    // La función devuelve un tSimbolo, es decir, devuelve información sobre un símbolo del lenguaje fuente.
    // Recibe cuatro parámetros:
    // s: el símbolo que ya fue leído del archivo. Es el punto desde el que continúa el análisis.
    // arch: un puntero al archivo fuente abierto. Se usa para seguir leyendo símbolos.
    // memoria[]: el arreglo de bytes donde se construye el ejecutable. 
    // Estamos recibiendo el arreglo completo, osea que la función trabaja sobre el original, no sobre una copia.
    // topememoria: un puntero al contador de bytes generados.
    
    tablaDeIdent tabla; // Crea una tabla para los identificadores.
    int distanciaFin; // Declara un entero para calcular el desplazamiento en un salto relativo.
    int d, i; // d se usa en el cálculo del valor que se escribe en la posición 1793 de memoria.
    // i se usa como contador en el for.
    int fileAligment, sectionAligment, sizeOfCodeSection, sizeOfRawData; // Declara variables para alineación del archivo, calcular valores alineados que se escriben en el encabezado y valores de tamaño.
    int posicionVar=0; // Inicializa posicionVar en 0, esto va a ir contando la cantidad variables declaradas.
    // topememoria luego de haber llamado a inicializarMemoria en el main termina con 1792 asignado.
    // (*topememoria)++ usa el valor actual como índice del array y luego incrementa.
    memoria[(*topememoria)++]= 0xBF; // Asigna 0xBF al array. Es decir hace un MOV EDI.
    CargarInt(memoria, topememoria, 0); // Carga el entero 0. Es decir hace un MOV EDI, 0.

    s = bloque(s, arch, tabla, 0, memoria, &*topememoria, &posicionVar); // Llama a la función bloque.
    // Cuando bloque termina, topememoria ya refleja cuánto se agregó, y entonces se puede calcular la ubicación de los datos.
    if (s.simbolo==PUNTO){ // Espera un punto después del bloque.
        s=aLex(arch); // Lee el siguiente token (debería ser FIN DE ARCHIVO).

        // Instrucción de salto relativo a 1416 - posición actual.
        distanciaFin=0x588-(*topememoria+5); // La fórmula está calculando una distancia relativa para que el salto llegue a la dirección representada por 0x588 (1416 en decimal). 
        // Como va a realizar un salto relativo, la distancia se interpreta respecto de la dirección de la instrucción siguiente al salto, no respecto del byte donde comienza el E9.
        // topememoria + 5 es porque quiero mover 5 bytes por la instruccion JMP dir.
        // Son 1 byte para el E9 y 4 bytes más para la dirección.
        CargarByte(memoria,topememoria,0xE9); // Realiza un salto relativo, JMP dir. 
        CargarInt(memoria,topememoria,distanciaFin); // Carga el entero distanciaFin. Es decir hace JMP distanciaFin.
        // Donde distanciaFin es 0x588-(*topememoria+5).
        // (*topememoria+5) son el byte que ocupa CargarByte + los 4 bytes que ocupa CargarInt.
        // Es decir hace un JMP 0x588, que es la posición 1416, menos la posición actual.

        // Completa el valor de MOV EDI.
        d=leerIntDeMem(memoria,212)+leerIntDeMem(memoria,204); // Guarda en d la suma de los valores de las posiciones 212 y 204.
        // leerIntDeMem(memoria, 212) lee un entero de cuatro bytes a partir de la posición 212 de la plantilla binaria.
        // leerIntDeMem(memoria, 204) hace lo mismo, pero a partir de la posición 204.
        // El programa suma ambos valores y guarda el resultado en d.
        // Calcula la dirección donde estarán los valores de las variables y la escribe como valor inmediato de MOV EDI.
        CargarIntEnMem(memoria,1793,(d+*topememoria-0x200)); // Carga en la posición 1793 el valor calculado.
        // d: el ajuste obtenido leyendo dos valores del encabezado.
        // topememoria: la posición alcanzada después de generar el código y, por lo tanto, el punto a partir del cuál el programa puede ubicar los datos de sus variables.
        // 0x200: equivale a 512 en decimal. Se resta como parte de la conversión entre la posición que ocupan los datos dentro del archivo y la referencia de dirección que necesita el ejecutable.
        
        // Reserva y deja inicializados en cero los espacios para las variables.
        for(i=0;i<posicionVar;i++){ // Recorre todas las variables.
            CargarInt(memoria,topememoria,0); // Agrega cuatro bytes con valor cero al final de la memoria generada.
            // Esto se hace por cada variable.
        }

        CargarIntEnMem(memoria,416,*topememoria-0x200); // Escribe un entero de cuatro bytes en la posición 416, sin cambiar el contador de memoria.
        // Como 0x200 equivale a 512, se está guardando el contador actual menos 512.

        fileAligment=leerIntDeMem(memoria,220); // Lee cuatro bytes desde la posición 220 y los interpreta como un entero.
        // El valor se guarda en fileAligment, que luego se usa para calcular el relleno que debe agregarse al final del archivo.
        while(*topememoria%fileAligment!=0){ // Este bucle agrega bytes cero hasta que topememoria sea divisible por fileAligment.
            CargarByte(memoria,topememoria,0x00);
        } // Este relleno se conoce como padding: bytes que no necesariamente representan instrucciones nuevas, pero que se agregan para respetar la alineación requerida por el formato del archivo.

        CargarIntEnMem(memoria,188,*topememoria-0x200); // En 188, se escribe topememoria - 512.
        CargarIntEnMem(memoria,424,*topememoria-0x200); // En 424, se escribe topememoria - 512.
        // Después lee esos campos.
        sizeOfCodeSection=leerIntDeMem(memoria,188);
        sizeOfRawData=leerIntDeMem(memoria,424);
        // Es decir, sizeOfCodeSection = sizeOfRawData.
        sectionAligment=leerIntDeMem(memoria,216); // Lee cuatro bytes desde la posición 216 y los guarda en sectionAligment.
        // A diferencia de fileAligment, que se utilizó para completar el tamaño del archivo.
        // Este valor se usa en los dos cálculos siguientes para ajustar tamaños relacionados con la alineación de las secciones en memoria.
        CargarIntEnMem(memoria,240,(2+sizeOfCodeSection/sectionAligment)*sectionAligment); // Escribe en la posición 240 el cálculo.
        // Utiliza el tamaño que leyó del campo 188 y el valor de alineación leído de la posición 216.
        // La división entre enteros descarta la parte decimal.
        // Por eso la fórmula agrega dos unidades de alineación al cociente calculado.
        CargarIntEnMem(memoria,208,(2+sizeOfRawData/sectionAligment)*sectionAligment); // Hace lo mismo.
        // La diferencia es que esta vez utiliza sizeOfRawData y escribe en la posición 208.
        // Estas dos operaciones ajustan campos de la plantilla ejecutable.
    }
    else // Si no es punto, error.
        error (1,s.cadena);
    return(s); // Retorna s, si encontró el punto probablemente s sea FIN DE ARCHIVO.
}

tSimbolo bloque(tSimbolo s, FILE *arch, tablaDeIdent tabla, int base, byte memoria[], int *topememoria, int *posicionVar) {
    // Esta función procesa las declaraciones de constantes, variables y procedimientos.
    // posicionVar cuenta cuántas variables se han asignado en el almacenamiento del programa (funciona como un índice).
    // topememoria indica la próxima posición libre de memoria, el arreglo donde el compilador está construyendo los bytes del ejecutable.
    // base indica la posición inicial de la zona de la tabla que utiliza el bloque.
    int desplazamiento=0, p; // desplazamiento indica cuántas entradas nuevas se registraron dentro de este bloque.
    // La siguiente entrada disponible se ubica en: base + desplazamiento.
    // Y el rango ocupado hasta ese momento termina en: base + desplazamiento - 1.
    
    int origen; // Variable para volver a cargar el salto.
    // Salto E9 00 00 00.
    CargarByte(memoria,topememoria,0xE9); // Instrucción JMP.
    CargarInt(memoria,topememoria,0); // Instrucción JMP 0.
    origen=*topememoria; // Guarda en origen la posición donde fue escrito el 0.
    
    if (s.simbolo==CONSTANTE) // Si el símbolo es una constante.
    {
        s=aLex(arch); // Lee el siguiente token.
        if (s.simbolo==IDENT) // Si es un identificador.
            {
                p=buscarIdent(s.cadena, tabla, base, base+desplazamiento-1); // Pasa la cadena, tabla, la pos inicial y la pos final.
                // Como la pos inicial es base y la final base+desplazamiento-1, si desplazamiento vale 0, entonces el rango queda vacío.
                if (p==-1) // Si devuelve -1 no encontró declaración previa.
                {
                    tabla[base+desplazamiento].tipo=CONSTANTE; // En la siguiente entrada disponible de la tabla guarda el tipo de ident.
                    strcpy(tabla[base+desplazamiento].nombre,s.cadena); // Guarda el nombre del ident.
                    uppercase(tabla[base+desplazamiento].nombre); // Pasa a mayúsculas.
                    // Nos falta el valor...
                }
                else
                    error(14, s.cadena); // Si existe el identificador, error.
                s=aLex(arch); // Sigue leyendo.
            }
        else // Si no es un ident, error.
            error(2, s.cadena); 
        if (s.simbolo==IGUAL) // Luego del ident, espera un igual.
            s=aLex(arch); // Sigue leyendo.
        else // Si no es igual, error.
            error(3, s.cadena); 
        if (s.simbolo==NUMERO) // Luego del igual, espera un número.
        {
            // ...en la siguiente entrada disponible de la tabla guarda el valor del número que leemos.
            tabla[base+desplazamiento].valor=atoi(s.cadena); // La función atoi transforma la cadena en integer.
            desplazamiento++; // Ahora que ya están completos todos los campos incrementa en uno el desplazamiento.
            s=aLex(arch); // Segue leyendo.
        }
        else // Si no es número, error.
            error(4, s.cadena); 
        while (s.simbolo==COMA) // Luego del número, podría haber una coma.
        {
            // Se realiza exactamente el mismo procedimiento que antes.
            s=aLex(arch);
            if (s.simbolo==IDENT)
            {
                p=buscarIdent(s.cadena, tabla, base, base+desplazamiento-1);
                if (p==-1)
                {
                    tabla[base+desplazamiento].tipo=CONSTANTE;
                    strcpy(tabla[base+desplazamiento].nombre,s.cadena);
                    uppercase(tabla[base+desplazamiento].nombre);
                }
                else
                    error(14, s.cadena);
                s=aLex(arch);
            }
            else
                error(2, s.cadena);
            if (s.simbolo==IGUAL)
                s=aLex(arch);
            else
                error(3, s.cadena);
            if (s.simbolo==NUMERO)
            {
                tabla[base+desplazamiento].valor=atoi(s.cadena);
                desplazamiento++;
                s=aLex(arch);
            }
            else
                error(4, s.cadena);
        } // Si ya no hay mas comas que enumeran los ident de las constantes.
        if (s.simbolo==PTOCOMA) // Espera punto y coma.
            s=aLex(arch); // Sigue leyendo.
        else // Si no hay punto y coma, error.
            error(5, s.cadena); 
    }
    if (s.simbolo==VAR) // Si es una variable.
    {
        s=aLex(arch); // Sigue leyendo.
        if (s.simbolo==IDENT) // Espera un ident.
        {
            p=buscarIdent(s.cadena, tabla, base, base+desplazamiento-1); // Busca igual que antes.
                if (p==-1)
                {
                    tabla[base+desplazamiento].tipo=VAR; // En la siguiente entrada disponible de la tabla guarda el tipo de ident.
                    tabla[base+desplazamiento].valor=(*posicionVar)++; // Guarda el valor actual de posicionVar y luego lo incrementa en uno.
                    strcpy(tabla[base+desplazamiento].nombre,s.cadena); // Guarda el nombre del ident.
                    uppercase(tabla[base+desplazamiento].nombre); // Lo pasa a mayúsculas.
                    desplazamiento++; // Incrementa el desplazamiento.
                }
                else // Si ya existe esa variable, error.
                    error(14, s.cadena); 
            s=aLex(arch); // Sigue leyendo.
        }
        else // Si no hay ident, error.
            error(2, s.cadena); 
        while (s.simbolo==COMA) // Mientras haya coma repite todo el procedimiento.
        {
            s=aLex(arch);
            if (s.simbolo==IDENT)
            {
                p=buscarIdent(s.cadena, tabla, base, base+desplazamiento-1);
                if (p==-1)
                {
                    tabla[base+desplazamiento].tipo=VAR;
                    tabla[base+desplazamiento].valor=(*posicionVar)++;;
                    strcpy(tabla[base+desplazamiento].nombre,s.cadena);
                    uppercase(tabla[base+desplazamiento].nombre);
                    desplazamiento++;
                }
                else
                    error(14, s.cadena);
                s=aLex(arch);
            }
            else
                error(2, s.cadena); // Si no hay ident error.
        }
        if (s.simbolo==PTOCOMA) // Luego del ident de una o varias variables espera punto y coma.
            s=aLex(arch); // Sigue leyendo.
        else // Si no hay punto y coma, error.
            error (5, s.cadena);
    }
    while (s.simbolo==PROCEDURE) // Como puede haber varios procedimientos tenemos un bucle.
    {
        s=aLex(arch); // Sigue leyendo.
        if (s.simbolo==IDENT) // Espera un ident.
        {
            p=buscarIdent(s.cadena, tabla, base, base+desplazamiento-1); // Busca que el ident no exista.
                if (p==-1)
                {
                    tabla[base+desplazamiento].tipo=PROCEDURE; // Asigna que es un procedimiento.
                    tabla[base+desplazamiento].valor=(*topememoria); // Guarda la posición de inicio de su código generado.
                    strcpy(tabla[base+desplazamiento].nombre,s.cadena); // Asigna el nombre.
                    uppercase(tabla[base+desplazamiento].nombre); // Lo pasa a mayúsculas.
                    desplazamiento++; // Incrementa el desplazamiento.
                }
                else // Si ya existe el ident, error.
                    error(14, s.cadena); 
            s=aLex(arch); // Sigue leyendo.
        }
        else // Si no es un ident, error.
            error(2, s.cadena); 
        if (s.simbolo==PTOCOMA) // Depués del ident, espera punto y coma.
            s=aLex(arch); // Sigue leyendo.
        else // Si no es punto y coma, error.
            error(5, s.cadena);
        // Dentro del procedure hay un bloque, llama a bloque recursivamente.
        s = bloque(s, arch, tabla, base+desplazamiento, memoria, &*topememoria, posicionVar); // Pero ahora a la base le suma el desplazamiento ya realizado.
        // Recordemos que al inicializar las variables de la función bloque desplazamiento volverá a iniciar en 0.
        CargarByte(memoria,topememoria,0xC3); ///RET (return) le indica al procesador que debe regresar de una llamada a una función o procedimiento.
        if (s.simbolo==PTOCOMA) // Después del bloque espera punto y coma para cerrar el procedimiento.
            s=aLex(arch); // Sigue leyendo.
        else // Si no hay punto y coma, error.
            error(5, s.cadena); 
    }
    CargarIntEnMem(memoria,origen-4,(*topememoria)-origen); // Carga en origen - 4, donde estaban los ceros del JMP, la posición actual - origen.
    s = proposicion(s, arch, tabla, base+desplazamiento-1, memoria, topememoria); // Llama a proposición.
    return(s); // Retorna el simbolo que devuelve proposición.
}

tSimbolo proposicion(tSimbolo s, FILE *arch, tablaDeIdent tabla, int posUltimoIdent, byte memoria[], int *topememoria) {
    int p, d;
    int distanciaWrite, distanciaRead;
    switch(s.simbolo) 
    {
    case IDENT: // Si el símbolo es ident.
        p=buscarIdent(s.cadena, tabla, 0, posUltimoIdent); // Llama a buscarIdent
        if (p==-1) // Si no lo encuentra, error.
            error(15, s.cadena);
        if (tabla[p].tipo!=VAR) // Si el tipo de ident no es var, error.
                error(16, s.cadena);
        s=aLex(arch); // Sigue leyendo.
        if (s.simbolo==ASIGNACION) // Espera asignación.
            s=aLex(arch); // Sigue leyendo.
        else // Si no es asignación, error.
            error(6, s.cadena);
        s = expresion(s,arch, tabla, posUltimoIdent, memoria, topememoria); // Llama a expresión.
        
        cargarPopEAX(memoria,topememoria); // Instrucción POP EAX, extrae el valor de la pila y lo coloca en el operando.
        // Equivale a CargarByte(memoria,topememoria,0x58);
        
        CargarByte(memoria,topememoria,0x89); // Instrucción MOV [EDI+abcdefgh], EAX.
        CargarByte(memoria,topememoria,0x87);
        CargarInt(memoria,topememoria,4*tabla[p].valor); // MOV [EDI+4*tabla[p].valor], EAX.
        // Como era una variable, el valor es la posición de la variable.
        break;
    case CALL: // Si el símbolo es call.
        s=aLex(arch); // Sigue leyendo.
        if (s.simbolo==IDENT) // Espera un ident.
            {
                p=buscarIdent(s.cadena, tabla, 0, posUltimoIdent); // Llama a buscar ident.
                if (p==-1) // Si no lo encuentra, error.
                    error(15, s.cadena);
                if (tabla[p].tipo!=PROCEDURE) // Si el tipo de ident no es procedure, error.
                        error(16, s.cadena);
                
                distanciaWrite= tabla[p].valor -(*topememoria+5); // Guarda en distanciaWrite la posición del procedure - (posición acutal + 5).
                // Suma 5 por el CargarByte y el CargarInt.
                CargarByte(memoria,topememoria,0xE8); // Instrucción CALL dir.
                CargarInt(memoria,topememoria,distanciaWrite); // Paso la dirección del procedimiento - actual.
                s=aLex(arch); // Sigue leyendo.
            }
        else // Si no es un ident, error.
            error(2, s.cadena);
        break;
    case BEGIN: // Si el símbolo es begin.
        s=aLex(arch); // Sigue leyendo.
        s = proposicion(s,arch, tabla, posUltimoIdent, memoria, topememoria); // Llama a proposición recursivamente.
        while (s.simbolo==PTOCOMA) // Mientras encuentre un punto y coma.
        {
            s=aLex(arch); // Sigue leyendo.
            s = proposicion(s, arch, tabla, posUltimoIdent, memoria, topememoria); // Llama a proposición recursivamente.
        }
        if (s.simbolo==END) // Espera un end.
            s=aLex(arch); // Sigue leyendo.
        else // Si no es end, error.
            error(7,s.cadena);
        break;
    case IF: // Si el símbolo es if.
        s=aLex(arch); // Sigue leyendo.
        s = condicion(s,arch, tabla, posUltimoIdent, memoria, topememoria); // Llama a condición.
        int origen=*topememoria; // Guarda en origen topememoria.
        if (s.simbolo==THEN) // Espera un then.
            s=aLex(arch); // Sigue leyendo.
        else // Si no es then, error.
            error(8,s.cadena);
        s = proposicion(s,arch, tabla, posUltimoIdent, memoria, topememoria); // Llama a proposición recursivamente.
        
        int distancia=*topememoria-origen; // Guarda en distancia hasta donde llegó la proposicón - donde inicio.
        CargarIntEnMem(memoria,origen-4,distancia); // Guarda en origen - 4, la distancia. 
        break;
    case WHILE: // Si el símbolo es while.
        s=aLex(arch); // Sigue leyendo.
        int inicioCiclo=*topememoria; // Guarda en inicioCiclo topememoria.
        s = condicion(s,arch, tabla, posUltimoIdent, memoria, topememoria); // Llama a condición.
        int origenCondicion=*topememoria; // Guarda en origenCondición topememoria.
        if (s.simbolo==DO) // Espera un do.
            s=aLex(arch); // Sigue leyendo.
        else // Si no es un do, error.
            error(9,s.cadena);
        s = proposicion(s,arch, tabla, posUltimoIdent, memoria, topememoria); // Llama a proposicón recursivamente.
        
        int dist=inicioCiclo-(*topememoria+5); // Guarda en dist el inicio del ciclo - posición acutal incluyendo la siguiente instrucción.
        memoria[(*topememoria)++]=0xE9; // Instrucción JMP dir. 
        //Equivale a llamar a CargarByte(memoria,topememoria,0xE9);
        CargarInt(memoria,topememoria,dist); // JMP dist. Salta al inicio del ciclo.
        dist=*topememoria-origenCondicion; // Guarda en dist, la posición actual - origen condición.
        CargarIntEnMem(memoria,origenCondicion-4,dist); // Carga en el origen de condición la nueva distancia.
        break;
    case READLN:
        s=aLex(arch);
        if (s.simbolo==ABREPAREN)
            s=aLex(arch);
        else
            error(10, s.cadena); //E10 falta
        if (s.simbolo==IDENT)
        {
            p=buscarIdent(s.cadena, tabla, 0, posUltimoIdent);
            if (p==-1)
                error(15, s.cadena);
            if (tabla[p].tipo!=VAR)
                error(16, s.cadena);
            s=aLex(arch);

            distanciaRead= 0x590-((*topememoria)+5);
            CargarByte(memoria,topememoria,0xE8);
            CargarInt(memoria,topememoria,distanciaRead);
            CargarByte(memoria,topememoria,0x89);
            CargarByte(memoria,topememoria,0x87);
            CargarInt(memoria,topememoria,4*tabla[p].valor);
        }
        else
            error(2, s.cadena);

        while (s.simbolo==COMA)
        {
            s=aLex(arch);
            if (s.simbolo==IDENT)
            {
                p=buscarIdent(s.cadena, tabla, 0, posUltimoIdent);
                if (p==-1)
                    error(15, s.cadena);
                if (tabla[p].tipo!=VAR)
                        error(16, s.cadena);
                s=aLex(arch);
            }
            else
                error(2,s.cadena);
        distanciaRead= 0x590-((*topememoria)+5);
        CargarByte(memoria,topememoria,0xE8);
        CargarInt(memoria,topememoria,distanciaRead);
        CargarByte(memoria,topememoria,0x89);
        CargarByte(memoria,topememoria,0x87);
        CargarInt(memoria,topememoria,4*tabla[p].valor);
        }
        if (s.simbolo==CIERRAPAREN)
            s=aLex(arch);
        else
            error(11, s.cadena);//E11 )
        break;
    case WRITE:
        s=aLex(arch);
        if (s.simbolo==ABREPAREN)
            s=aLex(arch);
        else
            error(10, s.cadena);
        if (s.simbolo==CADENA){
            d=leerIntDeMem(memoria,212)+leerIntDeMem(memoria,204);///Ajusto el valor de Mov EDI
            CargarByte(memoria,topememoria,0xB8);///revisar asterisco
            CargarInt(memoria, topememoria,d+*topememoria-0x200+14);

            distanciaWrite= 0x3E0-(*topememoria+5);
            CargarByte(memoria,topememoria,0xE8);
            CargarInt(memoria,topememoria,distanciaWrite);

            CargarByte(memoria,topememoria,0xE9);
            CargarInt(memoria, topememoria,strlen(s.cadena)-2+1);
            for (int i=1;i<strlen(s.cadena)-1;i++){
                CargarByte(memoria,topememoria,s.cadena[i]);
            }
            memoria[(*topememoria)++]='\0';
            s=aLex(arch);
        }
        else{
            s = expresion(s,arch, tabla, posUltimoIdent, memoria, topememoria);
            cargarPopEAX(memoria,topememoria);
            distanciaWrite= 0x420-(*topememoria+5);
            CargarByte(memoria,topememoria,0xE8);
            CargarInt(memoria,topememoria,distanciaWrite);

        }
        while (s.simbolo==COMA)
        {
            s=aLex(arch);
            if (s.simbolo==CADENA){
                d=leerIntDeMem(memoria,212)+leerIntDeMem(memoria,204);///Ajusto el valor de Mov EDI
                CargarByte(memoria,topememoria,0xB8);///revisar asterisco
                CargarInt(memoria, topememoria,d+*topememoria-0x200+14);

                distanciaWrite= 0x3E0-(*topememoria+5);
                CargarByte(memoria,topememoria,0xE8);
                CargarInt(memoria,topememoria,distanciaWrite);

                CargarByte(memoria,topememoria,0xE9);
                CargarInt(memoria, topememoria,strlen(s.cadena)-2+1);
                for (int i=1;i<strlen(s.cadena)-1;i++){
                    CargarByte(memoria,topememoria,s.cadena[i]);
                }
                memoria[(*topememoria)++]='\0';
                s=aLex(arch);
            }
            else{
                s = expresion(s,arch, tabla, posUltimoIdent, memoria, topememoria);
                cargarPopEAX(memoria,topememoria);
                distanciaWrite= 0x420-(*topememoria+5);
                CargarByte(memoria,topememoria,0xE8);
                CargarInt(memoria,topememoria,distanciaWrite);
            }
        }
        if (s.simbolo==CIERRAPAREN)
            s=aLex(arch);
        else
            error(11,s.cadena);
        break;
    case WRITELN:
            s=aLex(arch);
            if (s.simbolo==ABREPAREN)
            {
                s=aLex(arch);
                if (s.simbolo==CADENA){
                    d=leerIntDeMem(memoria,212)+leerIntDeMem(memoria,204);///Ajusto el valor de Mov EDI
                    CargarByte(memoria,topememoria,0xB8);///revisar asterisco
                    CargarInt(memoria, topememoria,d+*topememoria-0x200+14);

                    distanciaWrite= 0x3E0-(*topememoria+5);
                    CargarByte(memoria,topememoria,0xE8);
                    CargarInt(memoria,topememoria,distanciaWrite);

                    CargarByte(memoria,topememoria,0xE9);
                    CargarInt(memoria, topememoria,strlen(s.cadena)-2+1);
                    for (int i=1;i<strlen(s.cadena)-1;i++){
                        CargarByte(memoria,topememoria,s.cadena[i]);
                    }
                    memoria[(*topememoria)++]='\0';
                    s=aLex(arch);
                }
                else{
                    s = expresion(s,arch, tabla, posUltimoIdent, memoria, topememoria);
                    cargarPopEAX(memoria,topememoria);
                    distanciaWrite= 0x420-(*topememoria+5);
                    CargarByte(memoria,topememoria,0xE8);
                    CargarInt(memoria,topememoria,distanciaWrite);
                }
                while (s.simbolo==COMA)
                {
                    s=aLex(arch);
                    if (s.simbolo==CADENA){
                        d=leerIntDeMem(memoria,212)+leerIntDeMem(memoria,204);///Ajusto el valor de Mov EDI
                        CargarByte(memoria,topememoria,0xB8);///revisar asterisco
                        CargarInt(memoria, topememoria,d+*topememoria-0x200+14);

                        distanciaWrite= 0x3E0-(*topememoria+5);
                        CargarByte(memoria,topememoria,0xE8);
                        CargarInt(memoria,topememoria,distanciaWrite);

                        CargarByte(memoria,topememoria,0xE9);
                        CargarInt(memoria, topememoria,strlen(s.cadena)-2+1);
                        for (int i=1;i<strlen(s.cadena)-1;i++){
                            CargarByte(memoria,topememoria,s.cadena[i]);
                        }
                        memoria[(*topememoria)++]='\0';
                        s=aLex(arch);
                    }
                    else{
                        s = expresion(s,arch, tabla, posUltimoIdent, memoria, topememoria);
                        cargarPopEAX(memoria,topememoria);
                        distanciaWrite= 0x420-(*topememoria+5);
                        CargarByte(memoria,topememoria,0xE8);
                        CargarInt(memoria,topememoria,distanciaWrite);
                    }
                }
                if (s.simbolo==CIERRAPAREN)
                    s=aLex(arch);
                else
                    error(11,s.cadena);
            }
            int distanciaWriteln= 0x410-((*topememoria)+5);
            CargarByte(memoria,topememoria,0xE8);
            CargarInt(memoria,topememoria,distanciaWriteln);
            break;
    //default:
        //error(18,s.cadena);
        //break;
    }
    return(s);
}

tSimbolo condicion(tSimbolo s, FILE *arch, tablaDeIdent tabla, int posUltimoIdent, byte memoria[], int *topememoria) {
    byte aux;
    if (s.simbolo==ODD)
    {
        s=aLex(arch);
        s = expresion(s, arch, tabla, posUltimoIdent, memoria, topememoria);
        cargarPopEAX(memoria,topememoria);///POP EAX
        memoria[(*topememoria)++]=0xA8;///TEST AL
        memoria[(*topememoria)++]=0x01;///direccion del test
        memoria[(*topememoria)++]=0x7B;///JUMP IF PARITY ODD
        memoria[(*topememoria)++]=0x05;///direccion de salto del jump anterior
        memoria[(*topememoria)++]=0xE9;///JUMP (a la direccion de los siguientes bytes)
        memoria[(*topememoria)++]=0x00;///direcciones del salto del jump anterior
        memoria[(*topememoria)++]=0x00;
        memoria[(*topememoria)++]=0x00;
        memoria[(*topememoria)++]=0x00;
    }
    else
    {
        s = expresion(s,arch, tabla, posUltimoIdent, memoria, topememoria);

        switch(s.simbolo)
        {
        case IGUAL:
            aux=0x74;///salta si es = la comparacion
            s=aLex(arch);
            break;
        case DISTINTO:
            aux=0x75;
            s=aLex(arch);
            break;
        case MENOR:
            aux=0x7C;
            s=aLex(arch);
            break;
        case MENORIGUAL:
            aux=0x7E;
            s=aLex(arch);
            break;
        case MAYOR:
            aux=0x7F;
            s=aLex(arch);
            break;
        case MAYORIGUAL:
            aux=0x7D;
            s=aLex(arch);
            break;
        default:
            error(12, s.cadena);
            break;
        }
    s = expresion(s,arch, tabla, posUltimoIdent, memoria, topememoria);
    cargarPopEAX(memoria,topememoria);///POP EAX
    memoria[(*topememoria)++]=0x5B;///POP EBX
    memoria[(*topememoria)++]=0x39;///CMP EBX, EAX
    memoria[(*topememoria)++]=0xC3;
    memoria[(*topememoria)++]=aux;
    memoria[(*topememoria)++]=0x05;///posicion de memoria
    memoria[(*topememoria)++]=0xE9;///jump
    memoria[(*topememoria)++]=0x00;
    memoria[(*topememoria)++]=0x00;
    memoria[(*topememoria)++]=0x00;
    memoria[(*topememoria)++]=0x00;
    }
    return(s);
}

tSimbolo expresion(tSimbolo s, FILE *arch, tablaDeIdent tabla, int posUltimoIdent, byte memoria[], int *topememoria) {
    tTerminal operando=s.simbolo;
    if (s.simbolo==MAS)
        s=aLex(arch);
    else if (s.simbolo==MENOS)
        s=aLex(arch);
    s = termino(s,arch, tabla, posUltimoIdent, memoria, topememoria);
    if (operando==MENOS){
        cargarPopEAX(memoria,topememoria);///POP EAX
        memoria[(*topememoria)++]=0xF7;///NEG EAX
        memoria[(*topememoria)++]=0xD8;
        memoria[(*topememoria)++]=0x50;///PUSH EAX
    }
    while (s.simbolo==MAS || s.simbolo==MENOS)
    {
        operando=s.simbolo;
        s=aLex(arch);
        s = termino(s,arch, tabla, posUltimoIdent, memoria, topememoria);
        if (operando==MENOS){
            cargarPopEAX(memoria,topememoria);///POP EAX
            memoria[(*topememoria)++]=0x5B;///POP EBX
            memoria[(*topememoria)++]=0x93;///XCHG EAX, EBX
            memoria[(*topememoria)++]=0x29;///SUB EAX, EBX
            memoria[(*topememoria)++]=0xD8;
            memoria[(*topememoria)++]=0x50;///PUSH EAX
        }
        else{
            cargarPopEAX(memoria,topememoria);///POP EAX
            memoria[(*topememoria)++]=0x5B;///POP EBX
            memoria[(*topememoria)++]=0x01;///ADD EAX, EBX
            memoria[(*topememoria)++]=0xD8;
            memoria[(*topememoria)++]=0x50;///PUSH EAX
        }
    }
    return(s);
}

tSimbolo termino(tSimbolo s, FILE *arch, tablaDeIdent tabla, int posUltimoIdent, byte memoria[], int *topememoria) {
    tTerminal operando;
    s = factor(s,arch, tabla, posUltimoIdent, memoria, topememoria);
    while (s.simbolo==ASTERISCO||s.simbolo==BARRA)
    {
        operando=s.simbolo;
        s=aLex(arch);
        s = factor(s,arch, tabla, posUltimoIdent, memoria, &*topememoria);
        if (operando==ASTERISCO){
            cargarPopEAX(memoria,topememoria);///POP EAX
            memoria[(*topememoria)++]=0x5B;///POP EBX
            memoria[(*topememoria)++]=0xF7;///IMUL EBX
            memoria[(*topememoria)++]=0xEB;
            memoria[(*topememoria)++]=0x50;///PUSH EAX
        }
        else{
            cargarPopEAX(memoria,topememoria);///POP EAX
            memoria[(*topememoria)++]=0x5B;///POP EBX
            memoria[(*topememoria)++]=0x93;///XCHG EAX EBX
            memoria[(*topememoria)++]=0x99;///CDQ
            // memoria[(*topememoria)++]=0xF7;///IMUL EBX no deberia ser div
            memoria[(*topememoria)++]=0xFB;
            memoria[(*topememoria)++]=0x50;///PUSH EAX
        }
    }
    return(s);
}

tSimbolo factor(tSimbolo s, FILE *arch, tablaDeIdent tabla, int posUltimoIdent, byte memoria[], int *topememoria) {
    int p;
    switch(s.simbolo)
    {
    case IDENT:
        p=buscarIdent(s.cadena, tabla, 0, posUltimoIdent);
        if (p==-1)
            error(15, s.cadena);
        if (tabla[p].tipo!=VAR&&tabla[p].tipo!=CONSTANTE)
            error(16, s.cadena);
        if (tabla[p].tipo==VAR){
            memoria[(*topememoria)++]= 0x8B;    ///MOV EAX, [EDI+abcdefgh]
            memoria[(*topememoria)++]= 0x87;
            CargarInt(memoria,topememoria,4*tabla[p].valor);
        }
        else{
            memoria[(*topememoria)++]= 0xB8; //MOV EAX, abcdefgh
            CargarInt(memoria,topememoria,tabla[p].valor);
        }
        memoria[(*topememoria)++]= 0x50; //PUSH EAX
        s=aLex(arch);
        break;
    case NUMERO:
        memoria[(*topememoria)++]= 0xB8; //MOV EAX, abcdefgh
        CargarInt(memoria,topememoria,atoi(s.cadena));
        memoria[(*topememoria)++]= 0x50; //PUSH EAX
        s=aLex(arch);
        break;
    case ABREPAREN:
        s=aLex(arch);
        s = expresion(s,arch, tabla, posUltimoIdent, memoria, topememoria);
        if (s.simbolo==CIERRAPAREN)
            s=aLex(arch);
        else
            error(11,s.cadena);
        break;
    default:
        error(13, s.cadena);
        break;
    }
    return(s);
}

int buscarIdent (char *id, tablaDeIdent tabla, int posPrimerIdent, int posUltimoIdent){
    // Recibe un id (nombre del identificador), tabla (tabla de identificadores), posición del primero y posición del último.
    int j=0;
    char cadenaAux[MAX_LONGITUD_CADENA+3];
    strcpy(cadenaAux,id); // El id recibido se copia a cadenaAux.
    uppercase(cadenaAux); // Pasa cadenaAux a mayúsculas.
    int i=posUltimoIdent;
    printf("Se busca %s entre %d y %d\n",id,posUltimoIdent,posPrimerIdent);
    for (j=0;j<=posUltimoIdent;j++){ // Se recorre desde 0 hasta la posición del último identificador en la tabla.
        printf("%s %d %d\n",tabla[j].nombre,tabla[j].tipo,tabla[j].valor);
    }
    // Este while recorre para atras mientras la posición del último sea mas grande que del primero y no conicida el nombre retrocede.
    while ((i>=posPrimerIdent)&&(strcmp(cadenaAux,tabla[i].nombre)!=0)) 
        i--;
    // Recorrer al revés importa cuando existen identificadores con el mismo nombre en bloques anidados: 
    // La búsqueda puede encontrar primero la declaración más cercana.
    return (i>=posPrimerIdent?i:-1); // La función devuelve un entero: la posición del identificador en la tabla, o -1 si no lo encuentra dentro del rango indicado.
// Esta expresión utiliza el operador ternario. Es equivalente a escribir:
// if (i >= posPrimerIdent)
//     return i;
// else
//     return -1;
}

void inicializarMemoria (byte memoria[], int *topememoria){
    // Esta función inicializa la memoria con el encabezado que figura en el apunte.
    int i=448;
    memoria[0] = 0x4D; // 'M' (Magic number)
    memoria[1] = 0x5A; // 'Z'
    memoria[2] = 0x60; // Bytes on last block
    memoria[3] = 0x01; // (1 bl. = 512 bytes)
    memoria[4] = 0x01; // Number of blocks
    memoria[5] = 0x00; // in the EXE file
    memoria[6] = 0x00; // Number of rememoria[7] = 0x00; // location entries
    memoria[8] = 0x04; // Size of header
    memoria[9] = 0x00; // (x 16 bytes)
    memoria[10] = 0x00; // Minimum extra
    memoria[11] = 0x00; // paragraphs needed
    memoria[12] = 0xFF; // Maximum extra
    memoria[13] = 0xFF; // paragraphs needed
    memoria[14] = 0x00; // Initial (relative)
    memoria[15] = 0x00; // SS value
    memoria[16] = 0x60; // Initial SP value
    memoria[17] = 0x01;
    memoria[18] = 0x00; // Checksum
    memoria[19] = 0x00;
    memoria[20] = 0x00; // Initial IP value
    memoria[21] = 0x00;
    memoria[22] = 0x00; // Initial (relative)
    memoria[23] = 0x00; // CS value
    memoria[24] = 0x40; // Offset of the 1st
    memoria[25] = 0x00; // relocation item
    memoria[26] = 0x00; // Overlay number.
    memoria[27] = 0x00; // (0 = main program)
    memoria[28] = 0x00; // Reserved word
    memoria[29] = 0x00;
    memoria[30] = 0x00; // Reserved word
    memoria[31] = 0x00;
    memoria[32] = 0x00; // Reserved word
    memoria[33] = 0x00;
    memoria[34] = 0x00; // Reserved word
    memoria[35] = 0x00;
    memoria[36] = 0x00; // OEM identifier
    memoria[37] = 0x00;
    memoria[38] = 0x00; // OEM information
    memoria[39] = 0x00;
    memoria[40] = 0x00; // Reserved word
    memoria[41] = 0x00;
    memoria[42] = 0x00; // Reserved word
    memoria[43] = 0x00;
    memoria[44] = 0x00; // Reserved word
    memoria[45] = 0x00;
    memoria[46] = 0x00; // Reserved word
    memoria[47] = 0x00;
    memoria[48] = 0x00; // Reserved word
    memoria[49] = 0x00;
    memoria[50] = 0x00; // Reserved word
    memoria[51] = 0x00;
    memoria[52] = 0x00; // Reserved word
    memoria[53] = 0x00;
    memoria[54] = 0x00; // Reserved word
    memoria[55] = 0x00;
    memoria[56] = 0x00; // Reserved word
    memoria[57] = 0x00;
    memoria[58] = 0x00; // Reserved word
    memoria[59] = 0x00;
    memoria[60] = 0xA0; // Start of the COFF
    memoria[61] = 0x00; // header
    memoria[62] = 0x00;
    memoria[63] = 0x00;
    memoria[64] = 0x0E; // PUSH CS
    memoria[65] = 0x1F; // POP DS
    memoria[66] = 0xBA; // MOV DX,000E
    memoria[67] = 0x0E;
    memoria[68] = 0x00;
    memoria[69] = 0xB4; // MOV AH,09
    memoria[70] = 0x09;
    memoria[71] = 0xCD; // INT 21
    memoria[72] = 0x21;
    memoria[73] = 0xB8; // MOV AX,4C01
    memoria[74] = 0x01;
    memoria[75] = 0x4C;
    memoria[76] = 0xCD; // INT 21
    memoria[77] = 0x21;
    memoria[78] = 0x54; // 'T'
    memoria[79] = 0x68; // 'h'
    memoria[80] = 0x69; // 'i'
    memoria[81] = 0x73; // 's'
    memoria[82] = 0x20; // ' '
    memoria[83] = 0x70; // 'p'
    memoria[84] = 0x72; // 'r'
    memoria[85] = 0x6F; // 'o'
    memoria[86] = 0x67; // 'g'
    memoria[87] = 0x72; // 'r'
    memoria[88] = 0x61; // 'a'
    memoria[89] = 0x6D; // 'm'
    memoria[90] = 0x20; // ' '
    memoria[91] = 0x69; // 'i'
    memoria[92] = 0x73; // 's'
    memoria[93] = 0x20; // ' '
    memoria[94] = 0x61; // 'a'
    memoria[95] = 0x20; // ' '
    memoria[96] = 0x57; // 'W'
    memoria[97] = 0x69; // 'i'
    memoria[98] = 0x6E; // 'n'
    memoria[99] = 0x33; // '3'
    memoria[100] = 0x32; // '2'
    memoria[101] = 0x20; // ' '
    memoria[102] = 0x63; // 'c'
    memoria[103] = 0x6F; // 'o'
    memoria[104] = 0x6E; // 'n'
    memoria[105] = 0x73; // 's'
    memoria[106] = 0x6F; // 'o'
    memoria[107] = 0x6C; // 'l'
    memoria[108] = 0x65; // 'e'
    memoria[109] = 0x20; // ' '
    memoria[110] = 0x61; // 'a'
    memoria[111] = 0x70; // 'p'
    memoria[112] = 0x70; // 'p'
    memoria[113] = 0x6C; // 'l'
    memoria[114] = 0x69; // 'i'
    memoria[115] = 0x63; // 'c'
    memoria[116] = 0x61; // 'a'
    memoria[117] = 0x74; // 't'
    memoria[118] = 0x69; // 'i'
    memoria[119] = 0x6F; // 'o'
    memoria[120] = 0x6E; // 'n'
    memoria[121] = 0x2E; // '.'
    memoria[122] = 0x20; // ' '
    memoria[123] = 0x49; // 'I'
    memoria[124] = 0x74; // 't'
    memoria[125] = 0x20; // ' '
    memoria[126] = 0x63; // 'c'
    memoria[127] = 0x61; // 'a'
    memoria[128] = 0x6E; // 'n'
    memoria[129] = 0x6E; // 'n'
    memoria[130] = 0x6F; // 'o'
    memoria[131] = 0x74; // 't'
    memoria[132] = 0x20; // ' '
    memoria[133] = 0x62; // 'b'
    memoria[134] = 0x65; // 'e'
    memoria[135] = 0x20; // ' '
    memoria[136] = 0x72; // 'r'
    memoria[137] = 0x75; // 'u'
    memoria[138] = 0x6E; // 'n'
    memoria[139] = 0x20; // ' '
    memoria[140] = 0x75; // 'u'
    memoria[141] = 0x6E; // 'n'
    memoria[142] = 0x64; // 'd'
    memoria[143] = 0x65; // 'e'
    memoria[144] = 0x72; // 'r'
    memoria[145] = 0x20; // ' '
    memoria[146] = 0x4D; // 'M'
    memoria[147] = 0x53; // 'S'
    memoria[148] = 0x2D; // '-'
    memoria[149] = 0x44; // 'D'
    memoria[150] = 0x4F; // 'O'
    memoria[151] = 0x53; // 'S'
    memoria[152] = 0x2E; // '.'
    memoria[153] = 0x0D; // Carriage return
    memoria[154] = 0x0A; // Line feed
    memoria[155] = 0x24; // String end ('$')
    memoria[156] = 0x00;
    memoria[157] = 0x00;
    memoria[158] = 0x00;
    memoria[159] = 0x00;
    /* COFF HEADER - 8 Standard fields */
    memoria[160] = 0x50; // 'P'
    memoria[161] = 0x45; // 'E'
    memoria[162] = 0x00; // '\0'
    memoria[163] = 0x00; // '\0'
    memoria[164] = 0x4C; // Machine:
    memoria[165] = 0x01; // >= Intel 386
    memoria[166] = 0x01; // Number of
    memoria[167] = 0x00; // sections
    memoria[168] = 0x00; // Time/Date stamp
    memoria[169] = 0x00;
    memoria[170] = 0x53;
    memoria[171] = 0x4C;
    memoria[172] = 0x00; // Pointer to symbol
    memoria[173] = 0x00; // table
    memoria[174] = 0x00; // (deprecated)
    memoria[175] = 0x00;
    memoria[176] = 0x00; // Number of symbols
    memoria[177] = 0x00; // (deprecated)
    memoria[178] = 0x00;
    memoria[179] = 0x00;
    memoria[180] = 0xE0; // Size of optional
    memoria[181] = 0x00; // header
    memoria[182] = 0x02; // Characteristics:
    memoria[183] = 0x01; // 32BIT_MACHINE EXE
    /* OPTIONAL HEADER - 8 Standard fields */
    /* (For image files, it is required) */
    memoria[184] = 0x0B; // Magic number
    memoria[185] = 0x01; // (010B = PE32)
    memoria[186] = 0x01;// Maj.Linker Version
    memoria[187] = 0x00;// Min.Linker Version
    memoria[188] = 0x00; // Size of code
    memoria[189] = 0x06; // (text) section
    memoria[190] = 0x00;
    memoria[191] = 0x00;
    memoria[192] = 0x00; // Size of
    memoria[193] = 0x00; // initialized data
    memoria[194] = 0x00; // section
    memoria[195] = 0x00;
    memoria[196] = 0x00; // Size of
    memoria[197] = 0x00; // uninitialized
    memoria[198] = 0x00; // data section
    memoria[199] = 0x00;
    memoria[200] = 0x00; // Starting address
    memoria[201] = 0x15; // relative to the
    memoria[202] = 0x00; // image base
    memoria[203] = 0x00;
    memoria[204] = 0x00; // Base of code
    memoria[205] = 0x10;
    memoria[206] = 0x00;
    memoria[207] = 0x00;
    /* OPT.HEADER - 1 PE32 specific field */
    memoria[208] = 0x00; // Base of data
    memoria[209] = 0x20;
    memoria[210] = 0x00;
    memoria[211] = 0x00;
    /* OPT.HEADER - 21 Win-Specific Fields */
    memoria[212] = 0x00; // Image base
    memoria[213] = 0x00; // (Preferred
    memoria[214] = 0x40; // address of image
    memoria[215] = 0x00; // when loaded)
    memoria[216] = 0x00; // Section alignment
    memoria[217] = 0x10;
    memoria[218] = 0x00;
    memoria[219] = 0x00;
    memoria[220] = 0x00; // File alignment
    memoria[221] = 0x02; // (Default is 512)
    memoria[222] = 0x00;
    memoria[223] = 0x00;
    memoria[224] = 0x04; // Major OS version
    memoria[225] = 0x00;
    memoria[226] = 0x00; // Minor OS version
    memoria[227] = 0x00;
    memoria[228] = 0x00;// Maj. image version
    memoria[229] = 0x00;
    memoria[230] = 0x00;// Min. image version
    memoria[231] = 0x00;
    memoria[232] = 0x04;// Maj.subsystem ver.
    memoria[233] = 0x00;
    memoria[234] = 0x00;// Min.subsystem ver.
    memoria[235] = 0x00;
    memoria[236] = 0x00; // Win32 version
    memoria[237] = 0x00; // (Reserved, must
    memoria[238] = 0x00; // be zero)
    memoria[239] = 0x00;
    memoria[240] = 0x00;// Size of image
    memoria[241] = 0x20;// (It must be a
    memoria[242] = 0x00;// multiple of the
    memoria[243] = 0x00;// section alignment)
    memoria[244] = 0x00; // Size of headers
    memoria[245] = 0x02; // (rounded up to a
    memoria[246] = 0x00; // multiple of the
    memoria[247] = 0x00; // file alignment)
    memoria[248] = 0x00; // Checksum
    memoria[249] = 0x00;
    memoria[250] = 0x00;
    memoria[251] = 0x00;
    memoria[252] = 0x03; // Windows subsystem
    memoria[253] = 0x00; // (03 = console)
    memoria[254] = 0x00; // DLL characmemoria[255] = 0x00; // teristics
    memoria[256] = 0x00; // Size of stack
    memoria[257] = 0x00; // reserve
    memoria[258] = 0x10;
    memoria[259] = 0x00;
    memoria[260] = 0x00; // Size of stack
    memoria[261] = 0x10; // commit
    memoria[262] = 0x00;
    memoria[263] = 0x00;
    memoria[264] = 0x00; // Size of heap
    memoria[265] = 0x00; // reserve
    memoria[266] = 0x10;
    memoria[267] = 0x00;
    memoria[268] = 0x00; // Size of heap
    memoria[269] = 0x10; // commit
    memoria[270] = 0x00;
    memoria[271] = 0x00;
    memoria[272] = 0x00; // Loader flags
    memoria[273] = 0x00; // (Reserved, must
    memoria[274] = 0x00; // be zero)
    memoria[275] = 0x00;
    memoria[276] = 0x10; // Number of
    memoria[277] = 0x00; // relative virtual
    memoria[278] = 0x00; // addresses (RVAs)
    memoria[279] = 0x00;
    /* OPT. HEADER - 16 Data Directories */
    memoria[280] = 0x00; // Export Table
    memoria[281] = 0x00;
    memoria[282] = 0x00;
    memoria[283] = 0x00;
    memoria[284] = 0x00;
    memoria[285] = 0x00;
    memoria[286] = 0x00;
    memoria[287] = 0x00;
    memoria[288] = 0x1C; // Import Table
    memoria[289] = 0x10;
    memoria[290] = 0x00;
    memoria[291] = 0x00;
    memoria[292] = 0x28;
    memoria[293] = 0x00;
    memoria[294] = 0x00;
    memoria[295] = 0x00;
    memoria[296] = 0x00; // Resource Table
    memoria[297] = 0x00;
    memoria[298] = 0x00;
    memoria[299] = 0x00;
    memoria[300] = 0x00;
    memoria[301] = 0x00;
    memoria[302] = 0x00;
    memoria[303] = 0x00;
    memoria[304] = 0x00; // Exception Table
    memoria[305] = 0x00;
    memoria[306] = 0x00;
    memoria[307] = 0x00;
    memoria[308] = 0x00;
    memoria[309] = 0x00;
    memoria[310] = 0x00;
    memoria[311] = 0x00;
    memoria[312] = 0x00; // Certificate Table
    memoria[313] = 0x00;
    memoria[314] = 0x00;
    memoria[315] = 0x00;
    memoria[316] = 0x00;
    memoria[317] = 0x00;
    memoria[318] = 0x00;
    memoria[319] = 0x00;
    memoria[320] = 0x00; // Base Relocation
    memoria[321] = 0x00; // Table
    memoria[322] = 0x00;
    memoria[323] = 0x00;
    memoria[324] = 0x00;
    memoria[325] = 0x00;
    memoria[326] = 0x00;
    memoria[327] = 0x00;
    memoria[328] = 0x00; // Debug
    memoria[329] = 0x00;
    memoria[330] = 0x00;
    memoria[331] = 0x00;
    memoria[332] = 0x00;
    memoria[333] = 0x00;
    memoria[334] = 0x00;
    memoria[335] = 0x00;
    memoria[336] = 0x00; // Architecture
    memoria[337] = 0x00;
    memoria[338] = 0x00;
    memoria[339] = 0x00;
    memoria[340] = 0x00;
    memoria[341] = 0x00;
    memoria[342] = 0x00;
    memoria[343] = 0x00;
    memoria[344] = 0x00; // Global Ptr
    memoria[345] = 0x00;
    memoria[346] = 0x00;
    memoria[347] = 0x00;
    memoria[348] = 0x00;
    memoria[349] = 0x00;
    memoria[350] = 0x00;
    memoria[351] = 0x00;
    memoria[352] = 0x00; // TLS Table
    memoria[353] = 0x00;
    memoria[354] = 0x00;
    memoria[355] = 0x00;
    memoria[356] = 0x00;
    memoria[357] = 0x00;
    memoria[358] = 0x00;
    memoria[359] = 0x00;
    memoria[360] = 0x00; // Load Config Table
    memoria[361] = 0x00;
    memoria[362] = 0x00;
    memoria[363] = 0x00;
    memoria[364] = 0x00;
    memoria[365] = 0x00;
    memoria[366] = 0x00;
    memoria[367] = 0x00;
    memoria[368] = 0x00; // Bound Import
    memoria[369] = 0x00;
    memoria[370] = 0x00;
    memoria[371] = 0x00;
    memoria[372] = 0x00;
    memoria[373] = 0x00;
    memoria[374] = 0x00;
    memoria[375] = 0x00;
    memoria[376] = 0x00; // IAT
    memoria[377] = 0x10;
    memoria[378] = 0x00;
    memoria[379] = 0x00;
    memoria[380] = 0x1C;
    memoria[381] = 0x00;
    memoria[382] = 0x00;
    memoria[383] = 0x00;
    memoria[384] = 0x00; // Delay Import
    memoria[385] = 0x00; // Descriptor
    memoria[386] = 0x00;
    memoria[387] = 0x00;
    memoria[388] = 0x00;
    memoria[389] = 0x00;
    memoria[390] = 0x00;
    memoria[391] = 0x00;
    memoria[392] = 0x00; // CLR Runtime
    memoria[393] = 0x00; // Header
    memoria[394] = 0x00;
    memoria[395] = 0x00;
    memoria[396] = 0x00;
    memoria[397] = 0x00;
    memoria[398] = 0x00;
    memoria[399] = 0x00;
    memoria[400] = 0x00; // Reserved, must be
    memoria[401] = 0x00; // zero
    memoria[402] = 0x00;
    memoria[403] = 0x00;
    memoria[404] = 0x00;
    memoria[405] = 0x00;
    memoria[406] = 0x00;
    memoria[407] = 0x00;
    /* SECTIONS TABLE (40 bytes per entry) */
    /* FIRST ENTRY: TEXT HEADER */
    memoria[408] = 0x2E; // '.' (Name)
    memoria[409] = 0x74; // 't'
    memoria[410] = 0x65; // 'e'
    memoria[411] = 0x78; // 'x'
    memoria[412] = 0x74; // 't'
    memoria[413] = 0x00;
    memoria[414] = 0x00;
    memoria[415] = 0x00;
    memoria[416] = 0x24; // Virtual size
    memoria[417] = 0x05;
    memoria[418] = 0x00;
    memoria[419] = 0x00;
    memoria[420] = 0x00; // Virtual address
    memoria[421] = 0x10;
    memoria[422] = 0x00;
    memoria[423] = 0x00;
    memoria[424] = 0x00; // Size of raw data
    memoria[425] = 0x06;
    memoria[426] = 0x00;
    memoria[427] = 0x00;
    memoria[428] = 0x00; // Pointer to
    memoria[429] = 0x02; // raw data
    memoria[430] = 0x00;
    memoria[431] = 0x00;
    memoria[432] = 0x00; // Pointer to
    memoria[433] = 0x00; // relocations
    memoria[434] = 0x00;
    memoria[435] = 0x00;
    memoria[436] = 0x00; // Pointer to
    memoria[437] = 0x00; // line numbers
    memoria[438] = 0x00;
    memoria[439] = 0x00;
    memoria[440] = 0x00; // Number of
    memoria[441] = 0x00; // relocations
    memoria[442] = 0x00; // Number of
    memoria[443] = 0x00; // line numbers
    memoria[444] = 0x20; // Characteristics
    memoria[445] = 0x00; // (Readable,
    memoria[446] = 0x00; // Writeable &
    memoria[447] = 0xE0; // Executable)
    for (i=448;i<511;i++)
        memoria[i] = 0x00;
    memoria[512]=0x6E;
    memoria[513]=0x10;
    memoria[514]=0x00;
    memoria[515]=0x00;
    memoria[516]=0x7C;
    memoria[517]=0x10;
    memoria[518]=0x00;
    memoria[519]=0x00;
    memoria[520]=0x8C;
    memoria[521]=0x10;
    memoria[522]=0x00;
    memoria[523]=0x00;
    memoria[524]=0x98;
    memoria[525]=0x10;
    memoria[526]=0x00;
    memoria[527]=0x00;
    memoria[528]=0xA4;
    memoria[529]=0x10;
    memoria[530]=0x00;
    memoria[531]=0x00;
    memoria[532]=0xB6;
    memoria[533]=0x10;
    memoria[534]=0x00;
    memoria[535]=0x00;
    memoria[536]=0x00;
    memoria[537]=0x00;
    memoria[538]=0x00;
    memoria[539]=0x00;
    memoria[540]=0x52;
    memoria[541]=0x10;
    memoria[542]=0x00;
    memoria[543]=0x00;
    memoria[544]=0x00;
    memoria[545]=0x00;
    memoria[546]=0x00;
    memoria[547]=0x00;
    memoria[548]=0x00;
    memoria[549]=0x00;
    memoria[550]=0x00;
    memoria[551]=0x00;
    memoria[552]=0x44;
    memoria[553]=0x10;
    memoria[554]=0x00;
    memoria[555]=0x00;
    memoria[556]=0x00;
    memoria[557]=0x10;
    memoria[558]=0x00;
    memoria[559]=0x00;
    memoria[560]=0x00;
    memoria[561]=0x00;
    memoria[562]=0x00;
    memoria[563]=0x00;
    memoria[564]=0x00;
    memoria[565]=0x00;
    memoria[566]=0x00;
    memoria[567]=0x00;
    memoria[568]=0x00;
    memoria[569]=0x00;
    memoria[570]=0x00;
    memoria[571]=0x00;
    memoria[572]=0x00;
    memoria[573]=0x00;
    memoria[574]=0x00;
    memoria[575]=0x00;
    memoria[576]=0x00;
    memoria[577]=0x00;
    memoria[578]=0x00;
    memoria[579]=0x00;
    memoria[580]=0x4B;
    memoria[581]=0x45;
    memoria[582]=0x52;
    memoria[583]=0x4E;
    memoria[584]=0x45;
    memoria[585]=0x4C;
    memoria[586]=0x33;
    memoria[587]=0x32;
    memoria[588]=0x2E;
    memoria[589]=0x64;
    memoria[590]=0x6C;
    memoria[591]=0x6C;
    memoria[592]=0x00;
    memoria[593]=0x00;
    memoria[594]=0x6E;
    memoria[595]=0x10;
    memoria[596]=0x00;
    memoria[597]=0x00;
    memoria[598]=0x7C;
    memoria[599]=0x10;
    memoria[600]=0x00;
    memoria[601]=0x00;
    memoria[602]=0x8C;
    memoria[603]=0x10;
    memoria[604]=0x00;
    memoria[605]=0x00;
    memoria[606]=0x98;
    memoria[607]=0x10;
    memoria[608]=0x00;
    memoria[609]=0x00;
    memoria[610]=0xA4;
    memoria[611]=0x10;
    memoria[612]=0x00;
    memoria[613]=0x00;
    memoria[614]=0xB6;
    memoria[615]=0x10;
    memoria[616]=0x00;
    memoria[617]=0x00;
    memoria[618]=0x00;
    memoria[619]=0x00;
    memoria[620]=0x00;
    memoria[621]=0x00;
    memoria[622]=0x00;
    memoria[623]=0x00;
    memoria[624]=0x45;
    memoria[625]=0x78;
    memoria[626]=0x69;
    memoria[627]=0x74;
    memoria[628]=0x50;
    memoria[629]=0x72;
    memoria[630]=0x6F;
    memoria[631]=0x63;
    memoria[632]=0x65;
    memoria[633]=0x73;
    memoria[634]=0x73;
    memoria[635]=0x00;
    memoria[636]=0x00;
    memoria[637]=0x00;
    memoria[638]=0x47;
    memoria[639]=0x65;
    memoria[640]=0x74;
    memoria[641]=0x53;
    memoria[642]=0x74;
    memoria[643]=0x64;
    memoria[644]=0x48;
    memoria[645]=0x61;
    memoria[646]=0x6E;
    memoria[647]=0x64;
    memoria[648]=0x6C;
    memoria[649]=0x65;
    memoria[650]=0x00;
    memoria[651]=0x00;
    memoria[652]=0x00;
    memoria[653]=0x00;
    memoria[654]=0x52;
    memoria[655]=0x65;
    memoria[656]=0x61;
    memoria[657]=0x64;
    memoria[658]=0x46;
    memoria[659]=0x69;
    memoria[660]=0x6C;
    memoria[661]=0x65;
    memoria[662]=0x00;
    memoria[663]=0x00;
    memoria[664]=0x00;
    memoria[665]=0x00;
    memoria[666]=0x57;
    memoria[667]=0x72;
    memoria[668]=0x69;
    memoria[669]=0x74;
    memoria[670]=0x65;
    memoria[671]=0x46;
    memoria[672]=0x69;
    memoria[673]=0x6C;
    memoria[674]=0x65;
    memoria[675]=0x00;
    memoria[676]=0x00;
    memoria[677]=0x00;
    memoria[678]=0x47;
    memoria[679]=0x65;
    memoria[680]=0x74;
    memoria[681]=0x43;
    memoria[682]=0x6F;
    memoria[683]=0x6E;
    memoria[684]=0x73;
    memoria[685]=0x6F;
    memoria[686]=0x6C;
    memoria[687]=0x65;
    memoria[688]=0x4D;
    memoria[689]=0x6F;
    memoria[690]=0x64;
    memoria[691]=0x65;
    memoria[692]=0x00;
    memoria[693]=0x00;
    memoria[694]=0x00;
    memoria[695]=0x00;
    memoria[696]=0x53;
    memoria[697]=0x65;
    memoria[698]=0x74;
    memoria[699]=0x43;
    memoria[700]=0x6F;
    memoria[701]=0x6E;
    memoria[702]=0x73;
    memoria[703]=0x6F;
    memoria[704]=0x6C;
    memoria[705]=0x65;
    memoria[706]=0x4D;
    memoria[707]=0x6F;
    memoria[708]=0x64;
    memoria[709]=0x65;
    memoria[710]=0x00;
    memoria[711]=0x00;
    memoria[712]=0x00;
    memoria[713]=0x00;
    memoria[714]=0x00;
    memoria[715]=0x00;
    memoria[716]=0x00;
    memoria[717]=0x00;
    memoria[718]=0x00;
    memoria[719]=0x00;
    memoria[720]=0x50;
    memoria[721]=0xA2;
    memoria[722]=0x1C;
    memoria[723]=0x11;
    memoria[724]=0x40;
    memoria[725]=0x00;
    memoria[726]=0x31;
    memoria[727]=0xC0;
    memoria[728]=0x03;
    memoria[729]=0x05;
    memoria[730]=0x2C;
    memoria[731]=0x11;
    memoria[732]=0x40;
    memoria[733]=0x00;
    memoria[734]=0x75;
    memoria[735]=0x0D;
    memoria[736]=0x6A;
    memoria[737]=0xF5;
    memoria[738]=0xFF;
    memoria[739]=0x15;
    memoria[740]=0x04;
    memoria[741]=0x10;
    memoria[742]=0x40;
    memoria[743]=0x00;
    memoria[744]=0xA3;
    memoria[745]=0x2C;
    memoria[746]=0x11;
    memoria[747]=0x40;
    memoria[748]=0x00;
    memoria[749]=0x6A;
    memoria[750]=0x00;
    memoria[751]=0x68;
    memoria[752]=0x30;
    memoria[753]=0x11;
    memoria[754]=0x40;
    memoria[755]=0x00;
    memoria[756]=0x6A;
    memoria[757]=0x01;
    memoria[758]=0x68;
    memoria[759]=0x1C;
    memoria[760]=0x11;
    memoria[761]=0x40;
    memoria[762]=0x00;
    memoria[763]=0x50;
    memoria[764]=0xFF;
    memoria[765]=0x15;
    memoria[766]=0x0C;
    memoria[767]=0x10;
    memoria[768]=0x40;
    memoria[769]=0x00;
    memoria[770]=0x09;
    memoria[771]=0xC0;
    memoria[772]=0x75;
    memoria[773]=0x08;
    memoria[774]=0x6A;
    memoria[775]=0x00;
    memoria[776]=0xFF;
    memoria[777]=0x15;
    memoria[778]=0x00;
    memoria[779]=0x10;
    memoria[780]=0x40;
    memoria[781]=0x00;
    memoria[782]=0x81;
    memoria[783]=0x3D;
    memoria[784]=0x30;
    memoria[785]=0x11;
    memoria[786]=0x40;
    memoria[787]=0x00;
    memoria[788]=0x01;
    memoria[789]=0x00;
    memoria[790]=0x00;
    memoria[791]=0x00;
    memoria[792]=0x75;
    memoria[793]=0xEC;
    memoria[794]=0x58;
    memoria[795]=0xC3;
    memoria[796]=0x00;
    memoria[797]=0x57;
    memoria[798]=0x72;
    memoria[799]=0x69;
    memoria[800]=0x74;
    memoria[801]=0x65;
    memoria[802]=0x20;
    memoria[803]=0x65;
    memoria[804]=0x72;
    memoria[805]=0x72;
    memoria[806]=0x6F;
    memoria[807]=0x72;
    memoria[808]=0x00;
    memoria[809]=0x00;
    memoria[810]=0x00;
    memoria[811]=0x00;
    memoria[812]=0x00;
    memoria[813]=0x00;
    memoria[814]=0x00;
    memoria[815]=0x00;
    memoria[816]=0x00;
    memoria[817]=0x00;
    memoria[818]=0x00;
    memoria[819]=0x00;
    memoria[820]=0x00;
    memoria[821]=0x00;
    memoria[822]=0x00;
    memoria[823]=0x00;
    memoria[824]=0x00;
    memoria[825]=0x00;
    memoria[826]=0x00;
    memoria[827]=0x00;
    memoria[828]=0x00;
    memoria[829]=0x00;
    memoria[830]=0x00;
    memoria[831]=0x00;
    memoria[832]=0x60;
    memoria[833]=0x31;
    memoria[834]=0xC0;
    memoria[835]=0x03;
    memoria[836]=0x05;
    memoria[837]=0xCC;
    memoria[838]=0x11;
    memoria[839]=0x40;
    memoria[840]=0x00;
    memoria[841]=0x75;
    memoria[842]=0x37;
    memoria[843]=0x6A;
    memoria[844]=0xF6;
    memoria[845]=0xFF;
    memoria[846]=0x15;
    memoria[847]=0x04;
    memoria[848]=0x10;
    memoria[849]=0x40;
    memoria[850]=0x00;
    memoria[851]=0xA3;
    memoria[852]=0xCC;
    memoria[853]=0x11;
    memoria[854]=0x40;
    memoria[855]=0x00;
    memoria[856]=0x68;
    memoria[857]=0xD0;
    memoria[858]=0x11;
    memoria[859]=0x40;
    memoria[860]=0x00;
    memoria[861]=0x50;
    memoria[862]=0xFF;
    memoria[863]=0x15;
    memoria[864]=0x10;
    memoria[865]=0x10;
    memoria[866]=0x40;
    memoria[867]=0x00;
    memoria[868]=0x80;
    memoria[869]=0x25;
    memoria[870]=0xD0;
    memoria[871]=0x11;
    memoria[872]=0x40;
    memoria[873]=0x00;
    memoria[874]=0xF9;
    memoria[875]=0xFF;
    memoria[876]=0x35;
    memoria[877]=0xD0;
    memoria[878]=0x11;
    memoria[879]=0x40;
    memoria[880]=0x00;
    memoria[881]=0xFF;
    memoria[882]=0x35;
    memoria[883]=0xCC;
    memoria[884]=0x11;
    memoria[885]=0x40;
    memoria[886]=0x00;
    memoria[887]=0xFF;
    memoria[888]=0x15;
    memoria[889]=0x14;
    memoria[890]=0x10;
    memoria[891]=0x40;
    memoria[892]=0x00;
    memoria[893]=0xA1;
    memoria[894]=0xCC;
    memoria[895]=0x11;
    memoria[896]=0x40;
    memoria[897]=0x00;
    memoria[898]=0x6A;
    memoria[899]=0x00;
    memoria[900]=0x68;
    memoria[901]=0xD4;
    memoria[902]=0x11;
    memoria[903]=0x40;
    memoria[904]=0x00;
    memoria[905]=0x6A;
    memoria[906]=0x01;
    memoria[907]=0x68;
    memoria[908]=0xBE;
    memoria[909]=0x11;
    memoria[910]=0x40;
    memoria[911]=0x00;
    memoria[912]=0x50;
    memoria[913]=0xFF;
    memoria[914]=0x15;
    memoria[915]=0x08;
    memoria[916]=0x10;
    memoria[917]=0x40;
    memoria[918]=0x00;
    memoria[919]=0x09;
    memoria[920]=0xC0;
    memoria[921]=0x61;
    memoria[922]=0x90;
    memoria[923]=0x75;
    memoria[924]=0x08;
    memoria[925]=0x6A;
    memoria[926]=0x00;
    memoria[927]=0xFF;
    memoria[928]=0x15;
    memoria[929]=0x00;
    memoria[930]=0x10;
    memoria[931]=0x40;
    memoria[932]=0x00;
    memoria[933]=0x0F;
    memoria[934]=0xB6;
    memoria[935]=0x05;
    memoria[936]=0xBE;
    memoria[937]=0x11;
    memoria[938]=0x40;
    memoria[939]=0x00;
    memoria[940]=0x81;
    memoria[941]=0x3D;
    memoria[942]=0xD4;
    memoria[943]=0x11;
    memoria[944]=0x40;
    memoria[945]=0x00;
    memoria[946]=0x01;
    memoria[947]=0x00;
    memoria[948]=0x00;
    memoria[949]=0x00;
    memoria[950]=0x74;
    memoria[951]=0x05;
    memoria[952]=0xB8;
    memoria[953]=0xFF;
    memoria[954]=0xFF;
    memoria[955]=0xFF;
    memoria[956]=0xFF;
    memoria[957]=0xC3;
    memoria[958]=0x00;
    memoria[959]=0x52;
    memoria[960]=0x65;
    memoria[961]=0x61;
    memoria[962]=0x64;
    memoria[963]=0x20;
    memoria[964]=0x65;
    memoria[965]=0x72;
    memoria[966]=0x72;
    memoria[967]=0x6F;
    memoria[968]=0x72;
    memoria[969]=0x00;
    memoria[970]=0x00;
    memoria[971]=0x00;
    memoria[972]=0x00;
    memoria[973]=0x00;
    memoria[974]=0x00;
    memoria[975]=0x00;
    memoria[976]=0x00;
    memoria[977]=0x00;
    memoria[978]=0x00;
    memoria[979]=0x00;
    memoria[980]=0x00;
    memoria[981]=0x00;
    memoria[982]=0x00;
    memoria[983]=0x00;
    memoria[984]=0x00;
    memoria[985]=0x00;
    memoria[986]=0x00;
    memoria[987]=0x00;
    memoria[988]=0x00;
    memoria[989]=0x00;
    memoria[990]=0x00;
    memoria[991]=0x00;
    memoria[992]=0x60;
    memoria[993]=0x89;
    memoria[994]=0xC6;
    memoria[995]=0x30;
    memoria[996]=0xC0;
    memoria[997]=0x02;
    memoria[998]=0x06;
    memoria[999]=0x74;
    memoria[1000]=0x08;
    memoria[1001]=0x46;
    memoria[1002]=0xE8;
    memoria[1003]=0xE1;
    memoria[1004]=0xFE;
    memoria[1005]=0xFF;
    memoria[1006]=0xFF;
    memoria[1007]=0xEB;
    memoria[1008]=0xF2;
    memoria[1009]=0x61;
    memoria[1010]=0x90;
    memoria[1011]=0xC3;
    memoria[1012]=0x00;
    memoria[1013]=0x00;
    memoria[1014]=0x00;
    memoria[1015]=0x00;
    memoria[1016]=0x00;
    memoria[1017]=0x00;
    memoria[1018]=0x00;
    memoria[1019]=0x00;
    memoria[1020]=0x00;
    memoria[1021]=0x00;
    memoria[1022]=0x00;
    memoria[1023]=0x00;
    memoria[1024]=0x04;
    memoria[1025]=0x30;
    memoria[1026]=0xE8;
    memoria[1027]=0xC9;
    memoria[1028]=0xFE;
    memoria[1029]=0xFF;
    memoria[1030]=0xFF;
    memoria[1031]=0xC3;
    memoria[1032]=0x00;
    memoria[1033]=0x00;
    memoria[1034]=0x00;
    memoria[1035]=0x00;
    memoria[1036]=0x00;
    memoria[1037]=0x00;
    memoria[1038]=0x00;
    memoria[1039]=0x00;
    memoria[1040]=0xB0;
    memoria[1041]=0x0D;
    memoria[1042]=0xE8;
    memoria[1043]=0xB9;
    memoria[1044]=0xFE;
    memoria[1045]=0xFF;
    memoria[1046]=0xFF;
    memoria[1047]=0xB0;
    memoria[1048]=0x0A;
    memoria[1049]=0xE8;
    memoria[1050]=0xB2;
    memoria[1051]=0xFE;
    memoria[1052]=0xFF;
    memoria[1053]=0xFF;
    memoria[1054]=0xC3;
    memoria[1055]=0x00;
    memoria[1056]=0x3D;
    memoria[1057]=0x00;
    memoria[1058]=0x00;
    memoria[1059]=0x00;
    memoria[1060]=0x80;
    memoria[1061]=0x75;
    memoria[1062]=0x4E;
    memoria[1063]=0xB0;
    memoria[1064]=0x2D;
    memoria[1065]=0xE8;
    memoria[1066]=0xA2;
    memoria[1067]=0xFE;
    memoria[1068]=0xFF;
    memoria[1069]=0xFF;
    memoria[1070]=0xB0;
    memoria[1071]=0x02;
    memoria[1072]=0xE8;
    memoria[1073]=0xCB;
    memoria[1074]=0xFF;
    memoria[1075]=0xFF;
    memoria[1076]=0xFF;
    memoria[1077]=0xB0;
    memoria[1078]=0x01;
    memoria[1079]=0xE8;
    memoria[1080]=0xC4;
    memoria[1081]=0xFF;
    memoria[1082]=0xFF;
    memoria[1083]=0xFF;
    memoria[1084]=0xB0;
    memoria[1085]=0x04;
    memoria[1086]=0xE8;
    memoria[1087]=0xBD;
    memoria[1088]=0xFF;
    memoria[1089]=0xFF;
    memoria[1090]=0xFF;
    memoria[1091]=0xB0;
    memoria[1092]=0x07;
    memoria[1093]=0xE8;
    memoria[1094]=0xB6;
    memoria[1095]=0xFF;
    memoria[1096]=0xFF;
    memoria[1097]=0xFF;
    memoria[1098]=0xB0;
    memoria[1099]=0x04;
    memoria[1100]=0xE8;
    memoria[1101]=0xAF;
    memoria[1102]=0xFF;
    memoria[1103]=0xFF;
    memoria[1104]=0xFF;
    memoria[1105]=0xB0;
    memoria[1106]=0x08;
    memoria[1107]=0xE8;
    memoria[1108]=0xA8;
    memoria[1109]=0xFF;
    memoria[1110]=0xFF;
    memoria[1111]=0xFF;
    memoria[1112]=0xB0;
    memoria[1113]=0x03;
    memoria[1114]=0xE8;
    memoria[1115]=0xA1;
    memoria[1116]=0xFF;
    memoria[1117]=0xFF;
    memoria[1118]=0xFF;
    memoria[1119]=0xB0;
    memoria[1120]=0x06;
    memoria[1121]=0xE8;
    memoria[1122]=0x9A;
    memoria[1123]=0xFF;
    memoria[1124]=0xFF;
    memoria[1125]=0xFF;
    memoria[1126]=0xB0;
    memoria[1127]=0x04;
    memoria[1128]=0xE8;
    memoria[1129]=0x93;
    memoria[1130]=0xFF;
    memoria[1131]=0xFF;
    memoria[1132]=0xFF;
    memoria[1133]=0xB0;
    memoria[1134]=0x08;
    memoria[1135]=0xE8;
    memoria[1136]=0x8C;
    memoria[1137]=0xFF;
    memoria[1138]=0xFF;
    memoria[1139]=0xFF;
    memoria[1140]=0xC3;
    memoria[1141]=0x3D;
    memoria[1142]=0x00;
    memoria[1143]=0x00;
    memoria[1144]=0x00;
    memoria[1145]=0x00;
    memoria[1146]=0x7D;
    memoria[1147]=0x0B;
    memoria[1148]=0x50;
    memoria[1149]=0xB0;
    memoria[1150]=0x2D;
    memoria[1151]=0xE8;
    memoria[1152]=0x4C;
    memoria[1153]=0xFE;
    memoria[1154]=0xFF;
    memoria[1155]=0xFF;
    memoria[1156]=0x58;
    memoria[1157]=0xF7;
    memoria[1158]=0xD8;
    memoria[1159]=0x3D;
    memoria[1160]=0x0A;
    memoria[1161]=0x00;
    memoria[1162]=0x00;
    memoria[1163]=0x00;
    memoria[1164]=0x0F;
    memoria[1165]=0x8C;
    memoria[1166]=0xEF;
    memoria[1167]=0x00;
    memoria[1168]=0x00;
    memoria[1169]=0x00;
    memoria[1170]=0x3D;
    memoria[1171]=0x64;
    memoria[1172]=0x00;
    memoria[1173]=0x00;
    memoria[1174]=0x00;
    memoria[1175]=0x0F;
    memoria[1176]=0x8C;
    memoria[1177]=0xD1;
    memoria[1178]=0x00;
    memoria[1179]=0x00;
    memoria[1180]=0x00;
    memoria[1181]=0x3D;
    memoria[1182]=0xE8;
    memoria[1183]=0x03;
    memoria[1184]=0x00;
    memoria[1185]=0x00;
    memoria[1186]=0x0F;
    memoria[1187]=0x8C;
    memoria[1188]=0xB3;
    memoria[1189]=0x00;
    memoria[1190]=0x00;
    memoria[1191]=0x00;
    memoria[1192]=0x3D;
    memoria[1193]=0x10;
    memoria[1194]=0x27;
    memoria[1195]=0x00;
    memoria[1196]=0x00;
    memoria[1197]=0x0F;
    memoria[1198]=0x8C;
    memoria[1199]=0x95;
    memoria[1200]=0x00;
    memoria[1201]=0x00;
    memoria[1202]=0x00;
    memoria[1203]=0x3D;
    memoria[1204]=0xA0;
    memoria[1205]=0x86;
    memoria[1206]=0x01;
    memoria[1207]=0x00;
    memoria[1208]=0x7C;
    memoria[1209]=0x7B;
    memoria[1210]=0x3D;
    memoria[1211]=0x40;
    memoria[1212]=0x42;
    memoria[1213]=0x0F;
    memoria[1214]=0x00;
    memoria[1215]=0x7C;
    memoria[1216]=0x61;
    memoria[1217]=0x3D;
    memoria[1218]=0x80;
    memoria[1219]=0x96;
    memoria[1220]=0x98;
    memoria[1221]=0x00;
    memoria[1222]=0x7C;
    memoria[1223]=0x47;
    memoria[1224]=0x3D;
    memoria[1225]=0x00;
    memoria[1226]=0xE1;
    memoria[1227]=0xF5;
    memoria[1228]=0x05;
    memoria[1229]=0x7C;
    memoria[1230]=0x2D;
    memoria[1231]=0x3D;
    memoria[1232]=0x00;
    memoria[1233]=0xCA;
    memoria[1234]=0x9A;
    memoria[1235]=0x3B;
    memoria[1236]=0x7C;
    memoria[1237]=0x13;
    memoria[1238]=0xBA;
    memoria[1239]=0x00;
    memoria[1240]=0x00;
    memoria[1241]=0x00;
    memoria[1242]=0x00;
    memoria[1243]=0xBB;
    memoria[1244]=0x00;
    memoria[1245]=0xCA;
    memoria[1246]=0x9A;
    memoria[1247]=0x3B;
    memoria[1248]=0xF7;
    memoria[1249]=0xFB;
    memoria[1250]=0x52;
    memoria[1251]=0xE8;
    memoria[1252]=0x18;
    memoria[1253]=0xFF;
    memoria[1254]=0xFF;
    memoria[1255]=0xFF;
    memoria[1256]=0x58;
    memoria[1257]=0xBA;
    memoria[1258]=0x00;
    memoria[1259]=0x00;
    memoria[1260]=0x00;
    memoria[1261]=0x00;
    memoria[1262]=0xBB;
    memoria[1263]=0x00;
    memoria[1264]=0xE1;
    memoria[1265]=0xF5;
    memoria[1266]=0x05;
    memoria[1267]=0xF7;
    memoria[1268]=0xFB;
    memoria[1269]=0x52;
    memoria[1270]=0xE8;
    memoria[1271]=0x05;
    memoria[1272]=0xFF;
    memoria[1273]=0xFF;
    memoria[1274]=0xFF;
    memoria[1275]=0x58;
    memoria[1276]=0xBA;
    memoria[1277]=0x00;
    memoria[1278]=0x00;
    memoria[1279]=0x00;
    memoria[1280]=0x00;
    memoria[1281]=0xBB;
    memoria[1282]=0x80;
    memoria[1283]=0x96;
    memoria[1284]=0x98;
    memoria[1285]=0x00;
    memoria[1286]=0xF7;
    memoria[1287]=0xFB;
    memoria[1288]=0x52;
    memoria[1289]=0xE8;
    memoria[1290]=0xF2;
    memoria[1291]=0xFE;
    memoria[1292]=0xFF;
    memoria[1293]=0xFF;
    memoria[1294]=0x58;
    memoria[1295]=0xBA;
    memoria[1296]=0x00;
    memoria[1297]=0x00;
    memoria[1298]=0x00;
    memoria[1299]=0x00;
    memoria[1300]=0xBB;
    memoria[1301]=0x40;
    memoria[1302]=0x42;
    memoria[1303]=0x0F;
    memoria[1304]=0x00;
    memoria[1305]=0xF7;
    memoria[1306]=0xFB;
    memoria[1307]=0x52;
    memoria[1308]=0xE8;
    memoria[1309]=0xDF;
    memoria[1310]=0xFE;
    memoria[1311]=0xFF;
    memoria[1312]=0xFF;
    memoria[1313]=0x58;
    memoria[1314]=0xBA;
    memoria[1315]=0x00;
    memoria[1316]=0x00;
    memoria[1317]=0x00;
    memoria[1318]=0x00;
    memoria[1319]=0xBB;
    memoria[1320]=0xA0;
    memoria[1321]=0x86;
    memoria[1322]=0x01;
    memoria[1323]=0x00;
    memoria[1324]=0xF7;
    memoria[1325]=0xFB;
    memoria[1326]=0x52;
    memoria[1327]=0xE8;
    memoria[1328]=0xCC;
    memoria[1329]=0xFE;
    memoria[1330]=0xFF;
    memoria[1331]=0xFF;
    memoria[1332]=0x58;
    memoria[1333]=0xBA;
    memoria[1334]=0x00;
    memoria[1335]=0x00;
    memoria[1336]=0x00;
    memoria[1337]=0x00;
    memoria[1338]=0xBB;
    memoria[1339]=0x10;
    memoria[1340]=0x27;
    memoria[1341]=0x00;
    memoria[1342]=0x00;
    memoria[1343]=0xF7;
    memoria[1344]=0xFB;
    memoria[1345]=0x52;
    memoria[1346]=0xE8;
    memoria[1347]=0xB9;
    memoria[1348]=0xFE;
    memoria[1349]=0xFF;
    memoria[1350]=0xFF;
    memoria[1351]=0x58;
    memoria[1352]=0xBA;
    memoria[1353]=0x00;
    memoria[1354]=0x00;
    memoria[1355]=0x00;
    memoria[1356]=0x00;
    memoria[1357]=0xBB;
    memoria[1358]=0xE8;
    memoria[1359]=0x03;
    memoria[1360]=0x00;
    memoria[1361]=0x00;
    memoria[1362]=0xF7;
    memoria[1363]=0xFB;
    memoria[1364]=0x52;
    memoria[1365]=0xE8;
    memoria[1366]=0xA6;
    memoria[1367]=0xFE;
    memoria[1368]=0xFF;
    memoria[1369]=0xFF;
    memoria[1370]=0x58;
    memoria[1371]=0xBA;
    memoria[1372]=0x00;
    memoria[1373]=0x00;
    memoria[1374]=0x00;
    memoria[1375]=0x00;
    memoria[1376]=0xBB;
    memoria[1377]=0x64;
    memoria[1378]=0x00;
    memoria[1379]=0x00;
    memoria[1380]=0x00;
    memoria[1381]=0xF7;
    memoria[1382]=0xFB;
    memoria[1383]=0x52;
    memoria[1384]=0xE8;
    memoria[1385]=0x93;
    memoria[1386]=0xFE;
    memoria[1387]=0xFF;
    memoria[1388]=0xFF;
    memoria[1389]=0x58;
    memoria[1390]=0xBA;
    memoria[1391]=0x00;
    memoria[1392]=0x00;
    memoria[1393]=0x00;
    memoria[1394]=0x00;
    memoria[1395]=0xBB;
    memoria[1396]=0x0A;
    memoria[1397]=0x00;
    memoria[1398]=0x00;
    memoria[1399]=0x00;
    memoria[1400]=0xF7;
    memoria[1401]=0xFB;
    memoria[1402]=0x52;
    memoria[1403]=0xE8;
    memoria[1404]=0x80;
    memoria[1405]=0xFE;
    memoria[1406]=0xFF;
    memoria[1407]=0xFF;
    memoria[1408]=0x58;
    memoria[1409]=0xE8;
    memoria[1410]=0x7A;
    memoria[1411]=0xFE;
    memoria[1412]=0xFF;
    memoria[1413]=0xFF;
    memoria[1414]=0xC3;
    memoria[1415]=0x00;
    memoria[1416]=0xFF;
    memoria[1417]=0x15;
    memoria[1418]=0x00;
    memoria[1419]=0x10;
    memoria[1420]=0x40;
    memoria[1421]=0x00;
    memoria[1422]=0x00;
    memoria[1423]=0x00;
    memoria[1424]=0xB9;
    memoria[1425]=0x00;
    memoria[1426]=0x00;
    memoria[1427]=0x00;
    memoria[1428]=0x00;
    memoria[1429]=0xB3;
    memoria[1430]=0x03;
    memoria[1431]=0x51;
    memoria[1432]=0x53;
    memoria[1433]=0xE8;
    memoria[1434]=0xA2;
    memoria[1435]=0xFD;
    memoria[1436]=0xFF;
    memoria[1437]=0xFF;
    memoria[1438]=0x5B;
    memoria[1439]=0x59;
    memoria[1440]=0x3C;
    memoria[1441]=0x0D;
    memoria[1442]=0x0F;
    memoria[1443]=0x84;
    memoria[1444]=0x34;
    memoria[1445]=0x01;
    memoria[1446]=0x00;
    memoria[1447]=0x00;
    memoria[1448]=0x3C;
    memoria[1449]=0x08;
    memoria[1450]=0x0F;
    memoria[1451]=0x84;
    memoria[1452]=0x94;
    memoria[1453]=0x00;
    memoria[1454]=0x00;
    memoria[1455]=0x00;
    memoria[1456]=0x3C;
    memoria[1457]=0x2D;
    memoria[1458]=0x0F;
    memoria[1459]=0x84;
    memoria[1460]=0x09;
    memoria[1461]=0x01;
    memoria[1462]=0x00;
    memoria[1463]=0x00;
    memoria[1464]=0x3C;
    memoria[1465]=0x30;
    memoria[1466]=0x7C;
    memoria[1467]=0xDB;
    memoria[1468]=0x3C;
    memoria[1469]=0x39;
    memoria[1470]=0x7F;
    memoria[1471]=0xD7;
    memoria[1472]=0x2C;
    memoria[1473]=0x30;
    memoria[1474]=0x80;
    memoria[1475]=0xFB;
    memoria[1476]=0x00;
    memoria[1477]=0x74;
    memoria[1478]=0xD0;
    memoria[1479]=0x80;
    memoria[1480]=0xFB;
    memoria[1481]=0x02;
    memoria[1482]=0x75;
    memoria[1483]=0x0C;
    memoria[1484]=0x81;
    memoria[1485]=0xF9;
    memoria[1486]=0x00;
    memoria[1487]=0x00;
    memoria[1488]=0x00;
    memoria[1489]=0x00;
    memoria[1490]=0x75;
    memoria[1491]=0x04;
    memoria[1492]=0x3C;
    memoria[1493]=0x00;
    memoria[1494]=0x74;
    memoria[1495]=0xBF;
    memoria[1496]=0x80;
    memoria[1497]=0xFB;
    memoria[1498]=0x03;
    memoria[1499]=0x75;
    memoria[1500]=0x0A;
    memoria[1501]=0x3C;
    memoria[1502]=0x00;
    memoria[1503]=0x75;
    memoria[1504]=0x04;
    memoria[1505]=0xB3;
    memoria[1506]=0x00;
    memoria[1507]=0xEB;
    memoria[1508]=0x02;
    memoria[1509]=0xB3;
    memoria[1510]=0x01;
    memoria[1511]=0x81;
    memoria[1512]=0xF9;
    memoria[1513]=0xCC;
    memoria[1514]=0xCC;
    memoria[1515]=0xCC;
    memoria[1516]=0x0C;
    memoria[1517]=0x7F;
    memoria[1518]=0xA8;
    memoria[1519]=0x81;
    memoria[1520]=0xF9;
    memoria[1521]=0x34;
    memoria[1522]=0x33;
    memoria[1523]=0x33;
    memoria[1524]=0xF3;
    memoria[1525]=0x7C;
    memoria[1526]=0xA0;
    memoria[1527]=0x88;
    memoria[1528]=0xC7;
    memoria[1529]=0xB8;
    memoria[1530]=0x0A;
    memoria[1531]=0x00;
    memoria[1532]=0x00;
    memoria[1533]=0x00;
    memoria[1534]=0xF7;
    memoria[1535]=0xE9;
    memoria[1536]=0x3D;
    memoria[1537]=0x08;
    memoria[1538]=0x00;
    memoria[1539]=0x00;
    memoria[1540]=0x80;
    memoria[1541]=0x74;
    memoria[1542]=0x11;
    memoria[1543]=0x3D;
    memoria[1544]=0xF8;
    memoria[1545]=0xFF;
    memoria[1546]=0xFF;
    memoria[1547]=0x7F;
    memoria[1548]=0x75;
    memoria[1549]=0x13;
    memoria[1550]=0x80;
    memoria[1551]=0xFF;
    memoria[1552]=0x07;
    memoria[1553]=0x7E;
    memoria[1554]=0x0E;
    memoria[1555]=0xE9;
    memoria[1556]=0x7F;
    memoria[1557]=0xFF;
    memoria[1558]=0xFF;
    memoria[1559]=0xFF;
    memoria[1560]=0x80;
    memoria[1561]=0xFF;
    memoria[1562]=0x08;
    memoria[1563]=0x0F;
    memoria[1564]=0x8F;
    memoria[1565]=0x76;
    memoria[1566]=0xFF;
    memoria[1567]=0xFF;
    memoria[1568]=0xFF;
    memoria[1569]=0xB9;
    memoria[1570]=0x00;
    memoria[1571]=0x00;
    memoria[1572]=0x00;
    memoria[1573]=0x00;
    memoria[1574]=0x88;
    memoria[1575]=0xF9;
    memoria[1576]=0x80;
    memoria[1577]=0xFB;
    memoria[1578]=0x02;
    memoria[1579]=0x74;
    memoria[1580]=0x04;
    memoria[1581]=0x01;
    memoria[1582]=0xC1;
    memoria[1583]=0xEB;
    memoria[1584]=0x03;
    memoria[1585]=0x29;
    memoria[1586]=0xC8;
    memoria[1587]=0x91;
    memoria[1588]=0x88;
    memoria[1589]=0xF8;
    memoria[1590]=0x51;
    memoria[1591]=0x53;
    memoria[1592]=0xE8;
    memoria[1593]=0xC3;
    memoria[1594]=0xFD;
    memoria[1595]=0xFF;
    memoria[1596]=0xFF;
    memoria[1597]=0x5B;
    memoria[1598]=0x59;
    memoria[1599]=0xE9;
    memoria[1600]=0x53;
    memoria[1601]=0xFF;
    memoria[1602]=0xFF;
    memoria[1603]=0xFF;
    memoria[1604]=0x80;
    memoria[1605]=0xFB;
    memoria[1606]=0x03;
    memoria[1607]=0x0F;
    memoria[1608]=0x84;
    memoria[1609]=0x4A;
    memoria[1610]=0xFF;
    memoria[1611]=0xFF;
    memoria[1612]=0xFF;
    memoria[1613]=0x51;
    memoria[1614]=0x53;
    memoria[1615]=0xB0;
    memoria[1616]=0x08;
    memoria[1617]=0xE8;
    memoria[1618]=0x7A;
    memoria[1619]=0xFC;
    memoria[1620]=0xFF;
    memoria[1621]=0xFF;
    memoria[1622]=0xB0;
    memoria[1623]=0x20;
    memoria[1624]=0xE8;
    memoria[1625]=0x73;
    memoria[1626]=0xFC;
    memoria[1627]=0xFF;
    memoria[1628]=0xFF;
    memoria[1629]=0xB0;
    memoria[1630]=0x08;
    memoria[1631]=0xE8;
    memoria[1632]=0x6C;
    memoria[1633]=0xFC;
    memoria[1634]=0xFF;
    memoria[1635]=0xFF;
    memoria[1636]=0x5B;
    memoria[1637]=0x59;
    memoria[1638]=0x80;
    memoria[1639]=0xFB;
    memoria[1640]=0x00;
    memoria[1641]=0x75;
    memoria[1642]=0x07;
    memoria[1643]=0xB3;
    memoria[1644]=0x03;
    memoria[1645]=0xE9;
    memoria[1646]=0x25;
    memoria[1647]=0xFF;
    memoria[1648]=0xFF;
    memoria[1649]=0xFF;
    memoria[1650]=0x80;
    memoria[1651]=0xFB;
    memoria[1652]=0x02;
    memoria[1653]=0x75;
    memoria[1654]=0x0F;
    memoria[1655]=0x81;
    memoria[1656]=0xF9;
    memoria[1657]=0x00;
    memoria[1658]=0x00;
    memoria[1659]=0x00;
    memoria[1660]=0x00;
    memoria[1661]=0x75;
    memoria[1662]=0x07;
    memoria[1663]=0xB3;
    memoria[1664]=0x03;
    memoria[1665]=0xE9;
    memoria[1666]=0x11;
    memoria[1667]=0xFF;
    memoria[1668]=0xFF;
    memoria[1669]=0xFF;
    memoria[1670]=0x89;
    memoria[1671]=0xC8;
    memoria[1672]=0xB9;
    memoria[1673]=0x0A;
    memoria[1674]=0x00;
    memoria[1675]=0x00;
    memoria[1676]=0x00;
    memoria[1677]=0xBA;
    memoria[1678]=0x00;
    memoria[1679]=0x00;
    memoria[1680]=0x00;
    memoria[1681]=0x00;
    memoria[1682]=0x3D;
    memoria[1683]=0x00;
    memoria[1684]=0x00;
    memoria[1685]=0x00;
    memoria[1686]=0x00;
    memoria[1687]=0x7D;
    memoria[1688]=0x08;
    memoria[1689]=0xF7;
    memoria[1690]=0xD8;
    memoria[1691]=0xF7;
    memoria[1692]=0xF9;
    memoria[1693]=0xF7;
    memoria[1694]=0xD8;
    memoria[1695]=0xEB;
    memoria[1696]=0x02;
    memoria[1697]=0xF7;
    memoria[1698]=0xF9;
    memoria[1699]=0x89;
    memoria[1700]=0xC1;
    memoria[1701]=0x81;
    memoria[1702]=0xF9;
    memoria[1703]=0x00;
    memoria[1704]=0x00;
    memoria[1705]=0x00;
    memoria[1706]=0x00;
    memoria[1707]=0x0F;
    memoria[1708]=0x85;
    memoria[1709]=0xE6;
    memoria[1710]=0xFE;
    memoria[1711]=0xFF;
    memoria[1712]=0xFF;
    memoria[1713]=0x80;
    memoria[1714]=0xFB;
    memoria[1715]=0x02;
    memoria[1716]=0x0F;
    memoria[1717]=0x84;
    memoria[1718]=0xDD;
    memoria[1719]=0xFE;
    memoria[1720]=0xFF;
    memoria[1721]=0xFF;
    memoria[1722]=0xB3;
    memoria[1723]=0x03;
    memoria[1724]=0xE9;
    memoria[1725]=0xD6;
    memoria[1726]=0xFE;
    memoria[1727]=0xFF;
    memoria[1728]=0xFF;
    memoria[1729]=0x80;
    memoria[1730]=0xFB;
    memoria[1731]=0x03;
    memoria[1732]=0x0F;
    memoria[1733]=0x85;
    memoria[1734]=0xCD;
    memoria[1735]=0xFE;
    memoria[1736]=0xFF;
    memoria[1737]=0xFF;
    memoria[1738]=0xB0;
    memoria[1739]=0x2D;
    memoria[1740]=0x51;
    memoria[1741]=0x53;
    memoria[1742]=0xE8;
    memoria[1743]=0xFD;
    memoria[1744]=0xFB;
    memoria[1745]=0xFF;
    memoria[1746]=0xFF;
    memoria[1747]=0x5B;
    memoria[1748]=0x59;
    memoria[1749]=0xB3;
    memoria[1750]=0x02;
    memoria[1751]=0xE9;
    memoria[1752]=0xBB;
    memoria[1753]=0xFE;
    memoria[1754]=0xFF;
    memoria[1755]=0xFF;
    memoria[1756]=0x80;
    memoria[1757]=0xFB;
    memoria[1758]=0x03;
    memoria[1759]=0x0F;
    memoria[1760]=0x84;
    memoria[1761]=0xB2;
    memoria[1762]=0xFE;
    memoria[1763]=0xFF;
    memoria[1764]=0xFF;
    memoria[1765]=0x80;
    memoria[1766]=0xFB;
    memoria[1767]=0x02;
    memoria[1768]=0x75;
    memoria[1769]=0x0C;
    memoria[1770]=0x81;
    memoria[1771]=0xF9;
    memoria[1772]=0x00;
    memoria[1773]=0x00;
    memoria[1774]=0x00;
    memoria[1775]=0x00;
    memoria[1776]=0x0F;
    memoria[1777]=0x84;
    memoria[1778]=0xA1;
    memoria[1779]=0xFE;
    memoria[1780]=0xFF;
    memoria[1781]=0xFF;
    memoria[1782]=0x51;
    memoria[1783]=0xE8;
    memoria[1784]=0x14;
    memoria[1785]=0xFD;
    memoria[1786]=0xFF;
    memoria[1787]=0xFF;
    memoria[1788]=0x59;
    memoria[1789]=0x89;
    memoria[1790]=0xC8;
    memoria[1791]=0xC3; // Longitud fija de un programa 1792 bytes, el header de 448 y el resto de E/S.
    *topememoria=1792;
}

void Crear(byte memoria[], int topememoria, char argv[]){
    // Genera el archivo ejecutable a partir de los bytes acumulados en memoria.
    // Busca un punto en el nombre del archivo.
    // Si lo encuentra, reemplaza ese punto por el terminador '\0', eliminando la extensión desde allí.
    // Agrega la extensión .exe.
    // Abre el archivo en modo binario con fopen(argv, "wb").
    // Recorre memoria desde 0 hasta topememoria - 1 y escribe cada byte mediante fwrite().
    // Cierra el archivo.
    int index = -1;
    const char *ptr = strrchr(argv, '.');
    if(ptr) {
       index = ptr - argv;
    }

    if (index!=-1){
        argv[index]='\0';
    }
    strcat(argv,".exe");

    printf("\nSe creara el archivo binario %s\n",argv);
    FILE *archivo;
    int i=0;
    archivo=fopen(argv,"wb");
    for(i=0;i<topememoria;i++){
        fwrite(&memoria[i],sizeof(byte),1,archivo);
    }
    fclose(archivo);
}

void CargarInt (byte memoria[], int *topememoria, int n){
    // Guarda un entero de cuatro bytes en memoria, empezando en la posición indicada por *topememoria. 
    // Después avanza el contador cuatro posiciones.
    // Carga desde el menos significativo hasta el más significativo (little-endian).
    // Es el orden utilizado para representar los enteros en memoria en x86.
    // El byte menos significativo es el que contiene la parte de menor valor.
    // El más significativo es el que contiene la parte de mayor valor.
    // Cada byte puede representar 256 valores diferentes: de 0 a 255.
    unsigned int num; // num es una variable local que se utiliza para convertir el entero a una representación sin signo antes de separar sus bytes.
    if (n<0){ // Si n es negativo, la función calcula una representación equivalente a complemento a dos de 32 bits.
        num=4294967295U+n+1; // El valor 4294967295 es: 2^(32)-1. La letra U indica que el literal es de tipo entero sin signo.
    }
    else{
        num=n;
    }
        memoria[(*topememoria)]=num%256; // El módulo % 256 permite quedarse con el resto, que corresponde al byte que interesa extraer.
        (*topememoria)++; // Incrementa topememoria. La próxima escritura se hará en la posición siguiente.
        memoria[(*topememoria)]=(num/256)%256; // Divide el número por 256 para descartar el primer byte, y después usa % 256 para extraer el siguiente.
        // Las divisiones son divisiones enteras: se descarta la parte decimal.
        (*topememoria)++;
        memoria[(*topememoria)]=(num/256/256)%256;//printf(" %d \t",memoria[*topememoria]);
        (*topememoria)++;
        memoria[(*topememoria)]=(num/256/256/256);//printf(" %d \t",memoria[*topememoria]);
        (*topememoria)++;
    // ¿Por qué no guardarlos al revés? Porque el código máquina que está generando debe respetar el orden de bytes esperado por la arquitectura y por las instrucciones x86 que se ejecutarán.
}

void CargarIntEnMem (byte memoria[], int dirMem, int n){
    // Escribe un entero de cuatro bytes en una posición específica del arreglo, dirMem, pero no modifica topememoria.
    // Utiliza la misma conversión a cuatro bytes que CargarInt(), pero la escritura comienza en la posición que recibe como argumento.
    // Sirve para corregir un valor que ya se había escrito.
    unsigned int num;
    if (n<0){
        num=4294967295U+n+1;
    }
    else{
        num=n;
    }
        memoria[(dirMem++)]=num%256;//printf("\n %d \t",memoria[*topememoria]);
        memoria[(dirMem++)]=(num/256)%256;//printf(" %d \t",memoria[*topememoria]);
        memoria[(dirMem++)]=(num/256/256)%256;//printf(" %d \t",memoria[*topememoria]);
        memoria[(dirMem++)]=(num/256/256/256);//printf(" %d \t",memoria[*topememoria]);
}

int leerIntDeMem (byte memoria[], int dirMem){
    // No modifica topememoria, porque solamente lee.
    // Lee cuatro bytes consecutivos desde dirMem y los combina para reconstruir un entero.
    // Sirve para recuperar un entero que previamente fue almacenado en cuatro bytes.
    unsigned int entero=0;
    entero=entero+(memoria[dirMem++]);
    entero=entero+(256*memoria[dirMem++]);
    entero=entero+(256*256*memoria[dirMem++]);
    entero=entero+(256*256*256*memoria[dirMem++]);
    return(entero);
}

void CargarByte (byte memoria[], int *topememoria, byte n){
    // La función sirve para escribir un byte en esa posición y avanzar el contador para dejar libre la siguiente posición.
    // memoria es el arreglo donde se van guardando los bytes que forman el ejecutable que se está generando. 
    // topememoria indica en qué posición de ese arreglo debe escribirse el próximo byte.
    // byte memoria[]: recibe el arreglo donde se guardarán los bytes.
    // int *topememoria: recibe un puntero al contador, para poder modificar el contador original.
    // byte n: es el byte que queremos escribir.
    memoria[(*topememoria)++]=n; // Escribe n en la posición actual y luego incrementa el contador.
    // memoria [*topememoria]=n;
    // (*topememoria)++;
}

void CargarByteEnMem (byte memoria[], int dirMem, byte n){
    // Escribe un único byte en una posición específica.
    // Sirve para reemplazar un byte concreto que ya se había generado, sin agregar otro al final del código.
    memoria[dirMem]=n;
}

byte leerByte (byte memoria[], int topememoria){
    // Devuelve el byte que se encuentra en la posición indicada.
    return(memoria[topememoria]);
}

void cargarPopEAX (byte memoria[], int *topememoria){
    // Decide si conviene eliminar un PUSH EAX recién generado o agregar una instrucción POP EAX.
    if(leerByte(memoria, (*topememoria)-1)==0x50)
        (*topememoria)--;
    else
        CargarByte(memoria,topememoria,0x58);
}

/*Introducción
En este curso desarrollaremos un compilador para el lenguaje de 
programación PL/0, presentando una introducción general de la estructura 
y operación de los compiladores. 
1. Comentarios generales sobre la teoría de compiladores
El diseño de programas para resolver problemas complejos es mucho más 
sencillo utilizando lenguajes de alto nivel, ya que se requieren menos 
conocimientos sobre la estructura interna del computador, aunque es obvio 
que éste sólo entiende el código de máquina. Por lo tanto, para que un 
computador pueda ejecutar programas escritos en un lenguaje de alto 
nivel, éstos deben ser traducidos a código de máquina. A este proceso se 
lo denomina compilación, y la herramienta que la lleva a cabo se llama 
compilador. Por ende, los compiladores son fundamentales para la 
computación, y su importancia se mantendrá en el futuro. 
La entrada del compilador es el código fuente, es decir, el programa 
escrito en un lenguaje de alto nivel. El compilador analiza esta entrada 
y genera a su salida el código objeto. Existen distintas formas de código 
objeto, siendo una de las diferencias más destacables la que existe entre 
el código absoluto (el código de máquina con direcciones de memoria 
absolutas) y el código relocalizable (el código de máquina con 
desplazamientos de direcciones, y por lo tanto enlazable con otros 
módulos compilados por separado). 
p
pp
p Ensamblador
De acuerdo con los diferentes tipos de código y las diversas formas 
de funcionamiento, podemos distinguir entre los siguientes tipos de 
sistemas de compilación: 
: Traduce programas escritos en lenguaje ensamblador a 
código de máquina. El lenguaje ensamblador se caracteriza por el uso de 
mnemónicos para representar instrucciones y direcciones de memoria. 
p
pp
p Compilador
: Traduce programas escritos en un lenguaje de alto nivel a 
código intermedio o a código de máquina. El código intermedio puede ser, 
por ejemplo, un lenguaje ensamblador o alguna otra forma de 
representación intermedia. 
Construcción de un compilador de PL/0 para Win32: Pág. 2 
 
 
 
UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL                            INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    
    
 
Carrera: INFORMÁTICA APLICADA     Materia: SISTEMAS DE COMPUTACIÓN I     Docente: M. ING. DIEGO CORSI 
 
 
 
 Construcción de un compilador de PL/0 para Win32: Pág. 3 
pppp Intérprete: No genera código objeto, sino que analiza y ejecuta 
directamente cada sentencia del código fuente. Como no se genera código 
de máquina, en cierta forma los programas escritos para ser interpretados 
son independientes de la máquina. 
pppp Preprocesador: Reemplaza macros, incluye archivos o extiende el 
lenguaje. 
    En este curso sólo haremos hincapié en los compiladores, y 
estudiaremos las distintas fases del proceso de compilación, pero no nos 
detendremos en cuestiones marginales como los sistemas de edición y 
depuración que forman parte de todo ambiente de compilación moderno. 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
    El análisis léxico es llevado a cabo por el analizador léxico 
(lexical scanner o simplemente scanner) y consiste en reconocer los 
componentes léxicos (símbolos del lenguaje) contenidos en el código 
fuente del programa a compilar, que ingresa como un flujo de caracteres. 
    El análisis sintáctico tiene como objetivo revisar si los símbolos 
detectados durante el análisis léxico aparecen en el orden correcto como 
para constituir un programa válido. El analizador sintáctico se conoce 
usualmente como parser. 
    El análisis semántico reconoce si las unidades gramaticales tienen 
sentido, detectando errores como inconsistencia de tipos u operaciones 
con objetos no declarados. 
Código Fuente 
Análisis Léxico 
Análisis Sintáctico 
Análisis Semántico 
Código Intermedio 
Generación de Código 
Código Objeto 
C 
O 
M 
P 
I 
L 
A 
D 
O 
R 
 
 
 
UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL                            INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    
    
 
Carrera: INFORMÁTICA APLICADA     Materia: SISTEMAS DE COMPUTACIÓN I     Docente: M. ING. DIEGO CORSI 
 
 
 
 Construcción de un compilador de PL/0 para Win32: Pág. 4 
    Finalmente, la generación de código es llevada a cabo por un módulo 
que usualmente es reemplazable, con lo cual se pueden obtener códigos 
objeto para distintas plataformas a partir de un mismo código intermedio. 
Además, hoy en día es común que compiladores de distintos lenguajes 
generen el mismo código intermedio, por lo que un programa ejecutable 
puede obtenerse enlazando módulos de código objeto obtenidos a partir de 
códigos fuente escritos en lenguajes distintos. 
 
2. Estructura de los lenguajes 
 
    Los lenguajes se basan en un vocabulario. Sus elementos son 
comúnmente llamados palabras, pero en el estudio de los lenguajes 
formales se los denomina símbolos. 
    Es característico de los lenguajes que algunas secuencias de palabras 
sean reconocidas como frases correctas y otras no. Lo que determina si 
una secuencia de palabras es una frase correcta (o no) es la gramática, 
sintaxis o estructura del lenguaje. De hecho, definimos sintaxis como el 
conjunto de reglas que definen el conjunto de frases formalmente 
correctas. 
    Dado que la sintaxis provee a las frases de una estructura que nos 
sirve para reconocerles el significado, queda claro que la sintaxis y la 
semántica (el significado) están íntimamente conectados. Sin embargo, en 
un primer momento vamos a dedicarnos exclusivamente al estudio de la 
sintaxis. 
    Tomemos la frase "Martín duerme." La palabra "Martín" es el sujeto y 
la palabra "duerme" es el predicado. Esta frase pertenece a un lenguaje 
que puede, por ejemplo, estar definido por la siguiente sintaxis: 
    <frase> ::= <sujeto> <predicado> 
    <sujeto> ::= Martín | Julieta 
    <predicado> ::= duerme | juega 
El significado de estas tres líneas es el siguiente: 
    1. Una frase está formada por un sujeto seguido de un predicado. 
    2. El sujeto puede ser la palabra "Martín" o la palabra "Julieta" 
    3. El predicado puede ser la palabra "duerme" o la palabra "juega" 
Las frases correctas pueden derivarse a partir del símbolo inicial 
<frase> mediante la aplicación reiterada de reglas de sustitución. 
 
 
 
UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL                            INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    
    
 
Carrera: INFORMÁTICA APLICADA     Materia: SISTEMAS DE COMPUTACIÓN I     Docente: M. ING. DIEGO CORSI 
 
 
 
 Construcción de un compilador de PL/0 para Win32: Pág. 5 
    La notación utilizada para escribir estas reglas se denomina BNF. 
    Las palabras Martín, Julieta, duerme y juega se denominan símbolos 
terminales. Una secuencia nula de símbolos se representa con ∈∈∈∈. 
    <frase>, <sujeto> y <predicado> son los símbolos no terminales. 
Las reglas se denominan producciones, ya que determinan cómo se pueden 
generar o producir frases correctas. 
    Los símbolos  ::=  y  |  se denominan metasímbolos de la notación 
BNF, y se pronuncian "puede sustituirse por" y "o", respectivamente. 
Aunque no forman parte de BNF (ya que son una extensión del mismo), 
también son consideradas metasímbolos las llaves {  y  } para encerrar 
símbolos que se repiten cero o más veces. 
    A veces, es posible simplificar la notación, utilizando letras 
minúsculas para los símbolos terminales y letras mayúsculas para los 
símbolos no terminales, en lugar de distinguirlos con < y >. 
 
Ejemplo 1 
             S ::= AB 
             A ::= x|y 
             B ::= z|w 
 
    El lenguaje definido por esta sintaxis (que es equivalente a la 
sintaxis dada en la página anterior) es el compuesto por las cuatro 
frases xz, yz, xw, yw. 
    A diferencia del lenguaje anterior, el definido por la siguiente 
sintaxis está compuesto por un número infinito de frases: 
 
Ejemplo 2 
             S ::= xA 
             A ::= z|yA 
 
    A partir del símbolo inicial S pueden generarse las frases xz, xyz, 
xyyz, xyyyz, xyyyyz, .... 
    En síntesis, un lenguaje L se caracterizará con referencia a una 
gramática G (T, N, P, S), donde: 
    pppp T es el conjunto de símbolos terminales 
    pppp N es el conjunto de símbolos no terminales 
    pppp P es el conjunto de producciones 
    pppp S es el símbolo inicial (debe ser un símbolo no terminal) 
UNIVERSIDAD TECNOLÓGICA NACIONAL                            
UNIVERSIDAD TECNOLÓGICA NACIONAL    
INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    
INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO
INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO
INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO
Carrera
: INFORMÁTICA APLICADA     Materia
: SISTEMAS DE COMPUTACIÓN I     Docente
: M. ING. DIEGO CORSI 
3. El lenguaje de programación PL/0
Construcción de un compilador de PL/0 para Win32: Pág. 6 
 
 
 
UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL                            INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    
    
 
Carrera: INFORMÁTICA APLICADA     Materia: SISTEMAS DE COMPUTACIÓN I     Docente: M. ING. DIEGO CORSI 
 
 
 
 Construcción de un compilador de PL/0 para Win32: Pág. 7 
Ejemplo de programa escrito en PL/0 
 
const M = 7, N = 85; 
var X, Y, Z, Q, R; 
 
procedure MULTIPLICAR; 
var A, B; 
begin 
  A := X; 
  B := Y; 
  Z := 0; 
  while B > 0 do 
    begin 
      if odd B then Z := Z + A; 
      A := A * 2; 
      B := B / 2 
    end 
end; 
 
procedure DIVIDIR; 
var W; 
begin 
  R := X; 
  Q := 0; 
  W := Y; 
  while W <= R do W := W * 2; 
  while W > Y do 
    begin 
      Q := Q * 2; 
      W := W / 2; 
      if W <= R then 
        begin 
          R := R - W; 
          Q := Q + 1 
        end 
    end 
end; 
 
procedure MCD; 
var F, G; 
begin 
  F := X; 
  G := Y; 
  while F <> G do 
    begin 
      if F < G then G := G - F; 
      if G < F then F := F - G 
    end; 
  Z := F 
end; 
 
begin 
  X := M;  Y := N;  call MULTIPLICAR; 
  X := 25; Y := 3;  call DIVIDIR; 
  X := 84; Y := 36; call MCD 
end. 
 
 
 
UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL                            INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    
    
 
Carrera: INFORMÁTICA APLICADA     Materia: SISTEMAS DE COMPUTACIÓN I     Docente: M. ING. DIEGO CORSI 
 
 
 
 Construcción de un compilador de PL/0 para Win32: Pág. 8 
    Para describir el lenguaje PL/0 se utilizó una alternativa a la 
notación BNF: los grafos de sintaxis. Ambas notaciones son equivalentes, 
aunque los grafos dan una imagen más clara de la estructura del lenguaje 
cuya sintaxis describen. 
    Hay un grafo de sintaxis por cada producción. 
    Los símbolos no terminales son representados mediante nombres 
encerrados en rectángulos, con excepción del símbolo inicial, que 
solamente aparece al comienzo de la primera producción. 
    Los símbolos terminales son representados mediante nombres encerrados 
en círculos o rectángulos con bordes redondeados, y pueden ser de dos 
tipos: si el nombre está en mayúsculas representa una palabra reservada 
del lenguaje, pero si está en minúsculas se trata del nombre de un grupo 
de símbolos terminales, y no de un símbolo propiamente dicho, ya que 
hacer una enumeración completa sería poco práctico (cuando no imposible). 
 
    A pesar de su pequeño tamaño, PL/0 es relativamente completo. La 
asignación es su proposición básica. Los conceptos fundamentales de la 
programación estructurada (secuencia, condición y repetición) están 
representados por las proposiciones begin/end, if y while. PL/0 incorpora 
el concepto de subrutina mediante declaraciones de procedimientos y 
proposiciones de llamadas a los mismos. Esto ofrece la oportunidad de 
presentar el concepto de localidad de las constantes, las variables y los 
procedimientos. 
    Para reducir su complejidad, PL/0 sólo ofrece un tipo de datos: los 
enteros (en este curso: enteros de 32 bits con signo). Es posible 
declarar variables y constantes de este tipo. PL/0 dispone de los 
operadores aritméticos y relacionales convencionales. 
 
4. Análisis léxico 
 
    La tarea del analizador léxico consiste en: 
 a. Saltear los separadores (blancos, tabulaciones, comentarios). 
 b. Reconocer los símbolos válidos e informar sobre los no válidos. 
 c. Llevar la cuenta de los renglones del programa. 
 d. Copiar los caracteres de entrada a la salida, generando un listado 
    con los renglones numerados. 
 
 
 
UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL                            INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    
    
 
Carrera: INFORMÁTICA APLICADA     Materia: SISTEMAS DE COMPUTACIÓN I     Docente: M. ING. DIEGO CORSI 
 
 
 
 Construcción de un compilador de PL/0 para Win32: Pág. 9 
    El analizador léxico más simple es el de los lenguajes cuyos símbolos 
están compuestos por un único carácter. Esto no es lo más frecuente en el 
caso de los lenguajes de programación, donde las palabras reservadas, los 
identificadores, los números y los operadores pueden estar compuestos por 
más de un carácter. Para describir estos elementos, lo más conveniente es 
utilizar gramáticas regulares, o sea, gramáticas G (T, N, P, S) en las 
que cada producción P tiene la forma A ::= aB  o  A ::= a, y donde A y B 
pertenecen a N y a pertenece a T. 
    Es posible diseñar un autómata finito F para cada gramática regular 
G. Este autómata F acepta las frases del lenguaje definido por G, es 
decir, L (G) = L (F). 
 
    A continuación, se presenta un diagrama de transición rudimentario 
del autómata que reconoce los símbolos del lenguaje PL/0. 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL                            INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    
    
 
Carrera: INFORMÁTICA APLICADA     Materia: SISTEMAS DE COMPUTACIÓN I     Docente: M. ING. DIEGO CORSI 
 
 
 
 Construcción de un compilador de PL/0 para Win32: Pág. 10
    El analizador léxico que se desarrollará para este curso deberá ser 
un procedimiento que tenga una forma similar a la siguiente (la forma, en 
definitiva, dependerá del lenguaje utilizado para implementarlo): 
 
type terminal = (nulo, _begin, _call, _const, .... , coma, pto, ....); 
     archivo = file of char; 
     str63 = string [63]; 
 
procedure scanner (var Fuente, Listado: archivo; var S: terminal; var 
Cad: str63; var Restante: string; var NumLinea: integer); 
 
    Cada vez que se llame al procedimiento scanner y la cadena Restante 
esté vacía o sólo contenga separadores, se leerá en Restante un nuevo 
renglón del archivo Fuente y se lo escribirá en el archivo Listado, 
anteponiéndole el valor actualizado de la variable NumLinea. Si, en 
cambio, la variable Restante contuviera caracteres útiles para formar 
símbolos, se los utilizará (borrándolos de Restante) para formar el 
próximo símbolo terminal S y la cadena de caracteres Cad correspondiente. 
    Por ejemplo, si el archivo Fuente contiene en sus dos primeros 
renglones: 
                 CONST A=2; 
                 procedure RAIZ; 
 
Estos serán los valores luego de cada llamada: 
Llamada Listado S Cad Restante NumLinea 
1 1: CONST A=2; _const 'CONST' ' A=2;' 1 
2 1: CONST A=2; identificador 'A' '=2;' 1 
3 1: CONST A=2; igual '=' '2;' 1 
4 1: CONST A=2; numero '2' ';' 1 
5 1: CONST A=2; ptoycoma ';' '' 1 
6 1: CONST A=2; 
2: procedure RAIZ; 
_procedure 'PROCEDURE' ' RAIZ;' 2 
7 1: CONST A=2; 
2: procedure RAIZ; 
identificador 'RAIZ' ';' 2 
8 1: CONST A=2; 
2: procedure RAIZ; 
ptoycoma ';' '' 2 
 
 
 
 
 
UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL                            INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    
    
 
Carrera: INFORMÁTICA APLICADA     Materia: SISTEMAS DE COMPUTACIÓN I     Docente: M. ING. DIEGO CORSI 
 
 
 
 Construcción de un compilador de PL/0 para Win32: Pág. 11
5. Análisis sintáctico 
 
    El proceso de determinar si una frase puede ser generada a partir de 
un conjunto de producciones se denomina parsing. 
    En el ejemplo 2, la frase xyyz se obtiene aplicando una vez S y tres 
veces A (las dos primeras veces que se aplica A se elige la opción de la 
derecha y la última vez la opción de la izquierda). Cuál producción se 
aplica surge inmediatamente al leer la frase de a un símbolo, de 
izquierda a derecha. 
    Veamos ahora el siguiente caso: 
 
Ejemplo 3 
             S ::= A|B 
             A ::= xA|y 
             B ::= xB|z 
 
    Determinar las producciones aplicadas para generar xxxxxxz sólo es 
posible una vez que se leyó la frase completa, ya que habiendo leído sólo 
la primera x no es posible saber si al aplicar S corresponde elegir A o 
B. 
 
    Otro caso problemático se muestra a continuación: 
 
Ejemplo 4 
             S ::= Ax 
             A ::= x|∈∈∈∈ 
 
    Para generar la frase x, sólo es posible saber si corresponde aplicar 
la parte izquierda o la parte derecha de A una vez que A ya ha sido 
aplicada (bien o mal) y aparece la x final de S. 
    Las gramáticas que no presentan tales dificultades se denominan 
LL(1). La primera "L" significa que la entrada será leída de izquierda a 
derecha, y la segunda "L" indica derivaciones por la izquierda. El número 
"1" significa que alcanza con leer por anticipado un símbolo en cualquier 
paso del proceso de análisis sintáctico (o sea, solamente se emplea un 
símbolo de preanálisis). Al sistema de grafos con que se representa una 
gramática de este tipo se lo conoce como grafo de sintaxis 
determinístico. 
 
 
 
UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL                            INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    
    
 
Carrera: INFORMÁTICA APLICADA     Materia: SISTEMAS DE COMPUTACIÓN I     Docente: M. ING. DIEGO CORSI 
 
 
 
 Construcción de un compilador de PL/0 para Win32: Pág. 12
    El paso siguiente consiste en construir un reconocedor sintáctico 
(parser) para una sintaxis dada. Este tipo de programa se deriva 
directamente del grafo de sintaxis determinístico y requiere de un 
procedimiento que funcione como scanner, salvo que los símbolos del 
lenguaje consten de un único carácter, en cuyo caso cualquier 
procedimiento de entrada estándar servirá para el ingreso del siguiente 
símbolo. 
    Para escribir un reconocedor sintáctico a partir de un grafo de 
sintaxis determinístico deberán seguirse las siguientes reglas: 
 
R1. Reducir el sistema de grafos a la menor cantidad de grafos que sea 
    posible, realizando para ello las sustituciones que sean 
    necesarias. 
 
R2. Declarar para cada grafo un procedimiento que contenga las 
    sentencias resultantes de aplicarle al grafo las reglas R3 a R7. 
 
R3. Una secuencia de elementos 
 
                         ... 
 
    se traduce como una sentencia compuesta: 
 
    begin 
      T(S1); T(S2); ... T(Sn) 
    end 
 
    (donde T(Si) es la sentencia obtenida al traducir el grafo Si) 
 
R4. Una opción entre elementos 
 
 
 
 
            .... 
 
 
S1 S2 Sn 
S1 
S2 
Sn 
 
 
 
UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL                            INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    
    
 
Carrera: INFORMÁTICA APLICADA     Materia: SISTEMAS DE COMPUTACIÓN I     Docente: M. ING. DIEGO CORSI 
 
 
 
 Construcción de un compilador de PL/0 para Win32: Pág. 13
    se traduce como una sentencia condicional: 
 
    if SIM in L1 then T(S1) else 
    if SIM in L2 then T(S2) else 
    .... 
    if SIM in Ln then T(Sn); 
 
    donde SIM es el símbolo devuelto por el analizador léxico y Li es 
    el conjunto de símbolos iniciales de Si. Siempre que Li conste de 
    un único símbolo a, "SIM in Li" podrá expresarse como "SIM = a" 
 
R5. Un bucle de la forma 
 
 
 
 
    se traduce como la sentencia: 
 
    while SIM in L do T(S) 
 
R6. Una referencia a otro grafo A 
 
 
 
    se traduce como una sentencia de llamada al procedimiento A 
 
R7. Una referencia a un símbolo terminal x 
 
 
 
    se traduce como la sentencia: 
 
    if SIM = x then SCANNER(SIM) else ERROR 
 
    donde ERROR es un procedimiento encargado del tratamiento de los 
    errores. 
 
    El parser funciona haciendo una llamada al scanner (para tener un 
símbolo leído de antemano) y una llamada al procedimiento correspondiente 
S 
A 
x 
 
 
 
UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL                            INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    
    
 
Carrera: INFORMÁTICA APLICADA     Materia: SISTEMAS DE COMPUTACIÓN I     Docente: M. ING. DIEGO CORSI 
 
 
 
 Construcción de un compilador de PL/0 para Win32: Pág. 14
al primero de los grafos. A partir de este procedimiento se irán 
realizando llamadas a los demás, hasta que aparezca algún error o se 
termine reconociendo satisfactoriamente el programa leído. 
 
    Algunas construcciones redundantes pueden suprimirse al depurar el 
parser resultante de la estricta aplicación de las reglas R1 a R7. 
 
    Veamos ahora el siguiente caso: 
 
Ejemplo 5 
             A ::= x|(B) 
             B ::= AC 
             C ::= {+A} 
 
    Aquí, los símbolos terminales son "x", "(", ")" y "+". Es posible 
utilizar el procedimiento read para llevar a cabo la función del scanner, 
ya que todos los símbolos están formados por un único carácter. Algunas 
de las posibles frases del lenguaje son: 
    x        (x)         (x+x)         ((x)) 
 
    Los grafos equivalentes a la gramática expresada en BNF son: 
 
 
    Aplicando la regla R1: 
 
 
 
 
UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL                            INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    
    
 
Carrera: INFORMÁTICA APLICADA     Materia: SISTEMAS DE COMPUTACIÓN I     Docente: M. ING. DIEGO CORSI 
 
 
 
 Construcción de un compilador de PL/0 para Win32: Pág. 15
 
    Aplicando las reglas R2 a R7: 
 
program PARSER; 
var SIM: char; 
procedure A; 
begin 
  if SIM = 'x' 
    then read (SIM) 
    else if SIM = '(' 
           then begin 
                  read (SIM); 
                  A; 
                  while SIM = '+' do begin 
                                       read (SIM); 
                                       A 
                                     end; 
                  if SIM = ')' then read (SIM) 
                               else ERROR 
                end 
           else ERROR 
end; 
begin 
  read (SIM); 
  A 
end. 
 
 
6. Análisis semántico 
 
    El análisis sintáctico no garantiza que un programa esté libre de 
errores. El siguiente programa escrito en PL/0 es sintácticamente 
correcto, pero contiene un error semántico, ya que un identificador de 
constante no puede ser llamado mediante la proposición call (que es 
exclusiva para identificadores de procedimiento). 
 
Ejemplo 6 
             const K = 9; 
             var V; 
             procedure P; 
               var X; 
               begin 
                 X := K * 2; 
                 V := X 
               end; 
             call K. 
 
 
    Para poder determinar si un programa es semánticamente correcto, el 
compilador deberá cargar en una tabla cada identificador que se declare. 
En esa tabla podrán consultarse: 
 
 
 
UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL                            INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    
    
 
Carrera: INFORMÁTICA APLICADA     Materia: SISTEMAS DE COMPUTACIÓN I     Docente: M. ING. DIEGO CORSI 
 
 
 
 Construcción de un compilador de PL/0 para Win32: Pág. 16
         - nombre del identificador 
         - tipo de identificador 
         - valor del identificador: Un número de 32 bits con distinto 
           significado, según el tipo de identificador de que se 
           trate: pppp constante: el valor de la constante 
                  pppp variable: la dirección de memoria a que se refiere 
                    la variable (sólo el desplazamiento) 
                  pppp procedimiento: la dirección de memoria donde 
                    comienza la proposición a ejecutar en el bloque 
 
    Una posible definición del tipo con que se declarará la tabla podría 
ser la siguiente: 
 
     type TABLA = array [0..MaxIdent-1] of record 
                                             NOM: str63; 
                                             TIPO: terminal; 
                                             VALOR: longInt 
                                           end; 
    El procedimiento BLOQUE recibirá como parámetro (pasaje por valor) la 
entrada de la tabla a partir de la cual se podrán cargar identificadores. 
Este parámetro podría llamarse BASE. Cuando se llama a BLOQUE desde el 
grafo PROGRAMA, se le pasa como parámetro el valor 0, ya que no hay 
identificadores previamente declarados. 
    El procedimiento BLOQUE contendrá un variable local llamada 
DESPLAZAMIENTO, que se irá incrementando con cada identificador que se 
declare. Cuando se llama a BLOQUE desde el grafo BLOQUE, se le pasa como 
parámetro el valor BASE+DESPLAZAMIENTO. 
    De esta forma, cada vez que se declare un identificador, deberá 
verificarse si éste ya había sido declarado en el mismo ámbito, es decir, 
entre las posiciones de la tabla delimitadas por BASE y 
BASE+DESPLAZAMIENTO-1. 
    Para el análisis semántico, los identificadores se buscarán en toda 
la tabla, comenzando en la posición BASE+DESPLAZAMIENTO-1 y retrocediendo 
hasta la posición 0. De esta forma, siempre se encontrará primero el 
identificador que haya sido declarado localmente. En caso de no 
encontrarse el identificador, deberá darse aviso de la falta de 
declaración. 
UNIVERSIDAD TECNOLÓGICA NACIONAL                            
UNIVERSIDAD TECNOLÓGICA NACIONAL    
INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    
INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO
INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO
INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO
Carrera
: INFORMÁTICA APLICADA     Materia
: SISTEMAS DE COMPUTACIÓN I     Docente
: M. ING. DIEGO CORSI 
Una vez hallado el identificador en la tabla, con el campo TIPO podrá 
verificarse si el identificador es semánticamente correcto en la posición 
del programa donde fue encontrado. 
7. Generación de código
Antes de comenzar con el estudio de la generación de código, 
extenderemos la sintaxis de PL/0 mediante el agregado de tres 
proposiciones de E/S, ya que de lo contrario no podríamos ingresar 
valores en tiempo de ejecución ni podríamos ver los resultados de los 
cálculos realizados por el programa. Adoptaremos los nombres de los 
procedimientos de E/S de Pascal (readln, write y writeln), y les daremos 
una funcionalidad similar a la que tienen en este lenguaje cuando se los 
utiliza para realizar entrada desde el teclado y salida hacia la 
pantalla. Además, incorporaremos un símbolo terminal nuevo, la cadena 
literal, para permitir mostrar mensajes por pantalla. El analizador 
léxico considerará que cualquier secuencia de caracteres encerrada entre 
apóstrofos es una cadena. 
La proposición de entrada READLN tendrá la sintaxis: 
Las proposiciones de salida WRITELN y WRITE tendrán la sintaxis: 
Construcción de un compilador de PL/0 para Win32: Pág. 17
 
 
 
UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL    UNIVERSIDAD TECNOLÓGICA NACIONAL                            INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    INSTITUTO NACIONAL SUPERIOR DEL PROFESORADO TÉCNICO    
    
 
Carrera: INFORMÁTICA APLICADA     Materia: SISTEMAS DE COMPUTACIÓN I     Docente: M. ING. DIEGO CORSI 
 
 
 
 Construcción de un compilador de PL/0 para Win32: Pág. 18
    Finalmente, llegamos a la generación de código. Como plataforma de 
destino de la compilación, en este curso se adoptará una PC con un 
microprocesador que implemente IA-32 (Intel Architecture, 32-bit) y un 
sistema operativo compatible con la API Win32. 
 
    Utilizaremos (explícitamente) sólo 4 de los registros de 32 bits 
disponibles: EDI, EAX, EBX y EDX. Eventualmente, también accederemos a la 
parte baja de EAX, a través del subregistro AL (el cual permite acceder a 
los 8 bits menos significativos de EAX). 
 
    De los modos de direccionamiento soportados por el microprocesador, 
solamente vamos a usar los siguientes tres: 
 EJEMPLOS 
modo registro ADD EAX, EBX             (carga EAX con el valor de la 
suma de EAX más EBX) 
modo inmediato MOV EAX, 00000072        (carga EAX con el valor 
00000072) 
modo indexado MOV EAX, [EDI+00000072]  (carga EAX con el contenido 
de la dirección EDI+00000072) 
 
    El archivo ejecutable generado por el compilador será de tipo PE 
(Portable Executable). Este tipo es compatible con las versiones de 
Windows actuales, ya que los ejecutables de tipo DOS COM (archivos COM), 
DOS MZ (archivos EXE) y NE (New Executable) corresponden a aplicaciones 
de 16 bits que no corren en las versiones de Windows de 64 bits. 
 
    El código del programa estará compuesto por una parte de longitud 
fija y una parte de longitud variable. 
 
    La parte de longitud fija contendrá: 
• un encabezado compatible con MS-DOS, caracterizado por el número 
mágico 4D 5A (MZ), con instrucciones para avisar que la aplicación 
no es compatible con ese sistema operativo; 
• los encabezados COFF (Common Object File Format), con su número 
mágico 50 45 00 00 (PE··) y OH (Optional Header), formados por 
numerosos campos usados para cargar y ejecutar la aplicación; 
• una tabla de encabezados de secciones con el encabezado de text. 
• el comienzo de la sección text, formado por instrucciones del x86 
mediante las que se implementan las proposiciones de E/S. Estas 
instrucciones constituyen rutinas desde las cuales se realizarán 
las llamadas a las funciones de la API de Windows. */