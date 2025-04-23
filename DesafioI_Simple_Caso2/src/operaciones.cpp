#include "operaciones.h"
#include <cstdlib>  // NECESARIO para usar malloc y free



// Aplica XOR bit a bit entre dos imágenes RGB
void xorImagen(uint8_t*** destino, uint8_t*** fuente, int ancho, int alto) {
    for (int i = 0; i < alto; ++i)
        for (int j = 0; j < ancho; ++j)
            for (int c = 0; c < 3; ++c)
                destino[i][j][c] ^= fuente[i][j][c];
}

// Rota bits a la derecha para cada canal de color
void rotarImagen(uint8_t*** imagen, int ancho, int alto, int bits) {
    for (int i = 0; i < alto; ++i)
        for (int j = 0; j < ancho; ++j)
            for (int c = 0; c < 3; ++c) {
                uint8_t val = imagen[i][j][c];
                imagen[i][j][c] = (val >> bits) | (val << (8 - bits));
            }
}

// Desplaza la imagen como si fuera lineal
void desplazarImagen(uint8_t*** imagen, int ancho, int alto, int s) {
    int total = ancho * alto;
    for (int c = 0; c < 3; ++c) {
        uint8_t* temp = (uint8_t*)malloc(total);
        for (int i = 0; i < alto; ++i)
            for (int j = 0; j < ancho; ++j)
                temp[i * ancho + j] = imagen[i][j][c];
        for (int i = 0; i < alto; ++i)
            for (int j = 0; j < ancho; ++j)
                imagen[i][j][c] = temp[(i * ancho + j + s) % total];
        free(temp);
    }
}
