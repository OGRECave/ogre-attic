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
#ifndef __GLRenderSystem_H__
#define __GLRenderSystem_H__

#include "OgreGLPrerequisites.h"
#include "OgrePlatform.h"
#include "OgreRenderSystem.h"
#include "OgreGLHardwareBufferManager.h"
#include "OgreGLGpuProgramManager.h"
#include "OgreVector4.h"

#include "OgreGLSupport.h"


namespace Ogre {
    /**
      Implementation of SDL as a rendering system.
     */
    class GLRenderSystem : public RenderSystem
    {
    private:
        // Rendering loop control
        bool mStopRendering;

        // Array of up to 8 lights, indexed as per API
        // Note that a null value indicates a free slot
        #define MAX_LIGHTS 8
        Light* mLights[MAX_LIGHTS];

        // clip planes
        typedef std::vector<Vector4> PlaneList2;
        PlaneList2 mClipPlanes;
        void setGLClipPlanes() const;


        // view matrix to set world against
        Matrix4 mViewMatrix;
        Matrix4 mWorldMatrix;
        Matrix4 mTextureMatrix;

        // Last min & mip filtering options, so we can combine them
        FilterOptions mMinFilter;
        FilterOptions mMipFilter;

        // XXX 8 max texture units?
        size_t mTextureCoordIndex[OGRE_MAX_TEXTURE_COORD_SETS];

        /// holds texture type settings for every stage
        GLenum mTextureTypes[OGRE_MAX_TEXTURE_LAYERS];

        void initConfigOptions(void);
        void initInputDevices(void);
        void processInputDevices(void);

        void setGLLight(size_t index, Light* lt);
        void makeGLMatrix(GLfloat gl_matrix[16], const Matrix4& m);
 
        GLint getBlendMode(SceneBlendFactor ogreBlend) const;

        void setLights();

        // Store last depth write state
        bool mDepthWrite;
		// Store last colour write state
		bool mColourWrite[4];

        GLint convertCompareFunction(CompareFunction func) const;
        GLint convertStencilOp(StencilOperation op, bool invert = false) const;

		// internal method for anisotrophy validation
		GLfloat _getCurrentAnisotropy(size_t unit);
		
        /// GL support class, used for creating windows etc
        GLSupport* mGLSupport;
        
        /// Internal method to set pos / direction of a light
        void setGLLightPositionDirection(Light* lt, size_t lightindex);

        bool mUseAutoTextureMatrix;
        GLfloat mAutoTextureMatrix[16];

        // check if the GL system has already been initialized
        bool mGLInitialized;
        // Initialise GL context
        void initGL(void);

        HardwareBufferManager* mHardwareBufferManager;
        GLGpuProgramManager* mGpuProgramManager;

        unsigned short mCurrentLights;

        GLuint getCombinedMinMipFilter(void) const;

        GLGpuProgram* mCurrentVertexProgram;
        GLGpuProgram* mCurrentFragmentProgram;

    public:
        // Default constructor / destructor
        GLRenderSystem();
        ~GLRenderSystem();

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
        RenderWindow* initialise(bool autoCreateWindow, const String& windowTitle = "OGRE Render Window");
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
        RenderWindow* createRenderWindow(const String &name, unsigned int width, unsigned int height, unsigned int colourDepth,
            bool fullScreen, int left = 0, int top = 0, bool depthBuffer = true,
            RenderWindow* parentWindowHandle = 0);

        RenderTexture * createRenderTexture( const String & name, unsigned int width, unsigned int height );

        /** See
          RenderSystem
         */
        void destroyRenderWindow(RenderWindow* pWin);
        /** See
          RenderSystem
         */
        String getErrorDescription(long errorNumber) const;

        /** See
          RenderSystem
         */
        void convertColourValue(const ColourValue& colour, unsigned long* pDest);
        /** See
          RenderSystem
         */
        void setNormaliseNormals(bool normalise);

