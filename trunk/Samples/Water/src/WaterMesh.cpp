/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourgeforge.net/

Copyright © 2000-2003 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/


#include "WaterMesh.h"

WaterMesh::WaterMesh(const String& meshName, Real planeSize, int complexity)
{
	int x,y,b; // I prefer to initialize for() variables inside it, but VC doesn't like it ;(
	
	this->meshName = meshName ;
	this->complexity =  complexity ;
	numFaces = 2 * complexity * complexity; 
	numVertices = (complexity + 1) * (complexity + 1) ;
	
	// initialize algorithm parameters
	PARAM_C = 0.3f ; // ripple speed 
	PARAM_D = 0.4f ; // distance
	PARAM_U = 0.05f ; // viscosity
	PARAM_T = 0.1f ; // time
	useFakeNormals = false ;
	
	// allocate space for normal calculation
	vNormals = new Vector3[numVertices];
	
	// create mesh and submesh
	mesh= (Mesh*) MeshManager::getSingleton().createManual(meshName) ;
	subMesh = mesh->createSubMesh();
	subMesh->useSharedVertices=false;
	subMesh->useTriStrips=false;
	subMesh->geometry.hasColours=false;
	subMesh->geometry.hasNormals=true;
	subMesh->geometry.numTexCoords = 1 ;
	subMesh->geometry.numTexCoordDimensions[0]=2;
	subMesh->geometry.texCoordStride[0]=0;
	subMesh->geometry.normalStride=0;
	subMesh->numFaces = numFaces ;
	subMesh->faceVertexIndices = new unsigned short[numFaces*3];
	subMesh->geometry.numVertices = numVertices ;
	subMesh->geometry.pTexCoords[0] = new Real[numVertices * 2];
	subMesh->geometry.pNormals = new Real[numVertices * 3] ;
	// prepare faces
	for(y=0 ; y<complexity ; y++) {
		for(int x=0 ; x<complexity ; x++) {
			unsigned short *twoface = subMesh->faceVertexIndices + (y*complexity+x)*2*3;
			int p0 = y*(complexity+1) + x ;
			int p1 = y*(complexity+1) + x + 1 ;
			int p2 = (y+1)*(complexity+1) + x ;
			int p3 = (y+1)*(complexity+1) + x + 1 ;
			twoface[0]=p2; //first tri
			twoface[1]=p1;
			twoface[2]=p0;
			twoface[3]=p2; //second tri
			twoface[4]=p3;
			twoface[5]=p1;
		}
	}

	/*	prepare normals and texture coordinates */
	for(y=0;y<=complexity;y++) {
		for(x=0;x<=complexity;x++) {
			int numPoint = y*(complexity+1) + x ;
			Real* normal = subMesh->geometry.pNormals + 3*numPoint ;
			normal[0]=0;
			normal[1]=1;
			normal[2]=0;
			Real* texCoord = subMesh->geometry.pTexCoords[0] + 2*numPoint ;
			texCoord[0] = (float)x / complexity ;
			texCoord[1] = 1.0f - ((float)y / (complexity)) ;
		}
	}

	/*	prepare vertex positions
	 *	note - we use 3 vertex buffers, since algorighm uses two last phases 
	 *	to calculate the next one
	 */
	for(b=0;b<3;b++) {
		vertexBuffers[b] = new Real[numVertices * 3] ;
		for(y=0;y<=complexity;y++) {
			for(x=0;x<=complexity;x++) {
				int numPoint = y*(complexity+1) + x ;
				Real* vertex = vertexBuffers[b] + 3*numPoint ;
				vertex[0]=(float)(x) / (float)(complexity) * (float) planeSize ;
				vertex[1]= 0 ; // rand() % 30 ;
				vertex[2]=(float)(y) / (float)(complexity) * (float) planeSize ;
			}
		}
	}
	
	currentBuffNumber = 0 ;
	subMesh->geometry.pVertices = vertexBuffers[currentBuffNumber];
	mesh->_updateBounds(); 
}
/* ========================================================================= */
void WaterMesh::push(Real x, Real y, Real depth, bool absolute)
{
	Real *buf = vertexBuffers[currentBuffNumber]+1 ;
#define _PREP(addx,addy) { \
	Real *vertex=buf+3*((int)(y+addy)*(complexity+1)+(int)(x+addx)) ; \
	Real diffy = y - floor(y+addy); \
	Real diffx = x - floor(x+addx); \
	Real dist=sqrt(diffy*diffy + diffx*diffx) ; \
	Real power = 1 - dist ; \
	if (power<0)  \
		power = 0; \
	if (absolute) \
		*vertex = depth*power ;  \
	else \
		*vertex += depth*power ;  \
} /* #define */
	_PREP(0,0);
	_PREP(0,1);
	_PREP(1,0);
	_PREP(1,1);
