***Prerequisites***
-Mac OS 10.2
-You will need to have the Mac OS X Developer Tools installed.
 See http://developer.apple.com/tools/ for more information.
-You should download the latest 3rd party Frameworks from the 
site. These frameworks go in ogrenew/Mac/Frameworks.

***Building the Ogre Framework***
Open OgreFramework.pbproj in the ogrenew/Mac/PB/OgreFramework 
directory. Choose Build from the Build menu.

***Building the Samples***
You may build the individual samples in ogrenew/Mac/PB/Samples by
opening them and choosing build. You can batch-build the samples
by running the buildSamples script from the command line in that
directory. There is also a cleanSamples to batch clean them all.

***Using the Ogre Framework in your applications***
To create an application that uses the Ogre framework:
1) Build the Ogre frameworks as above.
2) Create a new project, using the Cocoa Application template
3) Delete the files main.m, MainMenu.nib and MyApp_Prefix.h.
4) Add SDLMain.m and SDLMain.h to your project. These are in the
ogrenew/Mac/PB/Classes folder. You probably want to copy these
items so you may change them without affecting the originals.
5) Add the freetype, SDL and Ogre frameworks to your project. The
first two are in the ogrenew/Mac/Frameworks folder, and the Ogre
framework will be in ogrenew/Mac/PB/OgreFramework/build after you
build it as above.

You may either place these in a standard location for frameworks
(eg /Library/Frameworks or ~/Library/Frameworks) or copy them into
your executable. To copy them into your executable, choose 'Edit
Active Target' from the project menu, control-click on the last
in the list of build phases and create a new Copy Files build 
phase. Choose 'Frameworks' from the 'Where' pop-up menu, and 
drag the Ogre, SDL and freetype frameworks to the 'Files' area.

If you copy them into the application, then users do not need
to install the Ogre, SDL or freetype frameworks before using
your application. On the other hand, the application is larger.

6) Create the required plugins.cfg and resources.cfg files. You 
may start from the examples of these files in ogrenew/Mac/PB/Samples.
The plugins.cfg can probably be left as is, but you will want to
remove the entries from resources.cfg, as they refer to the Sample
resources. The entries in resources.cfg are relative to the
'Resources' group in your application. By default, it and all of
its sub-directories are searched by Ogre.

7) There are some target settings to be changed. Choose
'Edit Current Target' from the 'Project' menu. Under 'GCC Compiler
Settings', clear the box that reads MyApp_Prefix.h. Under 'Search
Paths', you may wish to add the 'Headers' folder from the Ogre
framework to the 'Headers' section, so that you can refer to Ogre
headers as, eg, "OgreCamera.h" instead of <Ogre/OgreCamera.h>.

8) If you have built the Ogre framework with the Development build
style (the default), you will have to edit the Development build 
style of your Application to use the Ogre debug headers. To do this,
select 'Edit Current Build Style' from the 'Project' menu. Click the
add button, and fill in 'OTHER_CFLAGS' for the name and '-DDEBUG' for
the value, without the quotes.

9) Ogre requires a few resources for the debug overlays, so these 
should be added to the project, under Resources. The 
required files are all found in ogrenew/Samples/Media. They are:

New_Ogre_Border_Break.png
New_Ogre_Border_Center.png
New_Ogre_Border.png
New_Ogre_Logo.png
Ogre.material
Ogre.overlay
ogrelogo-small.jpg
sample.fontdef
trebucbd.ttf

10) You are now ready to add your own resources and source files.
Take a look at the Samples and the documentation on the ogre
website for guidance. You should be able to replicate the Samples
by adding all of the source files from the Sample to your application,
and adding the files from Media that it uses to your Resources.

*NB* The header Ogre.h must be included in the file that defines
your main method for SDL to work properly.