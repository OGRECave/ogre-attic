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
#include "OgreResourceGroupManager.h"

namespace Ogre {
    
    // init statics
	ColourImageAffector::CmdImageAdjust		ColourImageAffector::msImageCmd;

    //-----------------------------------------------------------------------
    ColourImageAffector::ColourImageAffector(ParticleSystem* psys)
        :ParticleAffector(psys)
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
		
		pParticle->colour.r = data[0] * div_255;
		pParticle->colour.g = data[1] * div_255;
		pParticle->colour.b = data[2] * div_255;
		pParticle->colour.a = data[3] * div_255;
    
	}
    //-----------------------------------------------------------------------
    void ColourImageAffector::_affectParticles(ParticleSystem* pSystem, Real timeElapsed)
    {
        Particle*			p;
		ParticleIterator	pi				= pSystem->_getIterator();

		int				   width			= mColourImage.getWidth()  - 1;
		int				   height			= mColourImage.getHeight() - 1;
		const uchar*		data			= mColourImage.getData();
        const Real      div_255         = 1.0f / 255.f;
        
		while (!pi.end())
		{
			p = pi.getNext();
			const Real		life_time		= p->totalTimeToLive;
			Real			particle_time	= 1.0f - (p->timeToLive / life_time); 

			if (particle_time > 1.0f)
				particle_time = 1.0f;
			if (particle_time < 0.0f)
				particle_time = 0.0f;

			const Real		float_index		= particle_time * width;
			const int		index			= (int)float_index;

            if(index < 0)
            {
                mColourImage.getColourAt(&p->colour, 0, 0, 0);
            }
            else if(index >= width) 
            {
                mColourImage.getColourAt(&p->colour, width, 0, 0);
            }
            else
            {
                // Linear interpolation
				const Real		fract		= float_index - (Real)index;
				const Real		to_colour	= fract;
				const Real		from_colour	= 1.0f - to_colour;
             
                ColourValue from,to;
                mColourImage.getColourAt(&from, index, 0, 0);
                mColourImage.getColourAt(&to, index+1, 0, 0);

				p->colour.r = from.r*from_colour + to.r*to_colour;
                p->colour.g = from.g*from_colour + to.g*to_colour;
                p->colour.b = from.b*from_colour + to.b*to_colour;
                p->colour.a = from.a*from_colour + to.a*to_colour;
			}
		}
    }
    
    //-----------------------------------------------------------------------
    void ColourImageAffector::setImageAdjust(String name)
    {
		mColourImageName = name;
        mColourImage.load(name, mParent->getResourceGroupName());

		PixelFormat	format = mColourImage.getFormat();

		if ( !PixelUtil::isAccessible(format) )
		{
			Except( Exception::ERR_INVALIDPARAMS, "Error: Image is not accessible (rgba) image.",
					"ColourImageAffector::setImageAdjust" );
		}

	}
    //-----------------------------------------------------------------------
    String ColourImageAffector::getImageAdjust(void) const
    {
        return mColourImageName;
    }
    
	
	//-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    // Command objects
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String ColourImageAffector::CmdImageAdjust::doGet(const void* target) const
    {
        return static_cast<const ColourImageAffector*>(target)->getImageAdjust();
    }
    void ColourImageAffector::CmdImageAdjust::doSet(void* target, const String& val)
    {
        static_cast<ColourImageAffector*>(target)->setImageAdjust(val);
    }

}



