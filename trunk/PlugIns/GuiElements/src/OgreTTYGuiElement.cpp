/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://ogre.sourceforge.net/

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

#include "OgreString.h"
#include "OgreGuiManager.h"
#include "OgreTTYGuiElement.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreOverlayManager.h"

namespace Ogre {

#define DEFAULT_INITIAL_CHARS 12
    //---------------------------------------------------------------------
    String TTYGuiElement::msTypeName = "TTY";
    TTYGuiElement::CmdCharHeight TTYGuiElement::msCmdCharHeight;
    TTYGuiElement::CmdSpaceWidth TTYGuiElement::msCmdSpaceWidth;
    TTYGuiElement::CmdFontName TTYGuiElement::msCmdFontName;
    TTYGuiElement::CmdColour TTYGuiElement::msCmdColour;
    TTYGuiElement::CmdColourBottom TTYGuiElement::msCmdColourBottom;
    TTYGuiElement::CmdColourTop TTYGuiElement::msCmdColourTop;
    TTYGuiElement::CmdScrollBar TTYGuiElement::msCmdScrollBar;
    TTYGuiElement::CmdTextLimit TTYGuiElement::msCmdTextLimit;
    //---------------------------------------------------------------------
    TTYGuiElement::TTYGuiElement(const String& name)
        : GuiElement(name)
    {
		    mpFont = 0;

        memset( &mRenderOp, 0, sizeof( mRenderOp ) );
        mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST;
        mRenderOp.vertexOptions = RenderOperation::VO_TEXTURE_COORDS | 
            RenderOperation::VO_DIFFUSE_COLOURS;
        mRenderOp.numTextureCoordSets = 1;
        mRenderOp.numTextureDimensions[0] = 2;
        mRenderOp.numVertices = 0;

        mColourTop = ColourValue::White;
        mColourBottom = ColourValue::White;
        Root::getSingleton().convertColourValue(mColourTop, &mTopColour);
        Root::getSingleton().convertColourValue(mColourBottom, &mBottomColour);

        mUpdateGeometry = false;
        mUpdateGeometryNotVisible = false;
        mTtlFaces = 0;
        mTtlChars = 0;
        mMaxChars = 2048;
        mTtlLines = 0;
        mTopLine = 0;
        mAutoScroll = true;

        mScrollBar = NULL;

        mAllocSize = 0;
        checkMemoryAllocation( DEFAULT_INITIAL_CHARS );

        mCharHeight = 0.02;
		    mPixelCharHeight = 12;
		    mSpaceWidth = 0;
		    mPixelSpaceWidth = 0;

        mScrLines = mHeight / mCharHeight;

        if (createParamDictionary("TTYGuiElement"))
        {
            addBaseParameters();
        }
    }

    void TTYGuiElement::appendText( const ColourValue &colour, const String& text )
    {
        RGBA sColour;

        Root::getSingleton().convertColourValue(colour, &sColour);

        appendText(sColour, sColour, text);
    }
    
    void TTYGuiElement::appendText( const ColourValue &tColour, const ColourValue& bColour, const String& text )
    {
        RGBA stColour, sbColour;

        Root::getSingleton().convertColourValue(tColour, &stColour);
        Root::getSingleton().convertColourValue(bColour, &sbColour);

        appendText(stColour, sbColour, text);
    }

    void TTYGuiElement::appendText( const RGBA &tColour, const RGBA& bColour, const String& text )
    {
        mTextBlockQueue.push_back(TextBlock(text, tColour, bColour));

        if( mUpdateGeometry )
        {
            TextBlockQueue::reverse_iterator i, j;

            i = mTextBlockQueue.rbegin();
            if( (j = (i + 1)) != mTextBlockQueue.rend() )
              updateTextGeometry(*i, j->end);
            else
              updateTextGeometry(*i);

            mTtlChars += (uint)i->text.size();
            mTtlLines += i->cntLines;
            mTtlFaces += i->cntFaces;

            pruneText();

            if( mAutoScroll && mTtlLines > mScrLines )
                mTopLine = mTtlLines - mScrLines;

            updateScrollBar();
            updateWindowGeometry();
        }
    }

