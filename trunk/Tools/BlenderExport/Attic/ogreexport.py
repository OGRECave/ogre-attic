# Blender to Ogre Mesh and Skeleton Exporter v0.7
# url: http://people.freenet.de/hoffmajs/exporter/

# Ogre exporter written by Jens Hoffmann
# based on the Cal3D exporter v0.5 written by Jean-Baptiste LAMY

# Copyright (C) 2003 Jens Hoffmann -- <hoffmajs@gmx.de>
# Copyright (C) 2003 Jean-Baptiste LAMY -- jiba@tuxfamily.org
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


# this export script needs blender 2.28 or newer.

# Usage:
# select the meshes you want to export and run this script (alt-p)

# note:
# exporting animations can be a problem, since the blender python api
# has no functions to get an ipo curve for a bone directly.
# instead, the ipo is choosen by its name which is normally
# "Action.<bone-name>[.xxx]". this is parsed as: bone <bone-name> in
# Action <Action.xxx>.
# But if you rename the ipo, or you choose a too long bone-name (which
# results in a trucated name in the ipo name) the exporter won't find
# the ipo for its bone.
# then, you have to rename the bone _and_ the ipo.

# Another problem is that the name of the Action is lost on export,
# instead the actions will be named "Action.000", "Action.001" and so on.
# you can set a mapping (see below) to rename the action.


# Parameters:

# The directory where the exported files are saved.
SAVE_DIR = "/tmp/ogre/"

# all material definitions go in this file (relative to save dir)
MATERIAL_FILE = "export.material"

# scale factor
SCALE = 5.0

# export sticky uv coordinates, if available (0 = off)
EXPORT_UV = 1

# export skeletons and bone weights in meshes (0 = off)
EXPORT_SKELETON = 1

# animation speed in frames per second
FPS = 25.0

# Use this dictionary to rename animations
RENAME_ANIMATIONS = {
   "Action.000" : "Walk",
   "Action.001" : "Jump",
  
  }

# create an extra mesh with the form of the skeleton
# (looks similar like the armature in blender)
EXPORT_TEST_SKELETON = 0


#######################################################################################
## Code starts here.

import sys, os.path, math, string
import Blender

#######################################################################################
## math functions
## (not everything is used, but im to lazy to sort it out)

def quaternion2matrix(q):
  xx = q[0] * q[0]
  yy = q[1] * q[1]
  zz = q[2] * q[2]
  xy = q[0] * q[1]
  xz = q[0] * q[2]
  yz = q[1] * q[2]
  wx = q[3] * q[0]
  wy = q[3] * q[1]
  wz = q[3] * q[2]
  return [[1.0 - 2.0 * (yy + zz),       2.0 * (xy + wz),       2.0 * (xz - wy), 0.0],
          [      2.0 * (xy - wz), 1.0 - 2.0 * (xx + zz),       2.0 * (yz + wx), 0.0],
          [      2.0 * (xz + wy),       2.0 * (yz - wx), 1.0 - 2.0 * (xx + yy), 0.0],
          [0.0                  , 0.0                  , 0.0                  , 1.0]]

def matrix2quaternion(m):
  s = math.sqrt(abs(m[0][0] + m[1][1] + m[2][2] + m[3][3]))
  if s == 0.0:
    x = abs(m[2][1] - m[1][2])
    y = abs(m[0][2] - m[2][0])
    z = abs(m[1][0] - m[0][1])
    if   (x >= y) and (x >= z): return 1.0, 0.0, 0.0, 0.0
    elif (y >= x) and (y >= z): return 0.0, 1.0, 0.0, 0.0
    else:                       return 0.0, 0.0, 1.0, 0.0
  return quaternion_normalize([
    -(m[2][1] - m[1][2]) / (2.0 * s),
    -(m[0][2] - m[2][0]) / (2.0 * s),
    -(m[1][0] - m[0][1]) / (2.0 * s),
    0.5 * s,
    ])

def quaternion_normalize(q):
  l = math.sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3])
  return q[0] / l, q[1] / l, q[2] / l, q[3] / l

def quaternion_multiply(q1, q2):
  r = [
    q2[3] * q1[0] + q2[0] * q1[3] + q2[1] * q1[2] - q2[2] * q1[1],
    q2[3] * q1[1] + q2[1] * q1[3] + q2[2] * q1[0] - q2[0] * q1[2],
    q2[3] * q1[2] + q2[2] * q1[3] + q2[0] * q1[1] - q2[1] * q1[0],
    q2[3] * q1[3] - q2[0] * q1[0] - q2[1] * q1[1] - q2[2] * q1[2],
    ]
  d = math.sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2] + r[3] * r[3])
  r[0] /= d
  r[1] /= d
  r[2] /= d
  r[3] /= d
  return r

def matrix_translate(m, v):
  m[3][0] += v[0]
  m[3][1] += v[1]
  m[3][2] += v[2]
  return m

