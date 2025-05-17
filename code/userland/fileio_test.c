#include "../userprog/syscall.h"

unsigned
StringLength(const char *s)
{
    unsigned i;
    for (i = 0; s[i] != '\0'; i++) {}
    return i;
}

int
PrintString(const char *s)
{
    unsigned len = StringLength(s);
    return Write(s, len, CONSOLE_OUTPUT);
}

int
PrintChar(char c)
{
    return Write(&c, 1, CONSOLE_OUTPUT);
}

#define TEST_FILENAME "test1.txt"
#define TEST_DATA "Este es un mensaje de prueba para Nachos."
#define TEST_DATA_SIZE (sizeof(TEST_DATA) / sizeof(TEST_DATA)[0])
#define READ_BUFFER_SIZE TEST_DATA_SIZE

int main() {
    char readBuffer[READ_BUFFER_SIZE];
    int fileId;
    int bytesAccessed;

    PrintString("--- Iniciando pruebas de syscalls de archivos ---\n");

    // --- Test 1: Crear un archivo ---
    PrintString("1. Test Create('");
    PrintString(TEST_FILENAME);
    PrintString("')...\n");

    if (Create(TEST_FILENAME) == 1) {
        PrintString("   Create: OK.\n");
    } else {
        PrintString("   Create: FALLIDO.\n");
        PrintString("--- Pruebas terminadas debido a fallo ---");
        Exit(1); // Asumo que Exit(status) está implementado
    }

    // --- Test 2: Abrir el archivo creado ---
    PrintString("2. Test Open('");
    PrintString(TEST_FILENAME);
    PrintString("')...\n");

    fileId = Open(TEST_FILENAME);
    if (fileId != -1) {
        PrintString("   Open: OK.\n");
    } else {
        PrintString("   Open: FALLIDO.\n");
        PrintString("--- Pruebas terminadas debido a fallo ---");
        Remove(TEST_FILENAME);
        Exit(1);
    }

    // --- Test 3: Escribir en el archivo ---
    PrintString("3. Test Write(...)\n");

    bytesAccessed = Write(TEST_DATA, TEST_DATA_SIZE, fileId);
    if (bytesAccessed == TEST_DATA_SIZE) {
        PrintString("   Write: OK.");
    } else {
        PrintString("   Write: FALLIDO o incompleto. ");
        PrintString("--- Pruebas terminadas debido a fallo ---");
        Remove(TEST_FILENAME);
        Exit(1);
    }

    // --- Test 4: Cerrar el archivo ---
    PrintString("4. Test Close(fileId)...\n");

    if (Close(fileId) == 1) {
        PrintString("   Close: OK.\n");
    } else {
        PrintString("   Close: FALLIDO.\n");
        PrintString("--- Pruebas terminadas debido a fallo ---");
        Exit(1);
    }

    // --- Test 5: Abrir el archivo de nuevo para leer ---
    PrintString("5. Test Open('");
    PrintString(TEST_FILENAME);
    PrintString("') de nuevo para leer...\n");

    fileId = Open(TEST_FILENAME);
    if (fileId != -1) {
        PrintString("   Open para leer: OK.\n");
    } else {
        PrintString("   Open para leer: FALLIDO.\n");
        PrintString("--- Pruebas terminadas debido a fallo ---");
        Exit(1);
    }

    // --- Test 6: Leer del archivo ---
    PrintString("6. Test Read(buffer, ");
    PrintString("bytes, fileId)...\n");

    for(int i=0; i<READ_BUFFER_SIZE; i++) {
        readBuffer[i] = 0;
    }

    bytesAccessed = Read(readBuffer, TEST_DATA_SIZE, fileId);

    if (bytesAccessed > 0) {
        readBuffer[bytesAccessed] = '\0';

        PrintString("   Read: OK. ");
        PrintString("Contenido: \"");
        PrintString(readBuffer);
        PrintString("\"\n");

        int match = 1;
        if (bytesAccessed != TEST_DATA_SIZE) {
            match = 0;
        } else {
            for (int i = 0; i < TEST_DATA_SIZE; i++) {
                if (readBuffer[i] != TEST_DATA[i]) {
                    match = 0;
                    break;
                }
            }
        }

        if (match == 1) {
            PrintString("   Contenido leido COINCIDE con el original.\n");
        } else {
            PrintString("   Contenido leido NO COINCIDE con el original.\n");
        }

    } else if (bytesAccessed == 0) {
        PrintString("   Read: Leidos 0 bytes (puede ser el final del archivo).\n");
        PrintString("--- Pruebas de lectura FALLIDA (se esperaban datos) ---\n");
    }
     else { // bytesAccessed == -1
        PrintString("   Read: FALLIDO (retorno -1).\n");
         PrintString("--- Pruebas terminadas debido a fallo ---");
    }

    // --- Test 7: Cerrar el archivo despues de leer ---
    PrintString("7. Test Close(fileId) despues de leer...\n");

    if (Close(fileId) == 1) {
        PrintString("   Close: OK.\n");
    } else {
        PrintString("   Close: FALLIDO.\n");
         PrintString("--- Pruebas terminadas debido a fallo ---");
         Exit(1);
    }

    // --- Test 8: Eliminar el archivo ---
    PrintString("8. Test Remove('");
    PrintString(TEST_FILENAME);
    PrintString("')...\n");

    if (Remove(TEST_FILENAME) == 1) {
        PrintString("   Remove: OK.\n");
    } else {
        PrintString("   Remove: FALLIDO.\n");
    }

    PrintString("--- Pruebas de syscalls de archivos terminadas ---\n");
    PrintString("--- Pruebas extras terminadas ---\n");

    Exit(0);
}