# inherit parent project defines 
exists ($$PWD/../../global.pri) {
  include($$PWD/../../global.pri)
}

# license embeddment from PARENT project
contains (DEFINES, CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API) {
  INCLUDEPATH += $$PWD/../../license_embeddment_api
}

# force debug info for all
CONFIG += force_debug_info

INCLUDEPATH += $$PWD/../common_thirdparty

!contains(DEFINES, DISABLE_BOOST) {
  INCLUDEPATH += $$PWD/../common_thirdparty/boost
}

DEFINES     += RAPIDJSON_HAS_STDSTRING
DEFINES     += USE_PYTHON_CUSTOM_MODULES
