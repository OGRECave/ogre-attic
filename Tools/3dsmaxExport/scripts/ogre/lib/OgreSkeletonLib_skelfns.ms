-----------------------------------------------------------------------------
-- gets from the controllers animation keyframes
-----------------------------------------------------------------------------
function getTimeList obj firstframe lastframe =
(
	local list,rotContr,posContr,  ;
	list = #(firstframe) ;

	rotContr = obj.rotation.controller ;
	posContr = obj.pos.controller ;
	
	subRotContr = getXYZControllers rotContr ;
	subPosContr = getXYZControllers posContr ;

	for i=1 to 3 do (
		for j=1 to (numKeys subRotContr[i]) do (
			time = (getKey subRotContr[i] j).time ;
			if (time<=lastframe and time>=firstframe) then
				appendOrdered list time
		)
		
		for j=1 to (numKeys subPosContr[i]) do (
			time = (getKey subPosContr[i] j).time ;
			if (time<=lastframe and time>=firstframe) then
				appendOrdered list time
			
		)
	)
	
	list ;
)


-----------------------------------------------------------------------------
-- write <track />
-- Selected keys belongs to [firstframe,lastFrame]
-- time = (key.time - firstFrame)*length/(lastFrame-firstFrame)
--   (e.g. first key has time 0.)
-----------------------------------------------------------------------------

function writeTrack sk boneId firstframe lastframe length scale outFile=
(
	local angle,timef,i,bname,d ;
	
	-- displays information in the maxscript listener
	format "retrieving key information for % ...\n" (skinOps.GetBoneName sk boneId 1) ;
	
	-- gets bone acording to the parameter boneId
	bname= skinOps.GetBoneName sk boneId 1 ;
	replaceSpaces bname ;
	d = execute ("$" + bname) ;
	
	-- gets keyframe list
	timelist = getTimeList d firstframe lastframe ;
	
	-- track header
	format("\t\t\t\t<track bone = \"%\">\n") d.name to:outFile ;
	format("\t\t\t\t\t<keyframes>\n") to:outFile ;
	
	-- gets initial transform at frame 0f
	at time 0f (
		initTform = d.transform ;
		if (d.parent != undefined) then (
			mparent = d.parent.transform ;
			initTform = initTform*inverse(mparent) ;
		)
	)
		
	-- for each frame in the list
	for i in timelist do
	(
		-- moves slider time and compute OGRE time
		at time i (
			timef = ((float) (i-firstFrame)*length)/(lastframe - firstframe ) ;
			
			-- gets transform		
			Tform = d.transform ;
			if (d.parent == undefined) then
				mparent = initTform ;
			else
				mparent = initTform*d.parent.transform ;
				
			Tform = Tform*inverse(mparent) ;
	
			-- extracts datas		
			pos = Tform.translationpart * scale ;
			rot = Tform.rotation as angleaxis ;
			angle = - degToRad (rot.angle) ; -- don't know why there must be this minus :((((((
				
			-- writes them !
			format("\t\t\t\t\t\t<keyframe time=\"%\">\n") timef to: outFile ;
			format("\t\t\t\t\t\t\t<translate x=\"%\" y=\"%\" z=\"%\" />\n") pos.x pos.y pos.z to: outFile ;
			format("\t\t\t\t\t\t\t<rotate angle=\"%\">\n") angle to:outFile ; 
			format("\t\t\t\t\t\t\t\t<axis x=\"%\" y=\"%\" z=\"%\" />\n") (rot.axis.x) (rot.axis.y) (rot.axis.z) to:outFile ;
			format("\t\t\t\t\t\t\t</rotate>\n") to:outFile ;
			format("\t\t\t\t\t\t</keyframe>\n") to:outFile ;
		)
	)
	
	-- track end
	format("\t\t\t\t\t</keyframes>\n") to:outFile ;
	format("\t\t\t\t</track>\n") d.name to: outFile ;
)


