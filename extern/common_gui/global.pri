# inherit parent project defines
exists ($$PWD/../../global.pri) {
  include($$PWD/../../global.pri)
}

# license embeddment from PARENT project
contains (DEFINES, CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API) {
  INCLUDEPATH += $$PWD/../../license_embeddment_api
}

INCLUDEPATH += $$PWD/../common_thirdparty \
               $$PWD/../common_core

#DEFINES += DISABLE_CUSTOM_WINDOWS
#DEFINES += DISABLE_SETTINGS
#DEFINES += DISABLE_UBJSON
#DEFINES += DISABLE_XLNT
#DEFINES += DISABLE_CYBER_FRAC_STYLE
#DEFINES += DISABLE_CONCURRENT
#DEFINES += DISABLE_STANDARD_MODELS
#DEFINES += DISABLE_STANDARD_VIEWS

# force debug info for all
CONFIG += force_debug_info

COMMON_GUI_OBJ_PATH = $$PWD/obj
COMMON_GUI_LIB_PATH = $$PWD/lib
COMMON_GUI_PATH = $$PWD
