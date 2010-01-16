cmake_minimum_required(VERSION 2.8)
project(wxLauncher cxx)

include(CPack)
find_package(wxWidgets 2.8.10
  REQUIRED)

  
INCLUDE(${wxWidgets_USE_FILE})
add_executable(registry_helper WIN32
  code/Win32ProfileManager.cpp platform/win32/registry_helper.cpp)
SET_TARGET_PROPERTIES(registry_helper
  PROPERTIES LINKER_LANGUAGE CXX)
TARGET_LINK_LIBRARIES(registry_helper ${wxWidgets_LIBRARIES})