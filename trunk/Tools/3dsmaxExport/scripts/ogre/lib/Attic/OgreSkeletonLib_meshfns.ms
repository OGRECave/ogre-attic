-----------------------------------------------------------------
-- exploreMesh returns a string which is "OK" or an error message
-- if there are warnings, it displays message boxes.
-----------------------------------------------------------------

function exploreMesh pmesh =
(
	local material, answer, m ;
	
	answer = "" ;
	m = snapShotAsMesh pmesh ;
	
	material = pmesh.material ;
	if (material == undefined) then
		answer = "This mesh doesn't have any material, please apply one\n" ;
	else
		if (getnumtverts m == 0) then
			answer = "This mesh must have UVW coords in order to be exported" ;
		
	if (answer == "") then 
		answer = "OK" ;
	
	delete m ;
	answer ;
)


----------------------------------------------------------------------------------------
-- ----------------------------------- WRITE MESH ----------------------------------- --
----------------------------------------------------------------------------------------
-- Remarque !
-- 3dsmax starts ID from 1 although OGRE starts from 0.
-- -> a vertex 0 and a bone 0 will be added

----------------------------------------------------------------------------
-- write <mesh />
-- flipNormal = true if you want to flip normals
----------------------------------------------------------------------------

function writeM tmesh flipyz scale flipNormal outFile =
(
    local num_tverts, num_faces,v,f,face,nmal,vert,table,tvtable,vertTmp,tvertTmp,faceTmp,matTable,mat,currentMatId,matId ;
	local couple,v1,v2,v3 ;
	
	format "writing mesh: writing geometry ... \n" ;
	
	num_tverts = tmesh.numtverts ;
	num_faces = tmesh.numfaces ;
	matTable = getMaterialTable tmesh ;
	currentMatId = matTable[1] ;
	ogreVertexTable = getOgreVertexTable tmesh ;
	
	-- Vertices positions and normals
	--	
	format("\t\t\t<geometry count=\"%\">\n") ((arrayLength ogreVertexTable)+1) to:outFile; -- +1 cf remarque en haut.
	format("\t\t\t\t<vertexbuffer positions=\"true\" normals=\"true\" colours=\"false\" numtexcoords=\"%\" texcoordsets=\"0\" texcoorddimensions=\"2\">\n") currentMatId to:outFile ;
		
		-- vertex number 0
		format("\t\t\t\t\t<vertex>\n") to:outFile ;
		format("\t\t\t\t\t\t<position x=\"0\" y=\"0\" z=\"0\" />\n") to:outFile ;
		format("\t\t\t\t\t\t<normal x=\"1\" y=\"0\" z=\"0\" />\n") to:outFile ;
		format("\t\t\t\t\t\t<texcoord u=\"0\" v=\"0\" />\n") to:outFile ;
		format("\t\t\t\t\t</vertex>\n") to:outFile ;
 
	for couple in ogreVertexTable do 
	( 
        --   This doesn't work :(
        --         
		--if (currentMatId != matTable[v]) then
		--(
		--	currentMatId = matTable[v] ;
		--	format("\t\t\t\t</vertexbuffer>\n") to:outFile ;
		--	format("\t\t\t\t<vertexbuffer positions=\"true\" normals=\"true\" colours=\"false\" numtexcoords=\"1\" texcoordsets=\"%\" texcoorddimensions=\"2\">\n") currentMatId to:outFile ;
		--)

		tvert = getTvert tmesh couple[2] ; 
		vert = getVert tmesh couple[1] ;
		nmal = getNormal tmesh couple[1] ;
		
		-- change scale
		vert = vert * scale ;
		
		-- flip axes		
		if (flipyz) then
		(
			vertTmp = copy vert ;
			vert[2] = vertTmp[3] ;
			vert[3] = -vertTmp[2] ;
		)
		
		format("\t\t\t\t\t<vertex>\n") to:outFile ;
		format("\t\t\t\t\t\t<position x=\"%\" y=\"%\" z=\"%\" />\n") vert.x vert.y vert.z to:outFile ;
		format("\t\t\t\t\t\t<normal x=\"%\" y=\"%\" z=\"%\" />\n") nmal.x nmal.y nmal.z to:outFile ;
		format("\t\t\t\t\t\t<texcoord u=\"%\" v=\"%\" />\n") tvert.x tvert.y to:outFile ;		
		format("\t\t\t\t\t</vertex>\n") to:outFile ;
	)
	format("\t\t\t\t</vertexbuffer>\n") to:outFile ;

	format("\t\t\t</geometry>\n") to:outFile ;

	-- Faces
	--
	format("\t\t\t<faces count=\"%\">\n") num_faces to:outFile;
	for f = 1 to num_faces do 
	(
		tvface = getTVFace tmesh f ;
		face = getFace tmesh f ;
		-- flipNormals
		if (flipNormal) then
		(
			faceTmp = copy face ;
			face[1] = faceTmp[3] ;
			face[3] = faceTmp[1] ;
		)
		
		v1 = searchOgreVertexTable ogreVertexTable face.x tvface.x ;
		v2 = searchOgreVertexTable ogreVertexTable face.y tvface.y ;
		v3 = searchOgreVertexTable ogreVertexTable face.z tvface.z ;

		
		format("\t\t\t\t<face ") to:outFile ;
		format("v1=\"%\" v2=\"%\" v3=\"%\"") (v1 as Integer) (v2 as Integer) (v3 as Integer) to:outFile ;
		format(" />\n") to:outFile ;

	)
	format("\t\t\t</faces>\n") to:outFile ;
)

