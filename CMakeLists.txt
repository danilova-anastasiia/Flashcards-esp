cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Flashcards-esp)

idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
target_compile_definitions(${lvgl_lib} PUBLIC
    LV_CONF_PATH="${CMAKE_CURRENT_SOURCE_DIR}/lib/lv_conf.h")