#undef _PREP	
}
/* ========================================================================= */
void WaterMesh::calculateFakeNormals()
{
	int x,y;
	Real *buf = vertexBuffers[currentBuffNumber] + 1;
	for(y=1;y<complexity;y++) {
		Real *nrow = subMesh->geometry.pNormals + 3*y*(complexity+1);
		Real *row = buf + 3*y*(complexity+1) ;
		Real *rowup = buf + 3*(y-1)*(complexity+1) ;
		Real *rowdown = buf + 3*(y+1)*(complexity+1) ;
		for(x=1;x<complexity;x++) {
			Real xdiff = row[3*x+3] - row[3*x-3] ;
			Real ydiff = rowup[3*x] - rowdown[3*x-3] ;
			Vector3 norm(xdiff,30,ydiff);
			norm.normalise();
			nrow[3*x+0] = norm.x;
			nrow[3*x+1] = norm.y;
			nrow[3*x+2] = norm.z;
		}
	}
}
/* ========================================================================= */
void WaterMesh::calculateNormals()
{
	int i,x,y;
	Real *buf = vertexBuffers[currentBuffNumber] + 1;
	// zero normals
	for(i=0;i<numVertices;i++) {
		vNormals[i] = Vector3::ZERO;
	}
	// first, calculate normals for faces, add them to proper vertices
	buf = vertexBuffers[currentBuffNumber] ;
	unsigned short* vinds = subMesh->faceVertexIndices ;
	for(i=0;i<numFaces;i++) {
		int p0 = vinds[3*i] ;
		int p1 = vinds[3*i+1] ;
		int p2 = vinds[3*i+2] ;
		Vector3 v0(buf[3*p0], buf[3*p0+1], buf[3*p0+2]);
		Vector3 v1(buf[3*p1], buf[3*p1+1], buf[3*p1+2]);
		Vector3 v2(buf[3*p2], buf[3*p2+1], buf[3*p2+2]);
		Vector3 diff1 = v2 - v1 ;
		Vector3 diff2 = v0 - v1 ;
		Vector3 fn = diff1.crossProduct(diff2);
		vNormals[p0] += fn ;
		vNormals[p1] += fn ;
		vNormals[p2] += fn ;
	}
	// now normalize vertex normals
	for(y=1;y<complexity;y++) {
		for(x=1;x<complexity;x++) {
			int numPoint = y*(complexity+1) + x ;
			Vector3 n = vNormals[numPoint] ;
			n.normalise() ;
			Real* normal = subMesh->geometry.pNormals + 3*numPoint ;
			normal[0]=n.x;
			normal[1]=n.y;
			normal[2]=n.z;
		}
	}
}
/* ========================================================================= */
void WaterMesh::updateMesh(Real timeSinceLastFrame)
{
	int x, y ;
	
	// switch buffers
	currentBuffNumber = (currentBuffNumber + 1) % 3 ;
	subMesh->geometry.pVertices = vertexBuffers[currentBuffNumber];
	Real *buf = vertexBuffers[currentBuffNumber] + 1 ; // +1 for Y coordinate
	Real *buf1 = vertexBuffers[(currentBuffNumber+2)%3] + 1 ; 
	Real *buf2 = vertexBuffers[(currentBuffNumber+1)%3] + 1; 	

	/* we use an algorithm from
	 * http://collective.valve-erc.com/index.php?go=water_simulation
	 * The params could be dynamically changed every frame ofcourse
	 */
	double C = PARAM_C; // ripple speed 
	double D = PARAM_D; // distance
	double U = PARAM_U; // viscosity
	double T = PARAM_T; // time
	Real TERM1 = ( 4.0f - 8.0f*C*C*T*T/(D*D) ) / (U*T+2) ;
	Real TERM2 = ( U*T-2.0f ) / (U*T+2.0f) ;
	Real TERM3 = ( 2.0f * C*C*T*T/(D*D) ) / (U*T+2) ;
	for(y=1;y<complexity;y++) { // don't do anything with border values
		Real *row = buf + 3*y*(complexity+1) ;
		Real *row1 = buf1 + 3*y*(complexity+1) ;
		Real *row1up = buf1 + 3*(y-1)*(complexity+1) ;
		Real *row1down = buf1 + 3*(y+1)*(complexity+1) ;
		Real *row2 = buf2 + 3*y*(complexity+1) ;
		for(x=1;x<complexity;x++) {
			row[3*x] = TERM1 * row1[3*x] 
				+ TERM2 * row2[3*x]
				+ TERM3 * ( row1[3*x-3] + row1[3*x+3] + row1up[3*x]+row1down[3*x] ) ;
		}
	}

	// we could update mesh bounds, but who cares...
	//~ mesh->_updateBounds(); 
	
	if (useFakeNormals) {
		calculateFakeNormals();
	} else {
		calculateNormals();
	}

	// set vertex buffer
	subMesh->geometry.pVertices = vertexBuffers[currentBuffNumber];
}
