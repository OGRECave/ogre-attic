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
#include <xsi_time.h>
#include <xsi_griddata.h>
#include <xsi_gridwidget.h>

#include "OgreXSIMeshExporter.h"
#include "OgreXSISkeletonExporter.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreXSIHelper.h"
#include "OgreProgressiveMesh.h"
#include "OgreString.h"
#include "OgreLogManager.h"
#include "OgreMeshManager.h"
#include "OgreSkeletonManager.h"
#include "OgreDefaultHardwareBufferManager.h"

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
    registrar.PutVersion( 1, 0 );
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
	Ogre::LogManager logMgr;
	logMgr.createLog("OgreXSIExporter.log", true);
	
	try
	{
		// Popup Returns true if the command was cancelled otherwise it returns false. 
		CStatus ret = Popup(L"OgreMeshExportOptions",CValue(),L"OGRE Mesh / Skeleton Export",(long)siModal,true);
		if (ret == CStatus::OK)
		{
			Ogre::XsiMeshExporter meshExporter;
			Ogre::XsiSkeletonExporter skelExporter;

			// retrieve the parameters
			Parameter param = prop.GetParameters().GetItem(L"objectName");
			CString objectName = param.GetValue();
			param = prop.GetParameters().GetItem( L"targetMeshFileName" );
			Ogre::String meshFileName = XSItoOgre(param.GetValue());
			if (meshFileName.empty())
			{
				OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, 
					"You must supply a mesh file name", 
					"OGRE Exporter");
			}
			param = prop.GetParameters().GetItem( L"mergeSubmeshes" );
			bool mergeSubmeshes = param.GetValue();
			param = prop.GetParameters().GetItem( L"exportChildren" );
			bool exportChildren = param.GetValue();
			param = prop.GetParameters().GetItem( L"calculateEdgeLists" );
			bool edgeLists = param.GetValue();
			param = prop.GetParameters().GetItem( L"calculateTangents" );
			bool tangents = param.GetValue();
			param = prop.GetParameters().GetItem( L"numLodLevels" );
			long numlods = param.GetValue();
			Ogre::XsiMeshExporter::LodData* lodData = 0;
			if (numlods > 0)
			{
				param = prop.GetParameters().GetItem( L"lodDistanceIncrement" );
				float distanceInc = param.GetValue();

				param = prop.GetParameters().GetItem(L"lodQuota");
				CString quota = param.GetValue();

				param = prop.GetParameters().GetItem(L"lodReduction");
				float reduction = param.GetValue();

				lodData = new Ogre::XsiMeshExporter::LodData;
				float currentInc = distanceInc;
				for (int l = 0; l < numlods; ++l)
				{
					lodData->distances.push_back(currentInc);
					currentInc += distanceInc;
				}
				lodData->quota = (quota == L"p") ?
					Ogre::ProgressiveMesh::VRQ_PROPORTIONAL : Ogre::ProgressiveMesh::VRQ_CONSTANT;
				if (lodData->quota == Ogre::ProgressiveMesh::VRQ_PROPORTIONAL)
					lodData->reductionValue = reduction * 0.01;
				else
					lodData->reductionValue = reduction;

			}

			param = prop.GetParameters().GetItem( L"exportSkeleton" );
			bool exportSkeleton = param.GetValue();


			Ogre::ResourceGroupManager rgm;
			Ogre::MeshManager meshMgr;
			Ogre::SkeletonManager skelMgr;;
			Ogre::DefaultHardwareBufferManager hardwareBufMgr;

			logMgr.createLog("OgreXSIExport.log", true);
			
			if (exportSkeleton)
			{
				param = prop.GetParameters().GetItem( L"targetSkeletonFileName" );
				Ogre::String skeletonFileName = XSItoOgre(param.GetValue());
				if (skeletonFileName.empty())
				{
					OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, 
						"You must supply a skeleton file name", 
						"OGRE Exporter");
				}
				param = prop.GetParameters().GetItem( L"fps" );
				float fps = param.GetValue();
				if (fps == 0.0f)
				{
					OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, 
						"You must supply a valid value for 'FPS'", 
						"OGRE Export");
				}

				param = prop.GetParameters().GetItem( L"animationSplit" );
				GridData gd = param.GetValue();
				if (gd.GetRowCount() == 0)
				{
					OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, 
						"You must supply at least one entry in 'Animations'", 
						"OGRE Export");
				}
				Ogre::AnimationList animList;
				int maxFrame = 0;
				for (int a = 0; a < gd.GetRowCount(); ++a)
				{
					Ogre::AnimationEntry anim;
					anim.animationName = XSItoOgre(CString(gd.GetCell(0, a)));
					if (anim.animationName.empty())
					{
						// Finished
						break;
					}
					anim.startFrame = gd.GetCell(1, a);
					if (anim.startFrame == 0 || anim.startFrame <= maxFrame)
					{
						OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, 
							"Start frame must be greater than zero and higher than "
							"any previous animation frame", 
							"OGRE Export");
					}
					if (gd.GetCell(1, a) == L"END")
					{
						anim.endFrame = -1;
					}
					else
					{
						anim.endFrame = gd.GetCell(1, a);
						if (anim.endFrame < anim.startFrame)
						{
							OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, 
								"End frame must be greater than Start frame",
								"OGRE Export");
						}
					}
					maxFrame = anim.endFrame;

					animList.push_back(anim);
				}

				// Truncate the skeleton filename to just the name (no path)
				Ogre::String skelName = skeletonFileName;
				int pos = skeletonFileName.find_last_of("\\");
				if (pos == Ogre::String::npos)
				{
					pos = skeletonFileName.find_last_of("/");
				}
				if (pos != Ogre::String::npos)
				{
					skelName = skelName.substr(pos+1, skelName.size() - pos - 1);
				}

				// Do the mesh
				Ogre::DeformerList& deformers = 
					meshExporter.exportMesh(meshFileName, mergeSubmeshes, 
						exportChildren, edgeLists, tangents, lodData, skelName);
				// do the skeleton
				skelExporter.exportSkeleton(skeletonFileName, deformers, fps, animList);



			}
			else
			{
				// Just mesh
				meshExporter.exportMesh(meshFileName, mergeSubmeshes, 
					exportChildren, edgeLists, tangents, lodData);
			}

			
			delete lodData;

		}

	}
	catch (Ogre::Exception& e)
	{
		// Will already have been logged to the Ogre log manager
		// Tell XSI
		app.LogMessage(OgretoXSI(e.getDescription()), XSI::siFatalMsg);
		app.LogMessage(OgretoXSI(e.getFullDescription()), XSI::siInfoMsg);
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
		L"objects",CValue::siRefArray, caps, 
		L"Collection of selected objects", L"", 
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
		L"mergeSubmeshes",CValue::siBool, caps, 
		L"Merge objects with same material?", 
		L"If false, a separate named SubMesh will be created for every PolygonMesh "
		L"preserving your model divisions. If true, the exporter will merge all "
		L"PolygonMesh objects with the same material, which is more efficient, but "
		L"does not preserve your modelling divisions.",
		CValue(true), param) ;	
	prop.AddParameter(
		L"exportChildren",CValue::siBool, caps, 
		L"Export Children", 
		L"If true, children of all selected objects will be exported.",
		CValue(true), param) ;	
    prop.AddParameter(	
        L"calculateEdgeLists",CValue::siBool, caps, 
        L"Calculate Edge Lists (stencil shadows)", L"", 
        CValue(true), param) ;	
    prop.AddParameter(	
        L"calculateTangents",CValue::siBool, caps, 
        L"Calculate Tangents (normal mapping)", L"", 
        CValue(false), param) ;	
	prop.AddParameter(	
		L"numLodLevels",CValue::siInt2, caps, 
		L"Levels of Detail", L"", 
		CValue(0L), param) ;	
	prop.AddParameter(	
		L"lodDistanceIncrement",CValue::siFloat, caps, 
		L"Distance Increment", L"", 
		CValue(2000L), param) ;	
	prop.AddParameter(	
		L"lodQuota",CValue::siString, caps, 
		L"Reduction Style", L"", 
		L"p", param) ;	
	prop.AddParameter(	
		L"lodReduction",CValue::siFloat, caps, 
		L"Reduction Value", L"", 
		CValue(20.0f), param) ;	
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
    prop.AddGridParameter(
        L"animationSplit");	


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

    // Object 
	oLayout.AddTab(L"Basic");
    oLayout.AddGroup(L"Object(s) to Export");

    item = oLayout.AddItem(L"objectName");
    item.PutAttribute( siUINoLabel, true );
	/*
    item.PutWidthPercentage(80);
    item = oLayout.AddButton(L"Refresh", L"Refresh");
    item.PutWidthPercentage(1) ;
	*/

    oLayout.EndGroup();

	// Mesh group
    item = oLayout.AddGroup(L"Mesh");
    item = oLayout.AddItem(L"exportMesh") ;
    item = oLayout.AddItem(L"targetMeshFileName", L"Target", siControlFilePath);
	item.PutAttribute( siUINoLabel, true );
	item.PutAttribute( siUIFileFilter, L"OGRE Mesh format (*.mesh)|*.mesh|All Files (*.*)|*.*||" );
	item = oLayout.AddItem(L"mergeSubmeshes") ;
	item = oLayout.AddItem(L"exportChildren") ;

	oLayout.EndGroup();
	// Skeleton Group
	item = oLayout.AddGroup(L"Skeleton");

	item = oLayout.AddItem(L"exportSkeleton");
	item = oLayout.AddItem(L"targetSkeletonFileName", L"Target", siControlFilePath);
	item.PutAttribute( siUINoLabel, true );
	item.PutAttribute( siUIFileFilter, L"OGRE Skeleton format (*.skeleton)|*.skeleton|All Files (*.*)|*.*||" );
	item = oLayout.AddItem(L"fps");
	item = oLayout.AddItem(L"animationSplit", L"Animations", siControlGrid);
	item.PutAttribute(siUIGridColumnWidths, L"0:120:80:80");
	item.PutAttribute(siUIGridHideRowHeader, true);

	oLayout.EndGroup();


	oLayout.AddTab(L"Advanced");

	oLayout.AddGroup(L"Mesh options");
    item = oLayout.AddItem(L"calculateEdgeLists");
    item = oLayout.AddItem(L"calculateTangents");
	oLayout.AddGroup(L"Level of Detail Reduction");
    item = oLayout.AddItem(L"numLodLevels");
	item = oLayout.AddItem(L"lodDistanceIncrement");
	CValueArray vals;
	vals.Add(L"Percentage");
	vals.Add(L"p");
	vals.Add(L"Constant");
	vals.Add(L"c");
	item = oLayout.AddEnumControl(L"lodQuota", vals, L"Quota", XSI::siControlCombo);
	item = oLayout.AddItem(L"lodReduction");
	oLayout.EndGroup();




	return CStatus::OK;	
}


