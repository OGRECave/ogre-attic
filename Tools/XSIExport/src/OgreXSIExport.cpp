/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
#include <xsi_value.h>
#include <xsi_status.h>
#include <xsi_application.h>
#include <xsi_plugin.h>
#include <xsi_pluginitem.h>
#include <xsi_pluginregistrar.h>
#include <xsi_pluginitem.h>
#include <xsi_command.h>
#include <xsi_argument.h>
#include <xsi_context.h>
#include <xsi_menuitem.h>
#include <xsi_menu.h>
#include <xsi_model.h>
#include <xsi_customproperty.h>
#include <xsi_ppglayout.h>
#include <xsi_ppgeventcontext.h>
#include <xsi_selection.h>
#include <xsi_comapihandler.h>
#include <xsi_uitoolkit.h>

#include "OgreXSIMeshExporter.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreXSIHelper.h"

using namespace XSI;

/** This is the main file for the OGRE XSI plugin.
The purpose of the methods in this file are as follows:

XSILoadPlugin
  registers the export command, the menu item, and the option dialog

XSIUnloadPlugin 
  cleans up

OgreMeshExportCommand_Init
  Defines the arguments to the export command

OgreMeshExportCommand_Execute
  Runs the exporter using arguments obtained from a context object
  (I assume this is to allow general access to this export rather than using
   the property dialog)

OgreMeshExportMenu_Init
  Defines the menu text and the event callback to execute (OnOgreMeshExportMenu)

OnOgreMeshExportMenu
  Callback event when clicking the export menu option. Adds an instance of the
  options dialog as a property, then uses the InspectObj XSI command to pop it up
  in a modal dialog. If it wasn't cancelled, performs an export.

OgreMeshExportOptions_Define
  Defines the persistable parameters on the options dialog

OgreMeshExportOptions_DefineLayout
  Defines the visual layout of the options dialog

OgreMeshExportOptions_PPGEvent
  Event handler for when the options dialog is interacted with
*/

CString GetUserSelectedObject();
CStatus Popup( const CString& in_inputobjs, const CString& in_keywords, const CString& in_title, const CValue& in_mode, bool in_throw );
void DeleteObj( const CValue& in_inputobj );


#ifdef unix
extern "C" 
#endif
/** Registers the export command, the menu item, and the option dialog */
CStatus XSILoadPlugin( XSI::PluginRegistrar& registrar )
{
	registrar.PutAuthor( L"Steve Streeting" );
	registrar.PutName( L"OGRE Exporter Plugin" );	
    registrar.PutVersion( 0, 15 );
    registrar.PutURL(L"http://www.ogre3d.org");
    

	// register the mesh export command
	registrar.RegisterCommand( L"OgreMeshExportCommand", L"OgreMeshExportCommand" );

    // register the menu under File > Export
	registrar.RegisterMenu(siMenuMainFileExportID, L"OgreMeshExportMenu", false, false);

	// register the export dialog properties factory
	registrar.RegisterProperty( L"OgreMeshExportOptions" );

#ifdef _DEBUG
    Application app;
    app.LogMessage( registrar.GetName() + L" has been loaded.");
#endif

    return XSI::CStatus::OK;	
}

#ifdef unix
extern "C" 
#endif
/** Cleans up */
XSI::CStatus XSIUnloadPlugin( const XSI::PluginRegistrar& registrar )
{
#ifdef _DEBUG
    Application app;
	app.LogMessage(registrar.GetName() + L" has been unloaded.");
#endif

	return XSI::CStatus::OK;
}

