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
#include "OgreXSIMaterialExporter.h"
#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreTextureUnitState.h"

#include <xsi_shader.h>

namespace Ogre {

	//-------------------------------------------------------------------------
	XsiMaterialExporter::XsiMaterialExporter()
	{

	}
	//-------------------------------------------------------------------------
	XsiMaterialExporter::~XsiMaterialExporter()
	{

	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::exportMaterials(MaterialMap& materials, 
		const String& filename, bool copyTextures)
	{
		LogOgreAndXSI("** Begin OGRE Material Export **");

		mMatSerializer.clearQueue();

		for (MaterialMap::iterator m = materials.begin(); m != materials.end(); ++m)
		{
			exportMaterial(m->second, copyTextures);
		}

		mMatSerializer.exportQueued(filename);

		LogOgreAndXSI("** OGRE Material Export Complete **");
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::exportMaterial(MaterialEntry* matEntry, bool copyTextures)
	{
		LogOgreAndXSI("Exporting " + matEntry->name);

		MaterialPtr mat = MaterialManager::getSingleton().create(matEntry->name,
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Technique* t = mat->createTechnique();
		Pass* firstPass = t->createPass();
		populatePass(firstPass, matEntry->xsiShader, copyTextures);

		XSI::Shader shader(matEntry->xsiShader);
		while (1)
		{
			XSI::CRef source = shader.GetParameter(L"previous").GetSource();
			if(!source.IsValid() || !source.IsA(XSI::siShaderID))
			{
				// finish
				break;
			}
			
			shader = XSI::Shader(source);
			Pass* newPass = t->createPass();
			populatePass(newPass, shader, copyTextures);
		}
		

		mMatSerializer.queueForExport(mat);
		

	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populatePass(Pass* pass, XSI::Shader& xsishader, 
		bool copyTextures)
	{
		XSI::Parameter param = xsishader.GetParameter(L"cullingmode");
		if (param.IsValid())
		{
			short xsiCull = param.GetValue();
			switch (xsiCull)
			{
			case 0:
				pass->setCullingMode(CULL_NONE);
				pass->setManualCullingMode(MANUAL_CULL_NONE);
				break;
			case 1:
				pass->setCullingMode(CULL_CLOCKWISE);
				pass->setManualCullingMode(MANUAL_CULL_BACK);
				break;
			case 2:
				pass->setCullingMode(CULL_ANTICLOCKWISE);
				pass->setManualCullingMode(MANUAL_CULL_FRONT);
				break;
					
			};
		}	

		param = xsishader.GetParameter(L"depthtest");
		if (param.IsValid())
		{
			bool depthTest = param.GetValue();
			pass->setDepthCheckEnabled(depthTest);
		}
		param = xsishader.GetParameter(L"depthwrite");
		if (param.IsValid())
		{
			bool depthWrite = param.GetValue();
			pass->setDepthWriteEnabled(depthWrite);
		}

		param = xsishader.GetParameter(L"blending");
		if (param.IsValid() && (bool)param.GetValue())
		{
			SceneBlendFactor src = SBF_ONE;
			SceneBlendFactor dst = SBF_ONE;
			
			param = xsishader.GetParameter(L"srcblendingfunction");
			if (param.IsValid())
			{
				src = convertSceneBlend(param.GetValue());
			}
			param = xsishader.GetParameter(L"dstblendingfunction");
			if (param.IsValid())
			{
				dst = convertSceneBlend(param.GetValue());
			}

			pass->setSceneBlending(src, dst);
		}

		ColourValue tmpColour;
		xsishader.GetColorParameterValue(L"Ambient", tmpColour.r, tmpColour.g, 
			tmpColour.b, tmpColour.a);
		pass->setAmbient(tmpColour);
		xsishader.GetColorParameterValue(L"Diffuse", tmpColour.r, tmpColour.g, 
			tmpColour.b, tmpColour.a);
		pass->setDiffuse(tmpColour);
		xsishader.GetColorParameterValue(L"Emissive", tmpColour.r, tmpColour.g, 
			tmpColour.b, tmpColour.a);
		pass->setSelfIllumination(tmpColour);
		xsishader.GetColorParameterValue(L"Specular", tmpColour.r, tmpColour.g, 
			tmpColour.b, tmpColour.a);
		pass->setSpecular(tmpColour);
		
		pass->setShininess(xsishader.GetParameter(L"Shininess").GetValue());

		pass->setLightingEnabled(
			xsishader.GetParameter(L"Enable_Lighting").GetValue());
		
	}
	//-------------------------------------------------------------------------
	SceneBlendFactor XsiMaterialExporter::convertSceneBlend(short xsiVal)
	{
		switch(xsiVal)
		{
		case 0:
			return SBF_ZERO;
		case 1:
			return SBF_ONE;
		case 2:
			return SBF_DEST_COLOUR;
		case 3:
			return SBF_ONE_MINUS_DEST_COLOUR;
		case 4:
			return SBF_SOURCE_ALPHA;
		case 5:
			return SBF_ONE_MINUS_SOURCE_ALPHA;
		case 6: 
			return SBF_DEST_ALPHA;
		case 7:
			return SBF_ONE_MINUS_DEST_ALPHA;
		};
		
	}

}

