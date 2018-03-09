TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    voice_manager.cpp \
    param_engine.cpp \
    utils.cpp \
    decoder.cpp \
    exponentiator.cpp \
    env_engine.cpp \
    audio_engine.cpp \
    key_event.cpp \
    param_engine_post.cpp

HEADERS += \
    voice_manager.h \
    global_defines_config.h \
    global_defines_params.h \
    global_defines_protocol.h \
    param_engine.h \
    utils.h \
    decoder.h \
    exponentiator.h \
    env_engine.h \
    global_defines_lists.h \
    audio_engine.h \
    global_defines_envelopes.h \
    key_event.h
