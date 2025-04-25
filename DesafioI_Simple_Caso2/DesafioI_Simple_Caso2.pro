TEMPLATE = app
TARGET = DesafioI_Simple_Caso2
CONFIG += console c++11
INCLUDEPATH += include

SOURCES += \
    main.cpp \
    src/bmp.cpp \
    src/desafio1.cpp \
    src/operaciones.cpp \
    src/verificador.cpp

HEADERS += \
    include/bmp.h \
    include/operaciones.h \
    include/verificador.h \
    src/desafio1.h
