cmake_minimum_required(VERSION 3.16)

project(dicom_viewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

# DCMTK - DICOM Toolkit
find_package(DCMTK REQUIRED)

# VTK - Visualization Toolkit
find_package(VTK REQUIRED COMPONENTS
    CommonCore
    CommonDataModel
    CommonExecutionModel
    IOImage
    ImagingCore
    InteractionImage
    InteractionStyle
    RenderingCore
    RenderingOpenGL2
    GUISupportQt
)

set(TS_FILES dicom_viewer_pt_BR.ts)

# Models
set(MODEL_SOURCES
)

# Services
set(SERVICE_SOURCES
)

# Viewer (VTK)
set(VIEWER_SOURCES
    src/viewer/DicomViewer.h
    src/viewer/DicomViewer.cpp
)

# UI
set(UI_SOURCES
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/styles/StyleManager.h
    src/ui/styles/StyleManager.cpp
    src/ui/mainwindow.ui
)

# All sources
set(PROJECT_SOURCES
    main.cpp
    ${MODEL_SOURCES}
    ${SERVICE_SOURCES}
    ${VIEWER_SOURCES}
    ${UI_SOURCES}

    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(dicom_viewer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET dicom_viewer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(dicom_viewer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(dicom_viewer
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(dicom_viewer PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    DCMTK::DCMTK
    ${VTK_LIBRARIES}
)

# VTK auto-init (must be after target creation)
vtk_module_autoinit(
    TARGETS dicom_viewer
    MODULES ${VTK_LIBRARIES}
)

# Include directories
target_include_directories(dicom_viewer PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${DCMTK_INCLUDE_DIRS}
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.dicom_viewer)
endif()
set_target_properties(dicom_viewer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS dicom_viewer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(dicom_viewer)
endif()
