/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2003 The OGRE Team
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/

/**
    \file 
        Dot3Bump.cpp
    \brief
        Specialisation of OGRE's framework application to show the
        dotproduct blending operation and normalization cube map usage
		for achieving bump mapping effect
	\par
		Tangent space computations made through the guide of the
		tutorial on bump mapping from http://users.ox.ac.uk/~univ1234
		author : paul.baker@univ.ox.ac.uk
**/

#include "Ogre.h"
#include "ExampleApplication.h"

#if OGRE_PLATFORM == PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

// entities we'll use
#define NUM_ENTITIES 4
Entity* mEntities[NUM_ENTITIES];
String mEntityMeshes[NUM_ENTITIES] = 
{
    "knot.mesh",
    "ogrehead.mesh",
    "razor.mesh",
    "robot.mesh"
};
size_t mCurrentEntity = 0;

// Lights
#define NUM_LIGHTS 3

// the light
Light *mLights[NUM_LIGHTS];
// billboards for lights
BillboardSet* mLightFlareSets[NUM_LIGHTS];
Billboard* mLightFlares[NUM_LIGHTS];
// Positions for lights
Vector3 mLightPositions[NUM_LIGHTS] = 
{
	Vector3(300,0,0),
	Vector3(-200,50,0),
	Vector3(0, -300, -100)
};
// Base orientations of the lights 
Real mLightRotationAngles[NUM_LIGHTS] = { 0,  30, 75 };
Vector3 mLightRotationAxes[NUM_LIGHTS] = {
    Vector3::UNIT_X, 
    Vector3::UNIT_Z,
    Vector3::UNIT_Y
};
// Rotation speed for lights, degrees per second
Real mLightSpeeds[NUM_LIGHTS] = { 30, 10, 50};

// Colours for the lights
ColourValue mDiffuseLightColours[NUM_LIGHTS] =
{
	ColourValue(1, 1, 1),
	ColourValue(1, 0, 0),
	ColourValue(1, 1, 0.5)
};
ColourValue mSpecularLightColours[NUM_LIGHTS] =
{
	ColourValue(1, 1, 1),
	ColourValue(1, 0.8, 0.8),
	ColourValue(1, 1, 0.8)
};
// Which lights are enabled
bool mLightState[NUM_LIGHTS] = 
{
	true,
	true,
	false
};
// The materials
#define NUM_MATERIALS 3
String mMaterialNames[NUM_MATERIALS] = 
{
	"Examples/BumpMapping/SingleLight",
	"Examples/BumpMapping/MultiLight",
	"Examples/BumpMapping/MultiLightSpecular"
};
size_t mCurrentMaterial = 1;

// the scene node of the entity
SceneNode *mMainNode;
// the light nodes
SceneNode* mLightNodes[NUM_LIGHTS];
// the light node pivots
SceneNode* mLightPivots[NUM_LIGHTS];

GuiElement* mObjectInfo;
GuiElement* mMaterialInfo;
GuiElement* mInfo;

#define KEY_PRESSED(_key,_timeDelay, _macro) \
{ \
    if (mInputDevice->isKeyDown(_key) && timeDelay <= 0) \
    { \
		timeDelay = _timeDelay; \
        _macro ; \
    } \
}


// struct wich hold 2D tex.coords.
struct sTexCoord
{
	Real s;
	Real t;
};
// this struct will hold data needed to calculate the
// tangent space basis for a general polygon
struct s4TangentSpace
{
	// position of the vertex (x, y, z)
	Vector3 position;
	// tex.coords for vertex (u, v or s, t)
	sTexCoord texCoords;
};

