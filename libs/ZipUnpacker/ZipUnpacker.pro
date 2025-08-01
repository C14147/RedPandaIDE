QT += core core-private gui-private
TEMPLATE = lib
CONFIG += staticlib
TARGET = ZipUnpacker
VERSION = 1.0.0

CONFIG += c++17

INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtCore/$$QT_VERSION                 \
               $$[QT_INSTALL_HEADERS]/QtCore/$$QT_VERSION/QtCore          \
               $$[QT_INSTALL_HEADERS]/QtCore/$$QT_VERSION/QtCore/private  \
message($$[QT_INSTALL_HEADERS]/QtCore/$$QT_VERSION/QtCore)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    zipunpacker.cpp \
    zipunpacker_global.cpp

HEADERS += \
    zipunpacker.h

TRANSLATIONS += \
    ZipUnpacker_en_US.ts

DEFINES += ZIPUNPACKER_LIBRARY

target.path = $$[QT_INSTALL_LIBS]
headers.path = $$[QT_INSTALL_HEADERS]/zipunpacker
headers.files = $$HEADERS

INSTALLS += target headers
