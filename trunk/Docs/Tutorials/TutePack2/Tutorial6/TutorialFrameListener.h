// TutorialFrameListener.h: interface for the TutorialFrameListener class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TUTORIALFRAMELISTENER_H__EA9A7D42_0B91_45B5_8F39_F445FB81EA8D__INCLUDED_)
#define AFX_TUTORIALFRAMELISTENER_H__EA9A7D42_0B91_45B5_8F39_F445FB81EA8D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ExampleFrameListener.h"

class TutorialFrameListener : public ExampleFrameListener
{
protected:
	SceneNode* mControlNode;
	SceneNode* mShipNode;
	SceneNode* mCameraNode;

	bool	mbFirstPerson;

	Real mfAfterburner;
	Real mfSpeed;
	Real mfPitch;
	Real mfYaw;
	Real mfRoll;
	Real mfShipRoll;
	Real mfShipPitch;
public:
    TutorialFrameListener(RenderWindow* win, Camera* cam, 
		SceneNode* controlNode, SceneNode* shipNode, SceneNode* cameraNode);
    bool frameStarted(const FrameEvent& evt);
};



#endif // !defined(AFX_TUTORIALFRAMELISTENER_H__EA9A7D42_0B91_45B5_8F39_F445FB81EA8D__INCLUDED_)
