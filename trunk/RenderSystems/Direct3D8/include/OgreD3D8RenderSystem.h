/*
Put in disclaimer
*/
#ifndef __D3D8RENDERSYSTEM_H__
#define __D3D8RENDERSYSTEM_H__

// Precompiler options
#include "OgreD3D8Prerequisites.h"
#include "OgreString.h"

#include "OgreRenderSystem.h"

// Include D3D files
#include "OgreNoMemoryMacros.h"
#include <d3d8.h>
#include <d3dx8.h>
#include <dxerr8.h>
#include "OgreMemoryMacros.h"

namespace Ogre {

	class D3D8DriverList;
	class D3D8Driver;

	struct D3D8VertexBuffer
	{
		LPDIRECT3DVERTEXBUFFER8 buffer;
		UINT count;
	};
	struct D3D8IndexBuffer
	{
		LPDIRECT3DINDEXBUFFER8 buffer;
		UINT count;
	};
	/**
	  Implementation of DirectX8 as a rendering system.
	 */
	class D3D8RenderSystem : public RenderSystem
	{
	private:

		// Direct3D rendering device
		// Only created after top-level window created
		LPDIRECT3D8			mpD3D;
		LPDIRECT3DDEVICE8	mpD3DDevice;
		
		// List of D3D drivers installed (video cards)
		// Enumerates itself
		D3D8DriverList* mDriverList;
		// Currently active driver
		D3D8Driver* mActiveD3DDriver;

		D3DCAPS8 mCaps;

		BYTE* mpRenderBuffer;
		DWORD mRenderBufferSize;

		// Vertex buffers.  Currently for rendering we need to place all the data
		// that we receive into one of the following vertex buffers (one for each 
		// component type).
		D3D8VertexBuffer mpXYZBuffer;
		D3D8VertexBuffer mpNormalBuffer;
		D3D8VertexBuffer mpDiffuseBuffer;
		D3D8VertexBuffer mpSpecularBuffer;
		D3D8VertexBuffer mpTextures[OGRE_MAX_TEXTURE_LAYERS][4]; // max 8 textures with max 4 units per texture
		UINT mStreamsInUse;
		D3D8IndexBuffer mpIndicies;

		/// enum identifying D3D9 tex. types
		enum eD3DTexType
		{
			/// standard texture
			D3D_TEX_TYPE_NORMAL,
			/// cube texture
			D3D_TEX_TYPE_CUBE,
			/// volume texture
			D3D_TEX_TYPE_VOLUME
		};

		/// return the D3DtexType equivalent of a Ogre tex. type
		eD3DTexType _ogreTexTypeToD3DTexType(TextureType ogreTexType)
		{
			eD3DTexType ret;
			switch (ogreTexType)
			{
			case TEX_TYPE_1D :
			case TEX_TYPE_2D :
				ret = D3D_TEX_TYPE_NORMAL;
				break;
			case TEX_TYPE_CUBE_MAP :
				ret = D3D_TEX_TYPE_CUBE;
				break;
			default :
				Except( Exception::ERR_INVALIDPARAMS, "Invalid tex.type", "D3D9RenderSystem::_ogreTexTypeToD3DTexType" );
				break;
			}
			return ret;
		}

		/// structure holding texture unit settings for every stage
		struct sD3DTextureStageDesc
		{
			/// the type of the texture
			eD3DTexType texType;
			/// wich texCoordIndex to use
			int coordIndex;
			/// type of auto tex. calc. used
			TexCoordCalcMethod autoTexCoordType;
			/// texture, if it's 0/NULL the tex layer is disabled
			IDirect3DBaseTexture8 *pTex;
		} mTexStageDesc[OGRE_MAX_TEXTURE_LAYERS];

		// With a quick bit of adding up, I cannot see our vertex shader declaration being larger then 26 items
		#define D3D_MAX_DECLSIZE 26
		DWORD mCurrentDecl[D3D_MAX_DECLSIZE];
		DWORD mhCurrentShader;

		// Array of up to 8 lights, indexed as per API
		// Note that a null value indeicates a free slot
		#define MAX_LIGHTS 8
		Light* mLights[MAX_LIGHTS];

		HINSTANCE mhInstance;

		// Stored options
		ConfigOptionMap mOptions;

		D3D8DriverList* getDirect3DDrivers(void);
		void refreshD3DSettings(void);

		inline bool compareDecls( DWORD* pDecl1, DWORD* pDecl2, int size );

		// Matrix conversion
		D3DXMATRIX makeD3DXMatrix( const Matrix4& mat );
		Matrix4 convertD3DXMatrix( const D3DXMATRIX& mat );

