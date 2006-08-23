/*
** This source file is part of OGRE (Object-oriented Graphics Rendering Engine)
** For the latest info, see http://www.ogre3d.org/
**
** OGRE Copyright goes for Ogre Team
** Hybrid Portal/BSP Scene Manager Copyright (c) 2006 Wael El Oraiby
** 
** This program is free software; you can redistribute it and/or modify it under
** the terms of the GNU Lesser General Public License as published by the Free Software
** Foundation; either version 2 of the License, or (at your option) any later
** version.
** 
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
** 
** You should have received a copy of the GNU Lesser General Public License along with
** this program; if not, write to the Free Software Foundation, Inc., 59 Temple
** Place - Suite 330, Boston, MA 02111-1307, USA, or go to
** http://www.gnu.org/copyleft/lesser.txt.
*/
#include "Ogre.h"
#include "OgreStringConverter.h"

#include "HpBspDotSceneLoader.h"

#include "tinyxml.h"

using namespace Ogre;
using namespace std;

String mtlCells[4] = {
	String("RedCell"),
	String("GreenCell"), 
	String("BlueCell"),
	String("PinkCell")
};

HpBspDotSceneLoader HpBspDotSceneLoader::singleton;

//-----------------------------------------------------------------------------
HpBspDotSceneLoader* HpBspDotSceneLoader::getSingleton()
{
//	if(singleton == NULL)
//		singleton = new HpBspDotSceneLoader();
	return &singleton;
}

//-----------------------------------------------------------------------------
HpBspDotSceneLoader::HpBspDotSceneLoader()
{
	mNumPortals = 0;

	mSM = NULL;
	mParentNode = NULL;
}

//-----------------------------------------------------------------------------
HpBspDotSceneLoader::~HpBspDotSceneLoader()
{
//	singleton = NULL;
}

//-----------------------------------------------------------------------------
void HpBspDotSceneLoader::resetInternals()
{
	mNumPortals = 0;

	mSM = NULL;

	mBspCSG = BspObject();
	mBspOccluders = BspObject();
	mBspPortals = BspObject();		

	mParentNode = NULL;

	mOrigOccluders.resize(0);
	mOrigPortals.resize(0);
	mCells.resize(0);
	mPortals.resize(0);
}

//-----------------------------------------------------------------------------
void getMeshInformation( MeshPtr& mesh,
						std::vector<Ogre::Vector3> &vertices,
						std::vector<unsigned long> &indices,
						const Ogre::Matrix4 &transform)
{
	bool added_shared = false;
	size_t current_offset = 0;
	size_t shared_offset = 0;
	size_t next_offset = 0;
	size_t index_offset = 0;
	size_t index_count = 0;
	size_t vertex_count = 0;

	// Calculate how many vertices and indices we're going to need
	for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
	{
		Ogre::SubMesh* submesh = mesh->getSubMesh( i );

		// We only need to add the shared vertices once
		if(submesh->useSharedVertices)
		{
			if( !added_shared )
			{
				vertex_count += mesh->sharedVertexData->vertexCount;
				added_shared = true;
			}
		}
		else
		{
			vertex_count += submesh->vertexData->vertexCount;
		}

		// Add the indices
		index_count += submesh->indexData->indexCount;
	}


	// Allocate space for the vertices and indices
	vertices.resize(0);
	indices.resize(0);

	added_shared = false;

	// Run through the submeshes again, adding the data into the arrays
	for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
	{
		Ogre::SubMesh* submesh = mesh->getSubMesh(i);

		Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

		if((!submesh->useSharedVertices)||(submesh->useSharedVertices && !added_shared))
		{
			if(submesh->useSharedVertices)
			{
				added_shared = true;
				shared_offset = current_offset;
			}

			const Ogre::VertexElement* posElem =
				vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

			Ogre::HardwareVertexBufferSharedPtr vbuf =
				vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

			unsigned char* vertex =
				static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

			// There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
			//  as second argument. So make it float, to avoid trouble when Ogre::Real will
			//  be comiled/typedefed as double:
			//      Ogre::Real* pReal;
			float* pReal;

			for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
			{
				posElem->baseVertexPointerToElement(vertex, &pReal);

				Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);

				// transform vertices as well
				vertices.push_back(transform * pt);
			}

			vbuf->unlock();
			next_offset += vertex_data->vertexCount;
		}


		Ogre::IndexData* index_data = submesh->indexData;
		size_t numTris = index_data->indexCount / 3;
		Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

		bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

		unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
		unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);


		size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;

		if ( use32bitindexes )
		{
			for ( size_t k = 0; k < numTris*3; ++k)
			{
				indices.push_back(pLong[k] + static_cast<unsigned long>(offset));
			}
		}
		else
		{
			for ( size_t k = 0; k < numTris*3; ++k)
			{
				indices.push_back(static_cast<unsigned long>(pShort[k]) +
					static_cast<unsigned long>(offset));
			}
		}

		ibuf->unlock();
		current_offset = next_offset;
	}
}

