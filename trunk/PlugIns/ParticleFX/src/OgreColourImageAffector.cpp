/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
	(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under 
the terms of the GNU Lesser General Public License as published by the Free Software 
Foundation; either version 2 of the License, or (at your option) any later 
version.

This program is distributed in the hope that it will be useful, but WITHOUT 
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with 
this program; if not, write to the Free Software Foundation, Inc., 59 Temple 
Place - Suite 330, Boston, MA 02111-1307, USA, or go to 
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreColourImageAffector.h"
#include "OgreParticleSystem.h"
#include "OgreStringConverter.h"
#include "OgreParticle.h"
#include "OgreException.h"


namespace Ogre {
    
    // init statics
	ColourImageAffector::CmdImageAdjust		ColourImageAffector::msImageCmd;

    //-----------------------------------------------------------------------
    ColourImageAffector::ColourImageAffector()
    {
        mType = "ColourImage";

        // Init parameters
        if (createParamDictionary("ColourImageAffector"))
        {
            ParamDictionary* dict = getParamDictionary();

			dict->addParameter(ParameterDef("image", "image where the colours come from", PT_STRING), &msImageCmd);
        }
    }
    //-----------------------------------------------------------------------
    void ColourImageAffector::_initParticle(Particle* pParticle)
	{
		const uchar*		data			= mColourImage.getData();
		const Real			div_255			= 1.0f / 255.f;
		
		pParticle->mColour.r = data[0] * div_255;
		pParticle->mColour.g = data[1] * div_255;
		pParticle->mColour.b = data[2] * div_255;
		pParticle->mColour.a = data[3] * div_255;
    
	}
    //-----------------------------------------------------------------------
    void ColourImageAffector::_affectParticles(ParticleSystem* pSystem, Real timeElapsed)
    {
        Particle*			p;
		ParticleIterator	pi				= pSystem->_getIterator();

		Real				width			= mColourImage.getWidth()  - 1;
		Real				height			= mColourImage.getHeight() - 1;
		const uchar*		data			= mColourImage.getData();

		while (!pi.end())
		{
			p = pi.getNext();
			const Real		life_time		= p->mTotalTimeToLive;
			Real			particle_time	= 1.0f - (p->mTimeToLive / life_time); 

			if (particle_time > 1.0f)
				particle_time = 1.0f;
			if (particle_time < 0.0f)
				particle_time = 0.0f;

			const Real		float_index		= particle_time * width;
			const int		index			= (int)float_index;
			const int		position		= index * 4;
			const Real		div_255			= 1.0f / 255.f;
				
			if (index <= 0 || index >= width)
			{
				p->mColour.r = (data[position + 0] * div_255);
				p->mColour.g = (data[position + 1] * div_255);
				p->mColour.b = (data[position + 2] * div_255);
				p->mColour.a = (data[position + 3] * div_255);
			} else
			{
				const Real		fract		= float_index - (Real)index;
				const Real		to_color	= fract * div_255;
				const Real		from_color	= (div_255 - to_color);

				p->mColour.r = (data[position + 0] * from_color) + (data[position + 4] * to_color);
				p->mColour.g = (data[position + 1] * from_color) + (data[position + 5] * to_color);
				p->mColour.b = (data[position + 2] * from_color) + (data[position + 6] * to_color);
				p->mColour.a = (data[position + 3] * from_color) + (data[position + 7] * to_color);
			}
		}
    }
    
    //-----------------------------------------------------------------------
    void ColourImageAffector::setImageAdjust(String name)
    {
		mColourImageName = name;
		mColourImage.load(name);

		PixelFormat	format = mColourImage.getFormat();

		if ( format != PF_A8R8G8B8 )
		{
			Except( Exception::ERR_INVALIDPARAMS, "Error: Image is not a rgba image.",
					"ColourImageAffector::setImageAdjust" );
		}
	}
    //-----------------------------------------------------------------------
    String ColourImageAffector::getImageAdjust(void)
    {
        return mColourImageName;
    }
    
	
	//-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    // Command objects
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String ColourImageAffector::CmdImageAdjust::doGet(void* target)
    {
        return static_cast<ColourImageAffector*>(target)->getImageAdjust();
    }
    void ColourImageAffector::CmdImageAdjust::doSet(void* target, const String& val)
    {
        static_cast<ColourImageAffector*>(target)->setImageAdjust(val);
    }

}



