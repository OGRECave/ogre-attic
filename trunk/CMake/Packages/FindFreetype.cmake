# - Try to find FreeType
# Once done, this will define
#
#  FREETYPE_FOUND - system has FreeType
#  FREETYPE_INCLUDE_DIRS - the FreeType include directories 
#  FREETYPE_LIBRARIES - link these to use FreeType

include(FindPkgMacros)
findpkg_begin(FREETYPE)

# construct search paths
set(FREETYPE_PREFIX_PATH ${FREETYPE_HOME} $ENV{FREETYPE_HOME})
create_search_paths(FREETYPE)
# redo search if prefix path changed
clear_if_changed(FREETYPE_PREFIX_PATH
  FREETYPE_LIBRARY_FWK
  FREETYPE_LIBRARY_REL
  FREETYPE_LIBRARY_DBG
  FREETYPE_INCLUDE_DIR
)

set(FREETYPE_LIBRARY_NAMES freetype freetype219 freetype235 freetype238)
get_debug_names(FREETYPE_LIBRARY_NAMES)

use_pkgconfig(FREETYPE_PKGC freetype2)

findpkg_framework(FREETYPE)

find_path(FREETYPE_INCLUDE_DIR NAMES freetype/freetype.h HINTS ${FREETYPE_INC_SEARCH_PATH} ${FREETYPE_PKGC_INCLUDE_DIRS} PATH_SUFFIXES freetype2)
find_library(FREETYPE_LIBRARY_REL NAMES ${FREETYPE_LIBRARY_NAMES} HINTS ${FREETYPE_LIB_SEARCH_PATH} ${FREETYPE_PKGC_LIBRARY_DIRS})
find_library(FREETYPE_LIBRARY_DBG NAMES ${FREETYPE_LIBRARY_NAMES_DBG} HINTS ${FREETYPE_LIB_SEARCH_PATH} ${FREETYPE_PKGC_LIBRARY_DIRS})
make_library_set(FREETYPE_LIBRARY)

findpkg_finish(FREETYPE)

