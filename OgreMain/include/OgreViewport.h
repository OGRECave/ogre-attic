/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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
#ifndef __Viewport_H__
#define __Viewport_H__

#include "OgrePrerequisites.h"

#include "OgreColourValue.h"

namespace Ogre {
    /** An abstraction of a viewport, i.e. a rendering region on a render
        target.
        @remarks
            A viewport is the meeting of a camera and a rendering surface -
            the camera renders the scene from a viewpoint, and places its
            results into some subset of a rendering target, which may be the
            whole surface or just a part of the surface. Each viewport has a
            single camera as source and a single target as destination. A
            camera only has 1 viewport, but a render target may have several.
            A viewport also has a Z-order, i.e. if there is more than one
            viewport on a single render target and they overlap, one must
            obscure the other in some predetermined way.
    */
    class _OgreExport Viewport
    {
    public:
        /** The usual constructor.
            @param
                cam Pointer to a camera to be the source for the image.
            @param
                target Pointer to the render target to be the destination
                for the rendering.
            @param
                left
            @param
                top
            @param
                width
            @param
                height
                Dimensions of the viewport, expressed a a pecentage between
                0 and 100. This allows the dimensions to apply irrespective of
                changes in the target's size: e.g. to fill the whole area,
                values of 0,0,100,100 are appropriate.
            @param
                ZOrder Relative Z-order on the target. Lower = further to
                the front.
        */
        Viewport(
            Camera* camera,
            RenderTarget* target,
            float left, float top,
            float width, float height,
            int ZOrder);

        /** Default destructor.
        */
        ~Viewport();

        /** Notifies the viewport of a possible change in dimensions.
            @remarks
                Used by the target to update the viewport's dimensions
                (usually the result of a change in target size).
            @note
                Internal use by Ogre only.
        */
        void _updateDimensions(void);

        /** Instructs the viewport to updates its contents.
        */
        void update(void);

        /** Retrieves a pointer to the render target for this viewport.
        */
        RenderTarget* getTarget(void) const;

        /** Retrieves a pointer to the camera for this viewport.
        */
        Camera* getCamera(void) const;

        /** Sets the camera to use for rendering to this viewport. */
        void setCamera(Camera* cam);

        /** Gets one of the relative dimensions of the viewport,
            a value between 0.0 and 1.0.
        */
        float getLeft(void) const;

        /** Gets one of the relative dimensions of the viewport, a value
            between 0.0 and 1.0.
        */
        float getTop(void) const;

        /** Gets one of the relative dimensions of the viewport, a value
            between 0.0 and 1.0.
        */

        float getWidth(void) const;
        /** Gets one of the relative dimensions of the viewport, a value
            between 0.0 and 1.0.
        */

        float getHeight(void) const;
        /** Gets one of the actual dimensions of the viewport, a value in
            pixels.
        */

        int getActualLeft(void) const;
        /** Gets one of the actual dimensions of the viewport, a value in
            pixels.
        */

        int getActualTop(void) const;
        /** Gets one of the actual dimensions of the viewport, a value in
            pixels.
        */
        int getActualWidth(void) const;
        /** Gets one of the actual dimensions of the viewport, a value in
            pixels.
        */

        int getActualHeight(void) const;

        /** Sets the dimensions (after creation).
            @param
                left
            @param
                top
            @param
                width
            @param
                height Dimensions relative to the size of the target,
                represented as real values between 0 and 1. i.e. the full
                target area is 0, 0, 1, 1.
        */
        void setDimensions(float left, float top, float width, float height);

        /** Sets the initial background colour of the viewport (before
            rendering).
        */
        void setBackgroundColour(ColourValue colour);

        /** Gets the background colour.
        */
        const ColourValue& getBackgroundColour(void) const;

        /** Determines whether to clear the viewport before rendering.
            @remarks
                If you expecting every pixel on the viewport to be redrawn
                every frame, you can save a little time by not clearing the
                viewport before every frame. Do so by passing 'false' to this
                method (the default is to clear every frame).
         */
        void setClearEveryFrame(bool clear);

        /** Determines if the viewport is cleared before every frame.
        */
        bool getClearEveryFrame(void) const;

        /** Access to actual dimensions (based on target size).
        */
        void getActualDimensions(
            int &left, int &top, int &width, int &height ) const;

        bool _isUpdated(void) const;
        void _clearUpdatedFlag(void);

        /** Gets the number of rendered faces in the last update.
        */
        unsigned int _getNumRenderedFaces(void) const;

        /** Tells this viewport whether it should display Overlay objects.
        @remarks
            Overlay objects are layers which appear on top of the scene. They are created via
            SceneManager::createOverlay and every viewport displays these by default.
            However, you probably don't want this if you're using multiple viewports,
            because one of them is probably a picture-in-picture which is not supposed to
            have overlays of it's own. In this case you can turn off overlays on this viewport
            by calling this method.
        @param enabled If true, any overlays are displayed, if false they are not.
        */
        void setOverlaysEnabled(bool enabled);

        /** Returns whether or not Overlay objects (created in the SceneManager) are displayed in this
            viewport. */
        bool getOverlaysEnabled(void);


    protected:
        Camera* mCamera;
        RenderTarget* mTarget;
        // Relative dimensions, irrespective of target dimensions (0..1)
        float mRelLeft, mRelTop, mRelWidth, mRelHeight;
        // Actual dimensions, based on target dimensions
        int mActLeft, mActTop, mActWidth, mActHeight;
        /// ZOrder
        int mZOrder;
        /// Background options
        ColourValue mBackColour;
        bool mClearEveryFrame;
        bool mUpdated;
        bool mShowOverlays;
    };

}

#endif