//----------------------------------------------------------------------------
static Light* LoadLight( TiXmlElement *XMLLight, SceneManager *mSceneMgr )
{
	TiXmlElement *XMLDiffuse, *XMLSpecular, *XMLAttentuation, *XMLPosition;

	// Create a light (point | directional | spot | radPoint)
	Light* l = mSceneMgr->createLight( XMLLight->Attribute("name") );
	if( !XMLLight->Attribute("type") || String(XMLLight->Attribute("type")) == "point" )
		l->setType( Light::LT_POINT );
	else if( String(XMLLight->Attribute("type")) == "directional")
		l->setType( Light::LT_DIRECTIONAL );
	else if( String(XMLLight->Attribute("type")) == "spot")
		l->setType( Light::LT_SPOTLIGHT );
	else if( String(XMLLight->Attribute("type")) == "radPoint")
		l->setType( Light::LT_POINT );

	XMLDiffuse = XMLLight->FirstChildElement("colourDiffuse");
	if( XMLDiffuse ){
		ColourValue Diffuse;
		Diffuse.r = Ogre::StringConverter::parseReal( XMLDiffuse->Attribute("r") );
		Diffuse.g = Ogre::StringConverter::parseReal( XMLDiffuse->Attribute("g") );
		Diffuse.b = Ogre::StringConverter::parseReal( XMLDiffuse->Attribute("b") );
		Diffuse.a = 1;
		l->setDiffuseColour(Diffuse);
	}
	XMLSpecular = XMLLight->FirstChildElement("colourSpecular");
	if( XMLSpecular ){
		ColourValue Specular;
		Specular.r = Ogre::StringConverter::parseReal( XMLSpecular->Attribute("r") );
		Specular.g = Ogre::StringConverter::parseReal( XMLSpecular->Attribute("g") );
		Specular.b = Ogre::StringConverter::parseReal( XMLSpecular->Attribute("b") );
		Specular.a = 1;
		l->setSpecularColour(Specular);
	}

	XMLAttentuation = XMLLight->FirstChildElement("lightAttenuation");
	if( XMLAttentuation )
	{
		//get defaults incase not all values specified
		Real range, constant, linear, quadratic;
		range = l->getAttenuationRange();
		constant = l->getAttenuationConstant();
		linear = l->getAttenuationLinear();
		quadratic = l->getAttenuationQuadric();

		if( XMLAttentuation->Attribute("range") )
			range = StringConverter::parseReal( XMLAttentuation->Attribute("range") );
		if( XMLAttentuation->Attribute("constant") )
			constant = StringConverter::parseReal( XMLAttentuation->Attribute("constant") );
		if( XMLAttentuation->Attribute("linear") )
			linear = StringConverter::parseReal( XMLAttentuation->Attribute("linear") );
		if( XMLAttentuation->Attribute("quadratic") )
			quadratic = StringConverter::parseReal( XMLAttentuation->Attribute("quadratic") );

		l->setAttenuation( range, constant, linear, quadratic );
	}

	XMLPosition = XMLLight->FirstChildElement("position");
	if( XMLPosition ) {
		Vector3 p = Vector3(0,0,0);
		if( XMLPosition->Attribute("x") )
			p.x = StringConverter::parseReal( XMLPosition->Attribute("x") );
		if( XMLPosition->Attribute("y") )
			p.y = StringConverter::parseReal( XMLPosition->Attribute("y") );
		if( XMLPosition->Attribute("z") )
			p.z = StringConverter::parseReal( XMLPosition->Attribute("z") );

		l->setPosition( p );
	}

	//castShadows      (true | false) "true"
	l->setCastShadows( true );
	if( XMLLight->Attribute("visible") )
		if( String(XMLLight->Attribute("visible")) == "false" )
			l->setCastShadows( false );

	//visible         (true | false) "true"      
	l->setVisible( true );
	if( XMLLight->Attribute("visible") )
		if( String(XMLLight->Attribute("visible")) == "false" )
			l->setVisible( false );

	return l;
}

