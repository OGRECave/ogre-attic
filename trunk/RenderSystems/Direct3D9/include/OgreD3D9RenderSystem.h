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
#ifndef __D3D9RENDERSYSTEM_H__
#define __D3D9RENDERSYSTEM_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "OgreRenderSystem.h"
#include "OgreD3D9HWBuffers.h"
#include "OgreD3D9Mappings.h"

#include "OgreNoMemoryMacros.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include "OgreMemoryMacros.h"

namespace Ogre 
{
#define MAX_LIGHTS 8
#define D3D_MAX_DECLSIZE 26

	class D3D9DriverList;
	class D3D9Driver;

	/**
	Implementation of DirectX9 as a rendering system.
	*/
	class D3D9RenderSystem : public RenderSystem
	{
	private:
		/// Direct3D
		LPDIRECT3D9			mpD3D;
		/// Direct3D rendering device
		LPDIRECT3DDEVICE9	mpD3DDevice;
		
		/// dynamic vertex buffers manager
		D3D9DynVBManager *mDVBMgr;
		/// dynamic index buffers manager
		D3D9DynIBManager *mDIBMgr;

		// Stored options
		ConfigOptionMap mOptions;

        /// wait for vsync
		bool mVSync;
		/// full-screen multisampling antialiasing type
		D3DMULTISAMPLE_TYPE mFSAAType;
		/// full-screen multisampling antialiasing level
		DWORD mFSAAQuality;

		/// external window handle ;)
		HWND mExternalHandle;
		/// instance
		HINSTANCE mhInstance;

		/// List of D3D drivers installed (video cards)
		D3D9DriverList* mDriverList;
		/// Currently active driver
		D3D9Driver* mActiveD3DDriver;
		/// Device caps.
		D3DCAPS9 mCaps;

		/// structure holding texture unit settings for every stage
		struct sD3DTextureStageDesc
		{
			/// the type of the texture
			D3D9Mappings::eD3DTexType texType;
			/// wich texCoordIndex to use
			int coordIndex;
			/// type of auto tex. calc. used
			TexCoordCalcMethod autoTexCoordType;
			/// texture 
			IDirect3DBaseTexture9 *pTex;
		} mTexStageDesc[OGRE_MAX_TEXTURE_LAYERS];

		// With a quick bit of adding up, I cannot see our vertex shader declaration being larger then 26 items
		D3DVERTEXELEMENT9 mCurrentDecl[D3D_MAX_DECLSIZE];
		LPDIRECT3DVERTEXDECLARATION9 mpCurrentVertexDecl;

		// Array of up to 8 lights, indexed as per API
		// Note that a null value indeicates a free slot
		Light* mLights[MAX_LIGHTS];

		D3D9DriverList* getDirect3DDrivers(void);
		void refreshD3DSettings(void);

		inline bool compareDecls( D3DVERTEXELEMENT9* pDecl1, D3DVERTEXELEMENT9* pDecl2, int size );

		// Matrix conversion
		D3DXMATRIX makeD3DXMatrix( const Matrix4& mat );
		Matrix4 convertD3DXMatrix( const D3DXMATRIX& mat );

		void initInputDevices(void);
		void processInputDevices(void);
		void setD3D9Light( int index, Light* light );
		
