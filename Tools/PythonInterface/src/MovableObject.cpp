#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportMovableObject()
{
    class_<MovableObject, boost::noncopyable>("MovableObject", no_init);
}

