include(../../global.pri)

QT += gui-private

TEMPLATE = lib

CONFIG  += staticlib

TARGET   = json_utils

HEADERS += \
    rapid.h \
    jsoncpp.h \
    base.h \
    files.h

SOURCES += \
    rapid.cpp \
    jsoncpp.cpp \
    base.cpp \
    files.cpp

INCLUDEPATH += ../.. ../../../common_thirdparty/rapidjson/include

DEFINES     += RAPIDJSON_HAS_STDSTRING

objDirsTarget.target   = ../../obj/$(TARGET)/$(CONFIG)
objDirsTarget.depends  = FORCE
objDirsTarget.commands = if not exist ..\..\obj\$(TARGET)\$(CONFIG) (mkdir ..\..\obj\$(TARGET)\$(CONFIG))

PRE_TARGETDEPS      += ../../obj/json_utils
QMAKE_EXTRA_TARGETS += objDirsTarget

CONFIG(debug, debug|release) {
  DESTDIR     = ../../lib/debug
  OBJECTS_DIR = ../../obj/json_utils/debug
  MOC_DIR     = ../../obj/json_utils/debug/.moc
} else {
  DESTDIR     = ../../lib/release
  OBJECTS_DIR = ../../obj/json_utils/release
  MOC_DIR     = ../../obj/json_utils/debug/.moc
}
