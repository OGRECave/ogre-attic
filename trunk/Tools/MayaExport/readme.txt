Here is the source code for the OGRE Exporter for Maya

Direcories:
+-- include: .h files containing class interfaces
|
+-- mel: .mel scripts and other files to place in the maya/scripts directory. If you already have a userSetup.mel, 
|        just copy and paste to append these extra lines.
|
+-- project: visual studio 6.0 scripts
|
+-- src: source files containing classes implementation

Compiling:
use the maya2ogre.dsw file to open the visual studio project,
you'll have to change some paths in the project script:

include paths:
- C:\Program Files\Alias\Maya6.5\include : change this to your maya include path, you can use different versions of maya too

link paths:
- C:\Program Files\Alias\Maya6.5\lib : change this to your maya lib path, same as above for other versions of maya.


!!!Important note!!!!
Remember to change priority of stlport directory to lowest possible, otherwise it won't compile or it will require stlport.dll
for running.


If you have any problems check on OGRE forum or e-mail to fra.giordana@tiscali.it