//-----------------------------------------------------------------------------
void HpBspDotSceneLoader::_load(HybridPortalBspSceneManager *sm,
								const String &fileName,
								const String &groupName)
{
	// set up shared object values
	TiXmlDocument  *XMLDoc;
	TiXmlElement   *XMLRoot, *XMLNodes;

	// destroy previously created objects
	resetInternals();

	mSM = sm;

	try
	{
		DataStreamPtr pStream = ResourceGroupManager::getSingleton().
			openResource( fileName, groupName );

		String data	= pStream->getAsString();
		// Open the .scene File
		XMLDoc = new TiXmlDocument();
		
		LogManager::getSingleton().logMessage("[dotSceneLoader] starting parsing...");
		
		XMLDoc->Parse( pStream->getAsString().c_str() );

		pStream->close();
		pStream.setNull();

		if( XMLDoc->Error() )
		{
			//We'll just log, and continue on gracefully
			LogManager::getSingleton().logMessage("[dotSceneLoader] The TiXmlDocument reported an error");
			delete XMLDoc;

			throw Ogre::Exception(0, "The TiXmlDocument reported an error", "_load", __FILE__, __LINE__);
		}
		LogManager::getSingleton().logMessage("[dotSceneLoader] document parsed well");
	}
	catch(...)
	{
		//We'll just log, and continue on gracefully
		LogManager::getSingleton().logMessage("[dotSceneLoader] Error creating TiXmlDocument");
		delete XMLDoc;
		
		throw Ogre::Exception(0, "Error creating TiXmlDoc", "_load", __FILE__, __LINE__);
	}

	// Validate the File
	XMLRoot = XMLDoc->RootElement();
	if( String( XMLRoot->Value()) != "scene" ) {
		LogManager::getSingleton().logMessage( "[dotSceneLoader]Error: Invalid .scene File. Missing <scene>" );
		delete XMLDoc;      
		
		throw Ogre::Exception(0, "Invalid .scene File", "_load", __FILE__, __LINE__);
	}

	// create the parent node
	mParentNode = mSM->getMoversRootSceneNode();
	LogManager::getSingleton().logMessage( "[dotSceneLoader] parent node is: " + mParentNode->getName());

	XMLNodes = XMLRoot->FirstChildElement( "nodes" );

	// Read in the scene nodes
	if( XMLNodes )
	{
		processNode(XMLNodes->FirstChildElement( "node" ), mParentNode);
	}

	// this need to be worked out from within the scene manager, apparently it still
	// goes to check all child nodes, we don't need to do this test: Ask tuan or sinbad
	// about making a flag in the scene manager that allows us to either chose to cascade
	// or not if a node is visible.
	mParentNode->setVisible(true);
	
	// build the scene
	buildScene();

	// set the bsp object for scene
	sm->_setBspObject(mBspCSG);

	// setup portals
	size_t i;
	for( i = 0; i < mPortals.size(); i++ )
		sm->_setPortalCells(static_cast<int>(i), mPortals[i].cellsId[0], mPortals[i].cellsId[1]);

	// setup cells
	for( i = 0; i < mCells.size(); i++ )
		sm->_setCellPortals(static_cast<int>(i), mCells[i].portals);

	// add portals, cells and occluders
/*
	{
		size_t i;
		for(i = 0; i < mPortals.size(); i++)
			mSM->addPortalSceneNode(mOrigPortals[i]);

		for(i = 0; i < mCells.size(); i++)
			mSM->addCellSceneNode(mCells[i].cell);

		for(i = 0; i < mOrigOccluders.size(); i++)
			mSM->addOccluderSceneNode(mOrigOccluders[i]);
	}
*/
	// Close the XML File
	delete XMLDoc;
}