		void initConfigOptions(void);
		void initInputDevices(void);
		void processInputDevices(void);
		void setD3D8Light( int index, Light* light );

#ifdef _DEBUG
		void DumpBuffer( BYTE* pBuffer, DWORD vertexFormat, unsigned int numVertices, unsigned int stride );
#endif
        D3DCMPFUNC convertCompareFunction(CompareFunction func);
        D3DSTENCILOP convertStencilOp(StencilOperation op);

		DWORD _getMipFilter(const TextureFilterOptions fo, eD3DTexType texType);
		DWORD _getMagFilter(const TextureFilterOptions fo, eD3DTexType texType);
		DWORD _getMinFilter(const TextureFilterOptions fo, eD3DTexType texType);
		DWORD _getCurrentAnisotropy(int unit);

		// state management methods, very primitive !!!
		HRESULT __SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
		HRESULT __SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);

	public:
		D3D8RenderSystem( HINSTANCE hInstance );
		~D3D8RenderSystem();

		// ------------------------------------------
		// Overridden RenderSystem functions
		// ------------------------------------------
		const String& getName(void) const;
		ConfigOptionMap& getConfigOptions(void);
		void setConfigOption( const String &name, const String &value );
		String validateConfigOptions(void);
		RenderWindow* initialise( bool autoCreateWindow );
		void reinitialise();
		void shutdown();
		void startRendering();
		void setAmbientLight( float r, float g, float b );
		void setShadingType( ShadeOptions so );
		void setLightingEnabled( bool enabled );
        RenderWindow* createRenderWindow(const String &name, int width, int height, int colourDepth,
            bool fullScreen, int left = 0, int top = 0, bool depthBuffer = true,
            RenderWindow* parentWindowHandle = 0);
        RenderTexture * createRenderTexture( const String & name, int width, int height );
		void destroyRenderWindow( RenderWindow* pWin );
		String getErrorDescription( long errorNumber );
		void convertColourValue( const ColourValue& colour, unsigned long* pDest );

		// ------------------------------------------
		// Low-level overridden members
		// ------------------------------------------
		void _addLight( Light* lt );
		void _removeLight( Light* lt );
		void _modifyLight( Light* lt );
		void _removeAllLights(void);
		void _pushRenderState(void);
		void _popRenderState(void);
		void _setWorldMatrix( const Matrix4 &m );
		void _setViewMatrix( const Matrix4 &m );
		void _setProjectionMatrix( const Matrix4 &m );
		void _setSurfaceParams( const ColourValue &ambient, const ColourValue &diffuse, const ColourValue &specular,
			const ColourValue &emissive, Real shininess );
		unsigned short _getNumTextureUnits(void);
		void _setTexture( int unit, bool enabled, const String &texname );
        void _setTextureCoordCalculation(int unit, TexCoordCalcMethod m);
        void _setTextureCoordSet( int stage, int index );
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
        /** See
          RenderSystem
         */
        void _setDepthBias(ushort bias);
		void _setFog( FogMode mode = FOG_NONE, ColourValue colour = ColourValue::White, Real expDensity = 1.0, Real linearStart = 0.0, Real linearEnd = 1.0 );
        void _makeProjectionMatrix(Real fovy, Real aspect, Real nearPlane, Real farPlane, Matrix4& dest);
        void _setRasterisationMode(SceneDetailLevel level);
        /** See
          RenderSystem
         */
        void setStencilCheckEnabled(bool enabled);
        /** See
          RenderSystem
         */
        bool hasHardwareStencil(void);
        /** See
          RenderSystem
         */
        ushort getStencilBufferBitDepth(void);
        /** See
          RenderSystem
         */
        void setStencilBufferFunction(CompareFunction func);
        /** See
          RenderSystem
         */
        void setStencilBufferReferenceValue(ulong refValue);
        /** See
          RenderSystem
         */
        void setStencilBufferMask(ulong mask);
        /** See
          RenderSystem
         */
        void setStencilBufferFailOperation(StencilOperation op);
        /** See
          RenderSystem
         */
        void setStencilBufferDepthFailOperation(StencilOperation op);
        /** See
          RenderSystem
         */
        void setStencilBufferPassOperation(StencilOperation op);
        /** See
          RenderSystem
         */
		void _setTextureLayerFiltering(int unit, const TextureFilterOptions texLayerFilterOps);
        /** See
          RenderSystem
         */
		void _setTextureLayerAnisotropy(int unit, int maxAnisotropy);
	};

}

#endif
