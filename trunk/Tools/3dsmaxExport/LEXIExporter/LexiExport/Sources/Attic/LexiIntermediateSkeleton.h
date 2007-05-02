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

#ifndef __MaxExporter_IntermediateSkeleton__
#define __MaxExporter_IntermediateSkeleton__

//

struct SVertexBoneData{
	ULONG boneIndex;
	float weight;
	bool bRead;

public:
	SVertexBoneData()
	{
		boneIndex = 0xFFFFFFFF;
		weight = 0.0f;
		bRead = false;
	}

	bool operator< (const SVertexBoneData& other)
	{
		// We do it the other way around since we want to sort decenting
		if(weight > other.weight)
			return true;
		return false;
	}
};

class CIntermediateSkeleton {



	private:

		std::vector<CIntermediateBone*> m_BoneList;
		std::vector<CIntermediateBone*> m_RootBoneList;
		std::map<int, CIntermediateBone*> m_IndexedBoneList;
		std::map<std::string, CIntermediateBone*> m_BoneNameList;

		std::map< ULONG, CIntermediateBone* > m_BoneHandles;
		std::map< int, std::vector<SVertexBoneData> > m_VertexBoneData;

		std::map< int, std::vector<SVertexBoneData> > m_ReindexVertexBoneData;

		float m_fFPS;

		bool RecursiveAssembleBones( CIntermediateBone* pIBone	 );
		void RecursivePopulateBoneHandleMap( CIntermediateBone* pIBone );

		void print( std::stringstream& output, CIntermediateBone* pBone, int indent=0);
		void printAnimationData( std::stringstream& output, CIntermediateBone* pBone, int indent);


	public:

		//
		//  General
		//

		// Constructor/Destructor
		CIntermediateSkeleton();
		~CIntermediateSkeleton();

		// Clear everything
		void clear();

		// Set FPS
		void SetFPS(float fFPS);

		// Get FPS
		float GetFPS() const;

		void PopulateBoneHandleMap( void );
		void BuildIndexedBoneList( void );

		CIntermediateBone* FindBone( ULONG handle );

		int GetNrOfAssignmentsOnVertex( int idx );
		bool GetVertexDataCollection( int idx, std::vector<SVertexBoneData>** returnCollection );
		void TrimVertexAssignments( int iMaxAssignments );
		void NormalizeVertexAssignments( void );
		bool GetVertexData( int idx, int assignmentNr, SVertexBoneData& returnVal);
		bool AddVertexData( int idx, SVertexBoneData vertexData );

		bool AssembleBones( void );

		void ExtractVertexAssignmentsArrays( SharedUtilities::fastvector< CMeshArray* > &bufferList );
		bool ApplyVertexAssignmentsArrays( SharedUtilities::fastvector< CMeshArray* > &bufferList );

		//bool PrepareReindexChange( int oldIndex, int newIndex );
		//bool ApplyReindexChanges( void );

		void MarkBoneAsRoot( CIntermediateBone* pIBone );
		const std::vector<CIntermediateBone*>& GetRootBones( void ) const;


		std::string ToString( void );
		//
		//  Bones
		//

		// Get number of bones
		unsigned int GetBoneCount() const;

		// Get bone
		CIntermediateBone* GetBone(unsigned int iBone);
		const CIntermediateBone* GetBone(unsigned int iBone) const;
		CIntermediateBone* GetBoneByName( const char* pszName ) const;
		CIntermediateBone* GetBoneByIndex( int index ) const;

		// Get bone name
		std::string GetBoneName(unsigned int iBone) const;

		// Add bone
		void AddBone(CIntermediateBone* pBone, const char* pszName);
		//bool AddBone(const char* pszName, unsigned int iNodeID, unsigned int iStartFrame, unsigned int iEndFrame, unsigned int iSampleRate);

};

//

#endif // __MaxExporter_IntermediateSkeleton__