
-- ogreToolbar.mcr
-- 
-- Copyright © 2002 John Martin
--
-- $Id: ogreToolbar.mcr,v 1.3 2003-03-19 22:51:52 sinbad Exp $
--
-- Macroscript for the Ogre Toolbar.


-- library functions
include "ogre/lib/ogreSkeletonLib_addmissingkeys.ms"
include "ogre/lib/ogreExportLib.ms"
include "ogre/lib/ogreToolsLib.ms"
include "ogre/lib/ogreSkeletonLib.ms"

-- Show an about box.
macroScript showAbout
	category:"Ogre Tools"
	internalCategory:"Ogre Tools"
	buttonText:"About"
	tooltip:"About Ogre Tools"
	Icon:#("OgreTools",1)
(
	-- create a floater
	aboutBoxFloater = newRolloutFloater "About" 498 426 
	
	-- define the rollout
	rollout aboutBoxRollout "About Ogre Tools"
	(
		bitmap logoBitmap "Ogre" filename:"$images\ogrelogo.jpg" pos:[11,8] width:450 height:228 ;
		label label1 "Ogre Tools are created for use with the Ogre graphics engine. See website for details. " pos:[11,248] width:422 height:21 ;
		label label2 "This software is distributed under the terms of the LGPL." pos:[11,272] width:402 height:21 ;
		label label3 "Copyright © 2002 The Ogre Team" pos:[11,331] width:200 height:21 ;
		button closeButton "Close" pos:[376,328] width:80 height:24 ;
		
		-- Close the dialog box.
		on closeButton pressed do
		(
			closeRolloutFloater aboutBoxFloater
		)
	)
	
	-- add the rollout, which contains the dialog
	addRollout aboutBoxRollout aboutBoxFloater
)

-- Export static mesh button.
macroScript exportStaticMesh
	category:"Ogre Tools"
	internalCategory:"Ogre Tools"
	buttonText:"Export Mesh"
	tooltip:"Export a Static Mesh"
	Icon:#("OgreTools",2)
(
	-- must be a max file open to use this feature
	if (maxFileName == "") then
	(
		messageBox "You must open a file first (or save your current work)." title:"Ogre Tools"
	)
	else
	(
		-- create a floater
		staticMeshExportFloater = newRolloutFloater "Export Static Mesh" 310 290 
		
		-- define the rollout
		rollout staticMeshExportRollout "Export Static Mesh"
		(
			-- mesh dropdown
			dropDownList meshDropdown "Select editable mesh" pos:[16,8] width:248 height:40 enabled:true
			
			-- filename text box
			editText filenameBox "File:" pos:[16,64] width:248 height:20
			
			-- file choose button
			button chooseFileButton "Choose File..." pos:[176,96] width:88 height:24
			
			-- notes label
			label debugLabel "Note: Debug information pertaining to the export will be written to the MaxScript listener and a log file within the 3dsmax directory." pos:[16,136] width:248 height:40

			-- cancel button
			button cancelButton "Cancel" pos:[160,192] width:48 height:24

			-- export button
			button exportButton "Export" pos:[216,192] width:48 height:24

			--
			-- Helper functions.
			--
			
			-- Make sure we have enough info to export.
			function validateExportInfo =
			(
				try
				(
					if (meshDropdown.selected == undefined) then
					(
						messageBox "There is no mesh to export. (Static mesh names must start with \"staticMesh_\".)" title:"Ogre Tools"
						return false
					)
	
					if (filenameBox.text == "") then
					(
						messageBox "Need a file to export to." title:"Ogre Tools"
						return false
					)
				)
				catch
				(
					messageBox "Unknown error validating export info." title:"Ogre Tools"
				)

				true
			)

			--
			-- Event handlers.
			--

			-- Init function.
			on staticMeshExportRollout open do
			(
				try
				(
					exportInfo = readStaticMeshExportInfoFromFile (getStaticMeshExportInfoFilename())
				)
				catch
				(
					messageBox "Error reading export configuation info." title:"Ogre Tools"
				)
				
				try
				(		
					meshName = exportInfo[1]
					filenameBox.text = exportInfo[2]
					
					-- populate the dropdown of meshes
					meshList = for obj in $objects collect obj.name
					meshDropdown.items = meshList
					
					-- try and select the previously selected mesh
					for i = 1 to meshList.count do
					(
						if (meshList[i] == meshName) then
						(
							meshDropdown.selection = i
						)
					)
				)
				catch
				(
					messageBox "Unknown error populating mesh list." title:"Ogre Tools"
				)
			)

			-- Close the dialog box without saving changes.
			on cancelButton pressed do
			(
				closeRolloutFloater staticMeshExportFloater
			)
			
			-- Export this sucker.
			on exportButton pressed do
			(
				if (validateExportInfo()) then
				(
					try
					(
						writeStaticMeshExportInfoToFile (getStaticMeshExportInfoFilename()) meshDropdown.selected filenameBox.text
					)
					catch
					(
						messageBox "Error saving export info." title:"Ogre Tools"
					)
					
					try
					(
						if (exportMesh meshDropdown.selected filenameBox.text) then
						(
							messageBox "Static mesh export successful." title:"Ogre Tools"
						)
						else
						(
							messageBox "Static mesh export failed." title:"Ogre Tools"
						)
						closeRolloutFloater staticMeshExportFloater
					)
					catch
					(
						messageBox "Static mesh export failed. Unknown error." title:"Ogre Tools"
					)
				)
			)

			-- Press of the choose file button.
			on chooseFileButton pressed do
			(
				filename = getSaveFileName types:"Ogre Mesh File (*.mesh)|*.mesh|All Files(*.*)|*.*|"
				
				if (filename != undefined) then
				(
					filenameBox.text = filename
				)
			)

		)
		
		-- add the rollout, which contains the dialog
		addRollout staticMeshExportRollout staticMeshExportFloater
	)
)

