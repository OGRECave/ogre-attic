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

/**
    \file 
        CubeMapping.h
    \brief
        Specialisation of OGRE's framework application to show the
        cube mapping feature where a wrap-around environment is reflected
        off of an object.
*/

#include "ExampleApplication.h"

#include "OgreStringConverter.h"

#define ENTITY_NAME "CubeMappedEntity"
#define MATERIAL_NAME "Examples/SceneCubeMap2"

/* ==================================================================== */
/*    Perlin Noise data and algorithms - copied from Perlin himself :)  */
/* ==================================================================== */
#define lerp(t,a,b) ( (a)+(t)*((b)-(a)) )
#define fade(t) ( (t)*(t)*(t)*(t)*((t)*((t)*6-15)+10) )
double grad(int hash, double x, double y, double z) {
	int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
	double u = h<8||h==12||h==13 ? x : y,   // INTO 12 GRADIENT DIRECTIONS.
		v = h<4||h==12||h==13 ? y : z;
	return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}
int p[512]={
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,

	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

double noise3(double x, double y, double z) {
	int X = (int)floor(x) & 255,                  // FIND UNIT CUBE THAT
		Y = (int)floor(y) & 255,                  // CONTAINS POINT.
		Z = (int)floor(z) & 255;
	x -= floor(x);                                // FIND RELATIVE X,Y,Z
	y -= floor(y);                                // OF POINT IN CUBE.
	z -= floor(z);
	double u = fade(x),                                // COMPUTE FADE CURVES
		v = fade(y),                                // FOR EACH OF X,Y,Z.
		w = fade(z);
	int A = p[X  ]+Y, AA = p[A]+Z, AB = p[A+1]+Z,      // HASH COORDINATES OF
		B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z;      // THE 8 CUBE CORNERS,

	return lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z   ),  // AND ADD
						grad(p[BA  ], x-1, y  , z   )), // BLENDED
					lerp(u, grad(p[AB  ], x  , y-1, z   ),  // RESULTS
						grad(p[BB  ], x-1, y-1, z   ))),// FROM  8
				lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1 ),  // CORNERS
						grad(p[BA+1], x-1, y  , z-1 )), // OF CUBE
					lerp(u, grad(p[AB+1], x  , y-1, z-1 ),
						grad(p[BB+1], x-1, y-1, z-1 ))));
}
/* ==================================================================== */
/*  Generic normal calculation, please fix it if it's slow/incorrect ;) */
/* ==================================================================== */
void _normalsZero(int numVertices, Real *normals) 
{
	memset(normals, 0, sizeof(Real)*numVertices * 3);
}
void _normalsAdd(int numFaces, unsigned short *vertexIndices, 
	Real *vertices, Real *normals) 
{
	for(int i=0;i<numFaces;i++) {
		int p0 = vertexIndices[3*i] ;
		int p1 = vertexIndices[3*i+1] ;
		int p2 = vertexIndices[3*i+2] ;
		Vector3 v0(vertices[3*p0], vertices[3*p0+1], vertices[3*p0+2]);
		Vector3 v1(vertices[3*p1], vertices[3*p1+1], vertices[3*p1+2]);
		Vector3 v2(vertices[3*p2], vertices[3*p2+1], vertices[3*p2+2]);
		Vector3 diff1 = v1 - v2 ;
		Vector3 diff2 = v1 - v0 ;
		Vector3 fn = diff1.crossProduct(diff2);
#define _ADD_VECTOR_TO_REALS(ptr,vec) { *(ptr)+=vec.x; *(ptr+1)+=vec.y; *(ptr+2)+=vec.z; }
		_ADD_VECTOR_TO_REALS(normals+3*p0, fn);
		_ADD_VECTOR_TO_REALS(normals+3*p1, fn);
		_ADD_VECTOR_TO_REALS(normals+3*p2, fn);
#undef _ADD_VECTOR_TO_REALS
	}
}