#ifdef unix
extern "C" 
#endif
/** Defines the arguments to the export command */
XSI::CStatus OgreMeshExportCommand_Init( const XSI::CRef& context )
{
	Context ctx(context);
	Command cmd(ctx.GetSource());

	Application app;
	app.LogMessage( L"Defining: " + cmd.GetName() );

	ArgumentArray args = cmd.GetArguments();

    args.Add( L"objectName", L"" );
    args.Add( L"exportMesh", L"true" );
	args.Add( L"targetMeshFileName", L"c:/default.mesh" );
	args.Add( L"calculateEdgeLists", L"true" );
    args.Add( L"calculateTangents", L"false" );
    args.Add( L"lodGeneration", L"" ); // complete if LOD generation required
    args.Add( L"exportSkeleton", L"true" );
    args.Add( L"targetSkeletonFileName", L"c:/default.skeleton" );
    args.Add( L"fps", L"24" );
    args.Add( L"animationSplit", L"" ); // complete if animation splitting required
	return XSI::CStatus::OK;

}

#ifdef unix
extern "C" 
#endif
/** Runs the exporter using arguments obtained from a context object
  (I assume this is to allow general access to this export rather than using
   the property dialog)
*/
XSI::CStatus OgreMeshExportCommand_Execute( XSI::CRef& in_context )
{
	Application app;
	Context ctxt(in_context);
	CValueArray args = ctxt.GetAttribute( L"Arguments" );

#ifdef _DEBUG
	for (long i=0; i<args.GetCount(); i++)
	{
		app.LogMessage( L"Arg" + CValue(i).GetAsText() + L": " + 
			args[i].GetAsText() );			
	}
#endif

	if ( args.GetCount() != 9 ) 
	{
		// Arguments of the command might not be properly registered
		return CStatus::InvalidArgument ;
	}

    // TODO - perform the export!

    return XSI::CStatus::OK;
}


#ifdef unix
extern "C" 
#endif
/** Defines the menu text and the event callback to execute (OnOgreMeshExportMenu) */
XSI::CStatus OgreMeshExportMenu_Init( XSI::CRef& in_ref )
{
	Context ctxt = in_ref;
	Menu menu = ctxt.GetSource();

	CStatus st;
	MenuItem item;
	menu.AddCallbackItem(L"OGRE Mesh / Skeleton...", L"OnOgreMeshExportMenu", item);

	return CStatus::OK;	
}

#ifdef unix
extern "C" 
#endif
/** Callback event when clicking the export menu option. Adds an instance of the
    options dialog as a property, then uses the InspectObj XSI command to pop it up
    in a modal dialog. If it wasn't cancelled, performs an export.
*/
XSI::CStatus OnOgreMeshExportMenu( XSI::CRef& in_ref )
{	
	Application app;
	CStatus st(CStatus::OK);
	Property prop = app.GetActiveSceneRoot().AddProperty( L"OgreMeshExportOptions" ) ;
	
	// Popup Returns true if the command was cancelled otherwise it returns false. 
	CStatus ret = Popup(L"OgreMeshExportOptions",CValue(),L"OGRE Mesh / Skeleton Export",(long)siModal,true);
    if (ret == CStatus::OK)
	{
        Ogre::XsiMeshExporter exporter;

        // retrieve the parameters
        Parameter param = prop.GetParameters().GetItem(L"objectName");
        CString objectName = param.GetValue();
        param = prop.GetParameters().GetItem( L"targetMeshFileName" );
        CString meshFileName = param.GetValue();
        param = prop.GetParameters().GetItem( L"calculateEdgeLists" );
        bool edgeLists = param.GetValue();
        param = prop.GetParameters().GetItem( L"calculateTangents" );
        bool tangents = param.GetValue();
        /* TODO
        "lodGeneration"
        "targetSkeletonFileName"
        "fps"
        "animationSplit"
        */

        try 
        {
            exporter.exportMesh(meshFileName, objectName, edgeLists, tangents);
        }
        catch (Ogre::Exception& e)
        {
            // Will already have been logged to the Ogre log manager
            // Tell XSI
            app.LogMessage(OgretoXSI(e.getFullDescription()), XSI::siErrorMsg);
        }

	}
	DeleteObj( L"OgreMeshExportOptions" );
	return st;	
}