    void TTYGuiElement::clearText()
    {
        mTextBlockQueue.clear();
        mTtlChars = 0;
        mTtlFaces  = 0;
        mTtlLines = 0;
        mTopLine  = 0;
        mAutoScroll = true;

        if( mUpdateGeometry )
        {
            updateScrollBar();
            updateWindowGeometry();
        }
    }

    void TTYGuiElement::setTextLimit( uint maxChars )
    {
        mMaxChars = maxChars;

        pruneText();
        if ( mUpdateGeometry )
        {
            updateScrollBar();
            updateWindowGeometry();
        }
    }

    void TTYGuiElement::setScrollBar(ScrollBarGuiElement *scrollBar)
    {
      if( mScrollBar == scrollBar )
        return;

      if( mScrollBar != NULL )
      {
        mScrollBar->removeScrollListener(this);
      }

      mScrollBar = scrollBar;

      if( mScrollBar != NULL )
      {
          mScrollBar->addScrollListener(this);
          updateScrollBar();
      }
    }

    void TTYGuiElement::scrollPerformed(ScrollEvent* e)
    {
        if( mUpdateGeometry )
        {
          mTopLine = e->getTopVisible();
          updateWindowGeometry();
        }
    }

    void TTYGuiElement::checkMemoryAllocation( uint numFaces )
    {
        if( mAllocSize < numFaces)
        {
            if( mRenderOp.pVertices )
                delete [] mRenderOp.pVertices;
            if (mRenderOp.pTexCoords[0])
                delete [] mRenderOp.pTexCoords[0];
            if (mRenderOp.pDiffuseColour)
                delete [] mRenderOp.pDiffuseColour;
            
            // 6 verts per char since we're doing tri lists without indexes
            mRenderOp.pVertices = new Real[ numFaces * 3 * 3 ];
            mRenderOp.pTexCoords[0] = new Real[ numFaces * 3 * 2 ];
            mRenderOp.pDiffuseColour = new RGBA[numFaces * 3];

            mAllocSize = numFaces;
        }

    }

    void TTYGuiElement::checkAndSetUpdateGeometry()
    {
      if( mpFont != NULL && mHeight >= mCharHeight && mWidth >= mCharHeight )
      {
         //if (mVisible || mUpdateGeometryNotVisible)
            mUpdateGeometry = true;
      }
      else
      {
          mUpdateGeometry = false;
          mRenderOp.numVertices = 0; // FIX ME: don't like this here
      }
    }

    void TTYGuiElement::pruneText()
    {
        TextBlockQueue::iterator i;

        if( (i = mTextBlockQueue.begin()) != mTextBlockQueue.end() )
            while( (mTtlChars - i->text.size()) >= mMaxChars )
            {
                mTtlChars -= (uint)i->text.size();
                mTtlLines -= i->cntLines;
                mTtlFaces -= i->cntFaces;
                i = mTextBlockQueue.erase(i);
            }
    }

    void TTYGuiElement::updateScrollBar()
    {
        if( !mUpdateGeometry || mScrollBar == NULL )
            return;
        mScrollBar->setLimits(mTopLine, (mScrLines <= mTtlLines ? mScrLines : mTtlLines), mTtlLines);          
    }

    void TTYGuiElement::updateTextGeometry(TextBlock &textBlock, Real lineWidth)
    {
        Real lenLine;    // running length of current line
        Real charWidth;  // character width
        String::iterator i, iend;
        uint cntLines;
        uint cntFaces;

        if( !mUpdateGeometry )
          return;

        cntLines = 0;
        cntFaces = 0;
        lenLine  = lineWidth;

        iend = textBlock.text.end();
        for( i = textBlock.text.begin(); i != iend; ++i )
        {
            switch (*i) {
            case '\n' :
              lenLine   = 0;
              charWidth = 0;
              ++cntLines;
              break;

            case ' ' :
              charWidth = mSpaceWidth;
              break;

            default :
              charWidth = mpFont->getGlyphAspectRatio( *i ) * (mCharHeight * 2.0);
              cntFaces += 2;
              break;
            }

            if( (lenLine + charWidth) > (mWidth * 2.0) ) // because of wrapping
            {
                lenLine = 0;
                ++cntLines;
            }

            lenLine += charWidth;
        }

        textBlock.cntLines = cntLines;
        textBlock.cntFaces = cntFaces;
        textBlock.begin    = lineWidth;
        textBlock.end      = lenLine;
    }

