cmake_minimum_required(VERSION 2.8)
project(wxlauncher CXX)
SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 6)
SET(VERSION_PATCH 0)

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
option(DEVELOPMENT_MODE "Doing development (cannot build installer in this mode)" OFF)
if(DEVELOPMENT_MODE)
  set(HELP_HTB_LOCATION ${helphtblocation})
  SET(RESOURCES_PATH ${PROJECT_SOURCE_DIR}/resources)
else(DEVELOPMENT_MODE)
  SET(RESOURCES_PATH resources)
  set(HELP_HTB_LOCATION onlinehelp.htb)
endif(DEVELOPMENT_MODE)

OPTION(USE_SPEECH "Build launcher with speech support?" OFF)
OPTION(USE_JOYSTICK "Build launcher with joystick support (currently windows MM only)?" OFF)
OPTION(USE_OPENAL "Build launcher with OpenAL support?" OFF)
if(USE_OPENAL)
  find_package(OpenAL REQUIRED)
  INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR})
endif(USE_OPENAL)


INCLUDE(${wxWidgets_USE_FILE})
if(WIN32) # helper is only needed on windows
add_executable(registry_helper WIN32
  code/Win32ProfileManager.cpp platform/win32/registry_helper.cpp)
SET_TARGET_PROPERTIES(registry_helper
  PROPERTIES LINKER_LANGUAGE CXX)
  SET_TARGET_PROPERTIES(registry_helper PROPERTIES LINK_FLAGS /MANIFESTUAC:NO)
TARGET_LINK_LIBRARIES(registry_helper ${wxWidgets_LIBRARIES})
endif()

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated/version_strings.cpp
  COMMAND ${PYTHON_EXECUTABLE} scripts/version.cpp.maker.py build ${CMAKE_CURRENT_BINARY_DIR}/generated/version_strings.cpp ${CMAKE_CURRENT_BINARY_DIR}/version.cpp.maker.temp
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp  ${helphtblocation}
  COMMAND ${PYTHON_EXECUTABLE} scripts/onlinehelpmaker.py build  ${CMAKE_CURRENT_BINARY_DIR}/generated/onlinehelp.htb ${CMAKE_SOURCE_DIR}/onlinehelp ${HELPMAKER_DEBUG} -t ${CMAKE_CURRENT_BINARY_DIR}/onlinehelpmaker -c ${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
file(GLOB_RECURSE helpmaker_temp_files ${CMAKE_CURRENT_BINARY_DIR}/onlinehelpmaker/*.*)
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

# find the runtime files
SET(VC9_RUNTIME_LOCATIONS "C:/Program Files (x86)/Microsoft Visual Studio 9.0/VC/redist/x86/Microsoft.VC90.CRT" "C:/Program Files/Microsoft Visual Studio 9.0/VC/redist/x86/Microsoft.VC90.CRT")
FIND_FILE(VC9_RUNTIME_msvcm90 msvcm90.dll 
  PATHS ${VC9_RUNTIME_LOCATIONS})
INSTALL(FILES ${VC9_RUNTIME_msvcm90} DESTINATION bin)
FIND_FILE(VC9_RUNTIME_msvcp90 msvcp90.dll 
  PATHS ${VC9_RUNTIME_LOCATIONS})
INSTALL(FILES ${VC9_RUNTIME_msvcp90} DESTINATION bin)
FIND_FILE(VC9_RUNTIME_msvcr90 msvcr90.dll 
  PATHS ${VC9_RUNTIME_LOCATIONS})
INSTALL(FILES ${VC9_RUNTIME_msvcr90} DESTINATION bin)
FIND_FILE(VC9_RUNTIME_manifest Microsoft.VC90.CRT.manifest 
  PATHS ${VC9_RUNTIME_LOCATIONS})
INSTALL(FILES ${VC9_RUNTIME_manifest} DESTINATION bin)
  
SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_PACKAGE_NAME wxLauncher for the SCP)
SET(CPACK_PACKAGE_VENDOR wxLauncher Team)
SET(CPACK_PACKAGE_INSTALL_DIRECTORY wxLauncher)
SET(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/License.txt)
SET(CPACK_RESOURCE_FILE_README ${PROJECT_SOURCE_DIR}/ReadMe.txt)
if(DEVELOPMENT_MODE)
  SET(CPACK_PACKAGE_FILE_NAME wxlaucher-development-mode)
else()
SET(CPACK_PACKAGE_FILE_NAME wxlauncher-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
endif()

if( WIN32 AND NOT UNIX )
    SET(CPACK_PACKAGE_EXECUTABLES "wxLauncher" "wxLauncher for the SCP")
endif()
include(CPack)
if(DEVELOPMENT_MODE)
  MESSAGE( "Development mode is set. Do not distribute installer.")
endif()
install(TARGETS wxlauncher RUNTIME DESTINATION bin)
if(WIN32)
install(TARGETS registry_helper RUNTIME DESTINATION bin)
endif()
install(DIRECTORY resources/ DESTINATION resources)
install(FILES ${helphtblocation} DESTINATION .)
