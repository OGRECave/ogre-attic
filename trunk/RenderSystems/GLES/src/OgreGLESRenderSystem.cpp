/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#include "OgreGLESRenderSystem.h"
#include "OgreGLESTextureManager.h"
#include "OgreGLESDefaultHardwareBufferManager.h"
#include "OgreGLESHardwarePixelBuffer.h"
#include "OgreGLESHardwareBufferManager.h"
#include "OgreGLESHardwareIndexBuffer.h"
#include "OgreGLESHardwareVertexBuffer.h"
#include "OgreGLESGpuProgramManager.h"
#include "OgreGLESUtil.h"
#include "OgreGLESRenderTexture.h"
#include "OgreGLESPBRenderTexture.h"
#include "OgreGLESTexture.h"
#include "OgreGLESContext.h"

#include "OgreEGLWindow.h"

#include "OgreCamera.h"
#include "OgreTexture.h"

// Convenience macro from ARB_vertex_buffer_object spec
#define VBO_BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace Ogre {
    GLESRenderSystem::GLESRenderSystem()
        : mDepthWrite(true),
          mGpuProgramManager(0),
          mStencilMask(0xFFFFFFFF), mHardwareBufferManager(0),
          mRTTManager(0)
    {
        GL_CHECK_ERROR;
        size_t i;

        mGLSupport = getGLSupport();

        for (i = 0; i < MAX_LIGHTS; i++)
            mLights[i] = 0;

        mWorldMatrix = Matrix4::IDENTITY;
        mViewMatrix = Matrix4::IDENTITY;

        mGLSupport->addConfig();

        mColourWrite[0] = mColourWrite[1] = mColourWrite[2] = mColourWrite[3] = true;

        for (i = 0; i < OGRE_MAX_TEXTURE_LAYERS; i++)
        {
            // DummyGL_TEXTURE_2D
            mTextureCoordIndex[i] = 99;
        }

        mTextureCount = 0;
        mActiveRenderTarget = 0;
        mCurrentContext = 0;
        mMainContext = 0;
        mGLInitialized = false;
        mCurrentLights = 0;

        mMinFilter = FO_LINEAR;
        mMipFilter = FO_POINT;
    }

    GLESRenderSystem::~GLESRenderSystem()
    {
        shutdown();

        RenderTargetMap::iterator i;
        for (i = mRenderTargets.begin(); i != mRenderTargets.end(); ++i)
        {
            delete i->second;
        }

        mRenderTargets.clear();
        delete mGLSupport;
    }

    const String& GLESRenderSystem::getName(void) const
    {
        static String strName("OpenGLES Rendering Subsystem");
        return strName;
    }

    ConfigOptionMap& GLESRenderSystem::getConfigOptions(void)
    {
        return mGLSupport->getConfigOptions();
    }

    void GLESRenderSystem::setConfigOption(const String &name, const String &value)
    {
        mGLSupport->setConfigOption(name, value);
    }

    String GLESRenderSystem::validateConfigOptions(void)
    {
        return mGLSupport->validateConfig();
    }

    RenderWindow* GLESRenderSystem::_initialise(bool autoCreateWindow,
                                                const String& windowTitle)
    {
        RenderWindow *autoWindow = mGLSupport->createWindow(autoCreateWindow,
                                                            this, windowTitle);
        RenderSystem::_initialise(autoCreateWindow, windowTitle);
        return autoWindow;
    }

    RenderSystemCapabilities* GLESRenderSystem::createRenderSystemCapabilities() const
    {
        RenderSystemCapabilities* rsc = new RenderSystemCapabilities();

        rsc->setCategoryRelevant(CAPS_CATEGORY_GL, true);
        rsc->setDriverVersion(mDriverVersion);

        const char* deviceName = (const char*)glGetString(GL_RENDERER);

        if (deviceName)
        {
            rsc->setDeviceName(deviceName);
        }

        rsc->setRenderSystemName(getName());
        rsc->setVendor(GPU_UNKNOWN);

        // Point size
        float ps;
        glGetFloatv(GL_POINT_SIZE, &ps);
        GL_CHECK_ERROR;
        rsc->setMaxPointSize(ps);

        // Supports fixed-function
        if (mGLSupport->checkExtension("GL_OES_fixed_point"))
        {
            rsc->setCapability(RSC_FIXED_FUNCTION);
        }


        // Multitexturing support and set number of texture units
        GLint units;
        glGetIntegerv(GL_MAX_TEXTURE_UNITS, &units);
        rsc->setNumTextureUnits(units);

        // Check for hardware stencil support and set bit depth
        GLint stencil;
        glGetIntegerv(GL_STENCIL_BITS, &stencil);
        GL_CHECK_ERROR;

        if (stencil)
        {
            rsc->setCapability(RSC_HWSTENCIL);
            rsc->setStencilBufferBitDepth(stencil);
        }

        rsc->setCapability(RSC_SCISSOR_TEST);
        rsc->setCapability(RSC_USER_CLIP_PLANES);


        if (mGLSupport->checkExtension("GL_ARB_vertex_buffer_object"))
            rsc->setCapability(RSC_VBO);

        // GL always shares vertex and fragment texture units (for now?)
        rsc->setVertexTextureUnitsShared(true);

        // Inifinite far plane always supported
        rsc->setCapability(RSC_INFINITE_FAR_PLANE);

        // TODO check here
        // TODO implement texture compression

        // Hardware support mipmapping
        // rsc->setCapability(RSC_AUTOMIPMAP);

        // Blending support
        // rsc->setCapability(RSC_BLENDING);

        // Anisotropy support
        // rsc->setCapability(RSC_ANISOTROPY);

        // DOT3 support: Cube map
        // rsc->setCapability(RSC_CUBEMAPPING);

        // Point sprites
        // rsc->setCapability(RSC_POINT_EXTENDED_PARAMETERS)

        // Vertex/Fragment Program: need implment
        // rsc->setCapability(RSC_VERTEX_PROGRAM);
        // rsc->setVertexProgramConstantBoolCount(0);
        // rsc->setVertexProgramConstantIntCount(0);
        // rsc->addShaderProfile("arbvp1");

        // TODO UBYTE4 always supported
        rsc->setCapability(RSC_VERTEX_FORMAT_UBYTE4);

        // hardware occlusion support
        // rsc->setCapability(RSC_HWOCCLUSION);

        // Check for Float textures
        rsc->setCapability(RSC_TEXTURE_FLOAT);

        // Mipmap LOD biasing?
        rsc->setCapability(RSC_MIPMAP_LOD_BIAS);

        return rsc;
    }

    void GLESRenderSystem::initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary)
    {
        if(caps->getRenderSystemName() != getName())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Trying to initialize GLESRenderSystem from RenderSystemCapabilities that do not support OpenGL",
                        "GLESRenderSystem::initialiseFromRenderSystemCapabilities");
        }

        mGpuProgramManager = new GLESGpuProgramManager();

        // set texture the number of texture units
        mFixedFunctionTextureUnits = caps->getNumTextureUnits();

        if(caps->hasCapability(RSC_VBO))
        {
            mHardwareBufferManager = new GLESHardwareBufferManager;
        }
        else
        {
            mHardwareBufferManager = new GLESDefaultHardwareBufferManager;
        }

        GL_CHECK_ERROR;
        mRTTManager = new GLESPBRTTManager(mGLSupport, primary);
        GL_CHECK_ERROR;

        mTextureManager = new GLESTextureManager(*mGLSupport);
        GL_CHECK_ERROR;
        mGLInitialized = true;
    }

    void GLESRenderSystem::reinitialise(void)
    {
        this->shutdown();
        this->_initialise(true);
    }

    void GLESRenderSystem::shutdown(void)
    {
        RenderSystem::shutdown();

        delete mGpuProgramManager;
        mGpuProgramManager = 0;

        delete mHardwareBufferManager;
        mHardwareBufferManager = 0;

        delete mRTTManager;
        mRTTManager = 0;

        mGLSupport->stop();

        delete mTextureManager;
        mTextureManager = 0;

        mGLInitialized = 0;
    }

    void GLESRenderSystem::setAmbientLight(float r, float g, float b)
    {
        GLfloat lmodel_ambient[] = { r, g, b, 1.0 };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::setShadingType(ShadeOptions so)
    {
        switch (so)
        {
            case SO_FLAT:
                glShadeModel(GL_FLAT);
                GL_CHECK_ERROR;
                break;
            default:
                glShadeModel(GL_SMOOTH);
                GL_CHECK_ERROR;
                break;
        }
    }

    void GLESRenderSystem::setLightingEnabled(bool enabled)
    {
        if (enabled)
        {
            glEnable(GL_LIGHTING);
            GL_CHECK_ERROR;
        }
        else
        {
            glDisable(GL_LIGHTING);
            GL_CHECK_ERROR;
        }
    }

    RenderWindow* GLESRenderSystem::_createRenderWindow(const String &name, unsigned int width, unsigned int height,
                                                        bool fullScreen, const NameValuePairList *miscParams)
    {
        if (mRenderTargets.find(name) != mRenderTargets.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Window with name '" + name + "' already exists",
                        "GLESRenderSystem::_createRenderWindow");
        }

        std::stringstream ss;
        ss << "GLESRenderSystem::_createRenderWindow \"" << name << "\", " <<
            width << "x" << height << " ";
        if (fullScreen)
        {
            ss << "fullscreen ";
        }
        else
        {
            ss << "windowed ";
        }

        if (miscParams)
        {
            ss << " miscParams: ";
            NameValuePairList::const_iterator it;
            for (it = miscParams->begin(); it != miscParams->end(); ++it)
            {
                ss << it->first << "=" << it->second << " ";
            }

            LogManager::getSingleton().logMessage(ss.str());
        }

        RenderWindow* win = mGLSupport->newWindow(name, width, height,
                                                  fullScreen, miscParams);
        attachRenderTarget((Ogre::RenderTarget&) *win);

        if (!mGLInitialized)
        {
            initialiseContext(win);

            StringVector tokens = StringUtil::split(mGLSupport->getGLVersion(), ".");
            if (!tokens.empty())
            {
                mDriverVersion.major = StringConverter::parseInt(tokens[0]);
                if (tokens.size() > 1)
                {
                    mDriverVersion.minor = StringConverter::parseInt(tokens[1]);
                }
                if (tokens.size() > 2)
                {
                    mDriverVersion.release = StringConverter::parseInt(tokens[2]);
                }
            }
            mDriverVersion.build = 0;
            mRealCapabilities = createRenderSystemCapabilities();

            if (!mUseCustomCapabilities)
            {
                mCurrentCapabilities = mRealCapabilities;
            }

            initialiseFromRenderSystemCapabilities(mCurrentCapabilities, (RenderTarget *) win);

            _oneTimeContextInitialization();
            if (mCurrentContext)
            {
                mCurrentContext->setInitialized();
            }
        }

        return win;
    }

    MultiRenderTarget* GLESRenderSystem::createMultiRenderTarget(const String & name)
    {
        MultiRenderTarget *retval = mRTTManager->createMultiRenderTarget(name);
        attachRenderTarget(*retval);
        return retval;
    }

    void GLESRenderSystem::destroyRenderWindow(RenderWindow* pWin)
    {
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

    String GLESRenderSystem::getErrorDescription(long errorNumber) const
    {
        // TODO find a way to get error string
        return StringUtil::BLANK;
    }

    VertexElementType GLESRenderSystem::getColourVertexElementType(void) const
    {
        return VET_COLOUR_ABGR;
    }

    void GLESRenderSystem::setNormaliseNormals(bool normalise)
    {
        if (normalise)
        {
            glEnable(GL_NORMALIZE);
            GL_CHECK_ERROR;
        }
        else
        {
            glDisable(GL_NORMALIZE);
            GL_CHECK_ERROR;
        }
    }

    void GLESRenderSystem::_useLights(const LightList& lights, unsigned short limit)
    {
        // Save previous modelview
        glMatrixMode(GL_MODELVIEW);
        GL_CHECK_ERROR;
        glPushMatrix();
        GL_CHECK_ERROR;

        // Just load view matrix (identity world)
        GLfloat mat[16];
        makeGLMatrix(mat, mViewMatrix);
        glLoadMatrixf(mat);
        GL_CHECK_ERROR;

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
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_setWorldMatrix(const Matrix4 &m)
    {
        GLfloat mat[16];
        mWorldMatrix = m;
        makeGLMatrix(mat, mViewMatrix * mWorldMatrix);
        glMatrixMode(GL_MODELVIEW);
        GL_CHECK_ERROR;
        glLoadMatrixf(mat);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_setViewMatrix(const Matrix4 &m)
    {
        mViewMatrix = m;

        GLfloat mat[16];
        makeGLMatrix(mat, mViewMatrix * mWorldMatrix);
        glMatrixMode(GL_MODELVIEW);
        GL_CHECK_ERROR;
        glLoadMatrixf(mat);
        GL_CHECK_ERROR;

        // also mark clip planes dirty
        if (!mClipPlanes.empty())
        {
            mClipPlanesDirty = true;
        }
    }

    void GLESRenderSystem::_setProjectionMatrix(const Matrix4 &m)
    {
        GLfloat mat[16];

        makeGLMatrix(mat, m);

        if (mActiveRenderTarget->requiresTextureFlipping())
        {
            mat[1] = -mat[1];
            mat[5] = -mat[5];
            mat[9] = -mat[9];
            mat[13] = -mat[13];
        }
        glMatrixMode(GL_PROJECTION);
        GL_CHECK_ERROR;
        glLoadMatrixf(mat);
        GL_CHECK_ERROR;
        glMatrixMode(GL_MODELVIEW);
        GL_CHECK_ERROR;

        if (!mClipPlanes.empty())
        {
            mClipPlanesDirty = true;
        }
    }

    void GLESRenderSystem::_setSurfaceParams(const ColourValue &ambient,
                                             const ColourValue &diffuse,
                                             const ColourValue &specular,
                                             const ColourValue &emissive,
                                             Real shininess,
                                             TrackVertexColourType tracking)
    {
        // Track vertex colour
        if(tracking != TVC_NONE)
        {
            GLenum gt = GL_DIFFUSE;
            // There are actually 15 different combinations for tracking, of which
            // GL only supports the most used 5. This means that we have to do some
            // magic to find the best match. NOTE:
            // GL_AMBIENT_AND_DIFFUSE != GL_AMBIENT | GL__DIFFUSE
            if (tracking & TVC_AMBIENT)
            {
                if (tracking & TVC_DIFFUSE)
                {
                    gt = GL_AMBIENT_AND_DIFFUSE;
                }
                else
                {
                    gt = GL_AMBIENT;
                }
            }
            else if (tracking & TVC_DIFFUSE)
            {
                gt = GL_DIFFUSE;
            }
            else if (tracking & TVC_SPECULAR)
            {
                gt = GL_SPECULAR;
            }
            else if (tracking & TVC_EMISSIVE)
            {
                gt = GL_EMISSION;
            }
            glEnable(gt);
            GL_CHECK_ERROR;
            glEnable(GL_COLOR_MATERIAL);
            GL_CHECK_ERROR;
        }
        else
        {
             glDisable(GL_COLOR_MATERIAL);
        }

        // XXX Cache previous values?
        // XXX Front or Front and Back?

        GLfloat f4val[4] = { diffuse.r, diffuse.g, diffuse.b, diffuse.a };
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, f4val);
        GL_CHECK_ERROR;
        f4val[0] = ambient.r;
        f4val[1] = ambient.g;
        f4val[2] = ambient.b;
        f4val[3] = ambient.a;
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, f4val);
        GL_CHECK_ERROR;
        f4val[0] = specular.r;
        f4val[1] = specular.g;
        f4val[2] = specular.b;
        f4val[3] = specular.a;
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, f4val);
        GL_CHECK_ERROR;
        f4val[0] = emissive.r;
        f4val[1] = emissive.g;
        f4val[2] = emissive.b;
        f4val[3] = emissive.a;
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, f4val);
        GL_CHECK_ERROR;
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_setPointParameters(Real size,
                                               bool attenuationEnabled,
                                               Real constant,
                                               Real linear,
                                               Real quadratic,
                                               Real minSize,
                                               Real maxSize)
    {
        if (attenuationEnabled &&
            mCurrentCapabilities->hasCapability(RSC_POINT_EXTENDED_PARAMETERS))
        {
            // Point size is still calculated in pixels even when attenuation is
            // enabled, which is pretty awkward, since you typically want a viewport
            // independent size if you're looking for attenuation.
            // So, scale the point size up by viewport size (this is equivalent to
            // what D3D does as standard)
            Real adjSize = size * mActiveViewport->getActualHeight();
            Real adjMinSize = minSize * mActiveViewport->getActualHeight();
            Real adjMaxSize;
            if (maxSize == 0.0f)
            {
                adjMaxSize = mCurrentCapabilities->getMaxPointSize(); // pixels
            }
            else
            {
                adjMaxSize = maxSize * mActiveViewport->getActualHeight();
            }
            glPointSize(adjSize);
            GL_CHECK_ERROR;

            // XXX: why do I need this for results to be consistent with D3D?
            // Equations are supposedly the same once you factor in vp height
            Real correction = 0.005;
            // scaling required
            float val[4] = { constant, linear * correction,
                             quadratic * correction, 1};
            glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, val);
            GL_CHECK_ERROR;
            glPointParameterf(GL_POINT_SIZE_MIN, adjMinSize);
            GL_CHECK_ERROR;
            glPointParameterf(GL_POINT_SIZE_MAX, adjMaxSize);
            GL_CHECK_ERROR;
        }
        else
        {
            // no scaling required
            // GL has no disabled flag for this so just set to constant
            glPointSize(size);
            GL_CHECK_ERROR;

            if (mCurrentCapabilities->hasCapability(RSC_POINT_EXTENDED_PARAMETERS))
            {
                float val[4] = { 1, 0, 0, 1 };
                glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, val);
                GL_CHECK_ERROR;
                glPointParameterf(GL_POINT_SIZE_MIN, minSize);
                GL_CHECK_ERROR;
                if (maxSize == 0.0f)
                {
                    maxSize = mCurrentCapabilities->getMaxPointSize();
                }
                glPointParameterf(GL_POINT_SIZE_MAX, maxSize);
                GL_CHECK_ERROR;
            }
        }
    }

    void GLESRenderSystem::_setPointSpritesEnabled(bool enabled)
    {
        GL_CHECK_ERROR;
        if (enabled)
        {
            glEnable(GL_POINT_SPRITE_OES);
            GL_CHECK_ERROR;
        }
        else
        {
            glDisable(GL_POINT_SPRITE_OES);
            GL_CHECK_ERROR;
        }
    }

    void GLESRenderSystem::_setTexture(size_t stage, bool enabled, const TexturePtr &texPtr)
    {
        GL_CHECK_ERROR;

        // TODO We need control texture types?????
        GLESTexturePtr tex = texPtr;

        glActiveTexture(GL_TEXTURE0 + stage);
        GL_CHECK_ERROR;

        if (enabled)
        {
            if (!tex.isNull())
            {
                // note used
                tex->touch();
            }

            glEnable(GL_TEXTURE_2D);
            GL_CHECK_ERROR;

            if (!tex.isNull())
            {
                glBindTexture(GL_TEXTURE_2D, tex->getGLID());
                GL_CHECK_ERROR;
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, static_cast<GLESTextureManager*>(mTextureManager)->getWarningTextureID());
                GL_CHECK_ERROR;
            }
        }
        else
        {
            // mTextureCount--;
            glEnable(GL_TEXTURE_2D);
            glDisable(GL_TEXTURE_2D);
            GL_CHECK_ERROR;

            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            GL_CHECK_ERROR;

            // bind zero texture
            glBindTexture (GL_TEXTURE_2D, 0);
            GL_CHECK_ERROR;
        }

        glActiveTexture(GL_TEXTURE0);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_setTextureCoordSet(size_t stage, size_t index)
    {
        mTextureCoordIndex[stage] = index;
    }

    void GLESRenderSystem::_setTextureCoordCalculation(size_t stage,
                                                       TexCoordCalcMethod m,
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

        GLfloat eyePlaneS[] = { 1.0, 0.0, 0.0, 0.0 };
        GLfloat eyePlaneT[] = { 0.0, 1.0, 0.0, 0.0 };
        GLfloat eyePlaneR[] = { 0.0, 0.0, 1.0, 0.0 };
        GLfloat eyePlaneQ[] = { 0.0, 0.0, 0.0, 1.0 };

        glActiveTexture(GL_TEXTURE0 + stage);
        GL_CHECK_ERROR;

        switch(m)
        {
            case TEXCALC_NONE:
                // glDisable( GL_TEXTURE_GEN_S );
                // glDisable( GL_TEXTURE_GEN_T );
                // glDisable( GL_TEXTURE_GEN_R );
                // glDisable( GL_TEXTURE_GEN_Q );
                break;

            case TEXCALC_ENVIRONMENT_MAP:
                mUseAutoTextureMatrix = true;
                memset(mAutoTextureMatrix, 0, sizeof(GLfloat)*16);
                mAutoTextureMatrix[0] = mAutoTextureMatrix[10] = mAutoTextureMatrix[15] = 1.0f;
                mAutoTextureMatrix[5] = -1.0f;
                break;

            case TEXCALC_ENVIRONMENT_MAP_PLANAR:
                // TODO not implemented
                break;

            case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
                // We need an extra texture matrix here
                // This sets the texture matrix to be the inverse of the view matrix
                mUseAutoTextureMatrix = true;
                makeGLMatrix(M, mViewMatrix);

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
                break;

            case TEXCALC_PROJECTIVE_TEXTURE:
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

        glActiveTexture(GL_TEXTURE0);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_setTextureBlendMode(size_t stage, const LayerBlendModeEx& bm)
    {
        if (stage >= mFixedFunctionTextureUnits)
        {
            // Can't do this
            return;
        }

        // Check to see if blending is supported
        if (!mCurrentCapabilities->hasCapability(RSC_BLENDING))
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
            case LBS_SPECULAR:
                src1op = GL_PRIMARY_COLOR;
                break;
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
            case LBS_SPECULAR:
                src2op = GL_PRIMARY_COLOR;
                break;
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
            case LBX_ADD_SMOOTH:
                cmd = GL_INTERPOLATE;
                break;
            case LBX_SUBTRACT:
                cmd = GL_SUBTRACT;
                break;
            case LBX_BLEND_DIFFUSE_COLOUR:
                cmd = GL_INTERPOLATE;
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
                cmd = mCurrentCapabilities->hasCapability(RSC_DOT3)
                    ? GL_DOT3_RGB : GL_MODULATE;
                break;
            default:
                cmd = 0;
        }

        glActiveTexture(GL_TEXTURE0 + stage);
        GL_CHECK_ERROR;
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
        GL_CHECK_ERROR;

        if (bm.blendType == LBT_COLOUR)
        {
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, cmd);
            GL_CHECK_ERROR;
            // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, src1op);
            // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, src2op);
            // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
        }
        else
        {
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, cmd);
            GL_CHECK_ERROR;
            // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, src1op);
            // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, src2op);
            // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_CONSTANT);
        }

        float blendValue[4] = {0, 0, 0, bm.factor};
        switch (bm.operation)
        {
            case LBX_BLEND_DIFFUSE_COLOUR:
                // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PRIMARY_COLOR);
                // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_PRIMARY_COLOR);
                break;
            case LBX_BLEND_DIFFUSE_ALPHA:
                // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PRIMARY_COLOR);
                // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_PRIMARY_COLOR);
                break;
            case LBX_BLEND_TEXTURE_ALPHA:
                // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_TEXTURE);
                // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_TEXTURE);
                break;
            case LBX_BLEND_CURRENT_ALPHA:
                // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PREVIOUS);
                // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_PREVIOUS);
                break;
            case LBX_BLEND_MANUAL:
                glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, blendValue);
                GL_CHECK_ERROR;
                break;
            default:
                break;
        };

        switch (bm.operation)
        {
            case LBX_MODULATE_X2:
                glTexEnvi(GL_TEXTURE_ENV, bm.blendType == LBT_COLOUR ?
                          GL_RGB_SCALE : GL_ALPHA_SCALE, 2);
                GL_CHECK_ERROR;
                break;
            case LBX_MODULATE_X4:
                glTexEnvi(GL_TEXTURE_ENV, bm.blendType == LBT_COLOUR ?
                          GL_RGB_SCALE : GL_ALPHA_SCALE, 4);
                GL_CHECK_ERROR;

                break;
            default:
                glTexEnvi(GL_TEXTURE_ENV, bm.blendType == LBT_COLOUR ?
                          GL_RGB_SCALE : GL_ALPHA_SCALE, 1);
                GL_CHECK_ERROR;
                break;
        }

        if (bm.blendType == LBT_COLOUR)
        {
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
            GL_CHECK_ERROR;
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
            GL_CHECK_ERROR;
            if (bm.operation == LBX_BLEND_DIFFUSE_COLOUR)
            {
                glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
                GL_CHECK_ERROR;
            }
            else
            {
                glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
                GL_CHECK_ERROR;
            }
        }

        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
        GL_CHECK_ERROR;
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
        GL_CHECK_ERROR;
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);
        GL_CHECK_ERROR;
        if (bm.source1 == LBS_MANUAL)
        {
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, cv1);
        }
        if (bm.source2 == LBS_MANUAL)
        {
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, cv2);
        }

        GL_CHECK_ERROR;
        glActiveTexture(GL_TEXTURE0);
        GL_CHECK_ERROR;
    }

    GLint GLESRenderSystem::getTextureAddressingMode(TextureUnitState::TextureAddressingMode tam) const
    {
        switch (tam)
        {
            case TextureUnitState::TAM_CLAMP:
            case TextureUnitState::TAM_BORDER:
                return GL_CLAMP_TO_EDGE;
            case TextureUnitState::TAM_MIRROR:
            case TextureUnitState::TAM_WRAP:
            default:
                return GL_REPEAT;
        }
    }

    void GLESRenderSystem::_setTextureAddressingMode(size_t stage, const TextureUnitState::UVWAddressingMode& uvw)
    {
        glActiveTexture(GL_TEXTURE0 + stage);
        GL_CHECK_ERROR;
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_S, getTextureAddressingMode(uvw.u));
        GL_CHECK_ERROR;
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_T, getTextureAddressingMode(uvw.v));
        GL_CHECK_ERROR;
        glActiveTexture(GL_TEXTURE0);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_setTextureBorderColour(size_t stage, const ColourValue& colour)
    {
        // Not supported
    }

    void GLESRenderSystem::_setTextureMipmapBias(size_t unit, float bias)
    {
        // Not supported
    }

    void GLESRenderSystem::_setTextureMatrix(size_t stage, const Matrix4& xform)
    {
        if (stage >= mFixedFunctionTextureUnits)
        {
            // Can't do this
            return;
        }

        GLfloat mat[16];
        makeGLMatrix(mat, xform);

        glActiveTexture(GL_TEXTURE0 + stage);
        GL_CHECK_ERROR;
        glMatrixMode(GL_TEXTURE);
        GL_CHECK_ERROR;

        // Load this matrix in
        glLoadMatrixf(mat);
        GL_CHECK_ERROR;

        if (mUseAutoTextureMatrix)
        {
            // Concat auto matrix
            glMultMatrixf(mAutoTextureMatrix);
        }

        glMatrixMode(GL_MODELVIEW);
        GL_CHECK_ERROR;
        glActiveTexture(GL_TEXTURE0);
        GL_CHECK_ERROR;
    }

    GLint GLESRenderSystem::getBlendMode(SceneBlendFactor ogreBlend) const
    {
        switch (ogreBlend)
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

    void GLESRenderSystem::_setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor)
    {
        GLint sourceBlend = getBlendMode(sourceFactor);
        GLint destBlend = getBlendMode(destFactor);
        if (sourceFactor == SBF_ONE && destFactor == SBF_ZERO)
        {
            glDisable(GL_BLEND);
            GL_CHECK_ERROR;
        }
        else
        {
            glEnable(GL_BLEND);
            GL_CHECK_ERROR;
            glBlendFunc(sourceBlend, destBlend);
            GL_CHECK_ERROR;
        }
    }

    void GLESRenderSystem::_setSeparateSceneBlending(SceneBlendFactor sourceFactor,
                                                   SceneBlendFactor destFactor,
                                                   SceneBlendFactor sourceFactorAlpha,
                                                   SceneBlendFactor destFactorAlpha)
    {
        // Not supported
    }

    void GLESRenderSystem::_setAlphaRejectSettings(CompareFunction func, unsigned char value)
    {
        if (func == CMPF_ALWAYS_PASS)
        {
            glDisable(GL_ALPHA_TEST);
            GL_CHECK_ERROR;
        }
        else
        {
            glEnable(GL_ALPHA_TEST);
            GL_CHECK_ERROR;
            glAlphaFunc(convertCompareFunction(func), value / 255.0f);
            GL_CHECK_ERROR;
        }
    }

    void GLESRenderSystem::_setViewport(Viewport *vp)
    {
        if (vp != mActiveViewport || vp->_isUpdated())
        {
            RenderTarget* target;

            target = vp->getTarget();
            _setRenderTarget(target);
            mActiveViewport = vp;

            GLsizei x, y, w, h;

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
            GL_CHECK_ERROR;

            glScissor(x, y, w, h);
            GL_CHECK_ERROR;
            vp->_clearUpdatedFlag();
        }
    }

    void GLESRenderSystem::_beginFrame(void)
    {
        if (!mActiveViewport)
        {
            OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                        "Cannot begin frame - no viewport selected.",
                        "GLESRenderSystem::_beginFrame");
        }

        glEnable(GL_SCISSOR_TEST);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_endFrame(void)
    {
        // Deactivate the viewport clipping.
        glDisable(GL_SCISSOR_TEST);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_setCullingMode(CullingMode mode)
    {
        mCullingMode = mode;
        // NB: Because two-sided stencil API dependence of the front face, we must
        // use the same 'winding' for the front face everywhere. As the OGRE default
        // culling mode is clockwise, we also treat anticlockwise winding as front
        // face for consistently. On the assumption that, we can't change the front
        // face by glFrontFace anywhere.

        GLenum cullMode;

        switch( mode )
        {
            case CULL_NONE:
                glDisable(GL_CULL_FACE);
                GL_CHECK_ERROR;
                return;

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

            case CULL_CLOCKWISE:
            default:
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
        }

        glEnable(GL_CULL_FACE);
        GL_CHECK_ERROR;
        glCullFace(cullMode);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_setDepthBufferParams(bool depthTest, bool depthWrite, CompareFunction depthFunction)
    {
        _setDepthBufferCheckEnabled(depthTest);
        _setDepthBufferWriteEnabled(depthWrite);
        _setDepthBufferFunction(depthFunction);
    }

    void GLESRenderSystem::_setDepthBufferCheckEnabled(bool enabled)
    {
        if (enabled)
        {
            glClearDepthf(1.0f);
            GL_CHECK_ERROR;
            glEnable(GL_DEPTH_TEST);
            GL_CHECK_ERROR;
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
            GL_CHECK_ERROR;
        }
    }

    void GLESRenderSystem::_setDepthBufferWriteEnabled(bool enabled)
    {
        GLboolean flag = enabled ? GL_TRUE : GL_FALSE;
        glDepthMask(flag);
        GL_CHECK_ERROR;

        // Store for reference in _beginFrame
        mDepthWrite = enabled;
    }

    void GLESRenderSystem::_setDepthBufferFunction(CompareFunction func)
    {
        glDepthFunc(convertCompareFunction(func));
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_setDepthBias(float constantBias, float slopeScaleBias)
    {
        if (constantBias != 0 || slopeScaleBias != 0)
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            GL_CHECK_ERROR;
            // glEnable(GL_POLYGON_OFFSET_POINT);
            // glEnable(GL_POLYGON_OFFSET_LINE);
            glPolygonOffset(-slopeScaleBias, -constantBias);
            GL_CHECK_ERROR;
        }
        else
        {
            glDisable(GL_POLYGON_OFFSET_FILL);
            GL_CHECK_ERROR;
            // glDisable(GL_POLYGON_OFFSET_POINT);
            // glDisable(GL_POLYGON_OFFSET_LINE);
        }
    }

    void GLESRenderSystem::_setColourBufferWriteEnabled(bool red, bool green, bool blue, bool alpha)
    {
        glColorMask(red, green, blue, alpha);
        GL_CHECK_ERROR;

        // record this
        mColourWrite[0] = red;
        mColourWrite[1] = blue;
        mColourWrite[2] = green;
        mColourWrite[3] = alpha;
    }

    void GLESRenderSystem::_setFog(FogMode mode, const ColourValue& colour, Real density, Real start, Real end)
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
            default:
                // Give up on it
                glDisable(GL_FOG);
                GL_CHECK_ERROR;
                return;
        }

        glEnable(GL_FOG);
        GL_CHECK_ERROR;
        glFogf(GL_FOG_MODE, fogMode);
        GLfloat fogColor[4] = { colour.r, colour.g, colour.b, colour.a };
        glFogfv(GL_FOG_COLOR, fogColor);
        GL_CHECK_ERROR;
        glFogf(GL_FOG_DENSITY, density);
        GL_CHECK_ERROR;
        glFogf(GL_FOG_START, start);
        GL_CHECK_ERROR;
        glFogf(GL_FOG_END, end);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_convertProjectionMatrix(const Matrix4& matrix,
                                                  Matrix4& dest,
                                                  bool forGpuProgram)
    {
        dest = matrix;
    }

    void GLESRenderSystem::_makeProjectionMatrix(const Radian& fovy, Real aspect,
                                               Real nearPlane, Real farPlane,
                                               Matrix4& dest, bool forGpuProgram)
    {
        Radian thetaY(fovy / 2.0f);
        Real tanThetaY = Math::Tan(thetaY);

        Real w = (1.0f / tanThetaY) / aspect;
        Real h = 1.0f / tanThetaY;
        Real q, qn;
        if (farPlane == 0)
        {
            q = Frustum::INFINITE_FAR_PLANE_ADJUST - 1;
            qn = nearPlane * (Frustum::INFINITE_FAR_PLANE_ADJUST - 2);
        }
        else
        {
            q = -(farPlane + nearPlane) / (farPlane - nearPlane);
            qn = -2 * (farPlane * nearPlane) / (farPlane - nearPlane);
        }

        dest = Matrix4::ZERO;
        dest[0][0] = w;
        dest[1][1] = h;
        dest[2][2] = q;
        dest[2][3] = qn;
        dest[3][2] = -1;
    }

    void GLESRenderSystem::_makeProjectionMatrix(Real left, Real right,
                                               Real bottom, Real top,
                                               Real nearPlane, Real farPlane,
                                               Matrix4& dest, bool forGpuProgram)
    {
        Real width = right - left;
        Real height = top - bottom;
        Real q, qn;
        if (farPlane == 0)
        {
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

    void GLESRenderSystem::_makeOrthoMatrix(const Radian& fovy, Real aspect,
                                          Real nearPlane, Real farPlane,
                                          Matrix4& dest, bool forGpuProgram)
    {
        Radian thetaY(fovy / 2.0f);
        Real tanThetaY = Math::Tan(thetaY);

        // Real thetaX = thetaY * aspect;
        Real tanThetaX = tanThetaY * aspect; // Math::Tan(thetaX);
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
        dest[2][3] = -(farPlane + nearPlane) / (farPlane - nearPlane);
        dest[3][3] = 1;
    }

    void GLESRenderSystem::_applyObliqueDepthProjection(Matrix4& matrix,
                                                      const Plane& plane,
                                                      bool forGpuProgram)
    {
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

    void GLESRenderSystem::_setPolygonMode(PolygonMode level)
    {
        // Not supported
    }

    void GLESRenderSystem::setStencilCheckEnabled(bool enabled)
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

    void GLESRenderSystem::setStencilBufferParams(CompareFunction func,
                                                uint32 refValue, uint32 mask,
                                                StencilOperation stencilFailOp,
                                                StencilOperation depthFailOp,
                                                StencilOperation passOp,
                                                bool twoSidedOperation)
    {
        // Not implemented
    }

    GLuint GLESRenderSystem::getCombinedMinMipFilter(void) const
    {
        switch(mMinFilter)
        {
            case FO_ANISOTROPIC:
            case FO_LINEAR:
                switch (mMipFilter)
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
                switch (mMipFilter)
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


    void GLESRenderSystem::_setTextureUnitFiltering(size_t unit, FilterType ftype, FilterOptions fo)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        GL_CHECK_ERROR;

        switch (ftype)
        {
            case FT_MIN:
                mMinFilter = fo;
                // Combine with existing mip filter
                glTexParameteri(GL_TEXTURE_2D,
                                GL_TEXTURE_MIN_FILTER,
                                getCombinedMinMipFilter());
                GL_CHECK_ERROR;
                break;

            case FT_MAG:
                switch (fo)
                {
                    case FO_ANISOTROPIC: // GL treats linear and aniso the same
                    case FO_LINEAR:
                        glTexParameteri(GL_TEXTURE_2D,
                                        GL_TEXTURE_MAG_FILTER,
                                        GL_LINEAR);
                        GL_CHECK_ERROR;
                        break;
                    case FO_POINT:
                    case FO_NONE:
                        glTexParameteri(GL_TEXTURE_2D,
                                        GL_TEXTURE_MAG_FILTER,
                                        GL_NEAREST);
                        GL_CHECK_ERROR;
                        break;
                    }
                break;
            case FT_MIP:
                mMipFilter = fo;
                // Combine with existing min filter
                glTexParameteri(GL_TEXTURE_2D,
                                GL_TEXTURE_MIN_FILTER,
                                getCombinedMinMipFilter());

                GL_CHECK_ERROR;
                break;
        }

        glActiveTexture(GL_TEXTURE0);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy)
    {
        // Not supported
    }

    void GLESRenderSystem::setVertexDeclaration(VertexDeclaration* decl)
    {
    }

    void GLESRenderSystem::setVertexBufferBinding(VertexBufferBinding* binding)
    {
    }

    void GLESRenderSystem::_render(const RenderOperation& op)
    {
        GL_CHECK_ERROR;
        // Call super class
        RenderSystem::_render(op);
        void* pBufferData = 0;

        const VertexDeclaration::VertexElementList& decl =
            op.vertexData->vertexDeclaration->getElements();

        VertexDeclaration::VertexElementList::const_iterator elem, elemEnd;

        elemEnd = decl.end();

        std::vector<GLuint> attribsBound;

        for (elem = decl.begin(); elem != elemEnd; ++elem)
        {
            if (!op.vertexData->vertexBufferBinding->isBufferBound(elem->getSource()))
            {
                continue; // skip unbound elements
            }

            HardwareVertexBufferSharedPtr vertexBuffer =
                op.vertexData->vertexBufferBinding->getBuffer(elem->getSource());

            if (mCurrentCapabilities->hasCapability(RSC_VBO))
            {
                glBindBuffer(GL_ARRAY_BUFFER,
                    static_cast<const GLESHardwareVertexBuffer*>(vertexBuffer.get())->getGLBufferId());
                GL_CHECK_ERROR;
                pBufferData = VBO_BUFFER_OFFSET(elem->getOffset());
            }
            else
            {
                pBufferData = static_cast<const GLESDefaultHardwareVertexBuffer*>(vertexBuffer.get())->getDataPtr(elem->getOffset());
            }

            if (op.vertexData->vertexStart)
            {
                pBufferData = static_cast<char*>(pBufferData) + op.vertexData->vertexStart * vertexBuffer->getVertexSize();
            }

            unsigned int i = 0;
            VertexElementSemantic sem = elem->getSemantic();

            {
                GL_CHECK_ERROR;
                // fixed-function & builtin attribute support
                switch (sem)
                {
                    case VES_POSITION:
                        glVertexPointer(VertexElement::getTypeCount(elem->getType()),
                                        GLESHardwareBufferManager::getGLType(elem->getType()),
                                        static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                                        pBufferData);
                        GL_CHECK_ERROR;
                        glEnableClientState(GL_VERTEX_ARRAY);
                        GL_CHECK_ERROR;
                        break;
                    case VES_NORMAL:
                        glNormalPointer(GLESHardwareBufferManager::getGLType(elem->getType()),
                                        static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                                        pBufferData);
                        GL_CHECK_ERROR;
                        glEnableClientState(GL_NORMAL_ARRAY);
                        GL_CHECK_ERROR;
                        break;
                    case VES_DIFFUSE:
                        glColorPointer(4, GLESHardwareBufferManager::getGLType(elem->getType()),
                                       static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                                       pBufferData);
                        GL_CHECK_ERROR;
                        glEnableClientState(GL_COLOR_ARRAY);
                        GL_CHECK_ERROR;
                        break;
                    case VES_SPECULAR:
                       break;
                    case VES_TEXTURE_COORDINATES:
                        {
                            // fixed function matching to units based on tex_coord_set
                            for (i = 0; i < mDisabledTexUnitsFrom; i++)
                            {
                                // Only set this texture unit's texcoord pointer if it
                                // is supposed to be using this element's index
                                if (mTextureCoordIndex[i] == elem->getIndex())
                                {
                                    GL_CHECK_ERROR;
                                    glClientActiveTexture(GL_TEXTURE0 + i);
                                    GL_CHECK_ERROR;
                                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                                    GL_CHECK_ERROR;
                                    glTexCoordPointer(VertexElement::getTypeCount(elem->getType()),
                                                      GLESHardwareBufferManager::getGLType(elem->getType()),
                                                      static_cast<GLsizei>(vertexBuffer->getVertexSize()),
                                                      pBufferData);
                                    GL_CHECK_ERROR;
                                }
                            }
                        }
                        break;
                    default:
                        break;
                };
            }
        }

        glClientActiveTexture(GL_TEXTURE0);
        GL_CHECK_ERROR;

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
            default:
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
            if(mCurrentCapabilities->hasCapability(RSC_VBO))
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                             static_cast<GLESHardwareIndexBuffer*>(op.indexData->indexBuffer.get())->getGLBufferId());

                GL_CHECK_ERROR;
                pBufferData = VBO_BUFFER_OFFSET(op.indexData->indexStart *
                                                op.indexData->indexBuffer->getIndexSize());
            }
            else
            {
                pBufferData = static_cast<GLESDefaultHardwareIndexBuffer*>(
                                op.indexData->indexBuffer.get())->getDataPtr(
                                op.indexData->indexStart * op.indexData->indexBuffer->getIndexSize());
            }

            GLenum indexType =  (op.indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_16BIT) ? GL_UNSIGNED_SHORT : GL_SHORT;

            do
            {
                // Update derived depth bias
                if (mDerivedDepthBias && mCurrentPassIterationNum > 0)
                {
                    _setDepthBias(mDerivedDepthBiasBase +
                                  mDerivedDepthBiasMultiplier * mCurrentPassIterationNum,
                                  mDerivedDepthBiasSlopeScale);
                }
                glDrawElements(primType, op.indexData->indexCount, indexType, pBufferData);
                GL_CHECK_ERROR;
            } while (updatePassIterationRenderState());
        }
        else
        {
            do
            {
                // Update derived depth bias
                if (mDerivedDepthBias && mCurrentPassIterationNum > 0)
                {
                    _setDepthBias(mDerivedDepthBiasBase +
                                  mDerivedDepthBiasMultiplier * mCurrentPassIterationNum,
                                  mDerivedDepthBiasSlopeScale);
                }
                glDrawArrays(primType, 0, op.vertexData->vertexCount);
                GL_CHECK_ERROR;
            } while (updatePassIterationRenderState());
        }

        glDisableClientState(GL_VERTEX_ARRAY);
        GL_CHECK_ERROR;

        int max_textures;
        glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max_textures);
        for (int i = 0; i < max_textures; i++)
        {
            glClientActiveTexture(GL_TEXTURE0 + i);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        glGetError();

        glClientActiveTexture(GL_TEXTURE0);
        GL_CHECK_ERROR;
        glDisableClientState(GL_NORMAL_ARRAY);
        GL_CHECK_ERROR;
        glDisableClientState(GL_COLOR_ARRAY);
        GL_CHECK_ERROR;

#if 0
        // unbind any custom attributes
        for (std::vector<GLuint>::iterator ai = attribsBound.begin(); ai != attribsBound.end(); ++ai)
        {
            glDisableVertexAttribArray(*ai);
        }
#endif
        glColor4f(1.0,1.0,1.0,1.0);
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::setScissorTest(bool enabled, size_t left,
                                        size_t top, size_t right,
                                        size_t bottom)
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
            {
                y = top;
            }
            else
            {
                y = targetHeight - bottom;
            }
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
            {
                y = mActiveViewport->getActualTop();
            }
            else
            {
                y = targetHeight - mActiveViewport->getActualTop() - h;
            }
            glScissor(x, y, w, h);
        }
    }

    void GLESRenderSystem::clearFrameBuffer(unsigned int buffers,
                                            const ColourValue& colour,
                                            Real depth, unsigned short stencil)
    {
        bool colourMask = !mColourWrite[0] || !mColourWrite[1] ||
                          !mColourWrite[2] || !mColourWrite[3];

        GLbitfield flags = 0;
        if (buffers & FBT_COLOUR)
        {
            flags |= GL_COLOR_BUFFER_BIT;
            if (colourMask)
            {
                glColorMask(true, true, true, true);
                GL_CHECK_ERROR;
            }
            glClearColor(colour.r, colour.g, colour.b, colour.a);
            GL_CHECK_ERROR;
        }
        if (buffers & FBT_DEPTH)
        {
            flags |= GL_DEPTH_BUFFER_BIT;
            if (!mDepthWrite)
            {
                glDepthMask(GL_TRUE);
                GL_CHECK_ERROR;
            }
            glClearDepthf(depth);
            GL_CHECK_ERROR;
        }
        if (buffers & FBT_STENCIL)
        {
            flags |= GL_STENCIL_BUFFER_BIT;
            glStencilMask(0xFFFFFFFF);
            GL_CHECK_ERROR;
            glClearStencil(stencil);
            GL_CHECK_ERROR;
        }

        GLboolean scissorTestEnabled = glIsEnabled(GL_SCISSOR_TEST);
        GL_CHECK_ERROR;
        if (!scissorTestEnabled)
        {
            glEnable(GL_SCISSOR_TEST);
            GL_CHECK_ERROR;
        }

        GLint viewport[4], scissor[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        GL_CHECK_ERROR;
        glGetIntegerv(GL_SCISSOR_BOX, scissor);
        GL_CHECK_ERROR;
        bool scissorBoxDifference =
            viewport[0] != scissor[0] || viewport[1] != scissor[1] ||
            viewport[2] != scissor[2] || viewport[3] != scissor[3];
        if (scissorBoxDifference)
        {
            glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
            GL_CHECK_ERROR;
        }

        glClear(flags);
        GL_CHECK_ERROR;

        // Restore scissor box
        if (scissorBoxDifference)
        {
            glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
            GL_CHECK_ERROR;
        }

        // Restore scissor test
        if (!scissorTestEnabled)
        {
            glDisable(GL_SCISSOR_TEST);
            GL_CHECK_ERROR;
        }

        // Reset buffer write state
        if (!mDepthWrite && (buffers & FBT_DEPTH))
        {
            glDepthMask(GL_FALSE);
            GL_CHECK_ERROR;
        }

        if (colourMask && (buffers & FBT_COLOUR))
        {
            glColorMask(mColourWrite[0], mColourWrite[1], mColourWrite[2], mColourWrite[3]);
            GL_CHECK_ERROR;
        }

        if (buffers & FBT_STENCIL)
        {
            glStencilMask(mStencilMask);
            GL_CHECK_ERROR;
        }
    }

    HardwareOcclusionQuery* GLESRenderSystem::createHardwareOcclusionQuery(void)
    {
        // Not supported
        return 0;
    }

    Real GLESRenderSystem::getHorizontalTexelOffset(void)
    {
        return 0.0;
    }

    Real GLESRenderSystem::getVerticalTexelOffset(void)
    {
        return 0.0;
    }

    Real GLESRenderSystem::getMinimumDepthInputValue(void)
    {
        return -1.0f;
    }

    Real GLESRenderSystem::getMaximumDepthInputValue(void)
    {
        return 1.0f;
    }

    void GLESRenderSystem::registerThread()
    {
        // Not implemented
    }

    void GLESRenderSystem::unregisterThread()
    {
        // Not implemented
    }

    void GLESRenderSystem::preExtraThreadsStarted()
    {
        // Not implemented
    }

    void GLESRenderSystem::postExtraThreadsStarted()
    {
        // Not implemented
    }

    void GLESRenderSystem::setClipPlanesImpl(const Ogre::PlaneList& clipPlanes)
    {
        // A note on GL user clipping:
        // When an ARB vertex program is enabled in GL, user clipping is completely
        // disabled. There is no way around this, it's just turned off.
        // When using GLSL, user clipping can work but you have to include a
        // glClipVertex command in your vertex shader.
        // Thus the planes set here may not actually be respected.
        size_t i = 0;
        size_t numClipPlanes;
        GLfloat clipPlane[4];

        // Save previous modelview
        glMatrixMode(GL_MODELVIEW);
        GL_CHECK_ERROR;
        glPushMatrix();
        GL_CHECK_ERROR;
        // just load view matrix (identity world)
        GLfloat mat[16];
        makeGLMatrix(mat, mViewMatrix);
        glLoadMatrixf(mat);
        GL_CHECK_ERROR;

        numClipPlanes = clipPlanes.size();

        GLint maxClip;
        glGetIntegerv(GL_MAX_CLIP_PLANES, &maxClip);

        if (numClipPlanes >= maxClip)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                        "Unable to set clip plane",
                        "GLESRenderSystem::setClipPlanes");
        }

        for (i = 0; i < numClipPlanes; ++i)
        {
            GLenum clipPlaneId = static_cast<GLenum>(GL_CLIP_PLANE0 + i);
            const Plane& plane = clipPlanes[i];

            clipPlane[0] = plane.normal.x;
            clipPlane[1] = plane.normal.y;
            clipPlane[2] = plane.normal.z;
            clipPlane[3] = plane.d;

            glClipPlanef(clipPlaneId, clipPlane);
            GL_CHECK_ERROR;
            glEnable(clipPlaneId);
            GL_CHECK_ERROR;
        }

        // disable remaining clip planes
        for ( ; i < maxClip; ++i)
        {
            glDisable(static_cast<GLenum>(GL_CLIP_PLANE0 + i));
            GL_CHECK_ERROR;
        }

        // restore matrices
        glPopMatrix();
        GL_CHECK_ERROR;
    }

    void GLESRenderSystem::_switchContext(GLESContext *context)
    {
        // It's ready to switching
        mCurrentContext->endCurrent();
        mCurrentContext = context;
        mCurrentContext->setCurrent();

        // Check if the context has already done one-time initialisation
        if (!mCurrentContext->getInitialized())
        {
            _oneTimeContextInitialization();
            mCurrentContext->setInitialized();
        }

        // Must reset depth/colour write mask to according with user desired, otherwise,
        // clearFrameBuffer would be wrong because the value we are recorded may be
        // difference with the really state stored in GL context.
        glDepthMask(mDepthWrite);
        glColorMask(mColourWrite[0], mColourWrite[1], mColourWrite[2], mColourWrite[3]);
        glStencilMask(mStencilMask);
    }

    void GLESRenderSystem::_unregisterContext(GLESContext *context)
    {
        if (mCurrentContext == context)
        {
            if (mCurrentContext != mMainContext)
            {
                _switchContext(mMainContext);
            }
            else
            {
                mCurrentContext->endCurrent();
                mCurrentContext = 0;
                mMainContext = 0;
            }
        }
    }

    void GLESRenderSystem::_oneTimeContextInitialization()
    {
        // glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
        // glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
        // glEnable(GL_COLOR_SUM);
        glDisable(GL_DITHER);
        GL_CHECK_ERROR;

#if 0
        int fsaa_active = false;
        glGetIntegerv(GL_SAMPLE_BUFFERS,(GLint*)&fsaa_active);
        GL_CHECK_ERROR;
        if (fsaa_active)
        {
            glEnable(GL_MULTISAMPLE);
            GL_CHECK_ERROR;
            LogManager::getSingleton().logMessage("Using FSAA from GL_multisample extension.");
        }
#endif
    }

    void GLESRenderSystem::initialiseContext(RenderWindow* primary)
    {
        mMainContext = 0;
        primary->getCustomAttribute("GLCONTEXT", &mMainContext);
        mCurrentContext = mMainContext;

        if (mCurrentContext)
            mCurrentContext->setCurrent();

        mGLSupport->initialiseExtensions();

        LogManager::getSingleton().logMessage("***************************");
        LogManager::getSingleton().logMessage("*** GL Renderer Started ***");
        LogManager::getSingleton().logMessage("***************************");
    }

    void GLESRenderSystem::_setRenderTarget(RenderTarget *target)
    {
        if (mActiveRenderTarget)
        {
            mRTTManager->unbind(mActiveRenderTarget);
        }

        mActiveRenderTarget = target;

        GLESContext *newContext = 0;
        target->getCustomAttribute("GLCONTEXT", &newContext);
        if (newContext && mCurrentContext != newContext)
        {
            _switchContext(newContext);
        }

        mRTTManager->bind(target);
    }

    void GLESRenderSystem::makeGLMatrix(GLfloat gl_matrix[16], const Matrix4& m)
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

    GLint GLESRenderSystem::convertCompareFunction(CompareFunction func) const
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
        // To keep compiler happy
        return GL_ALWAYS;
    }

    void GLESRenderSystem::setLights()
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

    void GLESRenderSystem::setGLLightPositionDirection(Light* lt, GLenum lightindex)
    {
        // Set position / direction
        Vector4 vec;
        // Use general 4D vector which is the same as GL's approach
        vec = lt->getAs4DVector();

        glLightfv(lightindex, GL_POSITION, vec.ptr());
        GL_CHECK_ERROR;

        // Set spotlight direction
        if (lt->getType() == Light::LT_SPOTLIGHT)
        {
            vec = lt->getDerivedDirection();
            vec.w = 0.0;

            glLightfv(lightindex, GL_SPOT_DIRECTION, vec.ptr());
            GL_CHECK_ERROR;
        }
    }

    void GLESRenderSystem::setGLLight(size_t index, Light* lt)
    {
        GLenum gl_index = GL_LIGHT0 + index;

        if (!lt)
        {
            // Disable in the scene
            glDisable(gl_index);
            GL_CHECK_ERROR;
        }
        else
        {
            switch (lt->getType())
            {
                case Light::LT_SPOTLIGHT:
                    glLightf(gl_index, GL_SPOT_CUTOFF, 0.5f * lt->getSpotlightOuterAngle().valueDegrees());
                    GL_CHECK_ERROR;
                    glLightf(gl_index, GL_SPOT_EXPONENT, lt->getSpotlightFalloff());
                    GL_CHECK_ERROR;
                    break;
                default:
                    glLightf(gl_index, GL_SPOT_CUTOFF, 180.0);
                    GL_CHECK_ERROR;
                    break;
            }

            // Color
            ColourValue col;
            col = lt->getDiffuseColour();

            GLfloat f4vals[4] = {col.r, col.g, col.b, col.a};
            glLightfv(gl_index, GL_DIFFUSE, f4vals);
            GL_CHECK_ERROR;

            col = lt->getSpecularColour();
            f4vals[0] = col.r;
            f4vals[1] = col.g;
            f4vals[2] = col.b;
            f4vals[3] = col.a;
            glLightfv(gl_index, GL_SPECULAR, f4vals);
            GL_CHECK_ERROR;

            // Disable ambient light for movables;
            f4vals[0] = 0;
            f4vals[1] = 0;
            f4vals[2] = 0;
            f4vals[3] = 1;
            glLightfv(gl_index, GL_AMBIENT, f4vals);
            GL_CHECK_ERROR;

            setGLLightPositionDirection(lt, gl_index);

            // Attenuation
            glLightf(gl_index, GL_CONSTANT_ATTENUATION, lt->getAttenuationConstant());
            GL_CHECK_ERROR;
            glLightf(gl_index, GL_LINEAR_ATTENUATION, lt->getAttenuationLinear());
            GL_CHECK_ERROR;
            glLightf(gl_index, GL_QUADRATIC_ATTENUATION, lt->getAttenuationQuadric());
            GL_CHECK_ERROR;
            // Enable in the scene
            glEnable(gl_index);
            GL_CHECK_ERROR;
        }
    }

    //---------------------------------------------------------------------
    void GLESRenderSystem::bindGpuProgram(GpuProgram* prg)
    {
        // Not implemented
    }

    void GLESRenderSystem::unbindGpuProgram(GpuProgramType gptype)
    {
        // Not implemented
    }

    void GLESRenderSystem::bindGpuProgramParameters(GpuProgramType gptype, GpuProgramParametersSharedPtr params, uint16 mask)
    {
        // Not implemented
    }

    void GLESRenderSystem::bindGpuProgramPassIterationParameters(GpuProgramType gptype)
    {
        // Not implemented
    }
}