    void TTYGuiElement::updateTextGeometry()
    {
        TextBlockQueue::iterator i;
        TextBlockQueue::iterator iend;
        Real lineWidth;

		    if (!mUpdateGeometry)
		    	return;

        lineWidth = 0;
        mTtlChars = 0;
        mTtlLines = 0;
        mTtlFaces = 0;

        iend = mTextBlockQueue.end();
        for ( i = mTextBlockQueue.begin(); i != iend; ++i )
        {
            updateTextGeometry(*i, lineWidth);
            lineWidth = i->end;

            mTtlChars += (uint)i->text.size();
            mTtlLines += i->cntLines;
            mTtlFaces += i->cntFaces;
        }

        pruneText();

        if (mAutoScroll && mTtlLines > mScrLines)
          mTopLine = mTtlLines - mScrLines;
    }

    void TTYGuiElement::updateWindowGeometry()
    {
        Real *pVert;
        Real *pTex;
        RGBA *pDest;
        Real len;    // running length of current line
        uint curLine;
        Real height; // character height
        Real farLeft;
        String::iterator i, iend;
        TextBlockQueue::iterator t;
        TextBlockQueue::iterator tend;

        if (!mUpdateGeometry)
          return;

        checkMemoryAllocation(mTtlFaces);

        height = mCharHeight * 2.0;

        pVert = mRenderOp.pVertices;
        pTex  = mRenderOp.pTexCoords[ 0 ];
        pDest = mRenderOp.pDiffuseColour;

        farLeft = _getDerivedLeft() * 2.0 - 1.0;

        Real left = farLeft;
        Real top  = -( (_getDerivedTop() * 2.0 ) - 1.0 );
        len       = 0;
        curLine   = 0;

        tend = mTextBlockQueue.end();
        for ( t = mTextBlockQueue.begin(); t != tend; ++t )
        {
            if( (curLine + t->cntLines) < mTopLine ) // skip whole text block if not visible
            {
                curLine += t->cntLines;
                continue;
            }

            left = farLeft + t->begin;
            iend = t->text.end();
            for( i = t->text.begin(); i != iend; )
            {
                if( curLine >= (mTopLine + mScrLines) )
                    break;

                if( *i == '\n' )
                {
                    left = farLeft;
                    len  = 0;
                    if (curLine >= mTopLine)
                        top -= height;
                    ++curLine;
                    ++i;
                    continue;
                }

                if ( *i == ' ') // just leave a gap, no triangle
                {
                    left += mSpaceWidth;
                    len  += mSpaceWidth;
                    ++i;
                    continue;
                }

                Real width = mpFont->getGlyphAspectRatio( *i ) * mCharHeight * 2.0;
                if( (len + width) > (mWidth * 2.0) )
                {
                    left = farLeft;
                    len  = 0;
                    if (curLine >= mTopLine)
                      top -= height;
                    ++curLine;
                    continue;
                }

                if( curLine < mTopLine )
                {
                  left += width;
                  len  += width;
                  ++i;
                  continue;
                }

                Real u1, u2, v1, v2; 
                mpFont->getGlyphTexCoords( *i, u1, v1, u2, v2 );

                //-------------------------------------------------------------------------------------
                // First tri
                //
                // Upper left
                *pVert++ = left;
                *pVert++ = top;
                *pVert++ = -1.0;

                // Bottom left
                *pVert++ = left;
                *pVert++ = top - height;
                *pVert++ = -1.0;

                // Top right
                *pVert++ = left + width;
                *pVert++ = top;
                *pVert++ = -1.0;
                //-------------------------------------------------------------------------------------

                //-------------------------------------------------------------------------------------
                // Second tri
                //
                // Top right (again)
                *pVert++ = left + width;
                *pVert++ = top;
                *pVert++ = -1.0;

                // Bottom left (again)
                *pVert++ = left;
                *pVert++ = top - height;
                *pVert++ = -1.0;

                // Bottom right
                *pVert++ = left + width;
                *pVert++ = top - height;
                *pVert++ = -1.0;
                //-------------------------------------------------------------------------------------

                //---------------------------------------------------------------------------------
                // First tri
                //
                *pTex++ = u1;
                *pTex++ = v1;

                *pTex++ = u1;
                *pTex++ = v2;

                *pTex++ = u2;
                *pTex++ = v1;
                //---------------------------------------------------------------------------------

                //---------------------------------------------------------------------------------
                // Second tri
                //
                *pTex++ = u2;
                *pTex++ = v1;

                *pTex++ = u1;
                *pTex++ = v2;

                *pTex++ = u2;
                *pTex++ = v2;
                //---------------------------------------------------------------------------------

                // First tri (top, bottom, top)
                *pDest++ = t->topColour;
                *pDest++ = t->bottomColour;
                *pDest++ = t->topColour;
                // Second tri (top, bottom, bottom)
                *pDest++ = t->topColour;
                *pDest++ = t->bottomColour;
                *pDest++ = t->bottomColour;

                left += width;
                len  += width;
                ++i;
            }
        }

        mRenderOp.numVertices = (pVert - mRenderOp.pVertices) / 3;
    }

