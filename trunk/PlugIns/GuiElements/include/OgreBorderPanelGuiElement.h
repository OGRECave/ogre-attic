/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2002 The OGRE Team
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

#ifndef __BorderPanelGuiElement_H__
#define __BorderPanelGuiElement_H__

#include "OgreGuiElementPrerequisites.h"
#include "OgrePanelGuiElement.h"

namespace Ogre {


    
    /** A specialisation of the PanelGuiElement to provide a panel with a border.
    @remarks
        Whilst the standard panel can use a single tiled material, this class allows
        panels with a tileable backdrop plus a border texture. This is handy for large
        panels that are too big to use a single large texture with a border, or
        for multiple different size panels where you want the border a constant width
        but the center to repeat.
    @par
        In addition to the usual PanelGuiElement properties, this class has a 'border
        material', which specifies the material used for the edges of the panel,
        a border width (which can either be constant all the way around, or specified
        per edge), and the texture coordinates for each of the border sections.
    */
    class _OgreGuiElementExport BorderPanelGuiElement : public PanelGuiElement
    {
        friend class BorderRenderable;
    public:
        /** Constructor */
        BorderPanelGuiElement(const String& name);
        ~BorderPanelGuiElement();

        /** Sets the size of the border.
        @remarks
            This method sets a constant size for all borders. There are also alternative
            methods which allow you to set border widths for individual edges separately.
            Remember that the dimensions specified here are in relation to the size of
            the screen, so 0.1 is 1/10th of the screen width or height. Also note that because
            most screen resolutions are 1.333:1 width:height ratio that using the same
            border size will look slightly bigger across than up.
        @param size The size of the border as a factor of the screen dimensions ie 0.2 is one-fifth
            of the screen size.
        */
        void setBorderSize(Real size);

        /** Sets the size of the border, with different sizes for vertical and horizontal borders.
        @remarks
            This method sets a size for the side and top / bottom borders separately. 
            Remember that the dimensions specified here are in relation to the size of
            the screen, so 0.1 is 1/10th of the screen width or height. Also note that because
            most screen resolutions are 1.333:1 width:height ratio that using the same
            border size will look slightly bigger across than up.
        @param sides The size of the side borders as a factor of the screen dimensions ie 0.2 is one-fifth
            of the screen size.
        @param topAndBottom The size of the top and bottom borders as a factor of the screen dimensions.
        */
        void setBorderSize(Real sides, Real topAndBottom);

        /** Sets the size of the border separately for all borders.
        @remarks
            This method sets a size all borders separately. 
            Remember that the dimensions specified here are in relation to the size of
            the screen, so 0.1 is 1/10th of the screen width or height. Also note that because
            most screen resolutions are 1.333:1 width:height ratio that using the same
            border size will look slightly bigger across than up.
        @param left The size of the left border as a factor of the screen dimensions ie 0.2 is one-fifth
            of the screen size.
        @param right The size of the left border as a factor of the screen dimensions.
        @param top The size of the top border as a factor of the screen dimensions.
        @param bottom The size of the bottom border as a factor of the screen dimensions.
        */
        void setBorderSize(Real left, Real right, Real top, Real bottom);

        /** Gets the size of the left border. */
        Real getLeftBorderSize(void);
        /** Gets the size of the right border. */
        Real getRightBorderSize(void);
        /** Gets the size of the top border. */
        Real getTopBorderSize(void);
        /** Gets the size of the bottom border. */
        Real getBottomBorderSize(void);

        /** Sets the texture coordinates for the left edge of the border.
        @remarks
            The border panel uses 8 panels for the border (9 including the center). 
            Imagine a table with 3 rows and 3 columns. The corners are always the same size,
            but the edges stretch depending on how big the panel is. Those who have done
            resizable HTML tables will be familiar with this approach.
        @par
            We only require 2 sets of uv coordinates, one for the top-left and one for the
            bottom-right of the panel, since it is assumed the sections are aligned on the texture.
        */
        void setLeftBorderUV(Real u1, Real v1, Real u2, Real v2);
        /** Sets the texture coordinates for the right edge of the border. 
        @remarks See setLeftBorderUV.
        */
        void setRightBorderUV(Real u1, Real v1, Real u2, Real v2);
        /** Sets the texture coordinates for the top edge of the border. 
        @remarks See setLeftBorderUV.
        */
        void setTopBorderUV(Real u1, Real v1, Real u2, Real v2);
        /** Sets the texture coordinates for the bottom edge of the border. 
        @remarks See setLeftBorderUV.
        */
        void setBottomBorderUV(Real u1, Real v1, Real u2, Real v2);
        /** Sets the texture coordinates for the top-left corner of the border. 
        @remarks See setLeftBorderUV.
        */
        void setTopLeftBorderUV(Real u1, Real v1, Real u2, Real v2);
        /** Sets the texture coordinates for the top-right corner of the border. 
        @remarks See setLeftBorderUV.
        */
        void setTopRightBorderUV(Real u1, Real v1, Real u2, Real v2);
        /** Sets the texture coordinates for the bottom-left corner of the border. 
        @remarks See setLeftBorderUV.
        */
        void setBottomLeftBorderUV(Real u1, Real v1, Real u2, Real v2);
        /** Sets the texture coordinates for the bottom-right corner of the border. 
        @remarks See setLeftBorderUV.
        */
        void setBottomRightBorderUV(Real u1, Real v1, Real u2, Real v2);

