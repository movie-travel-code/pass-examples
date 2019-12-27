cmake_minimum_required(VERSION 3.4.3)

#==============================================================================
# 1. LOAF LLVM CONFIGURATION
#==============================================================================
# Set this to a valid LLVM installation
set(LT_LLVM_INSTALL_DIR "" CAHCE PATH "LLVM installation directory")

# Add the location of LLVMConfig.cmake to CMake search paths(so that
# find_package can locate it)
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm")

find_package(LLVM 10.0.0 REQUIRED CONFIG)

# Non-constant includes headers from LLVM - update the include paths accordingly
include_directories(${LLVM_INCLUDE_DIRS})

#==============================================================================
# 2. BUILD CONFIGURATION
#==============================================================================
# Use the same C++ standard as LLVM does
set(CMAKE_CXX_STANDARD 14 CACHE STRING "")

# LLVM is normally built without RTTI. Be consistent with that.
if (NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

OPTION(LT_LEGACY_SKIP_PIPELINE_REGISTRATION "Don't register with any of the existing pipelines" OFF)
if (LT_LEGACY_SKIP_PIPELINE_REGISTRATION)
  add_definitions(-DLT_LEGACY_SKIP_PIPELINE_REGISTRATIOIN)
endif()

#==============================================================================
# 3. ADD THE TARGET
#==============================================================================
add_library(NonConstant SHARED NonConstant.cpp)

# Allow undefined symbols in shared objects on Darwin (this is the default 
# behavior on Linux)
target_link_libraries(NonConstant
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
