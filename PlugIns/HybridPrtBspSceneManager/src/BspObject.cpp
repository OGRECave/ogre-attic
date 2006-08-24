/*
** This source file is part of OGRE (Object-oriented Graphics Rendering Engine)
** For the latest info, see http://www.ogre3d.org/
**
** OGRE Copyright goes to Ogre Team
** Hybrid Portal/BSP Scene Manager Copyright (c) 2006 Wael El Oraiby
** 
** This program is free software; you can redistribute it and/or modify it under
** the terms of the GNU Lesser General Public License as published by the Free Software
** Foundation; either version 2 of the License, or (at your option) any later
** version.
** 
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
** 
** You should have received a copy of the GNU Lesser General Public License along with
** this program; if not, write to the Free Software Foundation, Inc., 59 Temple
** Place - Suite 330, Boston, MA 02111-1307, USA, or go to
** http://www.gnu.org/copyleft/lesser.txt.
*/
#include "Ogre.h"
#include "BspObject.h"

using namespace std;
using namespace Ogre;

enum POLY_CLASS {
	POLY_INFRONT,
	POLY_ONBACK,
	POLY_ON,
	POLY_SPANNING
};

const static int		SOLID_AREA	= -999999;

POLY_CLASS				classifyPolyToPlane(Plane &cmpPlane, const BspPolygon &poly);
POLY_CLASS				classifyOnPolyToPlane(Plane &cmpPlane, const BspPolygon &poly);
void					splitPoly(Plane &splitter, const BspPolygon &poly, BspPolygon &front, BspPolygon &back);

// for maps when we have to find a plane
inline bool EpsilonEqual ( const Plane &p0, const Plane &p1 )
{
	return (fabs(p0.normal.x - p1.normal.x) <= HPBSP_EPSILON && fabs(p0.normal.y - p1.normal.y) <= HPBSP_EPSILON &&
		fabs(p0.normal.z - p1.normal.z) <= HPBSP_EPSILON && fabs(p0.d - p1.d) <= HPBSP_EPSILON);
}

/*
** classify a polygon to a plane
*/
POLY_CLASS classifyPolyToPlane(Plane &cmpPlane, const BspPolygon &poly)
{
	int num = static_cast<int>(poly.verts.size());
	int front = 0, back = 0;
	float res = 0;

	for (int cnt = 0; cnt < num; cnt++)
	{
		Vector3	v = poly.verts[cnt];
		// compute point distance to plane
		res = cmpPlane.normal.dotProduct(v) + cmpPlane.d;

		if (res > HPBSP_EPSILON)	// point in front of the plane
			front++;
		else if (res < -HPBSP_EPSILON) // on back of the plane
			back++;
	}

	if (!back && !front)
		return POLY_ON;
	else if (front && !back)
		return POLY_INFRONT;
	else if (back && !front)
		return POLY_ONBACK;

	return POLY_SPANNING;
}

/*
** classify an ON polygon to a plane
*/
POLY_CLASS classifyOnPolyToPlane(Plane &cmpPlane, const BspPolygon &poly)
{
	int num = static_cast<int>(poly.verts.size());
	int front = 0, back = 0;
	float res = 0;

	for (int cnt = 0; cnt < num; cnt++)
	{
		Vector3	v = poly.verts[cnt];
		// compute point distance to plane
		res = cmpPlane.normal.dotProduct(v + poly.p.normal) + cmpPlane.d;

		if (res > HPBSP_EPSILON)	// point in front of the plane
			front++;
		else if (res < -HPBSP_EPSILON) // on back of the plane
			back++;
	}

	if (!back && !front)
		return POLY_ON;
	else if (front && !back)
		return POLY_INFRONT;
	else if (back && !front)
		return POLY_ONBACK;

	return POLY_SPANNING;
}

