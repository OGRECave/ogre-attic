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
#ifndef __MaterialSerializer_H__
#define __MaterialSerializer_H__

#include "OgrePrerequisites.h"
#include "OgreMaterial.h"

namespace Ogre {
    /** Class for serializing a Material to a material.script.*/
    class MaterialSerializer
    {
    public:
		MaterialSerializer() {};
		virtual ~MaterialSerializer() {};

		/** Imports a Material from the given Material object.*/
        //void importMaterial(const Material *pMat, bool exportDefaults = false);
        /** Exports a Material to the named material script file. */
        void exportMaterial(const Material *pMat, const String& filename, bool exportDefaults = false);
		/** return a string representing the parsed material */
		const String &getString() const;

    protected:
		void writeMaterial(const Material *pMat);
		void writeTextureLayer(const Material::TextureLayer *pTex);

		void writeSceneBlendFactor(const SceneBlendFactor sbf_src, const SceneBlendFactor sbf_dest);
		void writeSceneBlendFactor(const SceneBlendFactor sbf);
		void writeCompareFunction(const CompareFunction cf);
		void writeColourValue(const ColourValue &colour, bool writeAlpha = false);
		void writeLayerBlendOperationEx(const LayerBlendOperationEx op);
		void writeLayerBlendSource(const LayerBlendSource lbs);
		
		typedef std::multimap<Material::TextureLayer::TextureEffectType, Material::TextureLayer::TextureEffect> EffectMap;

		void writeRotationEffect(const Material::TextureLayer::TextureEffect effect, const Material::TextureLayer *pTex);
		void writeTransformEffect(const Material::TextureLayer::TextureEffect effect, const Material::TextureLayer *pTex);
		void writeScrollEffect(const Material::TextureLayer::TextureEffect effect, const Material::TextureLayer *pTex);
		void writeEnvironmentMapEffect(const Material::TextureLayer::TextureEffect effect, const Material::TextureLayer *pTex);

	private:
		String mBuffer;
		bool mDefaults;

		void beginSection(void)
		{
			mBuffer += "\n{";
		}

		void endSection(void)
		{
			mBuffer += "\n}\n";
		}

		void beginSubSection(void)
		{
			mBuffer += "\n\t{";
		}

		void endSubSection(void)
		{
			mBuffer += "\n\t}";
		}

		void writeAttribute(String att)
		{
			mBuffer += ("\n\t" + att);
		}

		void writeValue(String val)
		{
			mBuffer += (" " + val);
		}

		void writeSubAttribute(String att)
		{
			mBuffer += ("\n\t\t" + att);
		}

		void writeComment(String comment)
		{
			mBuffer += ("\n\t//" + comment);
		}

		void writeSubComment(String comment)
		{
			mBuffer += ("\n\t\t//" + comment);
		}
    };
}
#endif