def matrix_multiply(b, a):
  return [ [
    a[0][0] * b[0][0] + a[0][1] * b[1][0] + a[0][2] * b[2][0],
    a[0][0] * b[0][1] + a[0][1] * b[1][1] + a[0][2] * b[2][1],
    a[0][0] * b[0][2] + a[0][1] * b[1][2] + a[0][2] * b[2][2],
    0.0,
    ], [
    a[1][0] * b[0][0] + a[1][1] * b[1][0] + a[1][2] * b[2][0],
    a[1][0] * b[0][1] + a[1][1] * b[1][1] + a[1][2] * b[2][1],
    a[1][0] * b[0][2] + a[1][1] * b[1][2] + a[1][2] * b[2][2],
    0.0,
    ], [
    a[2][0] * b[0][0] + a[2][1] * b[1][0] + a[2][2] * b[2][0],
    a[2][0] * b[0][1] + a[2][1] * b[1][1] + a[2][2] * b[2][1],
    a[2][0] * b[0][2] + a[2][1] * b[1][2] + a[2][2] * b[2][2],
     0.0,
    ], [
    a[3][0] * b[0][0] + a[3][1] * b[1][0] + a[3][2] * b[2][0] + b[3][0],
    a[3][0] * b[0][1] + a[3][1] * b[1][1] + a[3][2] * b[2][1] + b[3][1],
    a[3][0] * b[0][2] + a[3][1] * b[1][2] + a[3][2] * b[2][2] + b[3][2],
    1.0,
    ] ]

def matrix_invert(m):
  det = (m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
       - m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
       + m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]))
  if det == 0.0: return None
  det = 1.0 / det
  r = [ [
      det * (m[1][1] * m[2][2] - m[2][1] * m[1][2]),
    - det * (m[0][1] * m[2][2] - m[2][1] * m[0][2]),
      det * (m[0][1] * m[1][2] - m[1][1] * m[0][2]),
      0.0,
    ], [
    - det * (m[1][0] * m[2][2] - m[2][0] * m[1][2]),
      det * (m[0][0] * m[2][2] - m[2][0] * m[0][2]),
    - det * (m[0][0] * m[1][2] - m[1][0] * m[0][2]),
      0.0
    ], [
      det * (m[1][0] * m[2][1] - m[2][0] * m[1][1]),
    - det * (m[0][0] * m[2][1] - m[2][0] * m[0][1]),
      det * (m[0][0] * m[1][1] - m[1][0] * m[0][1]),
      0.0,
    ] ]
  r.append([
    -(m[3][0] * r[0][0] + m[3][1] * r[1][0] + m[3][2] * r[2][0]),
    -(m[3][0] * r[0][1] + m[3][1] * r[1][1] + m[3][2] * r[2][1]),
    -(m[3][0] * r[0][2] + m[3][1] * r[1][2] + m[3][2] * r[2][2]),
    1.0,
    ])
  return r

def matrix_transpose(m):
  return [ [ m[0][0], m[1][0], m[2][0], m[3][0] ],
           [ m[0][1], m[1][1], m[2][1], m[3][1] ],
           [ m[0][2], m[1][2], m[2][2], m[3][2] ],
           [ m[0][3], m[1][3], m[2][3], m[3][3] ] ]

def matrix_rotate_x(angle):
  cos = math.cos(angle)
  sin = math.sin(angle)
  return [
    [1.0,  0.0, 0.0, 0.0],
    [0.0,  cos, sin, 0.0],
    [0.0, -sin, cos, 0.0],
    [0.0,  0.0, 0.0, 1.0],
    ]

def matrix_rotate_y(angle):
  cos = math.cos(angle)
  sin = math.sin(angle)
  return [
    [cos, 0.0, -sin, 0.0],
    [0.0, 1.0,  0.0, 0.0],
    [sin, 0.0,  cos, 0.0],
    [0.0, 0.0,  0.0, 1.0],
    ]

def matrix_rotate_z(angle):
  cos = math.cos(angle)
  sin = math.sin(angle)
  return [
    [ cos, sin, 0.0, 0.0],
    [-sin, cos, 0.0, 0.0],
    [ 0.0, 0.0, 1.0, 0.0],
    [ 0.0, 0.0, 0.0, 1.0],
    ]

def matrix_rotate(axis, angle):
  vx  = axis[0]
  vy  = axis[1]
  vz  = axis[2]
  vx2 = vx * vx
  vy2 = vy * vy
  vz2 = vz * vz
  cos = math.cos(angle)
  sin = math.sin(angle)
  co1 = 1.0 - cos
  return [
    [vx2 * co1 + cos,          vx * vy * co1 + vz * sin, vz * vx * co1 - vy * sin, 0.0],
    [vx * vy * co1 - vz * sin, vy2 * co1 + cos,          vy * vz * co1 + vx * sin, 0.0],
    [vz * vx * co1 + vy * sin, vy * vz * co1 - vx * sin, vz2 * co1 + cos,          0.0],
    [0.0, 0.0, 0.0, 1.0],
    ]

def matrix_scale(fx, fy, fz):
  return [
    [ fx, 0.0, 0.0, 0.0],
    [0.0,  fy, 0.0, 0.0],
    [0.0, 0.0,  fz, 0.0],
    [0.0, 0.0, 0.0, 1.0],
    ]
  
def point_by_matrix(p, m):
  return [p[0] * m[0][0] + p[1] * m[1][0] + p[2] * m[2][0] + m[3][0],
          p[0] * m[0][1] + p[1] * m[1][1] + p[2] * m[2][1] + m[3][1],
          p[0] * m[0][2] + p[1] * m[1][2] + p[2] * m[2][2] + m[3][2]]

def point_add(p1, p2):
  return [ p1[0] + p2[0],
           p1[1] + p2[1],
           p1[2] + p2[2] ]

