// OgrePostFilterManager
// Manuel Bua
//
// $HeadURL: svn://localhost/OgreDev/testBed/OgrePostFilter/OgrePostFilterPass.h $
// $Id: OgrePostFilterPass.h,v 1.1 2005-08-23 15:54:12 miathan6 Exp $

#pragma once

#include "Ogre.h"
#include "OgrePostFilterShared.h"
#include "OgreHardwarePixelBuffer.h"
using namespace Ogre;


// forward declaration
class OgrePostFilter;

// Interface class for pass listener
class OgrePostFilterPassListener
{
public:
	virtual ~OgrePostFilterPassListener() { }

	/** Notify that pass is about to be rendered */
	virtual void preUpdateNotify() = 0;
	/** Notify that pass has just been rendered */
	virtual void postUpdateNotify() = 0;
};

class OgrePostFilterPass {

	friend OgrePostFilter;

	public:

		OgrePostFilterPass( const String& aMaterialName );
		virtual ~OgrePostFilterPass();


	public:


		inline String& getMaterialName() {
		
			return iMaterialName;

		}

		// dbg, i'll replace pointers with iterators

		inline std::vector<TexturePtr>* getInputTextures() {

			return &iInputTextures;

		}

		inline RenderTarget *getOutputTexture() {

			return iOutputTexture;

		}


		// can be non inlined

		// a NULL input texture means the original scene will be passed as input
		inline void addInputTexture( TexturePtr &anInputTexture ) {

			iInputTextures.push_back( anInputTexture );

		}

		// set to NULL to use pass output as the final result
		inline void setOutputTexture( RenderTarget *anOutputTexture ) {

			iOutputTexture = anOutputTexture;

		}
		// set to NULL to use pass output as the final result
		inline void setOutputTexture( TexturePtr &anOutputTexture ) {

			iOutputTexture = anOutputTexture->getBuffer()->getRenderTarget();

		}

		void preUpdateNotify()
		{
			if(iListener)
				iListener->preUpdateNotify();
		}
		void postUpdateNotify()
		{
			if(iListener)
				iListener->postUpdateNotify();
		}
		void setListener(OgrePostFilterPassListener* l)
		{
			iListener = l;
		}
	private:

		String iMaterialName;
		RenderTarget *iOutputTexture;
		std::vector<TexturePtr> iInputTextures;
		OgrePostFilterPassListener* iListener;

};