    void TTYGuiElement::updatePositionGeometry()
    {
        checkAndSetUpdateGeometry();

        if (mUpdateGeometry)
        {
            mScrLines = mHeight / mCharHeight;
            updateTextGeometry();
            updateScrollBar();
            updateWindowGeometry();
        }
    }
    
    void TTYGuiElement::setCaption( const String& caption )
    {
        mTextBlockQueue.clear();
        mTtlChars = 0;
        mTtlFaces = 0;
        mTtlLines = 0;
        mTopLine = 0;
        mAutoScroll = true;

        appendText(caption);
    }

    const String& TTYGuiElement::getCaption() const
    {
        return mCaption; // FIX ME: not sure what to do here
    }

    void TTYGuiElement::setFontName( const String& font )
    {
        mpFont = (Font*)FontManager::getSingleton().getByName( font );
        mpFont->load();
        mpMaterial = mpFont->getMaterial();
        mpMaterial->setDepthCheckEnabled(false);
        mpMaterial->setLightingEnabled(false);

      	mSpaceWidth = mpFont->getGlyphAspectRatio( 'A' ) * (mCharHeight * 2.0);

        checkAndSetUpdateGeometry();
        updateTextGeometry();
        updateScrollBar();
        updateWindowGeometry();
    }

    const String& TTYGuiElement::getFontName() const
    {
        return mpFont->getName();
    }

    void TTYGuiElement::setCharHeight( Real height )
    {
        if (mMetricsMode == GMM_PIXELS)
        {
            mPixelCharHeight = height;
        }
        else
        {
            mCharHeight = height;
        }
        mGeomPositionsOutOfDate = true;
    }

    Real TTYGuiElement::getCharHeight() const
    {
        return mCharHeight;
    }

    void TTYGuiElement::setSpaceWidth( Real width )
    {
        if (mMetricsMode == GMM_PIXELS)
        {
            mPixelSpaceWidth = width;
        }
        else
        {
            mSpaceWidth = width;
        }

        mGeomPositionsOutOfDate = true;
    }

    Real TTYGuiElement::getSpaceWidth() const
    {
        return mSpaceWidth;
    }

