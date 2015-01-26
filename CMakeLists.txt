# let's go with lower case for commands (or at least be consistent)
# arbitrarily following http://techbase.kde.org/Policies/CMake_Coding_Style#Upper.2Flower_casing

cmake_minimum_required(VERSION 2.8)
# Continue to support compatiable interface for _DEBUG on targets
# see: http://www.cmake.org/cmake/help/v3.0/policy/CMP0043.html
cmake_policy(SET CMP0043 OLD)

# We don't use C but CMake on linux complains if we don't have it enabled
project(wxlauncher CXX C) 

set(VERSION_MAJOR 0)
set(VERSION_MINOR 10)
set(VERSION_PATCH 0)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake/include")

set(CMAKE_OSX_ARCHITECTURES i386)

if(NOT(DEFINED IS_WIN32 OR DEFINED IS_LINUX OR DEFINED IS_APPLE))
  if(WIN32)
    set(IS_WIN32 TRUE)
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(IS_APPLE TRUE)
  else()
    set(IS_LINUX TRUE)
  endif()
endif()

find_package(wxWidgets 2.8.10
  COMPONENTS base core net xml html adv qa richtext)
if(NOT wxWidgets_FOUND)
  find_package(wxWidgets 3.0.2
    COMPONENTS base core net xml html adv qa richtext)
endif()
  
if(NOT PYTHON_EXECUTABLE)
  # Only try to find python iff not already given
  include(FindPythonInterp) #PYTHON_EXECUTABLE
endif()

find_program(GIT_EXECUTABLE git)

option(onlinehelpmaker_debug "Have the onlinehelpmaker.py script output debug information" OFF)
if(onlinehelpmaker_debug)
  set(HELPMAKER_DEBUG "-d")
  set(HELPMAKER_QUIET "")
else(onlinehelpmaker_debug)
  set(HELPMAKER_DEBUG "")
  option(onlinehelpmaker_quiet "Have onlinehelpmaker.py produce even less output than normal" ON)
  if(onlinehelpmaker_quiet)
    set(HELPMAKER_QUIET "-q")
  else()
    set(HELPMAKER_QUIET "")
  endif()
endif(onlinehelpmaker_debug)

# This needs to be set so that VS2008 will link against the most recent
# version of the CRT, the one that is being distrubuted in the installer
if(IS_WIN32)
  add_definitions("/D_BIND_TO_CURRENT_CRT_VERSION=1")
  add_definitions("/D_CRT_SECURE_NO_WARNINGS")
endif(IS_WIN32)

set(helphtblocation ${CMAKE_CURRENT_BINARY_DIR}/generated/onlinehelp.htb)
option(DEVELOPMENT_MODE "Doing development (cannot build installer in this mode)" OFF)
if(DEVELOPMENT_MODE)
  set(HELP_HTB_LOCATION ${helphtblocation})
  set(RESOURCES_PATH ${PROJECT_SOURCE_DIR}/resources)
else(DEVELOPMENT_MODE)
  if (NOT DEFINED RESOURCES_PATH)
    if(IS_WIN32)
      set(RESOURCES_PATH resources)
    elseif(IS_APPLE)
      set(RESOURCES_PATH wxlauncher.app/Contents/Resources)
    else()
      set(RESOURCES_PATH /usr/local/share/wxlauncher/)
    endif()
  endif()
  set(HELP_HTB_LOCATION ${RESOURCES_PATH}/onlinehelp.htb)
endif(DEVELOPMENT_MODE)

option(PROFILE_DEBUGGING "Extra verbose debug logs that include snapshots of profile contents at important steps while auto-save is off" OFF)
option(USE_SPEECH "Build launcher with speech support?" OFF)
option(USE_JOYSTICK "Build launcher with joystick support?" ON)
option(USE_OPENAL "Build launcher with OpenAL support?" ON)
if(USE_OPENAL)
  find_package(OpenAL REQUIRED)
  include_directories(${OPENAL_INCLUDE_DIR})
endif(USE_OPENAL)

if(UNIX)
  find_package(SDL REQUIRED)
  include_directories(${SDL_INCLUDE_DIR})
  set(HAS_SDL true)
