#######################################################################
# Find all necessary and optional OGRE dependencies
#######################################################################

# OGRE_DEPENDENCIES_DIR can be used to specify a single base
# folder where the required dependencies may be found.
set(OGRE_DEPENDENCIES_DIR "" CACHE PATH "Path to prebuilt OGRE dependencies")
include(FindPkgMacros)
getenv_path(OGRE_DEPENDENCIES_DIR)
set(OGRE_DEP_SEARCH_PATH 
  ${OGRE_DEPENDENCIES_DIR}
  ${ENV_OGRE_DEPENDENCIES_DIR}
  "${OGRE_BINARY_DIR}/Dependencies"
  "${OGRE_SOURCE_DIR}/Dependencies"
  "${OGRE_BINARY_DIR}/../Dependencies"
  "${OGRE_SOURCE_DIR}/../Dependencies"
)

# Set hardcoded path guesses for various platforms
if (UNIX)
  set(OGRE_DEP_SEARCH_PATH ${OGRE_DEP_SEARCH_PATH} /usr/local)
endif ()

# give guesses as hints to the find_package calls
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${OGRE_DEP_SEARCH_PATH})
set(CMAKE_FRAMEWORK_PATH ${CMAKE_FRAMEWORK_PATH} ${OGRE_DEP_SEARCH_PATH})

# see if there is an install directive somewhere in the dependencies folders
find_file(OGRE_DEP_INSTALL_FILE OgreInstallDependencies.cmake 
  HINTS ${OGRE_DEP_SEARCH_PATH} NO_DEFAULT_PATH)
mark_as_advanced(OGRE_DEP_INSTALL_FILE)
if (OGRE_DEP_INSTALL_FILE)
  include(${OGRE_DEP_INSTALL_FILE})
endif ()


#######################################################################
# Core dependencies
#######################################################################

# Find zlib
find_package(ZLIB)
macro_log_feature(ZLIB_FOUND "zlib" "Simple data compression library" "http://www.zlib.net" TRUE "" "")

# Find zziplib
find_package(ZZip)
macro_log_feature(ZZip_FOUND "zziplib" "Extract data from zip archives" "http://zziplib.sourceforge.net" TRUE "" "")

# Find FreeImage
find_package(FreeImage)
macro_log_feature(FreeImage_FOUND "freeimage" "Support for commonly used graphics image formats" "http://freeimage.sourceforge.net" FALSE "" "")

# Find FreeType
find_package(Freetype)
macro_log_feature(FREETYPE_FOUND "freetype" "Portable font engine" "http://www.freetype.org" TRUE "" "")

# Find X11
if (UNIX)
	find_package(X11)
	macro_log_feature(X11_FOUND "X11" "X Window system" "http://www.x.org" TRUE "" "")
	macro_log_feature(X11_Xt_FOUND "Xt" "X Toolkit" "http://www.x.org" TRUE "" "")
	find_library(XAW_LIBRARY NAMES Xaw Xaw7 PATHS ${DEP_LIB_SEARCH_DIR} ${X11_LIB_SEARCH_PATH})
	macro_log_feature(XAW_LIBRARY "Xaw" "X11 Athena widget set" "http://www.x.org" TRUE "" "")
  mark_as_advanced(XAW_LIBRARY)
endif ()


#######################################################################
# RenderSystem dependencies
#######################################################################

# Find OpenGL
find_package(OpenGL)
macro_log_feature(OPENGL_FOUND "opengl" "Support for the OpenGL render system" "" FALSE "" "")

# Find OpenGLES
find_package(OpenGLES)
macro_log_feature(OPENGLES_FOUND "opengles" "Support for the OpenGL ES render system" "" FALSE "" "")

# Find DirectX
if(WIN32)
	find_package(DirectX)
	macro_log_feature(DirectX_FOUND "DirectX" "Support for the DirectX render system" "http://msdn.microsoft.com/en-us/directx/" FALSE "" "")
