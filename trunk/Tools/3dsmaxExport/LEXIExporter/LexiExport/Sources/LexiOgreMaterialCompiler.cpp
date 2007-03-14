/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
Bo Krohn

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

#include "LexiStdAfx.h"
#include "LexiOgreMaterialCompiler.h"
#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include <fstream>
#include <iostream>
#include <direct.h>


COgreMaterialCompiler::COgreMaterialCompiler( CIntermediateMaterial* pIntermediateMaterial )
{
	m_pOgreMaterial.setNull();
	m_pIMaterial = pIntermediateMaterial;

	InitializeOgreComponents();
	CreateOgreMaterial();

}

COgreMaterialCompiler::~COgreMaterialCompiler( void )
{
	
}

void COgreMaterialCompiler::InitializeOgreComponents( void )
{
	Ogre::MaterialManager::getSingletonPtr()->unloadAll();
	Ogre::MaterialManager::getSingletonPtr()->removeAll();
}


void COgreMaterialCompiler::CreateOgreMaterial( void )
{
	assert(m_pIMaterial);

	Ogre::MaterialManager* pMatMgr = Ogre::MaterialManager::getSingletonPtr();
	m_pOgreMaterial = pMatMgr->getByName( m_pIMaterial->GetName().c_str() );

	if(m_pOgreMaterial.isNull())
		m_pOgreMaterial = (Ogre::MaterialPtr)pMatMgr->create( m_pIMaterial->GetName().c_str(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
	else 
		return;

	assert(!m_pOgreMaterial.isNull());

	Ogre::Technique* technique = m_pOgreMaterial->getTechnique(0);
	if(technique == NULL)
		technique = m_pOgreMaterial->createTechnique();

	technique->setName("Default");

	Ogre::Pass*	pass = technique->getPass(0);
	if(pass == NULL)
		pass = technique->createPass();

	pass->setName("Main");
	pass->setAmbient( m_pIMaterial->GetAmbientColor() );
	pass->setSpecular( m_pIMaterial->GetSpecularColor() );
	pass->setShininess( m_pIMaterial->GetGlossiness() );
	//pass->setSelfIllumination( );
	pass->setCullingMode( m_pIMaterial->Get2Sided() ? Ogre::CULL_NONE : Ogre::CULL_CLOCKWISE);
	pass->setShadingMode( m_pIMaterial->GetFaceted() ? Ogre::SO_FLAT : Ogre::SO_GOURAUD);
	pass->setPolygonMode( m_pIMaterial->GetWired() ? Ogre::PM_WIREFRAME : Ogre::PM_SOLID);

	// create texture units
	ParseMaterialMaps(pass);
}

void COgreMaterialCompiler::ParseMaterialMaps( Ogre::Pass* pass )
{
	// Here we should pass the intermediate material on to the plugin which is registered with
	// a matching mask.. when we have the plugin system up and running :)

	// mask to see if material is of type diffuse or other

	short mask = m_pIMaterial->GetMask();

	LOGINFO "Parsing Material(%s) with Map Mask: %i", m_pIMaterial->GetName().c_str(), mask);

	if(mask == 0)		// no texture maps
		CreatePureBlinn(pass);
	else if(mask == 2)		// only diffuse
		CreateDiffuse(pass);
	else if(mask == 4)		// only specular color
		CreateSpecularColor(pass);
	else if(mask == 8)		// only specular level
		CreateSpecularLevel(pass);
	else if(mask == 32)		// only self illumination map
		CreateSelfIllumination(pass);
	else if(mask == 66)		// diffuse map + opacity
		CreateDiffuseAndOpacity(pass);
	else if(mask & 2)		// fallback on ordinary diffuse
		CreateDiffuse(pass);


//	const std::map< Ogre::String, STextureMapInfo >& lMats = m_pIMaterial->GetTextureMaps();
//	std::map< Ogre::String, STextureMapInfo >::const_iterator it = lMats.begin();
//	while (it != lMats.end())
//	{
//		Ogre::String ident = it->first;
//		STextureMapInfo texInfo = it->second;
//
//		Ogre::TextureUnitState* pTexUnit;
//
//		Ogre::String baseFile;
//		Ogre::String basePath;
//		Ogre::StringUtil::splitFilename( texInfo.m_sFilename, baseFile, basePath);
//
//		pTexUnit = pass->createTextureUnitState( baseFile, texInfo.m_iCoordSet-1 );
//		pTexUnit->setTextureNameAlias( texInfo.m_sMapType );
//
//		if(texInfo.m_fOffset[0] != 0 || texInfo.m_fOffset[1] != 0)
//			pTexUnit->setTextureScroll(texInfo.m_fOffset[0], texInfo.m_fOffset[1]);
//		if(texInfo.m_fAngle)
//			pTexUnit->setTextureRotate(Ogre::Radian(-texInfo.m_fAngle));
//		if(texInfo.m_fScale[0] != 1 || texInfo.m_fScale[1] != 1)
//			pTexUnit->setTextureScale(1/texInfo.m_fScale[0], 1/texInfo.m_fScale[1]);
//
//		pTexUnit->setTextureAddressingMode(texInfo.m_AdressingMode);
//
////		if(texInfo.m_sMapType.compare( "self_illumination" ) == 0)
////			pTexUnit->setColourOperation(Ogre::LBO_ADD);
////		else if(texInfo.m_sMapType.compare( "reflection" ) == 0)
////			pTexUnit->setEnvironmentMap( true, Ogre::Env)
//		//else
//		//	pTexUnit->setColourOperationEx(Ogre::LBX_MODULATE_X2);	
//
//
//		pass->setAlphaRejectFunction( Ogre::CMPF_ALWAYS_PASS ); //Default
//
//		if(texInfo.m_bAlpha) 
//		{
//			pass->setAlphaRejectFunction(Ogre::CMPF_GREATER);
//			pass->setAlphaRejectValue(128);
//		}
//
//		it++;
//	}
}

void COgreMaterialCompiler::CreateTextureUnits( Ogre::Pass* pass, STextureMapInfo texInfo  )
{
	Ogre::TextureUnitState* pTexUnit;

	Ogre::String baseFile;
	Ogre::String basePath;
	Ogre::StringUtil::splitFilename( texInfo.m_sFilename, baseFile, basePath);

	pTexUnit = pass->createTextureUnitState( baseFile, texInfo.m_iCoordSet-1 );
	pTexUnit->setTextureNameAlias( texInfo.m_sMapType );

	if(texInfo.m_fOffset[0] != 0 || texInfo.m_fOffset[1] != 0)
		pTexUnit->setTextureScroll(texInfo.m_fOffset[0], texInfo.m_fOffset[1]);
	if(texInfo.m_fAngle)
		pTexUnit->setTextureRotate(Ogre::Radian(-texInfo.m_fAngle));
	if(texInfo.m_fScale[0] != 1 || texInfo.m_fScale[1] != 1)
		pTexUnit->setTextureScale(1/texInfo.m_fScale[0], 1/texInfo.m_fScale[1]);

	pTexUnit->setTextureAddressingMode(texInfo.m_AdressingMode);

	//	const std::map< Ogre::String, STextureMapInfo >& lMats = m_pIMaterial->GetTextureMaps();
	//	std::map< Ogre::String, STextureMapInfo >::const_iterator it = lMats.begin();
	//	while (it != lMats.end())
	//	{
	//		Ogre::String ident = it->first;
	//		STextureMapInfo texInfo = it->second;
	//
	//		Ogre::TextureUnitState* pTexUnit;
	//
	//		Ogre::String baseFile;
	//		Ogre::String basePath;
	//		Ogre::StringUtil::splitFilename( texInfo.m_sFilename, baseFile, basePath);
	//
	//		pTexUnit = pass->createTextureUnitState( baseFile, texInfo.m_iCoordSet-1 );
	//		pTexUnit->setTextureNameAlias( texInfo.m_sMapType );
	//
	//		if(texInfo.m_fOffset[0] != 0 || texInfo.m_fOffset[1] != 0)
	//			pTexUnit->setTextureScroll(texInfo.m_fOffset[0], texInfo.m_fOffset[1]);
	//		if(texInfo.m_fAngle)
	//			pTexUnit->setTextureRotate(Ogre::Radian(-texInfo.m_fAngle));
	//		if(texInfo.m_fScale[0] != 1 || texInfo.m_fScale[1] != 1)
	//			pTexUnit->setTextureScale(1/texInfo.m_fScale[0], 1/texInfo.m_fScale[1]);
	//
	//		pTexUnit->setTextureAddressingMode(texInfo.m_AdressingMode);
	//
	////		if(texInfo.m_sMapType.compare( "self_illumination" ) == 0)
	////			pTexUnit->setColourOperation(Ogre::LBO_ADD);
	////		else if(texInfo.m_sMapType.compare( "reflection" ) == 0)
	////			pTexUnit->setEnvironmentMap( true, Ogre::Env)
	//		//else
	//		//	pTexUnit->setColourOperationEx(Ogre::LBX_MODULATE_X2);	
	//
	//
	//		pass->setAlphaRejectFunction( Ogre::CMPF_ALWAYS_PASS ); //Default
	//
	//		if(texInfo.m_bAlpha) 
	//		{
	//			pass->setAlphaRejectFunction(Ogre::CMPF_GREATER);
	//			pass->setAlphaRejectValue(128);
	//		}
	//
	//		it++;
	//	}
}

void COgreMaterialCompiler::CreatePureBlinn( Ogre::Pass* pass )
{
	pass->setVertexProgram("BlinnVP");
	pass->setFragmentProgram( "Blinn_Pure_FP" );

	Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();

	// HACK: For some reason we cannot set the SpecularLevel if it is its max at 9.99, so we clamp it to 9.98
	float specHack = m_pIMaterial->GetSpecularLevel() > 9.98 ? 9.98 : m_pIMaterial->GetSpecularLevel() ;
	// HACK: For some reason we cannot set the SpecularLevel if it is its max at 9.99, so we clamp it to 9.98
	float glossHack = m_pIMaterial->GetGlossiness() == 1 ? 0.9999 : m_pIMaterial->GetGlossiness() ;
	glossHack = glossHack > 0.01 ? glossHack * 100 : 0.0 ;

	params->setNamedConstant("ambientColor", m_pIMaterial->GetAmbientColor());
	params->setNamedConstant("diffuseColor", m_pIMaterial->GetDiffuseColor());
	params->setNamedConstant("specularColor", m_pIMaterial->GetSpecularColor());
	params->setNamedConstant("specularLevel", specHack);
	params->setNamedConstant("glossLevel", glossHack);
	params->setNamedConstant("opacity", m_pIMaterial->GetOpacity());

	pass->setFragmentProgramParameters(params);
}

void COgreMaterialCompiler::CreateDiffuse( Ogre::Pass* pass )
{
	pass->setVertexProgram("BlinnVP");
	pass->setFragmentProgram( "Blinn_DiffuseMap_FP" );

	Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();

	// HACK: For some reason we cannot set the SpecularLevel if it is its max at 9.99, so we clamp it to 9.98
	float specHack = m_pIMaterial->GetSpecularLevel() > 9.98 ? 9.98 : m_pIMaterial->GetSpecularLevel() ;
	// HACK: For some reason we cannot set the SpecularLevel if it is its max at 9.99, so we clamp it to 9.98
	float glossHack = m_pIMaterial->GetGlossiness() == 1 ? 0.9999 : m_pIMaterial->GetGlossiness() ;
	glossHack = glossHack > 0.01 ? glossHack * 100 : 0.0 ;

	params->setNamedConstant("ambientColor", m_pIMaterial->GetAmbientColor());
	params->setNamedConstant("diffuseColor", m_pIMaterial->GetDiffuseColor());
	params->setNamedConstant("specularColor", m_pIMaterial->GetSpecularColor());
	params->setNamedConstant("specularLevel", specHack);
	params->setNamedConstant("glossLevel", glossHack);
	params->setNamedConstant("opacity", m_pIMaterial->GetOpacity());

	STextureMapInfo texInfo = m_pIMaterial->GetTextureMapInfoFromName("diffuse");
	if ( !texInfo.isNull())
		params->setNamedConstant("amount", texInfo.m_fAmount);

	CreateTextureUnits(pass, texInfo);

	pass->setFragmentProgramParameters(params);
}

void COgreMaterialCompiler::CreateSpecularColor( Ogre::Pass* pass )
{
	pass->setVertexProgram("BlinnVP");
	pass->setFragmentProgram( "Blinn_SpecularColor_FP" );

	Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();

	// HACK: For some reason we cannot set the SpecularLevel if it is its max at 9.99, so we clamp it to 9.98
	float specHack = m_pIMaterial->GetSpecularLevel() > 9.98 ? 9.98 : m_pIMaterial->GetSpecularLevel() ;
	// HACK: For some reason we cannot set the SpecularLevel if it is its max at 9.99, so we clamp it to 9.98
	float glossHack = m_pIMaterial->GetGlossiness() == 1 ? 0.9999 : m_pIMaterial->GetGlossiness() ;
	glossHack = glossHack > 0.01 ? glossHack * 100 : 0.0 ;

	params->setNamedConstant("ambientColor", m_pIMaterial->GetAmbientColor());
	params->setNamedConstant("diffuseColor", m_pIMaterial->GetDiffuseColor());
	params->setNamedConstant("specularColor", m_pIMaterial->GetSpecularColor());
	params->setNamedConstant("specularLevel", specHack);
	params->setNamedConstant("glossLevel", glossHack);
	params->setNamedConstant("opacity", m_pIMaterial->GetOpacity());

	STextureMapInfo texInfo = m_pIMaterial->GetTextureMapInfoFromName("specular_color");
	if ( !texInfo.isNull())
		params->setNamedConstant("amount", texInfo.m_fAmount);

	CreateTextureUnits(pass, texInfo);

	pass->setFragmentProgramParameters(params);
}

void COgreMaterialCompiler::CreateSpecularLevel( Ogre::Pass* pass )
{
	pass->setVertexProgram("BlinnVP");
	pass->setFragmentProgram( "Blinn_SpecularLevel_FP" );

	Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();

	// HACK: For some reason we cannot set the SpecularLevel if it is its max at 9.99, so we clamp it to 9.98
	float specHack = m_pIMaterial->GetSpecularLevel() > 9.98 ? 9.98 : m_pIMaterial->GetSpecularLevel() ;
	// HACK: For some reason we cannot set the SpecularLevel if it is its max at 9.99, so we clamp it to 9.98
	float glossHack = m_pIMaterial->GetGlossiness() == 1 ? 0.9999 : m_pIMaterial->GetGlossiness() ;
	glossHack = glossHack > 0.01 ? glossHack * 100 : 0.0 ;

	params->setNamedConstant("ambientColor", m_pIMaterial->GetAmbientColor());
	params->setNamedConstant("diffuseColor", m_pIMaterial->GetDiffuseColor());
	params->setNamedConstant("specularColor", m_pIMaterial->GetSpecularColor());
	params->setNamedConstant("specularLevel", specHack);
	params->setNamedConstant("glossLevel", glossHack);
	params->setNamedConstant("opacity", m_pIMaterial->GetOpacity());

	STextureMapInfo texInfo = m_pIMaterial->GetTextureMapInfoFromName("specular_level");
	if ( !texInfo.isNull())
		params->setNamedConstant("amount", texInfo.m_fAmount);

	CreateTextureUnits(pass, texInfo);

	pass->setFragmentProgramParameters(params);
}

void COgreMaterialCompiler::CreateSelfIllumination( Ogre::Pass* pass )
{
	pass->setVertexProgram("Blinn_4UV_VP");
	pass->setFragmentProgram( "Blinn_SelfIllumination_FP" );

	Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();

	// HACK: For some reason we cannot set the SpecularLevel if it is its max at 9.99, so we clamp it to 9.98
	float specHack = m_pIMaterial->GetSpecularLevel() > 9.98 ? 9.98 : m_pIMaterial->GetSpecularLevel() ;
	// HACK: For some reason we cannot set the SpecularLevel if it is its max at 9.99, so we clamp it to 9.98
	float glossHack = m_pIMaterial->GetGlossiness() == 1 ? 0.9999 : m_pIMaterial->GetGlossiness() ;
	glossHack = glossHack > 0.01 ? glossHack * 100 : 0.0 ;

	params->setNamedConstant("ambientColor", m_pIMaterial->GetAmbientColor());
	params->setNamedConstant("diffuseColor", m_pIMaterial->GetDiffuseColor());
	params->setNamedConstant("specularColor", m_pIMaterial->GetSpecularColor());
	params->setNamedConstant("specularLevel", specHack);
	params->setNamedConstant("glossLevel", glossHack);
	params->setNamedConstant("opacity", m_pIMaterial->GetOpacity());

	STextureMapInfo texInfo = m_pIMaterial->GetTextureMapInfoFromName("self_illumination");
	if ( !texInfo.isNull())
	{
		params->setNamedConstant("amount", texInfo.m_fAmount);
		params->setNamedConstant("uvIndex", (float)texInfo.m_iCoordSet-1);
	}

	CreateTextureUnits(pass, texInfo);

	pass->setFragmentProgramParameters(params);
}

void COgreMaterialCompiler::CreateDiffuseAndOpacity( Ogre::Pass* pass )
{
	pass->setVertexProgram("BlinnVP");
	pass->setFragmentProgram( "Blinn_DiffuseAndOpacityMap_FP" );

	pass->setSceneBlending( Ogre::SBT_TRANSPARENT_ALPHA );
	pass->setDepthWriteEnabled(false);

	// HACK: For some reason we cannot set the SpecularLevel if it is its max at 9.99, so we clamp it to 9.98
	float specHack = m_pIMaterial->GetSpecularLevel() > 9.98 ? 9.98 : m_pIMaterial->GetSpecularLevel() ;
	// HACK: For some reason we cannot set the SpecularLevel if it is its max at 9.99, so we clamp it to 9.98
	float glossHack = m_pIMaterial->GetGlossiness() == 1 ? 0.9999 : m_pIMaterial->GetGlossiness() ;
	glossHack = glossHack > 0.01 ? glossHack * 100 : 0.0 ;

	Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
	params->setNamedConstant("ambientColor", m_pIMaterial->GetAmbientColor());
	params->setNamedConstant("diffuseColor", m_pIMaterial->GetDiffuseColor());
	params->setNamedConstant("specularColor", m_pIMaterial->GetSpecularColor());
	params->setNamedConstant("specularLevel", specHack);
	params->setNamedConstant("glossLevel", glossHack);

	STextureMapInfo texInfo = m_pIMaterial->GetTextureMapInfoFromName("diffuse");
	if ( !texInfo.isNull())
		params->setNamedConstant("amount", texInfo.m_fAmount);

	CreateTextureUnits(pass, texInfo);

	texInfo = m_pIMaterial->GetTextureMapInfoFromName("opacity");
	if ( !texInfo.isNull())
		params->setNamedConstant("opacity", texInfo.m_fAmount);
	
	CreateTextureUnits(pass, texInfo);

	pass->setFragmentProgramParameters(params);
}

void COgreMaterialCompiler::CopyTextureMaps( Ogre::String outPath )
{
	// copy texture files to target path
	// optionally convert to .dds

	const std::map< Ogre::String, STextureMapInfo >& lMats = m_pIMaterial->GetTextureMaps();
	std::map< Ogre::String, STextureMapInfo >::const_iterator it = lMats.begin();
	while (it != lMats.end())
	{
		Ogre::String ident = it->first;
		STextureMapInfo texInfo = it->second;

		Ogre::String baseFile;
		Ogre::String basePath;
		Ogre::StringUtil::splitFilename( texInfo.m_sFilename, baseFile, basePath);

		//Ogre::String srcFile = texInfo.m_sFilename;

		if( doFileCopy(texInfo.m_sFilename, outPath+baseFile) != 0 )
			LOGWARNING "Couldn´t copy texture map: %s", texInfo.m_sFilename.c_str());
		it++;
	}
}