// This function returns a 3D tex.coord.buffer from given VertexData.
// If such buffer didn't exist, it creates it.
HardwareVertexBufferSharedPtr get3DTexCoordBuffer(VertexData *vertexData)
{
	VertexDeclaration *vDecl = vertexData->vertexDeclaration ;
	VertexBufferBinding *vBind = vertexData->vertexBufferBinding ;

	const VertexElement *tex2D = vDecl->findElementBySemantic(VES_TEXTURE_COORDINATES, 0);
	if (!tex2D) {
		Except(Exception::ERR_INVALIDPARAMS, "Geometry data must have at least 1 2Dtex.coord.buffer !!!", "createNew3DTexCoordBuffer");
	}
	const VertexElement *tex3D = vDecl->findElementBySemantic(VES_TEXTURE_COORDINATES, 1);
	bool needsToBeCreated = false;
	
	if (!tex3D) { // no tex coords with index 1
			needsToBeCreated = true ;
	} else if (tex3D->getType() != VET_FLOAT3) { // no 3d-coords tex buffer
		vDecl->removeElement(VES_TEXTURE_COORDINATES, 1);
		vBind->unsetBinding(tex3D->getSource());
		needsToBeCreated = true ;
	}
	
	HardwareVertexBufferSharedPtr tex3DBuf ;
	if (needsToBeCreated) {
		tex3DBuf = HardwareBufferManager::getSingleton().createVertexBuffer(
			3*sizeof(float), vertexData->vertexCount,
			HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, 
			true );
		int source = vBind->getNextIndex()+1; // find next available source 
		vBind->setBinding(source, tex3DBuf);
		vDecl->addElement(source, 0, VET_FLOAT3, VES_TEXTURE_COORDINATES, 1);
	} else {
		tex3DBuf = vBind->getBuffer(tex3D->getSource());
	}
	
	return tex3DBuf ;
}

