/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2001 The OGRE Team
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

namespace Ogre {

#define DEFAULT_INITIAL_CHARS 12
    //---------------------------------------------------------------------
    String TextAreaGuiElement::msTypeName = "TextArea";
    TextAreaGuiElement::CmdCharHeight TextAreaGuiElement::msCmdCharHeight;
    TextAreaGuiElement::CmdFontName TextAreaGuiElement::msCmdFontName;
    //---------------------------------------------------------------------
    TextAreaGuiElement::TextAreaGuiElement(const String& name)
        : GuiElement(name)
    {
        mTransparent = false;
        mAlignment = Left;

        memset( &mRenderOp, 0, sizeof( mRenderOp ) );
        mRenderOp.operationType = RenderOperation::OT_TRIANGLE_LIST;
        mRenderOp.vertexOptions = RenderOperation::VO_TEXTURE_COORDS;
        mRenderOp.numTextureCoordSets = 1;
        mRenderOp.numTextureDimensions[0] = 2;


        mAllocSize = 0;
        checkMemoryAllocation( DEFAULT_INITIAL_CHARS );

        mCharHeight = 0.02;

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
            
            // 6 verts per char since we're doing tri lists without indexes
            mRenderOp.pVertices = new Real[ numChars * 6 * 3 ];
            mRenderOp.pTexCoords[0] = new Real[ numChars * 6 * 2 ];

            mAllocSize = numChars;
        }

    }

    void TextAreaGuiElement::updateGeometry()
    {
        Real *pVert, *pTex;

        checkMemoryAllocation( mCaption.length() );

        mRenderOp.numVertices = mCaption.length() * 6;

        pVert = mRenderOp.pVertices;
        pTex = mRenderOp.pTexCoords[ 0 ];

        float left = _getDerivedLeft() * 2.0 - 1.0;
        float top = -( (_getDerivedTop() * 2.0 ) - 1.0 );

        // Derive space with from a capital A
        Real spaceWidth = mpFont->getGlyphAspectRatio( 'A' ) * mCharHeight * 2.0;

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
                        len += spaceWidth;
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
                left += spaceWidth;
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
        }

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
        mCharHeight = height;
        updateGeometry();
    }
    Real TextAreaGuiElement::getCharHeight() const
    {
        return mCharHeight;
    }

    //---------------------------------------------------------------------
    TextAreaGuiElement::~TextAreaGuiElement()
    {
        if( mRenderOp.pVertices )
            delete [] mRenderOp.pVertices;

        for( ushort i = 0; i < OGRE_MAX_TEXTURE_LAYERS; i++ )
        {
            if( mRenderOp.pTexCoords[i] )
            {
                delete [] mRenderOp.pTexCoords[i];
            }
        }
    }
    //---------------------------------------------------------------------
    const String& TextAreaGuiElement::getTypeName(void)
    {
        return msTypeName;
    }
    //---------------------------------------------------------------------
    void TextAreaGuiElement::getRenderOperation(RenderOperation& rend)
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

        dict->addParameter(ParameterDef("font_name", 
            "Sets the name of the font to use."
            , PT_STRING),
            &msCmdFontName);
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
}
