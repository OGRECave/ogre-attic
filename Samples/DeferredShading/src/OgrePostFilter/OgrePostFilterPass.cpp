// OgrePostFilterManager
// Manuel Bua
//
// $HeadURL: svn://localhost/OgreDev/testBed/OgrePostFilter/OgrePostFilterPass.cpp $
// $Id$

#include "OgrePostFilterPass.h"

OgrePostFilterPass::OgrePostFilterPass( const String& aMaterialName ) {

	iInputTextures.clear();
	iMaterialName = aMaterialName;
	iOutputTexture = 0;
	iListener = 0;
}


OgrePostFilterPass::~OgrePostFilterPass() {

}