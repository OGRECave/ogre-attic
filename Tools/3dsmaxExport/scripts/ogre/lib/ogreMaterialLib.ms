-- write into "outfile" the selected OgreMaterial
-- precondition: iskindof material OgreMAterial == true.
-- Use export OgreMaterial.
function writeOgreMaterial material outFile =
(
	local t, matname, mat;

	mat = material ;
	
	if (mat != undefined) then 
	(
		matname = material.name ;	
		format "%\n" matname to:outFile ;
		format "{\n" to:outFile ;
		if ((mat.diffusemap != undefined) and (iskindof mat.diffusemap Bitmaptexture)) then
		(	
			t = filterstring mat.diffusemap.filename "\\" ;
			format "    {\n" to:outFile ; 
			format "        texture % \n" t[arrayLength t] to:outFile ;
			format "    }\n" to:outFile ;
		)
		else (
		format("    ambient % % %\n") (mat.ambient.r/255.0) (mat.ambient.g/255.0) (mat.ambient.b/255.0) to:outFile ;
	    format("    diffuse % % %\n") (mat.diffuse.r/255.0) (mat.diffuse.g/255.0) (mat.diffuse.b/255.0) to:outFile ;
    	format("    specular % % % %\n") (mat.specular.r/255.0) (mat.specular.g/255.0) (mat.specular.b/255.0) (mat.specularLevel*100.0) to:outFile ;
		format("    emissive % % %\n") (mat.selfIllumColor.r/255.0) (mat.selfIllumColor.g/255.0) (mat.selfIllumColor.b/255.0) to:outFile ;
		)
		format "}\n" to:outFile ;
	)
)

function exportOgreMaterial material outName =
(
    local outFile ;
	if (classof material == OgreMaterial or classof material == ogreMaterial) then
	(
	    outFile = createfile outName ;
		writeOgreMaterial material outFile ;
		close outFile ;
		messageBox "Exporting material successful !" ;
	)
	else
	(
		messageBox "You have to choose an OgreMaterial." ;
	)
)

---------------------------------------------------------------
-- Writes a standard material in a material script
---------------------------------------------------------------
function writeSubStandardMaterial material matname outFile =
(
	local t ;

	-- writes header	
	format "%\n" matname to:outFile ;
	format "{\n" to:outFile ;

	-- tests if there is (or not) a diffuse map.
	if ((material.diffusemap != undefined) and (iskindof material.diffusemap Bitmaptexture)) then
	(	
		t = filterstring material.diffusemap.filename "\\" ;
		format "    {\n" to:outFile ; 
		format "        texture % \n" t[arrayLength t] to:outFile ;
		format "    }\n" to:outFile ;
	)
	else (
		format("    ambient % % %\n") (material.ambient.r/255.0) (material.ambient.g/255.0) (material.ambient.b/255.0) to:outFile ;
	    format("    diffuse % % %\n") (material.diffuse.r/255.0) (material.diffuse.g/255.0) (material.diffuse.b/255.0) to:outFile ;
    	format("    specular % % % %\n") (material.specular.r/255.0) (material.specular.g/255.0) (material.specular.b/255.0) (material.specularLevel*100.0) to:outFile ;
		format("    emissive % % %\n") (material.selfIllumColor.r/255.0) (material.selfIllumColor.g/255.0) (material.selfIllumColor.b/255.0) to:outFile ;
	)
	-- writes footer
	format "}\n" to:outFile ;

)


---------------------------------------------------------------
-- Exports a multi material
---------------------------------------------------------------
function exportMultiMaterial material outname =
(
	local outFile,sub,i,matname ;
	if (material!=undefined and classof material == MultiMaterial) then
	(
		outFile = createfile outName ;
		matname = material.name ;
		
		i=0 ;
		for sub in material.materiallist do (
			i+=1 ;
			writeSubStandardMaterial sub (matname+"_sub"+(i as string) ) outFile ;
			format "\n" to:outFile ;
		)
		
		close outFile ;
		messageBox "Exporting material successful !" ;
	)
	else
		messageBox "You have to choose a MultiMaterial." ;



)