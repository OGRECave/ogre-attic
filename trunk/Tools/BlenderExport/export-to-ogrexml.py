##
# OGRE XML format exporter
#   This script currently only works with Blender Publisher 2.25
#
# Things I want to do:
#   - Interact directly with the xmlconverter
#   - GUI, gtk maybe, that'll make the progress bars a lot more helpful
#   - Colors
#
# Original Author:  Thomas "temas" Muldowney
##
import Blender
import sys
from xml.dom.minidom import Document

#######
# rescale factor, set to [1,1,1] for no rescaling
#######
rescale = [50, 50, 50]

#######
# not 0 to export UVs
#######
exportUVs = 0

######
# NOTHING TO EDIT PAST HERE
######

pbar = Blender.Window.draw_progressbar
if Blender.Window.__dict__.has_key("drawProgressBar"):
    pbar = Blender.Window.drawProgressBar

## Add a face to the xml node
def addFace(face, elems, verts, indices = [0,1,2]):
    face_elem = xmldoc.createElement("face")
    elems[0].appendChild(face_elem)
    curvert = 1
    for x in indices:
        vert = face[x]
        vertid = 0
        if verts.has_key(vert):
            vertid = verts[vert]
        else:
            vertex_elem = xmldoc.createElement("vertex")
            elems[1].appendChild(vertex_elem)
            tmp_elem = xmldoc.createElement("position")
            vertex_elem.appendChild(tmp_elem)
            xyz = multVertByMat4(vert, obj_mat)
            tmp_elem.setAttribute("x", str(xyz[0]))
            tmp_elem.setAttribute("y", str(xyz[1]))
            tmp_elem.setAttribute("z", str(xyz[2]))
            # Save the normals
            normal = vert.no
            vertex_elem = xmldoc.createElement("vertex")
            elems[2].appendChild(vertex_elem)
            tmp_elem = xmldoc.createElement("normal")
            vertex_elem.appendChild(tmp_elem)
            tmp_elem.setAttribute("x", str(normal[0]))
            tmp_elem.setAttribute("y", str(normal[1]))
            tmp_elem.setAttribute("z", str(normal[2]))
            # Save the texture coords
            if exportUVs and len(face.uv):
                texcoord = vert.uvco
                vertex_elem = xmldoc.createElement("vertex")
                elems[3].appendChild(vertex_elem)
                tmp_elem = xmldoc.createElement("texcoord")
                vertex_elem.appendChild(tmp_elem)
                tmp_elem.setAttribute("u", str(face.uv[x][0]))
                tmp_elem.setAttribute("v", str(face.uv[x][1]))
            # track the vertex id
            vertid = len(verts)
            verts[vert] = vertid
            
        # Set the vert index on the face
        face_elem.setAttribute("v%d" % curvert, str(vertid))
        curvert += 1

## Multiply a vertex by a 4x4 array
def multVertByMat4(vert, mat):
    global rescale

    res_vert = []
    res_vert.append((vert.co[0] * mat[0][0] + vert.co[1] * mat[1][0] + vert.co[2] * mat[2][0] + mat[3][0]) * rescale[0])
    res_vert.append((vert.co[0] * mat[0][1] + vert.co[1] * mat[1][1] + vert.co[2] * mat[2][1] + mat[3][1]) * rescale[1])
    res_vert.append((vert.co[0] * mat[0][2] + vert.co[1] * mat[1][2] + vert.co[2] * mat[2][2] + mat[3][2]) * rescale[2])

    return res_vert

def addMaterial(mat, xmldoc, mesh_elem):
    material_elem = xmldoc.createElement("material")
    material_elem.setAttribute("name", mat.name)
    materials_elem.appendChild(material_elem)
    tmp_elem = xmldoc.createElement("ambient")
    tmp_elem.setAttribute("red", str(mat.R))
    tmp_elem.setAttribute("blue", str(mat.B))
    tmp_elem.setAttribute("green", str(mat.G))
    tmp_elem.setAttribute("alpha", str(mat.alpha))
    material_elem.appendChild(tmp_elem)
    tmp_elem = xmldoc.createElement("diffuse")
    tmp_elem.setAttribute("red", str(mat.R))
    tmp_elem.setAttribute("blue", str(mat.B))
    tmp_elem.setAttribute("green", str(mat.G))
    tmp_elem.setAttribute("alpha", str(mat.alpha))
    material_elem.appendChild(tmp_elem)
    tmp_elem = xmldoc.createElement("specular")
    tmp_elem.setAttribute("red", str(mat.specR))
    tmp_elem.setAttribute("blue", str(mat.specB))
    tmp_elem.setAttribute("green", str(mat.specG))
    tmp_elem.setAttribute("alpha", str(mat.specTransp))
    material_elem.appendChild(tmp_elem)
    tmp_elem = xmldoc.createElement("texturelayers")
    material_elem.appendChild(tmp_elem)



