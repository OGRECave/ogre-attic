                    Instructions for building OGRE under Mac OSX
                    ============================================
                       Contributed by tcleu, mattias and jder


1) Install dlcompat (http://www.opendarwin.org/projects/dlcompat/). 
<./configre>, <make> then <make install) 

The version of dlcompat from fink does not work. 

2) Install freetype2 (freetype.sourceforge.net) v. 2.1.4 rc2 
<./configure>, <make>, then <make install> 

The version of freetype from fink does not work. 

3) Install fink 0.5.1 (http://fink.sourceforge.net/download/index.php) 

4) add "source /sw/bin/init.csh" in your ~/.tcshrc file (neccessary step to complete fink installation) 

5) Use fink to install the following packages 
<fink install libjpeg libpng pkgconfig sdl libtool14> 

6) run the following: 
"ln -s /sw/bin/glibtool /sw/bin/libtool" 

7) Install DevIL from http://homepage.mac.com/jrusak/DevIL.sit 
<./configure>, <make>, then <make install> 

8) Get the latest cvs version of OGRE (see http://ogre.sourceforge.net ) 

9) set the following using the setenv command: 
setenv ACLOCAL_FLAGS "-I /sw/share/aclocal" 
setenv CPPFLAGS "-I/usr/local/include -I/sw/include" 
setenv LDFLAGS "-L/usr/local/lib -L/sw/lib" 
setenv FT2_CONFIG "/usr/local/bin/freetype-config" 

10) run the following in the "ogrenew" dirctory: 
"./bootstrap" 
"./configure" 
"make" 
"make install" 

To run the samples, in the terminal, cd into ogrenew/Samples/Common/bin then run e.g "./CameraTrack"