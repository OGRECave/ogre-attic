/*
===============================================================================
This source file is part of the Ogre-Maya Tools.
Distributed as part of Ogre (Object-oriented Graphics Rendering Engine).

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
===============================================================================
*/

#include <OgreMayaOptions.h>
#include <iostream>

namespace OgreMaya {

    using namespace std;

    Options::Options() {

        reset();

        // init parameter map
        builderMap["-in"     ] = &Options::parseIn;
        builderMap["-mesh"   ] = &Options::parseMeshOut;
        builderMap["-vba"    ] = &Options::parseVBA;
        builderMap["-skel"   ] = &Options::parseSkelOut;
        builderMap["-mat"    ] = &Options::parseMatOut;
        builderMap["-mprefix"] = &Options::parseMatPrefix;
        builderMap["-anim"   ] = &Options::parseAnimation;
        builderMap["-m"      ] = &Options::parseM;
        builderMap["-s"      ] = &Options::parseS;
        builderMap["-n"      ] = &Options::parseN;
        builderMap["-c"      ] = &Options::parseC;
        builderMap["-t"      ] = &Options::parseT;
    }

    void Options::reset() {
        valid          = false;

        exportMesh     = false;
		exportSkeleton = false;
		exportVBA      = false;
		exportNormals  = false;
        exportColours  = false;
        exportUVs      = false;
        exportMaterial = false;

        inFile         = "";
        outMeshFile    = "";
        outSkelFile    = "";
        outMatFile     = "";

        matPrefix      = "";

        animations.clear();        

        argv           = 0;
        argc           = 0;
        currentArg     = 0;
    }


    Options& Options::instance() {
        static Options options;
        return options;
    }

    void Options::init(int argc, char** argv) {
        this->argv = argv;
        this->argc = argc;
        for(currentArg=1; currentArg<argc; currentArg++) {
            string arg = argv[currentArg];
            void (Options::*p)(void) = builderMap[arg];

            if(p) {
                (this->*p)();
            }
        }
    }

    void Options::debugOutput() {
        cout << "=== options ================================\n";
        cout << inFile << " -> mesh=" << outMeshFile << ", skel=" << outSkelFile << "\n";
        cout << "Material: prefix=" << matPrefix << ", file=" << outMatFile << "\n";
        cout << "exportMesh     :" << exportMesh << '\n';
        cout << "exportSkeleton :" << exportSkeleton << '\n';
        cout << "exportNormals  :" << exportNormals << '\n';
        cout << "exportColours  :" << exportColours << '\n';
        cout << "exportUVs      :" << exportUVs << '\n';        
        cout << "exportMaterial :" << exportMaterial << '\n';   
        cout << "============================================\n";
    }

    void Options::parseIn() {
        if(++currentArg < argc) {            
            inFile = argv[currentArg];
            int i = inFile.find_first_of('.');

            if(i>=0) {
                outMeshFile = inFile.substr(0, i) + ".mesh.xml";
                outSkelFile = inFile.substr(0, i) + ".skeleton.xml";
                outMatFile  = inFile.substr(0, i) + ".material";
            }
            else {
                outMeshFile = inFile + ".mesh.xml";
                outSkelFile = inFile + ".skeleton.xml";
                outMatFile  = inFile + ".material";
            }

            valid = true;
        }
    }

    void Options::parseMeshOut() {
        if(++currentArg < argc) {
			exportMesh = true;
            outMeshFile = argv[currentArg];
        }
    }

    void Options::parseSkelOut() {
        if(++currentArg < argc) {
            exportSkeleton = true;
            outSkelFile = argv[currentArg];
        }
    }

    void Options::parseMatOut() {
        if(++currentArg < argc) {
            exportMaterial = true;
            outMatFile = argv[currentArg];
        }
    }

    void Options::parseMatPrefix() {
        if(++currentArg < argc) {
//            exportMaterial = true;
            matPrefix = argv[currentArg];
        }
    }

    void Options::parseAnimation() {
        if(currentArg+4 < argc) {
            string name = argv[currentArg+1];
            int from    = atoi(argv[currentArg+2]);
            int to      = atoi(argv[currentArg+3]);
            int step    = atoi(argv[currentArg+4]);

            animations[name].from = from;
            animations[name].to   = to;
            animations[name].step = step;

            currentArg += 5;
        }
    }
    
    void Options::parseM() {
        exportMaterial = true;
    }

    void Options::parseS() {
        exportSkeleton = true;
    }

    void Options::parseVBA() {
        exportVBA = true;
    }

    void Options::parseN() {
        exportNormals = true;
    }

    void Options::parseC() {
        exportColours = true;
    }

    void Options::parseT() {
        exportUVs = true;
    }
    
} // namespace OgreMaya
