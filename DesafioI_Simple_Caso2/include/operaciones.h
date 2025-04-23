#pragma once
#include <cstdint>

// Aplica XOR entre dos imágenes RGB
void xorImagen(uint8_t*** destino, uint8_t*** fuente, int ancho, int alto);

// Rota los bits a la derecha en cada canal RGB
void rotarImagen(uint8_t*** imagen, int ancho, int alto, int bits);

// Desplaza píxel a píxel como si fuera un arreglo lineal
void desplazarImagen(uint8_t*** imagen, int ancho, int alto, int s);