def point_distance(p1, p2):
  return math.sqrt((p2[0] - p1[0]) ** 2 + (p2[1] - p1[1]) ** 2 + (p2[2] - p1[2]) ** 2)

def vector_by_matrix(p, m):
  return [p[0] * m[0][0] + p[1] * m[1][0] + p[2] * m[2][0],
          p[0] * m[0][1] + p[1] * m[1][1] + p[2] * m[2][1],
          p[0] * m[0][2] + p[1] * m[1][2] + p[2] * m[2][2]]

def vector_length(v):
  return math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])

def vector_normalize(v):
  l = math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])
  if l <= 0.000001:
    print "error in normalize"
    return [0 , l, 0]
  return [v[0] / l, v[1] / l, v[2] / l]

def normal_by_matrix(n, m):
  m = matrix_transpose(matrix_invert(m))
  return vector_normalize(vector_by_matrix(n, m))


def quaternion2axisangle(q):
  x, y, z, w = quaternion_normalize(q)
  s = math.sqrt(1 - w * w)
  if abs(s) < 0.00000001:
    s=0.00000001

  angle = 2 * math.acos(w)
  x , y, z = x/s, y/s, z/s
  return [angle, x, y, z]


def vector_dotproduct(v1, v2):
  return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]

def vector_crossproduct(v1, v2):
  return [
    v1[1] * v2[2] - v1[2] * v2[1],
    v1[2] * v2[0] - v1[0] * v2[2],
    v1[0] * v2[1] - v1[1] * v2[0],
    ]

def vector_angle(v1, v2):
  s = vector_length(v1) * vector_length(v2)
  f = vector_dotproduct(v1, v2) / s
  if f >=  1.0: return 0.0
  if f <= -1.0: return math.pi / 2.0
  return math.atan(-f / math.sqrt(1.0 - f * f)) + math.pi / 2.0


#######################################################################################
## data structures

class Material:
  def __init__(self, name, mat, texname):
    self.name = name
    self.mat = mat
    self.texture = texname

class SubMesh:
  def __init__(self, material):
    self.material   = material
    self.vertices   = []
    self.faces      = []

  def rename_vertices(self, new_vertices):
    # Rename (change ID) of all vertices, such as self.vertices == new_vertices.
    for i in range(len(new_vertices)): new_vertices[i].id = i
    self.vertices = new_vertices

class Vertex:
  def __init__(self, submesh, loc, normal):
    self.loc    = loc
    self.normal = normal
    self.uvmaps = []
    self.influences = []
    
    self.cloned_from = None
    self.clones      = []
    self.submesh = submesh
    self.id = len(submesh.vertices)
    submesh.vertices.append(self)

class UVMap:
  def __init__(self, u, v):
    self.u = u
    self.v = v

class Influence:
  def __init__(self, bone, weight):
    self.bone   = bone
    self.weight = weight
    
class Face:
  def __init__(self, submesh, vertex1, vertex2, vertex3):
    self.vertex1 = vertex1
    self.vertex2 = vertex2
    self.vertex3 = vertex3
    self.submesh = submesh
    submesh.faces.append(self)

class Skeleton:
  def __init__(self, name):
    self.name = name
    self.bones = []
    self.bonesDict = {}
    self.animationsDict = {}

class Bone:
  def __init__(self, skeleton, parent, name, loc, rot):
    self.parent = parent
    self.name   = name
    self.loc = loc # offset from parent bone
    self.rot = rot # axis as quaternion
    self.children = []

    if parent:
      parent.children.append(self)
    
    self.id = len(skeleton.bones)
    skeleton.bones.append(self)
    skeleton.bonesDict[name] =self

class Animation:
  def __init__(self, name, duration = 0.0):
    self.name     = name
    self.duration = duration
    self.tracksDict = {} # Map bone names to tracks
    
class Track:
  def __init__(self, animation, bone):
    self.bone      = bone
    self.keyframes = []
    
    self.animation = animation
    animation.tracksDict[bone.name] = self
    
class KeyFrame:
  def __init__(self, track, time, loc, rot, scale):
    self.time = time
    self.loc  = loc
    self.rot  = rot
    self.scale = scale
    
    self.track = track
    track.keyframes.append(self)

class TestSkel:
  def __init__(self, skeleton):
    self.skeleton = skeleton
    self.bones = []
    
  def addBone(self, name, p1, p2):
    self.bones.append([name, p1, p2])



#######################################################################################
## Armature stuff

def blender_bone2matrix(head, tail, roll):
  # Convert bone rest state (defined by bone.head, bone.tail and bone.roll)
  # to a matrix (the more standard notation).
  # Taken from blenkernel/intern/armature.c in Blender source.
  # See also DNA_armature_types.h:47.
  
  nor = vector_normalize([ tail[0] - head[0],
                           tail[1] - head[1],
                           tail[2] - head[2] ])

  # Find Axis & Amount for bone matrix
  target = [0.0, 1.0, 0.0]
  axis = vector_crossproduct(target, nor)
  
  # is nor a multiple of target?
  if vector_dotproduct(axis, axis) > 0.0000000000001:
    axis  = vector_normalize(axis)
    theta = math.acos(vector_dotproduct(target, nor))
    bMatrix = matrix_rotate(axis, theta)

  else:
    # point same direction, or opposite?
    if vector_dotproduct(target, nor) > 0.0:
      updown = 1.0    
    else:
      updown = -1.0
    
    # Quoted from Blender source : "I think this should work ..."
    bMatrix = [ [updown,    0.0, 0.0, 0.0],
                [   0.0, updown, 0.0, 0.0],
                [   0.0,    0.0, 1.0, 0.0],
                [   0.0,    0.0, 0.0, 1.0] ]

  rMatrix = matrix_rotate(nor, roll)
  return matrix_multiply(rMatrix, bMatrix)


