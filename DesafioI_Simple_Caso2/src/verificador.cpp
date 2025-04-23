#include "verificador.h"
#include <cstdio>

int leerDesplazamiento(const char* archivo) {
    FILE* f = fopen(archivo, "r");
    if (!f) return -1;
    int s;
    fscanf(f, "%d", &s);
    fclose(f);
    return s;
}
