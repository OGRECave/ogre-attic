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

#include "OgreTextAreaGuiElement.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreOverlayManager.h"

namespace Ogre {

#define DEFAULT_INITIAL_CHARS 12
    //---------------------------------------------------------------------
    String TextAreaGuiElement::msTypeName = "TextArea";
    TextAreaGuiElement::CmdCharHeight TextAreaGuiElement::msCmdCharHeight;
    TextAreaGuiElement::CmdSpaceWidth TextAreaGuiElement::msCmdSpaceWidth;
    TextAreaGuiElement::CmdFontName TextAreaGuiElement::msCmdFontName;
    TextAreaGuiElement::CmdColour TextAreaGuiElement::msCmdColour;
    TextAreaGuiElement::CmdColourBottom TextAreaGuiElement::msCmdColourBottom;
    TextAreaGuiElement::CmdColourTop TextAreaGuiElement::msCmdColourTop;
    //---------------------------------------------------------------------
    TextAreaGuiElement::TextAreaGuiElement(const String& name)
        : GuiElement(name)
    {
        mTransparent = false;
        mAlignment = Left;
		mpFont = 0;

        memset( &mRenderOp, 0, sizeof( mRenderOp ) );
        mRenderOp.operationType = LegacyRenderOperation::OT_TRIANGLE_LIST;
        mRenderOp.vertexOptions = LegacyRenderOperation::VO_TEXTURE_COORDS | 
            LegacyRenderOperation::VO_DIFFUSE_COLOURS;
        mRenderOp.numTextureCoordSets = 1;
        mRenderOp.numTextureDimensions[0] = 2;

        mColourTop = ColourValue::White;
        mColourBottom = ColourValue::White;


        mAllocSize = 0;
        checkMemoryAllocation( DEFAULT_INITIAL_CHARS );

        mCharHeight = 0.02;
		mPixelCharHeight = 12;
		mSpaceWidth = 0;
		mPixelSpaceWidth = 0;

        if (createParamDictionary("TextAreaGuiElement"))
        {
            addBaseParameters();
        }
    }

    void TextAreaGuiElement::checkMemoryAllocation( uint numChars )
    {
        if( mAllocSize < numChars)
        {
            if( mRenderOp.pVertices )
                delete [] mRenderOp.pVertices;
            if (mRenderOp.pTexCoords[0])
                delete [] mRenderOp.pTexCoords[0];
            if (mRenderOp.pDiffuseColour)
                delete [] mRenderOp.pDiffuseColour;
            
            // 6 verts per char since we're doing tri lists without indexes
            mRenderOp.pVertices = new Real[ numChars * 6 * 3 ];
            mRenderOp.pTexCoords[0] = new Real[ numChars * 6 * 2 ];
            mRenderOp.pDiffuseColour = new RGBA[numChars * 6];

            mAllocSize = numChars;
        }

    }

    void TextAreaGuiElement::updateGeometry()
    {
        Real *pVert, *pTex;

		if (!mpFont)
		{
			// not initialised yet, probably due to the order of creation in a template
			return;
		}
        size_t charlen = mCaption.size();
        checkMemoryAllocation( charlen );

        mRenderOp.numVertices = charlen * 6;

        pVert = mRenderOp.pVertices;
        pTex = mRenderOp.pTexCoords[ 0 ];

		float largestWidth = 0;
        float left = _getDerivedLeft() * 2.0 - 1.0;
        float top = -( (_getDerivedTop() * 2.0 ) - 1.0 );

        // Derive space with from a capital A
		if (mSpaceWidth == 0)
		{
			mSpaceWidth = mpFont->getGlyphAspectRatio( 'A' ) * mCharHeight * 2.0;
		}

        // Use iterator
        String::iterator i, iend;
        iend = mCaption.end();
        bool newLine = true;
        for( i = mCaption.begin(); i != iend; ++i )
        {
            if( newLine )
            {
                Real len = 0.0f;
                for( String::iterator j = i; j != iend && *j != '\n'; j++ )
                {
                    if (*j == ' ')
                    {
                        len += mSpaceWidth;
                    }
                    else 
                    {
                        len += mpFont->getGlyphAspectRatio( *j ) * mCharHeight * 2.0;
                    }
                }

                if( mAlignment == Right )
                    left -= len;
                else if( mAlignment == Center )
                    left -= len * 0.5;
                
                newLine = false;
            }

            if( *i == '\n' )
            {
                left = _getDerivedLeft() * 2.0 - 1.0;
                top -= mCharHeight * 2.0;
                newLine = true;
                continue;
            }

            if ( *i == ' ')
            {
                // Just leave a gap, no tris
                left += mSpaceWidth;
                // Also reduce tri count
                mRenderOp.numVertices -= 6;
                continue;
            }

            Real horiz_height = mpFont->getGlyphAspectRatio( *i );
            Real u1, u2, v1, v2; 
            mpFont->getGlyphTexCoords( *i, u1, v1, u2, v2 );

            //-------------------------------------------------------------------------------------
            // First tri
            //
            // Upper left
            *pVert++ = left;
            *pVert++ = top;
            *pVert++ = -1.0;

            top -= mCharHeight * 2.0;

            // Bottom left
            *pVert++ = left;
            *pVert++ = top;
            *pVert++ = -1.0;

            top += mCharHeight * 2.0;
            left += horiz_height * mCharHeight * 2.0;

            // Top right
            *pVert++ = left;
            *pVert++ = top;
            *pVert++ = -1.0;
            //-------------------------------------------------------------------------------------

            //-------------------------------------------------------------------------------------
            // Second tri
            //
            // Top right (again)
            *pVert++ = left;
            *pVert++ = top;
            *pVert++ = -1.0;

            top -= mCharHeight * 2.0;
            left -= horiz_height  * mCharHeight * 2.0;

            // Bottom left (again)
            *pVert++ = left;
            *pVert++ = top;
            *pVert++ = -1.0;

            left += horiz_height  * mCharHeight * 2.0;

            // Bottom right
            *pVert++ = left;
            *pVert++ = top;
            *pVert++ = -1.0;
            //-------------------------------------------------------------------------------------

            // Go back up with top
            top += mCharHeight * 2.0;

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

			float currentWidth = (left + 1)/2 - _getDerivedLeft();
			if (currentWidth > largestWidth)
			{
				largestWidth = currentWidth;

			}
        }
		setWidth(largestWidth);
        updateColours();

    }

