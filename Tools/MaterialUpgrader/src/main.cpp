/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
Also see acknowledgements in Readme.html

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
-----------------------------------------------------------------------------
*/


#include "Ogre.h"
#include "OgreDataChunk.h"
#include "OldMaterialReader.h"
#include "OgreMaterialManager.h"
#include "OgreMaterialSerializer.h"

#include <iostream>
#include <sys/stat.h>

using namespace std;

void help(void)
{
    // Print help message
    cout << endl << "OgreMaterialUpgrader: Upgrades .material files to the latest version." << endl;
    cout << "Provided for OGRE by Steve Streeting 2003" << endl << endl;
    cout << "Usage: OgreMaterialUpgrade sourcefile [destfile] " << endl;
    cout << "sourcefile = name of file to convert" << endl;
    cout << "destfile   = optional name of file to write to. If you don't" << endl;
    cout << "             specify this OGRE overwrites the existing file." << endl;

    cout << endl;
}


using namespace Ogre;

// Crappy globals
// NB some of these are not directly used, but are required to
//   instantiate the singletons used in the dlls
LogManager* logMgr;
Math* mth;
MaterialManager* matMgr;

int main(int numargs, char** args)
{
    if (numargs < 2)
    {
        help();
        return -1;
    }

    logMgr = new LogManager();
    mth = new Math();
    matMgr = new MaterialManager();;

    String source(args[1]);

    logMgr->createLog("OgreMaterialUpgrader.log");

    // Load the material
    struct stat tagStat;

    SDDataChunk chunk;
    stat( source, &tagStat );
    chunk.allocate( tagStat.st_size );
    FILE* pFile = fopen( source.c_str(), "r" );
    fread( (void*)chunk.getPtr(), tagStat.st_size, 1, pFile );
    fclose( pFile );

    // Read script, note this will create potentially many Materials and load them
    // into the MaterialManager
    OldMaterialReader reader;
    reader.parseScript(chunk);

    // Write out the converted mesh
    String dest;
    if (numargs == 3)
    {
        dest = args[2];
    }
    else
    {
        dest = source;
    }


    MaterialSerializer serializer;
    ResourceManager::ResourceMapIterator it = matMgr->getResourceIterator();
    while (it.hasMoreElements())
    {
        Material *m = static_cast<Material*>(it.getNext());
        serializer.queueForExport(m);
    }
    serializer.exportQueued(dest);
    
    delete matMgr;
    delete mth;
    delete logMgr;

    return 0;

}