def calc_rootaxis(pos, pos2, tmp_mat):
  # get root axis:
  # there is probably an easier way than this crap...
  
  # (pos2 - pos) is the y-axis (bone-axis) we want
  nor = vector_normalize([ pos2[0] - pos[0],
                           pos2[1] - pos[1],
                           pos2[2] - pos[2] ])
      
  pz = point_by_matrix([0, 0, 1], tmp_mat)
  pz = vector_normalize([ pz[0] - pos[0],
                          pz[1] - pos[1],
                          pz[2] - pos[2] ])

  px = point_by_matrix([1, 0, 0], tmp_mat)
  px = vector_normalize([ px[0] - pos[0],
                          px[1] - pos[1],
                          px[2] - pos[2] ])

  # px1 is px perpendicular to the y-axis
  px1 = vector_crossproduct(nor, pz)
  if vector_dotproduct(px1, px) < 0.0:
    px1 = vector_crossproduct(pz, nor)
    print "neg"
  px1 = vector_normalize(px1)

  # get new axis (in correct y-direction, but wrong roll)
  axis_rot = blender_bone2matrix(pos, pos2, 0)
  
  # correct the roll
  px2 = point_by_matrix([1, 0, 0], axis_rot)
  roll = math.acos(vector_dotproduct(px1, px2))

  rMatrix = matrix_rotate(nor, roll)
  axis_rot = matrix_multiply(rMatrix, axis_rot)

  #print "angle before: ", roll * 180/math.pi
  #px2 = point_by_matrix([1, 0, 0], axis_rot)
  #py = vector_normalize(nor)
  #roll = math.acos(vector_dotproduct(px1, px2))
  #print "new angle   : ", roll * 180/math.pi
  #print "px-px1:", vector_dotproduct(px, px1)
  #print "px-px2:", vector_dotproduct(px1, px2)
  #print "py-px2:", vector_dotproduct(py, px2)
  #print "pz-px2:", vector_dotproduct(pz, px2)
  
  return axis_rot


def convert_armature(skeleton, obj, debugskel):

  stack = []
  matrix = matrix_multiply(BASE_MATRIX, obj.getMatrix())

  # make a new root bone
  loc = [ 0, 0, 0 ]
  rot = [ 0, 0, 0, 1 ]
  parent = None #Bone(skeleton, None, "__root", loc, rot)
  #parent.pos = loc
  
  for bbone in obj.getData().getBones():
    stack.append([bbone, parent, matrix, loc])

  while len(stack):
    bbone, parent, accu_mat, parent_pos = stack.pop()

    head = bbone.getHead()
    tail = bbone.getTail()
    roll = bbone.getRoll()

    # get the restmat 
    R_bmat = blender_bone2matrix(head, tail, roll)

    # get the bone's root offset (in the parent's coordinate system)
    T_root = [ [       1,       0,       0,      0 ],
               [       0,       1,       0,      0 ],
               [       0,       0,       1,      0 ],
               [ head[0], head[1], head[2],      1 ] ]

    # get the bone length translation (length along y axis)
    dx, dy, dz = tail[0] - head[0], tail[1] - head[1], tail[2] - head[2]
    ds = math.sqrt(dx*dx + dy*dy + dz*dz)
    T_len = [ [ 1,  0,  0,  0 ],
              [ 0,  1,  0,  0 ],
              [ 0,  0,  1,  0 ],
              [ 0, ds,  0,  1 ] ]

    # calculate bone points in world coordinates
    accu_mat = matrix_multiply(accu_mat, T_root)
    pos = point_by_matrix([ 0, 0, 0 ], accu_mat)

    accu_mat = tmp_mat = matrix_multiply(accu_mat, R_bmat)
    accu_mat = matrix_multiply(accu_mat, T_len)
    pos2 = point_by_matrix([ 0, 0, 0 ], accu_mat)
    
    if debugskel:
      debugskel.addBone(bbone.getName(), pos, pos2)

    # local rotation and distance from parent bone
    if parent:
      dx = pos[0] - parent_pos[0]
      dy = pos[1] - parent_pos[1]
      dz = pos[2] - parent_pos[2]
      loc = [ 0, math.sqrt(dx*dx + dy*dy + dz*dz), 0]

      rot = matrix2quaternion(R_bmat)

    else:
      loc = pos
      
      axis_rot = calc_rootaxis(pos, pos2, tmp_mat)
      rot = matrix2quaternion(axis_rot)

    parent = Bone(skeleton, parent, bbone.getName(), loc, rot)
    for child in bbone.getChildren():
      stack.append([child, parent, accu_mat, pos])