    void TextAreaGuiElement::updatePositionGeometry()
    {
        updateGeometry();
    }

    void TextAreaGuiElement::setCaption( const String& caption )
    {
        mCaption = caption;
        updateGeometry();

    }
    const String& TextAreaGuiElement::getCaption() const
    {
        return mCaption;
    }

    void TextAreaGuiElement::setFontName( const String& font )
    {
        mpFont = (Font*)FontManager::getSingleton().getByName( font );
        if (!mpFont)
			Except( Exception::ERR_ITEM_NOT_FOUND, "Could not find font " + font,
				"TextAreaGuiElement::setFontName" );
        mpFont->load();
        mpMaterial = mpFont->getMaterial();
        mpMaterial->setDepthCheckEnabled(false);
        mpMaterial->setLightingEnabled(false);

        updateGeometry();
    }
    const String& TextAreaGuiElement::getFontName() const
    {
        return mpFont->getName();
    }

    void TextAreaGuiElement::setCharHeight( Real height )
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
    Real TextAreaGuiElement::getCharHeight() const
    {
        return mCharHeight;
    }

    void TextAreaGuiElement::setSpaceWidth( Real width )
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
    Real TextAreaGuiElement::getSpaceWidth() const
    {
        return mSpaceWidth;
    }

    //---------------------------------------------------------------------
    TextAreaGuiElement::~TextAreaGuiElement()
    {
        if( mRenderOp.pVertices )
            delete [] mRenderOp.pVertices;

        if( mRenderOp.pTexCoords[0] )
            delete [] mRenderOp.pTexCoords[0];
        
        if( mRenderOp.pDiffuseColour )
            delete [] mRenderOp.pDiffuseColour;

    }
    //---------------------------------------------------------------------
    const String& TextAreaGuiElement::getTypeName(void)
    {
        return msTypeName;
    }
    //---------------------------------------------------------------------
    void TextAreaGuiElement::getLegacyRenderOperation(LegacyRenderOperation& rend)
    {
        rend = mRenderOp;
    }
    //---------------------------------------------------------------------
    void TextAreaGuiElement::setMaterialName(const String& matName)
    {
        GuiElement::setMaterialName(matName);
        updateGeometry();
    }
    //---------------------------------------------------------------------
    void TextAreaGuiElement::addBaseParameters(void)
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
    }
    //---------------------------------------------------------------------
    void TextAreaGuiElement::setColour(const ColourValue& col)
    {
        mColourBottom = mColourTop = col;
        updateColours();
    }
    //---------------------------------------------------------------------
    ColourValue TextAreaGuiElement::getColour(void) const
    {
        // Either one
        return mColourTop;
    }
    //---------------------------------------------------------------------
    void TextAreaGuiElement::setColourBottom(const ColourValue& col)
    {
        mColourBottom = col;
        updateColours();
    }
    //---------------------------------------------------------------------
    ColourValue TextAreaGuiElement::getColourBottom(void)
    {
        return mColourBottom;
    }
    //---------------------------------------------------------------------
    void TextAreaGuiElement::setColourTop(const ColourValue& col)
    {
        mColourTop = col;
        updateColours();
    }
    //---------------------------------------------------------------------
    ColourValue TextAreaGuiElement::getColourTop(void)
    {
        return mColourTop;
    }
    //---------------------------------------------------------------------
    void TextAreaGuiElement::updateColours(void)
    {
        // Convert to system-specific
        RGBA topColour, bottomColour;
        Root::getSingleton().convertColourValue(mColourTop, &topColour);
        Root::getSingleton().convertColourValue(mColourBottom, &bottomColour);
        RGBA* pDest = mRenderOp.pDiffuseColour;
        for (uint i = 0; i < mAllocSize; ++i)
        {
            // First tri (top, bottom, top)
            *pDest++ = topColour;
            *pDest++ = bottomColour;
            *pDest++ = topColour;
            // Second tri (top, bottom, bottom)
            *pDest++ = topColour;
            *pDest++ = bottomColour;
            *pDest++ = bottomColour;
        }

    }
    //-----------------------------------------------------------------------
    void TextAreaGuiElement::setMetricsMode(GuiMetricsMode gmm)
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
    void TextAreaGuiElement::_update(void)
    {
        if (mMetricsMode == GMM_PIXELS && 
            (OverlayManager::getSingleton().hasViewportChanged() || mGeomPositionsOutOfDate))
        {
            // Recalc character size
            Real vpHeight;
            vpHeight = (Real) (OverlayManager::getSingleton().getViewportHeight());

            mCharHeight = (Real) mPixelCharHeight / vpHeight;
            mSpaceWidth = (Real) mPixelSpaceWidth / vpHeight;
        }
        GuiElement::_update();
		updateGeometry();

    }
    //---------------------------------------------------------------------------------------------
    // Char height command object
    //
    String TextAreaGuiElement::CmdCharHeight::doGet( void* target )
    {
        return StringConverter::toString( 
            static_cast< TextAreaGuiElement* >( target )->getCharHeight() );
    }
    void TextAreaGuiElement::CmdCharHeight::doSet( void* target, const String& val )
    {
        static_cast< TextAreaGuiElement* >( target )->setCharHeight( 
            StringConverter::parseReal( val ) );
    }
    //---------------------------------------------------------------------------------------------
    // Space width command object
    //
    String TextAreaGuiElement::CmdSpaceWidth::doGet( void* target )
    {
        return StringConverter::toString( 
            static_cast< TextAreaGuiElement* >( target )->getSpaceWidth() );
    }
    void TextAreaGuiElement::CmdSpaceWidth::doSet( void* target, const String& val )
    {
        static_cast< TextAreaGuiElement* >( target )->setSpaceWidth( 
            StringConverter::parseReal( val ) );
    }
    //---------------------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------------------
    // Font name command object
    //
    String TextAreaGuiElement::CmdFontName::doGet( void* target )
    {
        return static_cast< TextAreaGuiElement* >( target )->getFontName();
    }
    void TextAreaGuiElement::CmdFontName::doSet( void* target, const String& val )
    {
        static_cast< TextAreaGuiElement* >( target )->setFontName( val );
    }
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    // Colour command object
    //
    String TextAreaGuiElement::CmdColour::doGet( void* target )
    {
        return StringConverter::toString (
            static_cast< TextAreaGuiElement* >( target )->getColour());
    }
    void TextAreaGuiElement::CmdColour::doSet( void* target, const String& val )
    {
        static_cast< TextAreaGuiElement* >( target )->setColour( 
            StringConverter::parseColourValue(val) );
    }
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    // Top colour command object
    //
    String TextAreaGuiElement::CmdColourTop::doGet( void* target )
    {
        return StringConverter::toString (
            static_cast< TextAreaGuiElement* >( target )->getColourTop());
    }
    void TextAreaGuiElement::CmdColourTop::doSet( void* target, const String& val )
    {
        static_cast< TextAreaGuiElement* >( target )->setColourTop( 
            StringConverter::parseColourValue(val) );
    }
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    // Bottom colour command object
    //
    String TextAreaGuiElement::CmdColourBottom::doGet( void* target )
    {
        return StringConverter::toString (
            static_cast< TextAreaGuiElement* >( target )->getColourBottom());
    }
    void TextAreaGuiElement::CmdColourBottom::doSet( void* target, const String& val )
    {
        static_cast< TextAreaGuiElement* >( target )->setColourBottom( 
            StringConverter::parseColourValue(val) );
    }
    //---------------------------------------------------------------------------------------------
}
