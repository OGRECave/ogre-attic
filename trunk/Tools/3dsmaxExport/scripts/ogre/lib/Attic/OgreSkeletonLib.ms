----------------------------------------------------------------------------------------
-- ------------------------------ SOME USEFUL FUNCTIONS ----------------------------- --
----------------------------------------------------------------------------------------

-- point3[i] don't exist in 3dsmax3 so here is a function which get x,y or z
function p3get p i =
(
	if i==1 then p.x ;
	else if i==2 then p.y ;
	else p.z ;
)

-- point3[i] don't exist in 3dsmax3 so here is a function which set x,y or z
function p3set p i j=
(
	if i==1 then p.x = j ;
	else if i==2 then p.y = j ;
	else p.z = j;
)

-- return the length of an array
function arrayLength a = 
(
	local i ;
	i = 1 ;
	while (a[i] != undefined) do
		i = i + 1 ;	
	i-1 ;
)
-----------------------------------------------------------------------------
-- return the skin modifier or undefined if object don't have a skin modifier
-----------------------------------------------------------------------------

function getSkin obj =
(
	local s,i ;
	s = undefined ;
	for i in obj.modifiers do
	(
		if iskindof i Skin do
			s = i ;
	)
	s ;
)

--------------------------------------------------------
-- return an Array with the ID of root bones of the skin
--------------------------------------------------------

function getRoots skin =
(
	local rootstab,n,i,c,d ;
	rootstab = #() ;
	n = skinOps.GetNumberBones skin ;
	for i = 1 to n do
	(
		c= skinOps.GetBoneName skin i 1 ;
		d = execute ("$" + c) ;
		if (d.parent == undefined) then
			append rootstab d ;
	)
	rootstab ;
)

------------------------------------------------------------
-- write corresponding tables between vertices and tvertices
------------------------------------------------------------

-- return an array whose length is (getnumverts mesh)
-- and whose elements are #(numtvert1, numtvert2, ...)

function getVertTable mesh =
(
	local table1,i,f,tvf ;
	table = #() ;
	for i = 1 to (getnumverts mesh) do
		(
		append table #() ;
	) ;
	
	for i = 1 to (getnumfaces mesh) do
	(
		tvf = gettvface mesh i ;
		f = getface mesh i ;
		if (finditem table[f.x] tvf.x) == 0 then
			append (table[f.x]) (tvf.x as Integer) ;
		if (finditem table[f.y] tvf.y) == 0 then
			append (table[f.y]) (tvf.y as Integer) ;	
		if (finditem table[f.z] tvf.z) == 0 then
			append (table[f.z]) (tvf.z as Integer) ;
	)
	table ;
)

-- return an array whose length is (getnumtverts mesh)
-- and whose elements are the number of the assiociated vertex.

function getTVertTable mesh =
(
	local table1,i,f,tvf ;
	table = #() ;
	for i = 1 to (getnumverts mesh) do
		(
		append table 0 ;
	) ;
	
	for i = 1 to (getnumfaces mesh) do
	(
		tvf = gettvface mesh i ;
		f = getface mesh i ;
		
		table[tvf.x] = f.x as Integer ;
		table[tvf.y] = f.y as Integer ;
		table[tvf.z] = f.z as Integer ;
	)
	table ;
)

---------------------------------------------------------------------------------------
-- exploreKeys return an array #(message1, boneName1, message2, boneName2, ... )
-- where message =
-- OK if when a parameter (posX,posY,posZ or rotX,rotY,rotZ) has a key,
--       all the others have also a key.
-- an error message else.
-- in fact you have to test the first element. if it's OK, good!, else look at messages
---------------------------------------------------------------------------------------

