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
#include "OgreD3D10VertexDeclaration.h"
#include "OgreD3D10Mappings.h"
#include "OgreD3D10HLSLProgram.h"
#include "OgreD3D10Device.h"


namespace Ogre {

	//-----------------------------------------------------------------------
	D3D10VertexDeclaration::D3D10VertexDeclaration(D3D10Device &  device) 
		: mlpD3DDevice(device),  mNeedsRebuild(true)
	{

	}
	//-----------------------------------------------------------------------
	D3D10VertexDeclaration::~D3D10VertexDeclaration()
	{
		ShaderToILayoutMapIterator iter = mShaderToILayoutMap.begin();
		ShaderToILayoutMapIterator iterE = mShaderToILayoutMap.end();

		for ( ; iter != iterE ; iter++)
		{
			iter->second->Release();
		}


	}
	//-----------------------------------------------------------------------
	const VertexElement& D3D10VertexDeclaration::addElement(unsigned short source, 
		size_t offset, VertexElementType theType,
		VertexElementSemantic semantic, unsigned short index)
	{
		mNeedsRebuild = true;
		return VertexDeclaration::addElement(source, offset, theType, semantic, index);
	}
	//-----------------------------------------------------------------------------
	const VertexElement& D3D10VertexDeclaration::insertElement(unsigned short atPosition,
		unsigned short source, size_t offset, VertexElementType theType,
		VertexElementSemantic semantic, unsigned short index)
	{
		mNeedsRebuild = true;
		return VertexDeclaration::insertElement(atPosition, source, offset, theType, semantic, index);
	}
	//-----------------------------------------------------------------------
	void D3D10VertexDeclaration::removeElement(unsigned short elem_index)
	{
		VertexDeclaration::removeElement(elem_index);
		mNeedsRebuild = true;
	}
	//-----------------------------------------------------------------------
	void D3D10VertexDeclaration::removeElement(VertexElementSemantic semantic, unsigned short index)
	{
		VertexDeclaration::removeElement(semantic, index);
		mNeedsRebuild = true;
	}
	//-----------------------------------------------------------------------
	void D3D10VertexDeclaration::removeAllElements(void)
	{
		VertexDeclaration::removeAllElements();
		mNeedsRebuild = true;
	}
	//-----------------------------------------------------------------------
	void D3D10VertexDeclaration::modifyElement(unsigned short elem_index, 
		unsigned short source, size_t offset, VertexElementType theType,
		VertexElementSemantic semantic, unsigned short index)
	{
		VertexDeclaration::modifyElement(elem_index, source, offset, theType, semantic, index);
		mNeedsRebuild = true;
	}
	//-----------------------------------------------------------------------
	D3D10_INPUT_ELEMENT_DESC * D3D10VertexDeclaration::getD3DVertexDeclaration(void)
	{
		// Create D3D elements
		size_t iNumElements = mElementList.size();

		//SAFE_DELETE_ARRAY(mD3delems);

		D3D10_INPUT_ELEMENT_DESC*  D3delems = new D3D10_INPUT_ELEMENT_DESC[iNumElements];

		VertexElementList::const_iterator i, iend;
		unsigned int idx;
		iend = mElementList.end();
		for (idx = 0, i = mElementList.begin(); i != iend; ++i, ++idx)
		{
			D3delems[idx].SemanticName			= D3D10Mappings::get(i->getSemantic()); 
			D3delems[idx].SemanticIndex		= i->getIndex();
			D3delems[idx].Format				= D3D10Mappings::get(i->getType());
			D3delems[idx].InputSlot			= i->getSource();
			D3delems[idx].AlignedByteOffset	= static_cast<WORD>(i->getOffset());
			D3delems[idx].InputSlotClass		= D3D10_INPUT_PER_VERTEX_DATA;
			D3delems[idx].InstanceDataStepRate	= 0;

			/*// NB force index if colours since D3D uses the same usage for 
			// diffuse & specular
			if (i->getSemantic() == VES_SPECULAR)
			{
			mD3delems[idx].InputSlot = 1;
			}
			else if (i->getSemantic() == VES_DIFFUSE)
			{
			mD3delems[idx].InputSlot = 0;
			}
			else
			{
			mD3delems[idx].InputSlot = i->getIndex();
			}*/
		}
		// no terminator needed 






		//delete [] d3delems; // todo - delete it....



		return D3delems;
	}
	//-----------------------------------------------------------------------
	ID3D10InputLayout*  D3D10VertexDeclaration::getILayoutByShader(D3D10HLSLProgram* boundVertexProgram)
	{
		ShaderToILayoutMapIterator foundIter = mShaderToILayoutMap.find(boundVertexProgram);

		ID3D10InputLayout*  pVertexLayout = 0; 

		if (foundIter == mShaderToILayoutMap.end())
		{
			// if not found - create

			DWORD dwShaderFlags = 0;
			ID3D10Blob* pVSBuf = boundVertexProgram->getMicroCode();

			static int count = 0;
			count++;
			D3D10_INPUT_ELEMENT_DESC * elementDesc = getD3DVertexDeclaration();
			size_t elementCount = getElementCount();

			HRESULT hr = mlpD3DDevice->CreateInputLayout( 
				elementDesc, 
				(UINT)elementCount, 
				pVSBuf->GetBufferPointer(), 
				pVSBuf->GetBufferSize(),
				&pVertexLayout );

			SAFE_DELETE_ARRAY(elementDesc);



			if (FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set D3D10 vertex declaration", 
					"D3D10VertexDeclaration::BindToShader");
			}

			mShaderToILayoutMap[boundVertexProgram] = pVertexLayout;

		}
		else
		{
			pVertexLayout = foundIter->second;
		}

