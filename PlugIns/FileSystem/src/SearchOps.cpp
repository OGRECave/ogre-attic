#include "SearchOps.h"
#include <stdio.h>
#include <ctype.h>

/* Win32 directory operations emulation */
#if OGRE_PLATFORM != PLATFORM_WIN32

#include <OgreNoMemoryMacros.h>

/* If we've initialized yet */
static int G_searches_initialized = 0;

/* The possible searches */
static struct _find_search_t G_find_searches[MAX_FIND_SEARCHES];

long _findfirst(const char *pattern, struct _finddata_t *data)
{
    long find_key = 0;
    
    /* Initialize the system if it's needed */
    if (!G_searches_initialized)
    {
        int x;
        
        for (x = 0; x < MAX_FIND_SEARCHES; x++)
        {
            G_find_searches[x].in_use = 0;
        }

        G_searches_initialized = 1;
    }

    /* See if we have an available search slot */
    for (find_key = 0; find_key < MAX_FIND_SEARCHES; find_key++)
    {
        if (!G_find_searches[find_key].in_use)
            break;
    }

    if (find_key == MAX_FIND_SEARCHES)
    {
        /* uhoh, no more slots available */
        return -1;
    }
    else
    {
        /* We're using the slot */
        G_find_searches[find_key].in_use = 1;
    }

    if ( !(G_find_searches[find_key].dirfd = opendir(".")) )
        return -1;

    /* Hack for *.* from DOS/Windows */
    if (strcmp(pattern, "*.*") == 0)
        G_find_searches[find_key].pattern = strdup("*");
    else
        G_find_searches[find_key].pattern = strdup(pattern);

    /* Get the first entry */
    if (_findnext(find_key, data) < 0)
    {
        data = NULL;
        _findclose(find_key);
        return -1;
    }

    return find_key;
}

int _findnext(long id, struct _finddata_t *data)
{
    struct dirent *entry;
    struct stat stat_buf;

    /* Loop until we run out of entries or find the next one */
    do
    {
        entry = readdir(G_find_searches[id].dirfd);

        if (entry == NULL)
            return -1;

        /* See if the filename matches our pattern */
        if (fnmatch(G_find_searches[id].pattern, entry->d_name, 0) == 0)
            break;
    } while ( entry != NULL );

    data->name = entry->d_name;

    /* Default type to a normal file */
    data->attrib = _A_NORMAL;
    
    /* stat the file to get if it's a subdir */
    stat(data->name, &stat_buf);
    if (S_ISDIR(stat_buf.st_mode))
    {
        data->attrib = _A_SUBDIR;
    }

    return 0;
}

int _findclose(long id)
{
    int ret;
    
    ret = closedir(G_find_searches[id].dirfd);
    free(G_find_searches[id].pattern);
    G_find_searches[id].in_use = 0;

    return ret;
}

#include <OgreMemoryMacros.h>

#endif
