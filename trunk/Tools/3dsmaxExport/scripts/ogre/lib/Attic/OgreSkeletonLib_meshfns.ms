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
-- 3dsmax commence tous ses ID a 1 alors que Ogre commence à 0.
-- -> on va rajouter un vertex 0 et un bone 0 pour assurer la compatibilté.

----------------------------------------------------------------------------
-- write <mesh />
-- about axis: if xAxis = 1 and yAxis = 2 and zAxis = 3, nothing happens
--             but yAxis = 3 and zAxis = 2 will flip Y and Z
-- flipNormal = true if you want to flip normals
----------------------------------------------------------------------------

function writeM tmesh flipyz scale flipNormal outFile =
(
    local num_tverts, num_faces,v,f,face,nmal,vert,table,tvtable,vertTmp,tvertTmp,faceTmp ;
	
	format "writing mesh: writing geometry ... \n" ;
	
	table = getVertTable tmesh ;
	tvtable = getTVertTable tmesh ;
	num_tverts = tmesh.numtverts ;
	num_faces = tmesh.numfaces ;
	
	-- Vertices positions and normals
	--	
	format("\t\t\t<geometry count=\"%\">\n") (num_tverts+1) to:outFile; -- +1 cf remarque en haut.
	format("\t\t\t\t<vertexbuffer positions=\"true\" normals=\"true\" colours=\"false\" numtexcoords=\"1\" texcoordsets=\"0\" texcoorddimensions=\"2\">\n") to:outFile --> TODO, tounderstand :) !!!!              
		
		-- vertex number 0
		format("\t\t\t\t\t<vertex>\n") to:outFile ;
		format("\t\t\t\t\t\t<position x=\"0\" y=\"0\" z=\"0\" />\n") to:outFile ;
		format("\t\t\t\t\t\t<normal x=\"1\" y=\"0\" z=\"0\" />\n") to:outFile ;
		format("\t\t\t\t\t\t<texcoord u=\"0\" v=\"0\" />\n") to:outFile ;
		format("\t\t\t\t\t</vertex>\n") to:outFile ;

	for v = 1 to num_tverts do 
	( 
		tvert = getTvert tmesh v ; 
		vert = getVert tmesh (tvtable[v]) ;
		nmal = getNormal tmesh (tvtable[v]) ;
		
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
		face = getTVFace tmesh f ;
		-- flipNormals
		if (flipNormal) then
		(
			faceTmp = copy face ;
			face[1] = faceTmp[3] ;
			face[3] = faceTmp[1] ;
		)
		
		format("\t\t\t\t<face ") to:outFile ;
		format("v1=\"%\" v2=\"%\" v3=\"%\"") (face.x as Integer) (face.y as Integer) (face.z as Integer) to:outFile ;
		format(" />\n") to:outFile ;

	)
	format("\t\t\t</faces>\n") to:outFile ;
)

----------------------------
-- write <boneassignement />
----------------------------

function writeBoneAssignments tmesh sk outFile =
(
	local i,bid,num_tverts,vwcount,num_bones,j,w,table,vNum ;
	
	format "writing mesh: writing bone assignments ... \n" ;
	
	num_tverts = tmesh.numtverts ;
	table = getTVertTable tmesh ;
	num_bones = skinOps.GetNumberBones(sk) ;
	
	format("\t\t\t<boneassignments>\n") to:outFile ;
	for i=1 to num_tverts do
	(
		vNum = table[i] ; -- get the assiocated vertex.
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

---------------------------------
-- write <materials />
-- try to export only diffuse map
---------------------------------

function writeMaterial pmesh outFile =
(
	local t, matname, mat;

	mat = pmesh.material ;
	
	if (pmesh.material != undefined) then 
		matname = pmesh.material.name ;
	else matname = pmesh.name + "material" ;	
	
	format "writing mesh: writing material ... \n" ;
	format("\t<materials>\n") to:outFile ;
	format("\t\t<material name=\"%\">\n") matname to:outFile ;
    format("\t\t\t<ambient red=\"%\" green=\"%\" blue=\"%\" alpha=\"%\" />\n") (mat.ambient.r/255.0) (mat.ambient.g/255.0) (mat.ambient.b/255.0) (mat.ambient.alpha/255.0) to:outFile ;
    format("\t\t\t<diffuse red=\"%\" green=\"%\" blue=\"%\" alpha=\"%\" />\n") (mat.diffuse.r/255.0) (mat.diffuse.g/255.0) (mat.diffuse.b/255.0) (mat.diffuse.alpha/255.0) to:outFile ;
    format("\t\t\t<specular red=\"%\" green=\"%\" blue=\"%\" alpha=\"%\" />\n") (mat.specular.b/255.0) (mat.specular.g/255.0) (mat.specular.b/255.0) (mat.specular.alpha/255.0) to:outFile ;
    format("\t\t\t<shininess value=\"0\" />\n") to:outFile ;
    
	format("\t\t\t<texturelayers>\n") to:outFile ;
    -- if mesh has a material: reading the diffuse map:
		if (pmesh.material != undefined) then
			if ((pmesh.material.diffusemap != undefined) and (iskindof pmesh.material.diffusemap Bitmaptexture)) then
			(	
				t = filterstring pmesh.material.diffusemap.filename "\\" ;
				format("\t\t\t\t<texturelayer texture=\""+ t[arrayLength t] +"\" />\n") to:outFile ;
			)
	format("\t\t\t</texturelayers>\n") to:outFile ;
	
	format("\t\t</material>\n") to:outFile ;
    format("\t</materials>\n") to:outFile ;
)

---------------------------
-- write <sharedGeometry />
---------------------------

function writeSharedGeometry outFile =
(
	format "writing mesh: writing shared geometry ... \n" ;

	format("\t<sharedgeometry count=\"0\">\n") to:outFile ;
    format("\t\t<vertexbuffer positions=\"true\" normals=\"false\" colours=\"false\" numtexcoords=\"0\" />\n") to:outFile ;
    format("\t\t<vertexbuffer positions=\"false\" normals=\"false\" colours=\"false\" numtexcoords=\"1\" texcoordsets=\"0\" texcoorddimensions=\"2\" />\n") to:outFile ;
    format("\t</sharedgeometry>\n") to:outFile ;
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
			-- ecriture de l'entete
			format("<mesh>\n") to:outFile ;

			writeMaterial pmesh outFile ;
			-- TODO shared Geometry
			writeSharedGeometry outFile ;
	
	
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