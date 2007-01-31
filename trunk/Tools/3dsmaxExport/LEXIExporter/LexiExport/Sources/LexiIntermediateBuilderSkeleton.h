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

#ifndef __NDS_LexiExporter_IntermediateBuilderSkeleton__
#define __NDS_LexiExporter_IntermediateBuilderSkeleton__

//

class CIntermediateBuilderSkeleton {

public:

	CIntermediateBuilderSkeleton( void );
	~CIntermediateBuilderSkeleton( void );

	bool BuildIntermediateSkeleton( INode* pRootMaxNode );

	void CreateAnimation( CDDObject* pDDConfig, CExportProgressDlg *pProgressDlg );

	void Finalize( void );

	void SetVertexData( unsigned int maxIdx, unsigned int intermediateIdx );

	CIntermediateSkeleton* GetSkeleton( void );

private:

	bool FindSkinModifier( void );

	void CreateBonePool( void );

	void ConnectLinkedBones( void );

	void AssembleBones( void );

	void SetBindingPose( unsigned int frame );

	void SetBoneIndexes( void );

	void SetBoneIndexesRecursive( CIntermediateBone* pIBone );

	void AddFrame( CAnimationSetting animSetting);

	// Max node holding the mesh to be bone animated
	INode*					m_pMaxNode; 

	CIntermediateSkeleton*	m_pISkel;
	Modifier*				m_pSkinMod;
	ISkin*					m_pSkin;
	ISkinContextData*		m_pSkinContext;

	unsigned int			m_iBoneIndex;

	 CExportProgressDlg*	m_pProgressDlg;
};

#endif 