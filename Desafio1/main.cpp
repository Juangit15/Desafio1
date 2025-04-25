#include <iostream>
#include <fstream>
#include <QImage>
#include <QString>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>

// Función para cargar imagen BMP
unsigned char* cargarImagen(const QString& ruta, int& ancho, int& alto) {
    QImage imagen(ruta);
    if(imagen.isNull()) {
        qCritical() << "Error al cargar imagen:" << ruta;
        return nullptr;
    }

    if(imagen.format() != QImage::Format_RGB888) {
        imagen = imagen.convertToFormat(QImage::Format_RGB888);
    }

    ancho = imagen.width();
    alto = imagen.height();
    unsigned char* datos = new unsigned char[ancho * alto * 3];

    for(int y = 0; y < alto; y++) {
        memcpy(datos + y * ancho * 3, imagen.constScanLine(y), ancho * 3);
    }

    return datos;
}

// Función para guardar imagen BMP
bool guardarImagen(const QString& ruta, unsigned char* datos, int ancho, int alto) {
    QImage imagen(datos, ancho, alto, QImage::Format_RGB888);
    return imagen.save(ruta, "BMP");
}

// Función para cargar archivo de enmascaramiento
bool cargarEnmascaramiento(const QString& ruta, int& desplazamiento, unsigned int*& sumas, int& numSumas) {
    QFile archivo(ruta);
    if(!archivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Error al abrir archivo:" << ruta;
        return false;
    }

    QTextStream in(&archivo);
    desplazamiento = in.readLine().toInt();

    QVector<unsigned int> valores;
    while(!in.atEnd()) {
        QString linea = in.readLine();
        QStringList componentes = linea.split(' ', Qt::SkipEmptyParts);

        if(componentes.size() == 3) {
            valores.append(componentes[0].toUInt());
            valores.append(componentes[1].toUInt());
            valores.append(componentes[2].toUInt());
        }
    }

    numSumas = valores.size();
    sumas = new unsigned int[numSumas];
    for(int i = 0; i < numSumas; i++) {
        sumas[i] = valores[i];
    }

    return true;
}

// Operación XOR entre dos imágenes
void aplicarXOR(unsigned char* img1, const unsigned char* img2, int totalPixeles) {
    for(int i = 0; i < totalPixeles; i++) {
        img1[i] ^= img2[i];
    }
}

// Rotación de bits a la izquierda
void rotarIzquierda(unsigned char* img, int totalPixeles, int bits) {
    bits %= 8;
    for(int i = 0; i < totalPixeles; i++) {
        img[i] = (img[i] << bits) | (img[i] >> (8 - bits));
    }
}

// Revertir el enmascaramiento
void revertirEnmascaramiento(unsigned char* ID, const unsigned char* M,
                             const unsigned int* sumas, int desplazamiento,
                             int pixelesMascara, int anchoID, int anchoM) {
    for(int k = 0; k < pixelesMascara; k++) {
        for(int c = 0; c < 3; c++) {
            int pos = (desplazamiento * 3 + k * 3 + c) % (anchoID * anchoID * 3);
            int mPos = (k * 3 + c) % (anchoM * anchoM * 3);
            ID[pos] = (sumas[k * 3 + c] - M[mPos] + 256) % 256;
        }
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    qDebug() << "=== Iniciando proceso de reconstruccion completa ===";

    // Configurar rutas
    QDir directorio(QCoreApplication::applicationDirPath());
    QString rutaBase = directorio.absoluteFilePath("Caso2/");

    // 1. Cargar todas las imágenes necesarias
    int ancho, alto, anchoM, altoM;
    unsigned char* P3 = cargarImagen(rutaBase + "P3.bmp", ancho, alto);
    unsigned char* I_M = cargarImagen(rutaBase + "I_M.bmp", ancho, alto);
    unsigned char* M = cargarImagen(rutaBase + "M.bmp", anchoM, altoM);

    if(!P3 || !I_M || !M) {
        qCritical() << "Error al cargar imagenes base";
        return 1;
    }

    // 1. Primer XOR: P3 ^ I_M = P2 (imagen rotada)
    aplicarXOR(P3, I_M, ancho * alto * 3);
    guardarImagen(rutaBase + "P2_reconstruida.bmp", P3, ancho, alto);

    // 2. Rotación inversa (3 bits izquierda para deshacer rotación derecha original)
    rotarIzquierda(P3, ancho * alto * 3, 3);
    guardarImagen(rutaBase + "P1_reconstruida.bmp", P3, ancho, alto);

    // 3. Segundo XOR: P1 ^ I_M = IO (imagen original)
    aplicarXOR(P3, I_M, ancho * alto * 3);

    guardarImagen(rutaBase + "I_O_reconstruida.bmp", P3, ancho, alto);
    rotarIzquierda(P3, ancho * alto * 3, 6);
    guardarImagen(rutaBase + "I_O_rotada.bmp", P3, ancho, alto);
    // Liberar memoria
    delete[] P3;
    delete[] I_M;


    /*// 2. Primer XOR: P3 ^ I_M = P2_enmascarado
    unsigned char* P2_masked = new unsigned char[ancho * alto * 3];
    memcpy(P2_masked, P3, ancho * alto * 3);
    aplicarXOR(P2_masked, I_M, ancho * alto * 3);
    guardarImagen(rutaBase + "P2_enmascarado.bmp", P2_masked, ancho, alto);

    // 3. Primer desenmascaramiento: P2_masked + M2.txt = P2
    int desplazamiento2;
    unsigned int* sumas2;
    int numSumas2;
    if(cargarEnmascaramiento(rutaBase + "M2.txt", desplazamiento2, sumas2, numSumas2)) {
        revertirEnmascaramiento(P2_masked, M, sumas2, desplazamiento2, numSumas2/3, ancho, anchoM);
        delete[] sumas2;
    }
    guardarImagen(rutaBase + "P2.bmp", P2_masked, ancho, alto);

    // 4. Rotación inversa (izquierda) a P2 = P1_enmascarado
    rotarIzquierda(P2_masked, ancho * alto * 3, 3);
    guardarImagen(rutaBase + "P1_enmascarado.bmp", P2_masked, ancho, alto);

    // 5. Segundo desenmascaramiento: P1_enmascarado + M1.txt = P1
    int desplazamiento1;
    unsigned int* sumas1;
    int numSumas1;
    if(cargarEnmascaramiento(rutaBase + "M1.txt", desplazamiento1, sumas1, numSumas1)) {
        revertirEnmascaramiento(P2_masked, M, sumas1, desplazamiento1, numSumas1/3, ancho, anchoM);
        delete[] sumas1;
    }
    guardarImagen(rutaBase + "P1.bmp", P2_masked, ancho, alto);

    // 6. Segundo XOR: P1 ^ IM = I_D
    aplicarXOR(P2_masked, I_M, ancho * alto * 3);
    guardarImagen(rutaBase + "I_D.bmp", P2_masked, ancho, alto);

    // 7. Rotación final inversa (izquierda) a I_D = I_O_reconstruida
    rotarIzquierda(P2_masked, ancho * alto * 3, 3);
    guardarImagen(rutaBase + "I_O_reconstruida.bmp", P2_masked, ancho, alto);

    // Liberar memoria
    delete[] P3;
    delete[] I_M;
    delete[] M;
    delete[] P2_masked;*/

    qDebug() << "=== Proceso completado exitosamente ===";
    qDebug() << "Imagen original reconstruida en:" << rutaBase + "I_O_reconstruida.bmp";

    return 0;
}
