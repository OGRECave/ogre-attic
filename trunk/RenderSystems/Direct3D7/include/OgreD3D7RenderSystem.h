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
#ifndef __D3DRENDERSYSTEM_H__
#define __D3DRENDERSYSTEM_H__

// Precompiler options
#include "OgreD3D7Prerequisites.h"
#include "OgreString.h"

// Include D3D and DirectDraw stuff
#define D3D_OVERLOADS
#include <ddraw.h>
#include <d3d.h>


#include "OgreRenderSystem.h"
#include "OgreD3D7HardwareBufferManager.h"

namespace Ogre {

    class DDDriverList;
    class DDDriver;

    /**
      Implementation of DirectX as a rendering system.
     */
    class D3DRenderSystem : public RenderSystem
    {
    private:
        // Direct3D rendering device
        // Only created after top-level window created
        LPDIRECT3DDEVICE7 mlpD3DDevice;
        D3DDEVICEDESC7 mD3DDeviceDesc;

        // List of DD drivers installed (video cards)
        // Enumerates itself
        DDDriverList* mDriverList;
        // Currently active driver
        DDDriver* mActiveDDDriver;

        // Array of up to 8 lights, indexed as per API
        // Note that a null value indicates a free slot
        #define MAX_LIGHTS 8
        Light* mLights[MAX_LIGHTS];




        HINSTANCE mhInstance;



		// Stored options
		ConfigOptionMap mOptions;

        // Private utilities
        DDDriverList* getDirectDrawDrivers(void);
        void refreshDDSettings(void);

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
 			LPDIRECTDRAWSURFACE7 pTex;
 		} mTexStageDesc[OGRE_MAX_TEXTURE_LAYERS];


        // Matrix conversion
        D3DMATRIX makeD3DMatrix(const Matrix4& mat);
        Matrix4 convertD3DMatrix(const D3DMATRIX& mat);

        void initConfigOptions(void);
        void initInputDevices(void);
        void processInputDevices(void);
        void setD3DLight(int index, Light* light);

        D3DCMPFUNC convertCompareFunction(CompareFunction func);
        D3DSTENCILOP convertStencilOp(StencilOperation op);

