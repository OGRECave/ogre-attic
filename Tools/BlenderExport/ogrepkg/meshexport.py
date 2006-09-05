"""Mesh and animation export classes.

   @author Michael Reimpell
"""
# Copyright (C) 2005  Michael Reimpell
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

# epydoc doc format
__docformat__ = "javadoc en"

import base
from base import *
import materialexport
from materialexport import *
import armatureexport
from armatureexport import *

import Blender
import Blender.Mathutils
from Blender.Mathutils import *
import math

# OGRE_VERTEXCOLOUR_BGRA
#  workaround for Ogre's vertex colour conversion bug.
#  Set to 0 for RGBA, 1 for BGRA.
OGRE_OPENGL_VERTEXCOLOUR = 1

matrixOne = [[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]]

class Vertex:
	"""
	"""
	THRESHOLD = 1e-6
	def __init__(self, bMesh, bMFace, bIndex, index, parentTransform, armatureExporter=None):
		"""Represents an Ogre vertex.
		   
		   @param bIndex Index in the vertex list of the NMFace.
		   @param index Vertexbuffer position.
		   @param parentTransform Additional transformation to apply to the vertex.
		"""
		self.bMesh = bMesh
		# imples position
		# vertex in basis shape
		self.bMVert = bMFace.v[bIndex]
		bKey = self.bMesh.getKey()
		if (bKey and len(bKey.blocks)):
			# first shape key is rest position
			self.bMVert = bKey.blocks[0].data[self.bMVert.index]
		## Face properties in Blender
		self.normal = None
		self.colourDiffuse = None
		self.texcoord = None
		## bookkeeping
		# vertexbuffer position in vertexbuffer
		self.index = index
		self.parentTransform = parentTransform
		# implies influences
		self.armatureExporter = armatureExporter
		### populated attributes
		## normal
		if bMFace.smooth:
			# key blocks don't have normals
			self.normal = self._applyParentTransform(bMFace.v[bIndex].no)
		else:
			# create face normal
			# 1 - 2
			# | /
			# 3
			# n = (v_3 - v_1) x (v_2 - v_1)/||n||
			if (bKey and len(bKey.blocks)):
				# first shape key is rest position
				blockData = bKey.blocks[0].data
				v1 = self._applyParentTransform(blockData[bMFace.v[0].index].co)
				v2 = self._applyParentTransform(blockData[bMFace.v[1].index].co)
				v3 = self._applyParentTransform(blockData[bMFace.v[2].index].co)
			else:
				# self.normal = CrossVecs(bMFace.v[1].co - bMFace.v[0].co, bMFace.v[2].co - bMFace.v[0].co)
				v1 = self._applyParentTransform(bMFace.v[0].co)
				v2 = self._applyParentTransform(bMFace.v[1].co)
				v3 = self._applyParentTransform(bMFace.v[2].co)
			self.normal = CrossVecs(v2 - v1, v3 - v1)
		# self.normal.normalize() does not throw ZeroDivisionError exception
		normalLength = self.normal.length
		if (normalLength > Vertex.THRESHOLD):
			self.normal = Vector([coordinate/normalLength for coordinate in self.normal])
		else:
			Log.getSingleton().logWarning("Error in normalize! Face of mesh \"%s\" too small." % bMesh.name)
			self.normal = Vector([0,0,0])
		## colourDiffuse
		#Mesh#if bMesh.vertexColors:
		if bMesh.hasVertexColours():
			bMCol = bMFace.col[bIndex]
			if OGRE_OPENGL_VERTEXCOLOUR:
				self.colourDiffuse = (bMCol.b/255.0, bMCol.g/255.0, bMCol.r/255.0, bMCol.a/255.0)
			else:
				self.colourDiffuse = (bMCol.r/255.0, bMCol.g/255.0, bMCol.b/255.0, bMCol.a/255.0)
		## texcoord
		# origin in Ogre is top-left
		#Mesh#if bMesh.faceUV:
		if bMesh.hasFaceUV():
			self.texcoord = (bMFace.uv[bIndex][0], 1 - bMFace.uv[bIndex][1])
		#Mesh#elif bMesh.vertexUV:
		elif bMesh.hasVertexUV():
			self.texcoord = (self.bMVert.uvco[0], 1 - self.bMVert.uvco[1])
		return
	def __eq__(self, other):
		"""Tests if this vertex is equal to another vertex in the Ogre sense.
		
		   Does no take parentTransform into account!
		   Also, it does not compare the index.
		"""
		isEqual = 0
		# compare index, normal, colourDiffuse and texcoord
		if (self.bMVert.index == other.bMVert.index):
			pass
		elif ((self.normal - other.normal).length > Vertex.THRESHOLD):
			pass
		elif ((self.texcoord and not(other.texcoord)) or 
			(not(self.texcoord) and other.texcoord)):
			pass
		elif ((math.fabs(self.texcoord[0] - other.texcoord[0]) > Vertex.THRESHOLD)
			or (math.fabs(self.texcoord[1] - other.texcoord[1]) > Vertex.THRESHOLD)):
			pass
		elif ((self.colourDiffuse and not(other.colourDiffuse)) or 
			(not(self.colourDiffuse) and other.colourDiffuse)):
			pass
		elif ((math.fabs(self.colourDiffuse[0] - other.colourDiffuse[0]) > Vertex.THRESHOLD)
			or (math.fabs(self.colourDiffuse[1] - other.colourDiffuse[1]) > Vertex.THRESHOLD)
			or (math.fabs(self.colourDiffuse[2] - other.colourDiffuse[2]) > Vertex.THRESHOLD)
			or (math.fabs(self.colourDiffuse[3] - other.colourDiffuse[3]) > Vertex.THRESHOLD)):
			pass
		else:
			isEqual = 1
		return isEqual
	def writePosition(self, fileObject, indentation=0):
		fileObject.write(indent(indentation) + "<position x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" \
			% tuple(self.getPosition()))
		return
	def writeNormal(self, fileObject, indentation=0):
		fileObject.write(indent(indentation) + "<normal x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" \
			% tuple(self.normal))
		return
	def writeColourDiffuse(self, fileObject, indentation=0):
		if self.colourDiffuse:
			fileObject.write(indent(indentation) + "<colour_diffuse value=\"%.6f %.6f %.6f %.6f\"/>\n" \
				% self.colourDiffuse)
		return
	def writeTexcoord(self, fileObject, indentation=0):
		if self.texcoord:
			fileObject.write(indent(indentation) + "<texcoord u=\"%.6f\" v=\"%.6f\"/>\n"\
				% self.texcoord)
		return
	def writeVertex(self, fileObject, indentation=0):
		fileObject.write(indent(indentation) + "<vertex>\n")
		self.writePosition(fileObject, indentation + 1)
		self.writeNormal(fileObject, indentation + 1)
		self.writeColourDiffuse(fileObject, indentation + 1)
		self.writeTexcoord(fileObject, indentation + 1)
		fileObject.write(indent(indentation) + "</vertex>\n")
		return
	def writeBoneAssignments(self, fileObject, indentation=0):
		nAssignments = 0
		weightSum = 0
		for groupName in self.bMesh.getVertGroupNames():
			try:
				weight = self.bMesh.getVertsFromGroup(groupName, 1, [self.bMVert.index])[0][1]
			except IndexError:
				# vertex not in group groupName
				pass
			else:
				boneIndex = self.armatureExporter.getBoneIndex(groupName)
				if boneIndex is not None:
					# group belongs to a bone in the mesh's armature
					fileObject.write(indent(indentation) + \
						"<vertexboneassignment vertexindex=\"%d\" boneindex=\"%d\" weight=\"%.6f\"/>\n" \
						% (self.index, boneIndex, weight))
					nAssignments += 1
					weightSum += weight
		# warnings
		if (nAssignments == 0):
			Log.getSingleton().logWarning("Vertex without bone assignment!")
		elif (nAssignments  > 4):
			Log.getSingleton().logWarning("Vertex with more than 4 bone assignments!")
		if (weightSum > 1.0):
			Log.getSingleton().logWarning("Vertex with sum of bone assignment weights > 1!")
		return
	def getIndex(self):
		return self.index
	def getMVert(self):
		return self.bMVert
	def getPosition(self):
		"""Returns position vector of the rest position.
		"""
		return self._applyParentTransform(self.bMVert.co)
	def getCurrentFramePosition(self, bDeformedNMesh):
		"""Returns position of this vertex in the current frame of the possibly deformed mesh.
		"""
		return self._applyParentTransform(bDeformedNMesh.verts[self.bMVert.index].co)
	def getCurrentFrameRelativePosition(self, bDeformedNMesh):
		"""Returns relative position of this vertex in the current frame of the possibly deformed mesh.
		"""
		return (self.getCurrentFramePosition(bDeformedNMesh) - self.getPosition())
	def _applyParentTransform(self, vector):
		"""Applies transformation to threedimensional vector.
		"""
		vec = Vector(vector)
		vec.resize4D()
		vec = vec * self.parentTransform
		vec.resize3D()
		return vec

