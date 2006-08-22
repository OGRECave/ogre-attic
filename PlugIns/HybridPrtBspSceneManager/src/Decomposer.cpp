/*
** This source file is part of OGRE (Object-oriented Graphics Rendering Engine)
** For the latest info, see http://www.ogre3d.org/
** 
** Copyright (c) 2006 Wael El Oraiby
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
#include <vector>
#include <map>
#include <algorithm>

#include "Ogre.h"
#include "Decomposer.h"

using namespace std;
using namespace Ogre;

//-----------------------------------------------------------------------------
Decomposer::Decomposer()
{
}

//-----------------------------------------------------------------------------
int Decomposer::decompose(vector<BspPolygon> &inPolys)
{
	size_t		i;
	
	mPolys	= inPolys;

	for ( i = 0; i < mPolys.size(); i++ )
	{
		mPolys[i].cellId = -1;

		size_t	numVerts = inPolys[i].verts.size();
		for ( size_t j = 0; j < numVerts; j++ )
		{
			Vector3	v = mPolys[i].verts[j];
			addVertex(v, static_cast<int>(i));
		}
	}

	int		cellId = 0;
	for ( v_iter vi = mVerts.begin(); vi != mVerts.end(); vi++ )
	{
		if ( (*vi).second.cellId < 0 )
		{
			walkVertex((*vi).first, cellId);
			cellId++;
		}
	}

	inPolys	= mPolys;

	return cellId;
}

//-----------------------------------------------------------------------------
void Decomposer::addVertex(const Vector3 &vertex, int polyId)
{
	Vertex	vert;

	vert.polys.resize(0);

	if ( mVerts.find(vertex) == mVerts.end() )
	{
		vert.cellId		= -1;
		vert.polys.push_back(polyId);
		mVerts[vertex]	= vert;

		return;
	}

	mVerts[vertex].polys.push_back(polyId);
	return;
}

//-----------------------------------------------------------------------------
void Decomposer::walkVertex(const Vector3 &vertex, int cellId)
{
	mVerts[vertex].cellId	= cellId;
	Vertex	v = mVerts[vertex];

	for ( size_t i = 0; i < v.polys.size(); i++ )
	{
		int pcId = mPolys[v.polys[i]].cellId;
		if ( pcId < 0 )
			mPolys[v.polys[i]].cellId = cellId;
		else if ( pcId != cellId )
			cout << "Error: something is wrong with the mesh " << pcId << "-" << cellId << endl;

		for ( size_t j = 0; j < mPolys[v.polys[i]].verts.size(); j++ )
		{
			Vector3 vec = mPolys[v.polys[i]].verts[j];
			if ( mVerts[vec].cellId < 0 )
				walkVertex(vec, cellId);
		}
	}
}
