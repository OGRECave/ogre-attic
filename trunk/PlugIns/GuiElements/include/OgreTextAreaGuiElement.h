/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as 
published by the Free Software Foundation; either version 2.1 of the 
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public 
License for more details.

You should have received a copy of the GNU Lesser General Public License 
along with this library; if not, write to the Free Software Foundation, 
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------*/

#ifndef _TextAreaGuiElement_H__
#define _TextAreaGuiElement_H__

#include "OgreGuiElementPrerequisites.h"
#include "OgreGuiContainer.h"
#include "OgreMaterial.h"
#include "OgreStringConverter.h"
#include "OgreGuiElementFactory.h"
#include "OgreFont.h"
#include "OgreFontManager.h"

namespace Ogre
{
    /** GuiElement representing a flat, single-material (or transparent) panel which can contain other elements.
    @remarks
    This class subclasses GuiContainer because it can contain other elements. Like other
    containers, if hidden it's contents are also hidden, if moved it's contents also move etc. 
    The panel itself is a 2D rectangle which is either completely transparent, or is rendered 
    with a single material. The texture(s) on the panel can be tiled depending on your requirements.
    @par
    This component is suitable for backgrounds and grouping other elements. Note that because
    it has a single repeating material it cannot have a discrete border (unless the texture has one and
    the texture is tiled only once). For a bordered panel, see it's subclass BorderTextAreaGuiElement.
    @par
    Note that the material can have all the usual effects applied to it like multiple texture
    layers, scrolling / animated textures etc. For multiple texture layers, you have to set 
    the tiling level for each layer.
    */
    class _OgreGuiElementExport TextAreaGuiElement : public GuiElement
    {
    public:
        enum Alignment
        {
            Left,
            Right,
            Center
        };

    public:
        /** Constructor. */
        TextAreaGuiElement(const String& name);
        virtual ~TextAreaGuiElement();

        virtual void initialise(void);
        void setCaption( const String& caption );
        const String& getCaption() const;

        void setCharHeight( Real height );
        Real getCharHeight() const;

        void setSpaceWidth( Real width );
        Real getSpaceWidth() const;

        void setFontName( const String& font );
        const String& getFontName() const;

        /** See GuiElement. */
        virtual const String& getTypeName(void) const;
        /** See Renderable. */
        void getRenderOperation(RenderOperation& op);
        /** Overridden from GuiElement */
        void setMaterialName(const String& matName);

        /** Sets the colour of the text. 
        @remarks
            This method establishes a constant colour for 
            the entire text. Also see setColourBottom and 
            setColourTop which allow you to set a colour gradient.
        */
        void setColour(const ColourValue& col);

        /** Gets the colour of the text. */
        const ColourValue& getColour(void) const;
        /** Sets the colour of the bottom of the letters.
        @remarks
            By setting a separate top and bottom colour, you
            can create a text area which has a graduated colour
            effect to it.
        */
        void setColourBottom(const ColourValue& col);
        /** Gets the colour of the bottom of the letters. */
        const ColourValue& getColourBottom(void) const;
        /** Sets the colour of the top of the letters.
        @remarks
            By setting a separate top and bottom colour, you
            can create a text area which has a graduated colour
            effect to it.
        */
        void setColourTop(const ColourValue& col);
        /** Gets the colour of the top of the letters. */
        const ColourValue& getColourTop(void) const;

        inline void setAlignment( Alignment a )
        {
            mAlignment = a;
            updateGeometry();
        }
        inline Alignment getAlignment() const
        {
            return mAlignment;
        }

        /** Overridden from GuiElement */
        void setMetricsMode(GuiMetricsMode gmm);

        /** Overridden from GuiElement */
        void _update(void);

        //-----------------------------------------------------------------------------------------
        /** Command object for setting the caption.
                @see ParamCommand
        */
        class CmdCaption : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };
        //-----------------------------------------------------------------------------------------
        /** Command object for setting the char height.
                @see ParamCommand
        */
        class CmdCharHeight : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };
        //-----------------------------------------------------------------------------------------
        /** Command object for setting the width of a space.
                @see ParamCommand
        */
        class CmdSpaceWidth : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };
        //-----------------------------------------------------------------------------------------
        /** Command object for setting the caption.
                @see ParamCommand
        */
        class CmdFontName : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };
        //-----------------------------------------------------------------------------------------
        /** Command object for setting the top colour.
                @see ParamCommand
        */
        class CmdColourTop : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };
        //-----------------------------------------------------------------------------------------
        /** Command object for setting the bottom colour.
                @see ParamCommand
        */
        class CmdColourBottom : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };
        //-----------------------------------------------------------------------------------------
        /** Command object for setting the constant colour.
                @see ParamCommand
        */
        class CmdColour : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };
        //-----------------------------------------------------------------------------------------
        /** Command object for setting the alignment.
                @see ParamCommand
        */
        class CmdAlignment : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };

    protected:
        /// The text alignment
        Alignment mAlignment;

        /// Flag indicating if this panel should be visual or just group things
        bool mTransparent;

        /// Render operation
        RenderOperation mRenderOp;

        /// Method for setting up base parameters for this class
        void addBaseParameters(void);

        static String msTypeName;

        // Command objects
        static CmdCharHeight msCmdCharHeight;
        static CmdSpaceWidth msCmdSpaceWidth;
        static CmdFontName msCmdFontName;
        static CmdColour msCmdColour;
        static CmdColourTop msCmdColourTop;
        static CmdColourBottom msCmdColourBottom;
        static CmdAlignment msCmdAlignment;


        Font *mpFont;
        Real mCharHeight;
        ushort mPixelCharHeight;
        Real mSpaceWidth;
        ushort mPixelSpaceWidth;
        size_t mAllocSize;

        /// Colours to use for the vertices
        ColourValue mColourBottom;
        ColourValue mColourTop;
        bool mColoursChanged;


        /// Internal method to update the geometry
        virtual void updateGeometry();
        /// Internal method to allocate memory, only reallocates when necessary
        void checkMemoryAllocation( size_t numChars );
        /// Inherited function
        virtual void updatePositionGeometry();
        /// Updates vertex colours
        virtual void updateColours(void);
    };
}

#endif

