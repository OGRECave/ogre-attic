
-- ogreToolbar.mcr
-- 
-- Copyright © 2002 John Martin
--
-- $Id: ogreToolbar.mcr,v 1.9 2003-12-25 16:44:41 sinbad Exp $
--
-- Macroscript for the Ogre Toolbar.
--
-- New buttons added in order to run others scripts by Etienne Mallard in 2003.
--



-- library functions

-- include "ogre/lib/ogreExportLib.ms"
-- include "ogre/lib/ogreToolsLib.ms"
include "ogre/lib/ogreSkeletonLib.ms"
include "ogre/lib/ogreMaterialLib.ms"
include "ogre/lib/ogreMaterialPlugin.ms"

macroScript showSkeletonTools
	category:"Ogre Tools"
	internalCategory:"Ogre Tools"
	buttonText:"Skeleton"
	tooltip:"Skeleton Tools"
	Icon:#("Maintoolbar",49)
(

	-- create a floater
	SkeletonExportFloater = newRolloutFloater "SkeletonTools" 305 580 ; 

	rollout skeletonExportRollout "Export Mesh and Skeleton"
	(
		-- Elements
		-----------
		
		button chooseMeshButton "Select a mesh..." align:#right width:88 height:24 ;
		editText objectNameET "Selected object: " align:#left ;
		
		spinner scaleSP "Scale:" range:[0,100,1] type:#Float scale:0.1 align:#right;
		checkBox flipYZCB "Flip Y/Z" align:#right ;
		
		checkBox exportMeshCB "Export Mesh" align:#left ;
		checkBox flipNormalCB "Flip normals                     " align:#right ;
		checkBox ExportUVCB "Export texture information" align:#right ;
		checkBox ExportColCB "Export colour information " align:#right ;
		
		checkBox exportSkeletonCB "export Skeleton" align:#left ;
		checkBox useBipedCB "This skeleton is a Biped" align:#left ;
		
		editText animNameET "Animation name:" align:#left ;
 	 	spinner firstFrameSP "First frame of animation:" range:[0,1000,0] type:#Integer ;
		spinner lastFrameSP "Last frame of animation:" range:[0,1000,100] type:#Integer ;		
		spinner animLengthSP "Real length in seconds:" range:[0,1000,10] type:#Float scale:0.1;
		
		-- info label
		label infoLabel "NOTA:\nIf you choose aaa for example, exported filenames will be aaa.mesh.xml and aaa.skeleton.xml" align:#left width:248 height:40
	
		-- filename text box
		editText filenameET "File:" align:#left ;
		
		-- file choose button
		button chooseFileButton "Choose File..." align:#right width:88 height:24 ;

		-- options checkboxes
		label materialLabel "NOTA:\nMaterial definition IS NOT INCLUDED in the .mesh.xml file.\nWrite your material script or try the material exporter." align:#left height:40;
		
		-- export button
		button exportButton "Export" align:#center width:80 ;
		
		-- log label
		label logLabel "If you want to see a log file, open the Maxscript listener.\nYou can see how many submeshes have been exported." align:#left height:40;

		
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
			if (selection[1] != undefined) then (
				objectNameET.text = selection[1].name ;
			
				-- options accessible ou non
				exportMeshCB.enabled = true ;				
				exportSkeletonCB.enabled = (getSkin(selection[1]) != undefined) or (getPhysique(selection[1]) != undefined) ;
			)
						
		)
		
		-- Press of the export button.
		on exportButton pressed do
		(
			sliderTime = 0 ;
			m = execute ("$" + objectNameET.text) ;
			if (filenameET.text == "" or m == undefined ) then
				messageBox "You have to choose a filename and a valid object." ;
			else
			(
				clearlistener() ;
				
				options = exportOptions scale:scaleSP.value flipYZ:false flipNormal:false exportColours:false exportUV:false ;
				exportingMeshDone = false ;
				exportingSkelDone = false ;
				-- sets options
				---------------
				if (flipYZCB.checked) then
					options.flipYZ= true ;					
				if (flipNormalCB.checked and flipNormalCB.enabled) then
					options.flipNormal = true ;
				if (exportColCB.checked and exportColCB.enabled) then
					options.exportColours = true ;
				if (exportUVCB.checked and exportUVCB.enabled) then
					options.exportUV = true ;	
				
				-- exports mesh
				---------------
				if (exportMeshCB.enabled and exportMeshCB.checked) then
				(
					lastFile = setINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Settings" "lastFile" filenameET.text	
					exportingMeshDone = writeMesh m options (filenameET.text) ;
				)
			
				-- exports skeleton
				-------------------
				if (exportSkeletonCB.enabled and exportSkeletonCB.checked) then
				(
					lastAnimName = setINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Settings" "lastAnimName" animNameET.text
					options.firstFrame = firstFrameSP.value ;
					options.lastFrame = lastFrameSP.value ;
					options.length = animLengthSP.value ;
					options.animName = animNameET.text ;
					
					if (useBipedCB.checked) then
						writeBiped m options filenameET.text ;
					else
						writeSkeleton m options filenameET.text ;

					exportingSkelDone = true ;
				)
				
				-- post traitement
				------------------
				if (exportingMeshDone or exportingSkelDone) then (					
					runXMLConverter = getINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Tools" "runXMLConverter"
					if (runXMLConverter=="yes") then
					(
						xmlConvPath = getINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Directories" "XMLConverterPath"
						mediaPath = getINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Directories" "MediaPath"
						xmlexe= getINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Exe" "XMLConverterExe"
					
						if (exportingMeshDone) then (
							DOSCommand (xmlConvPath + "\\" + xmlexe + " \"" + filenameET.text + ".mesh.xml\" \"" + filenameET.text + ".mesh\"") ;
							DOSCommand ("copy \"" + filenameET.text + ".mesh\" \"" + mediaPath + "\"") ;
						)
						if (exportingSkelDone) then (
							DOSCommand (xmlConvPath + "\\" + xmlexe + " \"" + filenameET.text + ".skeleton.xml\" \"" + filenameET.text + ".skeleton\"") ;
							DOSCommand ("copy \"" + filenameET.text + ".skeleton\" \"" + mediaPath + "\"") ;						
						)
						messageBox "OgreXMLConverter has been run and files copied to the media directory."
						print (xmlConvPath + "\\" + xmlexe + " \"" + filenameET.text + ".mesh.xml\" \"" + filenameET.text + ".mesh\"") ;
						
					)
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
			useBipedCB.enabled = true ;
		)
		
		-- Press of exportSkeletonCheckBox
		on exportSkeletonCB changed false do
		(
			firstFrameSP.enabled = false ;
			lastFrameSP.enabled = false ;
			animNameET.enabled = false ;
			animLengthSP.enabled = false ;
			useBipedCB.enabled = false ;
		)
		
		-- Press of exportMeshCheckBox
		on exportMeshCB changed true do
		(
			flipNormalCB.enabled = true ;
			ExportColCB.enabled = true ;
			ExportUVCB.enabled = true ;
		)
		
		-- Press of exportMeshCheckBox
		on exportMeshCB changed false do
		(
			flipNormalCB.enabled = false ;
			ExportColCB.enabled = false ;
			ExportUVCB.enabled = false ;			
		)
		-- When the rollout is opened
		on skeletonExportRollout open do
		(
			
			lastFile = getINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Settings" "lastFile"
			lastAnimName = getINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Settings" "lastAnimName"
			
			filenameET.text = lastFile ;
			animNameET.text = lastAnimName ;
			exportMeshCB.enabled = false ;
			exportSkeletonCB.enabled = false ;
			firstFrameSP.enabled = false ;
			lastFrameSP.enabled = false ;
			animNameET.enabled = false ;
			animLengthSP.enabled = false ;
			flipNormalCB.enabled = false ;
			ExportColCB.enabled = false ;
			ExportUVCB.enabled = false ;
			ExportUVCB.checked = true ;
			flipYZCB.checked = true ;
			useBipedCB.enabled = false ;
			useBipedCB.checked = false ;

		)
	)
	
	rollout aboutBoxRollout "About"
	(
		label label11 "For use with the Ogre graphics engine." align:#center;
		label label12 "See website for details: http://ogre.sourceforge.net" align:#center;
		label label13 "This software is distributed under the terms of the LGPL." align:#center ;
		label label14 "by EarthquakeProof - mallard@iie.cnam.fr  (summer 2003)" align:#right ;

	)

	rollout optionRollout "Options"
	(
		-- Elements
		-----------
		
		-- option button
		button openButton "Open ogreScript.ini" align:#right ;

		-- Events 
		---------
		on openButton pressed do
		(			
			shellLaunch ((getDir #scripts) + "\\ogre\\ogreScript.ini") "" ;
		)
		
		--on optionRollout open do ()
		
		
	)

	-- add the rollout, which contains the dialog	
	addRollout optionRollout skeletonExportFloater ;
	addRollout skeletonExportRollout skeletonExportFloater ;
	addRollout aboutBoxRollout skeletonExportFloater ;

)

---------------------------------------------------------------
-------------------   MATERIAL EXPORTER -----------------------
---------------------------------------------------------------

macroScript showMaterialTools
	category:"Ogre Tools"
	internalCategory:"Ogre Tools"
	buttonText:"Material"
	tooltip:"Material Tools"
	Icon:#("Material_Modifiers",3)
(
	-- create a floater
	MaterialExportFloater = newRolloutFloater "MaterialTools" 305 155 ; 

	rollout materialExportRollout "Material Tools"
	(
		materialButton choosemtl "Pick Material" ;
	    editText filenameET "File:" align:#left ;
		button chooseFileButton "Choose File..." align:#right ;
		button materialExportButton "Export" align:#right ;
		button allMaterialsExportButton "Or Try to export all the materials used in the scene !" align:#center ;
		
		
		on materialExportButton pressed do
		(
			filename = filenameET.text ;			
			if (filename != "") then
			(
				-- saves settings
				setINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Settings" "lastMaterialFile" filenameET.text	

				exportMaterial (choosemtl.material) filename ;
			)
			else
			    messageBox "You have to choose a filename." ;
		)
		
		-- Press of the choose file button.
		on chooseFileButton pressed do
		(
			filename = getSaveFileName types:"OGRE material files (*.material)|*.*|" ;			
			if (filename != undefined) then
			(
				filenameET.text = filename ;
			)
		)
		
		-- Press of export all materials button
		on allMaterialsExportButton pressed do
		(
			filename = filenameET.text ;			
			if (filename != "") then
			(
				-- saves settings
				setINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Settings" "lastMaterialFile" filenameET.text	

				exportAllSceneMaterials filename ;
			)
			else
			    messageBox "You have to choose a filename." ;
		)

		
		on materialExportRollout open do
		(
			filenameET.text = getINISetting ((getDir #scripts) + "\\ogre\\ogreScript.ini") "Settings" "lastMaterialFile"
		)
		
	)
		
	rollout aboutBoxRollout "About"
	(
		label label11 "For use with the Ogre graphics engine." align:#left;
		label label12 "See website for details: http://ogre.sourceforge.net" align:#left;
		label label13 "This software is distributed under the terms of the LGPL." align:#left ;
		label label14 "by EarthquakeProof - mallard@iie.cnam.fr (summer 2003)" align:#right ;
		
		
	)
	-- add the rollout, which contains the dialog
	addRollout materialExportRollout materialExportFloater ;
	addRollout aboutBoxRollout materialExportFloater ;
)

