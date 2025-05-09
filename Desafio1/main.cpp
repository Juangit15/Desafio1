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

    qDebug() << "Imagen cargada:" << ruta << "| Dimensiones:" << ancho << "x" << alto;
    return datos;
}

// Función para guardar imagen BMP
bool guardarImagen(const QString& ruta, unsigned char* datos, int ancho, int alto) {
    QImage imagen(datos, ancho, alto, QImage::Format_RGB888);
    bool resultado = imagen.save(ruta, "BMP");
    qDebug() << "Imagen guardada:" << ruta << "| Exitoso:" << resultado;
    return resultado;
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
    qDebug() << "Desplazamiento leido:" << desplazamiento;

    // Primera pasada para contar elementos
    int contador = 0;
    while(!in.atEnd()) {
        QString linea = in.readLine();
        QStringList componentes = linea.split(' ', Qt::SkipEmptyParts);
        if(componentes.size() == 3) {
            contador += 3;
        }
    }

    // Volver al inicio
    archivo.seek(0);
    in.readLine(); // Saltar línea de desplazamiento

    // Reservar memoria
    numSumas = contador;
    sumas = new unsigned int[numSumas];
    int idx = 0;

    // Segunda pasada para leer datos
    while(!in.atEnd()) {
        QString linea = in.readLine();
        QStringList componentes = linea.split(' ', Qt::SkipEmptyParts);
        if(componentes.size() == 3) {
            sumas[idx++] = componentes[0].toUInt();
            sumas[idx++] = componentes[1].toUInt();
            sumas[idx++] = componentes[2].toUInt();
        }
    }

    qDebug() << "Archivo de enmascaramiento cargado:" << ruta << "| Sumas leidas:" << numSumas;
    return true;
}

/**
 * Aplica operación XOR entre dos imágenes
 * @param img1 Primera imagen (será modificada)
 * @param img2 Segunda imagen (no se modifica)
 * @param totalPixeles Número total de píxeles (ancho*alto*3)
 */
void aplicarXOR(unsigned char* img1, const unsigned char* img2, int totalPixeles) {
    qDebug() << "Aplicando XOR entre imágenes...";
    for(int i = 0; i < totalPixeles; i++) {
        img1[i] ^= img2[i];
    }
    qDebug() << "Operación XOR completada";
}

/**
 * Rota los bits de una imagen a la izquierda
 * @param img Imagen a rotar
 * @param totalPixeles Número total de píxeles
 * @param bits Cantidad de bits a rotar (1-8)
 */
void rotarIzquierda(unsigned char* img, int totalPixeles, int bits) {
    qDebug() << "Rotando" << bits << "bits a la izquierda...";
    bits %= 8;
    if(bits == 0) return;

    for(int i = 0; i < totalPixeles; i++) {
        img[i] = (img[i] << bits) | (img[i] >> (8 - bits));
    }
    qDebug() << "Rotación completada";
}

/**
 * Revertir el enmascaramiento aplicado a una imagen
 * @param ID Imagen destino (será modificada)
 * @param M Máscara utilizada
 * @param sumas Resultados del enmascaramiento original
 * @param desplazamiento Posición inicial del enmascaramiento
 * @param pixelesMascara Número de píxeles en la máscara
 * @param anchoID Ancho de la imagen destino
 * @param anchoM Ancho de la máscara
 */
void revertirEnmascaramiento(unsigned char* ID, const unsigned char* M,
                             const unsigned int* sumas, int desplazamiento,
                             int pixelesMascara, int anchoID, int anchoM) {
    qDebug() << "Revirtiendo enmascaramiento...";
    qDebug() << "Desplazamiento:" << desplazamiento << "| Pixeles máscara:" << pixelesMascara;

    for(int k = 0; k < pixelesMascara; k++) {
        for(int c = 0; c < 3; c++) {
            int pos = (desplazamiento * 3 + k * 3 + c) % (anchoID * anchoID * 3);
            int mPos = (k * 3 + c) % (anchoM * anchoM * 3);

            // Verificación de límites
            if(pos >= anchoID * anchoID * 3) {
                qWarning() << "Posición de imagen excede límites:" << pos;
                continue;
            }
            if(mPos >= anchoM * anchoM * 3) {
                qWarning() << "Posición de máscara excede límites:" << mPos;
                continue;
            }

            // Revertir la suma del enmascaramiento
            ID[pos] = (sumas[k * 3 + c] - M[mPos] + 256) % 256;
        }
    }
    qDebug() << "Enmascaramiento revertido";
}


/**
 * Aplica un desplazamiento circular a una imagen
 * @param img Imagen a desplazar (será modificada)
 * @param totalPixeles Número total de píxeles
 * @param desplazamiento Cantidad de bytes a desplazar (positivo = derecha, negativo = izquierda)
 */
void aplicarDesplazamiento(unsigned char* img, int totalPixeles, int desplazamiento) {
    if(desplazamiento == 0) return;

    qDebug() << "Aplicando desplazamiento de" << desplazamiento << "bytes...";

    // Normalizamos el desplazamiento
    desplazamiento %= totalPixeles;
    if(desplazamiento < 0) {
        desplazamiento += totalPixeles;
    }

    // Creamos buffer temporal
    unsigned char* temp = new unsigned char[totalPixeles];

    // Aplicamos desplazamiento circular
    memcpy(temp, img + totalPixeles - desplazamiento, desplazamiento);
    memcpy(temp + desplazamiento, img, totalPixeles - desplazamiento);
    memcpy(img, temp, totalPixeles);

    delete[] temp;
    qDebug() << "Desplazamiento aplicado";
}

