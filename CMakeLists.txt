cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(CAD_Model_Viewer LANGUAGES CXX)

# Policies
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
  cmake_policy(SET CMP0071 NEW)
endif()

# Enable automoc, autouic
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Find VTK and its Qt components
find_package(VTK REQUIRED COMPONENTS
  CommonCore
  CommonDataModel
  FiltersSources
  GUISupportQt
  InteractionStyle
  RenderingContextOpenGL2
  RenderingCore
  RenderingFreeType
  RenderingGL2PSOpenGL2
  RenderingOpenGL2
  RenderingQt
  IOGeometry
)

if(NOT VTK_FOUND)
  message(FATAL_ERROR "Unable to find VTK")
endif()

if(NOT TARGET VTK::GUISupportQt)
  message(FATAL_ERROR "VTK not built with Qt support")
endif()

# Use VTK's Qt version variable
if(NOT DEFINED VTK_QT_VERSION)
  set(VTK_QT_VERSION 5)
endif()

set(qt_components Core Gui Widgets)
if(${VTK_QT_VERSION} VERSION_GREATER_EQUAL 6)
  list(APPEND qt_components OpenGLWidgets)
endif()

find_package(Qt${VTK_QT_VERSION} REQUIRED COMPONENTS ${qt_components})

# Prepare Qt targets
set(qt_modules "")
foreach(_comp IN LISTS qt_components)
  list(APPEND qt_modules Qt${VTK_QT_VERSION}::${_comp})
endforeach()

# Source files (manually list or glob)
set(SOURCES
  main.cpp
  mainwindow.cpp
  mainwindow.h
  mainwindow.ui
)

add_executable(CAD_Model_Viewer MACOSX_BUNDLE
  ${SOURCES}
  mainwindow.ui
)

# Link libraries
target_link_libraries(CAD_Model_Viewer PRIVATE
  ${qt_modules}
  ${VTK_LIBRARIES}
)

# Initialize VTK modules properly
vtk_module_autoinit(
  TARGETS CAD_Model_Viewer
  MODULES ${VTK_LIBRARIES}
)

# macOS bundle properties
set_target_properties(CAD_Model_Viewer PROPERTIES
  MACOSX_BUNDLE TRUE
  WIN32_EXECUTABLE TRUE
)
