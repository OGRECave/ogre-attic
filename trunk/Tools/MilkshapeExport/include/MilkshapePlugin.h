/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/

#ifndef __MilkshapePlugin_H__
#define __MilkshapePlugin_H__

#include "msPlugIn.h"
#include "windows.h"
#include "Ogre.h"


/** A plugin class for Milkshape3D to export to OGRE model formats. 
@remarks
    This class is the implementor of the exporting interface for Milkshape3D, to allow
    you to export your models to OGRE format from that tool.
    Note that this plugin delegates most of the detail of exporting the model to the
    generic model export framework.
*/
class MilkshapePlugin : public cMsPlugIn
{
    
    char mTitle[64];

public:
	MilkshapePlugin ();
    virtual ~MilkshapePlugin ();

public:
    /// As required by Milkshape
    int             GetType ();
    /// As required by Milkshape
    const char *    GetTitle ();
    /// As required by Milkshape
    int             Execute (msModel* pModel);

    /** Callback to process window events */
    static BOOL CALLBACK DlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

    bool exportMaterials;
    bool exportMesh;
    bool exportSkeleton;

protected:
    void showOptions(void);
    void doExportMesh(msModel* pModel);
    void doExportMaterials(msModel* pModel);
    Ogre::Skeleton* doExportSkeleton(msModel* pModel, Ogre::Mesh* mesh); // Skeleton returned for deletion later
    bool locateSkeleton(Ogre::Mesh* mesh);
};

#endif