		return pVertexLayout;
	}
	//-----------------------------------------------------------------------
	void D3D10VertexDeclaration::bindToShader(D3D10HLSLProgram* boundVertexProgram)
	{
		ID3D10InputLayout*  pVertexLayout = getILayoutByShader(boundVertexProgram);


		// Set the input layout
		mlpD3DDevice->IASetInputLayout( pVertexLayout );
	}	
	//-----------------------------------------------------------------------
	HighLevelGpuProgramPtr D3D10VertexDeclaration::getFixFuncVs()
	{
		if(mVs.isNull())
		{
			bool bHasColor = hasColor();
			bool bHasTexcoord = hasTexcoord();

			String shaderSource = "";

			shaderSource = shaderSource + "struct VS_INPUT { ";

			uint8 semanticCount[100];
			ZeroMemory(semanticCount, sizeof(uint8) * 100);
			for (unsigned short i = 0 ; i < getElementCount() ; i++)
			{
				VertexElementSemantic semantic = getElement(i)->getSemantic();
				VertexElementType type = getElement(i)->getType();

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
					parameterType = "float4"; // position must be float4 (and not float3 like in the buffer)
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



			shaderSource = shaderSource + "float4x4  FullPosTransMatrix;\n";
			//shaderSource = shaderSource + "float4x4  World;\n";
			//shaderSource = shaderSource + "float4x4  View;\n";
			//shaderSource = shaderSource + "float4x4  Projection;\n";

			
			shaderSource = shaderSource + "struct VS_OUTPUT\n";
			shaderSource = shaderSource + "{\n";
			shaderSource = shaderSource + "float4 Pos : SV_POSITION;\n";
			if (bHasTexcoord)
			{
				shaderSource = shaderSource + "float2 tCord : TEXCOORD;\n";
			}
			if (bHasColor)
			{
				shaderSource = shaderSource + "float4 col : COLOR;\n";
			}
			shaderSource = shaderSource + "};\n";
			shaderSource = shaderSource + "VS_OUTPUT VS( VS_INPUT input )\n";
			shaderSource = shaderSource + "{\n";
			shaderSource = shaderSource + "VS_OUTPUT output = (VS_OUTPUT)0;\n";
			shaderSource = shaderSource + "output.Pos = mul( input.Position0, FullPosTransMatrix );\n";
			//shaderSource = shaderSource + "output.Pos = mul( input.Position0, World );\n";
			//shaderSource = shaderSource + "output.Pos = mul( output.Pos, View );\n";
			//shaderSource = shaderSource + "output.Pos = mul( output.Pos, Projection );\n";	
			if(bHasTexcoord)
			{
				shaderSource = shaderSource + "output.tCord = input.Texcoord0;\n";		
			}
			if (bHasColor)
			{
				shaderSource = shaderSource + "output.col.x = ((input.DiffuseColor0 >> 24) & 0xFF) / 255.0f;\n";
				shaderSource = shaderSource + "output.col.y = ((input.DiffuseColor0 >> 16) & 0xFF) / 255.0f;\n"; 
				shaderSource = shaderSource + "output.col.z = ((input.DiffuseColor0 >> 8) & 0xFF) / 255.0f;\n";
				shaderSource = shaderSource + "output.col.w = (input.DiffuseColor0 & 0xFF) / 255.0f;\n";
			}
			shaderSource = shaderSource + "return output;}\n";

			static size_t vsCount = 0;
			vsCount++;
			mVs = HighLevelGpuProgramManager::getSingleton().
				createProgram("D3D10VertexDeclaration_VS_" + StringConverter::toString(vsCount), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
				"hlsl", GPT_VERTEX_PROGRAM);	
			mVs->setSource(shaderSource);
			static_cast<D3D10HLSLProgram*>(mVs.get())->setEntryPoint("VS");
			static_cast<D3D10HLSLProgram*>(mVs.get())->setTarget("vs_4_0");
			static_cast<D3D10HLSLProgram*>(mVs.get())->loadFromSource();

			mVsParams = mVs->createParameters();
		}
		return mVs;
	}
	//-----------------------------------------------------------------------
	HighLevelGpuProgramPtr D3D10VertexDeclaration::getFixFuncFs()
	{
		if(mFs.isNull())
		{
			bool bHasColor = hasColor();
			bool bHasTexcoord = hasTexcoord();

			String shaderSource = "";
			shaderSource = shaderSource + "float4x4  TextureMatrix;\n";
			shaderSource = shaderSource + "float  LightingEnabled;\n";

			if(bHasColor &bHasTexcoord)
			{
				shaderSource = shaderSource + "sampler tex0 : register(s0);\n";
				shaderSource = shaderSource + "float4 PS( float4 Pos : SV_POSITION, float2 tCord : TEXCOORD, float4 col : COLOR ) : SV_Target\n";
				shaderSource = shaderSource + "{\n";
				shaderSource = shaderSource + "float4 texCordWithMatrix = float4(tCord.x, tCord.y, 0, 1);\n";
				shaderSource = shaderSource + "texCordWithMatrix = mul( texCordWithMatrix, TextureMatrix );\n";
				shaderSource = shaderSource + "float4 finalColor = max((float4(1.0,1.0,1.0,1.0) * (LightingEnabled)), col);\n";
				shaderSource = shaderSource + "return tex2D(tex0,texCordWithMatrix.xy) * finalColor;\n";
				shaderSource = shaderSource + "}";
			}
			else if(bHasTexcoord)
			{
				shaderSource = shaderSource + "sampler tex0 : register(s0);\n";
				shaderSource = shaderSource + "float4 PS( float4 Pos : SV_POSITION, float2 tCord : TEXCOORD) : SV_Target\n";
				shaderSource = shaderSource + "{\n";
				shaderSource = shaderSource + "float4 texCordWithMatrix = float4(tCord.x, tCord.y, 0, 1);\n";
				shaderSource = shaderSource + "texCordWithMatrix = mul( texCordWithMatrix, TextureMatrix );\n";
				shaderSource = shaderSource + "\n";
				shaderSource = shaderSource + "return tex2D(tex0,texCordWithMatrix.xy);\n";
				shaderSource = shaderSource + "}";
			}
			else if(bHasColor)
			{
				shaderSource = shaderSource + "float4 PS( float4 Pos : SV_POSITION, float4 col : COLOR) : SV_Target\n";
				shaderSource = shaderSource + "{\n";
				shaderSource = shaderSource + "float4 finalColor = max((float4(1.0,1.0,1.0,1.0) * (LightingEnabled)), col);\n";
				shaderSource = shaderSource + "{\nreturn finalColor;\n}";
				shaderSource = shaderSource + "}\n";
			}
			else 
			{
				shaderSource = shaderSource + "float4 PS( float4 Pos : SV_POSITION) : SV_Target\n";
				shaderSource = shaderSource + "{\nreturn float4(1.0, 1.0, 1.0, 1.0);\n}";
			}

			static size_t fsCount = 0;
			fsCount++;
			mFs = HighLevelGpuProgramManager::getSingleton().
				createProgram("D3D10VertexDeclaration_FS_" + StringConverter::toString(fsCount), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
				"hlsl", GPT_FRAGMENT_PROGRAM);	
			mFs->setSource(shaderSource);
			static_cast<D3D10HLSLProgram*>(mFs.get())->setEntryPoint("PS");
			static_cast<D3D10HLSLProgram*>(mFs.get())->setTarget("ps_4_0");
			static_cast<D3D10HLSLProgram*>(mFs.get())->loadFromSource();
		
			mFsParams = mFs->createParameters();
		}


		return mFs;
	}
	//-----------------------------------------------------------------------
	bool D3D10VertexDeclaration::hasColor()
	{
		return countVertexElementSemantic(VES_DIFFUSE) > 0;
	}
	//-----------------------------------------------------------------------
	bool D3D10VertexDeclaration::hasTexcoord()
	{
		return countVertexElementSemantic(VES_TEXTURE_COORDINATES)  > 0;
	}
	//-----------------------------------------------------------------------
	unsigned short D3D10VertexDeclaration::numberOfTexcoord()
	{
		return countVertexElementSemantic(VES_TEXTURE_COORDINATES);
	}
	//-----------------------------------------------------------------------
	unsigned short D3D10VertexDeclaration::countVertexElementSemantic( VertexElementSemantic semantic )
	{
		unsigned short res = 0;
		for (unsigned short i = 0 ; i < getElementCount() ; i++)
		{
			if(getElement(i)->getSemantic() == semantic)
			{
				res++;
			}
		}

		return res;

	}
	//-----------------------------------------------------------------------
	Ogre::GpuProgramParametersSharedPtr D3D10VertexDeclaration::getFixFuncVsParams()
	{
		return mVsParams;
	}
	//-----------------------------------------------------------------------
	Ogre::GpuProgramParametersSharedPtr D3D10VertexDeclaration::getFixFuncFsParams()
	{
		return mFsParams;
	}
	//-----------------------------------------------------------------------
}

