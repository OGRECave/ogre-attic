// TutorialApplication.h: interface for the TutorialApplication class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TUTORIALAPPLICATION_H__2F8E8A98_DA5D_49F9_9F09_8C6C05C27D14__INCLUDED_)
#define AFX_TUTORIALAPPLICATION_H__2F8E8A98_DA5D_49F9_9F09_8C6C05C27D14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ExampleApplication.h"

class TutorialApplication : public ExampleApplication  
{
public:
	TutorialApplication();
	virtual ~TutorialApplication();
protected:
	void createScene();
	void createFrameListener();
	void chooseSceneManager();


	Entity* mShip;	
	SceneNode* mControlNode;
	SceneNode* mShipNode;
	SceneNode* mCameraNode;
};

#endif // !defined(AFX_TUTORIALAPPLICATION_H__2F8E8A98_DA5D_49F9_9F09_8C6C05C27D14__INCLUDED_)