-------------------------------------------------------------------------------------------------
------------------------------------------- WRITE SKELETON --------------------------------------
-------------------------------------------------------------------------------------------------

-- Nota :
-- A vertex 0 and a bone 0 are added because 3dsmax starts with vertex 1 and bone 1.


------------------
-- write <bones />
------------------

function writeB sk id scale flipYZ outFile =
(
	-- gets bone acording to the parameter boneId
	bname= skinOps.GetBoneName sk id 1 ;
	replaceSpaces bname ;
	d = execute ("$" + bname) ;
	
	-- gets initial transform at frame 0f
	format("\t\t<bone id=\"%\" name=\"%\">\n") id d.name to:outFile ;
	
	slidertime = 0f ;
	Tform = d.transform ;
	if (d.parent != undefined) then (
		mparent = d.parent.transform ;
		Tform = Tform*inverse(mparent) ;
	)
	
	pos = Tform.translationpart * scale ;
		
	if (d.parent == undefined and flipYZ) then (
		format "- Flipping root bone... \n" ;
		-- rotate
		axis1 = point3 1 0 0 ;
		axis2 = point3 0 0 1 ;
		axis3 = point3 0 -1 0 ;
		t = point3 0 0 0 ;
		m=matrix3 axis1 axis2 axis3 t ;
		Tform = Tform*inverse(m) ;
		-- change pos
		tmp = pos.z ;
		pos.z = -pos.y ;
		pos.y = tmp ;
		
	)
	
	rot = Tform.rotation as angleaxis ;
	angle = - degToRad (rot.angle) ; -- don't know why there must be this minus :((((((
	
	format("\t\t\t<position x=\"%\" y=\"%\" z=\"%\" />\n") pos.x pos.y pos.z to:outFile ;
    format("\t\t\t<rotation angle=\"%\">\n") angle to:outFile ;
    format("\t\t\t\t<axis x=\"%\" y=\"%\" z=\"%\" />\n") rot.axis.x rot.axis.y rot.axis.z to:outFile ;
   	format("\t\t\t</rotation>\n") to:outFile ;
	format("\t\t</bone>\n") to:outFile ;
)
-----------------------------
-- write Bones (using writeB)
-----------------------------

function writeBones sk scale flipYZ outFile =
(
	local i ;
	
	format("\t<bones>\n") to:outFile;
	
	-- we create a virtual bone number 0.
	format("\t\t<bone id=\"0\" name=\"zzwxy\">\n") to:outFile ;
	format("\t\t\t<position x=\"0\" y=\"0\" z=\"0\" />\n") to:outFile ;
    format("\t\t\t<rotation angle=\"0\">\n") to:outFile ;
    format("\t\t\t\t<axis x=\"1\" y=\"0\" z=\"0\" />\n") to:outFile ;
    format("\t\t\t</rotation>\n") to:outFile ;
	format("\t\t</bone>\n") to:outFile ;
	i = 0 ;
	for i=1 to (skinOps.GetNumberBones sk) do
	(
		writeB sk i scale flipYZ outFile ;
	)
	format("\t</bones>\n") to:outFile;
)

-----------------------------------
-- recursive fun to write hierarchy
-----------------------------------

function writeH b outFile =
(
	if (b.parent != undefined) do
	(
		p = b.parent ;
		format("\t\t<boneparent bone=\"%\" parent=\"%\" />\n") b.name p.name to:outFile ;
	)
	childrenArray = b.children ;
	for i=1 to childrenArray.count do
	(
		writeH childrenArray[i] outFile ;
	)
)

--------------------------
-- write <bonehierarchy />
--------------------------

