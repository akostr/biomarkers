# include paths for all
INCLUDEPATH += $$PWD \
               $$PWD/extern \
               $$PWD/extern/common_core \
               $$PWD/extern/common_gui

DEFINES += WIN32_LEAN_AND_MEAN RAPIDJSON_HAS_STDSTRING

# settings writable location key
DEFINES += SETTINGS_WRITABLE_LOCATION_KEY=\\\"Genesis\\\"

# force debug info for all
CONFIG += force_debug_info

# obsolete
DEFINES += DISABLE_UBJSON \
           DISABLE_UBJSON_FORMAT \
           DISABLE_BOOST \
           DISABLE_CUSTOM_WINDOWS \
           DISABLE_STANDARD_MODELS_SERIALIZING \
           DISABLE_STANDARD_VIEWS \
           DISABLE_CONCURRENT \
           DISABLE_XLNT

DEFINES += JWT_ENABLED

macx {
# DEFINES += DISABLE_STYLES
}

# c++ latest
CONFIG          += C++1z
