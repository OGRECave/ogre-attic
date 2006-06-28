#ifndef _VERTEX_H
#define _VERTEX_H

/***** structure for uvsets info *****/
	typedef struct uvsettag
	{
		short size;					//number of coordinates (between 1 and 3)
	} uvset;
	/***** structure for texture coordinates *****/
	typedef struct texcoordstag
	{
		float u, v, w;				//texture coordinates	
	} texcoords;

	/***** structure for vertex bone assignements *****/
	typedef struct vbatag
	{
		float weight;	//weight
		int jointIdx;	//index of associated joint
	} vba;

	/***** structure for vertex data *****/
	typedef struct vertextag
	{
		double x, y, z;						//vertex coordinates
		MVector n;							//vertex normal
		float r,g,b,a;						//vertex colour
		std::vector<texcoords> texcoords;	//vertex texture coordinates
		std::vector<vba> vbas;				//vertex bone assignements
		long index;							//vertex index in the maya mesh to which this vertex refers
	} vertex;

	/***** structure for vertex info *****/
	// used to hold indices to access MFnMesh data
	typedef struct vertexInfotag
	{
		int pointIdx;				//index to points list (position)
		int normalIdx;				//index to normals list
		float r,g,b,a;				//colour
		std::vector<float> u;		//u texture coordinates
		std::vector<float> v;		//v texture coordinates
		std::vector<float> vba;		//vertex bone assignements
		std::vector<int> jointIds;	//ids of joints affecting this vertex
		int next;					//index of next vertex with same position
	} vertexInfo;

	/***** structure for face info *****/
	typedef struct facetag
	{
		long v[3];		//vertex indices
	} face;

	/***** array of face infos *****/
	typedef std::vector<face> faceArray;

#endif