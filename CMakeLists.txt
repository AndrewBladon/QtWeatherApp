# Copyright 2023 Esri

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.5)

project(ConditionsNavigator LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(IOS)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum iOS deployment version" FORCE)
elseif(APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment version" FORCE)
endif()

find_package(Qt6 COMPONENTS REQUIRED Core Quick Multimedia Positioning Sensors WebSockets Network Charts Widgets)
if(ANDROID OR IOS)
    find_package(Qt6 COMPONENTS REQUIRED Bluetooth)
endif()
find_package(ArcGISRuntime 200.3.0 COMPONENTS REQUIRED Cpp)

set(SOURCE_FILES
  main.cpp
  ConditionsNavigator.h
  ConditionsNavigator.cpp
  OpenMeteoForecastSource.h
  OpenMeteoForecastSource.cpp
  Mountain.h
  Mountain.cpp
  MountainLocations.h
  qml/qml.qrc
  Resources/Resources.qrc
  $<$<BOOL:${WIN32}>:Win/Resources.rc>
  $<$<BOOL:${APPLE}>:Mac/AppIcon.icns>)

if(ANDROID)
    qt_add_executable(ConditionsNavigator ${SOURCE_FILES})
    set_property(TARGET ConditionsNavigator APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android)
else()
  qt_add_executable(ConditionsNavigator MACOSX_BUNDLE ${SOURCE_FILES})
  # IOS uses static Runtimecore Framework
  if(IOS)
    set_target_properties(ConditionsNavigator PROPERTIES INSTALL_RPATH @executable_path/Frameworks 
    XCODE_EMBED_FRAMEWORKS ${ArcGISRuntime_runtimecore_LIB} 
    XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY TRUE)
  endif()
  # On MacOSX add icon to app bundle.
  set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)
  set_source_files_properties(Mac/AppIcon.icns
    PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

  # Copy required dynamic libraries to the build folder as a post-build step.
  if(DEFINED ArcGISRuntime_LIBRARIES)
    add_custom_command(TARGET ConditionsNavigator POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${ArcGISRuntime_LIBRARIES}
      $<TARGET_FILE_DIR:ConditionsNavigator>)
  endif()
endif()

target_compile_definitions(ConditionsNavigator
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(ConditionsNavigator PRIVATE
  Qt6::Core
  Qt6::Quick
  Qt6::Multimedia
  Qt6::Positioning
  Qt6::Sensors
  Qt6::WebSockets
  ArcGISRuntime::Cpp
  Qt6::Widgets)

if(ANDROID OR IOS)
  target_link_libraries(ConditionsNavigator PRIVATE Qt6::Bluetooth)
endif()

if(ANDROID)
  target_link_libraries(ConditionsNavigator PRIVATE Qt::CorePrivate)

  set(PROJECT_DEPLOYABLE_LIBS
    ${ArcGISRuntime_LIBRARIES})

  list(JOIN PROJECT_DEPLOYABLE_LIBS , PROJECT_DEPLOYABLE_LIBS_STRING)

  # Setup openssl by cloning the repo https://github.com/KDAB/android_openssl/blob/update/CMakeLists.txt . More info on it at https://doc.qt.io/qt-6/android-openssl-support.html
  # QtCreator can also be used to install openssl https://doc.qt.io/qtcreator/creator-developing-android.html#specifying-android-device-settings
  # Uncomment below line and point to your openssl path.

  include(C:/Users/<YourUsername>/AppData/Local/Android/Sdk/android_openssl/CMakeLists.txt) # path needs updating to build on Android

  # QtCreator supports the following variables for Android, which are identical
  # to qmake Android variables.
  # Check http://doc.qt.io/qt-6/deployment-android.html for more information.
  # Setting this property changed at Qt6 see doc below.
  # https://doc.qt.io/qt-6/cmake-target-property-qt-android-extra-libs.html

  set_property(TARGET ConditionsNavigator PROPERTY QT_ANDROID_EXTRA_LIBS ${PROJECT_DEPLOYABLE_LIBS_STRING} ${ANDROID_EXTRA_LIBS})
  get_property(EXTRA_LIBS TARGET ConditionsNavigator PROPERTY QT_ANDROID_EXTRA_LIBS)
  if(NOT EXTRA_LIBS MATCHES "libssl" OR NOT EXTRA_LIBS MATCHES "libcrypto")
      message(WARNING "openssl libraries are missing, check the project CMakeLists.txt and set up openssl environment")
  endif()
endif()
