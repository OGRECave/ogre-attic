/* Static water simulation by eru
 * Started 29.05.2003, 20:54:37
 */
#include "Ogre.h"
#include "OgreStringConverter.h"

#include "ExampleApplication.h"
#include "WaterMesh.h"

AnimationState* mAnimState;

// Mesh stuff 
#define MESH_NAME "WaterMesh"
#define ENTITY_NAME "WaterEntity"
#define MATERIAL_PREFIX "Examples/Water"
#define MATERIAL_NAME "Examples/Water0"
#define COMPLEXITY 64 		// watch out - number of polys is 2*ACCURACY*ACCURACY !
#define PLANE_SIZE 3000.0f

/* Some global variables */
SceneNode *headNode ;
Overlay *waterOverlay ;

// Event handler 
class WaterListener: public ExampleFrameListener
{
protected:
	WaterMesh *waterMesh ;
	Entity *waterEntity ;
	int materialNumber ;
	Real timeoutDelay ;

#define RAIN_FREQ 2
#define RAIN_HEIGHT_RANDOM 5
#define RAIN_HEIGHT_CONSTANT 5
	/** Simple rain */
	void rain()
	{
		if (! (rand() % RAIN_FREQ )) { // every X frames
			float x = rand() % (COMPLEXITY-1) + 1 ;
			float y = rand() % (COMPLEXITY-1) + 1 ;
			float h = rand() % RAIN_HEIGHT_RANDOM + RAIN_HEIGHT_CONSTANT ;
			waterMesh->push(x,y,-h) ;
		}
	}

	/** Head animation */
	Real headDepth ;
	void animateHead(Real timeSinceLastFrame)
	{
		// sine track? :)
		static double sines[4] = {0,100,200,300};
		static const double adds[4] = {0.3,-1.6,1.1,0.5};
		static Vector3 oldPos = Vector3::UNIT_Z;
		for(int i=0;i<4;i++) {
			sines[i]+=adds[i]*timeSinceLastFrame;
		}
		Real tx = ((sin(sines[0]) + sin(sines[1])) / 4 + 0.5 ) * (float)(COMPLEXITY-2) + 1 ;
		Real ty = ((sin(sines[2]) + sin(sines[3])) / 4 + 0.5 ) * (float)(COMPLEXITY-2) + 1 ;
		waterMesh->push(tx,ty, -headDepth);
		Real step = PLANE_SIZE / COMPLEXITY ;
		headNode->resetToInitialState();
		headNode->scale(3,3,3);
		Vector3 newPos = Vector3(step*tx, headDepth, step*ty);
		Vector3 diffPos = newPos - oldPos ;
		Quaternion headRotation = Vector3::UNIT_Z.getRotationTo(diffPos);
		oldPos = newPos ;
		headNode->translate(newPos);
		headNode->rotate(headRotation);
	}
	
	// GUI updaters
	void updateInfoParamC()
	{
		GuiManager::getSingleton().getGuiElement("Example/Water/Param_C") \
			->setCaption("[1/2]Ripple speed: "+StringConverter::toString(waterMesh->PARAM_C));		
	}
	void updateInfoParamD()
	{
		GuiManager::getSingleton().getGuiElement("Example/Water/Param_D") \
			->setCaption("[3/4]Distance: "+StringConverter::toString(waterMesh->PARAM_D));		
	}
	void updateInfoParamU()
	{
		GuiManager::getSingleton().getGuiElement("Example/Water/Param_U") \
			->setCaption("[5/6]Viscosity: "+StringConverter::toString(waterMesh->PARAM_U));		
	}
	void updateInfoParamT()
	{
		GuiManager::getSingleton().getGuiElement("Example/Water/Param_T") \
			->setCaption("[7/8]Frame time: "+StringConverter::toString(waterMesh->PARAM_T));		
	}
	void updateInfoNormals()
	{
		GuiManager::getSingleton().getGuiElement("Example/Water/Normals") \
			->setCaption(String("[N]Normals: ")+((waterMesh->useFakeNormals)?"fake":"real"));
	}
	void switchNormals()
	{
		waterMesh->useFakeNormals = !waterMesh->useFakeNormals ;
		updateInfoNormals() ;
	}
	void updateInfoHeadDepth()
	{
		GuiManager::getSingleton().getGuiElement("Example/Water/Depth") \
			->setCaption(String("[U/J]Head depth: ")+StringConverter::toString(headDepth));
	}
	