//This function will calculate the tangent space for a general polygon.
//You need to pass it three vertices of your polygon and three Vector3's in which to store 
//the resulting normal, s tangent and t tangent.
//If you want to find the tangent space basis for a single polygon, 
//you can call this function, passing in the first three vertices of the polygon, 
//and then assign the resulting tangents and normal to each vertex of the polygon.
void calculateTSB(const s4TangentSpace &v0, 
				  const s4TangentSpace &v1, 
				  const s4TangentSpace &v2,
				  Vector3 &normal, 
				  Vector3 &tangent, 
				  Vector3 &binormal)
{
	//side0 is the vector along one side of the triangle of vertices passed in, 
	//and side1 is the vector along another side. Taking the cross product of these returns the normal.
	Vector3 side0 = v0.position - v1.position;
	Vector3 side1 = v2.position - v1.position;
	//Calculate normal
	normal = side1.crossProduct(side0);
	normal.normalise();
	//Now we use a formula to calculate the s tangent. 
	//We then use the same formula for the t tangent.
	Real deltaT0 = v0.texCoords.t - v1.texCoords.t;
	Real deltaT1 = v2.texCoords.t - v1.texCoords.t;
	tangent = deltaT1 * side0 - deltaT0 * side1;
	tangent.normalise();
	//Calculate t tangent
	Real deltaS0 = v0.texCoords.s - v1.texCoords.s;
	Real deltaS1 = v2.texCoords.s - v1.texCoords.s;
	binormal = deltaS1 * side0 - deltaS0 * side1;
	binormal.normalise();
	//Now, we take the cross product of the tangents to get a vector which 
	//should point in the same direction as our normal calculated above. 
	//If it points in the opposite direction (the dot product between the normals is less than zero), 
	//then we need to reverse the s and t tangents. 
	//This is because the triangle has been mirrored when going from tangent space to object space.
	//reverse tangents if necessary
	Vector3 tangentCross = tangent.crossProduct(binormal);
	if (tangentCross.dotProduct(normal) < 0.0f)
	{
		tangent = -tangent;
		binormal = -binormal;
	}
}
// this method creates a new 3D tex coord buffer in the shared/submesh geometry for a given entity
// and creates a new 3D one at index 1 and fills it with tangent space basis vectors
void createTangentSpaceTextureCoords(Entity *pEnt)
{
	assert(pEnt);
	// our temp. buffers
	unsigned short	vertInd[3];
	s4TangentSpace	tvs[3];
	Vector3			lightVecs[3];
	Vector3			tslPollyVerts[3];
	Vector3			normal, tangent, binormal;
	Vector3			tVec;
	// get the mesh
	Mesh *pMesh = pEnt->getMesh();
	// setup a new 3D texture coord-set buffer for every sub mesh
	int nSubMesh = pMesh->getNumSubMeshes();
	for (int sm = 0; sm < nSubMesh; sm++)
	{
		// retrieve buffer pointers
		unsigned short	*pVIndices;	// the face indices buffer, read only
		Real			*p2DTC;		// pointer to 2D tex.coords, read only
		Real			*p3DTC;		// pointer to 3D tex.coords, write/read (discard)
		Real			*pVPos;		// vertex position buffer, read only

		SubMesh *pSubMesh = pMesh->getSubMesh(sm);

		// retrieve buffer pointers
		// first, indices
		IndexData *indexData = pSubMesh->indexData;
		HardwareIndexBufferSharedPtr buffIndex = indexData->indexBuffer ;
		pVIndices = (unsigned short*) buffIndex->lock(HardwareBuffer::HBL_READ_ONLY); // ***LOCK***
		// then, vertices
		VertexData *usedVertexData ;
		if (pSubMesh->useSharedVertices) {
			usedVertexData = pMesh->sharedVertexData;
		} else {
			usedVertexData = pSubMesh->vertexData;
		}
		VertexDeclaration *vDecl = usedVertexData->vertexDeclaration;
		VertexBufferBinding *vBind = usedVertexData->vertexBufferBinding;
		// get a new 3D tex.coord.buffer or an existing one
		HardwareVertexBufferSharedPtr buff3DTC = get3DTexCoordBuffer(usedVertexData);
		// clear it
		p3DTC = (Real*) buff3DTC->lock(HardwareBuffer::HBL_DISCARD); // ***LOCK***
		memset(p3DTC,0,buff3DTC->getSizeInBytes());
		// find a 2D tex coord buffer
		const VertexElement *elem2DTC = vDecl->findElementBySemantic(VES_TEXTURE_COORDINATES, 0);
		HardwareVertexBufferSharedPtr buff2DTC = vBind->getBuffer(elem2DTC->getSource());
		p2DTC = (Real*) buff2DTC->lock(HardwareBuffer::HBL_READ_ONLY); // ***LOCK***
		// find a vertex coord buffer
		const VertexElement *elemVPos = vDecl->findElementBySemantic(VES_POSITION);
		HardwareVertexBufferSharedPtr buffVPos = vBind->getBuffer(elemVPos->getSource());
		pVPos = (Real*) buffVPos->lock(HardwareBuffer::HBL_READ_ONLY); // ***LOCK***
		
		size_t numFaces = indexData->indexCount / 3 ;
		
		// loop through all faces to calculate the tangents and normals
		size_t n;
		for (n = 0; n < numFaces; ++n)
		{
			int i;
			for (i = 0; i < 3; ++i)
			{
				// get indexes of vertices that form a polygon in the position buffer
				vertInd[i] = pVIndices[n * 3 + i];
				// get the vertices positions from the position buffer
				tvs[i].position.x = pVPos[3 * vertInd[i] + 0];
				tvs[i].position.y = pVPos[3 * vertInd[i] + 1];
				tvs[i].position.z = pVPos[3 * vertInd[i] + 2];
				// get the vertices tex.coords from the 2D tex.coords buffer
				tvs[i].texCoords.s = p2DTC[2 * vertInd[i] + 0];
				tvs[i].texCoords.t = p2DTC[2 * vertInd[i] + 1];
			}
			// calculate the TSB
			calculateTSB(tvs[0], tvs[1], tvs[2], normal, tangent, binormal);
			// write new tex.coords 
            // note we only write the tangent, not the binormal since we can calculate
            // the binormal in the vertex program
			for (i = 0; i < 3; ++i)
			{
				// get indexes of vertices that form a pllygon in the position buffer
				vertInd[i] = pVIndices[n * 3 + i];
				// write values (they must be 0 and we must add them so we can average
                // all the contributions from all the faces
				p3DTC[3 * vertInd[i] + 0] += tangent.x;
				p3DTC[3 * vertInd[i] + 1] += tangent.y;
				p3DTC[3 * vertInd[i] + 2] += tangent.z;
			}
		}
		// now loop through all vertices and normalize them
		size_t numVerts = usedVertexData->vertexCount ;
		for (n = 0; n < numVerts * 3; n += 3)
		{
			// read the vertex
			tVec.x = p3DTC[n + 0];
			tVec.y = p3DTC[n + 1];
			tVec.z = p3DTC[n + 2];
			// normalize the vertex
			tVec.normalise();
			// write it back
			p3DTC[n + 0] = tVec.x;
			p3DTC[n + 1] = tVec.y;
			p3DTC[n + 2] = tVec.z;
		}
		// unlock buffers
		buffIndex->unlock();
		buff3DTC->unlock();
		buff2DTC->unlock();
		buffVPos->unlock();
	}
}

