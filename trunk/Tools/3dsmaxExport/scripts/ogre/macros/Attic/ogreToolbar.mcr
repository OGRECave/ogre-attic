
-- ogreToolbar.mcr
-- 
-- Copyright � 2002 John Martin
--
-- $Id: ogreToolbar.mcr,v 1.1 2002-11-06 21:10:41 sinbad Exp $
--
-- Macroscript for the Ogre Toolbar.


-- library functions
include "ogre/lib/ogreExportLib.ms"
include "ogre/lib/ogreToolsLib.ms"


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
		bitmap logoBitmap "Ogre" filename:"$images\ogrelogo.jpg" pos:[11,8] width:450 height:228
		label label1 "Ogre Tools are created for use with the Ogre graphics engine. See website for details. " pos:[11,248] width:422 height:21
		label label2 "This software is distributed under the terms of the LGPL." pos:[11,272] width:402 height:21
		label label3 "Copyright � 2002 The Ogre Team" pos:[11,331] width:200 height:21
		button closeButton "Close" pos:[376,328] width:80 height:24
		
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