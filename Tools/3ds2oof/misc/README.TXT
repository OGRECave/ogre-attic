
Guide to the 3ds2oof Conversion Utility
---------------------------------------

Author: Steve Streeting (steve@stevestreeting.com)

Provided for use with the OGRE (Object-oriented Graphics Rendering
Engine). See http://www.stevestreeting.com/ogre/ for more info.


Introduction
------------
The 3ds2oof utility is designed to extract geometry information from  
a 3D Studio (.3ds) object file into a custom file format designed for 
loading into OGRE, which I have named OOF (OGRE Object Format). 

I chose the 3DS format since it's one of the most popular formats, and
there are conversions available to it from most other formats. I
chose to provide a conversion tool rather than directly loading 3DS
in OGRE, because I wanted a bit more control over the format loaded
at runtime - the 3DS format is fine in some ways, but for a realtime
engine there's plenty of extras that you typically want - like splitting
the geometry down into chunks suitable for submission to the pipeline,
extra pre-calculable information like bounding boxes, level-of-detail
reductions, etc.

This conversion tool is not all my own work - I used code written by
Matthew Fairfax for his OpenGL engine as a starting point for loading
the raw 3DS data (with some of my own adjustments). I'm releasing the
source for this tool incase my alterations are usefule to anyone else.

Usage
-----

3ds2oof filename [-vn] [-tc] [-tn] [-vc] [-oObjectName] [-lnnn] [-snnn] [-extExtension]
  [-matnames] [-usecolours]

The parameters are:

filename : The name of the 3DS file to read from.
-vn      : (optional) Excluded vertex normals from the output file
		   even if they are present in the 3DS file (e.g. if you are
		   doing your own lighting)
-tc		 : (optional) Excludes texture co-ordinates from the output file.
-tn      : (optional) Excludes texture names from the output model. 
           You may wish to use this if you don't intend to use the same
		   textures in OGRE as have been used in 3DS.
-vc      : (optional) Excludes vertex colours from the output model.
-oName   : Outputs just the named object, if there are multiple objects
		   in the original 3DS file.
-snnn    : Sets the percentage vertex limit above which a material group
           will use the shared vertex pool. (Default 100%). See below.
-extExtn : Sets the extension of texture maps to a different value. Useful if
	   your 3ds file uses one type of graphics file but you wish to use
	   a different one (I use this to convert BMP references to PNG or JPG).
	   You obviously need to convert your graphics files too.
-matnames: Causes the utility to prompt you for new names for the materials defined in
	   the 3DS file. You will be asked for a new name for each; you can enter nothing
	   to leave the name unchanged.
-usecolours: Causes 3ds2oof to use the material colours stored in the 3DS file to tint the
             various parts of the model. Not normally recommended, so turned off by default,
	     since most 3DS models designed for texturing use colours only to indicate groups
             of items (so they tend to be garish). If your model uses material colours 
             properly, then use this option.
		   
Shared and dedicated vertex buffers
-----------------------------------
When rendering parts of a model which use different materials, separate
rendering calls have to be made with each material setting. When this 
call is made, a face list is passed which references vertices in a list.
This vertex list can either just include the vertices required for this
set of faces (dedicated), or may share a buffer of vertices with other
parts of the model which use different materials. 

If the face list for one material is very small, using a small percentage
of the shared vertex list, it is wasteful to pass all the models vertices
just to render a small set of faces. Therefore 3ds2oof has a parameter
to allow you to specify how much of the overall vertex list a material
group must use before sharing vertices; it defaults to -s100 i.e. share 
vertices only where the material uses 100% of the vertices in the model.

You can save memory (for multi-material objects) by reducing this number 
at the expense of slightly less efficient rendering calls. Also if you 
decide to use vertex buffers for rendering an object, you could set
-s0 to use a shared buffer for everything, allowing it to be put in
the hardware's vertex buffer for efficient processing (NB using 
vertex buffers in OGRE is not yet supported but is upcoming).

File Format
-----------
Please see OgreOofFile.h for details.