----------------------------
-- write <boneassignement />
----------------------------

function writeBoneAssignments tmesh sk outFile =
(
	local i,vNum,bid,num_tverts,vwcount,num_bones,j,w,couple,ogreVertexTable ;
	
	format "writing mesh: writing bone assignments ... \n" ;

	num_bones = skinOps.GetNumberBones(sk) ;
	ogreVertexTable = getOgreVertexTable tmesh ;

	format("\t\t\t<boneassignments>\n") to:outFile ;
	i=0 ;
	for couple in ogreVertexTable do
	(
	    i += 1 ;
		vNum = couple[1] ; -- gets the assiocated vertex.
		vwcount = skinOps.getVertexWeightCount sk vNum ;
		for j=1 to vwcount do
		(
			bid = skinOps.getVertexWeightBoneID sk vNum j ;
			w = skinOps.getVertexWeight sk vNum j ;
			-- There will be a bone assignement if weight > 0.1 --> it could increase speed.
			if w > 0.01 then
	    		format("\t\t\t\t<vertexboneassignment vertexIndex=\"%\" boneIndex=\"%\" weight=\"%\" />\n") i bid w to:outFile ;
		)
	)
	format("\t\t\t</boneassignments>\n") to:outFile ;
)

--------------------------------
-- write the mesh: main function
--------------------------------

function writeMesh pmesh exportOptions out_name =
(
	local m,sk,outFile,matname,message ;
	
	-- 3dsmax5: snapshotAsMesh
	m = snapshotAsMesh pmesh ;
	
	
	-- trying to find errors
	message = exploreMesh pmesh ;
	if (message != "OK") then
	(
		MessageBox ("\n There is a problem with your mesh:\n" + message + "\n\nOperation aborted") ;
	)
	else
	(	
		outFile = createfile (out_name + ".mesh.xml") ;
--		try 
		(
			-- comments:
			--format("<!--\nMaterial Ids used in this file:\n") to:outFile ;
			--materialIds = GetUsedMaterialIds m ;
			--print materialIds to:outFile ;
			--format("-->\n") to:outFile ;
			
			-- ecriture de l'entete
			format("<mesh>\n") to:outFile ;
	
			format("\t<submeshes>\n") to:outFile ;
			if (pmesh.material != undefined) then 
				matname = pmesh.material.name ;
			else matname = pmesh.name + "material" ;
		
			format("\t\t<submesh material = \"%\" useSharedVertices=\"false\">\n") matname to:outFile ;	
	
			writeM m exportOptions.flipyz exportOptions.scale exportOptions.flipNormal outFile ;
		
			sk = getSkin pmesh;
			if (sk != undefined) then
			(
				-- in order to perform, skin should be opened
				max modify mode ;
				modPanel.setCurrentObject pmesh.modifiers[#Skin] ;
			
				writeBoneAssignments m sk outFile ;
			)

			-- ecriture, fin des balises
			format("\t\t</submesh>\n") to:outFile ;
			format("\t</submeshes>\n") to:outFile ;
	
			-- <skeletonlink />	
			if (sk != undefined) then
			(
				t = filterstring out_name "\\" ;
				format ("\t<skeletonlink name=\"%\"/>\n") (t[arrayLength t] + ".skeleton") to:outFile ;
			)
			format("</mesh>\n") to: outFile ;
			delete m ;
			close outFile ;
			messageBox "Exporting mesh successful !" ;
		)
--		catch
--		(
--			messageBox "Error while exporting mesh: Sorry I can't tell you more...." ;
--			delete m ;
--			close outFile ;
--		)
	)
)