	void updateMaterial()
	{
		String materialName = MATERIAL_PREFIX+StringConverter::toString(materialNumber);
		Material *material = static_cast<Material*> (MaterialManager::getSingleton().getByName(materialName));
		if (!material){
			if(materialNumber){
				materialNumber = 0 ;
				updateMaterial();
				return ;
			} else {
				Except(Exception::ERR_INTERNAL_ERROR,
					"Material "+materialName+"doesn't exist!",
					"WaterListener::updateMaterial");
			}
		}
		waterEntity->setMaterialName(materialName);
		GuiManager::getSingleton().getGuiElement("Example/Water/Material") \
			->setCaption(String("[M]Material: ")+materialName);
	}
	void switchMaterial()
	{
		materialNumber++;
		updateMaterial();
	}
public:
    WaterListener(RenderWindow* win, Camera* cam, 
		WaterMesh *waterMesh, Entity *waterEntity)
        : ExampleFrameListener(win, cam)
    {
		this->waterMesh = waterMesh ;
		this->waterEntity = waterEntity ;
		materialNumber = 0;
		timeoutDelay = 0.0f;
		headDepth = 2.0f;
		
		updateMaterial();
		updateInfoParamC();
		updateInfoParamD();
		updateInfoParamU();
		updateInfoParamT();
		updateInfoNormals();
		updateInfoHeadDepth();
    }

    bool frameStarted(const FrameEvent& evt)
    {
        mAnimState->addTime(evt.timeSinceLastFrame);
		
		// process keyboard events
		mInputDevice->capture();
		Real changeSpeed = evt.timeSinceLastFrame ;
		
		// adjust keyboard speed with SHIFT (increase) and CONTROL (decrease)
		if (mInputDevice->isKeyDown(KC_LSHIFT) || mInputDevice->isKeyDown(KC_RSHIFT)) {
			changeSpeed *= 10.0f ;
		}
		if (mInputDevice->isKeyDown(KC_LCONTROL)) { 
			changeSpeed /= 10.0f ;
		}
		
		// rain
		if (mInputDevice->isKeyDown(KC_SPACE)) {
			rain();
		}

		// adjust values (some macros for faster change		
#define ADJUST_RANGE(_value,_keyPlus,_keyMinus,_minVal,_maxVal,_change,_macro) {\
	if (mInputDevice->isKeyDown(_keyPlus)) \
		{ _value+=_change ; if (_value>=_maxVal) _value = _maxVal ; _macro ; } ; \
	if (mInputDevice->isKeyDown(_keyMinus)) \
		{ _value-=_change; if (_value<=_minVal) _value = _minVal ; _macro ; } ; \
}

		ADJUST_RANGE(headDepth, KC_U, KC_J, 0, 10, 0.5*changeSpeed, updateInfoHeadDepth()) ;

		ADJUST_RANGE(waterMesh->PARAM_C, KC_2, KC_1, 0, 10, 0.1f*changeSpeed, updateInfoParamC()) ;

		ADJUST_RANGE(waterMesh->PARAM_D, KC_4, KC_3, 0.1, 10, 0.1f*changeSpeed, updateInfoParamD()) ;

		ADJUST_RANGE(waterMesh->PARAM_U, KC_6, KC_5, -2, 10, 0.1f*changeSpeed, updateInfoParamU()) ;

		ADJUST_RANGE(waterMesh->PARAM_T, KC_8, KC_7, 0, 10, 0.1f*changeSpeed, updateInfoParamT()) ;

		timeoutDelay-=evt.timeSinceLastFrame ;
		if (timeoutDelay<=0)
			timeoutDelay = 0;

#define SWITCH_VALUE(_key,_timeDelay, _macro) { \
		if (mInputDevice->isKeyDown(_key) && timeoutDelay==0) { \
			timeoutDelay = _timeDelay ; _macro ;} }
	
		SWITCH_VALUE(KC_N, 0.5f, switchNormals());
			
		SWITCH_VALUE(KC_M, 0.5f, switchMaterial());

		animateHead(evt.timeSinceLastFrame);
			
		waterMesh->updateMesh(evt.timeSinceLastFrame);

        // Call default
        return ExampleFrameListener::frameStarted(evt); 

    }
};

