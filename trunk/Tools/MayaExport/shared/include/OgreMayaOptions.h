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
#ifndef _OGREMAYA_OPTIONS_H_
#define	_OGREMAYA_OPTIONS_H_

#include "OgreMayaCommon.h"

#include <maya/MString.h>
#include <maya/MStringArray.h>

#include <map>
#include <string>

namespace OgreMaya {

//    using namespace std;
	using std::map;
	using std::string;


	class Options {
	public:        
        static Options& Options::instance();

        void init(int argc, char** argv);

        void reset();

        bool isValid() {return valid;}

        void debugOutput();

    public:
        struct KeyframeRange {
            KeyframeRange(int from=0, int to=0, int step=1): from(from), to(to), step(1) {}
            bool isValid() {return from>0 && to>0;}
            int from;
            int to;
            int step;
        };

        typedef map<string, void (Options::*)(void)> BuilderMap;
        typedef map<string, KeyframeRange>           KeyframeRangeMap;

        string
            inFile,
            
            outMeshFile,
            outSkelFile,
            outMatFile,           

            matPrefix;

        bool
            verboseMode,

            exportMesh,
			exportSkeleton,
            exportVBA,
			exportNormals,
            exportColours,
            exportUVs,
            exportMaterial;

        KeyframeRangeMap
            animations;

    private:
        bool valid;
        
        char** argv;
        int argc;
        int currentArg;

        BuilderMap builderMap;

    private:
        /** returns true if next token starts with '-' */
        bool isNextTokenOption();

    private:        
        void parseIn();
        void parseMeshOut();
        void parseSkelOut();
        void parseMatOut();
        void parseMatPrefix();
        void parseAnimation();
        void parseVBA();
        void parseN();
        void parseC();
        void parseT();
        void parseV();

    private:
        Options();
    };

} // namespace OgreMaya

#define OPTIONS Options::instance()

#endif