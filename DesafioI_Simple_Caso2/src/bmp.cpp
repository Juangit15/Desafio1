#include "bmp.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#pragma pack(push, 1)
struct BMPHeader {
    uint16_t tipo;
    uint32_t tamano;
    uint16_t reservado1;
    uint16_t reservado2;
    uint32_t offsetDatos;
    uint32_t tamanoEncabezado;
    int32_t ancho;
    int32_t alto;
    uint16_t planos;
    uint16_t bitsPorPixel;
    uint32_t compresion;
    uint32_t tamanoImagen;
    int32_t xPorMetro;
    int32_t yPorMetro;
    uint32_t coloresUsados;
    uint32_t coloresImportantes;
};
#pragma pack(pop)

uint8_t*** leerBMP(const char* archivo, int* ancho, int* alto) {
    FILE* f = fopen(archivo, "rb");
    if (!f) return NULL;

    BMPHeader cabecera;
    fread(&cabecera, sizeof(BMPHeader), 1, f);

    if (cabecera.tipo != 0x4D42 || cabecera.bitsPorPixel != 24) {
        fclose(f);
        return NULL;
    }

    *ancho = cabecera.ancho;
    *alto = cabecera.alto;
    int padding = (4 - (*ancho * 3) % 4) % 4;

    uint8_t*** datos = (uint8_t***)malloc(*alto * sizeof(uint8_t**));
    for (int i = 0; i < *alto; ++i) {
        datos[i] = (uint8_t**)malloc(*ancho * sizeof(uint8_t*));
        for (int j = 0; j < *ancho; ++j) {
            datos[i][j] = (uint8_t*)malloc(3);
        }
    }

    fseek(f, cabecera.offsetDatos, SEEK_SET);
    for (int i = *alto - 1; i >= 0; --i) {
        for (int j = 0; j < *ancho; ++j) {
            fread(datos[i][j], 1, 3, f);
        }
        fseek(f, padding, SEEK_CUR);
    }

    fclose(f);
    return datos;
}

void guardarBMP(const char* archivo, uint8_t*** imagen, int ancho, int alto) {
    FILE* f = fopen(archivo, "wb");
    if (!f) return;

    int padding = (4 - (ancho * 3) % 4) % 4;
    int filaTam = ancho * 3 + padding;
    int tamImagen = filaTam * alto;

    BMPHeader cabecera = {0};
    cabecera.tipo = 0x4D42;
    cabecera.tamano = sizeof(BMPHeader) + tamImagen;
    cabecera.offsetDatos = sizeof(BMPHeader);
    cabecera.tamanoEncabezado = 40;
    cabecera.ancho = ancho;
    cabecera.alto = alto;
    cabecera.planos = 1;
    cabecera.bitsPorPixel = 24;
    cabecera.tamanoImagen = tamImagen;

    fwrite(&cabecera, sizeof(BMPHeader), 1, f);

    uint8_t* relleno = (uint8_t*)calloc(padding, 1);
    for (int i = alto - 1; i >= 0; --i) {
        for (int j = 0; j < ancho; ++j) {
            fwrite(imagen[i][j], 1, 3, f);
        }
        fwrite(relleno, 1, padding, f);
    }
    free(relleno);
    fclose(f);
}

void liberarImagen(uint8_t*** imagen, int alto, int ancho) {
    for (int i = 0; i < alto; ++i) {
        for (int j = 0; j < ancho; ++j) {
            free(imagen[i][j]);
        }
        free(imagen[i]);
    }
    free(imagen);
}
