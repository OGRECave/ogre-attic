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
http://www.gnu.org/copyleft/lesser.txt.s
-----------------------------------------------------------------------------
*/


#include "OgreGLRenderSystem.h"
#include "OgreRenderSystem.h"
#include "OgreLogManager.h"
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


#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

// Convenience macro from ARB_vertex_buffer_object spec
#define VBO_BUFFER_OFFSET(i) ((char *)NULL + (i))

// Pointers to extension functions
GL_ActiveTextureARB_Func glActiveTextureARB_ptr;
GL_ClientActiveTextureARB_Func glClientActiveTextureARB_ptr;
GL_SecondaryColorPointerEXT_Func glSecondaryColorPointerEXT_ptr;
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
GL_CombinerStageParameterfvNV_Func glCombinerStageParameterfvNV_ptr;
GL_CombinerParameterfvNV_Func glCombinerParameterfvNV_ptr;
GL_CombinerParameteriNV_Func glCombinerParameteriNV_ptr;
GL_GetProgramivARB_Func glGetProgramivARB_ptr;
GL_LoadProgramNV_Func glLoadProgramNV_ptr;
GL_CombinerInputNV_Func glCombinerInputNV_ptr;
GL_CombinerOutputNV_Func glCombinerOutputNV_ptr;
GL_FinalCombinerInputNV_Func glFinalCombinerInputNV_ptr;
GL_TrackMatrixNV_Func glTrackMatrixNV_ptr;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2DARB_ptr;
GL_ActiveStencilFaceEXT_Func glActiveStencilFaceEXT_ptr;

namespace Ogre {

    // Callback function used when registering GLGpuPrograms
    GpuProgram* createGLArbGpuProgram(const String& name, GpuProgramType gptype, const String& syntaxCode)
    {
        return new GLArbGpuProgram(name, gptype, syntaxCode);
    }

    GpuProgram* createGLGpuNvparseProgram(const String& name, GpuProgramType gptype, const String& syntaxCode)
    {
        return new GLGpuNvparseProgram(name, gptype, syntaxCode);
    }

	GpuProgram* createGL_ATI_FS_GpuProgram(const String& name, GpuProgramType gptype, const String& syntaxCode)
	{

		return new ATI_FS_GLGpuProgram(name, gptype, syntaxCode);
	}

    GLRenderSystem::GLRenderSystem()
      : mDepthWrite(true), mHardwareBufferManager(0), mGpuProgramManager(0)
    {
        unsigned int i;

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
            mTextureCoordIndex[i] = 0;
        }

        for (i = 0; i < OGRE_MAX_TEXTURE_LAYERS; i++)
        {
            mTextureTypes[i] = 0;
        }

        mActiveRenderTarget = NULL;

        glActiveTextureARB_ptr = 0;
        glClientActiveTextureARB_ptr = 0;
        glSecondaryColorPointerEXT_ptr = 0;
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

        mCurrentLights = 0;
        mMinFilter = FO_LINEAR;
        mMipFilter = FO_POINT;
        mCurrentVertexProgram = 0;
        mCurrentFragmentProgram = 0;