class VertexManager:
	"""
	"""
	def __init__(self, bMesh, parentTransform, armatureExporter=None):
		self.bMesh = bMesh
		self.parentTransform = parentTransform
		# needed for boneassignments
		self.armatureExporter = armatureExporter
		# key: index, value: list of vertices with same MVert
		self.vertexDict = {}
		# vertices in ascending index order
		self.vertexList = []
		return
	def __iter__(self):
		return VertexManager.Iterator(self)
	def getNumberOfVertices(self):
		"""Returns the current number of vertices.
		"""
		return len(self.vertexList)
	def getVertex(self, bMFace, bIndex):
		"""Returns possibly shared vertex.
		
		   @param bMesh Blender Mesh.
		   @param bMFace Blender Face.
		   @param bIndex Index in the vertex list of the MFace.
		   @return Corresponding vertex.
		"""
		vertex = Vertex(self.bMesh, bMFace, bIndex, len(self.vertexList), self.parentTransform, self.armatureExporter)
		if self.vertexDict.has_key(bMFace.v[bIndex].index):
			# check Ogre vertices for that Blender vertex
			vertexList = self.vertexDict[bMFace.v[bIndex].index]
			found = 0
			listIndex = 0
			while (not(found) and (listIndex < len(vertexList))):
				if (vertex == vertexList[listIndex]):
					vertex = vertexList[listIndex]
					found = 1
				listIndex = listIndex + 1
			if not(found):
				# create Ogre vertex for that Blender vertex
				self.vertexDict[bMFace.v[bIndex].index].append(vertex)
				self.vertexList.append(vertex)
		else:
			# create Ogre vertex for that Blender vertex
			self.vertexDict[bMFace.v[bIndex].index] = [vertex]
			self.vertexList.append(vertex)
		return vertex
	def writeGeometry(self, fileObject, indentation=0):
		fileObject.write(indent(indentation) + "<geometry vertexcount=\"%d\">\n" % len(self.vertexList))
		# separate position vertexbuffer for vertex animation
		# == position vertexbuffer ==
		fileObject.write(indent(indentation + 1) + "<vertexbuffer positions=\"true\">\n")
		for vertex in self.vertexList:
			fileObject.write(indent(indentation + 2) + "<vertex>\n")
			vertex.writePosition(fileObject, indentation + 3)
			fileObject.write(indent(indentation + 2) + "</vertex>\n")
		fileObject.write(indent(indentation + 1) + "</vertexbuffer>\n")
		# == vertex buffer for rest ==
		fileObject.write(indent(indentation + 1) + "<vertexbuffer normals=\"true\"")
		## optional attributes
		#Mesh#if self.bMesh.vertexColors:
		if self.bMesh.hasVertexColours():
			fileObject.write(" colours_diffuse=\"true\"")
		#Mesh#if (self.bMesh.faceUV or self.bMesh.vertexUV):
		if (self.bMesh.hasFaceUV() or self.bMesh.hasVertexUV()):
			fileObject.write(" texture_coords=\"1\" texture_coord_dimensions_0=\"2\"")
		fileObject.write(">\n")
		for vertex in self.vertexList:
			fileObject.write(indent(indentation + 2) + "<vertex>\n")
			vertex.writeNormal(fileObject, indentation + 3)
			vertex.writeColourDiffuse(fileObject, indentation + 3)
			vertex.writeTexcoord(fileObject, indentation + 3)
			fileObject.write(indent(indentation + 2) + "</vertex>\n")
		fileObject.write(indent(indentation + 1) + "</vertexbuffer>\n")
