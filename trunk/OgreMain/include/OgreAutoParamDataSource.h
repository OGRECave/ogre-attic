/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
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
#ifndef __AutoParamDataSource_H_
#define __AutoParamDataSource_H_

#include "OgrePrerequisites.h"
#include "OgreMatrix4.h"
#include "OgreVector4.h"

namespace Ogre {


    /** This utility class is used to hold the information used to generate the matrices
    and other information required to automatically populate GpuProgramParameters.
    @remarks
        This class exercises a lazy-update scheme in order to avoid having to update all
        the information a GpuProgramParameters class could possibly want all the time. 
        It relies on the SceneManager to update it when the base data has changed, and
        will calculate concatenated matrices etc only when required, passing back precalculated
        matrices when they are requested more than once when the underlying information has
        not altered.
    */
    class _OgreExport AutoParamDataSource
    {
    protected:
        mutable Matrix4 mWorldMatrix[256];
        mutable Matrix4 mWorldViewMatrix;
        mutable Matrix4 mInverseWorldMatrix;
        mutable Matrix4 mInverseWorldViewMatrix;
        mutable Matrix4 mInverseViewMatrix;
        mutable Vector4 mCameraPositionObjectSpace;

        mutable bool mWorldMatrixDirty;
        mutable bool mWorldViewMatrixDirty;
        mutable bool mInverseWorldMatrixDirty;
        mutable bool mInverseWorldViewMatrixDirty;
        mutable bool mInverseViewMatrixDirty;
        mutable bool mCameraPositionObjectSpaceDirty;

        const Renderable* mCurrentRenderable;
        const Camera* mCurrentCamera;

        // TODO: lights

    public:
        AutoParamDataSource();
        ~AutoParamDataSource();
        /** Updates the current renderable */
        void setCurrentRenderable(const Renderable* rend);
        /** Updates the current camera */
        void setCurrentCamera(const Camera* cam);

        const Matrix4& getWorldMatrix(void) const;
        const Matrix4& getViewMatrix(void) const;
        const Matrix4& getWorldViewMatrix(void) const;
        const Matrix4& getInverseWorldMatrix(void) const;
        const Matrix4& getInverseWorldViewMatrix(void) const;
        const Matrix4& getInverseViewMatrix(void) const;
        const Vector4& getCameraPositionObjectSpace(void) const;


    };
}

#endif
