/*
===============================================================================
This source file is part of the Ogre-Maya Tools.
Distributed as part of Ogre (Object-oriented Graphics Rendering Engine).
Copyright © 2003 Fifty1 Software Inc. 

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

    using namespace std;

	class Options {
	public:        
        static Options& Options::instance();

        void init(int argc, char** argv);

        void reset();

        bool isValid() {return valid;}

        void debugOutput();

    public:
        string
            inFile,
            outMeshFile,
            outSkelFile;

        bool
            exportSkeleton,
            exportNormals,
            exportColours,
            exportUVs;

    private:
        bool valid;
        
        char** argv;
        int argc;
        int currentArg;

        map<string, void (Options::*)(void)> builderMap;

    private:
        void parseIn();
        void parseMeshOut();
        void parseSkelOut();
        void parseS();
        void parseN();
        void parseC();
        void parseT();

    private:
        Options();
    };

} // namespace OgreMaya

#define OPTIONS Options::instance()

#endif