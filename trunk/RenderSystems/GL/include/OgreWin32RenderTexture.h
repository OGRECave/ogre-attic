/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2004 The OGRE Team
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

#ifndef __Win32RT_H__
#define __Win32RT_H__

#include "OgreWin32Prerequisites.h"
#include "OgreGLTexture.h"


namespace Ogre {
    class Win32RenderTexture : public GLRenderTexture
    {
    public:
		Win32RenderTexture(Win32GLSupport &glsupport, const String & name, 
			unsigned int width, unsigned int height,
			TextureType texType, PixelFormat internalFormat, 
			const NameValuePairList *miscParams );
        ~Win32RenderTexture();
    protected:
        virtual void _copyToTexture();
        /// internal method for firing events
        // virtual void firePreUpdate(void);
        /// internal method for firing events
        // virtual void firePostUpdate(void);

        void createPBuffer();
		void destroyPBuffer();

		Win32GLSupport &mGLSupport;
		HDC		mHDC;
		HGLRC	mGlrc;
		HPBUFFERARB mPBuffer;
        Win32Context *mContext;
    };
}

#endif