/*
** split a polygon by a plane
*/
void splitPoly(Plane &splitter, const BspPolygon &poly, BspPolygon &front, BspPolygon &back)
{
	int		count = static_cast<int>(poly.verts.size());

	Vector3	ptA, ptB;
	float	sideA, sideB;

	ptA = poly.verts[count - 1];
	sideA = splitter.normal.dotProduct(ptA) + splitter.d;

	front.verts.resize(0);
	back.verts.resize(0);

	for (int i = 0; i < count; i++)
	{
		ptB = poly.verts[i];
		sideB = splitter.normal.dotProduct(ptB) + splitter.d;

		if (sideB > HPBSP_EPSILON )
		{
			if (sideA < -HPBSP_EPSILON)
			{
				Vector3 vt = ptB - ptA;
				float sect = -(splitter.normal.dotProduct(ptA) + splitter.d) / splitter.normal.dotProduct(vt);
				front.verts.push_back(ptA + (sect * vt));
				back.verts.push_back(ptA + (sect * vt));
			}
			front.verts.push_back(ptB);
		}
		else if (sideB < -HPBSP_EPSILON)
		{
			if (sideA > HPBSP_EPSILON)
			{
				Vector3 vt = ptB - ptA;
				float sect = -(splitter.normal.dotProduct(ptA) + splitter.d) / splitter.normal.dotProduct(vt);
				front.verts.push_back(ptA + (sect * vt));
				back.verts.push_back(ptA + (sect * vt));
			}
			back.verts.push_back(ptB);
		}
		else {
			front.verts.push_back(ptB);
			back.verts.push_back(ptB);
		}
		ptA = ptB;
		sideA = sideB;
	}

	front.p			= poly.p;
	front.cellId	= poly.cellId;
	front.portalId	= poly.portalId;

	back.p			= poly.p;
	back.cellId		= poly.cellId;
	back.portalId	= poly.portalId;
}

/*
** find best splitter
*/
int BspObject::findSplitter(const vector<BspPolygon> &polys, Plane &splitter)
{
	BspPolygon			cmp_face;	// comparaision face
	BspPolygon			curr;		// current face we are on
	int					best_id	= -1;	// start with an arbitrary plane
	int					best_score	= 999999;
	int					score;
	int					res, i, j, num_front, num_back, num_spanning, num_on;


	for ( i = 0; i < static_cast<int>(polys.size()); i++ )
	{
		cmp_face	= polys[i];

		num_front	= 0;
		num_back		= 0;
		num_spanning	= 0;
		num_on		= 0;

		// count front, back, spanning and on faces
		for ( j = 0; j < static_cast<int>(polys.size()); j++ )      
		{
			curr	= polys[j];

			if( i != j )
			{
				res = classifyPolyToPlane(cmp_face.p, curr);
				switch( res )
				{
				case POLY_INFRONT:
					num_front++;
					break;
				case POLY_ONBACK:
					num_back++;
					break;
				case POLY_ON:
					num_on++;
					break;
				case POLY_SPANNING:
					num_spanning++;
					break;
				}
			}
		}

		// estimate current plane score
		score = ((2 * num_spanning) + abs(num_front - num_back) + num_on);

		// update best plane if needed
		if (score != 0 && score < best_score) 
		{
			best_score	= score;
			splitter	= polys[i].p;
			best_id		= i;
		}
	}

	if ( best_id == -1 && polys.size() > 0 )
	{
		splitter	= polys[0].p;
		best_id	= 0;
	}


	return best_id;
}

/*
** build a bsp node
*/
void BspObject::buildBspNode(const vector<BspPolygon> &inPolys, int nodeId)
{
	Plane			splitter;
	BspPolygon		front, back;
	int				frontNodeId, backNodeId, i;
	BspNode			n;

	vector<BspPolygon>	onFaces;
	vector<BspPolygon>	frontFaces;
	vector<BspPolygon>	backFaces;

	// show progress
	cout << ".";

	// try to find a splitter
	int sp_id = findSplitter(inPolys, splitter);

	mNodes[nodeId].splitter	= splitter;

	for( i = 0; i < static_cast<int>(inPolys.size()); i++ )
	{
		BspPolygon	currPoly = inPolys[i];
		switch(classifyPolyToPlane(splitter, currPoly))
		{
		case POLY_INFRONT:
			frontFaces.push_back(currPoly);
			break;
		case POLY_ONBACK:
			backFaces.push_back(currPoly);
			break;
		case POLY_ON:
			if ( !EpsilonEqual(currPoly.p, splitter) )
                switch(classifyOnPolyToPlane(splitter, currPoly))
				{
				case POLY_INFRONT:
					frontFaces.push_back(currPoly);
					break;
				case POLY_ONBACK:
					backFaces.push_back(currPoly);
					break;
				case POLY_SPANNING:
					cout << "case is fucked!" << endl;
				}
			else onFaces.push_back(currPoly);
			break;
		case POLY_SPANNING:
			splitPoly(splitter, currPoly, front, back);
			if ( front.surface() > HPBSP_EPSILON ){
				frontFaces.push_back(front);
			} else	cout << endl << "degenerated" << endl;

			if ( back.surface() > HPBSP_EPSILON ){
				backFaces.push_back(back);
			} else cout << endl << "degenerated" << endl;
			break;
		}
	}

	if ( frontFaces.size() == 0 )
	{
		mNodes[nodeId].frontNode	= -onFaces[0].cellId;
	} else {
		frontNodeId	= static_cast<int>(mNodes.size());
		mNodes[nodeId].frontNode	= frontNodeId;
		mNodes.push_back(n);

/*		for ( size_t j = 0; j < onFaces.size(); j++ )
			if ( onFaces[j].p != splitter )
				frontFaces.push_back(onFaces[j]);
*/
		buildBspNode(frontFaces, frontNodeId);
	}

	if ( backFaces.size() == 0 )
	{
		mNodes[nodeId].backNode	= SOLID_AREA;
	} else {
		backNodeId	= static_cast<int>(mNodes.size());
		mNodes[nodeId].backNode	= backNodeId;
		mNodes.push_back(n);

		buildBspNode(backFaces, backNodeId);
	}
}