def export_skeleton(obj):
  global skeletonsDict
  
  if not EXPORT_SKELETON or skeletonsDict.has_key(obj.name) or obj.getType() != "Armature":
    return
  
  data = obj.getData()
  skeleton = Skeleton(obj.name)
  skeletonsDict[obj.name] = skeleton

  testskel = None
  if EXPORT_TEST_SKELETON:
    testskel = TestSkel(skeleton)

  convert_armature(skeleton, obj, testskel)

  if testskel:
    export_testskel(testskel)

  # get scale from obj matrix (for loc keys)
  matrix = matrix_multiply(BASE_MATRIX, obj.getMatrix())
  pz = point_by_matrix([0, 0, 0], matrix)

  p = point_by_matrix([1, 0, 0], matrix)
  dx, dy, dz = p[0] - pz[0], p[1] - pz[1], p[2] - pz[2], 
  scale_x = math.sqrt(dx*dx + dy*dy + dz*dz)

  p = point_by_matrix([0, 1, 0], matrix)
  dx, dy, dz = p[0] - pz[0], p[1] - pz[1], p[2] - pz[2], 
  scale_y = math.sqrt(dx*dx + dy*dy + dz*dz)

  p = point_by_matrix([0, 0, 1], matrix)
  dx, dy, dz = p[0] - pz[0], p[1] - pz[1], p[2] - pz[2], 
  scale_z = math.sqrt(dx*dx + dy*dy + dz*dz)

  #print "scale" , scale_x, scale_y, scale_z

  for ipo in Blender.Ipo.Get():
    if ipo.getNcurves() == 0:
      continue
    
    # too bad, there is no clean way to get the ipo for a bone :(
    ipoName = ipo.getName()
    prefix = "Action."
    if ipoName[0:len(prefix)] != prefix:
      print "error: ignored IPO: \"%s\" (only Action.<bone-name>[.xxx] is recognized)" % ipoName
      continue

    rest = ipoName[len(prefix):]
    ext = rest[-4:]
    d = string.digits
    if len(ext) == 4 and ext[1] in d and ext[2] in d and ext[3] in d:
      boneName = rest[0:-4]
      animationName = prefix+ext[1:]
    else:
      boneName = rest
      animationName = prefix+"000"
      
    bone = 0
    found = 0
    for b in skeleton.bones:
      if boneName == b.name or boneName+ext == b.name:
        found += 1
        bone = b

    if found != 1:
      if found == 0:
        print "error: can not find a bone for ipo \"%s\"." % ipoName
      else:
        print "error: more than one bone matches the ipo \"%s\"." % ipoName
      print "       please rename the bone and its ipo in blender."
      continue

    #print "IPO %s -> bone: %s, animation: %s" % (ipoName, bone.name, animationName)

    animation = skeleton.animationsDict.get(animationName)
    if not animation:
      animation = skeleton.animationsDict[animationName] = Animation(animationName)

    track = animation.tracksDict.get(boneName)
    if not track:
      track = animation.tracksDict[boneName] = Track(animation, bone)

    # map curve names to curvepos
    curveId = {}
    id = have_quat = 0
    for curve in ipo.getCurves():
      name = curve.getName()
      if (name == "LocX" or name == "LocY" or name == "LocZ" or
          name == "SizeX" or name == "SizeY" or name == "SizeZ" or
          name == "QuatX" or name == "QuatY" or name == "QuatZ" or name == "QuatW"):
        curveId[name] = id
        id += 1
      else:
        # bug: 2.28 does not return "Quat*"...
        if not have_quat:
          curveId["QuatX"] = id
          curveId["QuatY"] = id+1
          curveId["QuatZ"] = id+2
          curveId["QuatW"] = id+3
          id += 4
          have_quat = 1

    # get all frame numbers where this ipo has a point in one of its curves 
    framenumberSet = {}
    for i in range(ipo.getNcurves()):
      for bez in range(ipo.getNBezPoints(i)):
        frame = int(ipo.getCurveBeztriple(i, bez)[3])
        framenumberSet[frame] = frame
    framenumberSet[1] = 1 # make sure there is a keyframe at 1
    framenumbers = framenumberSet.values()
    framenumbers.sort()
    
    for frame in framenumbers:
      loc = [ 0.0, 0.0, 0.0 ]
      rot = [ 0.0, 0.0, 0.0, 1.0 ]
      size = 1.0

      if curveId.has_key("LocX"):
        loc = [ ipo.EvaluateCurveOn(curveId["LocX"], frame) * -1.0 * scale_x,
                ipo.EvaluateCurveOn(curveId["LocY"], frame) * -1.0 * scale_y,
                ipo.EvaluateCurveOn(curveId["LocZ"], frame) *  1.0 * scale_z ]
        
      if curveId.has_key("QuatX"):
        rot = [ ipo.EvaluateCurveOn(curveId["QuatX"], frame),
                ipo.EvaluateCurveOn(curveId["QuatY"], frame),
                ipo.EvaluateCurveOn(curveId["QuatZ"], frame),
                ipo.EvaluateCurveOn(curveId["QuatW"], frame) ]

      if curveId.has_key("SizeX"):
        sx = ipo.EvaluateCurveOn(curveId["SizeX"], frame)
        sy = ipo.EvaluateCurveOn(curveId["SizeY"], frame)
        sz = ipo.EvaluateCurveOn(curveId["SizeZ"], frame)
        size = sx
        size = max(size, sy)
        size = max(size, sz)

      # Convert time units from Blender's frame (starting at 1) to second
      time = (frame-1) / FPS
      if animation.duration < time:
        animation.duration = time

      KeyFrame(track, time, loc, rot, size)

  write_skeleton(skeleton)


