/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreRoot.h"
#include "OgreHlslFixedFuncEmuShaderGenerator.h"

namespace Ogre 
{
	//---------------------------------------------------------------------
	HlslFixedFuncEmuShaderGenerator::HlslFixedFuncEmuShaderGenerator()
	{
		mLanguageName	= "hlsl";
		mName			= "hlsl";
		mVpTarget		= "vs_3_0";
		mFpTarget		= "ps_3_0";
	}
	//---------------------------------------------------------------------
	HlslFixedFuncEmuShaderGenerator::~HlslFixedFuncEmuShaderGenerator()
	{

	}
	//---------------------------------------------------------------------
	const String HlslFixedFuncEmuShaderGenerator::getShaderSource( const String & vertexProgramName,
		const String & fragmentProgramName, const VertexBufferDeclaration & vertexBufferDeclaration, 
		FixedFuncState & fixedFuncState )
	{
		bool bHasColor = vertexBufferDeclaration.hasColor();
		uint8 texcoordCount = vertexBufferDeclaration.getTexcoordCount();

		String shaderSource = "";

		shaderSource = shaderSource + "struct VS_INPUT { ";

		uint8 semanticCount[100];
		ZeroMemory(semanticCount, sizeof(uint8) * 100);
		const VertexBufferElementList & vertexBufferElementList = vertexBufferDeclaration.getVertexBufferElementList();
		for (unsigned short i = 0 ; i < vertexBufferElementList.size() ; i++)
		{
			VertexElementSemantic semantic = vertexBufferElementList[i].getVertexElementSemantic();
			VertexElementType type = vertexBufferElementList[i].getVertexElementType();

			String thisElementSemanticCount = StringConverter::toString(semanticCount[semantic]);
			semanticCount[semantic]++;
			String parameterType = "";
			String parameterName = "";
			String parameterShaderTypeName = "";

			switch (type)
			{
			case VET_FLOAT1:
				parameterType = "float";
				break;
			case VET_FLOAT2:
				parameterType = "float2";
				break;
			case VET_FLOAT3:
				parameterType = "float3";
				break;
			case VET_FLOAT4:
				parameterType = "float4";
				break;
			case VET_COLOUR:
			case VET_COLOUR_ARGB:
			case VET_COLOUR_ABGR:
				parameterType = "unsigned int";		
				break;
			case VET_SHORT1:
				parameterType = "short";		
				break;
			case VET_SHORT2:
				parameterType = "short2";		
				break;
			case VET_SHORT3:
				parameterType = "short3";		
				break;
			case VET_SHORT4:
				parameterType = "short4";		
				break;
			case VET_UBYTE4:
				parameterType = "char4";
				break;

			}
			switch (semantic)
			{
			case VES_POSITION:
				parameterName = "Position";
				parameterShaderTypeName = "POSITION";
				//parameterType = "float4"; // position must be float4 (and not float3 like in the buffer)
				break;
			case VES_BLEND_WEIGHTS:
				parameterName = "BlendWeight";
				parameterShaderTypeName = "BLENDWEIGHT";
				break;
			case VES_BLEND_INDICES:
				parameterName = "BlendIndices";
				parameterShaderTypeName = "BLENDINDICES";
				break;
			case VES_NORMAL:
				parameterName = "Normal";
				parameterShaderTypeName = "NORMAL";
				break;
			case VES_DIFFUSE:
				parameterName = "DiffuseColor";
				parameterShaderTypeName = "COLOR";
				break;
			case VES_SPECULAR:
				parameterName = "SpecularColor";
				parameterShaderTypeName = "COLOR";
				thisElementSemanticCount = StringConverter::toString(semanticCount[VES_DIFFUSE]); // Diffuse is the "COLOR" count...
				semanticCount[VES_DIFFUSE]++;
				break;
			case VES_TEXTURE_COORDINATES:
				parameterName = "Texcoord";
				parameterShaderTypeName = "TEXCOORD";
				break;
			case VES_BINORMAL:
				parameterName = "Binormal";
				parameterShaderTypeName = "BINORMAL";
				break;
			case VES_TANGENT:
				parameterName = "Tangent";
				parameterShaderTypeName = "TANGENT";
				break;
			}



			shaderSource = shaderSource + parameterType + " " + parameterName + thisElementSemanticCount + " : " + parameterShaderTypeName + thisElementSemanticCount + ";\n";
		}

		shaderSource = shaderSource + " };";



		shaderSource = shaderSource + "float4x4  World;\n";
		shaderSource = shaderSource + "float4x4  View;\n";
		shaderSource = shaderSource + "float4x4  Projection;\n";
		shaderSource = shaderSource + "float4x4  ViewIT;\n";
		shaderSource = shaderSource + "float4x4  WorldViewIT;\n";	
	
		for(size_t i = 0 ; i < fixedFuncState.getTextureLayerStateList().size() ; i++)
		{
			String layerCounter = StringConverter::toString(i);
			shaderSource = shaderSource + "float4x4  TextureMatrix" + layerCounter + ";\n";
		}

		switch (fixedFuncState.getGeneralFixedFuncState().getFogMode())
		{
		case FOG_NONE:
			break;
		case FOG_EXP:
		case FOG_EXP2:
			shaderSource = shaderSource + "float FogDensity;\n";
			break;
		case FOG_LINEAR:
			shaderSource = shaderSource + "float FogStart;\n";
			shaderSource = shaderSource + "float FogEnd;\n";
			break;
		}

		if (fixedFuncState.getGeneralFixedFuncState().getLightingEnabled())
		{
			shaderSource = shaderSource + "float4 BaseLightAmbient;\n";

			for(size_t i = 0 ; i < fixedFuncState.getLights().size() ; i++)
			{
				String prefix = "Light" + StringConverter::toString(i) + "_";
				switch (fixedFuncState.getLights()[i])
				{
				case Light::LT_POINT:
					shaderSource = shaderSource + "float3 " + prefix + "Position;\n";
					shaderSource = shaderSource + "float4 " + prefix + "Ambient;\n";
					shaderSource = shaderSource + "float4 " + prefix + "Diffuse;\n";
					shaderSource = shaderSource + "float4 " + prefix + "Specular;\n";				
					shaderSource = shaderSource + "float  " + prefix + "Range;\n";				
					shaderSource = shaderSource + "float3 " + prefix + "Attenuation;\n";				
					break;
				case Light::LT_DIRECTIONAL:
					shaderSource = shaderSource + "float3 " + prefix + "Direction;\n";
					shaderSource = shaderSource + "float4 " + prefix + "Ambient;\n";
					shaderSource = shaderSource + "float4 " + prefix + "Diffuse;\n";
					shaderSource = shaderSource + "float4 " + prefix + "Specular;\n";				
					break;
				case Light::LT_SPOTLIGHT:
					shaderSource = shaderSource + "float3 " + prefix + "Direction;\n";
					shaderSource = shaderSource + "float3 " + prefix + "Position;\n";
					shaderSource = shaderSource + "float4 " + prefix + "Ambient;\n";
					shaderSource = shaderSource + "float4 " + prefix + "Diffuse;\n";
					shaderSource = shaderSource + "float4 " + prefix + "Specular;\n";				
					shaderSource = shaderSource + "float3 " + prefix + "Attenuation;\n";				
					shaderSource = shaderSource + "float3 " + prefix + "Spot;\n";				
					break;
				}
			}

		}
		


		shaderSource = shaderSource + "struct VS_OUTPUT\n";
		shaderSource = shaderSource + "{\n";
		shaderSource = shaderSource + "float4 Pos : SV_POSITION;\n";
		for(size_t i = 0 ; i < fixedFuncState.getTextureLayerStateList().size() ; i++)
		{
			String layerCounter = StringConverter::toString(i);
			shaderSource = shaderSource + "float2 Texcoord" + layerCounter + " : TEXCOORD" + layerCounter + ";\n";
		}
		
		shaderSource = shaderSource + "float4 Color : COLOR0;\n";
		shaderSource = shaderSource + "float4 ColorSpec : COLOR1;\n";

		if (fixedFuncState.getGeneralFixedFuncState().getFogMode() != FOG_NONE)
		{
			shaderSource = shaderSource + "float fogDist : FOGDISTANCE;\n"; 
		}

		shaderSource = shaderSource + "};\n";

		shaderSource = shaderSource + "VS_OUTPUT " + vertexProgramName + "( VS_INPUT input )\n";
		shaderSource = shaderSource + "{\n";
		shaderSource = shaderSource + "VS_OUTPUT output = (VS_OUTPUT)0;\n";	
		shaderSource = shaderSource + "float4 worldPos = mul( World, float4( input.Position0 , 1 ));\n";
		shaderSource = shaderSource + "float4 cameraPos = mul( View, worldPos );\n";
		shaderSource = shaderSource + "output.Pos = mul( Projection, cameraPos );\n";	





		for(size_t i = 0 ; i < fixedFuncState.getTextureLayerStateList().size() ; i++)
		{
			String layerCounter = StringConverter::toString(i);
			if (i < texcoordCount)
			{
				shaderSource = shaderSource + "float4 texCordWithMatrix = float4(input.Texcoord" + layerCounter + ".x, input.Texcoord" + layerCounter + ".y, 0, 1);\n";
				shaderSource = shaderSource + "texCordWithMatrix = mul(TextureMatrix" + layerCounter + ", texCordWithMatrix );\n";
				shaderSource = shaderSource + "output.Texcoord" + layerCounter + " = texCordWithMatrix.xy;\n";		
			}
			else
			{
				shaderSource = shaderSource + "output.Texcoord" + layerCounter + " = float2(0.0, 0.0);\n"; // so no error
			}
		}

		shaderSource = shaderSource + "output.ColorSpec = float4(0.0, 0.0, 0.0, 0.0);\n";


		if (fixedFuncState.getGeneralFixedFuncState().getLightingEnabled() && fixedFuncState.getLights().size() > 0)
		{
			shaderSource = shaderSource + "output.Color = BaseLightAmbient;\n";
			if (bHasColor)
			{
				shaderSource = shaderSource + "output.Color.x = ((input.DiffuseColor0 >> 24) & 0xFF) / 255.0f;\n";
				shaderSource = shaderSource + "output.Color.y = ((input.DiffuseColor0 >> 16) & 0xFF) / 255.0f;\n"; 
				shaderSource = shaderSource + "output.Color.z = ((input.DiffuseColor0 >> 8) & 0xFF) / 255.0f;\n";
				shaderSource = shaderSource + "output.Color.w = (input.DiffuseColor0 & 0xFF) / 255.0f;\n";
			}


			shaderSource = shaderSource + "float3 N = mul((float3x3)WorldViewIT, input.Normal0);\n";
			shaderSource = shaderSource + "float3 V = -normalize(cameraPos);\n";

			shaderSource = shaderSource + "#define fMaterialPower 16.f\n";

			for(size_t i = 0 ; i < fixedFuncState.getLights().size() ; i++)
			{
				String prefix = "Light" + StringConverter::toString(i) + "_";
				switch (fixedFuncState.getLights()[i])
				{
				case Light::LT_POINT:
					shaderSource = shaderSource + "{\n";
					shaderSource = shaderSource + "  float3 PosDiff = " + prefix + "Position-(float3)mul(World,input.Position0);\n";
					shaderSource = shaderSource + "  float3 L = mul((float3x3)ViewIT, normalize((PosDiff)));\n";
					shaderSource = shaderSource + "  float NdotL = dot(N, L);\n";
					shaderSource = shaderSource + "  float4 Color = " + prefix + "Ambient;\n";
					shaderSource = shaderSource + "  float4 ColorSpec = 0;\n";
					shaderSource = shaderSource + "  float fAtten = 1.f;\n";
					shaderSource = shaderSource + "  if(NdotL >= 0.f)\n";
					shaderSource = shaderSource + "  {\n";
					shaderSource = shaderSource + "    //compute diffuse color\n";
					shaderSource = shaderSource + "    Color += NdotL * " + prefix + "Diffuse;\n";
					shaderSource = shaderSource + "    //add specular component\n";
					shaderSource = shaderSource + "    float3 H = normalize(L + V);   //half vector\n";
					shaderSource = shaderSource + "    ColorSpec = pow(max(0, dot(H, N)), fMaterialPower) * " + prefix + "Specular;\n";
					shaderSource = shaderSource + "    float LD = length(PosDiff);\n";
					shaderSource = shaderSource + "    if(LD > " + prefix + "Range)\n";
					shaderSource = shaderSource + "    {\n";
					shaderSource = shaderSource + "      fAtten = 0.f;\n";
					shaderSource = shaderSource + "    }\n";
					shaderSource = shaderSource + "    else\n";
					shaderSource = shaderSource + "    {\n";
					shaderSource = shaderSource + "      fAtten *= 1.f/(" + prefix + "Attenuation.x + " + prefix + "Attenuation.y*LD + " + prefix + "Attenuation.z*LD*LD);\n";
					shaderSource = shaderSource + "    }\n";
					shaderSource = shaderSource + "    Color *= fAtten;\n";
					shaderSource = shaderSource + "    ColorSpec *= fAtten;\n";
					shaderSource = shaderSource + "    output.Color += Color;\n";
					shaderSource = shaderSource + "    output.ColorSpec += ColorSpec;\n";
					shaderSource = shaderSource + "  }\n";
					shaderSource = shaderSource + "}\n";

					break;
				case Light::LT_DIRECTIONAL:			
					shaderSource = shaderSource + "{\n";
					shaderSource = shaderSource + "  float3 L = mul((float3x3)ViewIT, -normalize(" + prefix + "Direction));\n";
					shaderSource = shaderSource + "  float NdotL = dot(N, L);\n";
					shaderSource = shaderSource + "  float4 Color = " + prefix + "Ambient;\n";
					shaderSource = shaderSource + "  float4 ColorSpec = 0;\n";
					shaderSource = shaderSource + "  if(NdotL > 0.f)\n";
					shaderSource = shaderSource + "  {\n";
					shaderSource = shaderSource + "    //compute diffuse color\n";
					shaderSource = shaderSource + "    Color += NdotL * " + prefix + "Diffuse;\n";
					shaderSource = shaderSource + "    //add specular component\n";
					shaderSource = shaderSource + "    float3 H = normalize(L + V);   //half vector\n";
					shaderSource = shaderSource + "    ColorSpec = pow(max(0, dot(H, N)), fMaterialPower) * " + prefix + "Specular;\n";
					shaderSource = shaderSource + "    output.Color += Color;\n";
					shaderSource = shaderSource + "    output.ColorSpec += ColorSpec;\n";
					shaderSource = shaderSource + "  }\n";
					shaderSource = shaderSource + "}\n";
					break;
				case Light::LT_SPOTLIGHT:
					shaderSource = shaderSource + "{\n";
					shaderSource = shaderSource + "  float3 PosDiff = " + prefix + "Position-(float3)mul(World,input.Position0);\n";
					shaderSource = shaderSource + "   float3 L = mul((float3x3)ViewIT, normalize((PosDiff)));\n";
					shaderSource = shaderSource + "   float NdotL = dot(N, L);\n";
					shaderSource = shaderSource + "   Out.Color = " + prefix + "Ambient;\n";
					shaderSource = shaderSource + "   Out.ColorSpec = 0;\n";
					shaderSource = shaderSource + "   float fAttenSpot = 1.f;\n";
					shaderSource = shaderSource + "   if(NdotL >= 0.f)\n";
					shaderSource = shaderSource + "   {\n";
					shaderSource = shaderSource + "      //compute diffuse color\n";
					shaderSource = shaderSource + "      Out.Color += NdotL * " + prefix + "Diffuse;\n";
					shaderSource = shaderSource + "      //add specular component\n";
					shaderSource = shaderSource + "       float3 H = normalize(L + V);   //half vector\n";
					shaderSource = shaderSource + "       Out.ColorSpec = pow(max(0, dot(H, N)), fMaterialPower) * " + prefix + "Specular;\n";
					shaderSource = shaderSource + "      float LD = length(PosDiff);\n";
					shaderSource = shaderSource + "      if(LD > lights[i].fRange)\n";
					shaderSource = shaderSource + "      {\n";
					shaderSource = shaderSource + "         fAttenSpot = 0.f;\n";
					shaderSource = shaderSource + "      }\n";
					shaderSource = shaderSource + "      else\n";
					shaderSource = shaderSource + "      {\n";
					shaderSource = shaderSource + "         fAttenSpot *= 1.f/(" + prefix + "Attenuation.x + " + prefix + "Attenuation.y*LD + " + prefix + "Attenuation.z*LD*LD);\n";
					shaderSource = shaderSource + "      }\n";
					shaderSource = shaderSource + "      //spot cone computation\n";
					shaderSource = shaderSource + "      float3 L2 = mul((float3x3)ViewIT, -normalize(" + prefix + "Direction));\n";
					shaderSource = shaderSource + "      float rho = dot(L, L2);\n";
					shaderSource = shaderSource + "      fAttenSpot *= pow(saturate((rho - " + prefix + "Spot.y)/(" + prefix + "Spot.x - " + prefix + "Spot.y)), " + prefix + "Spot.z);\n";
					shaderSource = shaderSource + "		Color *= fAttenSpot;\n";
					shaderSource = shaderSource + "		ColorSpec *= fAttenSpot;\n";
					shaderSource = shaderSource + "    output.Color += Color;\n";
					shaderSource = shaderSource + "    output.ColorSpec += ColorSpec;\n";
					shaderSource = shaderSource + "   }\n";
					shaderSource = shaderSource + "}\n";
					break;
				}
			}

		}
		else
		{
			if (bHasColor)
			{
				shaderSource = shaderSource + "output.Color.x = ((input.DiffuseColor0 >> 24) & 0xFF) / 255.0f;\n";
				shaderSource = shaderSource + "output.Color.y = ((input.DiffuseColor0 >> 16) & 0xFF) / 255.0f;\n"; 
				shaderSource = shaderSource + "output.Color.z = ((input.DiffuseColor0 >> 8) & 0xFF) / 255.0f;\n";
				shaderSource = shaderSource + "output.Color.w = (input.DiffuseColor0 & 0xFF) / 255.0f;\n";
			}
			else
			{
				shaderSource = shaderSource + "output.Color = float4(1.0, 1.0, 1.0, 1.0);\n";
			}
		}

		switch (fixedFuncState.getGeneralFixedFuncState().getFogMode())
		{
		case FOG_NONE:
			break;
		case FOG_EXP:
		case FOG_EXP2:
		case FOG_LINEAR:
			shaderSource = shaderSource + "output.fogDist = length(cameraPos.xyz);\n";
			break;
		}

		switch (fixedFuncState.getGeneralFixedFuncState().getFogMode())
		{
		case FOG_NONE:
			break;
		case FOG_EXP:
			shaderSource = shaderSource + "#define E 2.71828\n";
			shaderSource = shaderSource + "output.fogDist = 1.0 / pow( E, output.fogDist*FogDensity );\n";
			shaderSource = shaderSource + "output.fogDist = clamp( output.fogDist, 0, 1 );\n";
			break;
		case FOG_EXP2:
			shaderSource = shaderSource + "#define E 2.71828\n";
			shaderSource = shaderSource + "output.fogDist = 1.0 / pow( E, output.fogDist*output.fogDist*FogDensity*FogDensity );\n";
			shaderSource = shaderSource + "output.fogDist = clamp( output.fogDist, 0, 1 );\n";
			break;
		case FOG_LINEAR:
			shaderSource = shaderSource + "output.fogDist = (FogEnd - output.fogDist)/(FogEnd - FogStart);\n";
			shaderSource = shaderSource + "output.fogDist = clamp( output.fogDist, 0, 1 );\n";
			break;
		}

		shaderSource = shaderSource + "return output;}\n";

		// here starts the fragment shader

		for(size_t i = 0 ; i < fixedFuncState.getTextureLayerStateList().size() ; i++)
		{
			String layerCounter = StringConverter::toString(i);
			shaderSource = shaderSource + "sampler Texture" + layerCounter + " : register(s" + layerCounter + ");\n";
		}
		
		shaderSource = shaderSource + "float4  FogColor;\n";

		shaderSource = shaderSource + "float4 " + fragmentProgramName + "( VS_OUTPUT input ) : SV_Target\n";
		shaderSource = shaderSource + "{\n";
		shaderSource = shaderSource + "float4 finalColor = input.Color + input.ColorSpec;\n";

		for(size_t i = 0 ; i < fixedFuncState.getTextureLayerStateList().size() ; i++)
		{
			const TextureLayerState & curTextureLayerState = fixedFuncState.getTextureLayerStateList()[i];
			String layerCounter = StringConverter::toString(i);
			String coordIndex = StringConverter::toString(curTextureLayerState.getCoordIndex());
			shaderSource = shaderSource + "{\n";
			shaderSource = shaderSource + "finalColor = tex2D(Texture" + layerCounter + ", input.Texcoord" + coordIndex + ")  * finalColor;\n";
			shaderSource = shaderSource + "}\n";
		}

		if (fixedFuncState.getGeneralFixedFuncState().getFogMode() != FOG_NONE)
		{

			shaderSource = shaderSource + "finalColor.xyz = input.fogDist * finalColor.xyz + (1.0 - input.fogDist)*FogColor.xyz;\n";

		}

		shaderSource = shaderSource + "return finalColor;\n";
		shaderSource = shaderSource + "}\n";
		return shaderSource;
	}
	//---------------------------------------------------------------------
	FixedFuncPrograms * HlslFixedFuncEmuShaderGenerator::createFixedFuncPrograms()
	{
		return new HlslFixedFuncPrograms();
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	Hlsl4FixedFuncEmuShaderGenerator::Hlsl4FixedFuncEmuShaderGenerator()
	{
		mLanguageName	= "hlsl";
		mName			= "hlsl4";
		mVpTarget		= "vs_4_0";
		mFpTarget		= "ps_4_0";
	}
	//---------------------------------------------------------------------
	Hlsl4FixedFuncEmuShaderGenerator::~Hlsl4FixedFuncEmuShaderGenerator()
	{

	}
	//---------------------------------------------------------------------
	const String Hlsl4FixedFuncEmuShaderGenerator::getShaderSource( const String & vertexProgramName, const String & fragmentProgramName, const VertexBufferDeclaration & vertexBufferDeclaration, FixedFuncState & fixedFuncState )
	{
		return HlslFixedFuncEmuShaderGenerator::getShaderSource(vertexProgramName, fragmentProgramName, vertexBufferDeclaration, fixedFuncState);
	}
	//---------------------------------------------------------------------
	void HlslFixedFuncEmuShaderGenerator::HlslFixedFuncPrograms::setFixedFuncProgramsParameters( const FixedFuncProgramsParameters & params )
	{
		_setProgramMatrix4Parameter(GPT_VERTEX_PROGRAM, "World", params.getWorldMat());
		_setProgramMatrix4Parameter(GPT_VERTEX_PROGRAM, "View", params.getViewMat());
		_setProgramMatrix4Parameter(GPT_VERTEX_PROGRAM, "Projection", params.getProjectionMat());

		_setProgramMatrix4Parameter(GPT_VERTEX_PROGRAM, "ViewIT", params.getViewMat().inverse().transpose());


		Matrix4 WorldViewIT = params.getWorldMat() * params.getViewMat();
		WorldViewIT = WorldViewIT.inverse().transpose();
		_setProgramMatrix4Parameter(GPT_VERTEX_PROGRAM, "WorldViewIT", WorldViewIT);


		if (params.getLightingEnabled() && params.getLights().size() > 0)
		{
			_setProgramColorParameter(GPT_VERTEX_PROGRAM, "BaseLightAmbient", params.getLightAmbient());

			for(size_t i = 0 ; i < params.getLights().size() ; i++)
			{
				Light * curLight = params.getLights()[i];
				String prefix = "Light" + StringConverter::toString(i) + "_";

				_setProgramColorParameter(GPT_VERTEX_PROGRAM, prefix + "Ambient", ColourValue::Black);
				_setProgramColorParameter(GPT_VERTEX_PROGRAM, prefix + "Diffuse", curLight->getDiffuseColour());
				_setProgramColorParameter(GPT_VERTEX_PROGRAM, prefix + "Specular", curLight->getSpecularColour());		

				switch (curLight->getType())
				{
				case Light::LT_POINT:
					{
						_setProgramVector3Parameter(GPT_VERTEX_PROGRAM, prefix + "Position", curLight->getPosition());
						_setProgramFloatParameter(GPT_VERTEX_PROGRAM, prefix + "Range", curLight->getAttenuationRange());

						Vector3 attenuation;
						attenuation[0] = curLight->getAttenuationConstant();
						attenuation[1] = curLight->getAttenuationLinear();
						attenuation[2] = curLight->getAttenuationQuadric();
						_setProgramVector3Parameter(GPT_VERTEX_PROGRAM, prefix + "Attenuation", attenuation);
					}
					break;
				case Light::LT_DIRECTIONAL:
					_setProgramVector3Parameter(GPT_VERTEX_PROGRAM, prefix + "Direction", curLight->getDirection());

					break;
				case Light::LT_SPOTLIGHT:
					{

						_setProgramVector3Parameter(GPT_VERTEX_PROGRAM, prefix + "Direction", curLight->getDirection());
						_setProgramVector3Parameter(GPT_VERTEX_PROGRAM, prefix + "Position", curLight->getPosition());

						Vector3 attenuation;
						attenuation[0] = curLight->getAttenuationConstant();
						attenuation[1] = curLight->getAttenuationLinear();
						attenuation[2] = curLight->getAttenuationQuadric();
						_setProgramVector3Parameter(GPT_VERTEX_PROGRAM, prefix + "Attenuation", attenuation);

						Vector3 spot;
						spot[0] = curLight->getSpotlightInnerAngle().valueRadians() ;
						spot[1] = curLight->getSpotlightOuterAngle().valueRadians();
						spot[2] = curLight->getSpotlightFalloff();
						_setProgramVector3Parameter(GPT_VERTEX_PROGRAM, prefix + "Spot", spot);					
					}
					break;
				} // end of - switch (curLight->getType())
			} // end of - for(size_t i = 0 ; i < params.getLights().size() ; i++) 
		} // end of -  if (params.getLightingEnabled())





		switch (params.getFogMode())
		{
		case FOG_NONE:
			break;
		case FOG_EXP:
		case FOG_EXP2:
			_setProgramFloatParameter(GPT_VERTEX_PROGRAM, "FogDensity", params.getFogDensitiy());
			break;
		case FOG_LINEAR:
			_setProgramFloatParameter(GPT_VERTEX_PROGRAM, "FogStart", params.getFogStart());
			_setProgramFloatParameter(GPT_VERTEX_PROGRAM, "FogEnd", params.getFogEnd());
			break;
		}

		if (params.getFogMode() != FOG_NONE)
		{
			_setProgramColorParameter(GPT_FRAGMENT_PROGRAM, "FogColor", params.getFogColour());
		}


		for(size_t i = 0 ; i < params.getTextureMatrices().size() ; i++)
		{
			if (params.isTextureStageEnabled(i))
			{
				_setProgramMatrix4Parameter(GPT_VERTEX_PROGRAM, "TextureMatrix" + StringConverter::toString(i), params.getTextureMatrices()[i]);
			}
		}



	}
}
