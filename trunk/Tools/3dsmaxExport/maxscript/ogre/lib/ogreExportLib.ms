
-- ogreExportLib.ms
--
-- Copyright © 2002 John Martin
--
-- Contains functions to export meshes and materials to the Ogre format.
--
-- Assumptions about the mesh being exported:
--   - must be of type editable mesh
--   - must be created with mapping coordinates or have a UVW mod applied
--   - each face must have a material
--
-- Assumptions about each material being exported:
--   - material is of the standard type (no multi's)
--   - must have a single texture map
--
-- The mesh export does a couple of things to make the transition
-- to a format that d3d likes. Firstly, to deal with the different
-- coordinate systems, the y and z elements of each vertice (and
-- normal) are swapped. Secondly, the 2nd and 3rd vertex index in
-- a face definition are swapped to make the order or vertex
-- specification clockwise (I think), which is how d3d decides which
-- side a face is facing.
--
-- The material export records the ambient, diffuse, specular and
-- emmissive (self illumination) properties of a material. Plus
-- the texture map filename of course.
--
-- Notes about the vertex filtering algortithm:
-- 
-- The algorithms below massage the mesh data to a format that
-- Ogre likes. This basically involves duplicating any vertex
-- that has different materials or different texture positions
-- for any of the faces it is part of. The process that is used
-- to do this is described below.
--
-- In this case, to solve this problem, we iterate over each vertex
-- in each face in the face list, moving the info into temporary vertex
-- and face buffers. If we come across a vertex that we have already
-- seen but who's face has a different material to the first time we
-- saw it, then we "duplicate" this vertex by giving it the next
-- index in the vertex buffer, and updating the face list to index
-- this new vertex, not the original one. Further info can be
-- found in embedded comments.
--
-- Notes about material exporting:
--
-- When a material record is created, a modified offset figure is
-- calculated that takes into account the tiling setting. This is
-- required because max keeps raw texture coordinates (ie coordinates
-- not transformed for tiling and offset). The tiling and
-- modified offset settings are used to calculate a modified
-- texture coordinate, suitable for the game engine.


-- structure of the vertex buffer elements
struct vertexInfo (vert, tvert, normal, matId)

-- structure of the faces array elements
struct faceInfo (face, matId)

-- structure to hold temporary lookup table
struct vertexLookup (tvertIndex, matId, changeTo)

-- structure to hold extended material info
struct extMaterial (name,
                    ambientColor,
                    diffuseColor,
                    specularColor,
                    selfIllumColor,
                    shininess,
                    diffuseMap,
                    uTiling,
                    vTiling,
                    uOffset,
                    vOffset)

-- structure to hold a submesh
struct submesh (vertbuf, faces, material)


-- global out stream and tab level to make life easier
global gExportStream = undefined
global gExportTabLevel = 0


--
-- Filtering functions.
--

-- Adds an new entry to the lookup table.
function addNewLookup lookup v tv matId = 
(
	-- create a new lookup rec
	vlook = vertexLookup tv matId 0
	
	-- add it as the first entry of an array in the lookup
	lookup[v] = #()
	append lookup[v] vlook
)

-- Adds another lookup rec.
function addExistingLookup lookup v tv matId newv = 
(
	-- add a lookup rec to the old vert index's array
	oldvlook = vertexLookup tv matId newv

	-- add it to the end of the array
	append lookup[v] oldvlook
	
	-- now add it to the new spot
	addNewLookup lookup newv tv matId
)

-- Determines whether a lookup already exists. Returns the changeTo
-- index if it's found, else it returns -1. This function now looks
-- to match both the tvert and the matId.
function findLookup lookup v tv matId = 
(
	for i = 1 to lookup[v].count do
	(
		curLookup = lookup[v][i]
		
		-- looking for a tvert *and* matId match
		if (curLookup.tvertIndex == tv and curLookup.matId == matId) do
		(
			return lookup[v][i].changeTo
		)
	)
	
	-1
)

-- Creates a ext mat list record.
function createExtMaterial mat =
(
	-- grab the tiling amount
	uTiling = mat.diffuseMap.coords.u_tiling
	vTiling = mat.diffuseMap.coords.v_tiling

	-- grab the offset amount
	uRawOffset = mat.diffuseMap.coords.u_offset
	vRawOffset = mat.diffuseMap.coords.v_offset

	-- The algorithm used here looks like this:
	--
	--   shift amount = (((tiling + 1) mod 2) div 2)
	--
	-- It is simply working out an amount that we shift all texture
	-- coords by so that the max centered texture tiling looks the
	-- same as the d3d left-anchored texture tiling.
	--
	-- Note: the raw offset given by max needs to be multiplied by
	-- the tiling factor for our purposes. Note also that the v coordinate
	-- is reversed in max (as compared to d3d) so we subtract the offset
	-- here instead of adding it.

	-- figure out the amount of shift that we need
	uOffset = ((mod (uTiling + 1) 2) / 2) + (uRawOffset * uTiling)
	vOffset = ((mod (vTiling + 1) 2) / 2) - (vRawOffset * vTiling)

	-- create the record
	extMat = extMaterial mat.name \
	                     mat.ambient \
	                     mat.diffuse \
	                     mat.specular \
	                     mat.selfIllumColor \
	                     mat.specularLevel \
	                     (filenameFromPath mat.diffuseMap.filename) \
	                     uTiling \
	                     vTiling \
	                     uOffset \
	                     vOffset

	extMat
)

-- Creates an extended material list.
function createExtMaterialList xmesh = 
(
	xlist = #()
	
	-- material could be a single or multi
	if (classOf xmesh.material == StandardMaterial) then
	(
		append xlist (createExtMaterial xmesh.material)
	)
	else if (classOf xmesh.material == MultiMaterial) then
	(
		for i = 1 to xmesh.material.count do
		(
			append xlist (createExtMaterial xmesh.material[i])
		)
	)
	else
	(
		-- who knows wtf it was
		MessageBox "Unknown material type in mesh " + xmesh.name + ". Materials may not be properly exported."
	)

	xlist;
)

-- Extract and organise the data that we want from a mesh.
function filterMeshData xmesh vertbuf faces = 
(
	-- the temp lookup table
	lookup = #()
	
	-- the next vert index is the one past the last vertex
	nextVertIndex = (getNumVerts xmesh) + 1

	-- If there is only a single material for a mesh, then the mat ID for each
	-- face can be a reference to something inside that single material (why
	-- this happens I'm not sure) so we need to change that to reference just
	-- the first material.
	collapseMatId = false;
	if (classOf xmesh.material == StandardMaterial) then
	(
		collapseMatId = true;
	)
	
	-- iterate through all the faces
	for i = 1 to xmesh.numFaces do
	(
		face = getFace xmesh i
		tvface = getTVFace xmesh i
	
		-- figure out what to put for the mat id
		faceMatId = 0;
		if (not collapseMatId) do
		(
			faceMatId = (getFaceMatId xmesh i) - 1
		)
	
		-- init the face info struct and set the mat id (minus one cause ogre is 0 based)
		faces[i] = faceInfo [0, 0, 0] faceMatId
		
		-- do each of the three vertices of the face
		for j = 1 to 3 do
		(
			v = face[j]
			tv = tvface[j]
			
			-- if it doesn't already exist, then add a new rec
			if (lookup[v] == undefined) then
			(
				-- add to the lookup
				addNewLookup lookup v tv faceMatId
				
				-- grab the pieces
				vert   = getVert xmesh v
				tvert  = getTVert xmesh tv
				normal = getNormal xmesh v
				
				-- add the vert to the vert buf
				vertbuf[v] = vertexInfo vert tvert normal faceMatId
				
				-- add to the faces array (minus one cause ogre is 0 based)
				faces[i].face[j] = v - 1
			)
			-- else, duplicate the vert
			else
			(
				newv = findLookup lookup v tv faceMatId
				
				-- if we can't find it, then we add it
				if (newv == -1) then
				(
					newv = nextVertIndex
					
					-- add the the lookup
					addExistingLookup lookup v tv faceMatId newv
					
					-- grab the pieces
					vert   = getVert xmesh v
					tvert  = getTVert xmesh tv
					normal = getNormal xmesh v
					
					-- add the vert to the vert buf
					vertbuf[newv] = vertexInfo vert tvert normal faceMatId

					-- move the new vert index along
					nextVertIndex += 1
				)
				else if (newv == 0) then
				(
					newv = v
				)

				-- add to the faces array (minus one cause ogre is 0 based)
				faces[i].face[j] = newv - 1
			)
		)
	)

	-- return the lookup
	lookup
)

-- Breaks a mesh into submeshes and prepares the data for Ogre.
function splitAndPrepareMesh vertbuf faces mats =
(
	submeshs = #()
	lookup = #()
	
	-- Note, this gets a little ugly here because of 1-based maxscript arrays.
	
	-- create the submesh structs and lookups
	for i = 1 to mats.count do
	(
		submeshs[i] = submesh #() #() mats[i]
		lookup[i] = #()
	)

	-- split the vertex data and create the lookups
	for i = 1 to vertbuf.count do
	(
		-- grab the current vertex and submesh
		v = vertbuf[i]
		m = v.matId + 1
			
		sm = submeshs[m]
		
		-- swap y and z vertex coords
		t = v.vert.y
		v.vert.y = v.vert.z
		v.vert.z = -t

		-- swap y and z normal coords
		t = v.normal.y
		v.normal.y = v.normal.z
		v.normal.z = -t
		
		-- fix the texture coords
		
		-- get the proper xmat (the + 1 cause maxscript arrays start at 1)
		xmat = mats[m]

		-- adjust the texture vertices taking into account their tiling and offset values
		v.tvert.x = (v.tvert.x * xmat.uTiling) - (xmat.uOffset)
		v.tvert.y = ((1 - v.tvert.y) * xmat.vTiling) - (xmat.vOffset)

		-- add the vertexInfo
		append sm.vertbuf v
				
		-- update the lookup
		lookup[m][i] = sm.vertbuf.count - 1
	)
	
	-- split the faces up
	for i = 1 to faces.count do
	(
		m = faces[i].matId + 1
				
		next = submeshs[m].faces.count + 1
		submeshs[m].faces[next] = [0, 0, 0]
		
		-- update the face indexes
		submeshs[m].faces[next].x = lookup[m][faces[i].face.x as integer + 1]
		submeshs[m].faces[next].y = lookup[m][faces[i].face.y as integer + 1]
		submeshs[m].faces[next].z = lookup[m][faces[i].face.z as integer + 1]
	)

	submeshs
)


--
-- High Level functions.
--

-- Export a mesh to the ogre file format.
function exportMesh meshName filename = 
(
	format "Exporting mesh: %\n" meshName
	
	-- grab the actual mesh from it's name
	xmesh = execute ("$" + meshName)

	-- check that it's a mesh
	if not classOf xmesh == Editable_Mesh then
	(
		messageBox "Selected object must be an Editable Mesh."
		return false;
	)

	-- the vertex buffer, to hold vert, tvert and normal data
	vertbuf = #()
	
	-- the new faces/index array
	faces = #()

	format "Filtering mesh data.\n"
		
	-- filter and organise our vertices
	filterMeshData xmesh vertbuf faces
	
	format "Creating extended material list.\n"
	
	-- some preprocessing to make a clean array of materials
	mats = createExtMaterialList xmesh

	format "Splitting and preparing mesh for Ogre export.\n"
	
	-- break into submeshs
	submeshs = splitAndPrepareMesh vertbuf faces mats

	format "Calling C++ extension to write the file.\n"
	
	-- export to ogre format
	ogreExportStaticMesh meshName submeshs filename
	
	format "Exported mesh to OGRE format.\n"
		
	true
)
