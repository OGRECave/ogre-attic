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

// 4 entities we'll use
Entity *mEnt1, *mEnt2, *mEnt3, *mEnt4;
// the light
Light *mLight;
// the scene node of the entity
SceneNode *mMainNode;
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
				  Vector3 &sTangent, 
				  Vector3 &tTangent)
{
	//side0 is the vector along one side of the triangle of vertices passed in, 
	//and side1 is the vector along another side. Taking the cross product of these returns the normal.
	Vector3 side0 = v0.position - v1.position;
	Vector3 side1 = v2.position - v1.position;
	//Calculate normal
	normal = side1.crossProduct(side0);
	normal = normal.normalisedCopy();
	//Now we use a formula to calculate the s tangent. 
	//We then use the same formula for the t tangent.
	Real deltaT0 = v0.texCoords.t - v1.texCoords.t;
	Real deltaT1 = v2.texCoords.t - v1.texCoords.t;
	sTangent = deltaT1 * side0 - deltaT0 * side1;
	sTangent = sTangent.normalisedCopy();
	//Calculate t tangent
	Real deltaS0 = v0.texCoords.s - v1.texCoords.s;
	Real deltaS1 = v2.texCoords.s - v1.texCoords.s;
	tTangent = deltaS1 * side0 - deltaS0 * side1;
	tTangent = tTangent.normalisedCopy();
	//Now, we take the cross product of the tangents to get a vector which 
	//should point in the same direction as our normal calculated above. 
	//If it points in the opposite direction (the dot product between the normals is less than zero), 
	//then we need to reverse the s and t tangents. 
	//This is because the triangle has been mirrored when going from tangent space to object space.
	//reverse tangents if necessary
	Vector3 tangentCross = sTangent.crossProduct(tTangent);
	if (tangentCross.dotProduct(normal) < 0.0f)
	{
		sTangent = -sTangent;
		tTangent = -tTangent;
	}
}
// this method store the tangent space light vector in the 
// given polygon buffer (buffer with room for 3 vectors)
void fillPollygonWithTSLVectors(const Vector3 lightVecs[3], 
								const Vector3 &sTangent, 
								const Vector3 &tTangent, 
								const Vector3 &normal, 
								Vector3 pollyVerts[3])
{
	// vertex 1
	pollyVerts[0].x = sTangent.dotProduct(lightVecs[0]);
	pollyVerts[0].y = tTangent.dotProduct(lightVecs[0]);
	pollyVerts[0].z =   normal.dotProduct(lightVecs[0]);
	// vertex 2
	pollyVerts[1].x = sTangent.dotProduct(lightVecs[1]);
	pollyVerts[1].y = tTangent.dotProduct(lightVecs[1]);
	pollyVerts[1].z =   normal.dotProduct(lightVecs[1]);
	// vertex 3
	pollyVerts[2].x = sTangent.dotProduct(lightVecs[2]);
	pollyVerts[2].y = tTangent.dotProduct(lightVecs[2]);
	pollyVerts[2].z =   normal.dotProduct(lightVecs[2]);
}
// this method creates a new 3D tex coord buffer in the shared/submesh geometry for a given entity
// and creates a new 3D one at index 1 and fills it with tangent space light vectors
void create3DTexCoordsFromTSLVector(Entity *pEnt, Vector3 objectLightPositionVec)
{
	assert(pEnt);
	// our temp. buffers
	unsigned short	vertInd[3];
	s4TangentSpace	tvs[3];
	Vector3			lightVecs[3];
	Vector3			tslPollyVerts[3];
	Vector3			normal, sTangent, tTangent;
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
				// get indexes of vertices that form a pllygon in the position buffer
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
			calculateTSB(tvs[0], tvs[1], tvs[2], normal, sTangent, tTangent);
			// calculate the light vector for every vertex
			lightVecs[0] = objectLightPositionVec - tvs[0].position;
			lightVecs[1] = objectLightPositionVec - tvs[1].position;
			lightVecs[2] = objectLightPositionVec - tvs[2].position;
			// store the tangent space light vector in tslPollyVerts
			fillPollygonWithTSLVectors(lightVecs, sTangent, tTangent, normal, tslPollyVerts);
			// write new tex.coords 
			for (i = 0; i < 3; ++i)
			{
				// get indexes of vertices that form a pllygon in the position buffer
				vertInd[i] = pVIndices[n * 3 + i];
				// write values (they must be 0 and we must add them)
				p3DTC[3 * vertInd[i] + 0] += tslPollyVerts[i].x;
				p3DTC[3 * vertInd[i] + 1] += tslPollyVerts[i].y;
				p3DTC[3 * vertInd[i] + 2] += tslPollyVerts[i].z;
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
			tVec = tVec.normalisedCopy();
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
// this method creates a new 3D tex coord buffer in the shared/submesh geometry for a given entity
// and creates a new 3D one at index 1 and fills it with normals if they are present...
/*
void create3DTexCoordsFromNormals(Entity *pEnt)
{
	assert(pEnt);
	Vector3 vec = Vector3::ZERO;
	Mesh *pMesh = pEnt->getMesh();

	// setup a new 3D texture coord-set buffer 
	// for the shared geometry if present
	if (pMesh->sharedGeometry.numVertices)
	{
		// must have normals
		if (!pMesh->sharedGeometry.hasNormals)
			Except(Exception::ERR_INVALIDPARAMS, "'" + pMesh->getName() + "' should have normals", "create3DTexCoordsFromNormals");
		// create a new 3D tex.coord.buffer
		createNew3DTexCoordBuffer(pMesh->sharedGeometry);
		// copy normals to new tex.coord. buffer
		memcpy(	pMesh->sharedGeometry.pTexCoords[1], 
				pMesh->sharedGeometry.pNormals, 
				pMesh->sharedGeometry.numVertices * 3 * sizeof(Real));
	}

	// setup a new 3D texture coord-set buffer for every sub mesh
	for (int sm = 0; sm < pMesh->getNumSubMeshes(); sm++)
	{
		SubMesh *pSubMesh = pMesh->getSubMesh(sm);
		// must have vertices
		if (!pSubMesh->geometry.numVertices)
			break;
		// must have normals
		if (!pSubMesh->geometry.hasNormals)
			Except(Exception::ERR_INVALIDPARAMS, "SubMesh from '" + pMesh->getName() + "' should have normals", "create3DTexCoordsFromNormals");
		// create a new 3D tex.coord.buffer
		createNew3DTexCoordBuffer(pSubMesh->geometry);
		// copy normals to new tex.coord. buffer
		memcpy(	pSubMesh->geometry.pTexCoords[1], 
				pSubMesh->geometry.pNormals, 
				pSubMesh->geometry.numVertices * 3 * sizeof(Real));
	}
} // create3DTexCoordsFromNormals
*/

// Event handler to add ability to change material
class Dp3_Listener : public ExampleFrameListener
{
public:
    Dp3_Listener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam)
    {
    }

    bool frameStarted(const FrameEvent& evt)
    {
        if(!ExampleFrameListener::frameStarted(evt))
            return false;
        
		static Entity *pEnt = mEnt1;
		// switch meshes
		if (mInputDevice->isKeyDown(KC_F5))
			pEnt = mEnt1;
        if (mInputDevice->isKeyDown(KC_F6))
			pEnt = mEnt2;
        if (mInputDevice->isKeyDown(KC_F7))
			pEnt = mEnt3;
        if (mInputDevice->isKeyDown(KC_F8))
			pEnt = mEnt4;
		// set visible entity
		mEnt1->setVisible(pEnt == mEnt1 ? true : false);
		mEnt2->setVisible(pEnt == mEnt2 ? true : false);
		mEnt3->setVisible(pEnt == mEnt3 ? true : false);
		mEnt4->setVisible(pEnt == mEnt4 ? true : false);

		static String matName = "Examples/DP3Mat1";
		// switch materials
		if (mInputDevice->isKeyDown(KC_F1))
			matName = "Examples/DP3Mat1";
        if (mInputDevice->isKeyDown(KC_F2))
			matName = "Examples/DP3Mat2";
        if (mInputDevice->isKeyDown(KC_F3))
			matName = "Examples/DP3Mat3";
        if (mInputDevice->isKeyDown(KC_F4))
			matName = "Examples/DP3Mat4";
		// set material
		pEnt->setMaterialName(matName);

		// update the light position, the light is 'projected' 
		// and follows the camera in this demo !!!
		mLight->setPosition(mCamera->getPosition());
		// animate the main mesh node
		mMainNode->rotate(Vector3::UNIT_Y, 0.5f);
		// calculate the light position in object space
		mMainNode->getWorldTransforms(&mInvModelMatrix);
		mInvModelMatrix = mInvModelMatrix.inverse();
		mObjLightPosVec = mInvModelMatrix * mLight->getPosition();
		// create a new 3D tex.coord.buffer and fill it with TSLVectors
		// calculated from the light position in object space
		create3DTexCoordsFromTSLVector(pEnt, mObjLightPosVec);

		return true;
    }

private:
	Vector3 mObjLightPosVec;
	Matrix4 mInvModelMatrix;
};

class Dp3_Application : public ExampleApplication
{
public:
    Dp3_Application() {}
	
protected:
	SceneNode *mpObjsNode; // the node wich will hold our entities

	void createScene(void)
    {
        // First check that dot3 is supported
        if (!Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_DOT3))
        {
            Except(1, "Your card does not support the DOT3 blending operation, so cannot "
                "run this demo. Sorry!", 
                "Dot3Bump::createScene");
        }

		// Set default filtering/anisotropy
		MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
        // Set ambient light and fog
        mSceneMgr->setAmbientLight(ColourValue(1, 0.2, 0.2));
        mSceneMgr->setFog(FOG_EXP, ColourValue::White, 0.0002);
        // Create a skydome
        mSceneMgr->setSkyDome(true, "Examples/DP3Sky", 5, 8);
        // Create a light, use default parameters
        mLight = mSceneMgr->createLight("MainLight");
		mLight->setDiffuseColour(ColourValue(0, 1, 0));
		// Define a floor plane mesh
        Plane p;
        p.normal = Vector3::UNIT_Y;
        p.d = 200;
        MeshManager::getSingleton().createPlane("FloorPlane",p,2000,2000,1,1,true,1,5,5,Vector3::UNIT_Z);
        // Create an entity (the floor)
        Entity *floorEnt = mSceneMgr->createEntity("floor", "FloorPlane");
        floorEnt->setMaterialName("Examples/DP3Terrain");
        mSceneMgr->getRootSceneNode()->attachObject(floorEnt);
		// Load the meshes with non-default HBU options
		const char *meshNames[4]={ "knot.mesh", "cube.mesh", "ogrehead.mesh", "ball.mesh" } ;
		for(int mn=0;mn<4;mn++) {
			MeshManager::getSingleton().load(meshNames[mn],
				HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, 
				HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
				true, true); //so we can still read it
		}
        // Create the mesh(es) wich will be bump mapped
		mEnt1 = mSceneMgr->createEntity("knot", "knot.mesh");
		mEnt2 = mSceneMgr->createEntity("cube", "cube.mesh");
		mEnt3 = mSceneMgr->createEntity("head", "ogrehead.mesh");
		mEnt4 = mSceneMgr->createEntity("ball", "ball.mesh");
        // Attach to child of root node
        mMainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		mMainNode->attachObject(mEnt1);
		mMainNode->attachObject(mEnt2);
		mMainNode->attachObject(mEnt3);
		mMainNode->attachObject(mEnt4);
		// move the camera a bit right and make it look at the knot
		mCamera->moveRelative(Vector3(50, 0, 20));
		mCamera->lookAt(0, 0, 0);
		// show overlay
		Overlay *pOver = (Overlay *)OverlayManager::getSingleton().getByName("Example/DP3Overlay");    
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