    //---------------------------------------------------------------------
    TTYGuiElement::~TTYGuiElement()
    {
        //if( mScrollBar != NULL )
        //    mScrollBar->removeScrollListener(this);

        if( mRenderOp.pVertices )
            delete [] mRenderOp.pVertices;

        if( mRenderOp.pTexCoords[0] )
            delete [] mRenderOp.pTexCoords[0];
        
        if( mRenderOp.pDiffuseColour )
            delete [] mRenderOp.pDiffuseColour;

    }
    //---------------------------------------------------------------------
    const String& TTYGuiElement::getTypeName(void)
    {
        return msTypeName;
    }
    //---------------------------------------------------------------------
    void TTYGuiElement::getRenderOperation(RenderOperation& rend)
    {
        rend = mRenderOp;
    }
    //---------------------------------------------------------------------
    void TTYGuiElement::setMaterialName(const String& matName)
    {
        GuiElement::setMaterialName(matName);
        updateWindowGeometry();
    }
    //---------------------------------------------------------------------
    void TTYGuiElement::addBaseParameters(void)
    {
        GuiElement::addBaseParameters();
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("char_height", 
            "Sets the height of the characters in relation to the screen."
            , PT_REAL),
            &msCmdCharHeight);

        dict->addParameter(ParameterDef("space_width", 
            "Sets the width of a space in relation to the screen."
            , PT_REAL),
            &msCmdSpaceWidth);

        dict->addParameter(ParameterDef("font_name", 
            "Sets the name of the font to use."
            , PT_STRING),
            &msCmdFontName);

        dict->addParameter(ParameterDef("colour", 
            "Sets the colour of the font (a solid colour)."
            , PT_STRING),
            &msCmdColour);

        dict->addParameter(ParameterDef("colour_bottom", 
            "Sets the colour of the font at the bottom (a gradient colour)."
            , PT_STRING),
            &msCmdColourBottom);

        dict->addParameter(ParameterDef("colour_top", 
            "Sets the colour of the font at the top (a gradient colour)."
            , PT_STRING),
            &msCmdColourTop);

        dict->addParameter(ParameterDef("text_limit", 
            "sets a soft limit on the number stored characters."
            , PT_STRING),
            &msCmdTextLimit);