//-----------------------------------------------------------------------------
void HpBspDotSceneLoader::processNode(TiXmlElement *XMLNode, SceneNode *pAttach)
{
	TiXmlElement *XMLPosition, *XMLRotation, *XMLScale,  *XMLEntity, *XMLBillboardSet,  *XMLLight/*, *XMLUserData*/;

	while( XMLNode )
	{
		// Process the current node
		// Grab the name of the node
		// First create the new scene node
		String NodeName = XMLNode->Attribute("name");
		SceneNode* NewNode = NULL;

		if ( Ogre::StringUtil::startsWith(NodeName, PORTAL_NAME_PREFIX, false) )
		{
			NewNode = mSM->_createPortalSceneNode(mNumPortals, NodeName);
			mNumPortals++;
			LogManager::getSingleton().logMessage("[[[dotSceneLoader]]] portal " + NodeName + " is created at slot" + Ogre::StringConverter::toString(mNumPortals));
		} else if ( Ogre::StringUtil::startsWith(NodeName, OCCLUDER_NAME_PREFIX, false) )
		{
			NewNode = mSM->_createOccluderSceneNode(NodeName);
			LogManager::getSingleton().logMessage("[[[dotSceneLoader]]] occluder " + NodeName);
		} else
		{
			NewNode = pAttach->createChildSceneNode(NodeName);
		}

		// SceneNode* NewNode = pAttach->createChildSceneNode( NodeName );
		Vector3 TempVec;
		String TempValue;

		// Now position it...
		XMLPosition = XMLNode->FirstChildElement("position");
		if( XMLPosition ){
			TempValue = XMLPosition->Attribute("x");
			TempVec.x = StringConverter::parseReal(TempValue);
			TempValue = XMLPosition->Attribute("y");
			TempVec.y = StringConverter::parseReal(TempValue);
			TempValue = XMLPosition->Attribute("z");
			TempVec.z = StringConverter::parseReal(TempValue);
			NewNode->setPosition( TempVec );
		}

		// Rotate it...
		XMLRotation = XMLNode->FirstChildElement("rotation");
		if( XMLRotation ){
			Quaternion TempQuat;
			TempValue = XMLRotation->Attribute("qx");
			TempQuat.x = StringConverter::parseReal(TempValue);
			TempValue = XMLRotation->Attribute("qy");
			TempQuat.y = StringConverter::parseReal(TempValue);
			TempValue = XMLRotation->Attribute("qz");
			TempQuat.z = StringConverter::parseReal(TempValue);
			TempValue = XMLRotation->Attribute("qw");
			TempQuat.w = StringConverter::parseReal(TempValue);
			NewNode->setOrientation( TempQuat );
		}

		// Scale it.
		XMLScale = XMLNode->FirstChildElement("scale");
		if( XMLScale ){
			TempValue = XMLScale->Attribute("x");
			TempVec.x = StringConverter::parseReal(TempValue);
			TempValue = XMLScale->Attribute("y");
			TempVec.y = StringConverter::parseReal(TempValue);
			TempValue = XMLScale->Attribute("z");
			TempVec.z = StringConverter::parseReal(TempValue);
			NewNode->setScale( TempVec );
		}

		XMLLight = XMLNode->FirstChildElement( "light" );
		if( XMLLight )
			NewNode->attachObject( LoadLight( XMLLight, mSM ) );

		// Check for an Entity
		XMLEntity = XMLNode->FirstChildElement("entity");
		if( XMLEntity )
		{
			String EntityName, EntityMeshFilename;
			EntityName = XMLEntity->Attribute( "name" );
			EntityMeshFilename = XMLEntity->Attribute( "meshFile" );

			// see what kind of prefix the name is (determine whether it's a portal or an occluder)
			String upEntityName = EntityName;
			Ogre::StringUtil::toUpperCase(upEntityName);
			

			if ( !memcmp(upEntityName.c_str(), PORTAL_NAME_PREFIX, strlen(PORTAL_NAME_PREFIX)) )
			{
				LogManager::getSingleton().logMessage("[dotSceneLoader] portal " + EntityName + " is added");
				mOrigPortals.push_back(NewNode);
				
			} else
			if ( !memcmp(upEntityName.c_str(), OCCLUDER_NAME_PREFIX, strlen(OCCLUDER_NAME_PREFIX)) )
			{
				LogManager::getSingleton().logMessage("[dotSceneLoader] occluder " + EntityName + " is added");
				mOrigOccluders.push_back(NewNode);
			}

			// Create entity
			Entity* NewEntity = mSM->createEntity(EntityName, EntityMeshFilename);
			NewNode->attachObject( NewEntity );
		}

		XMLBillboardSet = XMLNode->FirstChildElement( "billboardSet" );
		if( XMLBillboardSet )
		{
			BillboardSet* bSet = mSM->createBillboardSet( NewNode->getName() );

			BillboardType Type;
			TempValue = XMLBillboardSet->Attribute( "type" );
			if( TempValue == "orientedCommon" )
				Type = BBT_ORIENTED_COMMON;
			else if( TempValue == "orientedSelf" )
				Type = BBT_ORIENTED_SELF;
			else Type = BBT_POINT;

			BillboardOrigin Origin;
			TempValue = XMLBillboardSet->Attribute( "type" );
			if( TempValue == "bottom_left" )
				Origin = BBO_BOTTOM_LEFT;
			else if( TempValue == "bottom_center" )
				Origin = BBO_BOTTOM_CENTER;
			else if( TempValue == "bottomRight"  )
				Origin = BBO_BOTTOM_RIGHT;
			else if( TempValue == "left" )
				Origin = BBO_CENTER_LEFT;
			else if( TempValue == "right" )
				Origin = BBO_CENTER_RIGHT;
			else if( TempValue == "topLeft" )
				Origin = BBO_TOP_LEFT;
			else if( TempValue == "topCenter" )
				Origin = BBO_TOP_CENTER;
			else if( TempValue == "topRight" )
				Origin = BBO_TOP_RIGHT;
			else
				Origin = BBO_CENTER;

			bSet->setBillboardType( Type );
			bSet->setBillboardOrigin( Origin );


			TempValue = XMLBillboardSet->Attribute( "name" );
			bSet->setMaterialName( TempValue );

			int width, height;
			width = (int) StringConverter::parseReal( XMLBillboardSet->Attribute( "width" ) );
			height = (int) StringConverter::parseReal( XMLBillboardSet->Attribute( "height" ) );
			bSet->setDefaultDimensions( width, height );
			bSet->setVisible( true );
			NewNode->attachObject( bSet );

			TiXmlElement *XMLBillboard;

			XMLBillboard = XMLBillboardSet->FirstChildElement( "billboard" );

			while( XMLBillboard )
			{
				Billboard *b;
				// TempValue;
				TempVec = Vector3( 0, 0, 0 );
				ColourValue TempColour(1,1,1,1);

				XMLPosition = XMLBillboard->FirstChildElement( "position" );
				if( XMLPosition ){
					TempValue = XMLPosition->Attribute("x");
					TempVec.x = StringConverter::parseReal(TempValue);
					TempValue = XMLPosition->Attribute("y");
					TempVec.y = StringConverter::parseReal(TempValue);
					TempValue = XMLPosition->Attribute("z");
					TempVec.z = StringConverter::parseReal(TempValue);
				}

				TiXmlElement* XMLColour = XMLBillboard->FirstChildElement( "colourDiffuse" );
				if( XMLColour ){
					TempValue = XMLColour->Attribute("r");
					TempColour.r = StringConverter::parseReal(TempValue);
					TempValue = XMLColour->Attribute("g");
					TempColour.g = StringConverter::parseReal(TempValue);
					TempValue = XMLColour->Attribute("b");
					TempColour.b = StringConverter::parseReal(TempValue);
				}

				b = bSet->createBillboard( TempVec, TempColour);

				XMLBillboard = XMLBillboard->NextSiblingElement( "billboard" );
			}
		}

		/* NO USER DATA FOR THE MOMENT
		XMLUserData = XMLNode->FirstChildElement( "userData" );
		if ( XMLUserData )
		{
			TiXmlElement *XMLProperty;
			XMLProperty = XMLUserData->FirstChildElement("property");
			while ( XMLProperty )
			{
				String first = NewNode->getName();
				String second = XMLProperty->Attribute("name");
				String third = XMLProperty->Attribute("data");
				String type = XMLProperty->Attribute("type");
				nodeProperty newProp(first,second,third,type);
				mNodeProperties.push_back(newProp);
				XMLProperty = XMLProperty->NextSiblingElement("property");
			}   
		}
		*/
		TiXmlElement * ChildXMLNode;
		ChildXMLNode = XMLNode->FirstChildElement( "node" );
		if(ChildXMLNode)
			processNode(ChildXMLNode, NewNode);	// recurse to do all my children

		XMLNode = XMLNode->NextSiblingElement( "node" ); // process my next sibling
	}
}