# == single vertexbuffer ==
#		fileObject.write(indent(indentation + 1) + "<vertexbuffer positions=\"true\" normals=\"true\"")
#		## optional attributes
#		#NMesh#if self.bMesh.vertexColors:
#		if self.bMesh.hasVertexColours():
#			fileObject.write(" colours_diffuse=\"true\"")
#		#NMesh#if (self.bMesh.faceUV or self.bMesh.vertexUV):
#		if (self.bMesh.hasFaceUV() or self.bMesh.hasVertexUV()):
#			fileObject.write(" texture_coords=\"1\" texture_coord_dimensions_0=\"2\"")
#		fileObject.write(">\n")
#		for vertex in self.vertexList:
#			vertex.writeVertex(fileObject, indentation + 2)
#		fileObject.write(indent(indentation + 1) + "</vertexbuffer>\n")
		fileObject.write(indent(indentation) + "</geometry>\n")
		return
	def writeBoneAssignments(self, fileObject, indentation=0):
		if self.armatureExporter:
			fileObject.write(indent(indentation) + "<boneassignments>\n")
			for vertex in self.vertexList:
				vertex.writeBoneAssignments(fileObject, indentation + 1)
			fileObject.write(indent(indentation) + "</boneassignments>\n")
		return
	class Iterator:
		"""Iterates over vertices in ascending index order.
		"""
		def __init__(self, vertexManager):
			self.vertexManager = vertexManager
			self.listIndex = 0
			return
		def next(self):
			if self.listIndex >= len(self.vertexManager.vertexList):
				raise StopIteration
			self.listIndex = self.listIndex + 1
			return self.vertexManager.vertexList[self.listIndex - 1]

