// OgrePostFilterManager
// Manuel Bua
//
// $HeadURL: svn://localhost/OgreDev/testBed/OgrePostFilter/OgrePostFilterObserver.h $
// $Id: OgrePostFilterObserver.h,v 1.1 2005-08-23 15:54:12 miathan6 Exp $

#include "Ogre.h"


// Let's the user be notified of rendering events
class OgrePostFilterObserver {

	public:

		inline virtual void preRenderPostFilterPass( MaterialPtr aMaterialPtr ) {};

};