print "Starting export to OGRE XML format..."

objs = Blender.Object.GetSelected()
if (len(objs) == 0):
    print "No Objects Selected!"
    exit

fd = open("OGREObject.xml", 'w')

# Create our xml document
xmldoc = Document()
# Add in our defaults
mesh_elem = xmldoc.createElement("mesh")
xmldoc.appendChild(mesh_elem)
materials_elem = xmldoc.createElement("materials")
mesh_elem.appendChild(materials_elem)
submeshes_elem = xmldoc.createElement("submeshes")
mesh_elem.appendChild(submeshes_elem)

all_mats = []

for obj in objs:
    if not obj or obj.getType() != "Mesh":
        print "Skipping non mesh"
        continue

    mesh = obj.getData()
    mesh_data = Blender.NMesh.GetRaw(mesh.name)
    if not mesh_data:
        continue

    print "Exporting ",obj.name
    pbar(0.0, "Exporting %s" % obj.name) 

    # The XML we'll need
    cmnt = xmldoc.createComment("Submesh for " + obj.name)
    submeshes_elem.appendChild(cmnt)
    submesh_elem = xmldoc.createElement("submesh")
    submeshes_elem.appendChild(submesh_elem)
    submesh_elem.setAttribute("useSharedVertices", "false")
    faces_elem = xmldoc.createElement("faces")
    submesh_elem.appendChild(faces_elem)
    geometry_elem = xmldoc.createElement("geometry")
    submesh_elem.appendChild(geometry_elem)
    vb_position_elem = xmldoc.createElement("vertexbuffer")
    vb_position_elem.setAttribute("positions", "true")
    vb_position_elem.setAttribute("normals", "false")
    vb_position_elem.setAttribute("colours", "false")
    vb_position_elem.setAttribute("numtexcoords", "0")
    geometry_elem.appendChild(vb_position_elem)
    vb_normal_elem = xmldoc.createElement("vertexbuffer")
    vb_normal_elem.setAttribute("positions", "false")
    vb_normal_elem.setAttribute("normals", "true")
    vb_normal_elem.setAttribute("colours", "false")
    vb_normal_elem.setAttribute("numtexcoords", "0")
    geometry_elem.appendChild(vb_normal_elem)
    vb_texcoord_elem = None
    if exportUVs and (len(mesh_data.faces[0].uv)):
        vb_texcoord_elem = xmldoc.createElement("vertexbuffer")
        vb_texcoord_elem.setAttribute("positions", "false")
        vb_texcoord_elem.setAttribute("normals", "false")
        vb_texcoord_elem.setAttribute("colours", "false")
        vb_texcoord_elem.setAttribute("numtexcoords", "1")
        vb_texcoord_elem.setAttribute("texcoordsets", "0")
        vb_texcoord_elem.setAttribute("texcoorddimensions", "2")
        geometry_elem.appendChild(vb_texcoord_elem)

    # Check for new materials
    mesh_mats = mesh.getMaterials()
    for mat in mesh_mats:
        if mat.name not in all_mats:
            addMaterial(mat, xmldoc, mesh_elem)
            all_mats.append(mat.name)
    if mesh_mats:
        submesh_elem.setAttribute("material", mesh_mats[0].name)
    else:
        submesh_elem.setAttribute("material", "BaseWhite");
    
    # start processing the mesh data
    obj_mat = obj.getMatrix()
    verts = {}

    # Save the faces with the correct vertex information
    num_faces = 0
    elems = [faces_elem, vb_position_elem, vb_normal_elem, vb_texcoord_elem]
    face_count = len(mesh_data.faces)
    for face in mesh_data.faces:
        if (len(face) == 4):
            addFace(face, elems, verts, [0,1,2])
            addFace(face, elems, verts, [2,3,0])
            num_faces += 2
            face_count += 1
        elif (len(face) == 2):
            print "Edge encountered"
        else:
            addFace(face, elems, verts)
            num_faces += 1
        cur_perc = float(num_faces)/float(face_count)
        pbar(cur_perc, "Exporting %s (%d%%)" % (obj.name, cur_perc * 100.0))

    faces_elem.setAttribute("count", str(num_faces))
    geometry_elem.setAttribute("count", str(len(verts)))

# Save it out
xmldoc.writexml(fd)
fd.close()

xmldoc.unlink()
del xmldoc

print "Done export to OGRE XML Format."
