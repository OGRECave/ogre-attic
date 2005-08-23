// OgrePostFilterManager
// Manuel Bua
//
// $HeadURL: svn://localhost/OgreDev/testBed/OgrePostFilter/OgrePostFilterPass.cpp $
// $Id: OgrePostFilterPass.cpp,v 1.1 2005-08-23 15:54:12 miathan6 Exp $

#include "OgrePostFilterPass.h"

OgrePostFilterPass::OgrePostFilterPass( const String& aMaterialName ) {

	iInputTextures.clear();
	iMaterialName = aMaterialName;
	iOutputTexture = 0;
	iListener = 0;
}


OgrePostFilterPass::~OgrePostFilterPass() {

}