
-- ogreToolbar.mcr
-- 
-- Copyright © 2002 John Martin
--
-- Macroscript for the Ogre Toolbar.


-- library functions
include "ogre/lib/ogreExportLib.ms"
include "ogre/lib/ogreToolsLib.ms"


-- Set options button.
macroScript setOptions
	category:"Ogre Tools"
	internalCategory:"Ogre Tools"
	buttonText:"Options"
	tooltip:"Set General Options"
(
	messageBox "Will eventually be able to set general options here." title:"Ogre Tools"
)

-- Export static mesh button.
macroScript exportStaticMesh
	category:"Ogre Tools"
	internalCategory:"Ogre Tools"
	buttonText:"Export Mesh"
	tooltip:"Export a Static Mesh"
(
	-- must be a max file open to use this feature
	if (maxFileName == "") then
	(
		messageBox "You must open a file first." title:"Ogre Tools"
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

				true
			)

			--
			-- Event handlers.
			--

			-- Init function.
			on staticMeshExportRollout open do
			(
				exportInfo = readStaticMeshExportInfoFromFile (getStaticMeshExportInfoFilename())
				
				meshName = exportInfo[1]
				filenameBox.text = exportInfo[2]
				
				-- populate the dropdown of meshes
				meshList = for obj in $objects where classOf obj == Editable_Mesh collect obj.name
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
					writeStaticMeshExportInfoToFile (getStaticMeshExportInfoFilename()) meshDropdown.selected filenameBox.text
					
					-- do the export
					ostream = createFile filenameBox.text
					if (ostream == undefined) then
					(
						messageBox "Couldn't open the export file for writing. Export was unsuccessful." title:"Ogre Tools"
						return false
					)
					
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
(
	messageBox "Will eventually be able to configure animation tracks here." title:"Ogre Tools"
)

-- Export anim mesh button.
macroScript exportAnimMesh
	category:"Ogre Tools"
	internalCategory:"Ogre Tools"
	buttonText:"Export Anim"
	tooltip:"Export Animated Mesh"
(
	messageBox "Will eventually be able to export an animated mesh here." title:"Ogre Tools"
)