-- Configure animations button.
macroScript configAnimations
	category:"Ogre Tools"
	internalCategory:"Ogre Tools"
	buttonText:"Config Anim"
	tooltip:"Configure Animations"
	Icon:#("OgreTools",3)
(
	messageBox "Animation configuration is not yet implemented." title:"Ogre Tools"
)

-- Export anim mesh button.
macroScript exportAnimMesh
	category:"Ogre Tools"
	internalCategory:"Ogre Tools"
	buttonText:"Export Anim"
	tooltip:"Export Animated Mesh"
	Icon:#("OgreTools",4)
(
	messageBox "Animated mesh export is not yet implemented." title:"Ogre Tools"
)

-- Set options button.
macroScript setOptions
	category:"Ogre Tools"
	internalCategory:"Ogre Tools"
	buttonText:"Options"
	tooltip:"Set General Options"
	Icon:#("OgreTools",5)
(
	messageBox "General options dialog is not yet implemented." title:"Ogre Tools"
)


macroScript showSkeletonTools
	category:"Ogre Tools"
	internalCategory:"Ogre Tools"
	buttonText:"Skeleton"
	tooltip:"Skeleton Tools"
	-- Icon:#("OgreTools",1)
(

	-- create a floater
	SkeletonExportFloater = newRolloutFloater "SkeletonTools" 305 500 ; 
	
	rollout skeletonExportRollout "Export Mesh and Skeleton"
	(
		-- Elements
		-----------
		
		button chooseMeshButton "Select a mesh..." align:#right width:88 height:24 ;
		editText objectNameET "Selected object: " align:#left ;
		spinner scaleSP "Scale:" range:[0,100,1] type:#Float scale:0.1 align:#right;
		checkBox flipYZCB "Flip Y/Z" align:#right ;
		checkBox exportMeshCB "Export Mesh" align:#left ;
		checkBox flipNormalCB "Flip normals" align:#right ;
		
		checkBox exportSkeletonCB "export Skeleton" align:#left ;
		
		editText animNameET "Animation name:" align:#left ;
 	 	spinner firstFrameSP "First frame of animation:" range:[0,1000,0] type:#Integer ;
		spinner lastFrameSP "Last frame of animation:" range:[0,1000,100] type:#Integer ;		
		spinner animLengthSP "Real length in seconds:" range:[0,1000,10] type:#Float scale:0.1;
		
		-- info label
		label infoLabel "Note:\nIf you choose aaa for example, exported filenames will be aaa.mesh.xml and aaa.skeleton.xml" align:#left width:248 height:40
		-- export button
	
		-- filename text box
		editText filenameET "File:" align:#left ;
		
		-- file choose button
		button chooseFileButton "Choose File..." align:#right width:88 height:24 ;


		button exportButton "Export" align:#right width:48 height:24 ;

		-- Events
		---------
		
		-- Press of the choose file button.
		on chooseFileButton pressed do
		(
			filename = getSaveFileName types:"All Files(*.*)|*.*|" ;			
			if (filename != undefined) then
			(
				filenameET.text = filename ;
			)
		)
		
		-- Press of the chooseMesh button
		on chooseMeshButton pressed do
		(
			max tool hlist ;
			if (selection[1] != undefined) then
				objectNameET.text = selection[1].name ;
			
			-- options accessible ou non
			exportMeshCB.enabled = true ;
			exportSkeletonCB.enabled = (getSkin(selection[1]) != undefined) ;
			firstFrameSP.enabled = exportSkeletonCB.enabled  ;
			lastFrameSP.enabled = exportSkeletonCB.enabled  ;
			animNameET.enabled = exportSkeletonCB.enabled  ;
			animLengthSP.enabled = exportSkeletonCB.enabled  ;
						
		)
		
		-- Press of the export button.
		on exportButton pressed do
		(
			sliderTime = 0 ;
			m = execute ("$" + objectNameET.text) ;
			if (filenameET.text == "" or m == undefined ) then
				messageBox "You have to choose a filename and an valid object." ;
			else
			(
				options = exportOptions scale:scaleSP.value flipYZ:false flipNormal:false;
				if (flipYZCB.checked) then
				(
					options.flipYZ= true ;
				)
				if (flipNormalCB.checked and flipNormalCB.enabled) then
					options.flipNormal = true ;
						
				if (exportMeshCB.enabled and exportMeshCB.checked) then
				(	
					writeMesh m options (filenameET.text) ;
				)
			
				if (exportSkeletonCB.enabled and exportSkeletonCB.checked) then
				(
					options.firstFrame = firstFrameSP.value ;
					options.lastFrame = lastFrameSP.value ;
					options.length = animLengthSP.value ;
					options.animName = animNameET.text ;
					
					writeSkeleton m options filenameET.text ;
				)
			)
		)
		
		-- Press of exportSkeletonCheckBox
		on exportSkeletonCB changed true do
		(
			firstFrameSP.enabled = true ;
			lastFrameSP.enabled = true ;
			animNameET.enabled = true ;
			animLengthSP.enabled = true ;
		)
		
		-- Press of exportSkeletonCheckBox
		on exportSkeletonCB changed false do
		(
			firstFrameSP.enabled = false ;
			lastFrameSP.enabled = false ;
			animNameET.enabled = false ;
			animLengthSP.enabled = false ;
		)
		
		-- Press of exportMeshCheckBox
		on exportMeshCB changed true do
		(
			flipNormalCB.enabled = true ;
		)
		
		-- Press of exportMeshCheckBox
		on exportMeshCB changed false do
		(
			flipNormalCB.enabled = false ;
		)
		-- When the rollout is opened
		on skeletonExportRollout open do
		(
			exportMeshCB.enabled = false ;
			exportSkeletonCB.enabled = false ;
			firstFrameSP.enabled = false ;
			lastFrameSP.enabled = false ;
			animNameET.enabled = false ;
			animLengthSP.enabled = false ;
			flipNormalCB.enabled = false ;
		)
		

	)
	
	rollout smallToolsBoxRollout "Other tools"
	(
		label smallToolsLabel1 "You can add missing keys to tracks of selected objects so there will not be an error when you press export button" align:#left width:248 height:40
		-- addMissingKeys button
		button addMissingKeysButton "Add missing keys to selected objects" align:#center ;
		
		on addMissingKeysButton pressed do
		(
			for obj in selection do
			(
				addMissingKeys obj ;
			)
			
			messageBox "Missing keys have been added" ;
		)
	)
	
	rollout aboutBoxRollout "About"
	(
		label label11 "For use with the Ogre graphics engine." align:#left;
		label label12 "See website for details: http://ogre.sourceforge.net" align:#left;
		label label13 "This software is distributed under the terms of the LGPL." align:#left ;
		
	)


	-- add the rollout, which contains the dialog
	
	addRollout skeletonExportRollout skeletonExportFloater ;

	addRollout smallToolsBoxRollout skeletonExportFloater ;

	addRollout aboutBoxRollout skeletonExportFloater ;

)