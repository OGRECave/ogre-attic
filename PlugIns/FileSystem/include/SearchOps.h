#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fnmatch.h>
#include <unistd.h>
#include <stdlib.h>

#include "OgrePlatform.h"

#if OGRE_PLATFORM != PLATFORM_WIN32

/* The max number of searches to allow at one time */
#define MAX_FIND_SEARCHES 10

struct _find_search_t
{
    int in_use;
    char *pattern;
    DIR *dirfd;
};

/* Our simplified data entry structure */
struct _finddata_t
{
    char *name;
    int attrib;
};

#define _A_NORMAL 0x00  /* Normalfile-Noread/writerestrictions */
#define _A_RDONLY 0x01  /* Read only file */
#define _A_HIDDEN 0x02  /* Hidden file */
#define _A_SYSTEM 0x04  /* System file */
#define _A_SUBDIR 0x10  /* Subdirectory */
#define _A_ARCH   0x20  /* Archive file */

long _findfirst(const char *pattern, struct _finddata_t *data);
int _findnext(long id, struct _finddata_t *data);
int _findclose(long id);

#endif
