-------------------------
-- Structures definitions
-------------------------

-- about axis: if xAxis = 1 and yAxis = 2 and zAxis = 3, nothing happens
--             but yAxis = 3 and zAxis = 2 will flip Y and Z
-- flipNormal = true or false

struct exportOptions (firstFrame, lastFrame, length, xAxis, yAxis, zAxis, flipyz, scale, flipNormal, animName) ;


----------------------------------------------------------------------------------------
-- ------------------------------ SOME USEFUL FUNCTIONS ----------------------------- --
----------------------------------------------------------------------------------------

---------------------------------------------------------------------
-- replace " " by "_" in a string.
-- when a name is for example Left Biceps max knows it at Left_Biceps
-- and execute fonction will not work if you don't use this function
---------------------------------------------------------------------
function replaceSpaces s =
(
	for i=1 to s.count do
	(
		if (s[i] == " ") then
			s[i] = "_" ;
	)
	s ;
)


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
--------------------------------
-- return the length of an array
--------------------------------
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
	if obj != undefined then
		for i in obj.modifiers do
		(
			if iskindof i Skin do
				s = i ;
		)
	s ;
)
--------------------------------------------------
-- return an Array with the root bones of the skin
--------------------------------------------------

function getRoots skin =
(
	local rootstab,n,i,c,d ;
	rootstab = #() ;
	n = skinOps.GetNumberBones skin ;
	for i = 1 to n do
	(
		c= skinOps.GetBoneName skin i 1 ;
		replaceSpaces c ;
		d = execute ("$" + c) ;
		if (d.parent == undefined) then
			append rootstab d ;
	)
	rootstab ;
)

--------------------------------------------------------
-- return an Array with the ID of root bones of the skin
--------------------------------------------------------

function getRootsId skin =
(
	local rootstab,n,i,c,d ;
	rootstab = #() ;
	n = skinOps.GetNumberBones skin ;
	for i = 1 to n do
	(
		c= skinOps.GetBoneName skin i 1 ;
		replaceSpaces c ;
		d = execute ("$" + c) ;
		if (d.parent == undefined) then
			append rootstab i ;
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
