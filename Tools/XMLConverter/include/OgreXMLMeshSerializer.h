/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#ifndef __XMLMeshSerializer_H__
#define __XMLMeshSerializer_H__

#include "OgreXMLPrerequisites.h"
#include "OgreMaterial.h"
#include "OgreMesh.h"


namespace Ogre {

    /** Class for serializing a Mesh to/from XML.
    @remarks
        This class behaves the same way as MeshSerializer in the main project,
        but is here to allow conversions to / from XML. This class is 
        deliberately not included in the main project because <UL>
        <LI>Dependence on Xerces would unnecessarily bloat the main library</LI>
        <LI>Runtime use of XML is discouraged because of the parsing overhead</LI></UL>
        This class gives people the option of saving out a Mesh as XML for examination
        and possible editing. It can then be converted back to the native format
        for maximum runtime efficiency.
    */
    class XMLMeshSerializer
    {
    public:

        XMLMeshSerializer();
        virtual ~XMLMeshSerializer();
        /** Imports a Mesh from the given XML file.
        @param filename The name of the file to import, expected to be in XML format.
        @param pMesh The pre-created Mesh object to be populated.
        */
        void importMesh(const String& filename, Mesh* pMesh);

        /** Exports a mesh to the named XML file. */
        void exportMesh(const Mesh* pMesh, const String& filename, bool includeMaterials = false);

    protected:
        // State for export
        TiXmlDocument* mXMLDoc;
        // State for import
        Mesh* mpMesh;

        // Internal methods
        void writeMaterial(TiXmlElement* mMaterialsNode, const Material* m);
        void writeTextureLayer(TiXmlElement* mLayersNode, const Material::TextureLayer* pTex);
        void writeMesh(const Mesh* pMesh);
        void writeSubMesh(TiXmlElement* mSubmeshesNode, const SubMesh* s);
        void writeGeometry(TiXmlElement* mParentNode, const GeometryData* pGeom);
        void writeSkeletonLink(TiXmlElement* mMeshNode, const String& skelName);
        void writeBoneAssignment(TiXmlElement* mBoneAssignNode, const VertexBoneAssignment* assign);

        void readMaterials(TiXmlElement* mMaterialsNode);
        void readSubMeshes(TiXmlElement* mSubmeshesNode);
        void readGeometry(TiXmlElement* mGeometryNode, GeometryData* pGeom);
        void readSkeletonLink(TiXmlElement* mSkelNode);
        void readBoneAssignments(TiXmlElement* mBoneAssignmentsNode);
        void readBoneAssignments(TiXmlElement* mBoneAssignmentsNode, SubMesh* sm);

    };




}



#endif
