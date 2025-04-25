/*
Programa para identificar y aplicar una transformación bit a bit a una imagen BMP RGB de 24 bits.
Descripción:
Este programa carga una imagen BMP (a resolver) y un archivo de texto que contiene una semilla (offset)
y un valor esperado después de aplicar una transformación bit a bit a un píxel específico.
El programa prueba las operaciones XOR y rotaciones (izquierda y derecha) en el píxel de la imagen
ubicado en la posición de la semilla. Si el resultado de alguna de estas operaciones coincide con
el valor esperado, se aplica la misma operación a todos los píxeles de la imagen y se guarda
la imagen modificada.
*/
#include <iostream>
#include <fstream>
#include <cstring>
#include <QImage>
#include <QString>

using namespace std;

// Función para cargar los píxeles de una imagen BMP RGB de 24 bits.
// Devuelve un puntero al arreglo de píxeles o nullptr en caso de error.
unsigned char* cargarPixelsBMP(const QString& rutaArchivo, int& ancho, int& alto) {
    QImage imagen(rutaArchivo);
    if (imagen.isNull() || imagen.format() != QImage::Format_RGB888) {
        cerr << "Error: No se pudo cargar la imagen BMP" << endl;
        return nullptr;
    }
    ancho = imagen.width();
    alto = imagen.height();
    int tamano = ancho * alto * 3; // 3 bytes por píxel (R, G, B)
    unsigned char* datos = new unsigned char[tamano];
    memcpy(datos, imagen.constBits(), tamano);
    return datos;
}

// Función para guardar un arreglo de píxeles como una imagen BMP RGB de 24 bits.
// Devuelve true si la imagen se guardó correctamente, false en caso de error.
bool guardarImagenBMP(const QString& rutaArchivo, const unsigned char* datos, int ancho, int alto) {
    QImage imagen(datos, ancho, alto, QImage::Format_RGB888);
    if (imagen.save(rutaArchivo, "BMP")) {
        cout << "Imagen modificada guardada en: " << rutaArchivo.toStdString() << endl;
        return true;
    } else {
        cerr << "Error: No se pudo guardar la imagen BMP." << endl;
        return false;
    }
}

// Función para cargar la semilla y el resultado esperado desde un archivo de texto.
// Devuelve true si la carga fue exitosa, false en caso de error.
bool cargarSemillaResultado(const QString& rutaArchivo, int& semilla, unsigned char& resultadoEsperado) {
    ifstream archivo(rutaArchivo.toStdString());
    if (archivo.is_open()) {
        if (archivo >> semilla >> hex >> reinterpret_cast<unsigned int&>(resultadoEsperado)) {
            archivo.close();
            return true;
        } else {
            cerr << "Error: Formato incorrecto en el archivo de texto (semilla o resultado esperado)." << endl;
        }
        archivo.close();
    } else {
        cerr << "Error: No se pudo abrir el archivo de texto." << endl;
    }
    return false;
}

// Función para aplicar la operación XOR bit a bit a un byte.
unsigned char aplicarXOR(unsigned char original, unsigned char valor) {
    return original ^ valor;
}

// Función para rotar un byte a la izquierda una cierta cantidad de bits.
unsigned char rotarIzquierda(unsigned char byte, int cantidad) {
    cantidad %= 8; // Asegura que la cantidad esté en el rango [0, 7]
    return (byte << cantidad) | (byte >> (8 - cantidad));
}

// Función para rotar un byte a la derecha una cierta cantidad de bits.
unsigned char rotarDerecha(unsigned char byte, int cantidad) {
    cantidad %= 8; // Asegura que la cantidad esté en el rango [0, 7]
    return (byte >> cantidad) | (byte << (8 - cantidad));
}

