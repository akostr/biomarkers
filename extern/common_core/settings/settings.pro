include(../global.pri)

TEMPLATE = lib

CONFIG  += staticlib

QT      += core

TARGET   = settings

SOURCES += settings.cpp

HEADERS += settings.h

# Uis
INCLUDEPATH += ..

CONFIG(debug, debug|release) {
  DESTDIR     = ../lib/debug
  OBJECTS_DIR = ../obj/settings/debug
  MOC_DIR     = ../obj/settings/debug/.moc
  RCC_DIR     = ../obj/settings/debug/.rcc
  UI_DIR      = ../obj/settings/debug/.ui
} else {
  DESTDIR     = ../lib/release
  OBJECTS_DIR = ../obj/settings/release
  MOC_DIR     = ../obj/settings/release/.moc
  RCC_DIR     = ../obj/settings/release/.rcc
  UI_DIR      = ../obj/settings/release/.ui
}