//-----------------------------------------------------------------------------
void HpBspDotSceneLoader::buildNodeBspPolys(SceneNode *node, vector<BspPolygon> &polys, int cellId, int portalId)
{
	vector<Vector3> verts;
	vector<unsigned long> idx;

	for ( int i = 0; i < node->numAttachedObjects(); i++ )
	{
		MeshPtr mesh = (static_cast<Entity*>(node->getAttachedObject(i)))->getMesh();
		
		verts.resize(0);
		idx.resize(0);

		getMeshInformation(mesh, verts, idx, node->_getFullTransform());

		for ( size_t j = 0; j < idx.size() / 3; j++ )
		{
			BspPolygon		poly;

			poly.cellId		= cellId;
			poly.portalId	= portalId;
		
			Vector3 v0, v1, v2;
			v0 = verts[idx[j * 3]];
			v1 = verts[idx[j * 3 + 1]];
			v2 = verts[idx[j * 3 + 2]];
			poly.verts.push_back(v0);
			poly.verts.push_back(v1);
			poly.verts.push_back(v2);

			poly.p = Plane(v0, v1, v2);

			polys.push_back(poly);
		}
	}
}

//-----------------------------------------------------------------------------
void HpBspDotSceneLoader::buildScene()
{
	size_t i;

	// update node transform and all its subnodes
	mParentNode->_update(true, true);

	vector<BspPolygon> occPolys;
	vector<BspPolygon> portalPolys;
	// build occluders
	LogManager::getSingleton().logMessage("[dotSceneLoader] building occluders polygons...");
	for ( i = 0; i < mOrigOccluders.size(); i++ )
		buildNodeBspPolys(mOrigOccluders[i], occPolys, 1, -1);
	LogManager::getSingleton().logMessage("[dotSceneLoader] occluders polygons are built...");

	// build portals
	LogManager::getSingleton().logMessage("[dotSceneLoader] building portals polygons...");
	for ( i = 0; i < mOrigPortals.size(); i++ )
		buildNodeBspPolys(mOrigPortals[i], portalPolys, 1, static_cast<int>(i));
	LogManager::getSingleton().logMessage("[dotSceneLoader] portals polygons are built...");

	// build occluders BSP
	LogManager::getSingleton().logMessage("[dotSceneLoader] building occluder BSP...");
	mBspOccluders = BspObject(occPolys);
	LogManager::getSingleton().logMessage("[dotSceneLoader] occluder BSP is built...");

	// build portals BSP
	LogManager::getSingleton().logMessage("[dotSceneLoader] building portals BSP...");
	mBspPortals = BspObject(portalPolys);
	LogManager::getSingleton().logMessage("[dotSceneLoader] portals BSP is built...");

	// clip polygons now
	vector<BspPolygon> portalsClippedPolys;
	LogManager::getSingleton().logMessage("[dotSceneLoader] clipping portals...");
	mBspOccluders.clipPolygons(portalPolys, portalsClippedPolys);
	LogManager::getSingleton().logMessage("[dotSceneLoader] clipping portals is done: " + StringConverter::toString(portalsClippedPolys.size()));

	vector<BspPolygon> occludersClippedPolys;
	LogManager::getSingleton().logMessage("[dotSceneLoader] clipping occluders...");
	mBspPortals.clipPolygons(occPolys, occludersClippedPolys);
	LogManager::getSingleton().logMessage("[dotSceneLoader] clipping occluders is done: " + StringConverter::toString(occludersClippedPolys.size()));

	// merge all polygons together now
	vector<BspPolygon> allPolys;
	allPolys = occludersClippedPolys;
	for ( i = 0; i < portalsClippedPolys.size(); i++ )
		allPolys.push_back(portalsClippedPolys[i]);

	// and now decompose the polygons into cells
	Decomposer decomp;

	int numCells = decomp.decompose(allPolys);

	LogManager::getSingleton().logMessage("[dotSceneLoader] number of decomposed regions: " + StringConverter::toString(numCells));

	// build the Bsp for the CSG result
	LogManager::getSingleton().logMessage("[dotSceneLoader] building CSG BSP...");
	mBspCSG = BspObject(allPolys);
	LogManager::getSingleton().logMessage("[dotSceneLoader] CSG BSP done: " + StringConverter::toString(allPolys.size()));

	// extract portal/cell connectivity
	extractConnectivity(allPolys, static_cast<int>(mOrigPortals.size()), numCells);

	// and create the cells
	for ( int c = 0; c < numCells; c++ )
	{
		buildMesh("Cell_" + StringConverter::toString(c), allPolys, c, -1);
		Entity* e = mSM->createEntity("Cell_" + StringConverter::toString(c), "Cell_" + StringConverter::toString(c));
		e->setVisible(true);

		mCells[c].cell = mSM->_createCellSceneNode(c, "Cell_" + StringConverter::toString(c));
		mCells[c].cell->attachObject(e);
	}
}

