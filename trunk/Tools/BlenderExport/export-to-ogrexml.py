##
# OGRE XML format exporter
#   This script currently only works with Blender Publisher 2.25
#
# Revisions:
#   0.1 :  Initial release
#   0.2 :  Moved to the libxml2 library for footprint reasons.  Can fairly
#           easily handle a 48k model now.
#
# Things I want to do:
#   - Interact directly with the xmlconverter
#   - GUI, gtk maybe, that'll make the progress bars a lot more helpful
#   - Colors
#
# Original Author:  Thomas "temas" Muldowney <temas@box5.net>
#
# HOW TO USE:
#   This script is fairly easy to use.  You open it up in a blender text
#   window (alt-f11).  Then you select all of the objects you want exported.
#   This is often easier in the relations view (alt-f9).  Once you have those
#   selected run the python script with alt-p over the text window.  If you get
#   errors about not being able to find the libxml2 library then you probably
#   need to export a PYTHONPATH environment variable before running the script.
#   For example I use:  
#     export PYTHONPATH=/usr/lib/python2.1/:/usr/lib/python2.1/site-packages
#   Below are a few options while exporting, rescaling on the X, Y, and Z axises
#   and you can enable UV exporting by chaning the 0 to a 1.  Make sure you've
#   assigned UV coordinates though.  Use the OGRE forums
#   (http://ogre.sf.net/phpBB2) if you have any questions, comments, problems,
#   or anything else fun like that.
##
import Blender
import sys
import libxml2

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
    face_elem = elems[0].newChild(None, "face", None)
    curvert = 1
    for x in indices:
        vert = face[x]
        vertid = 0
        if verts.has_key(vert):
            vertid = verts[vert]
        else:
            vertex_elem = elems[1].newChild(None, "vertex", None)
            tmp_elem = vertex_elem.newChild(None, "position", None)
            xyz = multVertByMat4(vert, obj_mat)
            tmp_elem.setProp("x", str(xyz[0]))
            tmp_elem.setProp("y", str(xyz[1]))
            tmp_elem.setProp("z", str(xyz[2]))
            # Save the normals
            normal = vert.no
            vertex_elem = elems[2].newChild(None, "vertex", None)
            tmp_elem = vertex_elem.newChild(None, "normal", None)
            tmp_elem.setProp("x", str(normal[0]))
            tmp_elem.setProp("y", str(normal[1]))
            tmp_elem.setProp("z", str(normal[2]))
            # Save the texture coords
            if exportUVs and len(face.uv):
                texcoord = vert.uvco
                vertex_elem = elems[3].newChild(None, "vertex", None)
                tmp_elem = vertex_elem.newChild(None, "texcoord", None)
                tmp_elem.setProp("u", str(face.uv[x][0]))
                tmp_elem.setProp("v", str(face.uv[x][1]))
            # track the vertex id
            vertid = len(verts)
            verts[vert] = vertid
            
        # Set the vert index on the face
        face_elem.setProp("v%d" % curvert, str(vertid))
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
    material_elem = materials_elem.newChild(None, "material", None)
    material_elem.setProp("name", mat.name)
    tmp_elem = material_elem.newChild(None, "ambient", None)
    tmp_elem.setProp("red", str(mat.R))
    tmp_elem.setProp("blue", str(mat.B))
    tmp_elem.setProp("green", str(mat.G))
    tmp_elem.setProp("alpha", str(mat.alpha))
    tmp_elem = material_elem.newChild(None, "diffuse", None)
    tmp_elem.setProp("red", str(mat.R))
    tmp_elem.setProp("blue", str(mat.B))
    tmp_elem.setProp("green", str(mat.G))
    tmp_elem.setProp("alpha", str(mat.alpha))
    tmp_elem = material_elem.newChild(None, "specular", None)
    tmp_elem.setProp("red", str(mat.specR))
    tmp_elem.setProp("blue", str(mat.specB))
    tmp_elem.setProp("green", str(mat.specG))
    tmp_elem.setProp("alpha", str(mat.specTransp))
    tmp_elem = material_elem.newChild(None, "texturelayers", None)



print "Starting export to OGRE XML format..."

objs = Blender.Object.GetSelected()
if (len(objs) == 0):
    print "No Objects Selected!"
    sys.exit(0)

# Create our xml document
xmldoc = libxml2.newDoc("1.0")
if not xmldoc:
    print "Unable to create doc!!!"
    sys.exit(0)

# Add in our defaults
mesh_elem = libxml2.newNode("mesh")
try:
    mesh_elem.docSetRootElement(xmldoc)
except libxml2.treeError:
    pass
materials_elem = mesh_elem.newChild(None, "materials", None)
submeshes_elem = mesh_elem.newChild(None, "submeshes", None)

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
    cmnt = xmldoc.newDocComment("Submesh for " + obj.name)
    submesh_elem = submeshes_elem.newChild(None, "submesh", None)
    submesh_elem.setProp("useSharedVertices", "false")
    faces_elem = submesh_elem.newChild(None, "faces", None)
    geometry_elem = submesh_elem.newChild(None, "geometry", None)
    vb_position_elem = geometry_elem.newChild(None, "vertexbuffer", None)
    vb_position_elem.setProp("positions", "true")
    vb_position_elem.setProp("normals", "false")
    vb_position_elem.setProp("colours", "false")
    vb_position_elem.setProp("numtexcoords", "0")
    vb_normal_elem = geometry_elem.newChild(None, "vertexbuffer", None)
    vb_normal_elem.setProp("positions", "false")
    vb_normal_elem.setProp("normals", "true")
    vb_normal_elem.setProp("colours", "false")
    vb_normal_elem.setProp("numtexcoords", "0")
    vb_texcoord_elem = None
    if exportUVs and (len(mesh_data.faces[0].uv)):
        vb_texcoord_elem = geometry_elem.newChild(None, "vertexbuffer", None)
        vb_texcoord_elem.setProp("positions", "false")
        vb_texcoord_elem.setProp("normals", "false")
        vb_texcoord_elem.setProp("colours", "false")
        vb_texcoord_elem.setProp("numtexcoords", "1")
        vb_texcoord_elem.setProp("texcoordsets", "0")
        vb_texcoord_elem.setProp("texcoorddimensions", "2")

    # Check for new materials
    mesh_mats = mesh.getMaterials()
    for mat in mesh_mats:
        if mat.name not in all_mats:
            addMaterial(mat, xmldoc, mesh_elem)
            all_mats.append(mat.name)
    if mesh_mats:
        submesh_elem.setProp("material", mesh_mats[0].name)
    else:
        submesh_elem.setProp("material", "BaseWhite");
    
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

    faces_elem.setProp("count", str(num_faces))
    geometry_elem.setProp("count", str(len(verts)))

# Save it out
print "Saving to OGREObject.xml"
xmldoc.saveFile("OGREObject.xml")

xmldoc.freeDoc()
del xmldoc

print "Done export to OGRE XML Format."
