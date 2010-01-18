cmake_minimum_required(VERSION 2.8)
project(wxLauncher cxx)

include(CPack)
find_package(wxWidgets 2.8.10
  REQUIRED base core net xml html adv media qa richtext)
include(FindPythonInterp) #PYTHON_EXECUTABLE

option(onlinehelpmaker_debug "Have the onlinehelpmaker.py script output debug information" OFF)
if(onlinehelpmaker_debug)
SET(HELPMAKER_DEBUG "-d")
else(onlinehelpmaker_debug)
SET(HELPMAKER_DEBUG "")
endif(onlinehelpmaker_debug)

SET(helphtblocation ${CMAKE_CURRENT_BINARY_DIR}/generated/onlinehelp.htb)
option(DEVELOPMENT_MODE "Doing development" OFF)
if(DEVELOPMENT_MODE)
  set(HELP_HTB_LOCATION ${helphtblocation})
  SET(RESOURCES_PATH ${PROJECT_SOURCE_DIR}/resources)
else(DEVELOPMENT_MODE)
  SET(RESOURCES_PATH ./)
  set(HELP_HTB_LOCATION ./onlinehelp.htb)
endif(DEVELOPMENT_MODE)

OPTION(USE_SPEECH "Build launcher with speech support?" OFF)
OPTION(USE_JOYSTICK "Build launcher with joystick support (currently windows MM only)?" OFF)
OPTION(USE_OPENAL "Build launcher with OpenAL support?" OFF)
if(USE_OPENAL)
  find_package(OpenAL REQUIRED)
  INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR})
endif(USE_OPENAL)


INCLUDE(${wxWidgets_USE_FILE})
add_executable(registry_helper WIN32
  code/Win32ProfileManager.cpp platform/win32/registry_helper.cpp)
SET_TARGET_PROPERTIES(registry_helper
  PROPERTIES LINKER_LANGUAGE CXX)
if(WIN32)
  SET_TARGET_PROPERTIES(registry_helper PROPERTIES LINK_FLAGS /MANIFESTUAC:NO)
endif(WIN32)
TARGET_LINK_LIBRARIES(registry_helper ${wxWidgets_LIBRARIES})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated/version.cpp
  COMMAND ${PYTHON_EXECUTABLE} scripts/version.cpp.maker.py build ${CMAKE_CURRENT_BINARY_DIR}/generated/version.cpp ${CMAKE_CURRENT_BINARY_DIR}/version.cpp.maker.temp
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp  ${helphtblocation}
  COMMAND ${PYTHON_EXECUTABLE} scripts/onlinehelpmaker.py build  ${CMAKE_CURRENT_BINARY_DIR}/generated/onlinehelp.htb ${CMAKE_SOURCE_DIR}/onlinehelp ${HELPMAKER_DEBUG} -t ${CMAKE_CURRENT_BINARY_DIR}/onlinehelpmaker -c ${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
file(GLOB_RECURSE helpmaker_temp_files ${CMAKE_CURRENT_BINARY_DIR}/onlinehelpmaker/*.*)

add_executable(wxlauncher WIN32 MACOSX_BUNDLE
  ${CMAKE_CURRENT_BINARY_DIR}/generated/version.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp
  ${helphtblocation}
  code/AdvSettingsPage.cpp code/BasicSettingsPage.cpp
  code/BottomButtons.cpp 
  code/FlagList.cpp code/FSOExecutable.cpp
  code/HelpManager.cpp code/InstallPage.cpp
  code/JoystickManager.cpp code/Logger.cpp
  code/MainWindow.cpp code/ModList.cpp
  code/ModsPage.cpp code/OpenALManager.cpp
  code/ProfileManager.cpp code/Skin.cpp
  code/SpeechManager.cpp code/StatusBar.cpp
  code/TCManager.cpp code/WelcomePage.cpp
  code/WelcomePage.cpp code/Win32ProfileManager.cpp
  code/wxLauncherApp.cpp platform/win32/wxlauncher.rc)
SET_TARGET_PROPERTIES(wxlauncher
  PROPERTIES LINKER_LANGUAGE CXX)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/onlinehelp.htb ${CMAKE_CURRENT_BINARY_DIR}/generated/confgiure_launcher.h
  PROPERTIES HEADER_FILE_ONLY true)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/code/configure_launcher.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/generated/configure_launcher.h)

foreach(temp_file ${helpmaker_temp_files})
  SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${temp_file})
endforeach(temp_file)

TARGET_LINK_LIBRARIES(wxlauncher ${wxWidgets_LIBRARIES})