//-----------------------------------------------------------------------------
void HpBspDotSceneLoader::extractConnectivity(const vector<BspPolygon> &polys, int numPortals, int numCells)
{
	size_t i;
	int j;

	for (j = 0; j < numPortals; j++)
	{
		Portal p;
		p.cellsId[0] = p.cellsId[1] = -1;
		mPortals.push_back(p);
	}

	for (j = 0; j < numCells; j++)
	{
		Cell c;
		mCells.push_back(c);
	}

	for (i = 0; i < polys.size(); i++)
	{
		if (polys[i].portalId >= 0)
		{
			// add cells to portals
			if (mPortals[polys[i].portalId].cellsId[0] == -1)
				mPortals[polys[i].portalId].cellsId[0] = polys[i].cellId;
			else if (mPortals[polys[i].portalId].cellsId[0] != polys[i].cellId)
					if (mPortals[polys[i].portalId].cellsId[1] == -1)
						mPortals[polys[i].portalId].cellsId[1] = polys[i].cellId;
					else if (mPortals[polys[i].portalId].cellsId[1] != polys[i].cellId)
						throw "Fatal Error: Mesh is screwed up for portals";

			// and add portals to cell
			size_t k = 0;
			for (; k < mCells[polys[i].cellId].portals.size(); k++)
			{
				if (mCells[polys[i].cellId].portals[k] == polys[i].portalId)
					break;
			}

			if (k == mCells[polys[i].cellId].portals.size())
				mCells[polys[i].cellId].portals.push_back(polys[i].portalId);

		}
	}

	// output to log
	for (i = 0; i < mPortals.size(); i++)
		LogManager::getSingleton().logMessage("[extractConnectivity] portal " + 
			StringConverter::toString(i) + " connects cell " +
			StringConverter::toString(mPortals[i].cellsId[0]) + " and cell " +
			StringConverter::toString(mPortals[i].cellsId[1]));

	for (i = 0; i < mCells.size(); i++)
	{
		String str = "[extractConnectivity] Cell " + StringConverter::toString(i) + " has portals:";

		for (size_t j = 0; j < mCells[i].portals.size(); j++)
			str += " " + StringConverter::toString(mCells[i].portals[j]);

		LogManager::getSingleton().logMessage(str);
	}
}

