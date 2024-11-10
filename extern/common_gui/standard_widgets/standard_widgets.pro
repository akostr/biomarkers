include(../global.pri)

TEMPLATE = lib

CONFIG  += staticlib

QT      += core gui widgets

TARGET   = standard_widgets

exists(../../extern_global.pri) {
  include(../../extern_global.pri)
}

exists(../../common_core/settings/settings.pri) {
  if (!contains(DEFINES, DISABLE_SETTINGS)) {
    include(../../common_core/settings/settings.pri)
  }
}

exists(../custom_windows/custom_windows.pri) {
  if (!contains(DEFINES, DISABLE_CUSTOM_WINDOWS)) {
    include(../custom_windows/custom_windows.pri)
  }
}

exists(../standard_style) {
  if (!contains(DEFINES, DISABLE_CYBER_FRAC_STYLE)) {
    INCLUDEPATH += ../standard_style
  }
}

SOURCES += collapsed_widgets_view.cpp \
            code_editor.cpp \
            color_select_button.cpp \
            elided_label.cpp \
            icon_cache.cpp \
            persistent_menu.cpp \
            plain_scroll_area.cpp \
            progress_dialog.cpp \
            range_slider.cpp \
            range_widget.cpp \
            switch_widget.cpp \
            tree_view_combobox.cpp \
            tumbler.cpp \
            wait_indicator.cpp \
            warnings_frame.cpp \
            progressicon.cpp \
            progresslabel.cpp

HEADERS += collapsed_widgets_view.h \
            code_editor.h \
            color_select_button.h \
            elided_label.h \
            icon_cache.h \
            persistent_menu.h \
            plain_scroll_area.h \
            progress_dialog.h \
            range_slider.h \
            range_widget.h \
            switch_widget.h \
            tumbler.h \
            wait_indicator.h \
            warnings_frame.h \
            tree_view_combobox.h \
            progressicon.h \
            progresslabel.h

INCLUDEPATH += ..

objDirsTarget.target   = ../obj/$(TARGET)/$(CONFIG)
objDirsTarget.depends  = FORCE
objDirsTarget.commands = if not exist ..\obj\$(TARGET)\$(CONFIG) (mkdir ..\obj\$(TARGET)\$(CONFIG))

PRE_TARGETDEPS      += ../obj/standard_widgets
QMAKE_EXTRA_TARGETS += objDirsTarget

CONFIG(debug, debug|release) {
  DESTDIR     = ../lib/debug
  OBJECTS_DIR = ../obj/standard_widgets/debug
  MOC_DIR     = ../obj/standard_widgets/debug/.moc
  UI_DIR      = ../obj/standard_widgets/debug/.ui
} else {
  DESTDIR     = ../lib/release
  OBJECTS_DIR = ../obj/standard_widgets/release
  MOC_DIR     = ../obj/standard_widgets/release/.moc
  UI_DIR      = ../obj/standard_widgets/release/.ui
}
