#pragma once
#include <cstdint>

// Lee una imagen BMP de 24 bits RGB
uint8_t*** leerBMP(const char* archivo, int* ancho, int* alto);

// Guarda una imagen RGB como BMP
void guardarBMP(const char* archivo, uint8_t*** imagen, int ancho, int alto);

// Libera la memoria dinámica usada por la imagen
void liberarImagen(uint8_t*** imagen, int alto, int ancho);
