/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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
http://www.gnu.org/copyleft/lesser.txt.s
-----------------------------------------------------------------------------
*/


#include "OgreGLRenderSystem.h"
#include "OgreRenderSystem.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreLight.h"
#include "OgreCamera.h"
#include "OgreGLTextureManager.h"
#include "OgreGLHardwareVertexBuffer.h"
#include "OgreGLHardwareIndexBuffer.h"
#include "OgreGLDefaultHardwareBufferManager.h"
#include "OgreGLUtil.h"
#include "OgreGLGpuProgram.h"
#include "OgreGLGpuNvparseProgram.h"
#include "ATI_FS_GLGpuProgram.h"
#include "OgreGLGpuProgramManager.h"
#include "OgreException.h"
#include "OgreGLATIFSInit.h"
#include "OgreGLSLExtSupport.h"
#include "OgreGLHardwareOcclusionQuery.h"
#include "OgreGLContext.h"


#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

// Convenience macro from ARB_vertex_buffer_object spec
#define VBO_BUFFER_OFFSET(i) ((char *)NULL + (i))

// Pointers to extension functions
GL_ActiveTextureARB_Func glActiveTextureARB_ptr;
GL_ClientActiveTextureARB_Func glClientActiveTextureARB_ptr;
GL_SecondaryColorPointerEXT_Func glSecondaryColorPointerEXT_ptr;
GL_SecondaryColor3fEXT_Func glSecondaryColor3fEXT_ptr;
GL_GenBuffersARB_Func glGenBuffersARB_ptr;
GL_BindBufferARB_Func glBindBufferARB_ptr;
GL_DeleteBuffersARB_Func glDeleteBuffersARB_ptr;
GL_MapBufferARB_Func glMapBufferARB_ptr;
GL_UnmapBufferARB_Func glUnmapBufferARB_ptr;
GL_BufferDataARB_Func glBufferDataARB_ptr;
GL_BufferSubDataARB_Func glBufferSubDataARB_ptr;
GL_GetBufferSubDataARB_Func glGetBufferSubDataARB_ptr;
GL_GenProgramsARB_Func glGenProgramsARB_ptr;
GL_DeleteProgramsARB_Func glDeleteProgramsARB_ptr;
GL_BindProgramARB_Func glBindProgramARB_ptr;
GL_ProgramStringARB_Func glProgramStringARB_ptr;
GL_ProgramLocalParameter4fvARB_Func glProgramLocalParameter4fvARB_ptr;
GL_ProgramParameter4fvNV_Func glProgramParameter4fvNV_ptr;
GL_VertexAttribPointerARB_Func glVertexAttribPointerARB_ptr;
GL_EnableVertexAttribArrayARB_Func glEnableVertexAttribArrayARB_ptr;
GL_DisableVertexAttribArrayARB_Func glDisableVertexAttribArrayARB_ptr;
GL_CombinerStageParameterfvNV_Func glCombinerStageParameterfvNV_ptr;
GL_CombinerParameterfvNV_Func glCombinerParameterfvNV_ptr;
GL_CombinerParameteriNV_Func glCombinerParameteriNV_ptr;
GL_GetProgramivARB_Func glGetProgramivARB_ptr;
GL_LoadProgramNV_Func glLoadProgramNV_ptr;
GL_CombinerInputNV_Func glCombinerInputNV_ptr;
GL_CombinerOutputNV_Func glCombinerOutputNV_ptr;
GL_FinalCombinerInputNV_Func glFinalCombinerInputNV_ptr;
GL_TrackMatrixNV_Func glTrackMatrixNV_ptr;
PFNGLCOMPRESSEDTEXIMAGE1DARBPROC glCompressedTexImage1DARB_ptr;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2DARB_ptr;
PFNGLCOMPRESSEDTEXIMAGE3DARBPROC glCompressedTexImage3DARB_ptr;
PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC glCompressedTexSubImage1DARB_ptr;
PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC glCompressedTexSubImage2DARB_ptr;
PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC glCompressedTexSubImage3DARB_ptr;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC glGetCompressedTexImageARB_ptr;
GL_ActiveStencilFaceEXT_Func glActiveStencilFaceEXT_ptr;
GL_GenOcclusionQueriesNV_Func glGenOcclusionQueriesNV_ptr;	
GL_DeleteOcclusionQueriesNV_Func glDeleteOcclusionQueriesNV_ptr;
GL_BeginOcclusionQueryNV_Func glBeginOcclusionQueryNV_ptr;
GL_EndOcclusionQueryNV_Func glEndOcclusionQueryNV_ptr;
GL_GetOcclusionQueryuivNV_Func glGetOcclusionQueryuivNV_ptr;
GL_GenQueriesARB_Func glGenQueriesARB_ptr;
GL_DeleteQueriesARB_Func glDeleteQueriesARB_ptr;
GL_BeginQueryARB_Func glBeginQueryARB_ptr;
GL_EndQueryARB_Func glEndQueryARB_ptr;
GL_GetQueryObjectuivARB_Func glGetQueryObjectuivARB_ptr;

namespace Ogre {