function exploreKeys sk =
(
	local i,d,boneName,rotContr,posContr,subRotContr,subPosContr ;
	local nkeys1,nkeys2,nkeys3,time1,time2,time3 ;
	local answer ;
	
	
	answer = #() ;
	
	for i=1 to (skinOps.getNumberBones sk) do
	(		
		boneName= skinOps.GetBoneName sk i 1 ;

		--format "verifying key information for % ...\n" boneName ;
		
		d = execute ("$" + boneName) ;
		rotContr = d.rotation.controller ;
		posContr = d.pos.controller ;
		--3dsmax5 :
		subRotContr = getXYZControllers rotContr ;
		subPosContr = getXYZControllers posContr ;
		--3dsmax3
		--subRotContr = #(rotContr.x_rotation.controller,rotContr.y_rotation.controller,rotContr.z_rotation.controller) ;
		--subPosContr = #(posContr.x_position.controller,posContr.y_position.controller,posContr.z_position.controller) ;
	
		nkeys1 = numKeys subRotContr[1] ;
		nkeys2 = numKeys subRotContr[2] ;
		nkeys3 = numKeys subRotContr[3] ;
		
		if (nkeys1 != nkeys2 or nkeys1 != nkeys3) then
		(
			append answer "Number of Keys is not the same for all tracks" ;
			append answer boneName ;
		)
		for j=1 to nkeys1 do
		(
			time1 = (getKey subRotContr[1] j).time ;
			time2 = (getKey subRotContr[2] j).time ;
			time3 = (getKey subRotContr[3] j).time ;
			if (time1 != time2 or time1 != time3) then
			(
				append answer "At a frame, all tracks don't have a key" ;
				append answer boneName ;
			)
		)	
	)
	append answer "OK" ;
	--format "results of verification: % \n" answer ;
	answer ;
)


-----------------------------------------------------------------------------
-- get allkeys return an array of #(time (int), axis (point3), pos (point3)).
-- Selected keys belongs to [firstframe,lastFrame]
-- time = (key.time - firstFrame)*length/(lastFrame-firstFrame)
-- nb: so first key has time 0.
-----------------------------------------------------------------------------

function getAllKeys sk boneId firstFrame lastFrame length out_file =
(
	local angle,ax,timef,nKeys ,i,j,tmp;
	local bname,d,rotContr,posContr,subRotContr,subPosContr,rotValues,posValues ;
	
	--format "retrieving key information for % ...\n" (skinOps.GetBoneName sk boneId 1) ;
	
	a = #() ;
	ax = #() ;
	angle = 0 ;
	timef = 0 ;
	
	bname= skinOps.GetBoneName sk boneId 1 ;
	d = execute ("$" + bname) ;
	
	rotContr = d.rotation.controller ;
	posContr = d.pos.controller ;
	--3dsmax5 :
	subRotContr = getXYZControllers rotContr ;
	subPosContr = getXYZControllers posContr ;
	--3dsmax3 : 
	--subRotContr = #(rotContr.x_rotation.controller,rotContr.y_rotation.controller,rotContr.z_rotation.controller) ;
	--subPosContr = #(posContr.x_position.controller,posContr.y_position.controller,posContr.z_position.controller) ;
	-- TODO: euh... surement des restrictions sur le type et le nombre de controlleurs .
	
	nkeys = numKeys subRotContr[1] ;
	-- attention limitation:
	-- il faut qu'a une meme frame, il y ait toutes les cles: position x, y, z et rotation x, y, z.
	-- utiliser exploreKeys...
	
	for i = 1 to nkeys do
	(
		--3dsmax3: timef = ((getKey subRotContr[1] i).time as float)/160 ;
		--3dsmax5
		timef = (float) ((getKey subRotContr[1] i).time) ;
		
		if (timef >=firstFrame and timef<=lastFrame) then
		(
			timef = (timef-firstFrame)*length/(lastFrame-firstFrame) ;
			rotValues = [0,0,0] ;
			posValues = [0,0,0] ;
		
			-- enregistrement des valeurs des controllers
			for j = 1 to 3 do
			(
				tmp = (radToDeg ((getKey subRotContr[j] i).value)) ;
				--3dsmax3 : p3set rotValues j tmp;
				--3dsmax5:
				rotValues[j] = tmp ;
			
				tmp = (getKey subPosContr[j] i).value ;
				
				--3dsmax3 : p3set posValues j tmp;
				--3dsmax5:
				posValues[j] = tmp ;
			)
			b = #(timef,rotValues,posValues)  ;
			append a b ;
		)
	)
	
	a ;
)
----------------------------------------------------------------------------------------
-- ----------------------------------- WRITE MESH ----------------------------------- --
----------------------------------------------------------------------------------------
-- Remarque !
-- 3dsmax commence tous ses ID a 1 alors que Ogre commence à 0.
-- -> on va rajouter un vertex 0 et un bone 0 pour assurer la compatibilté.

-----------------
-- write <mesh />
-----------------

