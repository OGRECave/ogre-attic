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

#ifndef __MaxExporter_IntermediateBone__
#define __MaxExporter_IntermediateBone__

//

class CIntermediateBone {

	private:

		std::string m_sName;

		std::vector<CIntermediateBone*> m_Bones;

		std::vector<float> m_Times;
		std::vector<Ogre::Vector3> m_Positions;
		std::vector<Ogre::Quaternion> m_Orientations;
		std::vector<Ogre::Vector3> m_Scales;

		ULONG m_Handle;
		ULONG m_ParentHandle;
		unsigned int m_iIndex;
		CIntermediateBone* m_pParent;

		// Binding data
		Ogre::Vector3 m_bindingPos;
		Ogre::Quaternion m_bindingOrientation;
		Ogre::Vector3 m_bindingScale;

	public:

		//
		//  General
		//

		// Constructor/Destructor
		CIntermediateBone(const char* pszName);
		~CIntermediateBone();

		// Clear everything
		void clear();

		// Get name
		std::string GetName() const;

		void SetHandle( ULONG handle );
		ULONG GetHandle( void );

		void SetParentHandle( ULONG handle );
		ULONG GetParentHandle( void );

		void		 SetIndex( unsigned int index );
		unsigned int GetIndex( void );

		//
		//  Bones
		//

		// Get number of bones
		unsigned int GetBoneCount() const;

		// Get bone
		CIntermediateBone* GetBone(unsigned int iBone);
		const CIntermediateBone* GetBone(unsigned int iBone) const;

		// Add bone
		void AddBone(CIntermediateBone* pBone);

		// Get/Set Parent
		CIntermediateBone* GetParent( void );
		void SetParent( CIntermediateBone* parent );

		//
		//  Frames
		//

		// Get number of frames
		unsigned int GetFrameCount() const;

		// Get frame
		void GetFrame(unsigned int iFrame, float& fTimeInSecs, Ogre::Vector3& vPos, Ogre::Quaternion& qOri, Ogre::Vector3& vScale) const;

		// Add frame
		void AddFrame( float timeInSecs, const Ogre::Vector3& vPos, const Ogre::Quaternion& qOri, const Ogre::Vector3& vScale);

		// Set initial bindind pose from which all transformations are relative to
		void SetBindingPose(const Ogre::Vector3& vPos, const Ogre::Quaternion& qOri, const Ogre::Vector3& vScale);

		void GetBindingPose(Ogre::Vector3& vPos, Ogre::Quaternion& qOri, Ogre::Vector3& vScale) const;
};

//

#endif // __MaxExporter_IntermediateBone__