endif()

#######################################################################
# Additional features
#######################################################################

# Find Cg
find_package(Cg)
macro_log_feature(Cg_FOUND "cg" "C for graphics shader language" "http://developer.nvidia.com/object/cg_toolkit.html" FALSE "" "")

# Find Boost
# Prefer static linking in all cases
set(Boost_USE_STATIC_LIBS TRUE)
set(Boost_ADDITIONAL_VERSIONS "1.37.0" "1.37" "1.38.0" "1.38")
# Components that need linking (NB does not include heaader-only components like bind)
set(OGRE_BOOST_COMPONENTS thread date_time)
find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} QUIET)
if (!Boost_FOUND)
	# Try dynamic
  set(Boost_USE_STATIC_LIBS FALSE)
	find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} QUIET)
endif()
# Optional Boost libs (Boost_${COMPONENT}_FOUND
macro_log_feature(Boost_FOUND "boost" "Boost (general)" "http://boost.org" FALSE "" "")
macro_log_feature(Boost_THREAD_FOUND "boost-thread" "Used for threading support" "http://boost.org" FALSE "" "")
macro_log_feature(Boost_DATE_TIME_FOUND "boost-date_time" "Used for threading support" "http://boost.org" FALSE "" "")

# POCO
find_package(POCO)
macro_log_feature(POCO_FOUND "POCO" "POCO framework" "http://pocoproject.org/" FALSE "" "")



#######################################################################
# Samples dependencies
#######################################################################

# Find CEGUI
find_package(CEGUI)
macro_log_feature(CEGUI_FOUND "CEGUI" "GUI system used for some of the samples" "http://www.cegui.org.uk" FALSE "" "")

# Find OIS
find_package(OIS)
macro_log_feature(OIS_FOUND "OIS" "Input library needed for the samples" "http://sourceforge.net/projects/wgois" FALSE "" "")

#######################################################################
# Tools
#######################################################################

find_package(Doxygen)
macro_log_feature(DOXYGEN_FOUND "Doxygen" "Tool for building API documentation" "http://doxygen.org" FALSE "" "")

#######################################################################
# Tests
#######################################################################

find_package(CppUnit)
macro_log_feature(CppUnit_FOUND "CppUnit" "Library for performing unit tests" "http://cppunit.sourceforge.net" FALSE "" "")

#######################################################################
# Apple-specific
#######################################################################
if (APPLE)
  find_package(Carbon)
  macro_log_feature(Carbon_FOUND "Carbon" "Carbon" "http://www.apple.com" TRUE "" "")

  find_package(Cocoa)
  macro_log_feature(Cocoa_FOUND "Cocoa" "Cocoa" "http://www.apple.com" TRUE "" "")

  find_package(IOKit)
  macro_log_feature(IOKit_FOUND "IOKit" "IOKit HID framework needed by the samples" "http://www.apple.com" FALSE "" "")
endif(APPLE)

# Display results, terminate if anything required is missing
MACRO_DISPLAY_FEATURE_LOG()


# Add library and include paths from the dependencies
include_directories(
  ${ZLIB_INCLUDE_DIRS}
  ${ZZip_INCLUDE_DIRS}
  ${FreeImage_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIRS}
  ${CEGUI_INCLUDE_DIRS}
  ${OIS_INCLUDE_DIRS}
  ${Cg_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${X11_INCLUDE_DIR}
  ${DirectX_INCLUDE_DIRS}
  ${CppUnit_INCLUDE_DIRS}
  ${Carbon_INCLUDE_DIRS}
  ${Cocoa_INCLUDE_DIRS}
)
link_directories(
  ${OPENGL_LIBRARY_DIRS}
  ${Cg_LIBRARY_DIRS}
  ${Boost_LIBRARY_DIRS}
  ${X11_LIBRARY_DIRS}
  ${DirectX_LIBRARY_DIRS}
  ${CppUnit_LIBRARY_DIRS}
)