#ifdef unix
extern "C" 
#endif
/** Defines the persistable parameters on the options dialog */
CStatus OgreMeshExportOptions_Define( const CRef & in_Ctx )
{
	// Here is where we add all the parameters to the 
	// Custom Property.  This will be called each time 
	// an new instance of the Custom Property is called.
	// It is not called when an persisted Custom Property is loaded.

    Application app ;
	CustomProperty prop = Context(in_Ctx).GetSource();
	Parameter param ;

	// Default capabilities for most of these parameters
	int caps = siPersistable  ;
	CValue nullValue;	// Used for arguments we don't want to set

    prop.AddParameter(	
        L"objectName",CValue::siString, caps, 
        L"Export Mesh", L"", 
        nullValue, param) ;	
	prop.AddParameter(	
        L"exportMesh",CValue::siBool, caps, 
		L"Export Mesh", L"", 
		CValue(true), param) ;	
	prop.AddParameter(	
        L"targetMeshFileName",CValue::siString, caps, 
		L"Mesh Filename", L"", 
		nullValue, param) ;	
    prop.AddParameter(	
        L"calculateEdgeLists",CValue::siBool, caps, 
        L"Calculate Edge Lists (stencil shadows)", L"", 
        CValue(true), param) ;	
    prop.AddParameter(	
        L"calculateTangents",CValue::siBool, caps, 
        L"Calculate Tangents (normal mapping)", L"", 
        CValue(false), param) ;	
    prop.AddParameter(	// TODO, review this
        L"lodGeneration",CValue::siString, caps, 
        L"LOD Generation", L"", 
        nullValue, param) ;	
    prop.AddParameter(	
        L"exportSkeleton",CValue::siBool, caps, 
        L"Export Skeleton", L"", 
        CValue(true), param) ;	
    prop.AddParameter(	
        L"targetSkeletonFileName",CValue::siString, caps, 
        L"Skeleton Filename", L"", 
        nullValue, param) ;	
    prop.AddParameter(	
        L"fps",CValue::siInt2, caps, 
        L"Frames per second", L"", 
        CValue(24l), param) ;	
    prop.AddParameter(	// TODO, review this
        L"animationSplit",CValue::siString, caps, 
        L"Split animations", L"", 
        nullValue, param) ;	


	return CStatus::OK;	
}

#ifdef unix
extern "C" 
#endif
/** Defines the visual layout of the options dialog */
CStatus OgreMeshExportOptions_DefineLayout( const CRef & in_Ctx )
{
	// XSI will call this to define the visual appearance of the CustomProperty
	// The layout is shared between all instances of the CustomProperty
	// and is cached.  You can force the code to re-execute by using the 
	// XSIUtils.Refresh feature.

	PPGLayout oLayout = Context( in_Ctx ).GetSource() ;
	PPGItem item ;

	oLayout.Clear() ;

    // Object picker
    oLayout.AddGroup(L"Object to Export");
    oLayout.AddRow();

    item = oLayout.AddItem(L"objectName");
    item.PutAttribute( siUINoLabel, true );
    item.PutWidthPercentage(80);
    item = oLayout.AddButton(L"Pick", L"Pick");
    item.PutWidthPercentage(1) ;

    oLayout.EndRow();
    oLayout.EndGroup();

	// Mesh group
    item = oLayout.AddGroup(L"Mesh");
    item = oLayout.AddItem(L"exportMesh") ;
    item = oLayout.AddItem(L"targetMeshFileName", L"Target", siControlFilePath);
    item.PutAttribute( siUINoLabel, true );
    item.PutAttribute( siUIFileFilter, L"OGRE Mesh format (*.mesh)|*.mesh|All Files (*.*)|*.*||" );

    item = oLayout.AddItem(L"calculateEdgeLists");
    item = oLayout.AddItem(L"calculateTangents");
    item = oLayout.AddItem(L"lodGeneration");

    oLayout.EndGroup();

    // Skeleton Group
    item = oLayout.AddGroup(L"Skeleton");

    item = oLayout.AddItem(L"exportSkeleton");
    item = oLayout.AddItem(L"targetSkeletonFileName", L"Target", siControlFilePath);
    item.PutAttribute( siUINoLabel, true );
    item.PutAttribute( siUIFileFilter, L"OGRE Skeleton format (*.skeleton)|*.skeleton|All Files (*.*)|*.*||" );
    item = oLayout.AddItem(L"fps");
    item = oLayout.AddItem(L"animationSplit");


    oLayout.EndGroup();


	return CStatus::OK;	
}