function writeHierarchy sk outFile =
(
	format("\t<bonehierarchy>\n") to:outFile ;
	--3dsmax3 :	roots = #(execute("$" + (skinops.getBoneName sk 2 1))) ;
	--3dsmax5 : 
	roots = getRoots sk ;
	
	-- ecriture de la track pour le bone 0.
	format("\t\t<boneparent bone=\"zzwxy\" parent=\"%\" />\n") roots[1].name to:outFile ;
	for b in roots do
	(
		writeH b outFile;
	)
	format("\t</bonehierarchy>\n") to:outFile ;
)

-----------------------
-- write <animations />
-----------------------
function writeAnim sk firstFrame lastFrame length scale name outFile =
(
	local i,n ;
	
	format("\t<animations>\n") to: outFile ;
    format("\t\t<animation name=\"%\" length=\"%\">\n") name length to:outFile ;
	format("\t\t\t<tracks>\n") to:outFile
	
	-- ecriture track 0
	format("\t\t\t\t<track bone = \"zzwxy\">\n") to: outFile ;
	format("\t\t\t\t\t<keyframes>\n") to:outFile ;
	format("\t\t\t\t\t\t<keyframe  time=\"0\">\n") to:outFile ;
	format("\t\t\t\t\t\t\t<translate x=\"0\" y=\"0\" z=\"0\" />\n") to: outFile ;
	format("\t\t\t\t\t\t\t<rotate angle=\"0\">\n")  to:outFile ; 
	format("\t\t\t\t\t\t\t\t<axis x=\"1\" y=\"0\" z=\"0\" />\n")  to:outFile ;
	format("\t\t\t\t\t\t\t</rotate>\n") to:outFile ;
	format("\t\t\t\t\t\t</keyframe>\n") to:outFile ;
	format("\t\t\t\t\t</keyframes>\n") to:outFile ;
	format("\t\t\t\t</track>\n") to: outFile ;
	
	n = skinOps.GetNumberBones sk ;
	liste = #(0,20,30,40) ;
	for i = 1 to n do
		writeTrack sk i firstframe lastframe length scale outFile ;

	format("\t\t\t</tracks>\n") to:outFile					
	format("\t\t</animation>\n") to: outFile ;
	format("\t</animations>\n") to: outFile ;
)

-------------------------------------------------------------
-- write <skeleton /> main function
-- write the animation in the file out_name + ".skeleton.xml"
-- between the frame firstFrame and lastFrame 
-- and scale time according to length
-------------------------------------------------------------

function writeSkeleton pmesh exportOptions out_name = 
(
	local sk,n,keys,initialKeys,messages ;
	sk = getSkin pmesh ;
	
	if (sk == undefined) then
	(
		MessageBox "There is no skin modifier for this object" ;
	)
	else
	(	
		-- in order to perform, skin should be opened
		max modify mode ;
		modPanel.setCurrentObject pmesh.modifiers[#Skin] ;
		
		if (false) then (
		)
		else
		(
			format "------------------------------------------\n"
			format "------ OGRE skeleton Exporter Log   ------\n"
			format "------------------------------------------\n"
		
			format "Exporter options :\n"
			format "- firstFrame: % \n- lastFrame: %\n" exportOptions.firstFrame exportOptions.lastFrame ;
			-- creates the output file
			outFile = createfile (out_name + ".skeleton.xml") ;

				-- writes header
				format("<skeleton>\n") to:outFile ;	
	
				format "Writing bones :\n" ;
				writeBones sk exportOptions.scale exportOptions.flipYZ outFile ;
				format "Writing bone hierarchy.\n" ;
				writeHierarchy sk outFile ;
				format "Writing bone tracks.\n" ;
				writeAnim sk exportOptions.firstFrame exportOptions.lastFrame exportOptions.length exportOptions.scale exportOptions.animName outFile ;

				-- ecriture, fin des balises
				format("</skeleton>\n") to: outFile ;
				
			format "------------------------------------------\n"
			format "----------          END          ---------\n"
			format "------------------------------------------\n"
		
				close outFile ;
				messageBox "Exporting skeleton successful !"
		)
	)
	
)



