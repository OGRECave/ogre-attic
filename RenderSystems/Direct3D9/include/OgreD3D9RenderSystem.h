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
#include "OgreD3D9Mappings.h"

#include "OgreNoMemoryMacros.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include "OgreMemoryMacros.h"

namespace Ogre 
{
#define MAX_LIGHTS 8

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
			size_t coordIndex;
			/// type of auto tex. calc. used
			TexCoordCalcMethod autoTexCoordType;
			/// texture 
			IDirect3DBaseTexture9 *pTex;
		} mTexStageDesc[OGRE_MAX_TEXTURE_LAYERS];

		// Array of up to 8 lights, indexed as per API
		// Note that a null value indeicates a free slot
		Light* mLights[MAX_LIGHTS];

		D3D9DriverList* getDirect3DDrivers(void);
		void refreshD3DSettings(void);

		inline bool compareDecls( D3DVERTEXELEMENT9* pDecl1, D3DVERTEXELEMENT9* pDecl2, size_t size );


		void initInputDevices(void);
		void processInputDevices(void);
		void setD3D9Light( size_t index, Light* light );
		
		// state management methods, very primitive !!!
		HRESULT __SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
		HRESULT __SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);
		HRESULT __SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);

		/// return anisotropy level
		DWORD _getCurrentAnisotropy(size_t unit);
		/// check if a FSAA is supported
		bool _checkMultiSampleQuality(D3DMULTISAMPLE_TYPE type, DWORD *outQuality, D3DFORMAT format, UINT adapterNum, D3DDEVTYPE deviceType, BOOL fullScreen);
		/// set FSAA
		void _setFSAA(D3DMULTISAMPLE_TYPE type, DWORD qualityLevel);
		
		D3D9HardwareBufferManager* mHardwareBufferManager;
		D3D9GpuProgramManager* mGpuProgramManager;
        D3D9HLSLProgramFactory* mHLSLProgramFactory;

		size_t mLastVertexSourceCount;


        /// Internal method for populating the capabilities structure
        void initCapabilities(void);

        void convertVertexShaderCaps(void);
        void convertPixelShaderCaps(void);

        unsigned short mCurrentLights;


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
		RenderWindow* initialise( bool autoCreateWindow, const String& windowTitle = "OGRE Render Window"  );
		RenderWindow* createRenderWindow(const String &name, unsigned int width, unsigned int height, unsigned int colourDepth, bool fullScreen, int left = 0, int top = 0, bool depthBuffer = true, RenderWindow* parentWindowHandle = 0);
		RenderTexture * createRenderTexture( const String & name, unsigned int width, unsigned int height );
		String getErrorDescription( long errorNumber );
		const String& getName(void) const;
		// Low-level overridden members
		void setConfigOption( const String &name, const String &value );
		void reinitialise();
		void shutdown();
		void setAmbientLight( float r, float g, float b );
		void setShadingType( ShadeOptions so );
		void setLightingEnabled( bool enabled );
		void destroyRenderWindow( RenderWindow* pWin );
		void convertColourValue( const ColourValue& colour, unsigned long* pDest );
		void setStencilCheckEnabled(bool enabled);
        void setStencilBufferParams(CompareFunction func = CMPF_ALWAYS_PASS, 
            ulong refValue = 0, ulong mask = 0xFFFFFFFF, 
            StencilOperation stencilFailOp = SOP_KEEP, 
            StencilOperation depthFailOp = SOP_KEEP,
            StencilOperation passOp = SOP_KEEP, 
            bool twoSidedOperation = false);
        void setNormaliseNormals(bool normalise);

		// Low-level overridden members, mainly for internal use
        void _useLights(const LightList& lights, unsigned short limit);
		void _setWorldMatrix( const Matrix4 &m );
		void _setViewMatrix( const Matrix4 &m );
		void _setProjectionMatrix( const Matrix4 &m );
		void _setSurfaceParams( const ColourValue &ambient, const ColourValue &diffuse, const ColourValue &specular, const ColourValue &emissive, Real shininess );
		void _setTexture( size_t unit, bool enabled, const String &texname );
        void _setTextureCoordSet( size_t unit, size_t index );
        void _setTextureCoordCalculation(size_t unit, TexCoordCalcMethod m);
		void _setTextureBlendMode( size_t unit, const LayerBlendModeEx& bm );
		void _setTextureAddressingMode( size_t unit, TextureUnitState::TextureAddressingMode tam );
		void _setTextureMatrix( size_t unit, const Matrix4 &xform );
		void _setSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor );
		void _setAlphaRejectSettings( CompareFunction func, unsigned char value );
		void _setViewport( Viewport *vp );
		void _beginFrame(void);
		void _endFrame(void);
		void _setCullingMode( CullingMode mode );
		void _setDepthBufferParams( bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL );
		void _setDepthBufferCheckEnabled( bool enabled = true );
		void _setColourBufferWriteEnabled(bool red, bool green, bool blue, bool alpha);
		void _setDepthBufferWriteEnabled(bool enabled = true);
		void _setDepthBufferFunction( CompareFunction func = CMPF_LESS_EQUAL );
		void _setDepthBias(ushort bias);
		void _setFog( FogMode mode = FOG_NONE, const ColourValue& colour = ColourValue::White, Real expDensity = 1.0, Real linearStart = 0.0, Real linearEnd = 1.0 );
		void _makeProjectionMatrix(Real fovy, Real aspect, Real nearPlane, Real farPlane, 
            Matrix4& dest, bool forGpuProgram = false);
		void _makeProjectionMatrix(Real left, Real right, Real bottom, Real top, Real nearPlane, 
            Real farPlane, Matrix4& dest, bool forGpuProgram = false);
		void _setRasterisationMode(SceneDetailLevel level);
        void _setTextureUnitFiltering(size_t unit, FilterType ftype, FilterOptions filter);
		void _setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy);
		void setVertexDeclaration(VertexDeclaration* decl);
		void setVertexBufferBinding(VertexBufferBinding* binding);
        void _render(const RenderOperation& op);
        void bindGpuProgram(GpuProgram* prg);
        void unbindGpuProgram(GpuProgramType gptype);
        void bindGpuProgramParameters(GpuProgramType gptype, GpuProgramParametersSharedPtr params);
        void setScissorTest(bool enabled, size_t left = 0, size_t top = 0, size_t right = 800, size_t bottom = 600);
        void clearFrameBuffer(unsigned int buffers, 
            const ColourValue& colour = ColourValue::Black, 
            Real depth = 1.0f, unsigned short stencil = 0);
		void setClipPlane (ushort index, Real A, Real B, Real C, Real D);
		void enableClipPlane (ushort index, bool enable);
	};
}
#endif
