// OgrePostFilterManager
// Manuel Bua
//
// $HeadURL: svn://localhost/OgreDev/testBed/OgrePostFilter/OgrePostFilterObserver.h $
// $Id$

#include "Ogre.h"


// Let's the user be notified of rendering events
class OgrePostFilterObserver {

	public:

		inline virtual void preRenderPostFilterPass( MaterialPtr aMaterialPtr ) {};

};