void COgreMaterialCompiler::CopyShaderSources( Ogre::String outPath )
{
	// copy texture files to target path
	// optionally convert to .dds

	// ensure we are in the 3dMax dir
	char szAppPath[MAX_PATH] = "";
	::GetModuleFileName(NULL,szAppPath,sizeof(szAppPath) - 1);

	Ogre::String cwd(szAppPath);
	Ogre::String fileName, filePath;
	Ogre::StringUtil::splitFilename(cwd, fileName, filePath);

	_chdir(filePath.c_str());
	
	doFileCopy("LexiExporter\\shaders\\Blinn.cg", outPath+Ogre::String("Blinn.cg") );
	doFileCopy("LexiExporter\\shaders\\Shaders.program", outPath+Ogre::String("Shaders.program") );
}

bool COgreMaterialCompiler::WriteOgreMaterial( const Ogre::String& sFilename )
{
	Ogre::MaterialSerializer* pMatWriter = new Ogre::MaterialSerializer();
	try
	{
		pMatWriter->exportMaterial( m_pOgreMaterial, sFilename );
	}
	catch (Ogre::Exception& e)
	{
		MessageBox( NULL, e.getFullDescription().c_str(), "ERROR", MB_ICONERROR);
		return false;
	}
	delete pMatWriter;
	return true;
}

Ogre::MaterialPtr COgreMaterialCompiler::GetOgreMaterial( void )
{
	return m_pOgreMaterial;
}


#define BUFF_SIZE 2048
int COgreMaterialCompiler::doFileCopy(Ogre::String inFile, Ogre::String outFile)
{
	char buff[BUFF_SIZE];
	int readBytes = 1;

	ifstream inFileStream(inFile.c_str(), ios::in|ios::binary);
	if(!inFileStream)
	{
		return -1;
	}

	ifstream tmpStream(outFile.c_str());
	if(tmpStream)
	{
		return -1;
	}
	tmpStream.close();

	ofstream outFileStream(outFile.c_str(), ios::out|ios::binary);
	if(!outFileStream)
	{
		return -1;
	}

	while(readBytes != 0)
	{
		inFileStream.read((char*)buff, BUFF_SIZE);
		readBytes = inFileStream.gcount();
		outFileStream.write((char*)buff, readBytes);
	}
	return 0;
}