		// state management methods, very primitive !!!
		HRESULT __SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
		HRESULT __SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);
		HRESULT __SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);

		/// return anisotropy level
		DWORD _getCurrentAnisotropy(int unit);
		/// check if a FSAA is supported
		bool _checkMultiSampleQuality(D3DMULTISAMPLE_TYPE type, DWORD *outQuality, D3DFORMAT format, UINT adapterNum, D3DDEVTYPE deviceType, BOOL fullScreen);
		/// set FSAA
		void _setFSAA(D3DMULTISAMPLE_TYPE type, DWORD qualityLevel);
		
		/// findout if a tex. stage use the coord.index specified
		bool _isCoordIndexInUse(int index)
		{
			for (int n = 0; n < OGRE_MAX_TEXTURE_LAYERS; n++)
			{
				if (mTexStageDesc[n].pTex > 0 &&
					mTexStageDesc[n].autoTexCoordType == TEXCALC_NONE &&
					mTexStageDesc[n].coordIndex == index)
					return true;
			}
			return false;
		}

		/// return the D3D tex.coord.dim FVF ID for a tex.coord.dim.
		D3DDECLTYPE _texCoordDimToDeclType(int coordDim)
		{
			D3DDECLTYPE ret;
			switch (coordDim)
			{
			case 1:
				ret = D3DDECLTYPE_FLOAT1;
				break;
			case 2:
				ret = D3DDECLTYPE_FLOAT2;
				break;
			case 3:
				ret = D3DDECLTYPE_FLOAT3;
				break;
			case 4:
				ret = D3DDECLTYPE_FLOAT4;
				break;
			default:
				Except( Exception::ERR_INVALIDPARAMS, "Invalid tex.coord.dimensions", "D3D9RenderSystem::_texCoordDimToDeclType" );
				break;
			}
			return ret;
		}

	public:
		// constructor
		D3D9RenderSystem( HINSTANCE hInstance );
		// destructor
		~D3D9RenderSystem();

		virtual initConfigOptions(void);
		// method for resizing/repositing the render window
 		virtual ResizeRepositionWindow(HWND wich);
		// method for setting external window hwnd
		void SetExternalWindowHandle(HWND externalHandle){mExternalHandle = externalHandle;};

		// Overridden RenderSystem functions
		ConfigOptionMap& getConfigOptions(void);
		String validateConfigOptions(void);
		RenderWindow* initialise( bool autoCreateWindow );
		RenderWindow* createRenderWindow(const String &name, int width, int height, int colourDepth, bool fullScreen, int left = 0, int top = 0, bool depthBuffer = true, RenderWindow* parentWindowHandle = 0);
		RenderTexture * createRenderTexture( const String & name, int width, int height );
		String getErrorDescription( long errorNumber );
		const String& getName(void) const;
		// Low-level overridden members
		void setConfigOption( const String &name, const String &value );
		void reinitialise();
		void shutdown();
		void startRendering();
		void setAmbientLight( float r, float g, float b );
		void setShadingType( ShadeOptions so );
		void setLightingEnabled( bool enabled );
		void destroyRenderWindow( RenderWindow* pWin );
		void convertColourValue( const ColourValue& colour, unsigned long* pDest );
		void setStencilCheckEnabled(bool enabled);
		void setStencilBufferFunction(CompareFunction func);
		void setStencilBufferReferenceValue(ulong refValue);
		void setStencilBufferMask(ulong mask);
		void setStencilBufferFailOperation(StencilOperation op);
		void setStencilBufferDepthFailOperation(StencilOperation op);
		void setStencilBufferPassOperation(StencilOperation op);

		// Low-level overridden members, mainly for internal use
		void _addLight( Light* lt );
		void _removeLight( Light* lt );
		void _modifyLight( Light* lt );
		void _removeAllLights(void);
		void _pushRenderState(void);
		void _popRenderState(void);
		void _setWorldMatrix( const Matrix4 &m );
		void _setViewMatrix( const Matrix4 &m );
		void _setProjectionMatrix( const Matrix4 &m );
		void _setSurfaceParams( const ColourValue &ambient, const ColourValue &diffuse, const ColourValue &specular, const ColourValue &emissive, Real shininess );
		void _setTexture( int unit, bool enabled, const String &texname );
        void _setTextureCoordSet( int stage, int index );
        void _setTextureCoordCalculation(int unit, TexCoordCalcMethod m);
		void _setTextureBlendMode( int stage, const LayerBlendModeEx& bm );
		void _setTextureAddressingMode( int stage, Material::TextureLayer::TextureAddressingMode tam );
		void _setTextureMatrix( int stage, const Matrix4 &xform );
		void _setSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor );
		void _setAlphaRejectSettings( CompareFunction func, unsigned char value );
		void _setViewport( Viewport *vp );
		void _beginFrame(void);
		void _render(const LegacyRenderOperation &op );
		void _endFrame(void);
		void _setCullingMode( CullingMode mode );
		void _setDepthBufferParams( bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL );
		void _setDepthBufferCheckEnabled( bool enabled = true );
		void _setDepthBufferWriteEnabled(bool enabled = true);
		void _setDepthBufferFunction( CompareFunction func = CMPF_LESS_EQUAL );
		void _setDepthBias(ushort bias);
		void _setFog( FogMode mode = FOG_NONE, ColourValue colour = ColourValue::White, Real expDensity = 1.0, Real linearStart = 0.0, Real linearEnd = 1.0 );
		void _makeProjectionMatrix(Real fovy, Real aspect, Real nearPlane, Real farPlane, Matrix4& dest);
		void _setRasterisationMode(SceneDetailLevel level);
		void _setTextureLayerFiltering(int unit, const TextureFilterOptions texLayerFilterOps);
		void _setTextureLayerAnisotropy(int unit, int maxAnisotropy);
		void setVertexDeclaration(VertexDeclaration* decl);
		void setVertexBufferBinding(VertexBufferBinding* binding);
        void _render(const RenderOperation& op);

	};
}
#endif