def export_testskel(testskel):

  def make_face(name, p1, p2, p3):
    normal = vector_normalize(vector_crossproduct(
      [ p3[0] - p2[0], p3[1] - p2[1], p3[2] - p2[2] ],
      [ p1[0] - p2[0], p1[1] - p2[1], p1[2] - p2[2] ]))
    
    v1 = Vertex(submesh, p1, normal)
    v2 = Vertex(submesh, p2, normal)
    v3 = Vertex(submesh, p3, normal)

    id = testskel.skeleton.bonesDict[name]
    v1.influences.append(Influence(id, 1.0))
    v2.influences.append(Influence(id, 1.0))
    v3.influences.append(Influence(id, 1.0))

    Face(submesh, v1, v2, v3)

  matName = "SkeletonMaterial"
  material = materialsDict.get(matName)
  if not material:
    material = Material(matName, 0, "")
    materialsDict[matName] = material

  submesh = SubMesh(material)
  for name, p1, p2 in testskel.bones:
    axis = blender_bone2matrix(p1, p2, 0)
    axis = matrix_translate(axis, p1)

    dx, dy, dz = p1[0] - p2[0], p1[1] - p2[1], p1[2] - p2[2]
    ds = math.sqrt(dx*dx + dy*dy + dz*dz)
    d = 0.1 + 0.2 * (ds / 10.0)
    
    c1 = point_by_matrix([-d, 0,-d], axis)
    c2 = point_by_matrix([-d, 0, d], axis)
    c3 = point_by_matrix([ d, 0, d], axis)
    c4 = point_by_matrix([ d, 0,-d], axis)

    make_face(name, p2, c1, c2)
    make_face(name, p2, c2, c3)
    make_face(name, p2, c3, c4)
    make_face(name, p2, c4, c1)
    make_face(name, c3, c2, c1)
    make_face(name, c1, c4, c3)

  file = "debug"+testskel.skeleton.name
  write_mesh(file, [submesh], testskel.skeleton)


#######################################################################################
## Mesh stuff

# remap vertices for faces
def process_face(face, submesh, data, matrix, skeleton):
  global verticesDict

  if not len(face.v) in [ 3, 4 ]:
    print "ignored face with %d edges" % len(face.v)
    return

  if not face.smooth:
    # calculate the normal. (blender only provides the smoothed normal)
    p1 = face.v[0].co
    p2 = face.v[1].co
    p3 = face.v[2].co
    normal = vector_crossproduct(
      [p3[0] - p2[0], p3[1] - p2[1], p3[2] - p2[2]],
      [p1[0] - p2[0], p1[1] - p2[1], p1[2] - p2[2]],
      )
    normal = normal_by_matrix(normal, matrix)

  face_vertices = [ 0, 0, 0, 0]
  for i in range(len(face.v)):
    vertex = verticesDict.get(face.v[i].index)

    if not vertex:
      coord  = point_by_matrix (face.v[i].co, matrix)

      if face.smooth:
        normal = normal_by_matrix(face.v[i].no, matrix)

      vertex = Vertex(submesh, coord, normal)
      verticesDict[face.v[i].index] = vertex

      # set bone influences
      if skeleton:
        influences = data.getVertexInfluences(face.v[i].index)
        if not influences:
          print "Error: vertex in skinned mesh without influence! check your mesh"

        # limit influences to 4 bones per vertex
        def cmpfunc(x, y):
          xname, xweight = x
          yname, yweight = y
          return cmp(yweight, xweight)
        influences.sort(cmpfunc)
        influences = influences[0:4]

        # and make sure the sum is 1.0
        total = 0.0
        for name, weight in influences:
            total += weight

        for name, weight in influences:
          vertex.influences.append(Influence(skeleton.bonesDict[name], weight/total))

    elif not face.smooth:
      if (vertex.normal[0] != normal[0] or
          vertex.normal[1] != normal[1] or
          vertex.normal[2] != normal[2]):
        # the vertex in blender has different normals for each face:
        # we need to clone the vertex for each normal..
        # note: for smoothed faces there is only one (averaged) normal for all faces 

        old_vertex = vertex
        vertex = Vertex(submesh, vertex.loc, normal)
        vertex.cloned_from = old_vertex
        vertex.influences = old_vertex.influences
        old_vertex.clones.append(vertex)

    if submesh.material.texture:
      uv = [face.uv[i][0], face.uv[i][1]]

      if not vertex.uvmaps:
        vertex.uvmaps.append(UVMap(*uv))
      elif (vertex.uvmaps[0].u != uv[0]) or (vertex.uvmaps[0].v != uv[1]):
        # the vertex in blender has different uv maps for each face
        # we need to clone the vertex for each uv coordinate

        for clone in vertex.clones:
          if (clone.uvmaps[0].u == uv[0]) and (clone.uvmaps[0].v == uv[1]):
            vertex = clone
            break
          else: # Not yet cloned...
            old_vertex = vertex
            vertex = Vertex(submesh, vertex.loc, vertex.normal)
            vertex.cloned_from = old_vertex
            vertex.influences = old_vertex.influences
            vertex.uvmaps.append(UVMap(*uv))
            old_vertex.clones.append(vertex)

    face_vertices[i] = vertex

  # Split faces with more than 3 vertices
  Face(submesh, face_vertices[0], face_vertices[1], face_vertices[2])
  if len(face.v) == 4:
    Face(submesh, face_vertices[2], face_vertices[3], face_vertices[0])


