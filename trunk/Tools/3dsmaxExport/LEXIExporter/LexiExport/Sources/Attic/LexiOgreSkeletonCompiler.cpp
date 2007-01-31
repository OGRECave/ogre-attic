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

#include "LexiStdAfx.h"
#include "LexiOgreSkeletonCompiler.h"

COgreSkeletonCompiler::COgreSkeletonCompiler( const CDDObject* pConfig, Ogre::String name, Ogre::MeshPtr ogreMesh )
{
	m_pOgreMesh = ogreMesh;
	m_pISkel = CIntermediateBuilder::Get()->GetSkeletonBuilder()->GetSkeleton();
	m_pSkel = Ogre::SkeletonManager::getSingletonPtr()->create(name + ".skeleton", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME /*"exporterSkelGroup"*/, true);
	m_pOgreMesh->setSkeletonName(name+".skeleton");

//	Ogre::LogManager::getSingletonPtr()->logMessage("COgreSkeletonCompiler: Reading Config.");
	ReadConfig(pConfig);
//	Ogre::LogManager::getSingletonPtr()->logMessage("COgreSkeletonCompiler: Config Read.");

	if( m_pISkel != NULL)
	{
		/*std::vector<CIntermediateBone*>::const_iterator it = m_pISkel->GetRootBones().begin();

		while ( it != m_pISkel->GetRootBones().end())
		{
			CreateSkeleton( *it );
			it++;
		}*/
		
		CreateSkeleton( NULL );

		m_pSkel->setBindingPose();
		LOGDEBUG "COgreSkeletonCompiler: Creating Ogre Skeleton Animations");
		CreateAnimations();
	}
	else
	{
		LOGWARNING "No skin modifier found, so no animations exported.");
	}
}

COgreSkeletonCompiler::~COgreSkeletonCompiler()
{
	Ogre::SkeletonManager::getSingletonPtr()->unload(m_pSkel->getHandle());
	Ogre::SkeletonManager::getSingletonPtr()->remove(m_pSkel->getHandle());
}


void COgreSkeletonCompiler::ReadConfig( const CDDObject* pConfig )
{
	assert(pConfig);
	//CDDObject* pAnimContainer = pConfig->GetDDObject("AnimationDataContainer");
	//assert(pAnimContainer);

	//m_lAnimSpecs.clear();

	//fastvector<const CDDObject*> lAnimList = pAnimContainer->GetDDList("Animations");
	//while(!lAnimList.empty())
	//{
	//	const CDDObject* pCurAnim = lAnimList.pop_back();
	//	SAnimSpec curAnimSpec;
	//	curAnimSpec.sAnimName = pCurAnim->GetString("AnimationNameID");
	//	curAnimSpec.iStartFrame = pCurAnim->GetInt("AnimationStartID");
	//	curAnimSpec.iEndFrame = pCurAnim->GetInt("AnimationEndID");
	//	curAnimSpec.fSampleRate = pCurAnim->GetFloat("AnimationSampleRateID");
	//	curAnimSpec.bOptimize = pCurAnim->GetBool("AnimationOptimizeID");

	//	m_lAnimSpecs.push_back(curAnimSpec);
	//}

	//m_sAnimName = pConfig->GetString("AnimationNameID");
	//m_iStartFrame = pConfig->GetInt("AnimationStartID");
	//m_iEndFrame = pConfig->GetInt("AnimationEndID");
	//m_fSampleRate = pConfig->GetFloat("AnimationSampleRateID");
	//m_bOptimize = pConfig->GetBool("AnimationOptimizeID");
}

void COgreSkeletonCompiler::CreateSkeleton( CIntermediateBone* pIBone )
{
	// The bones have been indexed according to a depthfirst search, so we know
	// that following this list we will always know the parent, and evenly more important
	// the bones will be added to the Ogre skeleton in the correct order so the index will
	// continue in the Ogre Skeletons bone list.

	Ogre::Bone* oBone = NULL;

	for(int i=0; i < m_pISkel->GetBoneCount(); i++)
	{
		CIntermediateBone* pIBone = m_pISkel->GetBoneByIndex(i);

		CIntermediateBone* pParent = pIBone->GetParent();
		if(pParent != NULL)
		{
			Ogre::Bone* oParent = NULL;
			try
			{
				oParent = m_pSkel->getBone(pParent->GetName());
			}
			catch (...)
			{
				LOGERROR "Error while creating skeleton..");
				return;
			}
			oBone = m_pSkel->createBone(pIBone->GetName());

			Ogre::Vector3 pos;
			Ogre::Vector3 scale;
			Ogre::Quaternion orient;
			pIBone->GetBindingPose(pos, orient, scale);

			oBone->setPosition( pos );
			oBone->setOrientation( orient );
			oBone->setScale( scale );

			oParent->addChild(oBone);
		}
		else
		{
			oBone = m_pSkel->createBone(pIBone->GetName());
			Ogre::Vector3 pos;
			Ogre::Vector3 scale;
			Ogre::Quaternion orient;
			pIBone->GetBindingPose(pos, orient, scale);

			oBone->setPosition( pos );
			oBone->setOrientation( orient );
			oBone->setScale( scale );
		}


	}




	/*
	Ogre::Bone* oBone = NULL;

	//for ( int i=0; i < m_pISkel->GetBoneCount(); i++)
	//{
	//	CIntermediateBone* pIBone = m_pISkel->GetBoneByIndex(i);

		// Init point
		if(pIBone == NULL)
		{
			pIBone = m_pISkel->GetBone(0);
		}

		CIntermediateBone* pParent = pIBone->GetParent();
		if(pParent != NULL)
		{
			Ogre::Bone* oParent = NULL;
			try
			{
				oParent = m_pSkel->getBone(pParent->GetName());
			}
			catch (...)
			{
				// new bone detected.. 
				oParent = m_pSkel->createBone(pParent->GetName());
			}
			oBone = m_pSkel->createBone(pIBone->GetName());

			Ogre::Vector3 pos;
			Ogre::Vector3 scale;
			Ogre::Quaternion orient;
			pIBone->GetBindingPose(pos, orient, scale);

			oBone->setPosition( pos );
			oBone->setOrientation( orient );
			oBone->setScale( scale );

			oParent->addChild(oBone);
		}
		else
		{
			oBone = m_pSkel->createBone(pIBone->GetName());
			Ogre::Vector3 pos;
			Ogre::Vector3 scale;
			Ogre::Quaternion orient;
			pIBone->GetBindingPose(pos, orient, scale);

			oBone->setPosition( pos );
			oBone->setOrientation( orient );
			oBone->setScale( scale );
		}

		for (int i=0; i<pIBone->GetBoneCount(); i++)
		{
			CreateSkeleton(pIBone->GetBone(i));
		}
	//}

	*/
}

