# Setup the packer

if(WIN32) # and WIN64
  # find the runtime files
  if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(VC9_RUNTIME_LOCATIONS "C:/Program Files (x86)/Microsoft Visual Studio 9.0/VC/redist/Debug_NonRedist/x86/Microsoft.VC90.DebugCRT" "C:/Program Files/Microsoft Visual Studio 9.0/VC/redist/Debug_NonRedist/x86/Microsoft.VC90.DebugCRT")
    find_file(VC9_RUNTIME_msvcm90 msvcm90d.dll 
      PATHS ${VC9_RUNTIME_LOCATIONS}
      NO_DEFAULT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH)
    install(FILES ${VC9_RUNTIME_msvcm90} DESTINATION bin)
    find_file(VC9_RUNTIME_msvcp90 msvcp90d.dll 
      PATHS ${VC9_RUNTIME_LOCATIONS}
      NO_DEFAULT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH)
    install(FILES ${VC9_RUNTIME_msvcp90} DESTINATION bin)
    find_file(VC9_RUNTIME_msvcr90 msvcr90d.dll 
      PATHS ${VC9_RUNTIME_LOCATIONS}
      NO_DEFAULT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH)
    install(FILES ${VC9_RUNTIME_msvcr90} DESTINATION bin)
    find_file(VC9_RUNTIME_manifest Microsoft.VC90.DebugCRT.manifest 
      PATHS ${VC9_RUNTIME_LOCATIONS}
      NO_DEFAULT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH)
    install(FILES ${VC9_RUNTIME_manifest} DESTINATION bin)
    find_file(wxLauncher_pdb wxlauncher.pdb PATHS ${CMAKE_CURRENT_BINARY_DIR})
    install(FILES ${wxLauncher_pdb} DESTINATION bin)
    find_file(registry_helper_pdb registry_helper.pdb PATHS ${CMAKE_CURRENT_BINARY_DIR}
      NO_DEFAULT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH)
    install(FILES ${registry_helper_pdb} DESTINATION bin)
  else()
    set(VC9_RUNTIME_LOCATIONS "C:/Program Files (x86)/Microsoft Visual Studio 9.0/VC/redist/x86/Microsoft.VC90.CRT" "C:/Program Files/Microsoft Visual Studio 9.0/VC/redist/x86/Microsoft.VC90.CRT")
    find_file(VC9_RUNTIME_msvcm90 msvcm90.dll 
      PATHS ${VC9_RUNTIME_LOCATIONS}
      NO_DEFAULT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH)
    install(FILES ${VC9_RUNTIME_msvcm90} DESTINATION bin)
    find_file(VC9_RUNTIME_msvcp90 msvcp90.dll 
      PATHS ${VC9_RUNTIME_LOCATIONS}
      NO_DEFAULT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH)
    install(FILES ${VC9_RUNTIME_msvcp90} DESTINATION bin)
    find_file(VC9_RUNTIME_msvcr90 msvcr90.dll 
      PATHS ${VC9_RUNTIME_LOCATIONS}
      NO_DEFAULT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH)
    install(FILES ${VC9_RUNTIME_msvcr90} DESTINATION bin)
    find_file(VC9_RUNTIME_manifest Microsoft.VC90.CRT.manifest 
      PATHS ${VC9_RUNTIME_LOCATIONS}
      NO_DEFAULT_PATH NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH)
    install(FILES ${VC9_RUNTIME_manifest} DESTINATION bin)
  endif()
endif() # WIN32
  
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_PACKAGE_NAME "wxLauncher")
set(CPACK_PACKAGE_VENDOR "wxLauncher Team")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "wxLauncher")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Launcher and installer for the FreeSpace SCP")
set(CPACK_PACKAGE_DESCRIPTION "Launcher and installer for the FreeSpace 2 Open Source Code Project")

# TODO we need separate welcome and readme files,
# and a readme better geared towards end users

if(NOT OSX) # a license agreement popping up whenever people open the DMG will drive them crazy
  set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/License.txt)
endif()

set(CPACK_RESOURCE_FILE_README ${PROJECT_SOURCE_DIR}/ReadMe.txt)
set(CPACK_RESOURCE_FILE_WELCOME ${PROJECT_SOURCE_DIR}/ReadMe.txt)

if(WIN32)
  set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY wxLauncher)
  # the \\\\ is because NSIS does not handle unix paths correctly - http://www.batchmake.org/Wiki/CMake:Packaging_With_CPack
  set(CPACK_NSIS_MUI_ICON ${PROJECT_SOURCE_DIR}/platform/win32\\\\wxlauncher.ico)
  set(CPACK_NSIS_MUI_UNIICON ${PROJECT_SOURCE_DIR}/platform/win32\\\\wxlauncher.ico)
  # the header image *must* be a bitmap
  set(CPACK_PACKAGE_ICON ${PROJECT_SOURCE_DIR}/platform/win32\\\\installer_header.bmp)
  set(CPACK_NSIS_HELP_LINK http::\\\\\\\\wxlauncher.googlecode.com)
  set(CPACK_NSIS_URL_INFO_ABOUT http::\\\\\\\\www.hard-light.net)