int main() {
    QString archivoImagen = "data/Caso1/I_M.bmp";  //imagen a resolver
    QString archivoResultadoGuardado = "data/imagen_modificada.bmp";
    QString archivoDatos = "data/Caso1/M1.txt";

    // Variables para almacenar las dimensiones y los píxeles de la imagen
    int anchoImagen = 0;
    int altoImagen = 0;
    unsigned char* pixelesImagen = cargarPixelsBMP(archivoImagen, anchoImagen, altoImagen);

    // Verificar si la carga de la imagen fue exitosa
    if (!pixelesImagen) {
        return 1; // Terminar el programa con código de error
    }

    // Variables para la semilla y el resultado esperado
    int semilla = 0;
    unsigned char resultadoEsperado = 0;

    // Cargar la semilla y el resultado esperado desde el archivo de texto
    if (!cargarSemillaResultado(archivoDatos, semilla, resultadoEsperado)) {
        delete[] pixelesImagen;
        return 1; // Terminar el programa con código de error
    }

    // Calcular el índice del byte correspondiente a la semilla
    if (semilla < 0 || semilla >= anchoImagen * altoImagen) {
        cerr << "Error: La semilla está fuera del rango de la imagen." << endl;
        delete[] pixelesImagen;
        return 1;
    }
    int indiceByte = semilla * 3; // Cada píxel tiene 3 bytes (R, G, B)

    // Obtener el valor del primer canal (Rojo) del píxel en la posición de la semilla
    unsigned char pixelOriginal = pixelesImagen[indiceByte];

    // Prueba de la operación XOR
    unsigned char resultadoXOR = aplicarXOR(pixelOriginal, 0xFF); // Prueba XOR con un valor constante (puedes ajustarlo)
    if (resultadoXOR == resultadoEsperado) {
        cout << "Aplicando operación XOR a la imagen." << endl;
        for (int i = 0; i < anchoImagen * altoImagen * 3; ++i) {
            pixelesImagen[i] = aplicarXOR(pixelesImagen[i], 0xFF); // Aplicar XOR a todos los canales
        }
        guardarImagenBMP(archivoResultadoGuardado, pixelesImagen, anchoImagen, altoImagen);
        delete[] pixelesImagen;
        return 0;
    }

    // Prueba de las rotaciones
    for (int cantidadRotacion = 1; cantidadRotacion < 8; ++cantidadRotacion) {
        // Prueba de rotación izquierda
        unsigned char resultadoRotacionIzquierda = rotarIzquierda(pixelOriginal, cantidadRotacion);
        if (resultadoRotacionIzquierda == resultadoEsperado) {
            cout << "Aplicando rotación izquierda de " << cantidadRotacion << " bits a la imagen." << endl;
            for (int i = 0; i < anchoImagen * altoImagen * 3; ++i) {
                pixelesImagen[i] = rotarIzquierda(pixelesImagen[i], cantidadRotacion);
            }
            guardarImagenBMP(archivoResultadoGuardado, pixelesImagen, anchoImagen, altoImagen);
            delete[] pixelesImagen;
            return 0;
        }

        // Prueba de rotación derecha
        unsigned char resultadoRotacionDerecha = rotarDerecha(pixelOriginal, cantidadRotacion);
        if (resultadoRotacionDerecha == resultadoEsperado) {
            cout << "Aplicando rotación derecha de " << cantidadRotacion << " bits a la imagen." << endl;
            for (int i = 0; i < anchoImagen * altoImagen * 3; ++i) {
                pixelesImagen[i] = rotarDerecha(pixelesImagen[i], cantidadRotacion);
            }
            guardarImagenBMP(archivoResultadoGuardado, pixelesImagen, anchoImagen, altoImagen);
            delete[] pixelesImagen;
            return 0;
        }
    }

    // Si no se encontró ninguna coincidencia
    cout << "No se encontró ninguna transformación bit a bit que coincida con el resultado esperado." << endl;

    // Liberar la memoria de los píxeles
    delete[] pixelesImagen;

    return 0; // Fin del programa
}