void _normalsSaveNormalized(int numVertices, Real *normals)
{
	for(int i=0;i<numVertices;i++, normals+=3) {
		Vector3 n(normals[0], normals[1], normals[2]);
		n.normalisedCopy();
		normals[0] = n.x ;
		normals[1] = n.y ;
		normals[2] = n.z ;
	}
}

void calculateMeshNormals(Mesh *mesh)
{
	bool sharedUsed = false ;
	for(int m=0;m<mesh->getNumSubMeshes();m++) {
		SubMesh *subMesh = mesh->getSubMesh(m);
		if (subMesh->useSharedVertices) { 
			if (!sharedUsed) { //first time
				_normalsZero(mesh->sharedGeometry.numVertices, 
					mesh->sharedGeometry.pNormals);
				sharedUsed = true ;
			}
			_normalsAdd(subMesh->numFaces, subMesh->faceVertexIndices,
				mesh->sharedGeometry.pVertices,
				mesh->sharedGeometry.pNormals);
		} else {
			_normalsZero(subMesh->geometry.numVertices,
				subMesh->geometry.pNormals);
			_normalsAdd(subMesh->numFaces, subMesh->faceVertexIndices,
				subMesh->geometry.pVertices,
				subMesh->geometry.pNormals);
			_normalsSaveNormalized(subMesh->geometry.numVertices,
				subMesh->geometry.pNormals);
		}
	}
	if (sharedUsed) {
		_normalsSaveNormalized(mesh->sharedGeometry.numVertices,
			mesh->sharedGeometry.pNormals);
	}
}
/* ==================================================================== */
/*                                 Main part                            */
/* ==================================================================== */

class CubeMapListener : public ExampleFrameListener
{
private:
	// main variables
	Real tm ;
	Real timeoutDelay ;
	SceneManager *mSceneMgr ;
	SceneNode *objectNode ;

	// mesh-specific data
	Mesh *objectMesh ;
	Entity *objectEntity ;
	int numSubMeshes ;
	Real **defaultVertices ;
	Real *parentVertexBuffer ;
	std::vector<Material*> clonedMaterials ;

	// configuration
	Real displacement ;
	Real density ;
	Real timeDensity ;
	bool noiseOn ;
	int currentMeshIndex ;
	int currentLBXindex ;
	LayerBlendOperationEx currentLBX ;
	std::vector<String> availableMeshes ;
	Material *material ;
	
	void _updateGeometryNoise(GeometryData& geometry, Real *defaultVertices)
	{
		int numVertices = geometry.numVertices ;
		for(int i=0;i<3*numVertices;i+=3) {
			double n = 1 + displacement * noise3(
				defaultVertices[i]/density + tm,
				defaultVertices[i+1]/density + tm,
				defaultVertices[i+2]/density + tm);
			geometry.pVertices[i+0] = defaultVertices[i] * n ;
			geometry.pVertices[i+1] = defaultVertices[i+1] * n ;
			geometry.pVertices[i+2] = defaultVertices[i+2] * n ;
		}
	}
	
	void updateNoise()
	{
		int currentVertexBuffer=0;
		bool usedShared = false ;
		for(int m=0;m<numSubMeshes;m++) { // for each subMesh
			SubMesh *subMesh = objectMesh->getSubMesh(m);
			if (subMesh->useSharedVertices) {
				usedShared = true ;
				continue ;
			}
			_updateGeometryNoise(subMesh->geometry, defaultVertices[m]);
		}
		if (usedShared) {
			_updateGeometryNoise(objectMesh->sharedGeometry, parentVertexBuffer);
		}
		calculateMeshNormals(objectMesh);
	}