        OgreUnguard();
    }

    GLRenderSystem::~GLRenderSystem()
    {
        // Destroy render windows
        RenderTargetMap::iterator i;
        for (i = mRenderTargets.begin(); i != mRenderTargets.end(); ++i)
        {
            delete i->second;
        }
        mRenderTargets.clear();

        if (mTextureManager)
            delete mTextureManager;

        if (mHardwareBufferManager)
            delete mHardwareBufferManager;
		if (mGpuProgramManager)
        	delete mGpuProgramManager;
        delete mCapabilities;
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

    RenderWindow* GLRenderSystem::initialise(bool autoCreateWindow)
    {

        mGLSupport->start();
		RenderWindow* autoWindow = mGLSupport->createWindow(autoCreateWindow, this);

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


        // Get extension function pointers
        glActiveTextureARB_ptr = 
            (GL_ActiveTextureARB_Func)mGLSupport->getProcAddress("glActiveTextureARB");
        glClientActiveTextureARB_ptr = 
            (GL_ClientActiveTextureARB_Func)mGLSupport->getProcAddress("glClientActiveTextureARB");
        glSecondaryColorPointerEXT_ptr = 
            (GL_SecondaryColorPointerEXT_Func)mGLSupport->getProcAddress("glSecondaryColorPointerEXT");
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
        glCompressedTexImage2DARB_ptr =
            (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)mGLSupport->getProcAddress("glCompressedTexImage2DARB");
        InitATIFragmentShaderExtensions(*mGLSupport);
        glActiveStencilFaceEXT_ptr = 
            (GL_ActiveStencilFaceEXT_Func)mGLSupport->getProcAddress("glActiveStencilFaceEXT");

        mCapabilities->log(LogManager::getSingleton().getDefaultLog());
    }

    void GLRenderSystem::reinitialise(void)
    {
        this->shutdown();
        this->initialise(true);
    }

    void GLRenderSystem::shutdown(void)
    {
        RenderSystem::shutdown();

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


    RenderWindow* GLRenderSystem::createRenderWindow(
            const String & name, unsigned int width, unsigned int height, unsigned int colourDepth,
            bool fullScreen, int left, int top, bool depthBuffer, 
            RenderWindow* parentWindowHandle)
    {
        if (mRenderTargets.find(name) != mRenderTargets.end())
        {
            Except(
                Exception::ERR_INVALIDPARAMS, 
                "Window with name '" + name + "' already exists",
                "GLRenderSystem::createRenderWindow" );
        }

        // Create the window
        RenderWindow* win = mGLSupport->newWindow(name, width, height, 
            colourDepth, fullScreen, left, top, depthBuffer, parentWindowHandle,
            mVSync);

        attachRenderTarget( *win );

        initGL();

        if (parentWindowHandle == NULL)
        {
            mTextureManager = new GLTextureManager(*mGLSupport);
        }

        // XXX Do more?

        return win;
    }

    RenderTexture * GLRenderSystem::createRenderTexture( const String & name, unsigned int width, unsigned int height )
    {
        RenderTexture* rt = new GLRenderTexture(name, width, height);
        attachRenderTarget(*rt);
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
                glLightf( gl_index, GL_SPOT_CUTOFF, lt->getSpotlightOuterAngle() );
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

            // Disable ambient light for movables
            glLighti(gl_index, GL_AMBIENT, 0);

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
        for (size_t i=0; i < 4; i++)
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
        makeGLMatrix(mat, mViewMatrix);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(mat);

        /* Set the lights here everytime the view
         * matrix changes.
         */
        setLights();
         
        makeGLMatrix(mat, mWorldMatrix);
        glMultMatrixf(mat);
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
        const ColourValue &emissive, Real shininess)
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
    }

    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setTexture(size_t stage, bool enabled, const String &texname)
    {
        GLTexture* tex = static_cast<GLTexture*>(TextureManager::getSingleton().getByName(texname));

        GLenum lastTextureType = mTextureTypes[stage];

		glActiveTextureARB_ptr( GL_TEXTURE0 + stage );
		if (enabled && tex)
        {
            mTextureTypes[stage] = tex->getGLTextureType();
            if(lastTextureType != mTextureTypes[stage] && lastTextureType != 0)
            {
                glDisable( lastTextureType );
            }

            glEnable( mTextureTypes[stage] );
            glBindTexture( mTextureTypes[stage], tex->getGLID() );
        }
        else
        {
			glDisable( mTextureTypes[stage] );
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }
        glActiveTextureARB_ptr( GL_TEXTURE0 );
    }

    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setTextureCoordSet(size_t stage, size_t index)
    {
        mTextureCoordIndex[stage] = index;
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setTextureCoordCalculation(size_t stage, TexCoordCalcMethod m)
    {
        GLfloat M[16];
        // Default to no extra auto texture matrix
        mUseAutoTextureMatrix = false;

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
        GLfloat mat[16];
        makeGLMatrix(mat, xform);

        mat[12] = mat[8];
        mat[13] = mat[9];
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

        if (mUseAutoTextureMatrix)
        {
            // Load auto matrix in
            glLoadMatrixf(mAutoTextureMatrix);
            // Concat new matrix
            glMultMatrixf(mat);

        }
        else
        {
            // Just load this matrix
            glLoadMatrixf(mat);
        }

        glMatrixMode(GL_MODELVIEW);
        glActiveTextureARB_ptr(GL_TEXTURE0);
    }
    //-----------------------------------------------------------------------------
    GLint GLRenderSystem::getBlendMode(SceneBlendFactor ogreBlend)
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
        
        glEnable(GL_BLEND);
        glBlendFunc(sourceBlend, destBlend);
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setAlphaRejectSettings(CompareFunction func, unsigned char value)
    {
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(convertCompareFunction(func), value / 255.0f);
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setViewport(Viewport *vp)
    {
        // Check if viewport is different
        if (vp != mActiveViewport || vp->_isUpdated())
        {
              mActiveViewport = vp;
              mActiveRenderTarget = vp->getTarget();
              // XXX Rendering target stuff?
              GLsizei x, y, w, h;
  
              RenderTarget* target;
              target = vp->getTarget();
  
              // Calculate the "lower-left" corner of the viewport
              w = vp->getActualWidth();
              h = vp->getActualHeight();
              x = vp->getActualLeft();
              y = target->getHeight() - vp->getActualTop() - h;
  
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
                setGLLightPositionDirection(lt, i);
            }
        }
	}

    //-----------------------------------------------------------------------------
    void GLRenderSystem::_beginFrame(void)
    {
        OgreGuard( "GLRenderSystem::_beginFrame" );
        
        if (!mActiveViewport)
            Except(999, "Cannot begin frame - no viewport selected.",
                "GLRenderSystem::_beginFrame");

        // Clear the viewport if required
        if (mActiveViewport->getClearEveryFrame())
        {
            // Activate the viewport clipping
            glEnable(GL_SCISSOR_TEST);

            clearFrameBuffer(FBT_COLOUR | FBT_DEPTH, 
                mActiveViewport->getBackgroundColour());
        }        

        // Update light positions / directions because GL modifies them
        setLights();
        OgreUnguard();
    }
   
    //-----------------------------------------------------------------------------
    void GLRenderSystem::_endFrame(void)
    {
        // XXX Do something?
    }

    //-----------------------------------------------------------------------------
    void GLRenderSystem::_setCullingMode(CullingMode mode)
    {
        GLint cullMode;

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
                cullMode = GL_CW;
            }
            else
            {
                cullMode = GL_CCW;
            }
            break;
        case CULL_ANTICLOCKWISE:
            if (mActiveRenderTarget && 
                ((mActiveRenderTarget->requiresTextureFlipping() && !mInvertVertexWinding) ||
                (!mActiveRenderTarget->requiresTextureFlipping() && mInvertVertexWinding)))
            {
                cullMode = GL_CCW;
            }
            else
            {
                cullMode = GL_CW;
            }
            break;
        }

        glEnable( GL_CULL_FACE );
        glFrontFace( cullMode );
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
            glPolygonOffset(1.0f, bias);
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
    String GLRenderSystem::getErrorDescription(long errCode)
    {
        // XXX FIXME
        return String("Uknown Error");
    }
    //-----------------------------------------------------------------------------
    void GLRenderSystem::setLightingEnabled(bool enabled)
    {
        if (enabled)
            glEnable(GL_LIGHTING);
        else
            glDisable(GL_LIGHTING);
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

    void GLRenderSystem::convertColourValue(const ColourValue& colour, unsigned long* pDest)
    {
    #if OGRE_ENDIAN == ENDIAN_BIG
        *pDest = colour.getAsLongRGBA();
    #else
      // GL accesses by byte, so use ABGR so little-endian format will make it RGBA in byte mode
        *pDest = colour.getAsLongABGR();
    #endif
    }
    
    void GLRenderSystem::_makeProjectionMatrix(Real fovy, Real aspect, Real nearPlane, 
        Real farPlane, Matrix4& dest, bool forGpuProgram)
    {
        Real thetaY = Math::AngleUnitsToRadians(fovy / 2.0f);
        Real tanThetaY = Math::Tan(thetaY);
        //Real thetaX = thetaY * aspect;
        //Real tanThetaX = Math::Tan(thetaX);

        // Calc matrix elements
        Real w = (1.0f / tanThetaY) / aspect;
        Real h = 1.0f / tanThetaY;
        Real q = -(farPlane + nearPlane) / (farPlane - nearPlane);
        //Real qn= q * mNearDist;
        Real qn = -2 * (farPlane * nearPlane) / (farPlane - nearPlane);

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

            // Deactivate the viewport clipping.
            glDisable(GL_SCISSOR_TEST);
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
    void GLRenderSystem::setStencilBufferParams(CompareFunction func, ulong refValue, 
        ulong mask, StencilOperation stencilFailOp, 
        StencilOperation depthFailOp, StencilOperation passOp, 
        bool twoSidedOperation)
    {
        if (twoSidedOperation)
        {
            if (!mCapabilities->hasCapability(RSC_TWO_SIDED_STENCIL))
                Except(Exception::ERR_INVALIDPARAMS, "2-sided stencils are not supported",
                    "GLRenderSystem::setStencilBufferParams");
            glActiveStencilFaceEXT_ptr(GL_FRONT);
        }
        
        glStencilMask(mask);
        glStencilFunc(convertCompareFunction(func), refValue, mask);
        glStencilOp(convertStencilOp(stencilFailOp), convertStencilOp(depthFailOp), 
            convertStencilOp(passOp));

        if (twoSidedOperation)
        {
            // set everything again, inverted
            glActiveStencilFaceEXT_ptr(GL_BACK);
            glStencilMask(mask);
            glStencilFunc(convertCompareFunction(func), refValue, mask);
            glStencilOp(
                convertStencilOp(stencilFailOp, true), 
                convertStencilOp(depthFailOp, true), 
                convertStencilOp(passOp, true));
            // reset
            glActiveStencilFaceEXT_ptr(GL_FRONT);
        }
    }
    //---------------------------------------------------------------------
    GLint GLRenderSystem::convertCompareFunction(CompareFunction func)
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
    GLint GLRenderSystem::convertStencilOp(StencilOperation op, bool invert)
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
    GLuint GLRenderSystem::getCombinedMinMipFilter(void)
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
                break;
            case FO_POINT:
                // linear min, point mip
                return GL_LINEAR_MIPMAP_NEAREST;
                break;
            case FO_NONE:
                // linear min, no mip
                return GL_LINEAR;
                break;
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
                break;
            case FO_POINT:
                // nearest min, point mip
                return GL_NEAREST_MIPMAP_NEAREST;
                break;
            case FO_NONE:
                // nearest min, no mip
                return GL_NEAREST;
                break;
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

		    cv2[0] = bm.colourArg2.r;
		    cv2[1] = bm.colourArg2.g;
		    cv2[2] = bm.colourArg2.b;
		    cv2[3] = bm.colourArg2.a;
        }

		if (bm.blendType == LBT_ALPHA)
        {
		    cv1[0] = 0;
		    cv1[1] = 0;
		    cv1[2] = 0;
		    cv1[3] = bm.alphaArg1;

		    cv2[0] = 0;
		    cv2[1] = 0;
		    cv2[2] = 0;
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
    void GLRenderSystem::setGLLightPositionDirection(Light* lt, size_t lightindex)
    {
        // Set position / direction
        Vector3 vec;
        GLfloat f4vals[4];
        if (lt->getType() == Light::LT_POINT)
        {
            vec = lt->getDerivedPosition();
            f4vals[0] = vec.x;
            f4vals[1] = vec.y;
            f4vals[2] = vec.z;
            f4vals[3] = 1.0;
            glLightfv(GL_LIGHT0 + lightindex, GL_POSITION, f4vals);
        }
        if (lt->getType() == Light::LT_DIRECTIONAL)
        {
            vec = lt->getDerivedDirection();
            f4vals[0] = -vec.x; // GL light directions are in eye coords
            f4vals[1] = -vec.y;
            f4vals[2] = -vec.z; // GL light directions are in eye coords
            f4vals[3] = 0.0; // important!
            // In GL you set direction through position, but the
            //  w value of the vector being 0 indicates which it is
            glLightfv(GL_LIGHT0 + lightindex, GL_POSITION, f4vals);
        }
        if (lt->getType() == Light::LT_SPOTLIGHT)
        {
            vec = lt->getDerivedPosition();
            f4vals[0] = vec.x;
            f4vals[1] = vec.y;
            f4vals[2] = vec.z;
            f4vals[3] = 1.0;
            glLightfv(GL_LIGHT0 + lightindex, GL_POSITION, f4vals);

            vec = lt->getDerivedDirection();
            f4vals[0] = vec.x; 
            f4vals[1] = vec.y;
            f4vals[2] = vec.z; 
            f4vals[3] = 0.0; 
            glLightfv(GL_LIGHT0 + lightindex, GL_SPOT_DIRECTION, f4vals);
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

                for (i = 0; i < mCapabilities->getNumTextureUnits(); i++)
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
                    op.vertexData->vertexStart * op.indexData->indexBuffer->getIndexSize());
            }
            else
            {
                pBufferData = static_cast<GLDefaultHardwareIndexBuffer*>(
                    op.indexData->indexBuffer.get())->getDataPtr(
                        op.vertexData->vertexStart * op.indexData->indexBuffer->getIndexSize());
            }

            GLenum indexType = (op.indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_16BIT) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

            glDrawRangeElements(primType, op.indexData->indexStart, 
                op.indexData->indexStart + op.indexData->indexCount - 1,
                op.indexData->indexCount, indexType, pBufferData);

        }
        else
        {
            glDrawArrays(primType, op.vertexData->vertexStart,
                op.vertexData->vertexCount);
        }

        glDisableClientState( GL_VERTEX_ARRAY );
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        glDisableClientState( GL_NORMAL_ARRAY );
        glDisableClientState( GL_COLOR_ARRAY );
        glDisableClientState( GL_SECONDARY_COLOR_ARRAY );
        glColor4f(1,1,1,1);

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
        GLuint glProgType = (gptype == GPT_VERTEX_PROGRAM) ? 
            GL_VERTEX_PROGRAM_ARB : GL_FRAGMENT_PROGRAM_ARB;

        if (gptype == GPT_VERTEX_PROGRAM)
        {
            mCurrentVertexProgram->unbindProgram();
            mCurrentVertexProgram = 0;
        }
        else
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
    void GLRenderSystem::setScissorTest(bool enabled, size_t left, 
        size_t top, size_t right, size_t bottom)
    {
        if (enabled)
        {
            glEnable(GL_SCISSOR_TEST);
            // NB GL uses width / height rather than right / bottom
            glScissor(left, top, right-left, bottom-top);
        }
        else
        {
            glDisable(GL_SCISSOR_TEST);
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


}
