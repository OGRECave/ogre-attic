#include "ExampleApplication.h"
#include "HpBspDotSceneLoader.h"
#include "HybridPortalBspSceneManagerFactory.h"
#include "HybridPortalBspSceneManager.h"


using namespace std;
using namespace Ogre;

class HybridPortalBspFrameListener : public ExampleFrameListener
{
public:
	HybridPortalBspFrameListener( RenderWindow* win, Camera* cam, SceneManager *sceneMgr, HybridPortalBspSceneLoader *hpbsp)
        : ExampleFrameListener(win, cam, false, false)
    {
		mHPBspOverlay = OverlayManager::getSingleton().getByName("HPBsp/MainOverlay");
		mHPBspOverlay->show();
		mSceneMgr = sceneMgr;
		mHPBsp = hpbsp;
    }

    bool frameEnded(const FrameEvent& evt)
    {
        updateStats();
        return true;
    }


protected:

    void updateStats(void)
    {
        static String currPos = "Current Position: ";
        static String currCell = "Current Cell: ";
        static String strPortals = "Cell Portals:";
        static String neighboringCells = "Neighboring Cells:";

        // update stats when necessary
        try {
			OverlayElement* guiPos = OverlayManager::getSingleton().getOverlayElement("HPBsp/CurrentPos");
			guiPos->setCaption(currPos + StringConverter::toString(mSceneMgr->getCamera("PlayerCam")->getPosition()));

			OverlayElement* guiCell = OverlayManager::getSingleton().getOverlayElement("HPBsp/CurrentCell");
			String strCell = currCell;
			
			HybridPortalBspSceneManager* sm = reinterpret_cast<HybridPortalBspSceneManager*>(mSceneMgr);
			int cellId = sm->getPointCell(mSceneMgr->getCamera("PlayerCam")->getPosition());
			
			if ( cellId < 0 )
				strCell += "Solid Space";
			else strCell += StringConverter::toString(cellId);

			guiCell->setCaption(strCell);

			OverlayElement* guiPortals = OverlayManager::getSingleton().getOverlayElement("HPBsp/Portals");
			vector<int> portalsId;
			sm->getCellPortals(cellId, portalsId);

			String tmpPortals = strPortals;

			size_t i = 0;
			for (; i < portalsId.size(); i++)
				tmpPortals += " " + StringConverter::toString(portalsId[i]);

			guiPortals->setCaption(tmpPortals);

			OverlayElement* guiNeighboringCells = OverlayManager::getSingleton().getOverlayElement("HPBsp/NeighboringCells");
			vector<int> cellsId;
			sm->getNeighboringCells(cellId, cellsId);

			String tmpCells = neighboringCells;

			for (i = 0; i < portalsId.size(); i++)
				tmpCells += " " + StringConverter::toString(cellsId[i]);

			guiNeighboringCells->setCaption(tmpCells);
        }
        catch(...)
        {
            // ignore
        }
    }

	Overlay*		mHPBspOverlay;
	SceneManager*	mSceneMgr;
	HybridPortalBspSceneLoader* mHPBsp;
};

class HybridPortalBspApplication : public ExampleApplication
{
public:
	HybridPortalBspApplication() : ExampleApplication()
	{
		mHPBspFrameListener = 0;

	}

	~HybridPortalBspApplication()
	{
		if(mHPBspFrameListener)
			delete mHPBspFrameListener;
	}

protected:

	HybridPortalBspSceneManagerFactory *factory;

	void chooseSceneManager(void)
	{
		factory = new HybridPortalBspSceneManagerFactory();
		Root::getSingleton().addSceneManagerFactory(factory);

		mSceneMgr = mRoot->createSceneManager("HybridPortalBspSceneManager", "demoPortalBsp");
	}

	void createScene(void)
	{
		reinterpret_cast<HybridPortalBspSceneManager*>(mSceneMgr)->loadLevel(HpBspDotSceneLoader::getSingleton(),
			"ex_3_2.scene", "General");
		reinterpret_cast<HybridPortalBspSceneManager*>(mSceneMgr)->showVisiblePortals(true);
		reinterpret_cast<HybridPortalBspSceneManager*>(mSceneMgr)->showVisibleCells(true);

		LogManager::getSingleton().logMessage(String("[createScene] ") + StringConverter::toString(reinterpret_cast<HybridPortalBspSceneManager*>(mSceneMgr)->getPortalCount()));
		
		mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox");
		mSceneMgr->setAmbientLight(ColourValue(1, 1, 1, 1));
		

		//mCamera->setPolygonMode(PM_WIREFRAME);
	}

	void createFrameListener()
	{
		ExampleApplication::createFrameListener();

		mHPBspFrameListener = new HybridPortalBspFrameListener(mWindow, mCamera, mSceneMgr, mHPBsp);
		mHPBspFrameListener->showDebugOverlay(true);
		mRoot->addFrameListener(mHPBspFrameListener);
	}

	void destroyScene()
	{
		Root::getSingleton().removeSceneManagerFactory(factory);
		delete factory;
	}

	HybridPortalBspFrameListener *mHPBspFrameListener;
	HpBspDotSceneLoader	*mHPBsp;
};

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
    HybridPortalBspApplication app;

    try {
        app.go();
    } catch( Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        fprintf(stderr, "An exception has occured: %s\n",
                e.getFullDescription().c_str());
#endif
    }

    return 0;
}