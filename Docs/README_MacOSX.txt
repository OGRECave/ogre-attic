QuickStart
----------
1)  Check your OS X and XCode install
    - OS X 10.2, minimum
    - Latest XCode (1.5) suggested
    - X11 Install Required (for freetype2 dylib)
2)  Retrieve the necessary external frameworks
    - SDL 1.2 (http://www.libsdl.org/download-1.2.php)
    - DevIL 1.6.7 (http://openil.sourceforge.net/download.php)
    - Cg 1.2 (http://developer.nvidia.com/object/cg_toolkit_1_2.html)
3)  Open and build Ogre.xcode
4)  Install the built Ogre.framework
5)  Open and build Samples.xcode
6)  Run samples and be happy.

FAQ
---

Q:  How to I build individual samples?
A:  In Samples.xcode select an individual app as the target rather than the
    aggregate target.  Build.

Q:  How to I run other samples?
A:  In Samples.xcode open the Run window (cmd-shift-R) and change the target
    executable in the toolbar.  Run.

Using the Ogre.framework in Your Applications
---------------------------------------------
1)  Build the Ogre.framework
2)  Create a new project, using the Cocoa Application template
3)  Delete the generated files.
4)  Add SDLMain.m and SDLMain.h to your project. These are in the
    ogrenew/Mac/XCode/Classes folder. You will  probably want to copy 
    these items so you may change them without affecting the originals.
5)  Add the SDL and Ogre frameworks to your project. 
    You can have these copied into your application when it is built, and then
    not have your users install these themselves.  For information on how to do
    this refer to the XCode documentation.
6)  Create the required plugins.cfg and resources.cfg files. 
    You may start from the examples of these files in 
    ogrenew/Mac/XCode/Samples.  The plugins.cfg can probably be left as is,
    but you will want to remove the entries from resources.cfg, as they refer
    to the Sample resources. The entries in resources.cfg are relative to the
    'Resources' group in your application. By default, it and all of its 
    sub-directories are searched by Ogre.
7)  If you have built the Ogre framework with the Development build
    style (the default), you will have to edit the Development build 
    style of your Application to use the Ogre debug headers. To do this,
    set your 'OTHER_CFLAGS' and 'OTHER_CXXFLAGS' to '-DDEBUG'.
10) You are now ready to add your own resources and source files.
    Take a look at the Samples and the documentation on the ogre
    website for guidance. You should be able to replicate the Samples
    by adding all of the source files from the Sample to your application,
    and adding the files from Media that it uses to your Resources.
