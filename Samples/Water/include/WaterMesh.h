#ifndef _WATER_MESH_H_
#define _WATER_MESH_H_

#include "Ogre.h"
#include "OgreMesh.h"

using namespace Ogre ;

class WaterMesh
{
private:
	Mesh *mesh ;
	SubMesh *subMesh ; 
	Real *vertexBuffers[3] ; // we need 3 vertex buffers
	int currentBuffNumber ;
	int complexity ;
	String meshName ;
	int numFaces ;
	int numVertices ;
	Vector3* vNormals ;
	
	void calculateFakeNormals();
	void calculateNormals();
public:
	WaterMesh(const String& meshName, Real planeSize, int complexity) ;

	/** "pushes" a mesh at position [x,y]. Note, that x,y are float, hence 
	*	4 vertices are actually pushed
	*	@note 
	*		This should be replaced by push with 'radius' parameter to simulate
	*  		big objects falling into water
	*/
	void push(Real x, Real y, Real depth, bool absolute=false) ;

	/** updates mesh */
	void updateMesh(Real timeSinceLastFrame) ;
	
	Real PARAM_C ; // ripple speed 
	Real PARAM_D ; // distance
	Real PARAM_U ; // viscosity
	Real PARAM_T ; // time
	bool useFakeNormals ;
} ;

#endif
