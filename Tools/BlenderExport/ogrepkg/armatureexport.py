"""Armature and armature animation export classes.

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

import Blender
import Blender.Mathutils
from Blender.Mathutils import *

class ArmatureAction:
	"""Manages a Blender action.
	"""
	def __init__(self, bAction, armatureExporter):
		self.bAction = bAction
		self.armatureExporter = armatureExporter
		return
	def getName(self):
		return self.bAction.getName()
	def hasEffect(self):
		"""If true, the action has an effect on at least one bone of the armature.
		"""
		hasEffect = 0
		channelIpoDict = self.bAction.getAllChannelIpos()
		channelIterator = iter(channelIpoDict)
		try:
			while not(hasEffect):
				channelName = channelIterator.next()
				if ((channelName in self.armatureExporter.boneIndices.keys())
					and channelIpoDict[channelName].getNcurves()):
					hasEffect = 1
		except StopIteration:
			pass
		return hasEffect
	
class ArmatureActionManager:
	def __init__(self, armatureExporter):
		self.armatureExporter = armatureExporter
		self.actionList = []
		for bAction in Blender.Armature.NLA.GetActions().values():
			action = ArmatureAction(bAction, self.armatureExporter)
			if action.hasEffect():
				self.actionList.append(action)
		return
	def __iter__(self):
		return ArmatureActionManager.Iterator(self)
	def getActions(self):
		return self.actionList
	class Iterator:
		"""Iterates over ArmatureActions.
		"""
		def __init__(self, armatureActionManager):
			self.armatureActionManager = armatureActionManager
			self.listIndex = 0
			return
		def next(self):
			if self.listIndex >= len(self.armatureActionManager.actionList):
				raise StopIteration
			self.listIndex = self.listIndex + 1
			return self.armatureActionManager.actionList[self.listIndex - 1]

class ArmatureAnimation:
	"""Resembles Blender's action actuators.
	"""
	def __init__(self, armatureAction, name, startFrame, endFrame):
		"""Constructor
		   
		   @param armatureAction ArmatureAction object of the animation
		   @param name           Animation name
		   @param startFrame     first frame of the animation
		   @param endFrame       last frame of the animation
		"""
		self.armatureAction = armatureAction
		self.name = name
		self.startFrame = startFrame
		self.endFrame = endFrame
		return
	def export(self):
		print "DEBUG"
		print "name = ", self.name
		print "action = ", self.armatureAction.getName()
		print "start = ", self.startFrame
		print "end = ", self.endFrame
		return

class SkeletonBone:
	"""Bone of an Ogre sekeleton.
	"""
	def __init__(self):
		return

class ArmatureExporter:
	"""Exports Blender armature and its animations.
	"""
	def __init__(self, bMeshObject, bArmatureObject):
		"""Constructor.
		"""
		# Note: getName() and getBoneIndex(boneName) already work prior to export.
		self.bMeshObject = bMeshObject
		self.bArmatureObject = bArmatureObject
		# cache Blender Armature
		self.bArmature = bArmatureObject.getData()
		# name, needed as mesh's skeletonlink name
		self.name = self.bMeshObject.getName() + "-" + self.bArmatureObject.getData(True)
		# boneindices, needed for mesh's vertexboneassignments
		# key = boneName, value = boneIndex
		boneNameList = self.bArmature.bones.keys()
		self.boneIndices = dict(zip(boneNameList, range(len(boneNameList))))
		# actions
		self.actionManager = ArmatureActionManager(self)
		# animations to export
		self.actionActuatorList = []
		# populated on export
		self.skeletonBoneList = None
		return
	def addAnimation(self, animation):
		"""Adds animation to export.
		
		   @param animation ArmatureAnimation
		"""
		return
	def export(self, dir, parentTransform):
		#TODO
		return
	def getName(self):
		return self.name
	def getBoneIndex(self, boneName):
		"""Returns bone index for a given bone name.
		
		   @param boneName Name of the bone.
		   @return Bone index or <code>None</code> if a bone with the given name does not exist.
		"""
		if self.boneIndices.has_key(boneName):
			index = self.boneIndices[boneName]
		else:
			index = None
		return index
	def getActions(self):
		"""Returns list of available actions.
		"""
		return self.actionManager.getActions()
	def write(self, dir):
		Log.getSingleton().logInfo("Exporting armature \"%s\"" % self.getName())
		f = open(Blender.sys.join(dir, self.getName() + ".skeleton.xml"), "w")
		f.write(indent(0) + "<skeleton>\n")
		self._writeRestpose(f, 1)
		self._writeBoneHierarchy(f, 1)
		self._writeAnimations(f, 1)
		f.write(indent(0) + "</skeleton>\n")
		f.close()
		return
	def _generateActionList(self):
		return
	def _convertRestpose(self):
		self.skeletonBoneList = []
		return
	def _writeRestpose(self, f, indent):
		return
	def _writeBoneHierarchy(self, f, indent):
		return
	def _convertAnimations(self):
		return
	def _writeAnimations(self, f, indent):
		return
