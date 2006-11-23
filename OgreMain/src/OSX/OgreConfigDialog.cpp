#include <Quicktime/Quicktime.h>
#include "OgreConfigDialog.h"
#include "OgreException.h"
#include "OgreImage.h"
#include "OgreLogManager.h"

namespace Ogre {

	ConfigDialog* dlg = NULL;
	static EventHandlerUPP WindowEventHandlerUPP;

	ConfigDialog::ConfigDialog() : iSelectedRenderSystem( NULL ), iVideoModeRef( NULL ), iLogoViewRef( NULL ), iWindowRef( NULL ), iNibRef( NULL ), iMenuRef( NULL )
	{
		LogManager::getSingleton().logMessage( "ConfigDialog::ConfigDialog()" );
		dlg = this;
	}
	
	ConfigDialog::~ConfigDialog()
	{
		LogManager::getSingleton().logMessage( "ConfigDialog::~OSXContext()" );
	}
	
	void ConfigDialog::initialise()
	{
		LogManager::getSingleton().logMessage( "ConfigDialog::initialise()" );
		Root::getSingleton().restoreConfig();
		RenderSystemList* renderers = Root::getSingleton().getAvailableRenderers();
		RenderSystem* renderer = renderers->front();
		ConfigOptionMap config = renderer->getConfigOptions();

		ConfigOptionMap::iterator cfi;
		
		cfi = config.find( "Full Screen" );
		if( cfi != config.end() )
		{
			if( cfi->second.currentValue == "Yes" )
			{
				LogManager::getSingleton().logMessage( "CONFIG <= FullScreen [ Yes ]" );
				SetControlValue( iFullScreenRef, 1 );
			}
			else
			{
				LogManager::getSingleton().logMessage( "CONFIG <= FullScreen [ No ]" );
				SetControlValue( iFullScreenRef, 2 );
			}
		}

		cfi = config.find( "FSAA" );
		if( cfi != config.end() )
		{
			if( cfi->second.currentValue == "0" )
			{
				LogManager::getSingleton().logMessage( "CONFIG <= FSAA [ 0 ]" );
				SetControlValue( iFSAARef, 1 );
			}
			else if( cfi->second.currentValue == "2" )
			{
				LogManager::getSingleton().logMessage( "CONFIG <= FSAA [ 2 ]" );
				SetControlValue( iFSAARef, 2 );
			}
			else if( cfi->second.currentValue == "4" )
			{
				LogManager::getSingleton().logMessage( "CONFIG <= FSAA [ 4 ]" );
				SetControlValue( iFSAARef, 3 );
			}
			else if( cfi->second.currentValue == "6" )
			{
				LogManager::getSingleton().logMessage( "CONFIG <= FSAA [ 6 ]" );
				SetControlValue( iFSAARef, 4 );
			}
		}

		cfi = config.find( "Colour Depth" );
		if( cfi != config.end() )
		{
			if( cfi->second.currentValue == "32" )
			{
				LogManager::getSingleton().logMessage( "CONFIG <= Depth [ 32 ]" );
				SetControlValue( iColorDepthRef, 1 );
			}
			else
			{
				LogManager::getSingleton().logMessage( "CONFIG <= Depth [ 16 ]" );
				SetControlValue( iColorDepthRef, 2 );
			}
		}
		
		cfi = config.find( "RTT Preferred Mode" );
		if( cfi != config.end() )
		{
			if( cfi->second.currentValue == "FBO" )
			{
				LogManager::getSingleton().logMessage( "CONFIG <= RTT Mode [ FBO ]" );
				SetControlValue( iRTTPrefModeRef, 1 );
			}
			else if( cfi->second.currentValue == "PBuffer" )
			{
				LogManager::getSingleton().logMessage( "CONFIG <= RTT Mode [ PBuffer ]" );
				SetControlValue( iRTTPrefModeRef, 2 );
			}
			else if( cfi->second.currentValue == "Copy" )
			{
				LogManager::getSingleton().logMessage( "CONFIG <= RTT Mode [ Copy ]" );
				SetControlValue( iRTTPrefModeRef, 3 );
			}
		}		
	}
	
