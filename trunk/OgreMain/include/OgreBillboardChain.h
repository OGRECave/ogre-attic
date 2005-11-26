/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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

// Thanks to Vincent Cantin (karmaGfa) for the original implementation

#ifndef _BillboardChain_H__
#define _BillboardChain_H__

#include "OgrePrerequisites.h"

#include "OgreSimpleRenderable.h"

namespace Ogre {


   /** Contains the data of an element of the BillboardChain.
   */
   class _OgreExport BillboardChainElement
   {

   public:

      BillboardChainElement();

      BillboardChainElement(Vector3 position,
                       Real width,
                       Real uTexCoord,
                       ColourValue colour);

      Vector3 position;
      Real width;
      Real uTexCoord;
      ColourValue colour;

   };

    /** Allows the rendering of a chain of connected billboards.
    */
   class _OgreExport BillboardChain : public SimpleRenderable
   {

   public:
         
      typedef std::vector<BillboardChainElement> BillboardChainElementList;

      BillboardChain(int maxNbChainElements = 10);
      virtual ~BillboardChain();

      virtual void _notifyCurrentCamera(Camera* cam);
      virtual Real getSquaredViewDepth(const Camera* cam) const;
      virtual Real getBoundingRadius(void) const;

      void setNbChainElements(unsigned int nbChainElements);
      void setChainElement(unsigned int elementIndex, const BillboardChainElement& billboardChainElement);
      void updateBoundingBox();

   protected:

      Real mRadius;

      int mCurrentNbChainElements;
      int mMaxNbChainElements;

      BillboardChainElementList mChainElementList;

      void updateHardwareBuffers();
   };

} // namespace

#endif