endif()
if(UNIX AND NOT DEFINED PLATFORM_HAS_BROKEN_OPENAL)
  set(PLATFORM_HAS_BROKEN_OPENAL TRUE)
endif()

if(IS_APPLE)
  option(USING_SDL_FRAMEWORK "Check this if building with an SDL framework." ON)
endif()

# Make PLATFORM_USES_REGISTRY true only on windows, so that wxLauncher
# will not try to compile the registry code on non Win32 systems.
# Allow the user to define this as false even on windows if they want
if((IS_WIN32) AND (NOT DEFINED PLATFORM_USES_REGISTRY))
  set(PLATFORM_USES_REGISTRY TRUE)
endif()

include(${wxWidgets_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/code)

if(IS_WIN32) # helper is only needed on windows
  add_executable(registry_helper WIN32
    code/global/ids.h
    code/global/BasicDefaults.h
    code/global/BasicDefaults.cpp
    code/global/ModDefaults.h
    code/global/ModDefaults.cpp
    code/global/ModIniKeys.h
    code/global/ModIniKeys.cpp
    code/global/ProfileKeys.h
    code/global/ProfileKeys.cpp
    code/global/RegistryKeys.h
    code/global/RegistryKeys.cpp
    code/global/SkinDefaults.h
    code/global/SkinDefaults.cpp
    code/global/Utils.h
    code/global/Utils.cpp
    code/apis/RegistryProfileManager.cpp
    platform/win32/registry_helper.cpp
    code/apis/PlatformProfileManagerShared.cpp
    code/apis/CmdLineManager.h
    code/apis/CmdLineManager.cpp
    code/apis/EventHandlers.h
    code/apis/EventHandlers.cpp
    code/apis/FlagListManager.h
    code/apis/FlagListManager.cpp
    code/apis/ProfileManager.h
    code/apis/ProfileManager.cpp
    code/apis/ProfileProxy.h
    code/apis/ProfileProxy.cpp
    code/apis/SkinManager.h
    code/apis/SkinManager.cpp
    code/apis/TCManager.h
    code/apis/TCManager.cpp
    code/controls/LightingPresets.h
    code/controls/LightingPresets.cpp
    code/controls/ModList.h
    code/controls/ModList.cpp
    code/datastructures/FlagFileData.h
    code/datastructures/FlagFileData.cpp
    code/datastructures/FSOExecutable.h
    code/datastructures/FSOExecutable.cpp
    code/datastructures/NewsSource.h
    code/datastructures/NewsSource.cpp
    )
  set_target_properties(registry_helper
    PROPERTIES LINKER_LANGUAGE CXX
    LINK_FLAGS /MANIFESTUAC:NO
    COMPILE_DEFINITIONS REGISTRY_HELPER
    )
  target_link_libraries(registry_helper ${wxWidgets_LIBRARIES})
endif()

# Check to see if we have a GIT executable so that we will be able to
# generate the version.cpp.  If we don't have a hg binary, notify and
# create the target to copy a template version.
if(GIT_EXECUTABLE)
  add_custom_target(version.cpp.maker ALL
    COMMAND ${PYTHON_EXECUTABLE} scripts/version.cpp.maker.py build ${CMAKE_CURRENT_BINARY_DIR}/generated/version_strings.cpp ${CMAKE_CURRENT_BINARY_DIR}/version.cpp.maker.temp --gitpath=${GIT_EXECUTABLE}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
else()
  add_custom_target(version.cpp.maker ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/code/global/version_strings.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/generated/version_strings.cpp
    )
  message( "Cannot find a commandline Mercurial client (hg).  version_strings.cpp will be generated by copying a dummy file.") 
endif()

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/generated/version_strings.cpp)

add_custom_target(helpmaker ALL
  COMMAND ${PYTHON_EXECUTABLE} scripts/onlinehelpmaker.py build  ${helphtblocation} ${CMAKE_SOURCE_DIR}/onlinehelp ${HELPMAKER_QUIET} ${HELPMAKER_DEBUG} -t ${CMAKE_CURRENT_BINARY_DIR}/onlinehelpmaker -c ${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${helphtblocation})
file(GLOB_RECURSE helpmaker_temp_files ${CMAKE_CURRENT_BINARY_DIR}/onlinehelpmaker/*.*)

# Call out the files that are built a part of the build
# The HTBs do not need to be run through a complier
set_source_files_properties(
	${helphtblocation}
	PROPERTIES GENERATED true EXTERNAL_OBJECT true)
set_source_files_properties(
	${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp
	${CMAKE_CURRENT_BINARY_DIR}/generated/version_strings.cpp
	PROPERTIES GENERATED true)

set(TAB_CODE_FILES
  code/tabs/AdvSettingsPage.h
  code/tabs/AdvSettingsPage.cpp
  code/tabs/BasicSettingsPage.h
  code/tabs/BasicSettingsPage.cpp
  code/tabs/InstallPage.h
  code/tabs/InstallPage.cpp
  code/tabs/ModsPage.h
  code/tabs/ModsPage.cpp
  code/tabs/WelcomePage.h
  code/tabs/WelcomePage.cpp
  )
source_group(Tabs FILES ${TAB_CODE_FILES})
set(GUI_CONTROL_CODE_FILES
  code/controls/BottomButtons.h
  code/controls/BottomButtons.cpp
  code/controls/FlagListBox.h
  code/controls/FlagListBox.cpp
  code/controls/LightingPresets.h
  code/controls/LightingPresets.cpp
  code/controls/Logger.h
  code/controls/Logger.cpp
  code/controls/ModList.h
  code/controls/ModList.cpp
  code/controls/StatusBar.h
  code/controls/StatusBar.cpp
  code/controls/TruncatableChoice.h
  code/controls/TruncatableChoice.cpp
  )
source_group("GUI Controls" FILES ${GUI_CONTROL_CODE_FILES})
set(GLOBAL_CODE_FILES
  code/global/configure_launcher.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/generated/configure_launcher.h
  code/global/BasicDefaults.h
  code/global/BasicDefaults.cpp
  code/global/ids.h
  code/global/MemoryDebugging.h
  code/global/ModDefaults.h
  code/global/ModDefaults.cpp
  code/global/ModIniKeys.h
  code/global/ModIniKeys.cpp
  code/global/ProfileKeys.h
  code/global/ProfileKeys.cpp
  code/global/RegistryKeys.h
  code/global/RegistryKeys.cpp
  code/global/SkinDefaults.h
  code/global/SkinDefaults.cpp
  code/global/targetver.h
  code/global/Utils.h
  code/global/Utils.cpp
  code/global/version.h
  code/global/version.cpp
  )
source_group(Global FILES ${GLOBAL_CODE_FILES})
set(DATASTRUCTURE_CODE_FILES
  code/datastructures/FlagInfo.cpp
  code/datastructures/FlagFileData.h
  code/datastructures/FlagFileData.cpp
  code/datastructures/FSOExecutable.h
  code/datastructures/FSOExecutable.cpp
  code/datastructures/NewsSource.h
  code/datastructures/NewsSource.cpp
  code/datastructures/ResolutionMap.h
  code/datastructures/ResolutionMap.cpp
  )
source_group("Data Structures" FILES ${DATASTRUCTURE_CODE_FILES})
set(API_CODE_FILES
  code/apis/CmdLineManager.h
  code/apis/CmdLineManager.cpp
  code/apis/EventHandlers.h
  code/apis/EventHandlers.cpp
  code/apis/FlagListManager.h
  code/apis/FlagListManager.cpp
  code/apis/FREDManager.h
  code/apis/FREDManager.cpp
  code/apis/HelpManager.h
  code/apis/HelpManager.cpp
  code/apis/JoystickManager.h
  code/apis/JoystickManager.cpp
  code/apis/OpenALManager.h
  code/apis/OpenALManager.cpp
  code/apis/ProfileManager.h
  code/apis/ProfileManagerOperator.h
  code/apis/ProfileManager.cpp
  code/apis/ProfileManagerOperator.cpp
  code/apis/ProfileProxy.h
  code/apis/ProfileProxy.cpp
  code/apis/SkinManager.h
  code/apis/SkinManager.cpp
  code/apis/SpeechManager.h
  code/apis/SpeechManager.cpp
  code/apis/TCManager.h
  code/apis/TCManager.cpp
  code/apis/PlatformProfileManager.h
  code/apis/RegistryProfileManager.cpp
  code/apis/FileProfileManager.cpp
  code/apis/PlatformProfileManagerShared.cpp
  )
source_group(Apis FILES ${API_CODE_FILES})
set(RESOURCE_FILES
  platform/win32/wxlauncher.rc
  ${CMAKE_CURRENT_BINARY_DIR}/generated/version_strings.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp
  )
source_group(Resources FILES ${RESOURCE_FILES})
set(CODE_FILES
  code/MainWindow.h
  code/MainWindow.cpp
  code/wxLauncherApp.h
  code/wxLauncherApp.cpp
  )
source_group("Main Code Files" FILES ${CODE_FILES})

add_executable(wxlauncher WIN32 MACOSX_BUNDLE
  ${TAB_CODE_FILES}
  ${GUI_CONTROL_CODE_FILES}
  ${GLOBAL_CODE_FILES}
  ${DATASTRUCTURE_CODE_FILES}
  ${API_CODE_FILES}
  ${RESOURCE_FILES}
  ${CODE_FILES}
  )
set_target_properties(wxlauncher
  PROPERTIES LINKER_LANGUAGE CXX)
# Files that are not to be compiled directly
set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/generated/helplinks.cpp   
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/onlinehelp.htb 
  ${CMAKE_CURRENT_BINARY_DIR}/generated/configure_launcher.h 
  code/datastructures/FlagInfo.cpp
  code/global/configure_launcher.h.in
  PROPERTIES HEADER_FILE_ONLY true)

add_dependencies(wxlauncher helpmaker version.cpp.maker)
if(IS_WIN32)
  add_dependencies(wxlauncher registry_helper)
endif(IS_WIN32)

configure_file(${CMAKE_SOURCE_DIR}/code/global/configure_launcher.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/generated/configure_launcher.h)

foreach(temp_file ${helpmaker_temp_files})
  set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${temp_file})
endforeach(temp_file)

target_link_libraries(wxlauncher ${wxWidgets_LIBRARIES} ${SDL_LIBRARY})

# adapted from http://www.cmake.org/Wiki/CMake_FAQ#How_can_I_apply_resources_on_Mac_OS_X_automatically.3F
# copies necessary resources (and frameworks, if needed) to .app bundle
if(IS_APPLE)
  set(APP_RESOURCES_PATH ${CMAKE_CURRENT_BINARY_DIR}/$(CONFIGURATION)/${RESOURCES_PATH})
  set(APP_FRAMEWORKS_PATH ${CMAKE_CURRENT_BINARY_DIR}/$(CONFIGURATION)/wxlauncher.app/Contents/Frameworks)
  add_custom_command(TARGET wxlauncher POST_BUILD
    COMMAND rm -rf ${APP_RESOURCES_PATH}
    COMMAND rm -rf ${APP_FRAMEWORKS_PATH}
    COMMAND mkdir ${APP_RESOURCES_PATH}
    COMMAND mkdir ${APP_FRAMEWORKS_PATH}
    COMMAND cp ${PROJECT_SOURCE_DIR}/resources/* ${APP_RESOURCES_PATH}
    COMMAND cp ${helphtblocation} ${APP_RESOURCES_PATH}
    COMMAND cp ${PROJECT_SOURCE_DIR}/platform/macosx/wxlauncher.icns ${APP_RESOURCES_PATH})
  if(USING_SDL_FRAMEWORK) # then copy the framework into the app
    add_custom_command(TARGET wxlauncher POST_BUILD
      COMMAND cp -R ${SDL_LIBRARY} ${APP_FRAMEWORKS_PATH})
  endif(USING_SDL_FRAMEWORK)
endif(IS_APPLE)

# packaging
set(LAUNCHER_CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/cmake)

if(DEVELOPMENT_MODE)
  message( "Development mode is set. Building of installers is not allowed.  Set DEVELOPMENT_MODE=OFF to build installers.")
  configure_file(${CMAKE_SOURCE_DIR}/cmake/FailCPack.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/CPackConfig.cmake COPYONLY)
  configure_file(${CMAKE_SOURCE_DIR}/cmake/FailCPack.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/CPackSourceConfig.cmake COPYONLY)
else()
  include(${LAUNCHER_CMAKE_SOURCE_DIR}/wxLauncherInstaller.cmake)
endif()