	void ConfigDialog::run()
	{
		LogManager::getSingleton().logMessage( "ConfigDialog::run()" );

		RenderSystemList* renderers = Root::getSingleton().getAvailableRenderers();
		RenderSystem* renderer = renderers->front();

		SInt16 value = 0;

		// temp
		value = GetControlValue( iVideoModeRef );
		renderer->setConfigOption( "Video Mode", "800 x 600" );

		// full screen
		value = GetControlValue( iFullScreenRef );
		if( value == 1 ) // Yes
		{
			LogManager::getSingleton().logMessage( "CONFIG => FullScreen [ Yes ]" );
			renderer->setConfigOption( "Full Screen", "Yes" );
		}
		else
		{
			LogManager::getSingleton().logMessage( "CONFIG => FullScreen [ No ]" );
			renderer->setConfigOption( "Full Screen", "No" );
		}
		
		// fsaa
		value = GetControlValue( iFSAARef );
		switch( value )
		{
			case 1:
				LogManager::getSingleton().logMessage( "CONFIG => FSAA [ 0 ]" );
				renderer->setConfigOption( "FSAA", "0" );
				break;
			case 2:
				LogManager::getSingleton().logMessage( "CONFIG => FSAA [ 2 ]" );
				renderer->setConfigOption( "FSAA", "2" );
				break;
			case 3:
				LogManager::getSingleton().logMessage( "CONFIG => FSAA [ 4 ]" );
				renderer->setConfigOption( "FSAA", "4" );
				break;
			case 4:
				LogManager::getSingleton().logMessage( "CONFIG => FSAA [ 6 ]" );
				renderer->setConfigOption( "FSAA", "6" );
				break;
			default:
				LogManager::getSingleton().logMessage( "CONFIG => FSAA [ 0 ] ( default )" );
				renderer->setConfigOption( "FSAA", "0" );
				break;
		}

		// fsaa
		value = GetControlValue( iColorDepthRef );
		if( value == 1 )
		{
			LogManager::getSingleton().logMessage( "CONFIG => Depth [ 32 ]" );
			renderer->setConfigOption( "Colour Depth", "32" );
		}
		else
		{
			LogManager::getSingleton().logMessage( "CONFIG => Depth [ 16 ]" );
			renderer->setConfigOption( "Colour Depth", "16" );
		}

		// rtt pref mode
		value = GetControlValue( iRTTPrefModeRef );
		switch( value )
		{
			case 1:
				LogManager::getSingleton().logMessage( "CONFIG => RTT Mode [ FBO ]" );
				renderer->setConfigOption( "RTT Preferred Mode", "FBO" );
				break;
			case 2:
				LogManager::getSingleton().logMessage( "CONFIG => RTT Mode [ PBuffer ]" );
				renderer->setConfigOption( "RTT Preferred Mode", "PBuffer" );
				break;
			case 3:
				LogManager::getSingleton().logMessage( "CONFIG => RTT Mode [ Copy ]" );
				renderer->setConfigOption( "RTT Preferred Mode", "Copy" );
				break;
		}

		Root::getSingleton().setRenderSystem( renderer );
		Root::getSingleton().saveConfig();

		iDisplayStatus = true;

		QuitAppModalLoopForWindow( iWindowRef );
	}
	
	void ConfigDialog::cancel()
	{
		LogManager::getSingleton().logMessage( "ConfigDialog::cancel()" );
		iDisplayStatus = false;
		QuitAppModalLoopForWindow( iWindowRef );
	}
	
	pascal OSStatus ConfigDialog::windowEventHandler( EventHandlerCallRef aNextHandler, EventRef aEvent, void* aUserData )
	{
		LogManager::getSingleton().logMessage( "ConfigDialog::windowEventHandler()" );
		#pragma unused ( inCallRef )
		OSStatus status	= eventNotHandledErr;
		UInt32 eventKind = GetEventKind( aEvent );
		UInt32 eventClass = GetEventClass( aEvent );
		WindowRef window = ( WindowRef ) aUserData;
		HICommand command;
		if( eventClass == kEventClassWindow && eventKind == kEventWindowClose )
		{
			LogManager::getSingleton().logMessage( "ConfigDialog::windowEventHandler() => kEventWindowClose" );
			QuitAppModalLoopForWindow( window );
		}
		else if( eventClass == kEventClassCommand && eventKind == kEventCommandProcess )
		{
			GetEventParameter( aEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( HICommand ), NULL, &command );
			switch( command.commandID )
			{
				case 'run!':
					{
					dlg->run();
					break;
					}
				case 'not!':
					{
					dlg->cancel();
					break;
					}
				default:
					break;
			}
		}
		return ( status );
	}