// Event handler to add ability to change material
class Dp3_Listener : public ExampleFrameListener
{
public:
    Dp3_Listener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam)
    {
    }

    void flipLightState(size_t i)
    {
        mLightState[i] = !mLightState[i];
        mLights[i]->setVisible(mLightState[i]);
        mLightFlareSets[i]->setVisible(mLightState[i]);
    }
    bool frameStarted(const FrameEvent& evt)
    {
        if(!ExampleFrameListener::frameStarted(evt))
            return false;
        
        static Real timeDelay = 0;

        timeDelay -= evt.timeSinceLastFrame;

		// switch meshes
        KEY_PRESSED(KC_O, 1, 
            mEntities[mCurrentEntity]->setVisible(false); 
            mCurrentEntity = (++mCurrentEntity) % NUM_ENTITIES; 
            mEntities[mCurrentEntity]->setVisible(true);
            mEntities[mCurrentEntity]->setMaterialName(mMaterialNames[mCurrentMaterial]);
            mObjectInfo->setCaption("Current: " + mEntityMeshes[mCurrentEntity]);
        );

		// switch materials
		KEY_PRESSED(KC_M, 1, 
            mCurrentMaterial = (++mCurrentMaterial) % NUM_MATERIALS; 
            mEntities[mCurrentEntity]->setMaterialName(mMaterialNames[mCurrentMaterial]);
            mMaterialInfo->setCaption("Current: " + mMaterialNames[mCurrentMaterial]);
        );

		// enable / disable lights
		KEY_PRESSED(KC_1, 1, flipLightState(0));
		// switch materials
		KEY_PRESSED(KC_2, 1, flipLightState(1));
		// switch materials
		KEY_PRESSED(KC_3, 1, flipLightState(2));

        // animate the lights
        for (size_t i = 0; i < NUM_LIGHTS; ++i)
            mLightPivots[i]->rotate(Vector3::UNIT_Z, mLightSpeeds[i] * evt.timeSinceLastFrame);

		return true;
    }

};

class Dp3_Application : public ExampleApplication
{
public:
    Dp3_Application() {}
	
protected:
	SceneNode *mpObjsNode; // the node wich will hold our entities