function writeM tmesh out_file =
(
    local num_verts, num_faces,v,f,face,nmal,vert,table,tvtable ;
	
	format "writing mesh: writing geometry ... \n" ;
	
	table = getVertTable tmesh ;
	tvtable = getTVertTable tmesh ;
	num_verts = tmesh.numtverts ;
	num_faces = tmesh.numfaces ;
	
	-- Vertices positions and normals
	--	
	format("\t\t\t<geometry count=\"%\">\n") (num_verts+1) to:out_file; -- +1 cf remarque en haut.
	format("\t\t\t\t<vertexbuffer positions=\"true\" normals=\"true\" colours=\"false\" numtexcoords=\"1\" texcoordsets=\"0\" texcoorddimensions=\"2\">\n") to:out_file --> TODO, tounderstand :) !!!!              
		
		-- vertex number 0
		format("\t\t\t\t\t<vertex>\n") to:out_file ;
		format("\t\t\t\t\t\t<position x=\"0\" y=\"0\" z=\"0\" />\n") to:out_file ;
		format("\t\t\t\t\t\t<normal x=\"1\" y=\"0\" z=\"0\" />\n") to:out_file ;
		format("\t\t\t\t\t\t<texcoord u=\"0\" v=\"0\" />\n") to:out_file ;
		format("\t\t\t\t\t</vertex>\n") to:out_file ;

	for v = 1 to num_verts do 
	( 
		tvert = getTvert tmesh v ; 
		vert = getVert tmesh (tvtable[v]) ;
		nmal = getNormal tmesh (tvtable[v]) ;

		format("\t\t\t\t\t<vertex>\n") to:out_file ;
		format("\t\t\t\t\t\t<position x=\"%\" y=\"%\" z=\"%\" />\n") vert.x vert.y vert.z to:out_file ;
		format("\t\t\t\t\t\t<normal x=\"%\" y=\"%\" z=\"%\" />\n") nmal.x nmal.y nmal.z to:out_file ;
		format("\t\t\t\t\t\t<texcoord u=\"%\" v=\"%\" />\n") tvert.x tvert.y to:out_file ;		
		format("\t\t\t\t\t</vertex>\n") to:out_file ;
	)
	format("\t\t\t\t</vertexbuffer>\n") to:out_file ;

	format("\t\t\t</geometry>\n") to:out_file ;

	-- Faces
	--
	format("\t\t\t<faces count=\"%\">\n") num_faces to:out_file;
	for f = 1 to num_faces do 
	( 
		face = getTVFace tmesh f ;
		format("\t\t\t\t<face ") to:out_file ;
		format("v1=\"%\" v2=\"%\" v3=\"%\"") (face.x as Integer) (face.y as Integer) (face.z as Integer) to:out_file ;
		format(" />\n") to:out_file ;

	)
	format("\t\t\t</faces>\n") to:out_file ;
)

----------------------------
-- write <boneassignement />
----------------------------

function writeBoneAssignments tmesh sk out_file =
(
	local i,bid,num_verts,vwcount,num_bones,j,w ;
	
	format "writing mesh: writing bone assignments ... \n" ;
	
	num_verts = tmesh.numverts ;
	num_bones = skinOps.GetNumberBones(sk) ;
	
	format("\t\t\t<boneassignments>\n") to:out_file ;
	for i=1 to num_verts do
	(
		vwcount = skinOps.getVertexWeightCount sk i ;
		for j=1 to vwcount do
		(
			bid = skinOps.getVertexWeightBoneID sk i j ;
			w = skinOps.getVertexWeight sk i j ;
			-- Condition pour qu'un bone soit pris en compte: weight > 0.1
			if w > 0.01 then
	    		format("\t\t\t\t<vertexboneassignment vertexIndex=\"%\" boneIndex=\"%\" weight=\"%\" />\n") i bid w to:out_file ;
		)
	)
	format("\t\t\t</boneassignments>\n") to:out_file ;
)

---------------------------------
-- write <materials />
-- try to export only diffuse map
---------------------------------

function writeMaterial pmesh out_file =
(
	local t ;
	
	format "writing mesh: writing material ... \n" ;

	format("\t<materials>\n") to:out_file ;
    format("\t\t<material name=\"3dsdefault\">\n") to:out_file ;
    format("\t\t\t<ambient red=\"1\" green=\"1\" blue=\"1\" alpha=\"1\" />\n") to:out_file ;
    format("\t\t\t<diffuse red=\"1\" green=\"1\" blue=\"1\" alpha=\"1\" />\n") to:out_file ;
    format("\t\t\t<specular red=\"1\" green=\"1\" blue=\"1\" alpha=\"1\" />\n") to:out_file ;
    format("\t\t\t<shininess value=\"0\" />\n") to:out_file ;
    
	format("\t\t\t<texturelayers>\n") to:out_file ;
    -- if mesh has a material: reading the diffuse map:
		if (pmesh.material != undefined) then
			if ((pmesh.material.diffusemap != undefined) and (iskindof pmesh.material.diffusemap Bitmaptexture)) then
			(	
				t = filterstring pmesh.material.diffusemap.filename "\\" ;
				format("\t\t\t\t<texturelayer texture="+ t[arrayLength t] +" />\n") to:out_file ;
			)
	format("\t\t\t</texturelayers>\n") to:out_file ;
	
	format("\t\t</material>\n") to:out_file ;
    format("\t</materials>\n") to:out_file ;
)