//-----------------------------------------------------------------------------
#define POSITIVE_INFINITY (999999.0f)
#define NEGATIVE_INFINITY (- POSITIVE_INFINITY)

void HpBspDotSceneLoader::buildMesh(const String &meshName, const std::vector<BspPolygon> &inPolys, int cellId, int portalId)
{
	size_t i;
	// first create a mesh using the mesh manager
	MeshPtr	msh = MeshManager::getSingleton().createManual(meshName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	// create a submesh for this mesh
	SubMesh *sm = msh->createSubMesh(meshName);

	// create a copy of the polygons we are going to use
	vector<Vector3> verts;
	vector<Vector3> normals;
	vector<unsigned long> idx;

	AxisAlignedBox aabb(POSITIVE_INFINITY, POSITIVE_INFINITY, POSITIVE_INFINITY,
		NEGATIVE_INFINITY, NEGATIVE_INFINITY, NEGATIVE_INFINITY);


	for ( i = 0; i < inPolys.size(); i++ )
	{
		if ( inPolys[i].cellId == cellId && inPolys[i].portalId == portalId )
		{
			unsigned long start_idx = static_cast<unsigned long>(verts.size());

			verts.push_back(inPolys[i].verts[0]);
			normals.push_back(inPolys[i].p.normal);
			aabb.merge(inPolys[i].verts[0]);

			verts.push_back(inPolys[i].verts[1]);
			normals.push_back(inPolys[i].p.normal);
			aabb.merge(inPolys[i].verts[1]);


			for ( size_t j = 2; j < inPolys[i].verts.size(); j++ )
			{
				idx.push_back(start_idx);
				idx.push_back(start_idx + j - 1);
				verts.push_back(inPolys[i].verts[j]);
				normals.push_back(inPolys[i].p.normal);
				aabb.merge(inPolys[i].verts[j]);
				idx.push_back(start_idx + j);
			}
		}
	}

	sm->useSharedVertices = false; 		// no shared vertices
	sm->vertexData = new VertexData();	// create the vertex data holder
	sm->vertexData->vertexStart = 0;	// vertex start
	sm->vertexData->vertexCount = verts.size();	// vertex count

	// setup vertex declaration now
	VertexDeclaration *dcl = sm->vertexData->vertexDeclaration;

	size_t offset = 0;
	dcl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
	offset += VertexElement::getTypeSize(VET_FLOAT3);
	dcl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
	offset += VertexElement::getTypeSize(VET_FLOAT3);
	dcl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES);
	offset += VertexElement::getTypeSize(VET_FLOAT2);

	// create the vertex buffer
	HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
		offset,
		sm->vertexData->vertexCount,
		HardwareBuffer::Usage::HBU_STATIC_WRITE_ONLY);
	float* pReal = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));

	for (i = 0; i < verts.size(); ++i)
	{
		// position
		*pReal++ = verts[i].x;
		*pReal++ = verts[i].y;
		*pReal++ = verts[i].z;
		// normal
		*pReal++ = normals[i].x;
		*pReal++ = normals[i].y;
		*pReal++ = normals[i].z;
		// uv
		*pReal++ = 0;
		*pReal++ = 0;
	}

	vbuf->unlock();
	sm->vertexData->vertexBufferBinding->setBinding(0, vbuf);
	sm->indexData->indexCount = idx.size();
	sm->indexData->indexBuffer = HardwareBufferManager::getSingleton()
		.createIndexBuffer(HardwareIndexBuffer::IT_32BIT, sm->indexData->indexCount,
		HardwareBuffer::HBU_STATIC_WRITE_ONLY);
	uint32* pI = static_cast<uint32*>(
		sm->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
	for (i = 0; i < idx.size(); ++i)
	{
		*pI++ = idx[i];
	}

	sm->indexData->indexBuffer->unlock();

	// never forget the bounding boxes
	msh->_setBoundingSphereRadius(10);
	msh->_setBounds(aabb);
	sm->setMaterialName(mtlCells[cellId & 3]);
	msh->load();
}