bool hasSkeleton(X3DObject& si, bool recurse)
{
	if (si.GetEnvelopes().GetCount() > 0)
	{
		return true;
	}

	if (recurse)
	{

		CRefArray children = si.GetChildren();

		for(long i = 0; i < children.GetCount(); i++)
		{
			X3DObject child(children[i]);
			bool ret = hasSkeleton(child, recurse);
			if (ret)
				return ret;
		}
	}

	return false;
	
}

bool hasSkeleton(Selection& sel, bool recurse)
{
	// iterate over selection
	for (int i = 0; i < sel.GetCount(); ++i)
	{
		X3DObject obj(sel[i]);
		bool ret = hasSkeleton(obj, recurse);
		if (ret)
			return ret;
	}

	return false;
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
	static bool hasSkel = false;

	PPGEventContext ctx( io_Ctx ) ;

	PPGEventContext::PPGEvent eventID = ctx.GetEventID() ;

	CustomProperty prop = ctx.GetSource() ;	
	Parameter objectNameParam = prop.GetParameters().GetItem( L"objectName" ) ;
    // On open dialog
    if ( eventID == PPGEventContext::siOnInit )
	{
        // Pre-populate object with currently selected item(s)
		Selection sel(app.GetSelection());
		if (sel.GetCount() > 0)
		{
			CString val;
			for (int i = 0; i < sel.GetCount(); ++i)
			{
				val += SIObject(sel[i]).GetName();
				if (i < sel.GetCount() - 1)
					val += L", ";
			}
			prop.PutParameterValue(L"objectName", val);
		}
		else
		{
			// no selection, assume entire scene
			prop.PutParameterValue(L"objectName", CString(L"[Entire Scene]"));
		}
        // Make the selection read-only
		objectNameParam.PutCapabilityFlag( siReadOnly, true );

        // enable / disable the skeleton export based on envelopes
		if (!hasSkeleton(sel, true))
		{
			prop.PutParameterValue(L"exportSkeleton", false);
			Parameter param = prop.GetParameters().GetItem(L"exportSkeleton");
			param.PutCapabilityFlag(siReadOnly, true);
			param = prop.GetParameters().GetItem(L"targetSkeletonFileName");
			param.PutCapabilityFlag(siReadOnly, true);
			param = prop.GetParameters().GetItem(L"fps");
			param.PutCapabilityFlag(siReadOnly, true);
			param = prop.GetParameters().GetItem(L"animationSplit");
			param.PutCapabilityFlag(siReadOnly, true);
			hasSkel = false;
		}
		else
		{
			prop.PutParameterValue(L"exportSkeleton", true);
			Parameter param = prop.GetParameters().GetItem(L"exportSkeleton");
			param.PutCapabilityFlag(siReadOnly, false);
			param = prop.GetParameters().GetItem(L"targetSkeletonFileName");
			param.PutCapabilityFlag(siReadOnly, false);
			param = prop.GetParameters().GetItem(L"fps");
			param.PutCapabilityFlag(siReadOnly, false);
			// default the frame rate to that selected in animation panel
			prop.PutParameterValue(L"fps", CTime().GetFrameRate());
			param = prop.GetParameters().GetItem(L"animationSplit");
			param.PutCapabilityFlag(siReadOnly, false);
			// value of param is a griddata object
			// initialise it with a single animation for the full period
			GridData gd = param.GetValue();
			gd.PutColumnCount(3);
			gd.PutColumnLabel(0, L"Name");
			gd.PutColumnLabel(1, L"Start Frame");
			gd.PutColumnLabel(2, L"End Frame");

			// User can't add rows, so have to provide enough
			gd.PutRowCount(20);
			gd.PutCell(0, 0, L"Default");
			gd.PutCell(1, 0, L"1");
			gd.PutCell(2, 0, L"END");
			
			hasSkel = true;
		}
	}
    // On clicking a button
	else if ( eventID == PPGEventContext::siButtonClicked )
	{
		CValue buttonPressed = ctx.GetAttribute( L"Button" );	
        // Clicked the refresh button
		/*
		if ( buttonPressed.GetAsText() == L"Refresh" )
		{
			objectNameParam.PutCapabilityFlag( siReadOnly, false );
			// Pre-populate object with currently selected item
			Selection sel(app.GetSelection());
			CString val;
			for (int i = 0; i < sel.GetCount(); ++i)
			{
				val += SIObject(sel[0]).GetName();
				if (i < sel.GetCount() - 1)
					val += L", ";
			}
			prop.PutParameterValue(L"objectName", val);
			prop.PutParameterValue(L"objects", CValue(CRefArray(sel.GetArray())));
			// Make the selection read-only
			objectNameParam.PutCapabilityFlag( siReadOnly, true );
		}
		*/
	}
    // Changed a parameter
	else if ( eventID == PPGEventContext::siParameterChange )
	{
		Parameter changed = ctx.GetSource() ;	
		CustomProperty prop = changed.GetParent() ;	
		CString   paramName = changed.GetScriptName() ; 

        // Check paramName against parameter names, perform custom onChanged event
		if (paramName == L"targetMeshFileName")
		{
			Ogre::String meshName = XSItoOgre(changed.GetValue());
			if (hasSkel && Ogre::StringUtil::endsWith(meshName, "mesh"))
			{
				Ogre::String skelName = meshName.substr(0, meshName.size() - 4) + "skeleton";
				CString xsiSkelName = OgretoXSI(skelName);
				prop.PutParameterValue(L"targetSkeletonFileName", xsiSkelName);
			}
		}
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
