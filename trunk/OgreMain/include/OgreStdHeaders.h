#ifndef __StdHeaders_H__
#define __StdHeaders_H__

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdarg>
#include <cmath>

// STL containers
#include <vector>
#include <map>
#include <string>
#include <set>
#include <list>
#include <deque>

// Note - not in the original STL, but exists in SGI STL and STLport
#ifdef EXT_HASH
#   include <ext/hash_map>
#   include <ext/hash_set>
#   include <ext/slist>
#else
#   include <hash_set>
#   include <hash_map>
#   include <slist>
#endif

// STL algorithms & functions
#include <algorithm>
#include <functional>

// C++ Stream stuff
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

#if OGRE_PLATFORM == PLATFORM_WIN32
#   include <direct.h>
#   include <io.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <windows.h>

#undef min
#undef max

#endif

#if OGRE_PLATFORM == PLATFORM_LINUX
#   include <unistd.h>
#   include <dlfcn.h>
#endif

#endif
