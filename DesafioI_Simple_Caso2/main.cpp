// main.cpp
#include "bmp.h"
#include "operaciones.h"
#include "verificador.h"
#include <iostream>
#include <cstdio>

using namespace std;

// Este archivo resuelve el Caso 1 y Caso 2 del Desafío I
int main() {
    int ancho, alto;

    // === CASO 1 ===
    uint8_t*** imagenID = leerBMP("C:/Users/juanm/Documents/DesafioI_Simple_Caso2/data/Caso1/I_M.bmp", &ancho, &alto);
    uint8_t*** imagenIM = leerBMP("C:/Users/juanm/Documents/DesafioI_Simple_Caso2/data/Caso1/M.bmp", &ancho, &alto);

    if (!imagenID || !imagenIM) {
        cout << "Error al leer las imagenes del Caso 1." << std::endl;
        return 1;
    }

    xorImagen(imagenID, imagenIM, ancho, alto); // Aplicar XOR canal por canal
    rotarImagen(imagenID, ancho, alto, 3); // Rotar 3 bits a la derecha por canal
    guardarBMP("resultado_caso1.bmp", imagenID, ancho, alto);

    liberarImagen(imagenID, alto, ancho);
    liberarImagen(imagenIM, alto, ancho);

    // === CASO 2 ===
    uint8_t*** imagenCaso2 = leerBMP("C:/Users/juanm/Documents/DesafioI_Simple_Caso2/data/Caso1/I_M.bmp", &ancho, &alto);
    if (!imagenCaso2) {
        cout << "Error al leer la imagen del Caso 2." << std::endl;
        return 1;
    }

    int s = leerDesplazamiento("C:/Users/juanm/Documents/DesafioI_Simple_Caso2/data/Caso1/M1.txt"); // Leer desplazamiento
    if (s == -1) {
        cout << "No se pudo leer el desplazamiento del archivo M1.txt." << std::endl;
        liberarImagen(imagenCaso2, alto, ancho);
        return 1;
    }

    desplazarImagen(imagenCaso2, ancho, alto, s); // Desplazar s píxeles
    guardarBMP("resultado_caso2.bmp", imagenCaso2, ancho, alto);

    liberarImagen(imagenCaso2, alto, ancho);
    cout << "Proceso de ambos casos completado. Revisa los resultados BMP generados." << std::endl;

    return 0;
}