	bool ConfigDialog::display()
	{		
		LogManager::getSingleton().logMessage( "ConfigDialog::display()" );
		iDisplayStatus = false;
		OSStatus status;
		CFStringRef logoRef = NULL;
		CFURLRef logoUrlRef = NULL;
		OSType dataRefType;
		GraphicsImportComponent	ci = NULL;
		CGImageRef cgImageRef = NULL;
		Handle dataRef = NULL;

		HIViewID logoViewID = { 'CONF', 100 };
		HIViewID videoModeViewID = { 'CONF', 104 };
		HIViewID colorDepthViewID = { 'CONF', 105 };
		HIViewID fsaaViewID = { 'CONF', 106 };
		HIViewID rttPrefModeViewID = { 'CONF', 107 };
		HIViewID fullScreenViewID = { 'CONF', 108 };

		const EventTypeSpec windowEvents[] =
		{
			{ kEventClassCommand, kEventCommandProcess },
			{ kEventClassWindow, kEventWindowClose }
		};

		status	= CreateNibReference( CFSTR( "main" ), &iNibRef );
		require_noerr( status, CantGetNibRef );

		status	= CreateWindowFromNib( iNibRef, CFSTR( "ConfigWindow" ), &iWindowRef );
		require_noerr( status, CantCreateWindow );

		if( WindowEventHandlerUPP == NULL )
			WindowEventHandlerUPP = NewEventHandlerUPP( windowEventHandler );

		status	= InstallWindowEventHandler( iWindowRef, WindowEventHandlerUPP, GetEventTypeCount( windowEvents ), windowEvents, iWindowRef, NULL );
		require_noerr( status, CantInstallWindowEventHandler );

		//logoRef = CFStringCreateWithCString( kCFAllocatorDefault, "file:///Users/aljen/Code/OSXConfig/logo.bmp", NULL );
		//logoUrlRef = CFURLCreateWithString( kCFAllocatorDefault, logoRef, NULL );

		//status = QTNewDataReferenceFromCFURL( logoUrlRef, 0, &dataRef, &dataRefType );
		//status	= GetGraphicsImporterForDataRef( dataRef, dataRefType, &ci );
		//require( ci != NULL, ImporterError );
		//status	= GraphicsImportCreateCGImage( ci, &cgImageRef, kGraphicsImportCreateCGImageUsingCurrentSettings );
		
		// logo view
		status	= HIViewFindByID( HIViewGetRoot( iWindowRef ), logoViewID, &iLogoViewRef );
		require_noerr( status, LogoViewNotFound );
		status	= HIImageViewSetImage( iLogoViewRef, cgImageRef );
		require_noerr( status, SetImageFailed );
		status	= HIImageViewSetScaleToFit( iLogoViewRef, false );
		require_noerr( status, ScaleFailed );
		status	= HIViewSetVisible( iLogoViewRef, true );
		require_noerr( status, SetVisibleFailed );
		
		// video mode view
		status	= HIViewFindByID( HIViewGetRoot( iWindowRef ), videoModeViewID, &iVideoModeRef );
		CreateNewMenu( iMenuID, 0, &iMenuRef );

		if( iMenuRef != NULL )
		{
			CFStringRef itemNames[] =
			{
				CFSTR( "640 x 480" ),
				CFSTR( "800 x 600" ),
				CFSTR( "1024 x 768" )
			};

			MenuItemIndex numItems = ( sizeof( itemNames ) / sizeof( CFStringRef ) );
			for( int i = 0; i < numItems; i++ )
			{
				MenuItemIndex newItem;
				AppendMenuItemTextWithCFString( iMenuRef, itemNames[ i ], 0, 0, &newItem );
			}
			
			SetControlData( iVideoModeRef, kControlEntireControl, kControlPopupButtonMenuRefTag, sizeof( MenuRef ), &iMenuRef );
		}

		// color depth view
		status = HIViewFindByID( HIViewGetRoot( iWindowRef ), colorDepthViewID, &iColorDepthRef );

		// fsaa view
		status = HIViewFindByID( HIViewGetRoot( iWindowRef ), fsaaViewID, &iFSAARef );

		// rtt pref mode view
		status = HIViewFindByID( HIViewGetRoot( iWindowRef ), rttPrefModeViewID, &iRTTPrefModeRef );

		// full screen view
		status = HIViewFindByID( HIViewGetRoot( iWindowRef ), fullScreenViewID, &iFullScreenRef );

		DisposeNibReference( iNibRef );

		initialise();
		
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => RepositionWindow()" );
		RepositionWindow( iWindowRef, NULL, kWindowCenterOnMainScreen );
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => ShowWindow()" );
		TransitionWindow( iWindowRef, kWindowFadeTransitionEffect, kWindowShowTransitionAction, NULL );
		SelectWindow( iWindowRef );
		ActivateWindow( iWindowRef, true );
		RunAppModalLoopForWindow( iWindowRef );

		LogManager::getSingleton().logMessage( "ConfigDialog::display() => HideWindow" );
		TransitionWindow( iWindowRef, kWindowFadeTransitionEffect, kWindowHideTransitionAction, NULL );
		DisposeWindow( iWindowRef );

		return iDisplayStatus;

	CantGetNibRef:
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => err:CantGetNibRef" );
	CantCreateWindow:
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => err:CantCreateWindow" );
	CantInstallWindowEventHandler:
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => err:CantInstallWindowEventHandler" );
	ImporterError:
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => err:ImporterError" );
	LogoViewNotFound:
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => err:LogoViewNotFound" );
	SetImageFailed:
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => err:SetImageFailed" );
	ScaleFailed:
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => err:SetScaleFailed" );
	SetVisibleFailed:
		LogManager::getSingleton().logMessage( "ConfigDialog::display() => err:SetVisibleFailed" );
		return( iDisplayStatus );

	/*
		GLXConfigurator test;
		Root::getSingleton().restoreConfig();
		if(Root::getSingleton().getRenderSystem())
			test.SetRenderSystem(Root::getSingleton().getRenderSystem());
		if(!test.CreateWindow())
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Could not create configuration dialog",
				   "GLXConfig::display");

		// Modal loop
		test.Main();
		if(!test.accept) // User did not accept
			return false;

		Root::getSingleton().setRenderSystem(test.mRenderer);
		Root::getSingleton().saveConfig();
	*/

		return true;
	}

};