---------------------------
-- write <sharedGeometry />
---------------------------

function writeSharedGeometry out_file =
(
	format "writing mesh: writing shared geometry ... \n" ;

	format("\t<sharedgeometry count=\"0\">\n") to:out_file ;
    format("\t\t<vertexbuffer positions=\"true\" normals=\"false\" colours=\"false\" numtexcoords=\"0\" />\n") to:out_file ;
    format("\t\t<vertexbuffer positions=\"false\" normals=\"false\" colours=\"false\" numtexcoords=\"1\" texcoordsets=\"0\" texcoorddimensions=\"2\" />\n") to:out_file ;
    format("\t</sharedgeometry>\n") to:out_file ;
)

--------------------------------
-- write the mesh: main function
--------------------------------

function writeMesh pmesh out_name =
(
	local m,sk,out_file ;
	
	-- 3dsmax5: snapshotAsMesh
	m = snapshotAsMesh pmesh ;
	
	out_file = createfile (out_name + ".mesh.xml") ;
	try 
	(
		-- ecriture de l'entete
		format("<mesh>\n") to:out_file ;

		writeMaterial pmesh out_file ;
		-- TODO shared Geometry
		writeSharedGeometry out_file ;
	
	
		format("\t<submeshes>\n") to:out_file ;	
		format("\t\t<submesh material = \"3dsdefault\" useSharedVertices=\"false\">\n") to:out_file ;	
	
		writeM m out_file ;
		
		sk = getSkin pmesh;

		if (sk != undefined) then
		(
			-- in order to perform, skin should be opened
			max modify mode ;
			modPanel.setCurrentObject pmesh.modifiers[#Skin] ;
			
			writeBoneAssignments pmesh sk out_file ;
		)

		-- ecriture, fin des balises
		format("\t\t</submesh>\n") to:out_file ;
		format("\t</submeshes>\n") to:out_file ;
	
		-- <skeletonlink />	
		if (sk != undefined) then
		(
			t = filterstring out_name "\\" ;
			format ("\t<skeletonlink name=\"%\"/>\n") (t[arrayLength t] + ".skeleton") to:out_file ;
		)
		format("</mesh>\n") to: out_file ;
		delete m ;
		close out_file ;
		messageBox "Exporting mesh successful !" ;
	)
	catch
	(
		messageBox "Error while exporting mesh: Sorry I can't tell you more...." ;
		delete m ;
		close out_file ;
	)
)

-------------------------------------------------------------------------------------------------
------------------------------------------- WRITE SKELETON --------------------------------------
-------------------------------------------------------------------------------------------------


-- Remarque !
-- 3dsmax commence tous ses ID a 1 alors que Ogre commence à 0.
-- -> on va rajouter un vertex 0 et un bone 0 pour assurer la compatibilté.


------------------
-- write <bones />
------------------

function writeB id sk keys out_file =
(
	local c,d,k,aax,aay,aaz,aa,mx,my,mz,angle ;
	
	c= skinOps.GetBoneName sk id 1 ;
	d = execute ("$" + c) ;
	format("\t\t<bone id=\"%\" name=\"%\">\n") id d.name to:out_file ;
	
	k = keys[id][1] ;
	
	aaX = Angleaxis (p3get (k[2]) 1) (point3 1 0 0) ;
	aaY = Angleaxis (p3get (k[2]) 2) (point3 0 1 0) ;
	aaZ = Angleaxis (p3get (k[2]) 3) (point3 0 0 1) ;
			
	mX = aaX as matrix3 ;
	mY = aaY as matrix3 ;
	mZ = aaZ as matrix3 ;
		
	m = mZ*mY*mX ;
			
	aa = m as angleaxis ;
	angle = aa.angle ;
	angle = degToRad(angle) ;
	
	format("\t\t\t<position x=\"%\" y=\"%\" z=\"%\" />\n") k[3].x k[3].y k[3].z to:out_file ;
    format("\t\t\t<rotation angle=\"%\">\n") angle to:out_file ;
    format("\t\t\t\t<axis x=\"%\" y=\"%\" z=\"%\" />\n") aa.axis.x aa.axis.y aa.axis.z to:out_file ;
    format("\t\t\t</rotation>\n") to:out_file ;
	format("\t\t</bone>\n") to:out_file ;
)
--------------
-- write Bones (using writeB)
--------------

function writeBones sk keys out_file =
(
	local i ;
	
	format("\t<bones>\n") to:out_file;
	
	-- creation du bone zero: cf remarque tout en haut.
	format("\t\t<bone id=\"0\" name=\"zzwxy\">\n") to:out_file ;
	format("\t\t\t<position x=\"0\" y=\"0\" z=\"0\" />\n") to:out_file ;
    format("\t\t\t<rotation angle=\"0\">\n") to:out_file ;
    format("\t\t\t\t<axis x=\"1\" y=\"0\" z=\"0\" />\n") to:out_file ;
    format("\t\t\t</rotation>\n") to:out_file ;
	format("\t\t</bone>\n") to:out_file ;
	i = 0 ;
	for k in keys do
	(
		i = i + 1 ; 
		writeB i sk keys out_file ;
	)
	format("\t</bones>\n") to:out_file;
)

-----------------------------------
-- recursive fun to write hierarchy
-----------------------------------

function writeH b out_file =
(
	if (b.parent != undefined) do
	(
		p = b.parent ;
		format("\t\t<boneparent bone=\"%\" parent=\"%\" />\n") b.name p.name to:out_file ;
	)
	childrenArray = b.children ;
	for i=1 to childrenArray.count do
	(
		writeH childrenArray[i] out_file ;
	)
)

--------------------------
-- write <bonehierarchy />
--------------------------

function writeHierarchy sk out_file =
(
	format("\t<bonehierarchy>\n") to:out_file ;
	-- !!! 3dsmax 3 ne permet pas de prendre la racine d'un squelette dans "peau" donc on supposera que c'est le 1er
	--3dsmax3 :	roots = #(execute("$" + (skinops.getBoneName sk 2 1))) ;
	--3dsmax5 : 
	roots = getRoots sk ;
	
	-- ecriture de la track pour le bone 0.
	format("\t\t<boneparent bone=\"zzwxy\" parent=\"%\" />\n") roots[1].name to:out_file ;
	for b in roots do
	(
		writeH b out_file;
	)
	format("\t</bonehierarchy>\n") to:out_file ;
)

--------------------------------------------------------------------------------------------------------------------------------------
-- write <track />
-- initialKeys are the keys for the frame 0, because we usually create mesh and bones at frame 0.
--   But animation we want to export can start at another frame number. And initial Rotation are expected in order to exportation work.
--   Use getAllkeys with firstFrame=0 and lastFrame = 0
-- keys is the array returned by getAllKeys
---------------------------------------------------------------------------------------------------------------------------------------

function writeTrack sk initialKeys keys boneId out_file =
(
	local bname,d,k1,k,trans,rot,mX,mY,mZ ;
	
	bname= skinOps.GetBoneName sk boneId 1 ;
	d = execute ("$" + bname) ;
	
	format("\t\t\t\t<track bone = \"%\">\n") d.name to: out_file ;
	format("\t\t\t\t\t<keyframes>\n") to:out_file ;

	k1 = initialKeys[boneId][1] ;
	
	for k in keys[boneId] do
	(	
		trans = k[3] - k1[3] ;
		rot = k[2] - k1[2] ;
		
		aaX = Angleaxis (k[2].x - k1[2].x) (point3 1 0 0) ;
		aaY = Angleaxis (k[2].y - k1[2].y) (point3 0 1 0) ;
		aaZ = Angleaxis (k[2].z - k1[2].z) (point3 0 0 1) ;
			
		mX = aaX as matrix3 ;
		mY = aaY as matrix3 ;
		mZ = aaZ as matrix3 ;
		
		m = mZ*mY*mX ;
				
		aa = m as angleaxis ;
		angle = aa.angle ;
		angle = degToRad(angle) ;

		format("\t\t\t\t\t\t<keyframe time=\"%\">\n") k[1] to: out_file ;
		format("\t\t\t\t\t\t\t<translate x=\"%\" y=\"%\" z=\"%\" />\n") trans.x trans.y trans.z to: out_file ;
		format("\t\t\t\t\t\t\t<rotate angle=\"%\">\n") angle to:out_file ; 
		format("\t\t\t\t\t\t\t\t<axis x=\"%\" y=\"%\" z=\"%\" />\n") (aa.axis.x) (aa.axis.y) (aa.axis.z) to:out_file ;
		format("\t\t\t\t\t\t\t</rotate>\n") to:out_file ;
		format("\t\t\t\t\t\t</keyframe>\n") to:out_file ;
	)
	format("\t\t\t\t\t</keyframes>\n") to:out_file ;
	format("\t\t\t\t</track>\n") d.name to: out_file ;

)

-----------------------
-- write <animations />
-----------------------

function writeAnim sk initialKeys keys name length out_file =
(
	format("\t<animations>\n") to: out_file ;
    format("\t\t<animation name=\"%\" length=\"%\">\n") name length to:out_file ;
	format("\t\t\t<tracks>\n") to:out_file
	
	-- ecriture track 0
	format("\t\t\t\t<track bone = \"zzwxy\">\n") to: out_file ;
	format("\t\t\t\t\t<keyframes>\n") to:out_file ;
	format("\t\t\t\t\t\t<keyframe  time=\"0\">\n") to:out_file ;
	format("\t\t\t\t\t\t\t<translate x=\"0\" y=\"0\" z=\"0\" />\n") to: out_file ;
	format("\t\t\t\t\t\t\t<rotate angle=\"0\">\n")  to:out_file ; 
	format("\t\t\t\t\t\t\t\t<axis x=\"1\" y=\"0\" z=\"0\" />\n")  to:out_file ;
	format("\t\t\t\t\t\t\t</rotate>\n") to:out_file ;
	format("\t\t\t\t\t\t</keyframe>\n") to:out_file ;
	format("\t\t\t\t\t</keyframes>\n") to:out_file ;
	format("\t\t\t\t</track>\n") to: out_file ;
	
	n = skinOps.GetNumberBones sk ;
	for i = 1 to n do
		writeTrack sk initialKeys keys i out_file ;

	format("\t\t\t</tracks>\n") to:out_file					
	format("\t\t</animation>\n") to: out_file ;
	format("\t</animations>\n") to: out_file ;
)

-------------------------------------------------------------
-- write <skeleton /> main function
-- write the animation in the file out_name + ".skeleton.xml"
-- between the frame firstFrame and lastFrame 
-- and scale time according to length
-------------------------------------------------------------

function writeSkeleton pmesh firstFrame lastFrame length animName out_name=
(
	local sk,n,keys,initialKeys,messages ;
	sk = getSkin pmesh ;
	
	format "Length: %\n" length ;
	
	if (sk == undefined) then
	(
		close out_file ;
		MessageBox "There is no skin modifier for this object" ;
	)
	else
	(	
		-- in order to perform, skin should be opened
		max modify mode ;
		modPanel.setCurrentObject pmesh.modifiers[#Skin] ;
		
		-- trying to find errors
		messages = exploreKeys sk ;
		if (messages[1] != "OK") then
		(
			MessageBox ("\n There is a problem with bone: " + messages[2] + "\n\nHere is the found error:\n" + messages[1] + "\n\nOperation aborted") ;
		)
		else
		(
			-- create the output file
			out_file = createfile (out_name + ".skeleton.xml") ;

			try
			(
				-- ecriture de l'entete
				format("<skeleton>\n") to:out_file ;	
	
				-- get all keys of all bones in keys.
				n = skinOps.GetNumberBones sk ;
				keys = #() ;
				for i = 1 to n do
					append keys (getAllkeys sk i firstFrame lastFrame length out_file) ;
				-- and now get keys at frame 0. they will be the parameter initialKeys.
				initialKeys = #() ;
				for i = 1 to n do
					append initialKeys (getAllkeys sk i 0 0 length out_file) ;
				
				writeBones sk initialKeys out_file ;
				writeHierarchy sk out_file ;
				writeAnim sk initialKeys keys animName length out_file ;

				-- ecriture, fin des balises
				format("</skeleton>\n") to: out_file ;
				close out_file ;
				messageBox "Exporting skeleton successful !"
			)
			catch
			(
				messageBox "Error while exporting skeleton: Sorry I can't tell you more...." ;
				close out_file ;
			)
		)
	)
	
)

