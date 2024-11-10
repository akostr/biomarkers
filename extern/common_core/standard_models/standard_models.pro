include(../global.pri)

TEMPLATE = lib

CONFIG  += staticlib

exists(../../extern_global.pri) {
  include(../../extern_global.pri)
}

exists(../../common_core/settings/settings.pri) {
  if (!contains(DEFINES, DISABLE_SETTINGS)) {
    include(../../common_core/settings/settings.pri)
  }
}

exists(../../common_core/utils/json_utils/json_utils.pri) {
  if (!contains(DEFINES, DISABLE_UBJSON)) {
    include(../../common_core/utils/json_utils/json_utils.pri)
  }
}

# must remain purely core
QT      += core-private core

TARGET   = standard_models

INCLUDEPATH += ..

# pure core standarditemmodel reimpl
SOURCES += abstract_uom_model.cpp \
           standard_item_model_base.cpp \
           standard_item_model.cpp \
           uom_pair.cpp

HEADERS += abstract_uom_model.h \
           abstract_uom_model_traits.h \
           standard_item_model_base_p.h \
           standard_item_model_base.h \
           standard_item_model.h

# impl
SOURCES += standard_uom_model.cpp \
           uom.cpp \
           uom_settings.cpp

HEADERS += standard_uom_model.h \
           uom.h \
           uom_pair.h \
           uom_settings.h

RESOURCES = standard_models.qrc

objDirsTarget.target   = ../obj/$(TARGET)/$(CONFIG)
objDirsTarget.depends  = FORCE
objDirsTarget.commands = if not exist ..\obj\$(TARGET)\$(CONFIG) (mkdir ..\obj\$(TARGET)\$(CONFIG))

PRE_TARGETDEPS      += ../obj/standard_models
QMAKE_EXTRA_TARGETS += objDirsTarget

CONFIG(debug, debug|release) {
  DESTDIR     = ../lib/debug
  OBJECTS_DIR = ../obj/standard_models/debug
  MOC_DIR     = ../obj/standard_models/debug/.moc
  UI_DIR      = ../obj/standard_models/debug/.ui
} else {
  DESTDIR     = ../lib/release
  OBJECTS_DIR = ../obj/standard_models/release
  MOC_DIR     = ../obj/standard_models/release/.moc
  UI_DIR      = ../obj/standard_models/release/.ui
}
