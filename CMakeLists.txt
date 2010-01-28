cmake_minimum_required(VERSION 2.8)
project(wxlauncher CXX)
SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 6)
SET(VERSION_PATCH 0)

find_package(wxWidgets 2.8.10
  REQUIRED base core net xml html adv qa richtext)
include(FindPythonInterp) #PYTHON_EXECUTABLE

option(onlinehelpmaker_debug "Have the onlinehelpmaker.py script output debug information" OFF)
if(onlinehelpmaker_debug)
  SET(HELPMAKER_DEBUG "-d")
  SET(HELPMAKER_QUIET "")
else(onlinehelpmaker_debug)
  SET(HELPMAKER_DEBUG "")
  option(onlinehelpmaker_quiet "Have onlinehelpmaker.py produce even less output than normal" ON)
  if(onlinehelpmaker_quiet)
    SET(HELPMAKER_QUIET "-q")
  else()
    SET(HELPMAKER_QUIET "")
  endif()
endif(onlinehelpmaker_debug)

SET(helphtblocation ${CMAKE_CURRENT_BINARY_DIR}/generated/onlinehelp.htb)
option(DEVELOPMENT_MODE "Doing development (cannot build installer in this mode)" OFF)
if(DEVELOPMENT_MODE)
  SET(HELP_HTB_LOCATION ${helphtblocation})
  SET(RESOURCES_PATH ${PROJECT_SOURCE_DIR}/resources)
else(DEVELOPMENT_MODE)
  if(WIN32)
    SET(RESOURCES_PATH resources)
    SET(HELP_HTB_LOCATION onlinehelp.htb)
  else()
    SET(RESOURCES_PATH /usr/local/share/wxlauncher/)
    SET(HELP_HTB_LOCATION /usr/local/share/wxlauncher/onlinehelp.htb)
  endif()
endif(DEVELOPMENT_MODE)

OPTION(USE_SPEECH "Build launcher with speech support?" OFF)
OPTION(USE_JOYSTICK "Build launcher with joystick support (currently windows MM only)?" OFF)
OPTION(USE_OPENAL "Build launcher with OpenAL support?" ON)
if(USE_OPENAL)
  find_package(OpenAL REQUIRED)
  INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR})
endif(USE_OPENAL)

if(UNIX)
  find_package(SDL REQUIRED)
  INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR})
  SET(HAS_SDL true)
endif()

INCLUDE(${wxWidgets_USE_FILE})
if(WIN32) # helper is only needed on windows
add_executable(registry_helper WIN32
  code/Win32ProfileManager.cpp platform/win32/registry_helper.cpp)
SET_TARGET_PROPERTIES(registry_helper
  PROPERTIES LINKER_LANGUAGE CXX)
  SET_TARGET_PROPERTIES(registry_helper PROPERTIES LINK_FLAGS /MANIFESTUAC:NO)
TARGET_LINK_LIBRARIES(registry_helper ${wxWidgets_LIBRARIES})
endif()

add_custom_target(version.cpp.maker ALL
  COMMAND ${PYTHON_EXECUTABLE} scripts/version.cpp.maker.py build ${CMAKE_CURRENT_BINARY_DIR}/generated/version_strings.cpp ${CMAKE_CURRENT_BINARY_DIR}/version.cpp.maker.temp
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/generated/version_strings.cpp)

add_custom_target(helpmaker ALL
  COMMAND ${PYTHON_EXECUTABLE} scripts/onlinehelpmaker.py build  ${helphtblocation} ${CMAKE_SOURCE_DIR}/onlinehelp ${HELPMAKER_QUIET} ${HELPMAKER_DEBUG} -t ${CMAKE_CURRENT_BINARY_DIR}/onlinehelpmaker -c ${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp)
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${helphtblocation})
file(GLOB_RECURSE helpmaker_temp_files ${CMAKE_CURRENT_BINARY_DIR}/onlinehelpmaker/*.*)
SET_SOURCE_FILES_PROPERTIES(${helphtblocation} ${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp ${CMAKE_CURRENT_BINARY_DIR}/generated/version_strings.cpp PROPERTIES GENERATED true EXTERNAL_OBJECT true)
if(WIN32)
SET(PLATFORM_PROFILE_MANAGER code/Win32ProfileManager.cpp)
else()
SET(PLATFORM_PROFILE_MANAGER code/UnixProfileManager.cpp)
endif()

add_executable(wxlauncher WIN32 MACOSX_BUNDLE
  ${CMAKE_CURRENT_BINARY_DIR}/generated/version_strings.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp
  ${helphtblocation}
  code/AdvSettingsPage.cpp code/BasicSettingsPage.cpp
  code/BottomButtons.cpp code/version.cpp
  code/FlagList.cpp code/FSOExecutable.cpp
  code/HelpManager.cpp code/InstallPage.cpp
  code/JoystickManager.cpp code/Logger.cpp
  code/MainWindow.cpp code/ModList.cpp
  code/ModsPage.cpp code/OpenALManager.cpp
  code/ProfileManager.cpp code/Skin.cpp
  code/SpeechManager.cpp code/StatusBar.cpp
  code/TCManager.cpp code/WelcomePage.cpp
  code/WelcomePage.cpp ${PLATFORM_PROFILE_MANAGER}
  code/wxLauncherApp.cpp platform/win32/wxlauncher.rc)
SET_TARGET_PROPERTIES(wxlauncher
  PROPERTIES LINKER_LANGUAGE CXX)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/onlinehelp.htb ${CMAKE_CURRENT_BINARY_DIR}/generated/configure_launcher.h
  PROPERTIES HEADER_FILE_ONLY true)
ADD_DEPENDENCIES(wxlauncher helpmaker version.cpp.maker)
if(WIN32)
ADD_DEPENDENCIES(wxlauncher registry_helper)
endif(WIN32)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/code/configure_launcher.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/generated/configure_launcher.h)

foreach(temp_file ${helpmaker_temp_files})
  SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${temp_file})
endforeach(temp_file)

TARGET_LINK_LIBRARIES(wxlauncher ${wxWidgets_LIBRARIES})

# packaging
SET(LAUNCHER_CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/cmake)
INCLUDE(${LAUNCHER_CMAKE_SOURCE_DIR}/wxLauncherInstaller.cmake)