def export_mesh(obj):
  global verticesDict
  global materialsDict
  global textureDict
  
  if obj.getType() != "Mesh":
    return

  # is this mesh attached to an armature?
  skeleton = 0
  if EXPORT_SKELETON:
    parent = obj.getParent()
    if parent and parent.getType() == "Armature":
      export_skeleton(parent)
      skeleton = skeletonsDict[parent.name]

  data = obj.getData()
  submeshes = []
  matrix = matrix_multiply(BASE_MATRIX, obj.getMatrix())

  defaultMat = 0
  nMaterials = len(data.materials)
  if nMaterials == 0:
    defaultMat = 1
    nMaterials = 1

  faces = data.faces
  while len(faces): # loop to catch all textures
    for matIndex in range(nMaterials):

      if defaultMat:
        mat = 0
        matName = "DefaultMaterial"
      else:
        mat = data.materials[matIndex]
        matName = mat.name

      # are there textures with sticky UV?
      imageName = ""
      image = None
      if EXPORT_UV and data.hasFaceUV():
        image = faces[0].image
      if image:
        imageName = image.filename
        texNum = textureDict.get(imageName)
        if not texNum:
          texNum = len(textureDict)
          textureDict[imageName] = texNum 

        texName = "Tex_%04d" % texNum
        if mat:
          matName = texName + "_" + matName
        else:
          matName = texName
      
      material = materialsDict.get(matName)
      if not material:
        material = Material(matName, mat, imageName)
        materialsDict[matName] = material
        
      submesh = SubMesh(material)
      verticesDict = {}
      tmpset = faces[:] # make a real copy (hence the [:])
      for face in tmpset:
        if defaultMat or face.materialIndex == matIndex:
          if not image or (image and image.filename == imageName):
            process_face(face, submesh, data, matrix, skeleton)
            faces.remove(face)

      if len(submesh.faces):
        submeshes.append(submesh)

  write_mesh(obj.getName(), submeshes, skeleton)


#######################################################################################
## file output

def tab(tabsize):
    return "  " * tabsize
    
def clamp(val):
    if val < 0.0:
        val = 0.0
    if val > 1.0:
        val = 1.0
    return val

def write_skeleton(skeleton):
  file = skeleton.name+".skeleton.xml"
  print "skeleton  \"%s\"" % file

  f = open(os.path.join(SAVE_DIR, file), "w")
  f.write(tab(0)+"<skeleton>\n")

  f.write(tab(1)+"<bones>\n")
  for bone in skeleton.bones:
    f.write(tab(2)+"<bone id=\"%d\" name=\"%s\">\n" % (bone.id, bone.name))

    x, y, z = bone.loc
    f.write(tab(3)+"<position x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" % (x, y, z))

    angle, x, y, z = quaternion2axisangle(bone.rot)
    f.write(tab(3)+"<rotation angle=\"%.6f\">\n" % angle)
    f.write(tab(4)+"<axis x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" % (x, y, z))
    f.write(tab(3)+"</rotation>\n")
    f.write(tab(2)+"</bone>\n")
  f.write(tab(1)+"</bones>\n")
  
  f.write(tab(1)+"<bonehierarchy>\n")
  for bone in skeleton.bones:
    parent = bone.parent
    if parent:
      f.write(tab(2)+"<boneparent bone=\"%s\" parent=\"%s\"/>\n" % (bone.name, parent.name))
  f.write(tab(1)+"</bonehierarchy>\n")

  f.write(tab(1)+"<animations>\n")

  for animation in skeleton.animationsDict.values():
    #if not animation.duration:
    #  continue

    name = RENAME_ANIMATIONS.get(animation.name) or animation.name

    f.write(tab(2)+"<animation")
    f.write(" name=\"%s\"" % name)
    f.write(" length=\"%f\">\n" % animation.duration )

    f.write(tab(3)+"<tracks>\n")

    for track in animation.tracksDict.values():
      f.write(tab(4)+"<track bone=\"%s\">\n" % track.bone.name)

      f.write(tab(5)+"<keyframes>\n")

      for keyframe in track.keyframes:
        f.write(tab(6)+"<keyframe time=\"%f\">\n" % keyframe.time)

        x, y, z = keyframe.loc
        f.write(tab(7)+"<translate x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" % (x, y, z))
          
        angle, x, y, z = quaternion2axisangle(keyframe.rot)
        f.write(tab(7)+"<rotate angle=\"%.6f\">\n" % angle)
        f.write(tab(8)+"<axis x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" % (x, y, z))
        f.write(tab(7)+"</rotate>\n")

        f.write(tab(7)+"<scale factor=\"%f\"/>\n" % keyframe.scale)

        f.write(tab(6)+"</keyframe>\n")

      f.write(tab(5)+"</keyframes>\n")
      f.write(tab(4)+"</track>\n");

    f.write(tab(3)+"</tracks>\n");
    f.write(tab(2)+"</animation>\n")

  f.write(tab(1)+"</animations>\n")
  f.write(tab(0)+"</skeleton>\n")
  f.close()


