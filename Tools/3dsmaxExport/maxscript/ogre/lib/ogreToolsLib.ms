
-- ogreToolsLib.ms
-- 
-- Copyright © 2002 John Martin
--
-- Contains functions to support the Ogre tools.


-- Creates a filename for static mesh export data.
function getStaticMeshExportInfoFilename = 
(
	("ogre_staticMeshExportInfo_" + (substring maxFileName 1 (maxFilename.count - 3)) + "otc")
)

-- Reads static mesh export config info from a exportInfo file.
function readStaticMeshExportInfoFromFile exportInfoFilename =
(
	f = openFile (maxFilePath + exportInfoFilename)

	meshName = ""
	filename = ""

	if (f != undefined) then
	(
		line = readLine f
		
		-- do regexp and grab out each item
		items = filterString line "|"
		
		meshName = items[1]
		filename = items[2]
	
		close f	
	)
	
	-- return the data as a list
	#(meshName, filename)
)

-- Writes export info to a file.
function writeStaticMeshExportInfoToFile exportFilename meshName filename =
(
	ostream = createFile (maxFilePath + exportFilename)

	if (ostream == undefined) then
	(
		messageBox "Could not write export config data to file. Changes have not been saved." title:" Ogre Tools"
		return false
	)

	format "%|%\n" meshName filename to:ostream
	
	close ostream
	
	true
)