	void clearEntity()
	{
		int m;
		// delete cloned materials 
		for(m=0;m<clonedMaterials.size();m++) {
			printf("###Destroying material %s\n", 
				clonedMaterials[m]->getName().c_str());
			MaterialManager::getSingleton().unload(clonedMaterials[m]) ;
			delete clonedMaterials[m];
		}
		clonedMaterials.clear();
		
		// delete vertex buffers
		for(m=0;m<numSubMeshes;m++) {
			if (defaultVertices[m]) 
				delete [] defaultVertices[m];
		}
		delete [] defaultVertices ;

		if (parentVertexBuffer) {
			delete [] parentVertexBuffer ;
		}

		// detach and destroy entity
		objectNode->detachAllObjects();
		mSceneMgr->removeEntity(ENTITY_NAME);
		
		// destroy mesh as well, to reset its geometry
		MeshManager::getSingleton().unload(objectMesh);
		delete objectMesh;
		
		objectEntity = 0 ;
	}

	void prepareEntity(const String& meshName) 
	{
		if (objectEntity) {
			clearEntity();
		}
		
		objectMesh = (Mesh*) MeshManager::getSingleton().getByName(meshName);
		if (!objectMesh) {
			objectMesh = MeshManager::getSingleton().load(meshName);
			if (!objectMesh) {
				Except(Exception::ERR_ITEM_NOT_FOUND,
					"Can't find a mesh: '"+meshName+"'",
					"CubeMapListener::prepareEntity");
			}
		}

		// save old vertices, prepare materials if necessary
		numSubMeshes = objectMesh->getNumSubMeshes();
		defaultVertices = new Real*[numSubMeshes];
		parentVertexBuffer = 0 ;

#define COPY_VERTEX_DATA(dstBuf,srcGeom) \
int numVertices = (srcGeom).numVertices; \
dstBuf = new Real[numVertices*3] ;\
memcpy(dstBuf,(srcGeom).pVertices,numVertices*3*sizeof(Real)); 

		for(int m=0;m<numSubMeshes;m++) {
			SubMesh *subMesh = objectMesh->getSubMesh(m);
			if (subMesh->useSharedVertices) { // need to store shared geometry 
				if (!parentVertexBuffer) { // only once
					COPY_VERTEX_DATA(parentVertexBuffer, objectMesh->sharedGeometry);
				}
				defaultVertices[m] = 0 ;
			} else {
				COPY_VERTEX_DATA(defaultVertices[m], subMesh->geometry);
			}
		}

        objectEntity = mSceneMgr->createEntity( ENTITY_NAME, meshName);
        objectEntity->setMaterialName( material->getName() );
		
		// go through subentities and set materials as required
		for(int m=0;m<numSubMeshes;m++) {
			SubMesh *subMesh = objectMesh->getSubMesh(m);
			SubEntity *subEntity = objectEntity->getSubEntity(m);
			// check if this submesh has material set
			if (subMesh->isMatInitialised()) {
				const String& matName = subMesh->getMaterialName();
				Material *subMat = (Material*) MaterialManager::getSingleton().
					getByName(matName);
				if (subMat && subMat) { // clone material, add layers from global material
					Material *cloned = subMat->clone(
						"CubeMapTempMaterial#"+StringConverter::toString(m));
					// can't help it - have to do it
					if (meshName=="knot.mesh") {
						for(int tl=0;tl<cloned->getNumTextureLayers();tl++) {
							Material::TextureLayer *orgTL = cloned->getTextureLayer(tl);
							orgTL->setScrollAnimation(0.25, 0);
						}
					}
					// add layers
					printf("SubMesh #%d cloned material had %d texture layers\n", m, cloned->getNumTextureLayers());
					for(int tl=0;tl<material->getNumTextureLayers();tl++) {
						Material::TextureLayer *orgTL = material->getTextureLayer(tl);
						Material::TextureLayer *newTL = cloned->addTextureLayer(
							orgTL->getTextureName());
						*newTL = *orgTL ;
						newTL->setColourOperationEx(currentLBX);
					}
					printf("SubMesh #%d cloned material now has %d texture layers\n", m, cloned->getNumTextureLayers());
					subEntity->setMaterialName(cloned->getName());
					clonedMaterials.push_back(cloned);
				} else {
					subEntity->setMaterialName(material->getName());
				}
			} else {
				subEntity->setMaterialName(material->getName());
			}
		}

		objectNode->attachObject(objectEntity);
		
		if (noiseOn)
			updateNoise();
	}
	
