cmake_minimum_required(VERSION 3.20)
project(JTLabelImage LANGUAGES CXX VERSION 1.0.0)

get_filename_component(KL_PROJECT_ROOT "${CMAKE_SOURCE_DIR}/.." ABSOLUTE)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${KL_PROJECT_ROOT}/install" CACHE PATH "Install prefix" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(MSVC)
    add_compile_options(/utf-8 /MP)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
find_package(OpenCV REQUIRED)

file(GLOB KL_SRC CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/algo/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/algo/*.h"
    "${CMAKE_SOURCE_DIR}/src/widgets/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/widgets/*.h"
    "${CMAKE_SOURCE_DIR}/src/app/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/app/*.h")

set(KL_RESOURCES "${CMAKE_SOURCE_DIR}/resources/resources.qrc")
if(WIN32)
    list(APPEND KL_RESOURCES "${CMAKE_SOURCE_DIR}/resources/app.rc")
endif()

add_executable(JTLabelImage WIN32 ${KL_SRC} ${KL_RESOURCES})

target_include_directories(JTLabelImage PRIVATE
    "${CMAKE_SOURCE_DIR}/src/algo"
    "${CMAKE_SOURCE_DIR}/src/widgets"
    "${CMAKE_SOURCE_DIR}/src/app"
    ${OpenCV_INCLUDE_DIRS})

target_compile_definitions(JTLabelImage PRIVATE
    KELVINLABEL_VERSION="${PROJECT_VERSION}")

target_link_libraries(JTLabelImage PRIVATE
    Qt6::Widgets
    Qt6::Network
    ${OpenCV_LIBS})

set(_KL_RUNTIME_DIR "${KL_PROJECT_ROOT}/msvc_release")
set_target_properties(JTLabelImage PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${_KL_RUNTIME_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${_KL_RUNTIME_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${_KL_RUNTIME_DIR}")

file(MAKE_DIRECTORY
    "${_KL_RUNTIME_DIR}/models"
    "${_KL_RUNTIME_DIR}/input"
    "${_KL_RUNTIME_DIR}/output")

install(TARGETS JTLabelImage RUNTIME DESTINATION .)

if(WIN32)
    get_target_property(_QMAKE_EXE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_QT_BIN_DIR "${_QMAKE_EXE}" DIRECTORY)
    find_program(WINDEPLOYQT_EXE windeployqt HINTS "${_QT_BIN_DIR}")
    if(WINDEPLOYQT_EXE)
        add_custom_command(TARGET JTLabelImage POST_BUILD
            COMMAND "${WINDEPLOYQT_EXE}"
                    --no-translations
                    --no-system-d3d-compiler
                    --no-opengl-sw
                    "$<TARGET_FILE:JTLabelImage>"
            COMMENT "Deploying Qt runtime dependencies")
    endif()
endif()