class Submesh:
	"""Ogre submesh.
	"""
	def __init__(self, bMesh, material, index, parentTransform, armatureExporter = None):
		"""Constructor.
		
		   @param index Index of submesh in submeshes list.
		"""
		self.bMesh = bMesh
		self.materialName = material.getName()
		self.index = index
		self.parentTransform = parentTransform
		self.armatureExporter = armatureExporter
		self.vertexManager = VertexManager(self.bMesh, self.parentTransform, self.armatureExporter)
		# list of (tuple of vertice indices)
		self.faces =[]
		return
	def getIndex(self):
		return self.index
	def addFace(self, bMFace):
		"""Adds a Blender face to the submesh.
		"""
		# vertex winding:
		# Blender: clockwise, Ogre: clockwise
		if (len(bMFace.v) == 3):
			v1 = self.vertexManager.getVertex(bMFace, 0)
			v2 = self.vertexManager.getVertex(bMFace, 1)
			v3 = self.vertexManager.getVertex(bMFace, 2)
			self.faces.append((v1.getIndex(), v2.getIndex(), v3.getIndex()))
		elif (len(bMFace.v) == 4):
			v1 = self.vertexManager.getVertex(bMFace, 0)
			v2 = self.vertexManager.getVertex(bMFace, 1)
			v3 = self.vertexManager.getVertex(bMFace, 2)
			v4 = self.vertexManager.getVertex(bMFace, 3)
			# Split face on shortest edge
			if ((v3.getPosition() - v1.getPosition()).length < (v4.getPosition() - v2.getPosition()).length):
				# 1 - 2
				# | \ |
				# 4 - 3
				self.faces.append((v1.getIndex(), v2.getIndex(), v3.getIndex()))
				self.faces.append((v1.getIndex(), v3.getIndex(), v4.getIndex()))
			else:
				# 1 - 2
				# | / |
				# 4 _ 3
				self.faces.append((v1.getIndex(), v2.getIndex(), v4.getIndex()))
				self.faces.append((v2.getIndex(), v3.getIndex(), v4.getIndex()))
		else:
			Log.getSingleton().logWarning("Ignored face with %d edges." % len(bMFace.v))
		return
	def getVertexManager(self):
		return self.vertexManager
	def write(self, fileObject, indentation=0):
		fileObject.write(indent(indentation) + "<submesh")
		## attributes
		fileObject.write(" material=\"%s\"" % self.materialName)
		fileObject.write(" usesharedvertices=\"false\"")
		if (self.vertexManager.getNumberOfVertices() > 65535):
			fileObject.write(" use32bitindexes=\"true\"")
			Log.getSingleton().logInfo("Switched to 32 bit indices for submesh \"%s\"!" % self.materialName)
		fileObject.write(">\n")
		## elements
		self._writeFaces(fileObject, indentation + 1)
		self.vertexManager.writeGeometry(fileObject, indentation + 1)
		self.vertexManager.writeBoneAssignments(fileObject, indentation + 1)
		fileObject.write(indent(indentation) + "</submesh>\n")
		return
	def _writeFaces(self, fileObject, indentation):
		fileObject.write(indent(indentation) + "<faces")
		## attributes
		fileObject.write(" count=\"%d\"" % len(self.faces))
		fileObject.write(">\n")
		## elements
		for face in self.faces:
			fileObject.write(indent(indentation + 1) + "<face v1=\"%d\" v2=\"%d\" v3=\"%d\"/>\n" % face)
		fileObject.write(indent(indentation) + "</faces>\n")
		return