void COgreSkeletonCompiler::CreateAnimations( void )
{
	CIntermediateBone* tmpBone = m_pISkel->GetBoneByIndex(0);
	std::map< Ogre::String, CAnimationData* > lAnimMap;
	if(tmpBone != NULL)
		lAnimMap = tmpBone->GetAnimations();
	else
		return;

	std::map< Ogre::String, CAnimationData* >::const_iterator animIter = lAnimMap.begin();

	while(animIter != lAnimMap.end() )
	{
		CAnimationData* pCurAnimData = (*animIter).second;
		Ogre::String curAnimName = (*animIter).first;
		animIter++;

		if(m_pSkel.get())
		{
			unsigned int iLastFrame = m_pISkel->GetBoneByIndex(0)->GetFrameCount( curAnimName );
			float animLength;
			Ogre::Vector3 endPos;
			Ogre::Vector3 endScale;
			Ogre::Quaternion endOrient;
			m_pISkel->GetBoneByIndex(0)->GetFrame( curAnimName, iLastFrame-1, animLength, endPos, endOrient, endScale);
			//float animLength = ( 1 /float(m_pISkel->GetFPS()) )*;
			Ogre::Animation* anim = m_pSkel->createAnimation(curAnimName, animLength);
			if(anim) {
				Ogre::AnimationStateSet* animSet = new Ogre::AnimationStateSet(); 

				for ( int i=0; i < m_pSkel->getNumBones(); i++)
				{
					Ogre::Bone* pBone = m_pSkel->getBone(i);
					Ogre::NodeAnimationTrack* pAnimTrack = anim->createNodeTrack(pBone->getHandle(),pBone);

					CIntermediateBone* pIBone = m_pISkel->GetBoneByName( pBone->getName().c_str() );
					
					for ( int j=0; j < pIBone->GetFrameCount(curAnimName); j++)
					{
						float fTimeInSecs;
						Ogre::Vector3 pos;
						Ogre::Vector3 scale;
						Ogre::Quaternion orient;
						pIBone->GetFrame(curAnimName,j,fTimeInSecs,pos,orient,scale);

						Ogre::TransformKeyFrame* pKeyFrame = pAnimTrack->createNodeKeyFrame(fTimeInSecs);
						pKeyFrame->setRotation( orient );
						pKeyFrame->setScale( scale );
						pKeyFrame->setTranslate( pos );
					}

					//// Add end animation Spline keyframe for tangent generation.
					//int iEndFrame = pIBone->GetFrameCount();
					//Ogre::TransformKeyFrame* pKeyFrame = pAnimTrack->createNodeKeyFrame(anim->getLength());
					//Ogre::Vector3 pos(0,0,0);
					//Ogre::Vector3 scale(1,1,1);
					//Ogre::Quaternion orient(Ogre::Radian(90), Ogre::Vector3::NEGATIVE_UNIT_X);
					////pIBone->GetFrame( iEndFrame-1,pos,orient,scale );
					//pKeyFrame->setRotation( orient );
					//pKeyFrame->setScale( scale );
					//pKeyFrame->setTranslate( pos );

					if(pCurAnimData->GetOptimize())
						pAnimTrack->optimise();
				}
				m_pSkel->_initAnimationState(animSet);
				m_pOgreMesh->_initAnimationState(animSet);
				m_pSkel->setAnimationState(*animSet);
			}
		}

		
	}
}

bool COgreSkeletonCompiler::WriteOgreSkeleton( const Ogre::String& sFilename )
{
	Ogre::SkeletonSerializer* pSkeletonWriter = new Ogre::SkeletonSerializer();
	try
	{
		pSkeletonWriter->exportSkeleton( m_pSkel.get(), sFilename );
	}
	catch (Ogre::Exception& e)
	{
		LOGERROR "OgreExeception caught: %s", e.getDescription().c_str()); 
		return false;
	}
	delete pSkeletonWriter;
	return true;
}