class WaterApplication : public ExampleApplication
{
public:
    WaterApplication() {
    
    }

    ~WaterApplication() {  }

protected:
	WaterMesh *waterMesh ;
	Entity *waterEntity ;

// Just override the mandatory create scene method
    void createScene(void)
    {

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(200,300,100);

		// Create water mesh and entity
		waterMesh = new WaterMesh(MESH_NAME, PLANE_SIZE, COMPLEXITY);
		waterEntity = mSceneMgr->createEntity(ENTITY_NAME, 
			MESH_NAME);
		waterEntity->setMaterialName(MATERIAL_NAME);
		SceneNode *waterNode = static_cast<SceneNode*>(mSceneMgr->getRootSceneNode()->createChild());
		waterNode->attachObject(waterEntity);
		SceneNode *followNode = static_cast<SceneNode*>(waterNode->createChild());
		followNode->translate(PLANE_SIZE/2, 0, PLANE_SIZE/2);

        // Add a head, give it it's own node
        headNode = static_cast<SceneNode*>(waterNode->createChild());
        Entity *ent = mSceneMgr->createEntity("head", "ogrehead.mesh");
        headNode->attachObject(ent);

		// Make sure the camera track this node
        mCamera->setAutoTracking(true, headNode);

		// Create the camera node & attach camera
        SceneNode* camNode = static_cast<SceneNode*>(mSceneMgr->getRootSceneNode()->createChild());
        camNode->attachObject(mCamera);
		
		// Create light node
        SceneNode* lightNode = static_cast<SceneNode*>(mSceneMgr->getRootSceneNode()->createChild());
		lightNode->attachLight(l);

        // set up spline animation of node
        Animation* anim = mSceneMgr->createAnimation("WaterTracks", 20);
        // Spline it for nice curves
        anim->setInterpolationMode(Animation::IM_SPLINE);
        // Create a track to animate the camera's node
        AnimationTrack* track = anim->createTrack(0, camNode);
        // Setup keyframes
        KeyFrame* key ;
		//~ = track->createKeyFrame(0); // startposition
        //~ key = track->createKeyFrame(2.5);
        //~ key->setTranslate(Vector3(500,500,-1000));
        //~ key = track->createKeyFrame(5);
        //~ key->setTranslate(Vector3(-1500,1000,-600));
        //~ key = track->createKeyFrame(7.5);
        //~ key->setTranslate(Vector3(0,200,0));
		Vector3 cpos0 ;
		for(int kf=0;kf<=16;kf+=2) {
			Vector3 cpos (
				rand()%(int)PLANE_SIZE,
				rand()%500+500,
				rand()%(int)PLANE_SIZE
				);
			key = track->createKeyFrame(kf);
			key->setTranslate(cpos);
			if (!kf) {
				cpos0=cpos ;
			}
		}
        key = track->createKeyFrame(20);
        key->setTranslate(cpos0);
		
		// create a random spline for light as well 
		track = anim->createTrack(1, lightNode);
		key = track->createKeyFrame(0);
		for(int ff=1;ff<=19;ff++) {
			key = track->createKeyFrame(ff);
			Vector3 lpos (
				rand()%(int)PLANE_SIZE , //- PLANE_SIZE/2,
				rand()%300+100,
				rand()%(int)PLANE_SIZE //- PLANE_SIZE/2
				);
			key->setTranslate(lpos);
		}
		key = track->createKeyFrame(20);
		
        // Create a new animation state to track this
        mAnimState = mSceneMgr->createAnimationState("WaterTracks");
        mAnimState->setEnabled(true);

        // Put in a bit of fog for the hell of it
        mSceneMgr->setFog(FOG_EXP, ColourValue::White, 0.0002);

		// show overlay
		waterOverlay = (Overlay*)OverlayManager::getSingleton().getByName("Example/WaterOverlay");    
		waterOverlay->show();
	}

    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new WaterListener(mWindow, mCamera, waterMesh, waterEntity);
        mRoot->addFrameListener(mFrameListener);
    }

};



#if OGRE_PLATFORM == PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"


INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
    WaterApplication app;
	
	srand(time(0));

    try {
        app.go();
    } catch( Exception& e ) {
#if OGRE_PLATFORM == PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        fprintf(stderr, "An exception has occured: %s\n",
                e.getFullDescription().c_str());
#endif
    }

    return 0;
}