class SubmeshManager:
	"""
	"""
	def __init__(self, bMesh, parentTransform, armatureExporter=None):
		self.bMesh = bMesh
		self.parentTransform = parentTransform
		self.armatureExporter = armatureExporter
		# key: material name, value: Submesh
		self.submeshDict = {}
		# submeshes in ascending index order
		self.submeshList = []
		return
	def __iter__(self):
		return SubmeshManager.Iterator(self)
	def getSubmesh(self, material):
		"""Returns a Submesh for that material.
		"""
		submesh = None
		if self.submeshDict.has_key(material.getName()):
			submesh = self.submeshDict[material.getName()]
		else:
			# return new Submesh
			index = len(self.submeshList)
			submesh = Submesh(self.bMesh, material, index, self.parentTransform, self.armatureExporter)
			self.submeshDict[material.getName()] = submesh
			self.submeshList.append(submesh)
		return submesh
	def write(self, fileObject, indentation=0):
		if len(self.submeshList):
			fileObject.write(indent(indentation) + "<submeshes>\n")
			for submesh in self.submeshList:
				submesh.write(fileObject, indentation + 1)
			fileObject.write(indent(indentation) + "</submeshes>\n")
		return
	class Iterator:
		"""Iterates over submeshes in ascending index order.
		"""
		def __init__(self, submeshManager):
			self.submeshManager = submeshManager
			self.listIndex = 0
			return
		def next(self):
			if self.listIndex >= len(self.submeshManager.submeshList):
				raise StopIteration
			self.listIndex = self.listIndex + 1
			return self.submeshManager.submeshList[self.listIndex - 1]

class Pose:
	"""
	"""
	THRESHOLD = 1e-7
	def __init__(self, bKeyBlock, submesh, index, parentTransform):
		"""Constructor.
		
		   @param index Index of pose in poses list.
		"""
		self.bKeyBlock = bKeyBlock
		self.submesh = submesh
		self.index = index
		self.parentTransform = parentTransform
		# list of pose offset tuples (vertexIndex, deltaX, deltaY, deltaZ)
		self.poseoffsetList	= []
		# calculate poseoffsets
		poseVertexList = self.bKeyBlock.data
		for vertex in self.submesh.getVertexManager():
			offset = self._applyParentTransform(poseVertexList[vertex.getMVert().index]) \
				 - vertex.getPosition()
			if (offset.length > Pose.THRESHOLD):
				self.poseoffsetList.append((vertex.getIndex(), offset.x, offset.y, offset.z))
		return
	def getIndex(self):
		return self.index
	def getInfluence(self):
		"""Returns influence of this pose in the current frame.
		"""
		return self.bKeyBlock.curval
	def getName(self):
		# unique name = KeyBlock name + submesh index
		return self.bKeyBlock.name + "-" + str(self.submesh.getIndex())
	def nPoseoffsets(self):
		return len(self.poseoffsetList)
	def write(self, fileObject, indentation=0):
		if len(self.poseoffsetList):
			fileObject.write(indent(indentation) + \
				"<pose target=\"submesh\" index=\"%d\" name=\"%s\">\n" \
				% (self.submesh.getIndex(), self.getName()))
			for poseoffset in self.poseoffsetList:
				fileObject.write(indent(indentation + 1) + \
					"<poseoffset index=\"%d\" x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" \
					% poseoffset)
			fileObject.write(indent(indentation) + "</pose>\n")		
		return
	def _applyParentTransform(self, vector):
		"""Applies transformation to threedimensional vector.
		"""
		vec = Vector(vector)
		vec.resize4D()
		vec = vec * self.parentTransform
		vec.resize3D()
		return vec
	
class PoseManager:
	"""
	"""
	def __init__(self, bMesh, submeshManager, parentTransform):
		self.bMesh = bMesh
		self.submeshManager = submeshManager
		self.parentTransform = parentTransform
		# key: submesh, value: poseList
		self.poseListDict = {}
		self.poseList = []
		# create poses
		# each keyblock creates a pose for every submesh
		bKey = self.bMesh.getKey()
		if bKey:
			for bKeyBlock in bKey.blocks:
				for submesh in self.submeshManager:
					index = len(self.poseList)
					pose = Pose(bKeyBlock, submesh, index, self.parentTransform)
					if (pose.nPoseoffsets() > 0):
						# add nonempty pose to list and dict
						self.poseList.append(pose)
						if self.poseListDict.has_key(submesh):
							self.poseListDict[submesh].append(pose)
						else:
							self.poseListDict[submesh] = [pose]
		return
	def getPoseList(self, submesh):
		if self.poseListDict.has_key(submesh):
			poseList = self.poseListDict[submesh]
		else:
			poseList = []
		return poseList
	def nPoses(self):
		return len(self.poseList)
	def write(self, fileObject, indentation=0):
		if len(self.poseList):
			fileObject.write(indent(indentation) + "<poses>\n")
			for pose in self.poseList:
				pose.write(fileObject, indentation + 1)
			fileObject.write(indent(indentation) + "</poses>\n")		
		return