/*
** get the point area in a node
*/
int BspObject::getPointCell(const Vector3 &v, int nodeId)
{
	float		pos;

	pos		= mNodes[nodeId].splitter.getDistance(v);

	// check the case where the point lies in front
	if ( pos >= -HPBSP_EPSILON )
	{
		// we have reached an area
		if ( mNodes[nodeId].frontNode <= 0 )
			return (mNodes[nodeId].frontNode);

		// else recurse
		return getPointCell(v, mNodes[nodeId].frontNode);

	}

	// if we lie on or on the back (the else)
	if ( mNodes[nodeId].backNode < 0 )
	{
		switch ( mNodes[nodeId].backNode )
		{
		case SOLID_AREA:
			return 1;	// it'll be inverted eventually
		default:	// shouldn't get here if we have a legal geometry
			cout << "illegal topology..." << endl;
			return (mNodes[nodeId].backNode);
		}
	}

	return getPointCell(v, mNodes[nodeId].backNode);
}

/*
** just to ease things for us
*/
int BspObject::getPointCell(const Vector3 &v)
{
	return (-getPointCell(v, 0));
}

void BspObject::clipPolygons(const vector<BspPolygon> &inPolys,
							 vector<BspPolygon> &outPolys,
							 int nodeId)
{
	Plane			splitter;
	BspPolygon		front, back;
	int				i;

	vector<BspPolygon>	onFaces;
	vector<BspPolygon>	frontFaces;
	vector<BspPolygon>	backFaces;

	splitter = mNodes[nodeId].splitter;
	
	for( i = 0; i < static_cast<int>(inPolys.size()); i++ )
	{
		BspPolygon	currPoly = inPolys[i];
		switch(classifyPolyToPlane(splitter, currPoly))
		{
		case POLY_INFRONT:
			frontFaces.push_back(currPoly);
			break;
		case POLY_ONBACK:
			backFaces.push_back(currPoly);
			break;
		case POLY_ON:
			//if ( currPoly.p != splitter )
                switch(classifyOnPolyToPlane(splitter, currPoly))
				{
				case POLY_INFRONT:
					frontFaces.push_back(currPoly);
					break;
				case POLY_ONBACK:
					backFaces.push_back(currPoly);
					break;
				}
			//else onFaces.push_back(currPoly);
			break;
		case POLY_SPANNING:
			splitPoly(splitter, currPoly, front, back);
			frontFaces.push_back(front);
			backFaces.push_back(back);
			break;
		}
	}

	// we are in a leaf node, we will just return the onFaces and frontFaces
	if (mNodes[nodeId].frontNode <= 0 )
	{
		for ( i = 0; i < static_cast<int>(frontFaces.size()); i++ )
			outPolys.push_back(frontFaces[i]);
	} else {
		clipPolygons(frontFaces, outPolys, mNodes[nodeId].frontNode);
	}

	if (mNodes[nodeId].backNode != SOLID_AREA)
	{
		clipPolygons(backFaces, outPolys, mNodes[nodeId].backNode);
	}

}

/*
** clip a polygon, return true if the polygon is clipped
*/
void BspObject::clipPolygons(const vector<BspPolygon> &inPolys, vector<BspPolygon> &outPolys)
{
	outPolys.resize(0);
	clipPolygons(inPolys, outPolys, 0);
}

/*
** class constructor
*/
BspObject::BspObject(const std::vector<BspPolygon> &faces)
{
	BspNode	rootNode;

	mNodes.push_back(rootNode);
	buildBspNode(faces, 0);
}

BspObject::BspObject(const vector<BspNode> &nodes)
{
	this->mNodes = nodes;
}

void BspObject::getBspNodes(vector<BspNode> &outNodes)
{
	outNodes	= mNodes;
}