// Función para cargar múltiples imágenes y verificar dimensiones
bool cargarImagenes(const QString& rutaBase, QString nombreP3, QString nombreIM, QString nombreM,
                    unsigned char*& P3, unsigned char*& I_M, unsigned char*& M,
                    int& ancho, int& alto, int& anchoM, int& altoM) {
    P3 = cargarImagen(rutaBase + nombreP3, ancho, alto);
    I_M = cargarImagen(rutaBase + nombreIM, ancho, alto);
    M = cargarImagen(rutaBase + nombreM, anchoM, altoM);

    if (!P3 || !I_M || !M) {
        qCritical() << "Error al cargar imagenes base";
        return false;
    }
    qDebug() << "Dimensiones M:" << anchoM << "x" << altoM;
    return true;
}


int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    qDebug() << "=== Iniciando proceso de reconstruccion completa ===";

    // Configurar rutas
    QDir directorio(QCoreApplication::applicationDirPath());
    QString rutaBase = directorio.absoluteFilePath("Caso1/");
    qDebug() << "Directorio base:" << rutaBase;

    // 1. Cargar todas las imágenes necesarias
    qDebug() << "\n=== Cargando imágenes ===";
    int ancho, alto, anchoM, altoM;
    unsigned char* P3 = nullptr;
    unsigned char* I_M = nullptr;
    unsigned char* M = nullptr;

    if (!cargarImagenes(rutaBase, "P3.bmp", "I_M.bmp", "M.bmp", P3, I_M, M, ancho, alto, anchoM, altoM)) {
        return 1;
    }

    // Verificar si las dimensiones de la máscara coinciden
    if (ancho != anchoM || alto != altoM) {
        qWarning() << "¡Advertencia! Las dimensiones de la máscara (M.bmp) no coinciden con las dimensiones de las otras imágenes.";
        qWarning() << "Imagen P3/I_M:" << ancho << "x" << alto << ", Máscara M:" << anchoM << "x" << altoM;
        // Considera si quieres detener la ejecución aquí si la discrepancia es crítica.
        // return 1;
    }

    // 2. Primer XOR: P3 ^ I_M = P2_enmascarado
    qDebug() << "\n=== Paso 1: Aplicando XOR (P3 ^ I_M) ===";
    unsigned char* P2_masked = new unsigned char[ancho * alto * 3];
    memcpy(P2_masked, P3, ancho * alto * 3);
    aplicarXOR(P2_masked, I_M, ancho * alto * 3);
    guardarImagen(rutaBase + "P2_enmascarado.bmp", P2_masked, ancho, alto);

    // 3. Primer desenmascaramiento: P2_masked + M2.txt = P2
    qDebug() << "\n=== Paso 2: Revirtiendo enmascaramiento (M2.txt) ===";
    int desplazamiento2;
    unsigned int* sumas2;
    int numSumas2;
    if(cargarEnmascaramiento(rutaBase + "M2.txt", desplazamiento2, sumas2, numSumas2)) {
        qDebug() << "Desplazamiento M2.txt:" << desplazamiento2;
        revertirEnmascaramiento(P2_masked, M, sumas2, desplazamiento2, numSumas2/3, ancho, anchoM);
        delete[] sumas2;
    }
    guardarImagen(rutaBase + "P2.bmp", P2_masked, ancho, alto);

    // 4. Rotación inversa (izquierda) a P2 = P1_enmascarado
    qDebug() << "\n=== Paso 3: Rotación inversa (3 bits izquierda) ===";
    rotarIzquierda(P2_masked, ancho * alto * 3, 3);
    guardarImagen(rutaBase + "P1_enmascarado.bmp", P2_masked, ancho, alto);

    // 5. Segundo desenmascaramiento: P1_enmascarado + M1.txt = P1
    qDebug() << "\n=== Paso 4: Revirtiendo enmascaramiento (M1.txt) ===";
    int desplazamiento1;
    unsigned int* sumas1;
    int numSumas1;
    if(cargarEnmascaramiento(rutaBase + "M1.txt", desplazamiento1, sumas1, numSumas1)) {
        qDebug() << "Desplazamiento M1.txt:" << desplazamiento1;
        revertirEnmascaramiento(P2_masked, M, sumas1, desplazamiento1, numSumas1/3, ancho, anchoM);
        delete[] sumas1;
    }
    guardarImagen(rutaBase + "P1.bmp", P2_masked, ancho, alto);

    // 6. Segundo XOR: P1 ^ IM = I_D (imagen original)
    qDebug() << "\n=== Paso 5: Aplicando XOR final (P1 ^ I_M) ===";
    aplicarXOR(P2_masked, I_M, ancho * alto * 3);
    guardarImagen(rutaBase + "I_O_reconstruida.bmp", P2_masked, ancho, alto);


    // Liberar memoria
    delete[] P3;
    delete[] I_M;
    delete[] M;
    delete[] P2_masked;

    qDebug() << "\n=== Proceso completado exitosamente ===";
    qDebug() << "Imagen original reconstruida en:" << rutaBase + "I_O_reconstruida.bmp";

    return 0;
}