class MorphAnimationTrack:
	"""
	"""
	def __init__(self, submesh):
		"""Constructor.
		
		   @param submesh Submesh.
		"""
		self.submesh = submesh
		# key: time, value: list of position in same order as in the VertexManager.
		self.keyframeDict = {}
		return
	def addKeyframe(self, bDeformedNMesh, time):
		"""Append current frame as keyframe at given time.
		"""
		positionList = []
		for vertex in self.submesh.getVertexManager():
			positionList.append(vertex.getCurrentFramePosition(bDeformedNMesh))
		self.keyframeDict[time] = positionList
		return
	def write(self, fileObject, indentation):
		fileObject.write(indent(indentation) + "<track target=\"submesh\" index=\"%d\" type=\"morph\">\n" \
			% self.submesh.getIndex())
		fileObject.write(indent(indentation + 1) + "<keyframes>\n")
		timeList = self.keyframeDict.keys()
		timeList.sort()
		for time in timeList:
			fileObject.write(indent(indentation + 2) + "<keyframe time=\"%.6f\">\n" % time)
			for position in self.keyframeDict[time]:
				fileObject.write(indent(indentation + 3) + "<position x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" \
					% tuple(position))
			fileObject.write(indent(indentation + 2) + "</keyframe>\n")
		fileObject.write(indent(indentation + 1) + "</keyframes>\n")
		fileObject.write(indent(indentation) + "</track>\n")
		return

class PoseAnimationTrack:
	"""Track with a single pose as keyframes.
	"""
	THRESHOLD = 1e-7
	def __init__(self, submesh, poseManager):
		"""Constructor.
		
		   @param submesh Submesh.
		"""
		self.submesh = submesh
		self.poseManager = poseManager
		# key: time, value: list of poseref tuples (poseindex, influence).
		self.keyframeDict = {}
		return
	def nKeyframes(self):
		return len(self.keyframeDict)
	def addKeyframe(self, time):
		for pose in self.poseManager.getPoseList(self.submesh):
			if (pose.getInfluence() > PoseAnimationTrack.THRESHOLD):
				poseref = (pose.getIndex(), pose.getInfluence())
				if self.keyframeDict.has_key(time):
					self.keyframeDict[time].append(poseref)
				else:
					self.keyframeDict[time] = [poseref]
		return
	def write(self, fileObject, indentation):
		fileObject.write(indent(indentation) + \
			"<track target=\"submesh\" index=\"%d\" type=\"pose\">\n" \
			% self.submesh.getIndex())
		fileObject.write(indent(indentation + 1) + "<keyframes>\n")
		timeList = self.keyframeDict.keys()
		timeList.sort()
		for time in timeList:
			fileObject.write(indent(indentation + 2) + "<keyframe time=\"%.6f\">\n" % time)
			for poseref in self.keyframeDict[time]:
				fileObject.write(indent(indentation + 3) + \
					"<poseref poseindex=\"%d\" influence=\"%.6f\"/>\n" \
					% poseref)
			fileObject.write(indent(indentation + 2) + "</keyframe>\n")
		fileObject.write(indent(indentation + 1) + "</keyframes>\n")
		fileObject.write(indent(indentation) + "</track>\n")
		return

class VertexAnimation:
	"""Animation base class.
	"""
	def __init__(self, name, startFrame, endFrame):
		self.name = name
		self.startFrame = startFrame
		self.endFrame = endFrame
		## populated on export
		self.length = None
		# same order as submeshList of the SubmeshManager
		self.trackList = None
		return
	def getName(self):
		return self.name
	def write(self, fileObject, indentation=0):
		if (len(self.trackList) > 0):
			fileObject.write(indent(indentation) + "<animation name=\"%s\" length = \"%.6f\">\n" \
				% (self.name, self.length))
			fileObject.write(indent(indentation + 1) + "<tracks>\n")
			for track in self.trackList:
				track.write(fileObject, indentation + 2)
			fileObject.write(indent(indentation + 1) + "</tracks>\n")
			fileObject.write(indent(indentation) + "</animation>\n")
		else:
			Log.getSingleton().logWarning("Skipped animation \"%s\" as it has no tracks!" \
				% self.name)
		return
	def _createFrameNumberDict(self):
		## frames to times
		self.length = 0
		fps = Blender.Scene.GetCurrent().getRenderingContext().framesPerSec()
		# frameNumberDict: key = export time, value = frame number
		frameNumberDict = {}
		if (self.startFrame <= self.endFrame):
			minFrame = self.startFrame
			maxFrame = self.endFrame
		else:
			minFrame = self.endFrame
			maxFrame = self.startFrame
		for frameNumber in range(int(minFrame), int(maxFrame+1)):
			if  (self.startFrame <= self.endFrame):
				time = float(frameNumber-self.startFrame)/fps
			else:
				# backward animation
				time = float(self.endFrame-frameNumber)/fps
			# update animation duration
			if self.length < time:
				self.length = time
			frameNumberDict[time] = frameNumber
		return frameNumberDict

