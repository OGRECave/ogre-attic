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
#include <xsi_imageclip.h>
#include <xsi_image.h>


namespace Ogre {

	//-------------------------------------------------------------------------
	XsiMaterialExporter::XsiMaterialExporter()
	{

	}
	//-------------------------------------------------------------------------
	XsiMaterialExporter::~XsiMaterialExporter()
	{
		clearPassQueue();
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::exportMaterials(MaterialMap& materials, 
		const String& filename, bool copyTextures)
	{
		LogOgreAndXSI("** Begin OGRE Material Export **");
		
		String texturePath;
		if (copyTextures)
		{
			// derive the texture path
			String::size_type pos = filename.find_last_of("\\");
			if (pos == String::npos)
			{
				pos = filename.find_last_of("/");			
			}
			if (pos != String::npos)
			{
				texturePath = filename.substr(0, pos + 1);
			}
		}
		
		mMatSerializer.clearQueue();

		for (MaterialMap::iterator m = materials.begin(); m != materials.end(); ++m)
		{
			exportMaterial(m->second, copyTextures, texturePath);
		}

		mMatSerializer.exportQueued(filename);

		LogOgreAndXSI("** OGRE Material Export Complete **");
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::exportMaterial(MaterialEntry* matEntry, 
		bool copyTextures, const String& texturePath)
	{
		LogOgreAndXSI("Exporting " + matEntry->name);

		MaterialPtr mat = MaterialManager::getSingleton().create(matEntry->name,
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Technique* t = mat->createTechnique();

		// collect the passes into our queue
		// XSI stores passes in reverse order, so invert them
		clearPassQueue();
		XSI::Shader shader(matEntry->xsiShader);
		PassEntry* passEntry = new PassEntry();
		mPassQueue.push_front(passEntry);
		while (1)
		{
			passEntry->shaders.Add(shader);

			XSI::CRef source = shader.GetParameter(L"previous").GetSource();
			if(!source.IsValid() || !source.IsA(XSI::siShaderID))
			{
				// finish
				break;
			}

			shader = XSI::Shader(source);
			// If we find a 'blending' parameter, we're on a new pass
			if (shader.GetParameter(L"blending").IsValid())
			{
				passEntry = new PassEntry();
				mPassQueue.push_front(passEntry); // push front to invert order
			}
		}


		// Now go through each pass and create OGRE version
		for (PassQueue::iterator p = mPassQueue.begin(); p != mPassQueue.end(); ++p)
		{
			PassEntry* passEntry = *p;
			Pass* pass = t->createPass();
			LogOgreAndXSI("Added Pass");

			// Need to pre-populate pass textures to match up transforms
			populatePassTextures(pass, passEntry, copyTextures, texturePath);
			// Do the rest
			for (int s = 0; s < passEntry->shaders.GetCount(); ++s)
			{
				XSI::Shader shader(passEntry->shaders[s]);
				populatePass(pass, shader);
			}

		}


		mMatSerializer.queueForExport(mat);
		

	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::clearPassQueue(void)
	{
		for (PassQueue::iterator i = mPassQueue.begin(); i != mPassQueue.end(); ++i)
		{
			delete *i;
		}
		mPassQueue.clear();

	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populatePass(Pass* pass, XSI::Shader& xsishader)
	{
		populatePassDepthCull(pass, xsishader);
		populatePassSceneBlend(pass, xsishader);
		populatePassLighting(pass, xsishader);
		populatePassTextureTransforms(pass, xsishader);
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populatePassDepthCull(Pass* pass, 
		XSI::Shader& xsishader)
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
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populatePassSceneBlend(Pass* pass, 
		XSI::Shader& xsishader)
	{
		XSI::Parameter param = xsishader.GetParameter(L"blending");
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
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populatePassLighting(Pass* pass, 
		XSI::Shader& xsishader)
	{
		XSI::Parameter param = xsishader.GetParameter(L"Enable_Lighting").GetValue();
		if (param.IsValid())
		{
			pass->setLightingEnabled(param.GetValue());

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
		}

		
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populatePassTextures(Pass* pass, 
		PassEntry* passEntry, bool copyTextures, const String& targetFolder)
	{
		// We need to search all shaders back to the point we would change
		// passes, and add all the textures. This is because we don't know
		// where in the shaders the texture transforms might be, since they
		// are linked via 'target' not by being on the same object.
		mTextureUnitTargetMap.clear();

		for (int s = 0; s < passEntry->shaders.GetCount(); ++s)
		{
			XSI::Shader shader(passEntry->shaders[s]);

			if (shader.GetParameter(L"target").IsValid() && 
				!shader.GetParameter(L"bottom").IsValid())
			{
				add2DTexture(pass, shader, copyTextures, targetFolder);
			}
			else if (shader.GetParameter(L"bottom").IsValid())
			{
				addCubicTexture(pass, shader, copyTextures, targetFolder);
			}

			
		}
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::add2DTexture(Pass* pass, XSI::Shader& shader, 
		bool copyTextures, const String& targetFolder)
	{
		// create texture unit state and map from target incase future xforms
		TextureUnitState* tex = pass->createTextureUnitState();

		long target = shader.GetParameter(L"target").GetValue();
		mTextureUnitTargetMap[target] = tex;

		// Get image
		XSI::CRef src = shader.GetParameter(L"Texture").GetSource();
		if (src.IsValid() && src.IsA(XSI::siImageClipID))
		{
			XSI::ImageClip imgClip(src);
			String srcTextureName = 
				XSItoOgre(imgClip.GetParameter(L"SourceFileName").GetValue());

			String::size_type pos = srcTextureName.find_last_of("\\");
			if (pos == String::npos)
			{
				pos = srcTextureName.find_last_of("/");
			}
			String textureName = 
				srcTextureName.substr(pos+1, srcTextureName.size() - pos - 1);
			String destTextureName = targetFolder + textureName;

			// TODO - copy texture if required

			LogOgreAndXSI("Adding texture " + textureName);
			tex->setTextureName(textureName);
		}



	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::addCubicTexture(Pass* pass, XSI::Shader& shader, 
		bool copyTextures, const String& targetFolder)
	{
		// TODO
	}
	//-------------------------------------------------------------------------
	void XsiMaterialExporter::populatePassTextureTransforms(Pass* pass, 
		XSI::Shader& xsishader)
	{
		// TODO
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

		return SBF_ZERO;
		
	}

}

