// Include the TutorialApplication header
#include "TutorialApplication.h"

// We are always windows, no need for checks
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// The nasty WinMain
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR
   strCmdLine, INT )
{
   // Start the application
   try 
   {
      // Create an instance of our application
      TutorialApplication TutorialApp;

      // Run the Application
      TutorialApp.go();
   } 
   catch ( Ogre::Exception &error ) 
   {
      // An exception has occured
      MessageBox( NULL, error.getFullDescription().c_str(), 
         "An exception has occured!",
         MB_OK | MB_ICONERROR | MB_TASKMODAL );
   }
   return 0;
}
