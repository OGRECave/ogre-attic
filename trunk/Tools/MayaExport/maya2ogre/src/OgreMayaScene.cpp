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
#include "OgreMayaScene.h"
#include "OgreMayaOptions.h"

#include <maya/MLibrary.h>
#include <maya/MFileIO.h>
#include <maya/MStatus.h>

#include <direct.h>
#include <iostream>

namespace OgreMaya {
	
	using namespace std;

	//	--------------------------------------------------------------------------
	/**	Standard constructor.
	*/	
	//	--------------------------------------------------------------------------
	SceneMgr::SceneMgr() {
		mbInitialized = false;			
	}


	//	--------------------------------------------------------------------------
	/**	Destructor. Cleans up Maya library if required.
	*/	
	//	--------------------------------------------------------------------------
	SceneMgr::~SceneMgr() {
		if (mbInitialized) {
			MLibrary::cleanup();
			mbInitialized = false;
		}
	}



	//	--------------------------------------------------------------------------
	/**	Loads a Maya scene file, initializing the Maya library first if required.
		\param		sFilename
					Path and filename of Maya scene file to load
		\return		True if file loaded successfully, False otherwise
	*/	
	//	--------------------------------------------------------------------------
	bool SceneMgr::load() {
		MStatus status;

		// Store working directory to restore later
		char szDir[300];
		_getcwd(szDir, 300);

		// Initialize Maya if required
		if (!mbInitialized) {
			cout << "SceneMgr: Initializing Maya...\n";

			status = MLibrary::initialize("Maya-to-Ogre", false);
			if (!status) {
				status.perror("MLibrary::initialize");
				return false;
			}
			mbInitialized = true;

			cout << "SceneMgr: Maya initialized\n";
        }

		// Prepare Maya to read a new scene file
		status = MFileIO::newFile(true);
		if (!status) {
			cout << "SceneMgr: MFileIO::newFile() failed\n";
			return false;
		}

		// Restore working directory
		_chdir(szDir);

		// Read the scene file
		status = MFileIO::open(OPTIONS.inFile.c_str());
		if (!status) {
			cout << "SceneMgr: MFileIO::open() failed:\n";
			return false;
		}
		cout << "SceneMgr: File " << OPTIONS.inFile.c_str() << " opened\n";

		// Done
		return true;
	}

}