def write_mesh(name, submeshes, skeleton):
  file = name+".mesh.xml"
  print "mesh      \"%s\"" % file

  f = open(os.path.join(SAVE_DIR, file), "w")
  f.write(tab(0)+"<mesh>\n")
  f.write(tab(1)+"<submeshes>\n")
  for submesh in submeshes:

    f.write(tab(2)+"<submesh")
    f.write(" material=\"%s\"" % submesh.material.name)
    f.write(" usesharedvertices=\"false\"")
    f.write(" use32bitindexes=\"false\"")
    f.write(" operationtype=\"triangle_list\"")
    f.write(">\n")
    f.write(tab(3)+"<faces count=\"%d\">\n" % len(submesh.faces))

    for face in submesh.faces:
      v1, v2, v3  = face.vertex1.id, face.vertex2.id, face.vertex3.id
      f.write(tab(4)+"<face v1=\"%d\" v2=\"%d\" v3=\"%d\"/>\n" % (v1, v2, v3))
    f.write(tab(3)+"</faces>\n")

    f.write(tab(3)+"<geometry vertexcount=\"%d\">\n" % len(submesh.vertices))

    f.write(tab(4)+"<vertexbuffer ")
    f.write("positions=\"true\" ")
    f.write("normals=\"true\" ")
    if submesh.material.texture:
       f.write("texture_coord_dimensions_0=\"2\" texture_coords=\"1\"")
    f.write(">\n")
    
    for v in submesh.vertices:
      f.write(tab(5)+"<vertex>\n")

      f.write(tab(6)+"<position x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n"
              % (v.loc[0], v.loc[1], v.loc[2]))

      f.write(tab(6)+"<normal x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n"
              % (v.normal[0], v.normal[1], v.normal[2]))

      if submesh.material.texture:
        f.write(tab(6)+"<texcoord u=\"%.6f\" v=\"%.6f\"/>\n"
                % (v.uvmaps[0].u, v.uvmaps[0].v))

      f.write(tab(5)+"</vertex>\n")

    f.write(tab(4)+"</vertexbuffer>\n")
    f.write(tab(3)+"</geometry>\n")

    if skeleton:
      f.write(tab(3)+"<boneassignments>\n")
      for v in submesh.vertices:
        for influence in v.influences:
          f.write(tab(4)+"<vertexboneassignment ")
          f.write("vertexindex=\"%d\" boneindex=\"%d\" weight=\"%.6f\"/>\n"
                  % (v.id, influence.bone.id, influence.weight))
      f.write(tab(3)+"</boneassignments>\n")

    f.write(tab(2)+"</submesh>\n")        

  f.write(tab(1)+"</submeshes>\n")

  if skeleton:
    f.write(tab(1)+"<skeletonlink name=\"%s.skeleton\"/>\n" % skeleton.name) 

  f.write(tab(0)+"</mesh>\n")    
  f.close()


def write_materials():
  file = MATERIAL_FILE
  print "materials \"%s\"" % file

  f = open(os.path.join(SAVE_DIR, file), "w")
  for name, material in materialsDict.items():
    f.write("%s\n" % name)
    f.write("{\n");

    mat = material.mat
    if mat:
      if mat.amb > 0.0:
        ambR = mat.amb * mat.rgbCol[0]
        ambG = mat.amb * mat.rgbCol[1]
        ambB = mat.amb * mat.rgbCol[2]
        f.write(tab(1)+"ambient %f %f %f\n" % (ambR, ambG, ambB))

      diffR = mat.rgbCol[0]
      diffG = mat.rgbCol[1]
      diffB = mat.rgbCol[2]
      f.write(tab(1)+"diffuse %f %f %f\n" % (diffR, diffG, diffB))
            
      if mat.spec > 0.0:
        specR = clamp(mat.spec * mat.specCol[0])
        specG = clamp(mat.spec * mat.specCol[1])
        specB = clamp(mat.spec * mat.specCol[2])
        specShine = mat.hard
        f.write(tab(1)+"specular %f %f %f %f\n" % (specR, specG, specB, specShine))

    if material.texture:
      f.write(tab(1)+"{\n")
      f.write(tab(2)+"texture %s\n" % material.texture)
      f.write(tab(1)+"}\n")
      
    f.write("}\n\n");

  f.close()
  
#######################################################################################
## main

def main():
    global materialsDict
    global textureDict
    global skeletonsDict
    global BASE_MATRIX
    
    materialsDict = {}
    textureDict = {}
    skeletonsDict = {}

    # set matrix to 90 degree rotation around x-axis and scale
    rot_mat = matrix_rotate_x(-math.pi / 2.0)
    scale_mat = matrix_scale(SCALE, SCALE, SCALE)
    BASE_MATRIX = matrix_multiply(scale_mat, rot_mat)
    
    if not os.path.exists(SAVE_DIR):
      print "invalid path: "+SAVE_DIR
      return

    objs = Blender.Object.GetSelected()
    print
    print "exporting selected objects:"
    n = 0
    for obj in objs:
      if not obj:
        continue
      
      if obj.getType() == "Mesh":
        export_mesh(obj)
        n = 1

      elif obj.getType() == "Armature":
        export_skeleton(obj)

    if n == 0:
        print "no mesh objects selected!"

    if len(materialsDict):
      write_materials()

    print
    print "finished."
main()
      
