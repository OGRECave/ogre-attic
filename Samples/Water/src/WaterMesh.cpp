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
	int i,x,y,b; // I prefer to initialize for() variables inside it, but VC doesn't like it ;(
	
	this->meshName = meshName ;
	this->complexity =  complexity ;
	numFaces = 2 * complexity * complexity; 
	numVertices = (complexity + 1) * (complexity + 1) ;
	
	// initialize algorithm parameters
	PARAM_C = 0.3f ; // ripple speed 
	PARAM_D = 0.4f ; // distance
	PARAM_U = 0.05f ; // viscosity
	PARAM_T = 0.13f ; // time
	useFakeNormals = false ;
	
	// allocate space for normal calculation
	vNormals = new Vector3[numVertices];
	
	// create mesh and submesh
	mesh= (Mesh*) MeshManager::getSingleton().createManual(meshName) ;
	subMesh = mesh->createSubMesh();
	subMesh->useSharedVertices=false;
	
	// Vertex buffers
	subMesh->vertexData = new VertexData();
	VertexDeclaration* vdecl = subMesh->vertexData->vertexDeclaration; 
	VertexBufferBinding* vbind = subMesh->vertexData->vertexBufferBinding;
	
	// Prepare texture coords buffer - static one
	// todo: optimize to write directly into buffer
	Real *texcoordsBufData = new Real[numVertices*2];
	for(y=0;y<=complexity;y++) {
		for(x=0;x<=complexity;x++) {
			texcoordsBufData[2*(y*(complexity+1)+x)+0] = (float)x / complexity ;
			texcoordsBufData[2*(y*(complexity+1)+x)+1] = 1.0f - ((float)y / (complexity)) ;
		}
	}
	texcoordsVertexBuffer = 
         HardwareBufferManager::getSingleton().createVertexBuffer( 
            2*sizeof(Real), 
			numVertices, 
			HardwareBuffer::HBU_STATIC_WRITE_ONLY); 
	texcoordsVertexBuffer->writeData(0, 
		texcoordsVertexBuffer->getSizeInBytes(),
		texcoordsBufData,
		true); // true?
	delete [] texcoordsBufData;
    vbind->setBinding(0, texcoordsVertexBuffer); 
	vdecl->addElement(0, 0, VET_FLOAT2, VES_TEXTURE_COORDINATES);
	
	// Prepare buffer for normals - write only
	normVertexBuffer = 
         HardwareBufferManager::getSingleton().createVertexBuffer( 
            3*sizeof(Real), 
			numVertices, 
			HardwareBuffer::HBU_WRITE_ONLY);
	vbind->setBinding(1, normVertexBuffer);
	vdecl->addElement(1, 0, VET_FLOAT3, VES_NORMAL);
	
	// Prepare buffer for positions - todo: first attempt, slow
	posVertexBuffer = 
         HardwareBufferManager::getSingleton().createVertexBuffer( 
            3*sizeof(Real), 
			numVertices, 
			HardwareBuffer::HBU_WRITE_ONLY);
	vbind->setBinding(2, posVertexBuffer);
	vdecl->addElement(2, 0, VET_FLOAT3, VES_POSITION);
	
	// Prepare buffer for indices
	indexBuffer = 
		HardwareBufferManager::getSingleton().createIndexBuffer(
			HardwareIndexBuffer::IT_16BIT, 
			3*numFaces, 
			HardwareBuffer::HBU_STATIC, true);
	unsigned short *faceVertexIndices = (unsigned short*) 
		indexBuffer->lock(0, numFaces*3*2, HardwareBuffer::HBL_DISCARD);
	for(y=0 ; y<complexity ; y++) {
		for(int x=0 ; x<complexity ; x++) {
			unsigned short *twoface = faceVertexIndices + (y*complexity+x)*2*3;
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
	indexBuffer->unlock();
	// Set index buffer for this submesh
	subMesh->indexData->indexBuffer = indexBuffer;
	subMesh->indexData->indexStart = 0;
	subMesh->indexData->indexCount = 3*numFaces;
	
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
	
	AxisAlignedBox meshBounds(0,0,0,
		planeSize,0, planeSize);
	mesh->_setBounds(meshBounds);
	
	currentBuffNumber = 0 ;
	posVertexBuffer->writeData(0, 
		posVertexBuffer->getSizeInBytes(), // size 
		vertexBuffers[currentBuffNumber], // source
		true); // discard?
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
Real WaterMesh::getHeight(Real x, Real y)
{
#define hat(_x,_y) buf[3*((int)_y*(complexity+1)+(int)(_x))]
	Real *buf = vertexBuffers[currentBuffNumber] ;
	Real xa = floor(x);
	Real xb = xa + 1 ;
	Real ya = floor(y);
	Real yb = ya + 1 ;
	Real *vertex = buf + 3*((int)(ya)*(complexity+1)+(int)(xa));
	Real yaxavg = hat(xa,ya) * (1.0f-fabs(xa-x)) + hat(xb,ya) * (1.0f-fabs(xb-x));
	Real ybxavg = hat(xa,yb) * (1.0f-fabs(xa-x)) + hat(xb,yb) * (1.0f-fabs(xb-x));
	Real yavg = yaxavg * (1.0f-fabs(ya-y)) + ybxavg * (1.0f-fabs(yb-y)) ;
	return yavg ;
}
/* ========================================================================= */
void WaterMesh::calculateFakeNormals()
{
	int x,y;
	Real *buf = vertexBuffers[currentBuffNumber] + 1;
	Real *pNormals = (Real*) normVertexBuffer->lock(
		0,normVertexBuffer->getSizeInBytes(), HardwareBuffer::HBL_DISCARD);
	for(y=1;y<complexity;y++) {
		Real *nrow = pNormals + 3*y*(complexity+1);
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
	normVertexBuffer->unlock();
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
	unsigned short* vinds = (unsigned short*) indexBuffer->lock(
		0, indexBuffer->getSizeInBytes(), HardwareBuffer::HBL_READ_ONLY);
	Real *pNormals = (Real*) normVertexBuffer->lock(
		0, normVertexBuffer->getSizeInBytes(), HardwareBuffer::HBL_DISCARD);
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
	for(y=0;y<=complexity;y++) {
		for(x=0;x<=complexity;x++) {
			int numPoint = y*(complexity+1) + x ;
			Vector3 n = vNormals[numPoint] ;
			n.normalise() ;
			Real* normal = pNormals + 3*numPoint ;
			normal[0]=n.x;
			normal[1]=n.y;
			normal[2]=n.z;
		}
	}
	indexBuffer->unlock();
	normVertexBuffer->unlock();
}
/* ========================================================================= */
void WaterMesh::updateMesh(Real timeSinceLastFrame)
{
	int x, y ;
	
	// switch buffer numbers
	currentBuffNumber = (currentBuffNumber + 1) % 3 ;
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

	if (useFakeNormals) {
		calculateFakeNormals();
	} else {
		calculateNormals();
	}

	// set vertex buffer
	posVertexBuffer->writeData(0, 
		posVertexBuffer->getSizeInBytes(), // size 
		vertexBuffers[currentBuffNumber], // source
		true); // discard?
}