class MorphAnimation(VertexAnimation):
	"""Morph animation.
	"""
	def export(self, bObject, submeshManager):
		Log.getSingleton().logInfo("Exporting morph animation \"%s\" of mesh \"%s\"" % (self.name, bObject.getData(True)))
		## submeshes to tracks
		self.trackList = []
		for submesh in submeshManager:
			self.trackList.append(MorphAnimationTrack(submesh))
		## frames to times
		frameNumberDict = self._createFrameNumberDict()
		## export
		timeList = frameNumberDict.keys()
		timeList.sort()
		for time in timeList:
			Blender.Set('curframe', frameNumberDict[time])
			bDeformedNMesh = Blender.NMesh.GetRawFromObject(bObject.getName())
			for track in self.trackList:
				track.addKeyframe(bDeformedNMesh, time)
		return
	
class PoseAnimation(VertexAnimation):
	"""Pose animation.
	"""
	def export(self, bObject, submeshManager, poseManager):
		Log.getSingleton().logInfo("Exporting pose animation \"%s\" of mesh \"%s\"" % (self.name, bObject.getData(True)))
		## submeshes to tracks
		self.trackList = []
		trackList = []
		for submesh in submeshManager:
			trackList.append(PoseAnimationTrack(submesh, poseManager))
		## frames to times
		frameNumberDict = self._createFrameNumberDict()
		## export
		timeList = frameNumberDict.keys()
		timeList.sort()
		for time in timeList:
			Blender.Set('curframe', frameNumberDict[time])
			for track in trackList:
				track.addKeyframe(time)
		for track in trackList:
			if (track.nKeyframes() > 0):
				self.trackList.append(track)
		# at least one track?
		if (len(self.trackList) == 0):
			# no pose offsets
			Log.getSingleton().logWarning("Pose animation \"%s\" does not differ from restpose." % self.name)
		return

class VertexAnimationExporter:
	"""
	"""
	def __init__(self, meshExporter):
		self.meshExporter = meshExporter
		self.morphAnimationList = []
		self.poseAnimationList = []
		self.poseManager = None
		return
	def addMorphAnimation(self, morphAnimation):
		"""Adds a morph animation.
		"""
		self.morphAnimationList.append(morphAnimation)
		return
	def addPoseAnimation(self, poseAnimation):
		"""Adds a pose for pose animation.
		"""
		self.poseAnimationList.append(poseAnimation)
		return
	def hasAnimation(self):
		return (len(self.morphAnimationList) or len(self.poseAnimationList))
	def export(self, parentTransform):
		# generate poses
		self.poseManager = PoseManager(self.meshExporter.getObject().getData(), self.meshExporter.getSubmeshManager(), parentTransform)
		if self.hasAnimation():
			# sample animations
			animationNameList = []
			bCurrentFrame = Blender.Get('curframe')
			if len(self.poseAnimationList):
				# pose animations
				if (self.poseManager.nPoses() > 0):
					for poseAnimation in self.poseAnimationList:
						# warn on pose animation name clash
						animationName = poseAnimation.getName()
						if animationName in animationNameList:
							Log.getSingleton().logWarning("Duplicate animation name \"%s\" for mesh \"%s\"!" \
								% (animationName, self.meshExporter.getName()))
						animationNameList.append(animationName)
						# export
						poseAnimation.export(self.meshExporter.getObject(), self.meshExporter.getSubmeshManager(), self.poseManager)
				else:
					Log.getSingleton().logWarning("Skipped pose animation export as mesh \"%s\"has no shape keys!" \
						% self.meshExporter.getName())
					# clear poseAnimationList to prevent writing
					self.poseAnimationList = []
				if len(self.morphAnimationList):
					# morph and pose animation cannot share the same vertex data
					Log.getSingleton().logError("Skipping morph animations of mesh \"%s\": Cannot share vertex data with pose animation!"
						% self.meshExporter.getName())
					self.morphAnimationList = []
			elif len(self.morphAnimationList):
				# morph animations
				for morphAnimation in self.morphAnimationList:
					# warn on morph animation name clash
					animationName = morphAnimation.getName()
					if animationName in animationNameList:
						Log.getSingleton().logWarning("Duplicate animation name \"%s\" for mesh \"%s\"!" \
							% (animationName, self.meshExporter.getName()))
					animationNameList.append(animationName)
					# export
					morphAnimation.export(self.meshExporter.bObject, self.meshExporter.getSubmeshManager())
			Blender.Set('curframe', bCurrentFrame)
		return
	def write(self, fileObject, indentation=0):
		# poses
		self.poseManager.write(fileObject, indentation)
		if (len(self.morphAnimationList) or len(self.poseAnimationList)):
			fileObject.write(indent(indentation) + "<animations>\n")
			if len(self.poseAnimationList):
				# pose animations
				for poseAnimation in self.poseAnimationList:
					poseAnimation.write(fileObject, indentation + 1)
			elif len(self.morphAnimationList):
				# morph animations
				for morphAnimation in self.morphAnimationList:
					morphAnimation.write(fileObject, indentation + 1)
			fileObject.write(indent(indentation) + "</animations>\n")
		return