    // Callback function used when registering GLGpuPrograms
    GpuProgram* createGLArbGpuProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle, 
        const String& group, bool isManual, ManualResourceLoader* loader,
        GpuProgramType gptype, const String& syntaxCode)
    {
        GLArbGpuProgram* ret = new GLArbGpuProgram(
            creator, name, handle, group, isManual, loader);
        ret->setType(gptype);
        ret->setSyntaxCode(syntaxCode);
        return ret;
    }

    GpuProgram* createGLGpuNvparseProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle, 
        const String& group, bool isManual, ManualResourceLoader* loader,
        GpuProgramType gptype, const String& syntaxCode)
    {
        GLGpuNvparseProgram* ret = new GLGpuNvparseProgram(
            creator, name, handle, group, isManual, loader);
        ret->setType(gptype);
        ret->setSyntaxCode(syntaxCode);
        return ret;
    }

    GpuProgram* createGL_ATI_FS_GpuProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle, 
        const String& group, bool isManual, ManualResourceLoader* loader,
        GpuProgramType gptype, const String& syntaxCode)
	{

        ATI_FS_GLGpuProgram* ret = new ATI_FS_GLGpuProgram(
            creator, name, handle, group, isManual, loader);
        ret->setType(gptype);
        ret->setSyntaxCode(syntaxCode);
        return ret;
	}

    GLRenderSystem::GLRenderSystem()
      : mDepthWrite(true), mHardwareBufferManager(0),
        mGpuProgramManager(0)
    {
        size_t i;

        OgreGuard( "GLRenderSystem::GLRenderSystem" );

        LogManager::getSingleton().logMessage(getName() + " created.");

        // Get our GLSupport
        mGLSupport = getGLSupport();

        for( i=0; i<MAX_LIGHTS; i++ )
            mLights[i] = NULL;

        mWorldMatrix = Matrix4::IDENTITY;
        mViewMatrix = Matrix4::IDENTITY;
        
        initConfigOptions();

        mColourWrite[0] = mColourWrite[1] = mColourWrite[2] = mColourWrite[3] = true;

        for (i = 0; i < OGRE_MAX_TEXTURE_COORD_SETS; i++)
        {
			// Dummy value
            mTextureCoordIndex[i] = 99;
        }

        for (i = 0; i < OGRE_MAX_TEXTURE_LAYERS; i++)
        {
            mTextureTypes[i] = 0;
        }

        mActiveRenderTarget = 0;
        mCurrentContext = 0;
        mMainContext = 0;

        mGLInitialized = false;

        glActiveTextureARB_ptr = 0;
        glClientActiveTextureARB_ptr = 0;
        glSecondaryColorPointerEXT_ptr = 0;
        glSecondaryColor3fEXT_ptr = 0;
        glGenBuffersARB_ptr = 0;
        glBindBufferARB_ptr = 0;
        glDeleteBuffersARB_ptr = 0;
        glMapBufferARB_ptr = 0;
        glUnmapBufferARB_ptr = 0;
        glBufferDataARB_ptr = 0;
        glBufferSubDataARB_ptr = 0;
        glGetBufferSubDataARB_ptr = 0;
        glGenProgramsARB_ptr = 0;
        glDeleteProgramsARB_ptr = 0;
        glBindProgramARB_ptr = 0;
        glProgramStringARB_ptr = 0;
        glProgramLocalParameter4fvARB_ptr = 0;
        glProgramParameter4fvNV_ptr = 0;
        glCombinerStageParameterfvNV_ptr = 0;
        glCombinerParameterfvNV_ptr = 0;
        glCombinerParameteriNV_ptr = 0;
        glGetProgramivARB_ptr = 0;
        glLoadProgramNV_ptr = 0;
        glCombinerInputNV_ptr = 0;
        glCombinerOutputNV_ptr = 0;
        glFinalCombinerInputNV_ptr = 0;
        glTrackMatrixNV_ptr = 0;
        glActiveStencilFaceEXT_ptr = 0;
        glGenOcclusionQueriesNV_ptr = 0;
        glDeleteOcclusionQueriesNV_ptr = 0;
        glBeginOcclusionQueryNV_ptr = 0;
        glEndOcclusionQueryNV_ptr = 0;
        glGetOcclusionQueryuivNV_ptr = 0;
		glGenQueriesARB_ptr = 0;
		glDeleteQueriesARB_ptr = 0;
		glBeginQueryARB_ptr = 0;
		glEndQueryARB_ptr = 0;
		glGetQueryObjectuivARB_ptr = 0;

        mCurrentLights = 0;
        mMinFilter = FO_LINEAR;
        mMipFilter = FO_POINT;
        mCurrentVertexProgram = 0;
        mCurrentFragmentProgram = 0;

        mClipPlanes.reserve(6);

        OgreUnguard();
    }

    GLRenderSystem::~GLRenderSystem()
    {
        shutdown();

        // Destroy render windows
        RenderTargetMap::iterator i;
        for (i = mRenderTargets.begin(); i != mRenderTargets.end(); ++i)
        {
            delete i->second;
        }
        mRenderTargets.clear();

        if (mTextureManager)
            delete mTextureManager;

        delete mGLSupport;
    }

    const String& GLRenderSystem::getName(void) const
    {
        static String strName("OpenGL Rendering Subsystem");
        return strName;
    }

    void GLRenderSystem::initConfigOptions(void)
    {
        OgreGuard("GLRenderSystem::initConfigOptions");
        mGLSupport->addConfig();
        OgreUnguard();
    }
    
    ConfigOptionMap& GLRenderSystem::getConfigOptions(void)
    {
        return mGLSupport->getConfigOptions();
    }

    void GLRenderSystem::setConfigOption(const String &name, const String &value)
    {
        mGLSupport->setConfigOption(name, value);
    }

    String GLRenderSystem::validateConfigOptions(void)
    {
        // XXX Return an error string if something is invalid
        return mGLSupport->validateConfig();
    }

    RenderWindow* GLRenderSystem::initialise(bool autoCreateWindow, const String& windowTitle)
    {
        mGLSupport->start();
        
		RenderWindow* autoWindow = mGLSupport->createWindow(autoCreateWindow, this, windowTitle);


        _setCullingMode( mCullingMode );
        
        return autoWindow;
    }

    void GLRenderSystem::initGL(void)
    {
        mGLSupport->initialiseExtensions();

        LogManager::getSingleton().logMessage(
            "***************************\n"
            "*** GL Renderer Started ***\n"
            "***************************");


        // Check for hardware mipmapping support.
        // Note: This is disabled for ATI cards until they fix their drivers
        if(mGLSupport->getGLVendor() != "ATI" && 
            (mGLSupport->checkMinGLVersion("1.4.0") || 
             mGLSupport->checkExtension("GL_SGIS_generate_mipmap")))
        {
            mCapabilities->setCapability(RSC_AUTOMIPMAP);
        }

        // Check for blending support
        if(mGLSupport->checkMinGLVersion("1.3.0") || 
            mGLSupport->checkExtension("GL_ARB_texture_env_combine") || 
            mGLSupport->checkExtension("GL_EXT_texture_env_combine"))
        {
            mCapabilities->setCapability(RSC_BLENDING);
        }

        // Check for Multitexturing support and set number of texture units
        if(mGLSupport->checkMinGLVersion("1.3.0") || 
            mGLSupport->checkExtension("GL_ARB_multitexture"))
        {
            GLint units;
            glGetIntegerv( GL_MAX_TEXTURE_UNITS, &units );
			mFixedFunctionTextureUnits = units;

			if (mGLSupport->checkExtension("GL_ARB_fragment_program"))
			{
				// Also check GL_MAX_TEXTURE_IMAGE_UNITS_ARB since NV at least
				// only increased this on the FX/6x00 series
				GLint arbUnits;
				glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &arbUnits );
				if (arbUnits > units)
					units = arbUnits;
			}
	    
            mCapabilities->setNumTextureUnits(units);

        }
        else
        {
            // If no multitexture support then set one texture unit
            mCapabilities->setNumTextureUnits(1);
        }
            
        // Check for Anisotropy support
        if(mGLSupport->checkExtension("GL_EXT_texture_filter_anisotropic"))
        {
            mCapabilities->setCapability(RSC_ANISOTROPY);
        }

        // Check for DOT3 support
        if(mGLSupport->checkMinGLVersion("1.3.0") ||
            mGLSupport->checkExtension("GL_ARB_texture_env_dot3") ||
            mGLSupport->checkExtension("GL_EXT_texture_env_dot3"))
        {
            mCapabilities->setCapability(RSC_DOT3);
        }

        // Check for cube mapping
        if(mGLSupport->checkMinGLVersion("1.3.0") || 
            mGLSupport->checkExtension("GL_ARB_texture_cube_map") || 
            mGLSupport->checkExtension("GL_EXT_texture_cube_map"))
        {
            mCapabilities->setCapability(RSC_CUBEMAPPING);
        }
        
        // Check for hardware stencil support and set bit depth
        GLint stencil;
        glGetIntegerv(GL_STENCIL_BITS,&stencil);

        if(stencil)
        {
            mCapabilities->setCapability(RSC_HWSTENCIL);
            mCapabilities->setStencilBufferBitDepth(stencil);
        }

        // Check for VBO support
        if(mGLSupport->checkExtension("GL_ARB_vertex_buffer_object"))
        {
            mCapabilities->setCapability(RSC_VBO);

            mHardwareBufferManager = new GLHardwareBufferManager;
        }
        else
        {
            mHardwareBufferManager = new GLDefaultHardwareBufferManager;
        }

        // XXX Need to check for nv2 support and make a program manager for it
        // XXX Probably nv1 as well for older cards
        // GPU Program Manager setup
        mGpuProgramManager = new GLGpuProgramManager();

		if(mGLSupport->checkExtension("GL_ARB_vertex_program"))
        {
            mCapabilities->setCapability(RSC_VERTEX_PROGRAM);

            // Vertex Program Properties
            mCapabilities->setMaxVertexProgramVersion("arbvp1");
            mCapabilities->setVertexProgramConstantBoolCount(0);
            mCapabilities->setVertexProgramConstantIntCount(0);
            mCapabilities->setVertexProgramConstantFloatCount(
                GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB);

            mGpuProgramManager->_pushSyntaxCode("arbvp1");
            mGpuProgramManager->registerProgramFactory("arbvp1", createGLArbGpuProgram);
			if (mGLSupport->checkExtension("GL_NV_vertex_program2_option"))
			{
				mCapabilities->setMaxVertexProgramVersion("vp30");
				mGpuProgramManager->_pushSyntaxCode("vp30");
				mGpuProgramManager->registerProgramFactory("vp30", createGLArbGpuProgram);
			}

			if (mGLSupport->checkExtension("GL_NV_vertex_program3"))
			{
				mCapabilities->setMaxVertexProgramVersion("vp40");
				mGpuProgramManager->_pushSyntaxCode("vp40");
				mGpuProgramManager->registerProgramFactory("vp40", createGLArbGpuProgram);
			}
		}

        if (mGLSupport->checkExtension("GL_NV_register_combiners2") &&
            mGLSupport->checkExtension("GL_NV_texture_shader"))
        {
            mCapabilities->setCapability(RSC_FRAGMENT_PROGRAM);
            mCapabilities->setMaxFragmentProgramVersion("fp20");

            mGpuProgramManager->_pushSyntaxCode("fp20");
            mGpuProgramManager->registerProgramFactory("fp20", createGLGpuNvparseProgram);
        }


		// NFZ - check for ATI fragment shader support
		if (mGLSupport->checkExtension("GL_ATI_fragment_shader"))
		{
            mCapabilities->setCapability(RSC_FRAGMENT_PROGRAM);
            mCapabilities->setMaxFragmentProgramVersion("ps_1_4");
            // no boolean params allowed
            mCapabilities->setFragmentProgramConstantBoolCount(0);
            // no integer params allowed
            mCapabilities->setFragmentProgramConstantIntCount(0);

			// only 8 Vector4 constant floats supported
            mCapabilities->setFragmentProgramConstantFloatCount(8);

            mGpuProgramManager->_pushSyntaxCode("ps_1_4");
            mGpuProgramManager->_pushSyntaxCode("ps_1_3");
            mGpuProgramManager->_pushSyntaxCode("ps_1_2");
            mGpuProgramManager->_pushSyntaxCode("ps_1_1");

            mGpuProgramManager->registerProgramFactory("ps_1_4", createGL_ATI_FS_GpuProgram);
            mGpuProgramManager->registerProgramFactory("ps_1_3", createGL_ATI_FS_GpuProgram);
            mGpuProgramManager->registerProgramFactory("ps_1_2", createGL_ATI_FS_GpuProgram);
            mGpuProgramManager->registerProgramFactory("ps_1_1", createGL_ATI_FS_GpuProgram);
		}


        if (mGLSupport->checkExtension("GL_ARB_fragment_program"))
        {
            mCapabilities->setCapability(RSC_FRAGMENT_PROGRAM);
            // Fragment Program Properties
            mCapabilities->setMaxFragmentProgramVersion("arbfp1");
            mCapabilities->setFragmentProgramConstantBoolCount(0);
            mCapabilities->setFragmentProgramConstantIntCount(0);
            mCapabilities->setFragmentProgramConstantFloatCount(
                GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB);

            mGpuProgramManager->_pushSyntaxCode("arbfp1");
            mGpuProgramManager->registerProgramFactory("arbfp1", createGLArbGpuProgram);
			if (mGLSupport->checkExtension("GL_NV_fragment_program_option"))
			{
				mCapabilities->setMaxFragmentProgramVersion("fp30");
				mGpuProgramManager->_pushSyntaxCode("fp30");
				mGpuProgramManager->registerProgramFactory("fp30", createGLArbGpuProgram);
			}

			if (mGLSupport->checkExtension("GL_NV_fragment_program2"))
			{
				mCapabilities->setMaxFragmentProgramVersion("fp40");
				mGpuProgramManager->_pushSyntaxCode("fp40");
				mGpuProgramManager->registerProgramFactory("fp40", createGLArbGpuProgram);
			}        
		}

		// NFZ - check if GLSL is supported
		if ( mGLSupport->checkExtension("GL_ARB_shading_language_100") &&
			 mGLSupport->checkExtension("GL_ARB_shader_objects") &&
			 mGLSupport->checkExtension("GL_ARB_fragment_shader") &&
			 mGLSupport->checkExtension("GL_ARB_vertex_shader") )
		{
			// NFZ - check for GLSL vertex and fragment shader support successful
            mGpuProgramManager->_pushSyntaxCode("glsl");
			LogManager::getSingleton().logMessage("GLSL support detected");
		}

		// Check for texture compression
        if(mGLSupport->checkMinGLVersion("1.3.0") ||
            mGLSupport->checkExtension("GL_ARB_texture_compression"))
        {   
            mCapabilities->setCapability(RSC_TEXTURE_COMPRESSION);
         
            // Check for dxt compression
            if(mGLSupport->checkExtension("GL_EXT_texture_compression_s3tc"))
            {
                mCapabilities->setCapability(RSC_TEXTURE_COMPRESSION_DXT);
            }
            // Check for vtc compression
            if(mGLSupport->checkExtension("GL_NV_texture_compression_vtc"))
            {
                mCapabilities->setCapability(RSC_TEXTURE_COMPRESSION_VTC);
            }
        }

        // Scissor test is standard in GL 1.2 (is it emulated on some cards though?)
        mCapabilities->setCapability(RSC_SCISSOR_TEST);
		// As are user clipping planes
		mCapabilities->setCapability(RSC_USER_CLIP_PLANES);

        // 2-sided stencil?
        if (mGLSupport->checkExtension("GL_EXT_stencil_two_side"))
        {
            mCapabilities->setCapability(RSC_TWO_SIDED_STENCIL);
        }
        // stencil wrapping?
        if (mGLSupport->checkExtension("GL_EXT_stencil_wrap"))
        {
            mCapabilities->setCapability(RSC_STENCIL_WRAP);
        }

        // Check for hardware occlusion support
        if(mGLSupport->checkExtension("GL_NV_occlusion_query"))
        {
            mCapabilities->setCapability(RSC_HWOCCLUSION);		
        }

		// UBYTE4 always supported
		mCapabilities->setCapability(RSC_VERTEX_FORMAT_UBYTE4);

        // Inifinite far plane always supported
        mCapabilities->setCapability(RSC_INFINITE_FAR_PLANE);

        // Check for non-power-of-2 texture support
		if(mGLSupport->checkExtension("GL_ARB_texture_non_power_of_two"))
        {
            mCapabilities->setCapability(RSC_NON_POWER_OF_2_TEXTURES);
        }

        // Check for Float textures
        if(mGLSupport->checkExtension("GL_ATI_texture_float") ||
//           mGLSupport->checkExtension("GL_NV_float_buffer") ||
           mGLSupport->checkExtension("GL_ARB_texture_float"))
        {
            mCapabilities->setCapability(RSC_TEXTURE_FLOAT);
        }
		
		// 3D textures should be supported by GL 1.2, which is our minimum version
        mCapabilities->setCapability(RSC_TEXTURE_3D);

		// Check for GLSupport specific extensions
		mGLSupport->initialiseCapabilities(*mCapabilities);

        // Get extension function pointers
        glActiveTextureARB_ptr = 
            (GL_ActiveTextureARB_Func)mGLSupport->getProcAddress("glActiveTextureARB");
        glClientActiveTextureARB_ptr = 
            (GL_ClientActiveTextureARB_Func)mGLSupport->getProcAddress("glClientActiveTextureARB");
        glSecondaryColorPointerEXT_ptr = 
            (GL_SecondaryColorPointerEXT_Func)mGLSupport->getProcAddress("glSecondaryColorPointerEXT");
        glSecondaryColor3fEXT_ptr = 
            (GL_SecondaryColor3fEXT_Func)mGLSupport->getProcAddress("glSecondaryColor3fEXT");
        glGenBuffersARB_ptr = 
            (GL_GenBuffersARB_Func)mGLSupport->getProcAddress("glGenBuffersARB");
        glBindBufferARB_ptr = 
            (GL_BindBufferARB_Func)mGLSupport->getProcAddress("glBindBufferARB");
        glDeleteBuffersARB_ptr = 
            (GL_DeleteBuffersARB_Func)mGLSupport->getProcAddress("glDeleteBuffersARB");
        glMapBufferARB_ptr = 
            (GL_MapBufferARB_Func)mGLSupport->getProcAddress("glMapBufferARB");
        glUnmapBufferARB_ptr = 
            (GL_UnmapBufferARB_Func)mGLSupport->getProcAddress("glUnmapBufferARB");
        glBufferDataARB_ptr = 
            (GL_BufferDataARB_Func)mGLSupport->getProcAddress("glBufferDataARB");
        glBufferSubDataARB_ptr = 
            (GL_BufferSubDataARB_Func)mGLSupport->getProcAddress("glBufferSubDataARB");
        glGetBufferSubDataARB_ptr = 
            (GL_GetBufferSubDataARB_Func)mGLSupport->getProcAddress("glGetBufferSubDataARB");
        glGenProgramsARB_ptr =
            (GL_GenProgramsARB_Func)mGLSupport->getProcAddress("glGenProgramsARB");
        glDeleteProgramsARB_ptr =
            (GL_DeleteProgramsARB_Func)mGLSupport->getProcAddress("glDeleteProgramsARB");
        glBindProgramARB_ptr =
            (GL_BindProgramARB_Func)mGLSupport->getProcAddress("glBindProgramARB");
        glProgramStringARB_ptr =
            (GL_ProgramStringARB_Func)mGLSupport->getProcAddress("glProgramStringARB");
        glProgramLocalParameter4fvARB_ptr =
            (GL_ProgramLocalParameter4fvARB_Func)mGLSupport->getProcAddress("glProgramLocalParameter4fvARB");
         glProgramParameter4fvNV_ptr =
            (GL_ProgramParameter4fvNV_Func)mGLSupport->getProcAddress("glProgramParameter4fvNV");
         glVertexAttribPointerARB_ptr =
             (GL_VertexAttribPointerARB_Func)mGLSupport->getProcAddress("glVertexAttribPointerARB");
         glEnableVertexAttribArrayARB_ptr =
             (GL_EnableVertexAttribArrayARB_Func)mGLSupport->getProcAddress("glEnableVertexAttribArrayARB");
         glDisableVertexAttribArrayARB_ptr =
             (GL_DisableVertexAttribArrayARB_Func)mGLSupport->getProcAddress("glDisableVertexAttribArrayARB");
         glCombinerStageParameterfvNV_ptr =
            (GL_CombinerStageParameterfvNV_Func)mGLSupport->getProcAddress("glCombinerStageParameterfvNV");
        glCombinerParameterfvNV_ptr = 
            (GL_CombinerParameterfvNV_Func)mGLSupport->getProcAddress("glCombinerParameterfvNV");
         glCombinerParameteriNV_ptr = (GL_CombinerParameteriNV_Func)mGLSupport->getProcAddress("glCombinerParameteriNV");
        glGetProgramivARB_ptr = 
            (GL_GetProgramivARB_Func)mGLSupport->getProcAddress("glGetProgramivARB");
        glLoadProgramNV_ptr = 
            (GL_LoadProgramNV_Func)mGLSupport->getProcAddress("glLoadProgramNV");
        glCombinerInputNV_ptr =
            (GL_CombinerInputNV_Func)mGLSupport->getProcAddress("glCombinerInputNV");
        glCombinerOutputNV_ptr =
            (GL_CombinerOutputNV_Func)mGLSupport->getProcAddress("glCombinerOutputNV");
        glFinalCombinerInputNV_ptr = 
            (GL_FinalCombinerInputNV_Func)mGLSupport->getProcAddress("glFinalCombinerInputNV");
        glTrackMatrixNV_ptr = 
            (GL_TrackMatrixNV_Func)mGLSupport->getProcAddress("glTrackMatrixNV");
		glCompressedTexImage1DARB_ptr =
            (PFNGLCOMPRESSEDTEXIMAGE1DARBPROC)mGLSupport->getProcAddress("glCompressedTexImage1DARB");
        glCompressedTexImage2DARB_ptr =
            (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)mGLSupport->getProcAddress("glCompressedTexImage2DARB");
		glCompressedTexImage3DARB_ptr =
            (PFNGLCOMPRESSEDTEXIMAGE3DARBPROC)mGLSupport->getProcAddress("glCompressedTexImage3DARB");
        glCompressedTexSubImage1DARB_ptr =
            (PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC)mGLSupport->getProcAddress("glCompressedTexSubImage1DARB");
        glCompressedTexSubImage2DARB_ptr =
            (PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC)mGLSupport->getProcAddress("glCompressedTexSubImage2DARB");
        glCompressedTexSubImage3DARB_ptr =
            (PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC)mGLSupport->getProcAddress("glCompressedTexSubImage3DARB");
		glGetCompressedTexImageARB_ptr =
			(PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)mGLSupport->getProcAddress("glGetCompressedTexImageARB");
        InitATIFragmentShaderExtensions(*mGLSupport);
		InitGLShaderLanguageExtensions(*mGLSupport);
        glActiveStencilFaceEXT_ptr = 
            (GL_ActiveStencilFaceEXT_Func)mGLSupport->getProcAddress("glActiveStencilFaceEXT");
        glGenOcclusionQueriesNV_ptr =
            (GL_GenOcclusionQueriesNV_Func)mGLSupport->getProcAddress("glGenOcclusionQueriesNV");
        glDeleteOcclusionQueriesNV_ptr =
            (GL_DeleteOcclusionQueriesNV_Func)mGLSupport->getProcAddress("glDeleteOcclusionQueriesNV");
        glBeginOcclusionQueryNV_ptr =
            (GL_BeginOcclusionQueryNV_Func)mGLSupport->getProcAddress("glBeginOcclusionQueryNV");
        glEndOcclusionQueryNV_ptr =
            (GL_EndOcclusionQueryNV_Func)mGLSupport->getProcAddress("glEndOcclusionQueryNV");
        glGetOcclusionQueryuivNV_ptr =
            (GL_GetOcclusionQueryuivNV_Func)mGLSupport->getProcAddress("glGetOcclusionQueryuivNV");

		glGenQueriesARB_ptr =
			(GL_GenQueriesARB_Func)mGLSupport->getProcAddress("glGenQueriesARB");
		glDeleteQueriesARB_ptr =
			(GL_DeleteQueriesARB_Func)mGLSupport->getProcAddress("glDeleteQueriesARB");
		glBeginQueryARB_ptr =
			(GL_BeginQueryARB_Func)mGLSupport->getProcAddress("glBeginQueryARB");
		glEndQueryARB_ptr =
			(GL_EndQueryARB_Func)mGLSupport->getProcAddress("glEndQueryARB");
		glGetQueryObjectuivARB_ptr =
			(GL_GetQueryObjectuivARB_Func)mGLSupport->getProcAddress("glGetQueryObjectuivARB");

		Log* defaultLog = LogManager::getSingleton().getDefaultLog();
		if (defaultLog)
		{
	        mCapabilities->log(defaultLog);
		}
        mGLInitialized = true;
    }

    void GLRenderSystem::reinitialise(void)
    {
        this->shutdown();
        this->initialise(true);
    }

    void GLRenderSystem::shutdown(void)
    {
        RenderSystem::shutdown();

        // Deleting the GPU program manager and hardware buffer manager.  Has to be done before the mGLSupport->stop().
		if (mGpuProgramManager)
        {
        	delete mGpuProgramManager;
        	mGpuProgramManager = 0;
        }

        if (mHardwareBufferManager)
        {
            delete mHardwareBufferManager;
            mHardwareBufferManager = 0;
        }


        mGLSupport->stop();
        mStopRendering = true;
    }

    void GLRenderSystem::setAmbientLight(float r, float g, float b)
    {
        GLfloat lmodel_ambient[] = {r, g, b, 1.0};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    }

    void GLRenderSystem::setShadingType(ShadeOptions so)
    {
        switch(so)
        {
        case SO_FLAT:
            glShadeModel(GL_FLAT);
            break;
        default:
            glShadeModel(GL_SMOOTH);
            break;
        }
    }

	RenderWindow* GLRenderSystem::createRenderWindow(const String &name, 
		unsigned int width, unsigned int height, bool fullScreen,
		const NameValuePairList *miscParams)
    {
        if (mRenderTargets.find(name) != mRenderTargets.end())
        {
            OGRE_EXCEPT(
                Exception::ERR_INVALIDPARAMS, 
                "Window with name '" + name + "' already exists",
                "GLRenderSystem::createRenderWindow" );
        }
		// Log a message
		std::stringstream ss;
		ss << "GLRenderSystem::createRenderWindow \"" << name << "\", " <<
			width << "x" << height << " ";
		if(fullScreen)
			ss << "fullscreen ";
		else
			ss << "windowed ";
		if(miscParams)
		{
			ss << " miscParams: ";
			NameValuePairList::const_iterator it;
			for(it=miscParams->begin(); it!=miscParams->end(); ++it)
			{
				ss << it->first << "=" << it->second << " ";
			}
			LogManager::getSingleton().logMessage(ss.str());
		}

        // Create the window
        RenderWindow* win = mGLSupport->newWindow(name, width, height, 
            fullScreen, miscParams);

        attachRenderTarget( *win );

        if (!mGLInitialized) 
        {
            // Initialise GL after the first window has been created
            initGL();
            mTextureManager = new GLTextureManager(*mGLSupport);
            // Set main and current context
            ContextMap::iterator i = mContextMap.find(win);
            if(i != mContextMap.end()) {
                mCurrentContext = i->second;
                mMainContext =  i->second;
                mCurrentContext->setCurrent();
            }
            // Initialise the main context
            _oneTimeContextInitialization();
        }


        // XXX Do more?

        return win;
    }

	RenderTexture * GLRenderSystem::createRenderTexture( const String & name, unsigned int width, unsigned int height,
		TextureType texType, PixelFormat internalFormat, const NameValuePairList *miscParams ) 
    {
		// Log a message
		std::stringstream ss;
		ss << "GLRenderSystem::createRenderTexture \"" << name << "\", " <<
			width << "x" << height << " texType=" << texType <<
			" internalFormat=" << PixelUtil::getFormatName(internalFormat) << " ";
		if(miscParams)
		{
			ss << "miscParams: ";
			NameValuePairList::const_iterator it;
			for(it=miscParams->begin(); it!=miscParams->end(); ++it)
			{
				ss << it->first << "=" << it->second << " ";
			}
			LogManager::getSingleton().logMessage(ss.str());
		}
		// Pass on the create call
        RenderTexture *rt = mGLSupport->createRenderTexture(name, width, height, texType, internalFormat, miscParams);
        attachRenderTarget( *rt );
        return rt;
    }

    //-----------------------------------------------------------------------
    void GLRenderSystem::destroyRenderWindow(RenderWindow* pWin)
    {
        // Find it to remove from list
        RenderTargetMap::iterator i = mRenderTargets.begin();

        while (i != mRenderTargets.end())
        {
            if (i->second == pWin)
            {
                mRenderTargets.erase(i);
                delete pWin;
                break;
            }
        }
    }

	//---------------------------------------------------------------------
    void GLRenderSystem::_useLights(const LightList& lights, unsigned short limit)
    {
        // Save previous modelview
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        // just load view matrix (identity world)
        GLfloat mat[16];
        makeGLMatrix(mat, mViewMatrix);
        glLoadMatrixf(mat);

        LightList::const_iterator i, iend;
        iend = lights.end();
        unsigned short num = 0;
        for (i = lights.begin(); i != iend && num < limit; ++i, ++num)
        {
            setGLLight(num, *i);
            mLights[num] = *i;
        }
        // Disable extra lights
        for (; num < mCurrentLights; ++num)
        {
            setGLLight(num, NULL);
            mLights[num] = NULL;
        }
        mCurrentLights = std::min(limit, static_cast<unsigned short>(lights.size()));

        setLights();

        // restore previous
        glPopMatrix();

    }

    void GLRenderSystem::setGLLight(size_t index, Light* lt)
    {
        GLenum gl_index = GL_LIGHT0 + index;

        if (!lt)
        {
            // Disable in the scene
            glDisable(gl_index);
        }
        else
        {
            switch (lt->getType())
            {
            case Light::LT_SPOTLIGHT:
                glLightf( gl_index, GL_SPOT_CUTOFF, 0.5f * lt->getSpotlightOuterAngle().valueDegrees() );
                glLightf(gl_index, GL_SPOT_EXPONENT, lt->getSpotlightFalloff());
                break;
            default:
                glLightf( gl_index, GL_SPOT_CUTOFF, 180.0 );
                break;
            }

            // Color
            ColourValue col;
            col = lt->getDiffuseColour();


            GLfloat f4vals[4] = {col.r, col.g, col.b, col.a};
            glLightfv(gl_index, GL_DIFFUSE, f4vals);
            
            col = lt->getSpecularColour();
            f4vals[0] = col.r;
            f4vals[1] = col.g;
            f4vals[2] = col.b;
            f4vals[3] = col.a;
            glLightfv(gl_index, GL_SPECULAR, f4vals);


            // Disable ambient light for movables;
            f4vals[0] = 0;
            f4vals[1] = 0;
            f4vals[2] = 0;
            f4vals[3] = 1;
            glLightfv(gl_index, GL_AMBIENT, f4vals);

            setGLLightPositionDirection(lt, gl_index);


            // Attenuation
            glLightf(gl_index, GL_CONSTANT_ATTENUATION, lt->getAttenuationConstant());
            glLightf(gl_index, GL_LINEAR_ATTENUATION, lt->getAttenuationLinear());
            glLightf(gl_index, GL_QUADRATIC_ATTENUATION, lt->getAttenuationQuadric());
            // Enable in the scene
            glEnable(gl_index);

        }

    }

    //-----------------------------------------------------------------------------
    void GLRenderSystem::makeGLMatrix(GLfloat gl_matrix[16], const Matrix4& m)
    {
        size_t x = 0;
        for (size_t i = 0; i < 4; i++)
        {
            for (size_t j = 0; j < 4; j++)
            {
                gl_matrix[x] = m[j][i];
                x++;
            }
        }
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setWorldMatrix( const Matrix4 &m )
    {
        GLfloat mat[16];
        mWorldMatrix = m;
        makeGLMatrix( mat, mViewMatrix * mWorldMatrix );
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(mat);
    }

    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setViewMatrix( const Matrix4 &m )
    {
        mViewMatrix = m;

        GLfloat mat[16];
		makeGLMatrix( mat, mViewMatrix * mWorldMatrix );
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(mat);

        setGLClipPlanes();
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setProjectionMatrix(const Matrix4 &m)
    {
        GLfloat mat[16];
        makeGLMatrix(mat, m);
        if (mActiveRenderTarget->requiresTextureFlipping())
        {
            // Invert y
            mat[5] = -mat[5];
        }
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(mat);
        glMatrixMode(GL_MODELVIEW);
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setSurfaceParams(const ColourValue &ambient,
        const ColourValue &diffuse, const ColourValue &specular,
        const ColourValue &emissive, Real shininess,
        TrackVertexColourType tracking)
    {
        // XXX Cache previous values?
        // XXX Front or Front and Back?

        GLfloat f4val[4] = {diffuse.r, diffuse.g, diffuse.b, diffuse.a};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, f4val);
        f4val[0] = ambient.r;
        f4val[1] = ambient.g;
        f4val[2] = ambient.b;
        f4val[3] = ambient.a;
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, f4val);
        f4val[0] = specular.r;
        f4val[1] = specular.g;
        f4val[2] = specular.b;
        f4val[3] = specular.a;
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, f4val);
        f4val[0] = emissive.r;
        f4val[1] = emissive.g;
        f4val[2] = emissive.b;
        f4val[3] = emissive.a;
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, f4val);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
        
        // Track vertex colour
        if(tracking != TVC_NONE) 
        {
            GLenum gt = GL_DIFFUSE;
            // There are actually 15 different combinations for tracking, of which
            // GL only supports the most used 5. This means that we have to do some
            // magic to find the best match. NOTE: 
            //  GL_AMBIENT_AND_DIFFUSE != GL_AMBIENT | GL__DIFFUSE
            if(tracking & TVC_AMBIENT) 
            {
                if(tracking & TVC_DIFFUSE)
                {
                    gt = GL_AMBIENT_AND_DIFFUSE;
                } 
                else 
                {
                    gt = GL_AMBIENT;
                }
            }
            else if(tracking & TVC_DIFFUSE) 
            {
                gt = GL_DIFFUSE;
            }
            else if(tracking & TVC_SPECULAR) 
            {
                gt = GL_SPECULAR;              
            }
            else if(tracking & TVC_EMISSIVE) 
            {
                gt = GL_EMISSION;
            }
            glColorMaterial(GL_FRONT_AND_BACK, gt);
            
            glEnable(GL_COLOR_MATERIAL);
        } 
        else 
        {
            glDisable(GL_COLOR_MATERIAL);          
        }
    }

    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setTexture(size_t stage, bool enabled, const String &texname)
    {
        GLTexturePtr tex = TextureManager::getSingleton().getByName(texname);

        GLenum lastTextureType = mTextureTypes[stage];

		glActiveTextureARB_ptr( GL_TEXTURE0 + stage );
		if (enabled)
        {
            if (!tex.isNull())
            {
                // note used
                tex->touch();
                mTextureTypes[stage] = tex->getGLTextureTarget();
            }
            else
                // assume 2D
                mTextureTypes[stage] = GL_TEXTURE_2D;

            if(lastTextureType != mTextureTypes[stage] && lastTextureType != 0)
            {
				if (stage < mFixedFunctionTextureUnits)
				{
	                glDisable( lastTextureType );
				}
            }

			if (stage < mFixedFunctionTextureUnits)
			{
				glEnable( mTextureTypes[stage] );
			}

			if(!tex.isNull())
				glBindTexture( mTextureTypes[stage], tex->getGLID() );
			else
				glBindTexture( mTextureTypes[stage], static_cast<GLTextureManager*>(mTextureManager)->getWarningTextureID() );
        }
        else
        {
			if (stage < mFixedFunctionTextureUnits)
			{
				if (lastTextureType != 0)
				{
					glDisable( mTextureTypes[stage] );
				}
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			}
        }

        glActiveTextureARB_ptr( GL_TEXTURE0 );

	}

    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setTextureCoordSet(size_t stage, size_t index)
    {
        mTextureCoordIndex[stage] = index;
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setTextureCoordCalculation(size_t stage, TexCoordCalcMethod m, 
        const Frustum* frustum)
    {
		if (stage >= mFixedFunctionTextureUnits)
		{
			// Can't do this
			return;
		}

		
		GLfloat M[16];
        Matrix4 projectionBias;

        // Default to no extra auto texture matrix
        mUseAutoTextureMatrix = false;

        GLfloat eyePlaneS[] = {1.0, 0.0, 0.0, 0.0};
        GLfloat eyePlaneT[] = {0.0, 1.0, 0.0, 0.0};
        GLfloat eyePlaneR[] = {0.0, 0.0, 1.0, 0.0};
        GLfloat eyePlaneQ[] = {0.0, 0.0, 0.0, 1.0};

        glActiveTextureARB_ptr( GL_TEXTURE0 + stage );

		switch( m )
        {
        case TEXCALC_NONE:
            glDisable( GL_TEXTURE_GEN_S );
            glDisable( GL_TEXTURE_GEN_T );
            glDisable( GL_TEXTURE_GEN_R );
            glDisable( GL_TEXTURE_GEN_Q );
            break;

        case TEXCALC_ENVIRONMENT_MAP:
            glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
            glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );

            glEnable( GL_TEXTURE_GEN_S );
            glEnable( GL_TEXTURE_GEN_T );
            glDisable( GL_TEXTURE_GEN_R );
            glDisable( GL_TEXTURE_GEN_Q );

            // Need to use a texture matrix to flip the spheremap
            mUseAutoTextureMatrix = true;
            memset(mAutoTextureMatrix, 0, sizeof(GLfloat)*16);
            mAutoTextureMatrix[0] = mAutoTextureMatrix[10] = mAutoTextureMatrix[15] = 1.0f;
            mAutoTextureMatrix[5] = -1.0f;

            break;

        case TEXCALC_ENVIRONMENT_MAP_PLANAR:            
            // XXX This doesn't seem right?!
#ifdef GL_VERSION_1_3
            glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
            glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
            glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );

            glEnable( GL_TEXTURE_GEN_S );
            glEnable( GL_TEXTURE_GEN_T );
            glEnable( GL_TEXTURE_GEN_R );
            glDisable( GL_TEXTURE_GEN_Q );
#else
            glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
            glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );

            glEnable( GL_TEXTURE_GEN_S );
            glEnable( GL_TEXTURE_GEN_T );
            glDisable( GL_TEXTURE_GEN_R );
            glDisable( GL_TEXTURE_GEN_Q );