#ifdef unix
extern "C" 
#endif
/** Event handler for when the options dialog is interacted with */
CStatus OgreMeshExportOptions_PPGEvent( const CRef& io_Ctx )
{
	// This callback is called when events happen in the user interface
	// This is where you implement the "logic" code.

	Application app ;

	PPGEventContext ctx( io_Ctx ) ;

	PPGEventContext::PPGEvent eventID = ctx.GetEventID() ;

	CustomProperty prop = ctx.GetSource() ;	
	Parameter objectNameParam = prop.GetParameters().GetItem( L"objectName" ) ;
    // On open dialog
    if ( eventID == PPGEventContext::siOnInit )
	{
        // Pre-populate object with currently selected item
		Selection sel(app.GetSelection());
		if ( sel.GetCount() > 0 )
			prop.PutParameterValue( L"objectName", SIObject(sel[0]).GetName()); 
        // Make the selection read-only
		objectNameParam.PutCapabilityFlag( siReadOnly, true );

        // TODO - disable the export skeleton if no animation found
	}
    // On clicking a button
	else if ( eventID == PPGEventContext::siButtonClicked )
	{
		CValue buttonPressed = ctx.GetAttribute( L"Button" );	
        // Clicked the pick button
		if ( buttonPressed.GetAsText() == L"Pick" )
		{
			objectNameParam.PutCapabilityFlag( siReadOnly, false );
			prop.PutParameterValue( L"objectName", GetUserSelectedObject() ) ; 
		}
	}
    // Changed a parameter
	else if ( eventID == PPGEventContext::siParameterChange )
	{
		Parameter changed = ctx.GetSource() ;	
		CustomProperty prop = changed.GetParent() ;	
		CString   paramName = changed.GetScriptName() ; 

        // Check paramName against parameter names, perform custom onChanged event
	}


	return CStatus::OK;	

}

CString GetUserSelectedObject()
{
	Application app;
	Model root(app.GetActiveSceneRoot());
	CStringArray emptyArray;
	CRefArray cRefArray = root.FindChildren( L"", L"", emptyArray, true );

	CStringArray nameArray(cRefArray.GetCount());
	for ( long i=0; i < cRefArray.GetCount(); i++ )
	{
		nameArray[i] = SIObject(cRefArray[i]).GetName();
	}
	//todo qsort the nameArray

	// Using the COMAPIHandler for creating a "XSIDial.XSIDialog"
	CComAPIHandler xsidialog;
	xsidialog.CreateInstance( L"XSIDial.XSIDialog");
	CValue index;
	CValueArray args(cRefArray.GetCount());
	for (long y=0; y < cRefArray.GetCount(); y++)
		args[y]=nameArray[y];

	xsidialog.Call(L"Combo",index,L"Select Item",args );

	long ind = (long)index;
	return args[ind];
}


CStatus Popup( const CString& in_inputobjs, const CString& in_keywords, const CString& in_title, const CValue& /*number*/ in_mode, bool in_throw )
{
	Application app;
	CValueArray args(5);
	CValue retval;
	long i(0);

	args[i++]= in_inputobjs;
	args[i++]= in_keywords;
	args[i++]= in_title;
	args[i++]= in_mode;
	args[i++]= in_throw;

	return app.ExecuteCommand( L"InspectObj", args, retval );

}

void DeleteObj( const CValue& in_inputobj )
{
	Application app;
	CValueArray args(1);
	CValue retval;
	long i(0);

	args[i++]= in_inputobj;

	CStatus st = app.ExecuteCommand( L"DeleteObj", args, retval );

	return;
}
