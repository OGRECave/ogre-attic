/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
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
#ifndef __PlatformManager_H__
#define __PlatformManager_H__

#include "OgrePrerequisites.h"

#include "OgreSingleton.h"

namespace Ogre {
    typedef void (*DLL_CREATECONFIGDIALOG)(ConfigDialog** ppDlg);
    typedef void (*DLL_CREATEERRORDIALOG)(ErrorDialog** ppDlg);
    typedef void (*DLL_CREATEINPUTREADER)(InputReader** ppReader);
	typedef void (*DLL_CREATETIMER)(Timer** ppTimer);

    typedef void (*DLL_DESTROYCONFIGDIALOG)(ConfigDialog* ppDlg);
    typedef void (*DLL_DESTROYERRORDIALOG)(ErrorDialog* ppDlg);
    typedef void (*DLL_DESTROYINPUTREADER)(InputReader* ppReader);
    typedef void (*DLL_DESTROYTIMER)(Timer* ppTimer);

    /** Class which manages the platform settings Ogre runs on.
        @remarks
            Because Ogre is designed to be platform-independent, it
            dynamically loads a library containing all the platform-specific
            elements like dialogs etc. 
        @par
            This class manages that load and provides a simple interface to
            the platform.
    */
    class _OgreExport PlatformManager : public Singleton<PlatformManager>
    {
    protected:
        DLL_CREATECONFIGDIALOG mpfCreateConfigDialog;
        DLL_CREATEERRORDIALOG mpfCreateErrorDialog;
        DLL_CREATEINPUTREADER mpfCreateInputReader;
		DLL_CREATETIMER mpfCreateTimer;

        DLL_DESTROYCONFIGDIALOG mpfDestroyConfigDialog;
        DLL_DESTROYERRORDIALOG mpfDestroyErrorDialog;
        DLL_DESTROYINPUTREADER mpfDestroyInputReader;
        DLL_DESTROYTIMER mpfDestroyTimer;
		
    public:
        /** Default constructor.
        */
        PlatformManager();

        /** Gets a new instance of a platform-specific config dialog.
            @remarks
                The instance returned from this method will be a
                platform-specific subclass of ConfigDialog, and must be
                destroyed by the caller when required.
        */
        ConfigDialog* createConfigDialog();

        /** Destroys an instance of a platform-specific config dialog.
            @remarks
                Required since deletion of objects must be performed on the
                correct heap.
        */
        void destroyConfigDialog(ConfigDialog* dlg);

        /** Gets a new instance of a platform-specific config dialog.
            @remarks
                The instance returned from this method will be a
                platform-specific subclass of ErrorDialog, and must be
                destroyed by the caller when required.
        */
        ErrorDialog* createErrorDialog();

        /** Destroys an instance of a platform-specific error dialog.
            @remarks
                Required since deletion of objects must be performed on the
                correct heap.
        */
        void destroyErrorDialog(ErrorDialog* dlg);

        /** Gets a new instance of a platform-specific input reader.
            @remarks
                The instance returned from this method will be a
                platform-specific subclass of InputReader, and must be
                destroyed by the caller when required.
        */
        InputReader* createInputReader();

        /** Destroys an instance of a platform-specific input reader.
            @remarks
                Required since deletion of objects must be performed on the
                correct heap.
        */
        void destroyInputReader(InputReader* reader);
		
		/** Creates a new Timer instance
		*/
		Timer* createTimer();

        /** Destroys an instance of a timer. */
        void destroyTimer(Timer* timer);


    };


}

#endif