#endif
            break;
        case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
            
            glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
            glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
            glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );

            glEnable( GL_TEXTURE_GEN_S );
            glEnable( GL_TEXTURE_GEN_T );
            glEnable( GL_TEXTURE_GEN_R );
            glDisable( GL_TEXTURE_GEN_Q );

            // We need an extra texture matrix here
            // This sets the texture matrix to be the inverse of the modelview matrix
            mUseAutoTextureMatrix = true;
            glGetFloatv( GL_MODELVIEW_MATRIX, M );

            // Transpose 3x3 in order to invert matrix (rotation)
            // Note that we need to invert the Z _before_ the rotation
            // No idea why we have to invert the Z at all, but reflection is wrong without it
            mAutoTextureMatrix[0] = M[0]; mAutoTextureMatrix[1] = M[4]; mAutoTextureMatrix[2] = -M[8];
            mAutoTextureMatrix[4] = M[1]; mAutoTextureMatrix[5] = M[5]; mAutoTextureMatrix[6] = -M[9];
            mAutoTextureMatrix[8] = M[2]; mAutoTextureMatrix[9] = M[6]; mAutoTextureMatrix[10] = -M[10];
            mAutoTextureMatrix[3] = mAutoTextureMatrix[7] = mAutoTextureMatrix[11] = 0.0f;
            mAutoTextureMatrix[12] = mAutoTextureMatrix[13] = mAutoTextureMatrix[14] = 0.0f;
            mAutoTextureMatrix[15] = 1.0f;

            break;
        case TEXCALC_ENVIRONMENT_MAP_NORMAL:
            glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP );
            glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP );
            glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP );

            glEnable( GL_TEXTURE_GEN_S );
            glEnable( GL_TEXTURE_GEN_T );
            glEnable( GL_TEXTURE_GEN_R );
            glDisable( GL_TEXTURE_GEN_Q );
            break;
        case TEXCALC_PROJECTIVE_TEXTURE:
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
            glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
            glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
            glTexGenfv(GL_S, GL_EYE_PLANE, eyePlaneS);
            glTexGenfv(GL_T, GL_EYE_PLANE, eyePlaneT);
            glTexGenfv(GL_R, GL_EYE_PLANE, eyePlaneR);
            glTexGenfv(GL_Q, GL_EYE_PLANE, eyePlaneQ);
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);
            glEnable(GL_TEXTURE_GEN_R);
            glEnable(GL_TEXTURE_GEN_Q);

            mUseAutoTextureMatrix = true;

            // Set scale and translation matrix for projective textures
            projectionBias = Matrix4::CLIPSPACE2DTOIMAGESPACE;

            projectionBias = projectionBias * frustum->getProjectionMatrix();
            projectionBias = projectionBias * frustum->getViewMatrix();
            projectionBias = projectionBias * mWorldMatrix;

            makeGLMatrix(mAutoTextureMatrix, projectionBias);
            break;
        default:
            break;
        }
        glActiveTextureARB_ptr( GL_TEXTURE0 );
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setTextureAddressingMode(size_t stage, TextureUnitState::TextureAddressingMode tam)
    {
		GLint type;
        switch(tam)
        {
        default:
        case TextureUnitState::TAM_WRAP:
            type = GL_REPEAT;
            break;
        case TextureUnitState::TAM_MIRROR:
            type = GL_MIRRORED_REPEAT;
            break;
        case TextureUnitState::TAM_CLAMP:
            type = GL_CLAMP_TO_EDGE;
            break;
        }

        glActiveTextureARB_ptr( GL_TEXTURE0 + stage );
        glTexParameteri( mTextureTypes[stage], GL_TEXTURE_WRAP_S, type );
        glTexParameteri( mTextureTypes[stage], GL_TEXTURE_WRAP_T, type );
        glTexParameteri( mTextureTypes[stage], GL_TEXTURE_WRAP_R, type );
        glActiveTextureARB_ptr( GL_TEXTURE0 );
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setTextureMatrix(size_t stage, const Matrix4& xform)
    {
		if (stage >= mFixedFunctionTextureUnits)
		{
			// Can't do this
			return;
		}

		GLfloat mat[16];
        makeGLMatrix(mat, xform);

		if(mTextureTypes[stage] != GL_TEXTURE_3D && 
		   mTextureTypes[stage] != GL_TEXTURE_CUBE_MAP)
		{
			// Convert 3x3 rotation/translation matrix to 4x4
			mat[12] = mat[8];
			mat[13] = mat[9];
            mat[8] = 0;
            mat[9] = 0;
		}
//        mat[14] = mat[10];
//        mat[15] = mat[11];

//        for (int j=0; j< 4; j++)
//        {
//            int x = 0;
//            for (x = 0; x < 4; x++)
//            {
//                printf("[%2d]=%0.2f\t", (x*4) + j, mat[(x*4) + j]);
//            }
//            printf("\n");
//        }

        glActiveTextureARB_ptr(GL_TEXTURE0 + stage);
        glMatrixMode(GL_TEXTURE);

        // Load this matrix in
        glLoadMatrixf(mat);

        if (mUseAutoTextureMatrix)
        {
            // Concat auto matrix
            glMultMatrixf(mAutoTextureMatrix);
        }

        glMatrixMode(GL_MODELVIEW);
        glActiveTextureARB_ptr(GL_TEXTURE0);
    }
    //-----------------------------------------------------------------------------
    GLint GLRenderSystem::getBlendMode(SceneBlendFactor ogreBlend) const
    {
        switch(ogreBlend)
        {
        case SBF_ONE:
            return GL_ONE;
        case SBF_ZERO:
            return GL_ZERO;
        case SBF_DEST_COLOUR:
            return GL_DST_COLOR;
        case SBF_SOURCE_COLOUR:
            return GL_SRC_COLOR;
        case SBF_ONE_MINUS_DEST_COLOUR:
            return GL_ONE_MINUS_DST_COLOR;
        case SBF_ONE_MINUS_SOURCE_COLOUR:
            return GL_ONE_MINUS_SRC_COLOR;
        case SBF_DEST_ALPHA:
            return GL_DST_ALPHA;
        case SBF_SOURCE_ALPHA:
            return GL_SRC_ALPHA;
        case SBF_ONE_MINUS_DEST_ALPHA:
            return GL_ONE_MINUS_DST_ALPHA;
        case SBF_ONE_MINUS_SOURCE_ALPHA:
            return GL_ONE_MINUS_SRC_ALPHA;
        };
		// to keep compiler happy
		return GL_ONE;
    }

    void GLRenderSystem::_setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor)
    {
        GLint sourceBlend = getBlendMode(sourceFactor);
        GLint destBlend = getBlendMode(destFactor);
        if(sourceFactor == SBF_ONE && destFactor == SBF_ZERO)
        {
            glDisable(GL_BLEND);
        }
        else
        {
            glEnable(GL_BLEND);
            glBlendFunc(sourceBlend, destBlend);
        }
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setAlphaRejectSettings(CompareFunction func, unsigned char value)
    {
        if(func == CMPF_ALWAYS_PASS)
        {
            glDisable(GL_ALPHA_TEST);
        }
        else
        {
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(convertCompareFunction(func), value / 255.0f);
        }
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setViewport(Viewport *vp)
    {
        // Check if viewport is different
        if (vp != mActiveViewport || vp->_isUpdated())
        {
            RenderTarget* target;
            target = vp->getTarget();
            _setRenderTarget(target);
            mActiveViewport = vp;
              
              GLsizei x, y, w, h;
 
              // Calculate the "lower-left" corner of the viewport
              w = vp->getActualWidth();
              h = vp->getActualHeight();
              x = vp->getActualLeft();
              y = vp->getActualTop();
              if (!target->requiresTextureFlipping())
              {
                  // Convert "upper-left" corner to "lower-left"
                  y = target->getHeight() - h - y;
              }
              glViewport(x, y, w, h);
  
              // Configure the viewport clipping
              glScissor(x, y, w, h);
  
              vp->_clearUpdatedFlag();
        }
    }

	void GLRenderSystem::setLights()
	{
        for (size_t i = 0; i < MAX_LIGHTS; ++i)
        {
            if (mLights[i] != NULL)
            {
                Light* lt = mLights[i];
                setGLLightPositionDirection(lt, GL_LIGHT0 + i);
            }
        }
	}

    //-----------------------------------------------------------------------------
    void GLRenderSystem::_beginFrame(void)
    {
        OgreGuard( "GLRenderSystem::_beginFrame" );
        
        if (!mActiveViewport)
            OGRE_EXCEPT(999, "Cannot begin frame - no viewport selected.",
                "GLRenderSystem::_beginFrame");

        // Activate the viewport clipping
        glEnable(GL_SCISSOR_TEST);
        // Clear the viewport if required
        if (mActiveViewport->getClearEveryFrame())
        {
            clearFrameBuffer(FBT_COLOUR | FBT_DEPTH, 
                mActiveViewport->getBackgroundColour());
        }        

        OgreUnguard();
    }
   
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_endFrame(void)
    {
        // Deactivate the viewport clipping.
        glDisable(GL_SCISSOR_TEST);
    }

    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setCullingMode(CullingMode mode)
    {
        // NB: Because two-sided stencil API dependence of the front face, we must
        // use the same 'winding' for the front face everywhere. As the OGRE default
        // culling mode is clockwise, we also treat anticlockwise winding as front
        // face for consistently. On the assumption that, we can't change the front
        // face by glFrontFace anywhere.

        GLenum cullMode;

        switch( mode )
        {
        case CULL_NONE:
            glDisable( GL_CULL_FACE );
            return;
        case CULL_CLOCKWISE:
            if (mActiveRenderTarget && 
                ((mActiveRenderTarget->requiresTextureFlipping() && !mInvertVertexWinding) ||
                (!mActiveRenderTarget->requiresTextureFlipping() && mInvertVertexWinding)))
            {
                cullMode = GL_FRONT;
            }
            else
            {
                cullMode = GL_BACK;
            }
            break;
        case CULL_ANTICLOCKWISE:
            if (mActiveRenderTarget && 
                ((mActiveRenderTarget->requiresTextureFlipping() && !mInvertVertexWinding) ||
                (!mActiveRenderTarget->requiresTextureFlipping() && mInvertVertexWinding)))
            {
                cullMode = GL_BACK;
            }
            else
            {
                cullMode = GL_FRONT;
            }
            break;
        }

        glEnable( GL_CULL_FACE );
        glCullFace( cullMode );
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setDepthBufferParams(bool depthTest, bool depthWrite, CompareFunction depthFunction)
    {
        _setDepthBufferCheckEnabled(depthTest);
        _setDepthBufferWriteEnabled(depthWrite);
        _setDepthBufferFunction(depthFunction);
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setDepthBufferCheckEnabled(bool enabled)
    {
        if (enabled)
        {
            glClearDepth(1.0f);
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setDepthBufferWriteEnabled(bool enabled)
    {
        GLboolean flag = enabled ? GL_TRUE : GL_FALSE;
        glDepthMask( flag );  
        // Store for reference in _beginFrame
        mDepthWrite = enabled;
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setDepthBufferFunction(CompareFunction func)
    {
        glDepthFunc(convertCompareFunction(func));
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setDepthBias(ushort bias)
    {
        if (bias > 0)
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glEnable(GL_POLYGON_OFFSET_POINT);
            glEnable(GL_POLYGON_OFFSET_LINE);
            // Bias is in {0, 16}, scale the unit addition appropriately
            glPolygonOffset(0.0f, -bias);
        }
        else
        {
            glDisable(GL_POLYGON_OFFSET_FILL);
            glDisable(GL_POLYGON_OFFSET_POINT);
            glDisable(GL_POLYGON_OFFSET_LINE);
        }
    }
	//-----------------------------------------------------------------------------
	void GLRenderSystem::_setColourBufferWriteEnabled(bool red, bool green, bool blue, bool alpha)
	{
		glColorMask(red, green, blue, alpha);
		// record this
		mColourWrite[0] = red;
		mColourWrite[1] = blue;
		mColourWrite[2] = green;
		mColourWrite[3] = alpha;
	}
	//-----------------------------------------------------------------------------
    String GLRenderSystem::getErrorDescription(long errCode) const
    {
        const GLubyte *errString = gluErrorString (errCode);
        return String((const char*) errString);
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::setLightingEnabled(bool enabled)
    {
        if (enabled) 
        {      
            glEnable(GL_LIGHTING);
        } 
        else 
        {
            glDisable(GL_LIGHTING);
        }
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setFog(FogMode mode, const ColourValue& colour, Real density, Real start, Real end)
    {

        GLint fogMode;
        switch (mode)
        {
        case FOG_EXP:
            fogMode = GL_EXP;
            break;
        case FOG_EXP2:
            fogMode = GL_EXP2;
            break;
        case FOG_LINEAR:
            fogMode = GL_LINEAR;
            break;
        default:
            // Give up on it
            glDisable(GL_FOG);
            return;
        }

        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, fogMode);
        GLfloat fogColor[4] = {colour.r, colour.g, colour.b, colour.a};
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogf(GL_FOG_DENSITY, density);
        glFogf(GL_FOG_START, start);
        glFogf(GL_FOG_END, end);
        // XXX Hint here?
    }

    void GLRenderSystem::convertColourValue(const ColourValue& colour, uint32* pDest)
    {
    #if OGRE_ENDIAN == OGRE_ENDIAN_BIG
        *pDest = colour.getAsRGBA();
    #else
      // GL accesses by byte, so use ABGR so little-endian format will make it RGBA in byte mode
        *pDest = colour.getAsABGR();
    #endif
    }
    
    void GLRenderSystem::_makeProjectionMatrix(const Radian& fovy, Real aspect, Real nearPlane, 
        Real farPlane, Matrix4& dest, bool forGpuProgram)
    {
        Radian thetaY ( fovy / 2.0f );
        Real tanThetaY = Math::Tan(thetaY);
        //Real thetaX = thetaY * aspect;
        //Real tanThetaX = Math::Tan(thetaX);

        // Calc matrix elements
        Real w = (1.0f / tanThetaY) / aspect;
        Real h = 1.0f / tanThetaY;
        Real q, qn;
        if (farPlane == 0)
        {
            // Infinite far plane
            q = Frustum::INFINITE_FAR_PLANE_ADJUST - 1;
            qn = nearPlane * (Frustum::INFINITE_FAR_PLANE_ADJUST - 2);
        }
        else
        {
            q = -(farPlane + nearPlane) / (farPlane - nearPlane);
            qn = -2 * (farPlane * nearPlane) / (farPlane - nearPlane);
        }

        // NB This creates Z in range [-1,1]
        //
        // [ w   0   0   0  ]
        // [ 0   h   0   0  ]
        // [ 0   0   q   qn ]
        // [ 0   0   -1  0  ]

        dest = Matrix4::ZERO;
        dest[0][0] = w;
        dest[1][1] = h;
        dest[2][2] = q;
        dest[2][3] = qn;
        dest[3][2] = -1;

    }
    
    void GLRenderSystem::_makeOrthoMatrix(const Radian& fovy, Real aspect, Real nearPlane, 
        Real farPlane, Matrix4& dest, bool forGpuProgram)
    {
        Radian thetaY (fovy / 2.0f);
        Real tanThetaY = Math::Tan(thetaY);

        //Real thetaX = thetaY * aspect;
        Real tanThetaX = tanThetaY * aspect; //Math::Tan(thetaX);
        Real half_w = tanThetaX * nearPlane;
        Real half_h = tanThetaY * nearPlane;
        Real iw = 1.0 / half_w;
        Real ih = 1.0 / half_h;
        Real q;
        if (farPlane == 0)
        {
            q = 0;
        }
        else
        {
            q = 2.0 / (farPlane - nearPlane);
        }
        dest = Matrix4::ZERO;
        dest[0][0] = iw;
        dest[1][1] = ih;
        dest[2][2] = -q;
        dest[2][3] = - (farPlane + nearPlane)/(farPlane - nearPlane);
        dest[3][3] = 1;
	}

    void GLRenderSystem::_setRasterisationMode(SceneDetailLevel level)
    {
        GLenum glmode;
        switch(level)
        {
        case SDL_POINTS:
            glmode = GL_POINT;
            break;
        case SDL_WIREFRAME:
            glmode = GL_LINE;
            break;
        case SDL_SOLID:
            glmode = GL_FILL;
            break;
        }
        glPolygonMode(GL_FRONT_AND_BACK, glmode);
    }
    //---------------------------------------------------------------------
    void GLRenderSystem::setStencilCheckEnabled(bool enabled)
    {
        if (enabled)
        {
            glEnable(GL_STENCIL_TEST);
        }
        else
        {
            glDisable(GL_STENCIL_TEST);
        }
    }
    //---------------------------------------------------------------------
    void GLRenderSystem::setStencilBufferParams(CompareFunction func, 
        uint32 refValue, uint32 mask, StencilOperation stencilFailOp, 
        StencilOperation depthFailOp, StencilOperation passOp, 
        bool twoSidedOperation)
    {
        bool flip;

        if (twoSidedOperation)
        {
            if (!mCapabilities->hasCapability(RSC_TWO_SIDED_STENCIL))
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "2-sided stencils are not supported",
                    "GLRenderSystem::setStencilBufferParams");
            glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
            // NB: We should always treat CCW as front face for consistent with default
            // culling mode. Therefore, we must take care with two-sided stencil settings.
            flip = (mInvertVertexWinding && !mActiveRenderTarget->requiresTextureFlipping()) ||
                   (!mInvertVertexWinding && mActiveRenderTarget->requiresTextureFlipping());

            // Set alternative versions of ops
            glActiveStencilFaceEXT_ptr(GL_BACK);
            glStencilMask(mask);
            glStencilFunc(convertCompareFunction(func), refValue, mask);
            glStencilOp(
                convertStencilOp(stencilFailOp, !flip), 
                convertStencilOp(depthFailOp, !flip), 
                convertStencilOp(passOp, !flip));
            // reset
            glActiveStencilFaceEXT_ptr(GL_FRONT);
        }
        else
        {
            glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
            flip = false;
        }

        glStencilMask(mask);
        glStencilFunc(convertCompareFunction(func), refValue, mask);
        glStencilOp(
            convertStencilOp(stencilFailOp, flip),
            convertStencilOp(depthFailOp, flip), 
            convertStencilOp(passOp, flip));
    }
    //---------------------------------------------------------------------
    GLint GLRenderSystem::convertCompareFunction(CompareFunction func) const
    {
        switch(func)
        {
        case CMPF_ALWAYS_FAIL:
            return GL_NEVER;
        case CMPF_ALWAYS_PASS:
            return GL_ALWAYS;
        case CMPF_LESS:
            return GL_LESS;
        case CMPF_LESS_EQUAL:
            return GL_LEQUAL;
        case CMPF_EQUAL:
            return GL_EQUAL;
        case CMPF_NOT_EQUAL:
            return GL_NOTEQUAL;
        case CMPF_GREATER_EQUAL:
            return GL_GEQUAL;
        case CMPF_GREATER:
            return GL_GREATER;
        };
        // to keep compiler happy
        return GL_ALWAYS;
    }
    //---------------------------------------------------------------------
    GLint GLRenderSystem::convertStencilOp(StencilOperation op, bool invert) const
    {
        switch(op)
        {
        case SOP_KEEP:
            return GL_KEEP;
        case SOP_ZERO:
            return GL_ZERO;
        case SOP_REPLACE:
            return GL_REPLACE;
        case SOP_INCREMENT:
            return invert ? GL_DECR : GL_INCR;
        case SOP_DECREMENT:
            return invert ? GL_INCR : GL_DECR;
        case SOP_INCREMENT_WRAP:
            return invert ? GL_DECR_WRAP_EXT : GL_INCR_WRAP_EXT;
        case SOP_DECREMENT_WRAP:
            return invert ? GL_INCR_WRAP_EXT : GL_DECR_WRAP_EXT;
        case SOP_INVERT:
            return GL_INVERT;
        };
        // to keep compiler happy
        return SOP_KEEP;
    }
	//---------------------------------------------------------------------
    GLuint GLRenderSystem::getCombinedMinMipFilter(void) const
    {
        switch(mMinFilter)
        {
        case FO_ANISOTROPIC:
        case FO_LINEAR:
            switch(mMipFilter)
            {
            case FO_ANISOTROPIC:
            case FO_LINEAR:
                // linear min, linear mip
                return GL_LINEAR_MIPMAP_LINEAR;
            case FO_POINT:
                // linear min, point mip
                return GL_LINEAR_MIPMAP_NEAREST;
            case FO_NONE:
                // linear min, no mip
                return GL_LINEAR;
            }
            break;
        case FO_POINT:
        case FO_NONE:
            switch(mMipFilter)
            {
            case FO_ANISOTROPIC:
            case FO_LINEAR:
                // nearest min, linear mip
                return GL_NEAREST_MIPMAP_LINEAR;
            case FO_POINT:
                // nearest min, point mip
                return GL_NEAREST_MIPMAP_NEAREST;
            case FO_NONE:
                // nearest min, no mip
                return GL_NEAREST;
            }
            break;
        }

        // should never get here
        return 0;

    }
	//---------------------------------------------------------------------
    void GLRenderSystem::_setTextureUnitFiltering(size_t unit, 
        FilterType ftype, FilterOptions fo)
	{
        OgreGuard( "GLRenderSystem::_setTextureUnitFiltering" );        

		glActiveTextureARB_ptr( GL_TEXTURE0 + unit );
        switch(ftype)
        {
        case FT_MIN:
            mMinFilter = fo;
            // Combine with existing mip filter
			glTexParameteri(
                mTextureTypes[unit],
				GL_TEXTURE_MIN_FILTER, 
				getCombinedMinMipFilter());
            break;
        case FT_MAG:
            switch (fo)
            {
            case FO_ANISOTROPIC: // GL treats linear and aniso the same
            case FO_LINEAR:
			    glTexParameteri(
                    mTextureTypes[unit],
				    GL_TEXTURE_MAG_FILTER, 
				    GL_LINEAR);
                break;
            case FO_POINT:
            case FO_NONE:
			    glTexParameteri(
                    mTextureTypes[unit],
				    GL_TEXTURE_MAG_FILTER, 
				    GL_NEAREST);
                break;
            }
            break;
        case FT_MIP:
            mMipFilter = fo;
            // Combine with existing min filter
			glTexParameteri(
                mTextureTypes[unit],
				GL_TEXTURE_MIN_FILTER, 
				getCombinedMinMipFilter());
            break;
		}

        glActiveTextureARB_ptr( GL_TEXTURE0 );

		OgreUnguard();
	}
	//---------------------------------------------------------------------
	GLfloat GLRenderSystem::_getCurrentAnisotropy(size_t unit)
	{
		GLfloat curAniso = 0;
		glGetTexParameterfv(mTextureTypes[unit], 
            GL_TEXTURE_MAX_ANISOTROPY_EXT, &curAniso);
		return curAniso ? curAniso : 1;
	}
	//---------------------------------------------------------------------
	void GLRenderSystem::_setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy)
	{
       if (!mCapabilities->hasCapability(RSC_ANISOTROPY))
			return;

		GLfloat largest_supported_anisotropy = 0;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
		if (maxAnisotropy > largest_supported_anisotropy)
			maxAnisotropy = largest_supported_anisotropy ? largest_supported_anisotropy : 1;
		if (_getCurrentAnisotropy(unit) != maxAnisotropy)
			glTexParameterf(mTextureTypes[unit], GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
	}
	//-----------------------------------------------------------------------------
    void GLRenderSystem::_setTextureBlendMode(size_t stage, const LayerBlendModeEx& bm)
    {       
		if (stage >= mFixedFunctionTextureUnits)
		{
			// Can't do this
			return;
		}

		// Check to see if blending is supported
        if(!mCapabilities->hasCapability(RSC_BLENDING))
            return;

        GLenum src1op, src2op, cmd;
        GLfloat cv1[4], cv2[4];

		if (bm.blendType == LBT_COLOUR)
        {
		    cv1[0] = bm.colourArg1.r;
		    cv1[1] = bm.colourArg1.g;
		    cv1[2] = bm.colourArg1.b;
		    cv1[3] = bm.colourArg1.a;
			mManualBlendColours[stage][0] = bm.colourArg1;


		    cv2[0] = bm.colourArg2.r;
		    cv2[1] = bm.colourArg2.g;
		    cv2[2] = bm.colourArg2.b;
		    cv2[3] = bm.colourArg2.a;
			mManualBlendColours[stage][1] = bm.colourArg2;
        }

		if (bm.blendType == LBT_ALPHA)
        {
		    cv1[0] = mManualBlendColours[stage][0].r;
		    cv1[1] = mManualBlendColours[stage][0].g;
		    cv1[2] = mManualBlendColours[stage][0].b;
		    cv1[3] = bm.alphaArg1;

		    cv2[0] = mManualBlendColours[stage][1].r;
		    cv2[1] = mManualBlendColours[stage][1].g;
		    cv2[2] = mManualBlendColours[stage][1].b;
		    cv2[3] = bm.alphaArg2;
        }

        switch (bm.source1)
        {
        case LBS_CURRENT:
            src1op = GL_PREVIOUS;
            break;
        case LBS_TEXTURE:
            src1op = GL_TEXTURE;
            break;
        case LBS_MANUAL:
            src1op = GL_CONSTANT;
			break;
        case LBS_DIFFUSE:
            src1op = GL_PRIMARY_COLOR;
			break;
        // XXX
        case LBS_SPECULAR:
		default:
            src1op = 0;
        }

        switch (bm.source2)
        {
        case LBS_CURRENT:
            src2op = GL_PREVIOUS;
            break;
        case LBS_TEXTURE:
            src2op = GL_TEXTURE;
            break;
        case LBS_MANUAL:
			src2op = GL_CONSTANT;
			break;
		case LBS_DIFFUSE:
            src2op = GL_PRIMARY_COLOR;
			break;
        // XXX
        case LBS_SPECULAR:
		default:
            src2op = 0;
        }

        switch (bm.operation)
        {
        case LBX_SOURCE1:
            cmd = GL_REPLACE;
            break;
        case LBX_SOURCE2:
            cmd = GL_REPLACE;
            break;
        case LBX_MODULATE:
            cmd = GL_MODULATE;
            break;
        case LBX_MODULATE_X2:
            cmd = GL_MODULATE;
            break;
        case LBX_MODULATE_X4:
            cmd = GL_MODULATE;
            break;
        case LBX_ADD:
            cmd = GL_ADD;
            break;
        case LBX_ADD_SIGNED:
            cmd = GL_ADD_SIGNED;
            break;
        case LBX_SUBTRACT:
            cmd = GL_SUBTRACT;
            break;
        case LBX_BLEND_DIFFUSE_ALPHA:
            cmd = GL_INTERPOLATE;
            break;
        case LBX_BLEND_TEXTURE_ALPHA:
            cmd = GL_INTERPOLATE;
            break;
        case LBX_BLEND_CURRENT_ALPHA:
            cmd = GL_INTERPOLATE;
            break;
        case LBX_BLEND_MANUAL:
            cmd = GL_INTERPOLATE;
            break;
        case LBX_DOTPRODUCT:
            cmd = mCapabilities->hasCapability(RSC_DOT3) 
                ? GL_DOT3_RGB : GL_MODULATE;
            break;
		default:
            cmd = 0;
        }

		glActiveTextureARB_ptr(GL_TEXTURE0 + stage);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

	    if (bm.blendType == LBT_COLOUR)
	    {
		    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, cmd);
		    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, src1op);
		    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, src2op);
		    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
	    }
	    else
	    {
		    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, cmd);
		    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, src1op);
		    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, src2op);
		    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_CONSTANT);
	    }

        float blendValue[4] = {0, 0, 0, bm.factor};
        switch (bm.operation)
        {
        case LBX_BLEND_DIFFUSE_ALPHA:
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PRIMARY_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_PRIMARY_COLOR);
            break;
        case LBX_BLEND_TEXTURE_ALPHA:
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_TEXTURE);
            break;
        case LBX_BLEND_CURRENT_ALPHA:
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PREVIOUS);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_PREVIOUS);
            break;
        case LBX_BLEND_MANUAL:
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, blendValue);
            break;
        default:
            break;
        };

        switch (bm.operation)
        {
        case LBX_MODULATE_X2:
			glTexEnvi(GL_TEXTURE_ENV, bm.blendType == LBT_COLOUR ? 
                GL_RGB_SCALE : GL_ALPHA_SCALE, 2);
            break;
        case LBX_MODULATE_X4:
			glTexEnvi(GL_TEXTURE_ENV, bm.blendType == LBT_COLOUR ? 
                GL_RGB_SCALE : GL_ALPHA_SCALE, 4);
            break;
        default:
			glTexEnvi(GL_TEXTURE_ENV, bm.blendType == LBT_COLOUR ? 
                GL_RGB_SCALE : GL_ALPHA_SCALE, 1);
            break;
		}

		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);
        if(bm.source1 == LBS_MANUAL)
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, cv1);
        if (bm.source2 == LBS_MANUAL)
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, cv2);

        glActiveTextureARB_ptr(GL_TEXTURE0);
	}
    //---------------------------------------------------------------------
    void GLRenderSystem::setGLLightPositionDirection(Light* lt, GLenum lightindex)
    {
        // Set position / direction
        Vector4 vec;
		// Use general 4D vector which is the same as GL's approach
		vec = lt->getAs4DVector();

#if OGRE_DOUBLE_PRECISION
		// Must convert to float*
		float tmp[4] = {vec.x, vec.y, vec.z, vec.w};
		glLightfv(lightindex, GL_POSITION, tmp);
#else
		glLightfv(lightindex, GL_POSITION, vec.val);
#endif
		// Set spotlight direction
        if (lt->getType() == Light::LT_SPOTLIGHT)
        {
            vec = lt->getDerivedDirection();
            vec.w = 0.0; 
#if OGRE_DOUBLE_PRECISION
			// Must convert to float*
			float tmp2[4] = {vec.x, vec.y, vec.z, vec.w};
			glLightfv(lightindex, GL_SPOT_DIRECTION, tmp2);
#else
            glLightfv(lightindex, GL_SPOT_DIRECTION, vec.val);
#endif
        }
    }
    //---------------------------------------------------------------------
	void GLRenderSystem::setVertexDeclaration(VertexDeclaration* decl)
	{
	}
    //---------------------------------------------------------------------
	void GLRenderSystem::setVertexBufferBinding(VertexBufferBinding* binding)
	{
	}
    //---------------------------------------------------------------------
    void GLRenderSystem::_render(const RenderOperation& op)
	{
        // Guard
        OgreGuard ("GLRenderSystem::_render");
        // Call super class
        RenderSystem::_render(op);

        void* pBufferData = 0;
        
        const VertexDeclaration::VertexElementList& decl = 
            op.vertexData->vertexDeclaration->getElements();
        VertexDeclaration::VertexElementList::const_iterator elem, elemEnd;
        elemEnd = decl.end();

        for (elem = decl.begin(); elem != elemEnd; ++elem)
        {
            HardwareVertexBufferSharedPtr vertexBuffer = 
                op.vertexData->vertexBufferBinding->getBuffer(elem->getSource());
            if(mCapabilities->hasCapability(RSC_VBO))
            {
                glBindBufferARB_ptr(GL_ARRAY_BUFFER_ARB, 
                    static_cast<const GLHardwareVertexBuffer*>(vertexBuffer.get())->getGLBufferId());
                pBufferData = VBO_BUFFER_OFFSET(elem->getOffset());
            }
            else
            {
                pBufferData = static_cast<const GLDefaultHardwareVertexBuffer*>(vertexBuffer.get())->getDataPtr(elem->getOffset());
            }
            if (op.vertexData->vertexStart)
            {
                pBufferData = static_cast<char*>(pBufferData) + op.vertexData->vertexStart * vertexBuffer->getVertexSize();
            }

            unsigned int i = 0;

            switch(elem->getSemantic())
            {
            case VES_POSITION:
                glVertexPointer(VertexElement::getTypeCount(
                    elem->getType()), 
                    GLHardwareBufferManager::getGLType(elem->getType()), 
                    static_cast<GLsizei>(vertexBuffer->getVertexSize()), 
                    pBufferData);
                glEnableClientState( GL_VERTEX_ARRAY );
                break;
            case VES_NORMAL:
                glNormalPointer(
                    GLHardwareBufferManager::getGLType(elem->getType()), 
                    static_cast<GLsizei>(vertexBuffer->getVertexSize()), 
                    pBufferData);
                glEnableClientState( GL_NORMAL_ARRAY );
                break;
            case VES_DIFFUSE:
                glColorPointer(4, 
                    GLHardwareBufferManager::getGLType(elem->getType()), 
                    static_cast<GLsizei>(vertexBuffer->getVertexSize()), 
                    pBufferData);
                glEnableClientState( GL_COLOR_ARRAY );
                break;
            case VES_SPECULAR:
                glSecondaryColorPointerEXT_ptr(4, 
                    GLHardwareBufferManager::getGLType(elem->getType()), 
                    static_cast<GLsizei>(vertexBuffer->getVertexSize()), 
                    pBufferData);
                glEnableClientState( GL_SECONDARY_COLOR_ARRAY );
                break;
            case VES_TEXTURE_COORDINATES:

                for (i = 0; i < OGRE_MAX_TEXTURE_COORD_SETS; i++)
                {
					// Only set this texture unit's texcoord pointer if it
					// is supposed to be using this element's index
					if (mTextureCoordIndex[i] == elem->getIndex())
					{
						glClientActiveTextureARB_ptr(GL_TEXTURE0 + i);
						glTexCoordPointer(
							VertexElement::getTypeCount(elem->getType()), 
							GLHardwareBufferManager::getGLType(elem->getType()),
                            static_cast<GLsizei>(vertexBuffer->getVertexSize()), 
                                pBufferData);
						glEnableClientState( GL_TEXTURE_COORD_ARRAY );
					}
                }
                break;
            case VES_BLEND_INDICES:
                assert(mCapabilities->hasCapability(RSC_VERTEX_PROGRAM));
                glVertexAttribPointerARB_ptr(
                    7, // matrix indices are vertex attribute 7 (no def?)
                    VertexElement::getTypeCount(elem->getType()), 
                    GLHardwareBufferManager::getGLType(elem->getType()), 
                    GL_FALSE, // normalisation disabled
                    static_cast<GLsizei>(vertexBuffer->getVertexSize()), 
                    pBufferData);
                glEnableVertexAttribArrayARB_ptr(7);
                break;
            case VES_BLEND_WEIGHTS:
                assert(mCapabilities->hasCapability(RSC_VERTEX_PROGRAM));
                glVertexAttribPointerARB_ptr(
                    1, // weights are vertex attribute 1 (no def?)
                    VertexElement::getTypeCount(elem->getType()), 
                    GLHardwareBufferManager::getGLType(elem->getType()), 
                    GL_FALSE, // normalisation disabled
                    static_cast<GLsizei>(vertexBuffer->getVertexSize()), 
                    pBufferData);
                glEnableVertexAttribArrayARB_ptr(1);
                break;
            default:
                break;
            };

        }

        glClientActiveTextureARB_ptr(GL_TEXTURE0);

        // Find the correct type to render
        GLint primType;
        switch (op.operationType)
        {
        case RenderOperation::OT_POINT_LIST:
            primType = GL_POINTS;
            break;
        case RenderOperation::OT_LINE_LIST:
            primType = GL_LINES;
            break;
        case RenderOperation::OT_LINE_STRIP:
            primType = GL_LINE_STRIP;
            break;
        case RenderOperation::OT_TRIANGLE_LIST:
            primType = GL_TRIANGLES;
            break;
        case RenderOperation::OT_TRIANGLE_STRIP:
            primType = GL_TRIANGLE_STRIP;
            break;
        case RenderOperation::OT_TRIANGLE_FAN:
            primType = GL_TRIANGLE_FAN;
            break;
        }

        if (op.useIndexes)
        {
            if(mCapabilities->hasCapability(RSC_VBO))
            {
                glBindBufferARB_ptr(GL_ELEMENT_ARRAY_BUFFER_ARB, 
                    static_cast<GLHardwareIndexBuffer*>(
                        op.indexData->indexBuffer.get())->getGLBufferId());

                pBufferData = VBO_BUFFER_OFFSET(
                    op.indexData->indexStart * op.indexData->indexBuffer->getIndexSize());
            }
            else
            {
                pBufferData = static_cast<GLDefaultHardwareIndexBuffer*>(
                    op.indexData->indexBuffer.get())->getDataPtr(
                        op.indexData->indexStart * op.indexData->indexBuffer->getIndexSize());
            }

            GLenum indexType = (op.indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_16BIT) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

            glDrawElements(primType, op.indexData->indexCount, indexType, pBufferData);

        }
        else
        {
            glDrawArrays(primType, 0, op.vertexData->vertexCount);
        }

        glDisableClientState( GL_VERTEX_ARRAY );
        for (int i = 0; i < OGRE_MAX_TEXTURE_COORD_SETS; i++)
        {
            glClientActiveTextureARB_ptr(GL_TEXTURE0 + i);
            glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        }
        glClientActiveTextureARB_ptr(GL_TEXTURE0);
        glDisableClientState( GL_NORMAL_ARRAY );
        glDisableClientState( GL_COLOR_ARRAY );
        glDisableClientState( GL_SECONDARY_COLOR_ARRAY );
        if (mCapabilities->hasCapability(RSC_VERTEX_PROGRAM))
        {
            glDisableVertexAttribArrayARB_ptr(7); // disable indices
            glDisableVertexAttribArrayARB_ptr(1); // disable weights
        }
        glColor4f(1,1,1,1);
        glSecondaryColor3fEXT_ptr(0.0f, 0.0f, 0.0f);

        // UnGuard
        OgreUnguard();
	}
    //---------------------------------------------------------------------
    void GLRenderSystem::setNormaliseNormals(bool normalise)
    {
        if (normalise)
            glEnable(GL_NORMALIZE);
        else
            glDisable(GL_NORMALIZE);

    }
	//---------------------------------------------------------------------
    void GLRenderSystem::bindGpuProgram(GpuProgram* prg)
    {
        GLGpuProgram* glprg = static_cast<GLGpuProgram*>(prg);
        glprg->bindProgram();
        if (glprg->getType() == GPT_VERTEX_PROGRAM)
        {
            mCurrentVertexProgram = glprg;
        }
        else
        {
            mCurrentFragmentProgram = glprg;
        }
    }
	//---------------------------------------------------------------------
    void GLRenderSystem::unbindGpuProgram(GpuProgramType gptype)
    {

        if (gptype == GPT_VERTEX_PROGRAM && mCurrentVertexProgram)
        {
            mCurrentVertexProgram->unbindProgram();
            mCurrentVertexProgram = 0;
        }
        else if (gptype == GPT_FRAGMENT_PROGRAM && mCurrentFragmentProgram)
        {
            mCurrentFragmentProgram->unbindProgram();
            mCurrentFragmentProgram = 0;
        }


    }
	//---------------------------------------------------------------------
    void GLRenderSystem::bindGpuProgramParameters(GpuProgramType gptype, GpuProgramParametersSharedPtr params)
    {
        if (gptype == GPT_VERTEX_PROGRAM)
        {
            mCurrentVertexProgram->bindProgramParameters(params);
        }
        else
        {
            mCurrentFragmentProgram->bindProgramParameters(params);
        }
    }
	//---------------------------------------------------------------------
    void GLRenderSystem::setClipPlanes(const PlaneList& clipPlanes)
    {
        size_t i;
        size_t numClipPlanes;
        GLdouble clipPlane[4];

        numClipPlanes = clipPlanes.size();
        for (i = 0; i < numClipPlanes; ++i)
        {
            GLenum clipPlaneId = static_cast<GLenum>(GL_CLIP_PLANE0 + i);
            const Plane& plane = clipPlanes[i];

            if (i >= 6/*GL_MAX_CLIP_PLANES*/)
            {
                OGRE_EXCEPT(0, "Unable to set clip plane", 
                    "GLRenderSystem::setClipPlanes");
            }

            clipPlane[0] = plane.normal.x;
            clipPlane[1] = plane.normal.y;
            clipPlane[2] = plane.normal.z;
            clipPlane[3] = plane.d;

            glClipPlane(clipPlaneId, clipPlane);
            glEnable(clipPlaneId);
        }

            // disable remaining clip planes
        for ( ; i < 6/*GL_MAX_CLIP_PLANES*/; ++i)
        {
            glDisable(static_cast<GLenum>(GL_CLIP_PLANE0 + i));
        }
    }
	//---------------------------------------------------------------------
    void GLRenderSystem::setScissorTest(bool enabled, size_t left, 
        size_t top, size_t right, size_t bottom)
    {
        // If request texture flipping, use "upper-left", otherwise use "lower-left"
        bool flipping = mActiveRenderTarget->requiresTextureFlipping();
        //  GL measures from the bottom, not the top
        size_t targetHeight = mActiveRenderTarget->getHeight();
        // Calculate the "lower-left" corner of the viewport
        GLsizei w, h, x, y;

        if (enabled)
        {
            glEnable(GL_SCISSOR_TEST);
            // NB GL uses width / height rather than right / bottom
            x = left;
            if (flipping)
                y = top;
            else
                y = targetHeight - bottom;
            w = right - left;
            h = bottom - top;
            glScissor(x, y, w, h);
        }
        else
        {
            glDisable(GL_SCISSOR_TEST);
            // GL requires you to reset the scissor when disabling
            w = mActiveViewport->getActualWidth();
            h = mActiveViewport->getActualHeight();
            x = mActiveViewport->getActualLeft();
            if (flipping)
                y = mActiveViewport->getActualTop();
            else
                y = targetHeight - mActiveViewport->getActualTop() - h;
            glScissor(x, y, w, h);
        }
    }
    //---------------------------------------------------------------------
    void GLRenderSystem::clearFrameBuffer(unsigned int buffers, 
        const ColourValue& colour, Real depth, unsigned short stencil)
    {

        GLbitfield flags = 0;
        if (buffers & FBT_COLOUR)
        {
            flags |= GL_COLOR_BUFFER_BIT;
        }
        if (buffers & FBT_DEPTH)
        {
            flags |= GL_DEPTH_BUFFER_BIT;
        }
        if (buffers & FBT_STENCIL)
        {
            flags |= GL_STENCIL_BUFFER_BIT;
        }


        // Enable depth & colour buffer for writing if it isn't

        if (!mDepthWrite)
        {
            glDepthMask( GL_TRUE );
        }
        bool colourMask = !mColourWrite[0] || !mColourWrite[1] 
        || !mColourWrite[2] || mColourWrite[3]; 
        if (colourMask)
        {
            glColorMask(true, true, true, true);
        }
        // Set values
        glClearColor(colour.r, colour.g, colour.b, colour.a);
        glClearDepth(depth);
        glClearStencil(stencil);
        // Clear buffers
        glClear(flags);
        // Reset depth write state if appropriate
        // Enable depth buffer for writing if it isn't
        if (!mDepthWrite)
        {
            glDepthMask( GL_FALSE );
        }
        if (colourMask)
        {
            glColorMask(mColourWrite[0], mColourWrite[1], mColourWrite[2], mColourWrite[3]);
        }

    }
    // ------------------------------------------------------------------
    void GLRenderSystem::_makeProjectionMatrix(Real left, Real right, 
        Real bottom, Real top, Real nearPlane, Real farPlane, Matrix4& dest, 
        bool forGpuProgram)
    {
        Real width = right - left;
        Real height = top - bottom;
        Real q, qn;
        if (farPlane == 0)
        {
            // Infinite far plane
            q = Frustum::INFINITE_FAR_PLANE_ADJUST - 1;
            qn = nearPlane * (Frustum::INFINITE_FAR_PLANE_ADJUST - 2);
        }
        else
        {
            q = -(farPlane + nearPlane) / (farPlane - nearPlane);
            qn = -2 * (farPlane * nearPlane) / (farPlane - nearPlane);
        }
        dest = Matrix4::ZERO;
        dest[0][0] = 2 * nearPlane / width;
        dest[0][2] = (right+left) / width;
        dest[1][1] = 2 * nearPlane / height;
        dest[1][2] = (top+bottom) / height;
        dest[2][2] = q;
        dest[2][3] = qn;
        dest[3][2] = -1;
    }

    // ------------------------------------------------------------------
    void GLRenderSystem::setClipPlane (ushort index, Real A, Real B, Real C, Real D)
    {
        if (ushort(mClipPlanes.size()) < index+1)
            mClipPlanes.resize(index+1);
        mClipPlanes[index] = Vector4 (A, B, C, D);
        GLdouble plane[4] = { A, B, C, D };
        glClipPlane (GL_CLIP_PLANE0 + index, plane);
    }

    // ------------------------------------------------------------------
    void GLRenderSystem::setGLClipPlanes() const
    {
        size_t size = mClipPlanes.size();
        for (size_t i=0; i<size; i++)
        {
            const Vector4 &p = mClipPlanes[i];
            GLdouble plane[4] = { p.x, p.y, p.z, p.w };
            glClipPlane (GL_CLIP_PLANE0 + i, plane);
        }
    }

    // ------------------------------------------------------------------
    void GLRenderSystem::enableClipPlane (ushort index, bool enable)
    {
        glEnable (GL_CLIP_PLANE0 + index);
    }
    //---------------------------------------------------------------------
    HardwareOcclusionQuery* GLRenderSystem::createHardwareOcclusionQuery(void)
    {
        GLHardwareOcclusionQuery* ret = new GLHardwareOcclusionQuery(); 
		mHwOcclusionQueries.push_back(ret);
		return ret;
    }
    //---------------------------------------------------------------------
    Real GLRenderSystem::getHorizontalTexelOffset(void)
    {
        // No offset in GL
        return 0.0f;
    }
    //---------------------------------------------------------------------
    Real GLRenderSystem::getVerticalTexelOffset(void)
    {
        // No offset in GL
        return 0.0f;
    }
 	//---------------------------------------------------------------------
    void GLRenderSystem::_applyObliqueDepthProjection(Matrix4& matrix, const Plane& plane, 
        bool forGpuProgram)
    {
        // Thanks to Eric Lenyel for posting this calculation at www.terathon.com

        // Calculate the clip-space corner point opposite the clipping plane
        // as (sgn(clipPlane.x), sgn(clipPlane.y), 1, 1) and
        // transform it into camera space by multiplying it
        // by the inverse of the projection matrix

        Vector4 q;
        q.x = (Math::Sign(plane.normal.x) + matrix[0][2]) / matrix[0][0];
        q.y = (Math::Sign(plane.normal.y) + matrix[1][2]) / matrix[1][1];
        q.z = -1.0F;
        q.w = (1.0F + matrix[2][2]) / matrix[2][3];

        // Calculate the scaled plane vector
        Vector4 clipPlane4d(plane.normal.x, plane.normal.y, plane.normal.z, plane.d);
        Vector4 c = clipPlane4d * (2.0F / (clipPlane4d.dotProduct(q)));

        // Replace the third row of the projection matrix
        matrix[2][0] = c.x;
        matrix[2][1] = c.y;
        matrix[2][2] = c.z + 1.0F;
        matrix[2][3] = c.w; 
    }
    //---------------------------------------------------------------------
    void GLRenderSystem::_oneTimeContextInitialization()
    {
        // Set nicer lighting model -- d3d9 has this by default
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);        
        glEnable(GL_COLOR_SUM);
        glDisable(GL_DITHER);

        // Check for FSAA
        // Enable the extension if it was enabled by the GLSupport
        if (mGLSupport->checkExtension("GL_ARB_multisample"))
        {
            int fsaa_active = false;
            glGetIntegerv(GL_SAMPLE_BUFFERS_ARB,(GLint*)&fsaa_active);
            if(fsaa_active)
            {
                glEnable(GL_MULTISAMPLE_ARB);
                LogManager::getSingleton().logMessage("Using FSAA from GL_ARB_multisample extension.");
            }            
        }
    }
    //---------------------------------------------------------------------
    void GLRenderSystem::_switchContext(GLContext *context)
    {
        // Unbind GPU programs and rebind to new context later, because
        // scene manager treat render system as ONE 'context' ONLY, and it
        // cached the GPU programs using state.
        if (mCurrentVertexProgram)
            mCurrentVertexProgram->unbindProgram();
        if (mCurrentFragmentProgram)
            mCurrentFragmentProgram->unbindProgram();

        // It's ready to switching
        mCurrentContext->endCurrent();
        mCurrentContext = context;
        mCurrentContext->setCurrent();
        
        // Check if the context has already done one-time initialisation
        if(!mCurrentContext->getInitialized()) 
        {
            _oneTimeContextInitialization();
            mCurrentContext->setInitialized();
        }

        // Rebind GPU programs to new context
        if (mCurrentVertexProgram)
            mCurrentVertexProgram->bindProgram();
        if (mCurrentFragmentProgram)
            mCurrentFragmentProgram->bindProgram();

        // Must reset depth/colour write mask to according with user desired, otherwise,
        // clearFrameBuffer would be wrong because the value we are recorded may be
        // difference with the really state stored in GL context.
        glDepthMask(mDepthWrite);
        glColorMask(mColourWrite[0], mColourWrite[1], mColourWrite[2], mColourWrite[3]);

    }
    //---------------------------------------------------------------------
    void GLRenderSystem::_setRenderTarget(RenderTarget *target)
    {
        mActiveRenderTarget = target;
        // Switch context if different from current one
        ContextMap::iterator i = mContextMap.find(target);
        if(i != mContextMap.end() && mCurrentContext != i->second) {
            _switchContext(i->second);
        }
    }
    //---------------------------------------------------------------------
    void GLRenderSystem::_registerContext(RenderTarget *target, GLContext *context)
    {
        mContextMap[target] = context;
    }
    //---------------------------------------------------------------------
    void GLRenderSystem::_unregisterContext(RenderTarget *target)
    {
        ContextMap::iterator i = mContextMap.find(target);
        if(i != mContextMap.end() && mCurrentContext == i->second) {
            // Change the context to something else so that a valid context
            // remains active. When this is the main context being unregistered,
            // we set the main context to 0.
            if(mCurrentContext != mMainContext) {
                _switchContext(mMainContext);
            } else {
                mMainContext = 0;
            }
        }
        mContextMap.erase(target);
    }
    //---------------------------------------------------------------------
    GLContext *GLRenderSystem::_getMainContext() {
        return mMainContext;
    }
    //---------------------------------------------------------------------
    Real GLRenderSystem::getMinimumDepthInputValue(void)
    {
        // Range [-1.0f, 1.0f]
        return -1.0f;
    }
    //---------------------------------------------------------------------
    Real GLRenderSystem::getMaximumDepthInputValue(void)
    {
        // Range [-1.0f, 1.0f]
        return 1.0f;
    }

}