		// state management methods, very primitive !!!
		HRESULT __SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
		HRESULT __SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);

		DWORD _getMipFilter(const TextureFilterOptions fo);
		DWORD _getMagFilter(const TextureFilterOptions fo);
		DWORD _getMinFilter(const TextureFilterOptions fo);
		DWORD _getCurrentAnisotropy(int unit);

        HardwareBufferManager* mHardwareBufferManager;

        bool mForcedNormalisation;


	public:
        // Default constructor / destructor
        D3DRenderSystem(HINSTANCE hInstance);
        ~D3DRenderSystem();



        // ----------------------------------
        // Overridden RenderSystem functions
        // ----------------------------------
        /** See
          RenderSystem
         */
        const String& getName(void) const;
        /** See
          RenderSystem
         */
        ConfigOptionMap& getConfigOptions(void);
        /** See
          RenderSystem
         */
        void setConfigOption(const String &name, const String &value);
        /** See
          RenderSystem
         */
        String validateConfigOptions(void);
        /** See
          RenderSystem
         */
        RenderWindow* initialise(bool autoCreateWindow);
        /** See
          RenderSystem
         */
        void reinitialise(void); // Used if settings changed mid-rendering
        /** See
          RenderSystem
         */
        void shutdown(void);

        /** See
          RenderSystem
         */
        void startRendering(void);
        /** See
          RenderSystem
         */
        void setAmbientLight(float r, float g, float b);
        /** See
          RenderSystem
         */
        void setShadingType(ShadeOptions so);
        /** See
          RenderSystem
         */
        void setLightingEnabled(bool enabled);
        /** See
          RenderSystem
         */
        RenderWindow* createRenderWindow(const String &name, int width, int height, int colourDepth,
            bool fullScreen, int left = 0, int top = 0, bool depthBuffer = true,
            RenderWindow* parentWindowHandle = 0);

        RenderTexture * createRenderTexture( const String & name, int width, int height );

        /** See
          RenderSystem
         */
        void destroyRenderWindow(RenderWindow* pWin);

        /** See
          RenderSystem
         */
        String getErrorDescription(long errorNumber);

        /** See
          RenderSystem
         */
        void convertColourValue(const ColourValue& colour, unsigned long* pDest);

        // -----------------------------
        // Low-level overridden members
        // -----------------------------
        /** See
          RenderSystem
         */
        void _addLight(Light *lt);
        /** See
          RenderSystem
         */
        void _removeLight(Light *lt);
        /** See
          RenderSystem
         */
        void _modifyLight(Light* lt);
        /** See
          RenderSystem
         */
        void _removeAllLights(void);
        /** See
          RenderSystem
         */
        void _pushRenderState(void);
        /** See
          RenderSystem
         */
        void _popRenderState(void);
        /** See
          RenderSystem
         */
        void _setWorldMatrix(const Matrix4 &m);
        /** See
          RenderSystem
         */
        void _setViewMatrix(const Matrix4 &m);
        /** See
          RenderSystem
         */
        void _setProjectionMatrix(const Matrix4 &m);
        /** See
          RenderSystem
         */
        void _setSurfaceParams(const ColourValue &ambient,
            const ColourValue &diffuse, const ColourValue &specular,
            const ColourValue &emissive, Real shininess);
        /** See
          RenderSystem
         */
        void _setTexture(int unit, bool enabled, const String &texname);
        /** See
          RenderSystem
         */
        void _setTextureBlendMode(int stage, const LayerBlendModeEx& bm);
        /** See
          RenderSystem
         */
        void _setTextureAddressingMode(int stage, Material::TextureLayer::TextureAddressingMode tam);
        /** See
          RenderSystem
         */
        void _setTextureMatrix(int stage, const Matrix4& xform);
        /** See
          RenderSystem
         */
        void _setTextureCoordSet( int stage, int index );
        /** See
          RenderSystem
         */
        void _setTextureCoordCalculation(int unit, TexCoordCalcMethod m);
        /** See
          RenderSystem
         */
        void _setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor);
        /** See
          RenderSystem
         */
        void _setAlphaRejectSettings(CompareFunction func, unsigned char value);
        /** See
          RenderSystem
         */
        void _setViewport(Viewport *vp);
        /** See
          RenderSystem
         */
        void _beginFrame(void);
        /** See
          RenderSystem
         */
        void _render(const RenderOperation& op);
        /** See
          RenderSystem
         */
        void _endFrame(void);
        /** See
          RenderSystem
         */
        void _setCullingMode(CullingMode mode);
        /** See
          RenderSystem
         */
        void _setDepthBufferParams(bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL);
        /** See
          RenderSystem
         */
        void _setDepthBufferCheckEnabled(bool enabled = true);
        /** See
          RenderSystem
         */
        void _setDepthBufferWriteEnabled(bool enabled = true);
        /** See
          RenderSystem
         */
        void _setDepthBufferFunction(CompareFunction func = CMPF_LESS_EQUAL);
        /** See
          RenderSystem
         */
        void _setDepthBias(ushort bias);
        /** See
          RenderSystem
         */
        void _setFog(FogMode mode, const ColourValue& colour, Real density, Real start, Real end);
        /** See
          RenderSystem
         */
        void _makeProjectionMatrix(Real fovy, Real aspect, Real nearPlane, Real farPlane, Matrix4& dest);
        /** See
          RenderSystem
         */
        void _setRasterisationMode(SceneDetailLevel level);
        /** See
          RenderSystem
         */
        void setStencilCheckEnabled(bool enabled);
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
        /** See
          RenderSystem
         */
		void setVertexDeclaration(VertexDeclaration* decl);
        /** See
          RenderSystem
         */
		void setVertexBufferBinding(VertexBufferBinding* binding);
        /** See
          RenderSystem
         */
        void setNormaliseNormals(bool normalise);
        /** See
          RenderSystem
         */
        void bindGpuProgram(GpuProgram* prg) { /* do nothing */}
        /** See
          RenderSystem
         */
        void unbindGpuProgram(GpuProgramType gptype){ /* do nothing */}
        /** See
          RenderSystem
         */
        void bindGpuProgramParameters(GpuProgramType gptype, GpuProgramParameters* params) { /* do nothing */}

        // ----------------------------------
        // End Overridden members
        // ----------------------------------
    };
}
#endif

