/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
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

#ifndef _TTYGuiElement_H__
#define _TTYGuiElement_H__

#include <deque>
#include "OgreEventListeners.h"
#include "OgreGuiElementPrerequisites.h"
#include "OgreGuiContainer.h"
#include "OgreMaterial.h"
#include "OgreStringConverter.h"
#include "OgreGuiElementFactory.h"
#include "OgreFont.h"
#include "OgreFontManager.h"
#include "OgreScrollBarGuiElement.h"

namespace Ogre
{
    /** GuiElement representing a TTY style text area.
    @remarks
    This class subclasses GuiElement and provides a TTY style interface for displaying textual
    data.  Text will wrap when it exceeds the window width and scroll when the window height is
    exceeded.  In addition, each block of text can have it's own colour associated with it.
    @par
    The setCaption method is depreciated in this class.
    */
    class _OgreGuiElementExport TTYGuiElement : public GuiElement, private ScrollListener
    {
    public:
        /** Constructor. */
        TTYGuiElement(const String& name);
        ~TTYGuiElement();

        /** Initialise */
        void initialise(void);

        /** Appends specified text to end of screen buffer. 
        @remarks
            Depreciated, this setCaption method equivalent to calling
            clearText and appendText.
        */
        void setCaption( const String& caption );
        const String& getCaption() const;

        /** Appends specified text to end of screen buffer. 
        @remarks
            This method is used to append text to the end of the
            screen buffer using the default colours.
        */
        virtual void appendText(const String &text) { appendText(mTopColour, mBottomColour, text); }
        /** Appends specified text to end of screen buffer. 
        @remarks
            This method is used to append text to the end of the
            screen buffer using the specified color for both the top and bottom colors.
        */
        virtual void appendText(const ColourValue& colour, const String &text);
        /** Appends specified text to end of screen buffer. 
        @remarks
            This method is used to append text to the end of the
            screen buffer using the specified colors.
        */
        virtual void appendText(const ColourValue& tColour, const ColourValue& bColour, const String &text);
        /** Appends specified text to end of screen buffer. 
        @remarks
            This method is used to append text to the end of the
            screen buffer using the specified colors.
        */
        virtual void appendText(const RGBA& tColour, const RGBA& bColour, const String &text);

        virtual void clearText();

        virtual void setScrollBar(ScrollBarGuiElement *scrollBar);
        virtual ScrollBarGuiElement* getScrollBar() const { return mScrollBar; }

        virtual void setTextLimit( uint maxChars );
        virtual uint getTextLimit() const { return mMaxChars; }

        virtual void setCharHeight( Real height );
        virtual Real getCharHeight() const;

        virtual void setSpaceWidth( Real width );
        virtual Real getSpaceWidth() const;

        virtual void setFontName( const String& font );
        virtual const String& getFontName() const;

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
        virtual void setColour(const ColourValue& col);

        /** Gets the colour of the text. */
        virtual const ColourValue& getColour(void) const;
        /** Sets the colour of the bottom of the letters.
        @remarks
            By setting a separate top and bottom colour, you
            can create a text area which has a graduated colour
            effect to it.
        */
        virtual void setColourBottom(const ColourValue& col);
        /** Gets the colour of the bottom of the letters. */
        virtual const ColourValue& getColourBottom(void) const;
        /** Sets the colour of the top of the letters.
        @remarks
            By setting a separate top and bottom colour, you
            can create a text area which has a graduated colour
            effect to it.
        */
        virtual void setColourTop(const ColourValue& col);
        /** Gets the colour of the top of the letters. */
        virtual const ColourValue& getColourTop(void) const;

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
        /** Command object for setting the text limit.
                @see ParamCommand
        */
        class CmdTextLimit : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };
        //-----------------------------------------------------------------------------------------
        /** Command object for setting the scrollbar.
                @see ParamCommand
        */
        class CmdScrollBar : public ParamCommand
        {
        public:
            String doGet( const void* target ) const;
            void doSet( void* target, const String& val );
        };


    protected:

        struct TextBlock {
          String text;
          RGBA   topColour;
          RGBA   bottomColour;
          uint   cntLines; // how many screen "lines" this text covers (cached, update if mWidth changes)
          uint   cntFaces; // how many faces (triangles) this text requires (cached, update if text changes)
          Real   begin;    // window location (side to side) where this text begins
          Real   end;      // window location (side to side) where this text ends

          TextBlock(const String &_text, const RGBA &_topColour, const RGBA &_bottomColour)
            :text(_text), topColour(_topColour), bottomColour(_bottomColour),
             cntLines(0), cntFaces(0), begin(0), end(0) {}
        };

        typedef std::deque<TextBlock> TextBlockQueue;

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
        static CmdTextLimit msCmdTextLimit;
        static CmdScrollBar msCmdScrollBar;


        Font *mpFont;
        Real mCharHeight;
        ushort mPixelCharHeight;
        Real mSpaceWidth;
        ushort mPixelSpaceWidth;
        uint mAllocSize;

        /// Colours to use for the vertices
        ColourValue mColourBottom;
        ColourValue mColourTop;
        RGBA mTopColour;    // cached
        RGBA mBottomColour; // cached

        uint mScrLines;                 // number of screen lines (update if mHeight changes)

          // text info
        bool mUpdateGeometry;           // whether or not geometry updates are enabled
        bool mUpdateGeometryNotVisible; // whether or not to update geometry if not visible
        TextBlockQueue mTextBlockQueue; // all text blocks (in order received)
        uint mTtlChars;                 // total number of chars stored
        uint mMaxChars;                 // a soft limit on the number of chars to store
        uint mTtlFaces;                 // total number of faces (triangles) for all text
        uint mTtlLines;                 // total number of screen lines covered by all of the text
        uint mTopLine;                  // top line
        bool mAutoScroll;

        ScrollBarGuiElement *mScrollBar;


        /// Internal methods to update geometry
        void checkAndSetUpdateGeometry();
        void pruneText();
        void updateScrollBar();
        void updateTextGeometry(TextBlock &text, Real lineWidth = 0.0);
        void updateTextGeometry();
        void updateWindowGeometry();

        /// Internal method to allocate memory, only reallocates when necessary
        void checkMemoryAllocation( uint numChar );

        /// Inherited function
    		virtual void scrollPerformed(ScrollEvent* e);
        virtual void updatePositionGeometry();
    };
}

#endif

