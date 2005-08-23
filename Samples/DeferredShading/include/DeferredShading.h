/**
Implementation of Deferred Shading in OGRE using Multiple Render Targets and
HLSL/GLSL high level language shaders.
	// W.J. :wumpus: van der Laan 2005 //

Deferred shading renders the scene to a 'fat' texture format, using a shader that outputs colour, 
normal, depth, and possible other attributes per fragment. Multi Render Target is required as we 
are dealing with many outputs which get written into multiple render textures in the same pass.

After rendering the scene in this format, the shading (lighting) can be done as a post process. 
This means that lighting is done in screen space. Adding them requires nothing more than rendering 
a screenful quad; thus the method allows for an enormous amount of lights without noticable 
performance loss.

Little lights affecting small area ("Minilights") can be even further optimised by rendering 
their convex bounding geometry. This is also shown in this demo by 6 swarming lights.

The paper for GDC2004 on Deferred Shading can be found here:
  http://www.talula.demon.co.uk/DeferredShading.pdf

This uses a heavily hacked version of the Ogre PostProcessing framework by Manuel.
*******************************************************************************
Copyright (c) W.J. van der Laan

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software  and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject 
to the following conditions:

The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************
*/
#ifndef H_WJ_DeferredShadingSystem
#define H_WJ_DeferredShadingSystem
#include "OgrePostFilter/OgrePostFilterManager.h"


/** Renderable minilight. Do not create this directly, but use 
	DeferredShadingSystem::createMLight.
 */
class MLight
{
public:
	MLight(Ogre::SceneManager *mSceneMgr, Ogre::SceneNode *parent, uint32 visibilityFlags);

	/** Set constant, linear, quadratic falloff terms 
	 */
	void setFalloff(float c, float b, float a);

	/** Set the diffuse colour 
	 */
	void setDiffuseColour(const Ogre::ColourValue &col);
	void setDiffuseColour(float r=1.0f, float g=1.0f, float b=1.0f, float a=1.0f)
	{
		setDiffuseColour(Ogre::ColourValue(r,g,b,a));
	}

	/** Set the specular colour
	 */
	void setSpecularColour(const Ogre::ColourValue &col);
	void setSpecularColour(float r=1.0f, float g=1.0f, float b=1.0f, float a=1.0f)
	{
		setSpecularColour(Ogre::ColourValue(r,g,b,a));
	}

	/** Get node to animate this light entity
	 */
	Ogre::SceneNode *getNode();

	/** Get light entity
	 */
	Ogre::Entity *getEntity() { return geom; }

	/** Get diffuse colour.
	*/
	Ogre::ColourValue getDiffuseColour();

	/** Get specular colour. 
	*/
	Ogre::ColourValue getSpecularColour();
protected:
	Ogre::SceneNode *nodeM; // Position node
	Ogre::SceneNode *nodeL; // Scaling node
	Ogre::Entity *geom;
};

/** System to manage Deferred Shading for a camera/render target.
 */
class DeferredShadingSystem
{
public:
	DeferredShadingSystem(Ogre::RenderTarget *win, Ogre::SceneManager *sm, Ogre::SceneNode *rootNode, Ogre::Camera *cam);
	~DeferredShadingSystem();

	enum DSMode
	{
		DSM_SINGLEPASS = 0,  // Single pass + two lights
		DSM_MULTIPASS = 1,   // Multi pass
		DSM_SHOWCOLOUR = 2,  // Show diffuse (for debugging)
		DSM_SHOWNORMALS = 3, // Show normals (for debugging)
		DSM_SHOWDSP = 4,	 // Show depth and specular channel (for debugging)
		DSM_COUNT = 5
	};

	/** Set rendering mode (one of DSMode)
	 */
	void setMode(DSMode mode);

	/** Activate or deactivate system
	 */
	void setActive(bool active);

	/** Activate or deactivate visiblity of MiniLights
	 */
	void setMLightsVisible(bool active);

	/** Create a new MiniLight 
	 */
	MLight *createMLight();

	/** Destroy a MiniLight
	 */
	void destroyMLight(MLight *m);

	/** Update fat (origin) render target
	*/
	void update();

	/// Visibility mask for scene
	static const uint32 SceneVisibilityMask = 0x00000001;
	/// Visibility mask for post-processing geometry (lights, unlit particles)
	static const uint32 PostVisibilityMask = 0x00000002;
protected:
	Ogre::RenderTarget *mWindow;
	Ogre::SceneManager *mSceneMgr;
	Ogre::SceneNode *mRootNode;
	Ogre::Camera *mCamera;
	
	// Our post filter manager
	OgrePostFilterManager *iPfManager;
	// Fat render target
	Ogre::MultiRenderTarget *rttTex;
	// Filters
	OgrePostFilter *iDeferredShadingPf;
	OgrePostFilter *iShowNormalPf;
	OgrePostFilter *iShowDSPf;
	OgrePostFilter *iShowColourPf;
	OgrePostFilter *iDeferredMultiShadingPf;

	std::set<MLight*> mLights;
	Ogre::SceneNode *mLightParentNode;
	Ogre::TexturePtr mTexture0, mTexture1;

    // Just override the mandatory create scene method
    void createResources();
	void initialiseLightGeometry();

};

#endif
