                    Instructions for building OGRE under Mac OSX
					============================================
                          Contributed by tcleu and jder


1) Install dlcompat (http://www.opendarwin.org/projects/dlcompat/). 
<./configre>, <make> then <make install) 

2) Install freetype2 (freetype.sourceforge.net) 
<./configure>, <make>, then <make install> 

3) Install fink 0.5.0a (http://fink.sourceforge.net/download/index.php) 

4) add "source /sw/bin/init.csh" in your ~/.tcshrc file (neccessary step to complete fink installation) 

5) optional: run <fink selfupdate> in a new terminal to to update the list of packages 

6) edit /sw/etc/fink.conf, add unstable/main and unstable/crypto to the beginning of the Trees: line 

7) Use fink to install the following packages 
<fink install libjpeg libpng pkgconfig sdl libtool14> 

8) run the following: 
"ln -s /sw/bin/glibtool /sw/bin/libtool" 

9) Get the latest cvs version of OGRE (see http://ogre.sourceforge.net) 

10) set the following using the setenv command: 
setenv ACLOCAL_FLAGS "-I /sw/share/aclocal" 
setenv CPPFLAGS "-I/usr/local/include -I/sw/include" 
setenv LDFLAGS "-L/usr/local/lib -L/sw/lib" 
setenv FT2_CONFIG "/usr/local/bin/freetype-config" 

11) run the following in the "ogrenew" dirctory: 
"./bootstrap" 
"./configure" 
"make" 
"make install" 

That's it! To run the samples, in the terminal, cd into ogrenew/Samples/Common/bin then run e.g "./CameraTrack"