        // -----------------------------
        // Low-level overridden members
        // -----------------------------
        /** See
          RenderSystem
         */
        void _useLights(const LightList& lights, unsigned short limit);
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
        void _setTexture(size_t unit, bool enabled, const String &texname);
        /** See
          RenderSystem
         */
        void _setTextureCoordSet(size_t stage, size_t index);
        /** See
          RenderSystem
         */
        void _setTextureCoordCalculation(size_t stage, TexCoordCalcMethod m, 
            const Frustum* frustum = 0);
        /** See
          RenderSystem
         */
        void _setTextureBlendMode(size_t stage, const LayerBlendModeEx& bm);
        /** See
          RenderSystem
         */
        void _setTextureAddressingMode(size_t stage, TextureUnitState::TextureAddressingMode tam);
        /** See
          RenderSystem
         */
        void _setTextureMatrix(size_t stage, const Matrix4& xform);
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
        void _setColourBufferWriteEnabled(bool red, bool green, bool blue, bool alpha);
		/** See
          RenderSystem
         */
        void _setFog(FogMode mode, const ColourValue& colour, Real density, Real start, Real end);
        /** See
          RenderSystem
         */
        void _makeProjectionMatrix(Real fovy, Real aspect, Real nearPlane, Real farPlane, 
            Matrix4& dest, bool forGpuProgram = false);
        /** See
        RenderSystem
        */
        void _makeProjectionMatrix(Real left, Real right, Real bottom, Real top, 
            Real nearPlane, Real farPlane, Matrix4& dest, bool forGpuProgram = false);
        /** See
          RenderSystem
         */
		void _makeOrthoMatrix(Real fovy, Real aspect, Real nearPlane, Real farPlane, 
            Matrix4& dest, bool forGpuProgram = false);
        /** See
        RenderSystem
        */
        void setClipPlane (ushort index, Real A, Real B, Real C, Real D);
        /** See
        RenderSystem
        */
        void enableClipPlane (ushort index, bool enable);
        /** See
          RenderSystem
         */
        void _setRasterisationMode(SceneDetailLevel level);
        /** See
          RenderSystem
         */
        void setStencilCheckEnabled(bool enabled);
        /** See RenderSystem.
         */
        void setStencilBufferParams(CompareFunction func = CMPF_ALWAYS_PASS, 
            ulong refValue = 0, ulong mask = 0xFFFFFFFF, 
            StencilOperation stencilFailOp = SOP_KEEP, 
            StencilOperation depthFailOp = SOP_KEEP,
            StencilOperation passOp = SOP_KEEP, 
            bool twoSidedOperation = false);
        /** See
          RenderSystem
         */
        void _setTextureUnitFiltering(size_t unit, FilterType ftype, FilterOptions filter);
        /** See
          RenderSystem
         */
		void _setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy);
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
        void _render(const RenderOperation& op);
        /** See
          RenderSystem
         */
        void bindGpuProgram(GpuProgram* prg);
        /** See
          RenderSystem
         */
        void unbindGpuProgram(GpuProgramType gptype);
        /** See
          RenderSystem
         */
        void bindGpuProgramParameters(GpuProgramType gptype, GpuProgramParametersSharedPtr params);
        /** See
          RenderSystem
         */
        void setClipPlanes(const PlaneList& clipPlanes);
        /** See
          RenderSystem
         */
        void setScissorTest(bool enabled, size_t left = 0, size_t top = 0, size_t right = 800, size_t bottom = 600) ;
        void clearFrameBuffer(unsigned int buffers, 
            const ColourValue& colour = ColourValue::Black, 
            Real depth = 1.0f, unsigned short stencil = 0);
        HardwareOcclusionQuery* createHardwareOcclusionQuery(void);
        Real getHorizontalTexelOffset(void);
        Real getVerticalTexelOffset(void);

        // ----------------------------------
        // End Overridden members
        // ----------------------------------
    };
}
#endif

