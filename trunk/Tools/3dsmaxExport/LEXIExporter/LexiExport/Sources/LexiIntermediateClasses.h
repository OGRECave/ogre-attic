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

#ifndef __NDS_LexiExporter_IntermediateClasses__
#define __NDS_LexiExporter_IntermediateClasses__

//

class CMeshArray {

	public:

		virtual void Create(unsigned int iSize) = 0;
		virtual void Create(unsigned int iSize, const void* pData) = 0;

		virtual void* Data() = 0;
		virtual const void* Data() const = 0;
		virtual void* Data(unsigned int iElement) = 0;
		virtual const void* Data(unsigned int iElement) const = 0;

		virtual void Zero() = 0;

		virtual unsigned int Size() const = 0;

		virtual unsigned int ElementSize() const = 0;

};

//

template <class T> class CTMeshArray : public CMeshArray {

	private:

		T* m_pArray;
		unsigned int m_iSize;

	public:

		CTMeshArray()
		{
			m_pArray = NULL;
			m_iSize = 0;
		}
		CTMeshArray(unsigned int iSize)
		{
			m_pArray = NULL;
			m_iSize = 0;

			Create(iSize);
		}
		virtual ~CTMeshArray()
		{
			if(m_pArray) delete []m_pArray;
		}

		//

		void* Data()
		{
			return m_pArray;
		}
		const void* Data() const
		{
			return m_pArray;
		}

		void* Data(unsigned int iElement)
		{
			return &m_pArray[iElement];
		}

		const void* Data(unsigned int iElement) const
		{
			return &m_pArray[iElement];
		}

		unsigned int Size() const
		{
			return m_iSize;
		}

		unsigned int ElementSize() const
		{
			return sizeof(T);
		}

		//

		void Create(unsigned int iSize)
		{
			if(m_pArray) delete []m_pArray;
			if(iSize) m_pArray = new T[iSize];
			else m_pArray = NULL;
			m_iSize = iSize;
		}

		void Create(unsigned int iSize, const void* pData)
		{
			Create(iSize);
			memcpy(m_pArray, pData, m_iSize * sizeof(T));
		}

		//

		void Zero()
		{
			memset(m_pArray, 0, m_iSize * sizeof(T));
		}

		//

		T& operator [] (unsigned int iIndex)
		{
			return m_pArray[iIndex];
		}
		const T& operator [] (unsigned int iIndex) const
		{
			return m_pArray[iIndex];
		}

};

//

typedef CTMeshArray<Ogre::Vector2> CVec2Array;
typedef CTMeshArray<Ogre::Vector3> CVec3Array;
typedef CTMeshArray<Ogre::Vector4> CVec4Array;
typedef CTMeshArray<float> CFloatArray;
typedef CTMeshArray<unsigned char> CUInt8Array;

//

class CTriangle {

	public:

		unsigned int m_Vertices[3];
		CIntermediateMaterial* m_pMaterial;

};

typedef CTMeshArray<CTriangle> CTriangleArray;

//

#endif // __NDS_LexiExporter_IntermediateClasses__