	void updateInfoDisplacement()
	{
		GuiManager::getSingleton().getGuiElement("Example/CubeMapping/Displacement")
			->setCaption("[1/2] Displacement: "+StringConverter::toString(displacement));		
	}
	void updateInfoDensity()
	{
		GuiManager::getSingleton().getGuiElement("Example/CubeMapping/Density")
			->setCaption("[3/4] Noise density: "+StringConverter::toString(density));		
	}
	void updateInfoTimeDensity()
	{
		GuiManager::getSingleton().getGuiElement("Example/CubeMapping/TimeDensity")
			->setCaption("[5/6] Time density: "+StringConverter::toString(timeDensity));
	}
	void switchObjects()
	{
		currentMeshIndex++;
		if (currentMeshIndex >= availableMeshes.size())
			currentMeshIndex=0;

		const String& meshName = availableMeshes[currentMeshIndex];
		printf("Switching to object: %s\n", meshName.c_str());
		prepareEntity(meshName);
		GuiManager::getSingleton().getGuiElement("Example/CubeMapping/Object")
			->setCaption("[O] Object: "+meshName);		
	}
	void switchNoiseOn()
	{
		noiseOn = !noiseOn ;
		GuiManager::getSingleton().getGuiElement("Example/CubeMapping/Noise")
			->setCaption(String("[N] Noise: ")+ ((noiseOn)?"on":"off") );		
	}
	void switchMaterialBlending()
	{
		currentLBXindex++;
		if (currentLBXindex>5) {
			currentLBXindex = 0;
		}
		String lbxName ;
#define _LAZYERU_(a,b,c) case a : currentLBX = b ; lbxName = c ; break ;
		switch (currentLBXindex) {
			_LAZYERU_(0, LBX_ADD, "ADD")
			_LAZYERU_(1, LBX_MODULATE, "MODULATE")
			_LAZYERU_(2, LBX_MODULATE_X2, "MODULATE X2")
			_LAZYERU_(3, LBX_MODULATE_X4, "MODULATE X4")
			_LAZYERU_(4, LBX_SOURCE1, "SOURCE1")
			_LAZYERU_(5, LBX_SOURCE2, "SOURCE2")
			// more?
		}
#undef _LAZYERU_		
		// reset entities, materials and so on
		prepareEntity(availableMeshes[currentMeshIndex]);
		GuiManager::getSingleton().getGuiElement("Example/CubeMapping/Material")
			->setCaption("[M] Material blend:"+lbxName);
	}
	
#define RANDOM_FROM(a,b) (((float)(rand() & 65535)) / 65536.0f * ((b)-(a)) + (a))
	void goRandom()
	{
		displacement = RANDOM_FROM(0.0f, 1.0f);
		updateInfoDisplacement();

		density = RANDOM_FROM(1.0f, 300.0f);
		updateInfoDensity();
		
		timeDensity = RANDOM_FROM(1.0f, 10.0f);
		updateInfoTimeDensity();
	}
	
public:
    CubeMapListener(RenderWindow* win, Camera* cam,
			SceneManager *sceneMgr, SceneNode *objectNode)
        : ExampleFrameListener(win, cam)
    {
		this->mSceneMgr = sceneMgr ;
		this->objectNode = objectNode ;

		tm = 0 ;
		timeoutDelay = 0 ;
		displacement = 0.1f;
		density = 50.0f;
		timeDensity = 5.0f;
		objectEntity = 0 ;
		
		material = (Material*) MaterialManager::getSingleton().
			getByName(MATERIAL_NAME);
		if (!material) {
			Except( Exception::ERR_ITEM_NOT_FOUND,
				"Can't find material: "+StringConverter::toString(MATERIAL_NAME),
				"CubeMapListener::CubeMapListener");
		}

		// these two make problems - numVertices = 0, will play later
		availableMeshes.push_back("ogrehead.mesh");
		availableMeshes.push_back("geosphere4500.mesh");
		availableMeshes.push_back("knot.mesh");
		availableMeshes.push_back("geosphere12500.mesh");
		availableMeshes.push_back("razor.mesh");
		availableMeshes.push_back("robot.mesh");
		availableMeshes.push_back("geosphere1000.mesh");
		availableMeshes.push_back("geosphere8000.mesh");
		availableMeshes.push_back("geosphere19220.mesh");
		availableMeshes.push_back("sphere.mesh");

		currentMeshIndex = -1 ;
		switchObjects();
		
		currentLBXindex = -1 ;
		switchMaterialBlending();
		
		noiseOn = false ;
		switchNoiseOn();

		updateInfoDisplacement();
		updateInfoDensity();
		updateInfoTimeDensity();
    }
    virtual bool frameStarted(const FrameEvent& evt)
    {
		tm += evt.timeSinceLastFrame / timeDensity ;

		if (noiseOn)
			updateNoise();
		
        // Call default
        return ExampleFrameListener::frameStarted(evt);
    }
	virtual bool processUnbufferedKeyInput(const FrameEvent& evt)
    {
		bool retval = ExampleFrameListener::processUnbufferedKeyInput(evt);

		Real changeSpeed = evt.timeSinceLastFrame ;
		
		// adjust keyboard speed with SHIFT (increase) and CONTROL (decrease)
		if (mInputDevice->isKeyDown(KC_LSHIFT) || mInputDevice->isKeyDown(KC_RSHIFT)) {
			changeSpeed *= 10.0f ;
		}
		if (mInputDevice->isKeyDown(KC_LCONTROL)) { 
			changeSpeed /= 10.0f ;
		}
		
#define ADJUST_RANGE(_value,_keyPlus,_keyMinus,_minVal,_maxVal,_change,_macro) {\
	if (mInputDevice->isKeyDown(_keyPlus)) \
		{ _value+=_change ; if (_value>=_maxVal) _value = _maxVal ; _macro ; } ; \
	if (mInputDevice->isKeyDown(_keyMinus)) \
		{ _value-=_change; if (_value<=_minVal) _value = _minVal ; _macro ; } ; \
}
		