endif()

if(DEVELOPMENT_MODE)
  set(CPACK_PACKAGE_FILE_NAME wxlaucher-development-mode-no-distribution)
else()
  if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CPACK_PACKAGE_FILE_NAME wxlauncher-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-debug-withpdbs)
  elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(CPACK_PACKAGE_FILE_NAME wxlauncher-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-withpdbs)
  else()
    set(CPACK_PACKAGE_FILE_NAME wxlauncher-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
  endif()
endif()

if( WIN32 AND NOT UNIX )
    set(CPACK_PACKAGE_EXECUTABLES "wxLauncher" "wxLauncher for the SCP")
endif()

if(OSX)
  set(MACOSX_BUNDLE_ICON_FILE wxlauncher.icns)
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
  set(MACOSX_BUNDLE_COPYRIGHT "Copyright © 2009-2011 ${CPACK_PACKAGE_VENDOR}")
endif(OSX)

if(UNIX AND NOT OSX) # i.e., Linux
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Iss Mneur <iss.mneur@telus.net>")
  set(CPACK_PACKAGE_DEPENDS "libwxgtk2.8 libopenal1")
  set(CPACK_DEBIAN_PACKAGE_SECTION "Games")
endif()

if(WIN32)
  set(CPACK_BINARY_NSIS "ON")
  option(BUILD_BINARY_ZIP "Generate a binary zip" OFF)
  if(BUILD_BINARY_ZIP)
    set(CPACK_BINARY_ZIP "ON")
  else()
    set(CPACK_BINARY_ZIP "OFF")
  endif()
elseif(OSX)
  set(CPACK_BINARY_DRAGNDROP "ON")
  set(CPACK_BINARY_PACKAGEMAKER "OFF")
  set(CPACK_BINARY_STGZ "OFF")
  set(CPACK_BINARY_TZ "OFF")
  set(CPACK_BINARY_TGZ "OFF")
else()
  set(CPACK_BINARY_DEB "ON")
  find_program(rpmbuild_exists rpmbuild)
  if(rpmbuild_exists)
    set(CPACK_BINARY_RPM "ON")
  else()
    set(CPACK_BINARY_RPM "OFF")
    message(STATUS "rpmbuild does not exist, will not build RPM")    
  endif()
  set(CPACK_BINARY_STGZ "OFF")
  set(CPACK_BINARY_TZ "OFF")
  set(CPACK_BINARY_TGZ "OFF")
endif()

include(CPack)

if(WIN32)
  install(TARGETS registry_helper RUNTIME DESTINATION bin)
  install(TARGETS wxlauncher RUNTIME DESTINATION bin)
elseif(OSX)
  install(TARGETS wxlauncher 
	RUNTIME DESTINATION .
	BUNDLE DESTINATION .)
else()  
  install(TARGETS wxlauncher 
	RUNTIME DESTINATION local/bin
	BUNDLE DESTINATION bin)
endif()

if(WIN32)
  install(DIRECTORY resources/ DESTINATION resources)
  install(FILES ${helphtblocation} DESTINATION .)
elseif(OSX)
  ### TODO there's probably a better way to add things to the .app, but this works for now
  install(DIRECTORY ${SDL_LIBRARY} DESTINATION wxlauncher.app/Contents/Frameworks)
  file(GLOB resourceFiles "resources/*")
  install(FILES ${resourceFiles} DESTINATION wxlauncher.app/Contents/Resources)
  install(FILES ${helphtblocation} DESTINATION wxlauncher.app/Contents/Resources)
  install(FILES ${PROJECT_SOURCE_DIR}/platform/macosx/wxlauncher.icns DESTINATION wxlauncher.app/Contents/Resources)
  install(FILES ${PROJECT_SOURCE_DIR}/License.txt DESTINATION .)
  install(FILES ${PROJECT_SOURCE_DIR}/GPLv2.txt DESTINATION .)
#  add_custom_target(RemoveAppsLink ALL ${PROJECT_SOURCE_DIR}/platform/macosx/removeAppsLink.sh "${CMAKE_CURRENT_BINARY_DIR}/_CPack_Packages/Darwin/DragNDrop/wxlauncher-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}" VERBATIM) # attempt to remove link to Applications folder, will keep this as placeholder until I come up with a better idea -- maybe a postinstall script to delete the generated DMG, remove the Applications link, then create a new DMG with hdiutil? 
else()
  install(DIRECTORY resources/ DESTINATION local/share/wxlauncher)
  install(FILES ${helphtblocation} DESTINATION local/share/wxlauncher)
endif()