        /** Sets the name of the material to use for the borders. */
        void setBorderMaterialName(const String& name);
        /** Gets the name of the material to use for the borders. */
        const String& getBorderMaterialName(void);

        /** Overridden from GuiContainer */
        void _updateRenderQueue(RenderQueue* queue);

        /** Overridden from GuiElement */
        void setMetricsMode(GuiMetricsMode gmm);

        /** Overridden from GuiElement */
        void _update(void);


        /** Command object for specifying border sizes (see ParamCommand).*/
        class CmdBorderSize : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying the Material for the border (see ParamCommand).*/
        class CmdBorderMaterial : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying texture coordinates for the border (see ParamCommand).*/
        class CmdBorderLeftUV : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying texture coordinates for the border (see ParamCommand).*/
        class CmdBorderTopUV : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying texture coordinates for the border (see ParamCommand).*/
        class CmdBorderRightUV : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying texture coordinates for the border (see ParamCommand).*/
        class CmdBorderBottomUV : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying texture coordinates for the border (see ParamCommand).*/
        class CmdBorderTopLeftUV : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying texture coordinates for the border (see ParamCommand).*/
        class CmdBorderBottomLeftUV : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying texture coordinates for the border (see ParamCommand).*/
        class CmdBorderBottomRightUV : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying texture coordinates for the border (see ParamCommand).*/
        class CmdBorderTopRightUV : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
    protected:
        Real mLeftBorderSize;
        Real mRightBorderSize;
        Real mTopBorderSize;
        Real mBottomBorderSize;

        ushort mPixelLeftBorderSize;
        ushort mPixelRightBorderSize;
        ushort mPixelTopBorderSize;
        ushort mPixelBottomBorderSize;

        String mBorderMaterialName;
        Material* mpBorderMaterial;

        // Render operation for the border area
        RenderOperation mRenderOp2;

        static String msTypeName;

        /// internal method for setting up geometry, called by GuiElement::update
        void updatePositionGeometry(void);

        /// Internal method for setting up parameters
        void addBaseParameters(void);

        enum BorderCellIndex {
            BCELL_TOP_LEFT = 0,
            BCELL_TOP = 1,
            BCELL_TOP_RIGHT = 2,
            BCELL_LEFT = 3,
            BCELL_RIGHT = 4,
            BCELL_BOTTOM_LEFT = 5,
            BCELL_BOTTOM = 6,
            BCELL_BOTTOM_RIGHT = 7
        };
        /// Internal method for updating UV
        void setCellUV(BorderCellIndex idx, Real& u1, Real& v1, Real& u2, Real& v2);

        // Command objects
        static CmdBorderSize msCmdBorderSize;
        static CmdBorderMaterial msCmdBorderMaterial;
        static CmdBorderLeftUV msCmdBorderLeftUV;
        static CmdBorderTopUV msCmdBorderTopUV;
        static CmdBorderBottomUV msCmdBorderBottomUV;
        static CmdBorderRightUV msCmdBorderRightUV;
        static CmdBorderTopLeftUV msCmdBorderTopLeftUV;
        static CmdBorderBottomLeftUV msCmdBorderBottomLeftUV;
        static CmdBorderTopRightUV msCmdBorderTopRightUV;
        static CmdBorderBottomRightUV msCmdBorderBottomRightUV;

    protected:
        BorderRenderable* mBorderRenderable;

    };

    /** Class for rendering the border of a BorderPanelGuiElement.
    @remarks
        We need this because we have to render twice, once with the inner panel's repeating
        material (handled by superclass) and once for the border's separate meterial. 
    */
    class _OgreGuiElementExport BorderRenderable : public Renderable
    {
    protected:
        BorderPanelGuiElement* mParent;
    public:
        /** Constructed with pointers to parent. */
        BorderRenderable(BorderPanelGuiElement* parent) : mParent(parent) {}
        Material* getMaterial(void) const { return mParent->mpBorderMaterial; }
        void getRenderOperation(RenderOperation& rend) { rend = mParent->mRenderOp2; }
        void getWorldTransforms(Matrix4* xform) { mParent->getWorldTransforms(xform); }
        unsigned short getNumWorldTransforms(void) { return 1; }
        bool useIdentityProjection(void) { return true; }
        bool useIdentityView(void) { return true; }
    };

}

#endif
