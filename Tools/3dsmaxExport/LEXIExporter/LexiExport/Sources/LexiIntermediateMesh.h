/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
Bo Krohn

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

#ifndef __NDS_LexiExporter_IntermediateMesh__
#define __NDS_LexiExporter_IntermediateMesh__

//

class CIntermediateMesh : public Ogre::MovableObject {

	friend class CIntermediateBuilder;

	private:

		typedef struct {

			unsigned int m_iTime;
			CMeshArray* m_pArray;

		} MeshArray;

	private:

		void BuildMaterialList();

		unsigned int m_iNumTriangles;
		CTriangleArray m_Triangles;

		typedef std::map<std::string, MeshArray*> ArrayMap;
		ArrayMap m_Arrays;

		fastvector<CIntermediateMaterial*> m_Materials;

		CIntermediateSkeleton* m_pISkeleton;

	protected:

		unsigned int m_iNodeID;

		//

		CIntermediateMesh(unsigned int iNumTriangles, unsigned int iNodeID);

		void ForceCreateArray(const char* pszName);

		//

        virtual const Ogre::String& getMovableType(void) const;
        virtual const Ogre::AxisAlignedBox& getBoundingBox(void) const;
		virtual Ogre::Real getBoundingRadius(void) const;
        virtual void _updateRenderQueue(Ogre::RenderQueue* queue);

	public:

		// Destructor
		~CIntermediateMesh();

		//
		//  Triangle

		// Get number of triangles
		unsigned int GetNumTriangles() const;

		// Get reference to triangle
		CTriangle& GetTriangle(unsigned int iIndex);
		const CTriangle& GetTriangle(unsigned int iIndex) const;

		// Get reference to entire triangle array
		CTriangleArray& GetTriangles();
		const CTriangleArray& GetTriangles() const;

		// Get triangle indices using a specific material
		void GetTrianglesUsingMaterial(CIntermediateMaterial* pMaterial, std::vector<unsigned int>& Triangles) const;

		//
		//  Array

		// Get array
		CMeshArray* GetArray(const char* pszName, TimeValue iTime);

		//
		//  Material

		// Get number of materials
		unsigned int GetNumMaterials() const;

		// Get material
		CIntermediateMaterial* GetMaterial(unsigned int iIndex) const;

		//
		// Skeleton
		void SetSkeleton( CIntermediateSkeleton* pSkel );
		CIntermediateSkeleton* GetSkeleton( void );

		//
		//  Utilities

		// Reindex vertices
		void Reindex(const fastvector<CMeshArray*>& ArrayList);

};

//

#endif // __NDS_LexiExporter_IntermediateMesh__