        dict->addParameter(ParameterDef("scroll_bar", 
            "set the controlling scroll bar."
            , PT_STRING),
            &msCmdScrollBar);
    }
    //---------------------------------------------------------------------
    void TTYGuiElement::setColour(const ColourValue& col)
    {
        mColourBottom = mColourTop = col;

        Root::getSingleton().convertColourValue(mColourTop, &mTopColour);
        mBottomColour = mTopColour;
    }
    //---------------------------------------------------------------------
    ColourValue TTYGuiElement::getColour(void) const
    {
        // Either one
        return mColourTop;
    }
    //---------------------------------------------------------------------
    void TTYGuiElement::setColourBottom(const ColourValue& col)
    {
        mColourBottom = col;
        Root::getSingleton().convertColourValue(mColourBottom, &mBottomColour);
    }
    //---------------------------------------------------------------------
    ColourValue TTYGuiElement::getColourBottom(void)
    {
        return mColourBottom;
    }
    //---------------------------------------------------------------------
    void TTYGuiElement::setColourTop(const ColourValue& col)
    {
        mColourTop = col;
        Root::getSingleton().convertColourValue(mColourTop, &mTopColour);
    }
    //---------------------------------------------------------------------
    ColourValue TTYGuiElement::getColourTop(void)
    {
        return mColourTop;
    }
    //-----------------------------------------------------------------------
    void TTYGuiElement::setMetricsMode(GuiMetricsMode gmm)
    {
        GuiElement::setMetricsMode(gmm);
        if (gmm == GMM_PIXELS)
        {
            // Set pixel variables based on viewport multipliers
            Real vpHeight;
            vpHeight = (Real) (OverlayManager::getSingleton().getViewportHeight());

            mPixelCharHeight = mCharHeight * vpHeight;
            mPixelSpaceWidth = mSpaceWidth * vpHeight;
        }
    }
    //-----------------------------------------------------------------------
    void TTYGuiElement::_update(void)
    {
        if (mMetricsMode == GMM_PIXELS && OverlayManager::getSingleton().hasViewportChanged())
        {
            // Recalc character size
            Real vpHeight;
            vpHeight = (Real) (OverlayManager::getSingleton().getViewportHeight());

            mCharHeight = (Real) mPixelCharHeight / vpHeight;
            mSpaceWidth = (Real) mPixelSpaceWidth / vpHeight;
            updateScrollBar();
            updateWindowGeometry();
        }
        GuiElement::_update();
    }

    //---------------------------------------------------------------------------------------------
    // Char height command object
    //
    String TTYGuiElement::CmdCharHeight::doGet( void* target )
    {
        return StringConverter::toString( 
            static_cast< TTYGuiElement* >( target )->getCharHeight() );
    }
    void TTYGuiElement::CmdCharHeight::doSet( void* target, const String& val )
    {
        static_cast< TTYGuiElement* >( target )->setCharHeight( 
            StringConverter::parseReal( val ) );
    }
    //---------------------------------------------------------------------------------------------
    // Space width command object
    //
    String TTYGuiElement::CmdSpaceWidth::doGet( void* target )
    {
        return StringConverter::toString( 
            static_cast< TTYGuiElement* >( target )->getSpaceWidth() );
    }
    void TTYGuiElement::CmdSpaceWidth::doSet( void* target, const String& val )
    {
        static_cast< TTYGuiElement* >( target )->setSpaceWidth( 
            StringConverter::parseReal( val ) );
    }
    //---------------------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------------------
    // Font name command object
    //
    String TTYGuiElement::CmdFontName::doGet( void* target )
    {
        return static_cast< TTYGuiElement* >( target )->getFontName();
    }
    void TTYGuiElement::CmdFontName::doSet( void* target, const String& val )
    {
        static_cast< TTYGuiElement* >( target )->setFontName( val );
    }
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    // Colour command object
    //
    String TTYGuiElement::CmdColour::doGet( void* target )
    {
        return StringConverter::toString (
            static_cast< TTYGuiElement* >( target )->getColour());
    }
    void TTYGuiElement::CmdColour::doSet( void* target, const String& val )
    {
        static_cast< TTYGuiElement* >( target )->setColour( 
            StringConverter::parseColourValue(val) );
    }
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    // Top colour command object
    //
    String TTYGuiElement::CmdColourTop::doGet( void* target )
    {
        return StringConverter::toString (
            static_cast< TTYGuiElement* >( target )->getColourTop());
    }
    void TTYGuiElement::CmdColourTop::doSet( void* target, const String& val )
    {
        static_cast< TTYGuiElement* >( target )->setColourTop( 
            StringConverter::parseColourValue(val) );
    }
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    // Bottom colour command object
    //
    String TTYGuiElement::CmdColourBottom::doGet( void* target )
    {
        return StringConverter::toString (
            static_cast< TTYGuiElement* >( target )->getColourBottom());
    }
    void TTYGuiElement::CmdColourBottom::doSet( void* target, const String& val )
    {
        static_cast< TTYGuiElement* >( target )->setColourBottom( 
            StringConverter::parseColourValue(val) );
    }
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    // Text limit command object
    //
    String TTYGuiElement::CmdTextLimit::doGet( void* target )
    {
         return StringConverter::toString( 
            static_cast< TTYGuiElement* >( target )->getTextLimit() );

    }
    void TTYGuiElement::CmdTextLimit::doSet( void* target, const String& val )
    {
        static_cast< TTYGuiElement* >( target )->setTextLimit( 
            StringConverter::parseInt( val ) );
    }
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    // ScrollBar command object
    //
    String TTYGuiElement::CmdScrollBar::doGet( void* target )
    {
        ScrollBarGuiElement *scrollBar;
        scrollBar = static_cast< TTYGuiElement* >( target )->getScrollBar();
        return (scrollBar != NULL) ? scrollBar->getName() : "";
    }
    void TTYGuiElement::CmdScrollBar::doSet( void* target, const String& val )
    {
        ScrollBarGuiElement* scrollBar;
        scrollBar = static_cast<ScrollBarGuiElement* >(GuiManager::getSingleton().getGuiElement(val));
        static_cast< TTYGuiElement* >( target )->setScrollBar(scrollBar);
    }
    //---------------------------------------------------------------------------------------------
}