		ADJUST_RANGE(displacement, KC_2, KC_1, -2, 2, 0.1f*changeSpeed, updateInfoDisplacement()) ;

		ADJUST_RANGE(density, KC_4, KC_3, 0.1, 500, 10.0f*changeSpeed, updateInfoDensity()) ;

		ADJUST_RANGE(timeDensity, KC_6, KC_5, 1, 10, 1.0f*changeSpeed, updateInfoTimeDensity()) ;

#define SWITCH_VALUE(_key,_timeDelay, _macro) { \
		if (mInputDevice->isKeyDown(_key) && timeoutDelay==0) { \
			timeoutDelay = _timeDelay ; _macro ;} }
	
		timeoutDelay-=evt.timeSinceLastFrame ;
		if (timeoutDelay<=0)
			timeoutDelay = 0;

		SWITCH_VALUE(KC_O, 0.5f, switchObjects());

		SWITCH_VALUE(KC_N, 0.5f, switchNoiseOn());

		SWITCH_VALUE(KC_M, 0.5f, switchMaterialBlending());
		
		SWITCH_VALUE(KC_SPACE, 0.5f, goRandom());

		return retval ;
	}
     	
	
} ;

class CubeMapApplication : public ExampleApplication
{
public:
    CubeMapApplication() {}

protected:
	SceneNode *objectNode;

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a skybox
        mSceneMgr->setSkyBox(true, "Examples/SceneSkyBox2");

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);

        objectNode = static_cast<SceneNode*>(mSceneMgr->getRootSceneNode()->createChild());

		// show overlay
		Overlay *overlay = (Overlay*)OverlayManager::getSingleton().getByName("Example/CubeMappingOverlay");    
		overlay->show();
	}

    void createFrameListener(void)
    {
        mFrameListener= new CubeMapListener(mWindow, mCamera, mSceneMgr, objectNode);
        mRoot->addFrameListener(mFrameListener);
    }
};