class MeshExporter:
	"""Exports a Blender mesh to Ogre.
	
	   Exports mesh, armature and animations to Ogre XML resp. script files. Materials are
	   exported to a MaterialManager.
	"""
	def __init__(self, bObject):
		"""
		"""
		# mesh
		self.bObject = bObject
		self.name = self.bObject.getData(True)
		# vertex animations
		self.vertexAnimationExporter = VertexAnimationExporter(self)
		# skeleton
		self.armatureExporter = None
		parent = self.bObject.getParent()
		if (parent and (parent.getType() == "Armature")):
			self.armatureExporter = ArmatureExporter(self.bObject, parent)
		# populated on export
		self.submeshManager = None
		return
	def export(self, dir, materialManager=MaterialManager(), parentTransform=Matrix(*matrixOne), colouredAmbient=False, gameEngineMaterials=False, convertXML=False):
		# leave editmode
		editmode = Blender.Window.EditMode()
		if editmode:
			Blender.Window.EditMode(0)
		Log.getSingleton().logInfo("Exporting mesh \"%s\"" % self.getName())
		## export possible armature
		if self.armatureExporter:
			self.armatureExporter.export(dir, parentTransform)
		## export meshdata
		self._generateSubmeshes(parentTransform, materialManager, colouredAmbient, gameEngineMaterials)
		## export vertex animations
		self.vertexAnimationExporter.export(parentTransform)
		## write files
		self._write(dir, convertXML)
		## cleanup
		self.submeshManager = None
		# reenter editmode
		if editmode:
			Blender.Window.EditMode(1)
		return
	def getObject(self):
		return self.bObject
	def getName(self):
		return self.name
	def getVertexAnimationExporter(self):
		return self.vertexAnimationExporter
	def getArmatureExporter(self):
		return self.armatureExporter
	def getSubmeshManager(self):
		return self.submeshManager
	def _generateSubmeshes(self, parentTransform, materialManager, colouredAmbient, gameEngineMaterials):
		"""Generates submeshes of the mesh.
		"""
		#NMesh# Blender.Mesh.Mesh does not provide access to mesh shape keys, use Blender.NMesh.NMesh
		bMesh = self.bObject.getData()
		self.submeshManager = SubmeshManager(bMesh, parentTransform, self.armatureExporter)
		for bMFace in bMesh.faces:
			faceMaterial = materialManager.getMaterial(bMesh, bMFace, colouredAmbient, gameEngineMaterials)
			if faceMaterial:
				# append face to submesh
				self.submeshManager.getSubmesh(faceMaterial).addFace(bMFace)
		return
	def _write(self, dir, convertXML):
		file = self.getName() + ".mesh.xml"
		Log.getSingleton().logInfo("Writing mesh file \"%s\"" % file)
		fileObject = open(os.path.join(dir, file), "w")
		fileObject.write(indent(0)+"<mesh>\n")
		# submeshes
		self.submeshManager.write(fileObject, 1)
		# skeleton
		if self.armatureExporter:
			fileObject.write(indent(1)+"<skeletonlink name=\"%s.skeleton\"/>\n" % self.armatureExporter.getName())
		# vertex animations
		self.vertexAnimationExporter.write(fileObject, 1)
		fileObject.write(indent(0)+"</mesh>\n")
		fileObject.close()
		if convertXML:
			OgreXMLConverter.getSingleton().convert(os.path.join(dir, file))
		return