	void createScene(void)
    {
        // First check that vertex programs and dot3 or fragment programs are supported
		const RenderSystemCapabilities* caps = Root::getSingleton().getRenderSystem()->getCapabilities();
        if (!caps->hasCapability(RSC_VERTEX_PROGRAM))
        {
            Except(1, "Your card does not support vertex programs, so cannot "
                "run this demo. Sorry!", 
                "Dot3Bump::createScene");
        }
        if (!(caps->hasCapability(RSC_FRAGMENT_PROGRAM) 
			|| caps->hasCapability(RSC_DOT3)) )
        {
            Except(1, "Your card does not support dot3 blending or fragment programs, so cannot "
                "run this demo. Sorry!", 
                "Dot3Bump::createScene");
        }

        // Set ambient light and fog
        mSceneMgr->setAmbientLight(ColourValue(0.0, 0.0, 0.0));
        /*
		// Define a floor plane mesh
        Plane p;
        p.normal = Vector3::UNIT_Y;
        p.d = 200;
        MeshManager::getSingleton().createPlane("FloorPlane",p,2000,2000,1,1,true,1,5,5,Vector3::UNIT_Z);
        // Create an entity (the floor)
        Entity *floorEnt = mSceneMgr->createEntity("floor", "FloorPlane");
        floorEnt->setMaterialName("Examples/DP3Terrain");
        mSceneMgr->getRootSceneNode()->attachObject(floorEnt);
        */

        mMainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

        // Load the meshes with non-default HBU options
		for(int mn = 0; mn < NUM_ENTITIES; mn++) {
			MeshManager::getSingleton().load(mEntityMeshes[mn],
				HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, 
				HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
				true, true); //so we can still read it
            // Create entity
            mEntities[mn] = mSceneMgr->createEntity("Ent" + StringConverter::toString(mn), 
                mEntityMeshes[mn]);
            // Calc tangents
    		createTangentSpaceTextureCoords(mEntities[mn]);
            // Attach to child of root node
    		mMainNode->attachObject(mEntities[mn]);
            // Make invisible, except for index 0
            if (mn == 0)
                mEntities[mn]->setMaterialName(mMaterialNames[mCurrentMaterial]);
            else
                mEntities[mn]->setVisible(false);
		}

        for (unsigned int i = 0; i < NUM_LIGHTS; ++i)
        {
            mLightPivots[i] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
            mLightPivots[i]->rotate(mLightRotationAxes[i], mLightRotationAngles[i]);
            // Create a light, use default parameters
            mLights[i] = mSceneMgr->createLight("Light" + StringConverter::toString(i));
			mLights[i]->setPosition(mLightPositions[i]);
			mLights[i]->setDiffuseColour(mDiffuseLightColours[i]);
			mLights[i]->setSpecularColour(mSpecularLightColours[i]);
			mLights[i]->setVisible(mLightState[i]);
            // Attach light
            mLightPivots[i]->attachObject(mLights[i]);
			// Create billboard for light
			mLightFlareSets[i] = mSceneMgr->createBillboardSet("Flare" + StringConverter::toString(i));
			mLightFlareSets[i]->setMaterialName("Examples/Flare");
			mLightPivots[i]->attachObject(mLightFlareSets[i]);
			mLightFlares[i] = mLightFlareSets[i]->createBillboard(mLightPositions[i]);
			mLightFlares[i]->setColour(mDiffuseLightColours[i]);
			mLightFlareSets[i]->setVisible(mLightState[i]);
        }
        // move the camera a bit right and make it look at the knot
		mCamera->moveRelative(Vector3(50, 0, 20));
		mCamera->lookAt(0, 0, 0);
		// show overlay
		Overlay *pOver = (Overlay *)OverlayManager::getSingleton().getByName("Example/DP3Overlay");    
        mObjectInfo = GuiManager::getSingleton().getGuiElement("Example/DP3/ObjectInfo");
        mMaterialInfo = GuiManager::getSingleton().getGuiElement("Example/DP3/MaterialInfo");
        mInfo = GuiManager::getSingleton().getGuiElement("Example/DP3/Info");

        mObjectInfo->setCaption("Current: " + mEntityMeshes[mCurrentEntity]);
        mMaterialInfo->setCaption("Current: " + mMaterialNames[mCurrentMaterial]);
        if (!caps->hasCapability(RSC_FRAGMENT_PROGRAM))
        {
            mInfo->setCaption("NOTE: Light colours and specular highlights are not supported by your card.");
        }
		pOver->show();
	}

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new Dp3_Listener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);
    }
};

#if OGRE_PLATFORM == PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
    Dp3_Application app;

    try {
        app.go();
    } catch( Exception& e ) {
#if OGRE_PLATFORM == PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        fprintf(stderr, "An exception has occured: %s\n",
                e.getFullDescription().c_str());
#endif
    }

    return 0;
}
