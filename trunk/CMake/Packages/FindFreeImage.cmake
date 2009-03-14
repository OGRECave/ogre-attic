# - Try to find FreeImage
# Once done, this will define
#
#  FreeImage_FOUND - system has FreeImage
#  FreeImage_INCLUDE_DIRS - the FreeImage include directories 
#  FreeImage_LIBRARIES - link these to use FreeImage

include(FindPkgMacros)
findpkg_begin(FreeImage)

# construct search paths
set(FreeImage_PREFIX_PATH ${FREEIMAGE_HOME} $ENV{FREEIMAGE_HOME})
create_search_paths(FreeImage)
# redo search if prefix path changed
clear_if_changed(FreeImage_PREFIX_PATH
  FreeImage_LIBRARY_FWK
  FreeImage_LIBRARY_REL
  FreeImage_LIBRARY_DBG
  FreeImage_INCLUDE_DIR
)

set(FreeImage_LIBRARY_NAMES freeimage)
get_debug_names(FreeImage_LIBRARY_NAMES)

use_pkgconfig(FreeImage_PKGC freeimage)

findpkg_framework(FreeImage)

find_path(FreeImage_INCLUDE_DIR NAMES FreeImage.h HINTS ${FreeImage_INC_SEARCH_PATH} ${FreeImage_PKGC_INCLUDE_DIRS})
find_library(FreeImage_LIBRARY_REL NAMES ${FreeImage_LIBRARY_NAMES} HINTS ${FreeImage_LIB_SEARCH_PATH} ${FreeImage_PKGC_LIBRARY_DIRS})
find_library(FreeImage_LIBRARY_DBG NAMES ${FreeImage_LIBRARY_NAMES_DBG} HINTS ${FreeImage_LIB_SEARCH_PATH} ${FreeImage_PKGC_LIBRARY_DIRS})
make_library_set(FreeImage_LIBRARY)

findpkg_finish(FreeImage)

