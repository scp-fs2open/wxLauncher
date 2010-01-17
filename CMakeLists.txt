cmake_minimum_required(VERSION 2.8)
project(wxLauncher cxx)

include(CPack)
find_package(wxWidgets 2.8.10
  REQUIRED base core net xml html adv media qa richtext)
include(FindPythonInterp) #PYTHON_EXECUTABLE

  
INCLUDE(${wxWidgets_USE_FILE})
add_executable(registry_helper WIN32
  code/Win32ProfileManager.cpp platform/win32/registry_helper.cpp)
SET_TARGET_PROPERTIES(registry_helper
  PROPERTIES LINKER_LANGUAGE CXX)
TARGET_LINK_LIBRARIES(registry_helper ${wxWidgets_LIBRARIES})

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated/version.cpp
  COMMAND ${PYTHON_EXECUTABLE} scripts/version.cpp.maker.py build ${CMAKE_CURRENT_BINARY_DIR}/generated/version.cpp ${CMAKE_CURRENT_BINARY_DIR}/version.cpp.maker.temp
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp ${CMAKE_CURRENT_BINARY_DIR}/generated/onlinehelp.htb
  COMMAND ${PYTHON_EXECUTABLE} scripts/onlinehelpmaker.py build  ${CMAKE_CURRENT_BINARY_DIR}/generated/onlinehelp.htb ${CMAKE_SOURCE_DIR}/onlinehelp ${HELPMAKER_DEBUG} -t ${CMAKE_CURRENT_BINARY_DIR}/onlinehelpmaker -c ${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  
add_executable(wxlauncher WIN32 MACOSX_BUNDLE
  ${CMAKE_CURRENT_BINARY_DIR}/generated/version.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/generated/onlinehelp.htb
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
  code/wxLauncherApp.cpp )
SET_TARGET_PROPERTIES(wxlauncher
  PROPERTIES LINKER_LANGUAGE CXX)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/onlinehelp.htb
  PROPERTIES HEADER_FILE_ONLY true)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
TARGET_LINK_LIBRARIES(wxlauncher ${wxWidgets_LIBRARIES})