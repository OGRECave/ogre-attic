#!BPY

"""
Name: 'Ogre XML'
Blender: 234
Group: 'Export'
Tooltip: 'Exports selected meshs with armature animations to Ogre3D'
"""

__author__ = ['Michael Reimpell', 'Jens Hoffmann', 'et al.']
__version__ = '0.15.0'
__url__ = ['OGRE website, http://www.ogre3d.org',
	'Script manual, http://www.ogre3d.org/docs/Tutorials/blender/index.html',
	'OGRE forum, http://www.ogre3d.org/phpBB2/']
__bpydoc__ = """\
Exports selected meshs with armature animations to Ogre3D.
"""

# Blender to Ogre Mesh and Skeleton Exporter v0.15.0
# url: http://www.ogre3d.org

# Ogre exporter written by Jens Hoffmann and Michael Reimpell
# based on the Cal3D exporter v0.5 written by Jean-Baptiste LAMY

# Copyright (C) 2004 Michael Reimpell -- <M.Reimpell@tu-bs.de>
# Copyright (C) 2003 Jens Hoffmann -- <hoffmajs@gmx.de>
# Copyright (C) 2003 Jean-Baptiste LAMY -- jiba@tuxfamily.org
#
# ChangeLog:
#   0.7 :  released by Jens Hoffman
#   0.8 :  * Mon Feb 02 2004 Michael Reimpell <M.Reimpell@tu-bs.de>
#          - added GUI
#   0.9 :  * Tue Feb 03 2004 Michael Reimpell <M.Reimpell@tu-bs.de>
#          - added special header to be registered in blenders export menu
#   0.10:  * Wed Feb 04 2004 Michael Reimpell <M.Reimpell@tu-bs.de>
#          - blenders broken Draw.Scrollbar replaced with own class
#          - texture origin changed to top-left (Ogre v0.13.0)
#          - export log is shown in message window
#          - dirty hack for blender 2.32 (does not implement IpoCurve.getName()
#            for action Ipos)
#   0.11:  * Mon Feb 09 2004 Michael Reimpell <M.Reimpell@tu-bs.de>
#          - strip path from texture filenames in material file
#          - back button for doneMessageBox
#          - changed scrollbar focus behaviour
#          - log text position offset
#          - Ogre v0.13.0 material script support
#            Material specific:
#             amb * rgbCol          -> ambient <r> <g> <b> 
#             rgbCol                -> diffuse <r> <g> <b>
#             spec * specCol, hard  -> specular <r> <g> <b> <hard>
#             emit*rgbCol           -> emissive <r> <g> <b>
#             Material.mode
#              ZINVERT              -> depth_func greater_equal
#              ENV                  -> depth_func always_fail
#              SHADELESS            -> lighting off
#              NOMIST               -> fog_override true
#            Face specific:
#             NMFace.mode
#              INVISIBLE            -> no export
#              TEX                  -> texture_unit
#             NMFace.transp
#              SOLID                -> Default: scene_blend one zero
#              ADD                  -> scene_blend add
#              ALPHA                -> scene_blend alpha_blend
#            Texture specific:
#             NMFace.image.filename -> texture <name without path>
#   0.12:  * Mon Feb 16 2004 Michael Reimpell <M.Reimpell@tu-bs.de>
#          - black border flashing removed
#          - added material script support for
#            Material.mode
#              TEXFACE -> disable ambient, diffuse, specular and emissive
#          - exit on ESCKEY or QKEY pressed (not released)
#          - added frame based animation export
#   0.12.1: * Wed Feb 18 2004 Michael Reimpell <M.Reimpell@tu-bs.de>
#          - changed two user interface strings to avoid confusion
#   0.13:  * Mon Feb 23 2004 Michael Reimpell <M.Reimpell@tu-bs.de>
#          - scrollbar marker moves on focus click without MOUSEY event
#          - scrollbar marker focus light
#          - show version number in GUI
#          - transparent load and save of export settings
#   0.13.1: * Wed Feb 25 2004 Michael Reimpell <M.Reimpell@tu-bs.de>
#          - added support for vertices with different uv coordinates but same normal
#          - improved button handling
#          - added support for sticky uv coordinates
#   0.13.2: * Thu Jun 03 2004 Michael Reimpell <M.Reimpell@tu-bs.de>
#          - added warning if no materials or textures are defined
#          - added warning if mesh has no visible faces
#          - displays a message while exporting
#          - added material script support for
#            Material.mode
#              SHADOW -> receive_shadows
#   0.13.3: * Sun Jun 06 2004 Michael Reimpell <M.Reimpell@tu-bs.de>
#          - changed GUI positions to ints to avoid DeprecationWarnings (Blender 2.33)
#          - get frames per second setting from the render buttons (Blender 2.33)
#          - respect new Armature.getBones() behaviour (Blender 2.33)
#          - added missing argument in SkeletonMaterial creation
#          - added option to rotate the coordinate system on export
#   0.14.0: * Sun Jul 04 2004 Michael Reimpell <M.Reimpell@tu-bs.de>
#          - script loadable via command line
#          - Ogre logo added
#          - changed material mapping:
#             amb * World.getAmb() -> ambient <r> <g> <b>
#            where World is the first world returned by Blender.World.Get()
#          - selected objects menu visible even if "Export Armature" option disabled
#          - use Blender.Armature.NLA submodule to access Action objects (Blender 2.33)
#   0.14.1: * Fri Aug 13 2004 Michael Reimpell <M.Reimpell@tu-bs.de>
#          - additional changes due to new Armature.getBones() behaviour (Blender 2.33)
#          - ordering of the additional rotation on export changed
#          - allow actions with less channels than bones
#          - support for dotted parenting in armature edit mode
#          - workaround for quaternion naming bug in Blender 2.34
#          - support for single loc ipo curves
#          - use Blender.World.GetActive() to get the ambient colour of the current world (Blender 2.34)
#          - added mousewheel support to the scrollbar (Blender 2.34)
#          - change keyframes and name of animation according to selected action
#          - use Object.getMatrix("worldspace") (Blender 2.34)
#          - smoothed scrollbar movement and removed flicker
#   0.14.2: * Fri Aug 13 2004 Michael Reimpell <M.Reimpell@tu-bs.de>
#          - location key frame values fixed
#          - fixed redraw if action is changed
#   0.15.0: * Sun Oct 24 2004 Michael Reimpell <M.Reimpell@tu-bs.de>
#          - scalar product range correction in calc_rootaxis
#          - made ArmatureAction.createArmatureActionDict a static method
#          - renamed private methods to begin with an underscore
#          - switched to javadoc comments
#          - changed vertex buffer layout to allow software skinning
#          - settings are now stored inside the .blend file
#          - importing pickle module now optional
#          - support for non-uniform keyframe scaling
#          - export vertex colours
#   0.15.1: * Michael Reimpell <M.Reimpell@tu-bs.de>
#          - use Blender.World.GetCurrent() for Blender > 2.34
#          - fixed calculation of initial bone rotation
#          - preliminary normal map support
#          - option to export in objects local coordinates
#          - changed material file default name to the current scene name
#          - files are now named after their datablock name
#          - path selection starts with current export path
#          - material ambient colour is scaled white
#          - option to use scaled diffuse colour as ambient
#          - BPy documentation added
#          - coloured log
#          - crossplatform path handling
#          - allow empty material list entries
#   0.15.1: * Sun Nov 27 2004 John Bartholomew <johnb213@users.sourceforge.net>
#          - option to run OgreXMLConverter automatically on the exported files
#
# TODO:
#          - vertex colours
#          - code cleanup
#          - noninteractive mode when called from command line
#          - TWOSIDE face mode, TWOSIDED mesh mode
#          - SUBSURF mesh mode
#          - assign unskinned vertices to a static bone
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

# this export script is assumed to be used with the latest blender version.
#
# Usage:
# select the meshes you want to export and run this script (alt-p)

# KEEP_SETTINGS (enable = 1, disable = 0)
#  transparently load and save settings to the text 'ogreexport.cfg'
#  inside the current .blend file.
KEEP_SETTINGS = 1

# OGRE_XML_CONVERTER
#  the command line used to run the OgreXMLConverter tool.
#  Set to '' to disable automatical conversion of XML files.
OGRE_XML_CONVERTER = ''

#######################################################################################
## Code starts here.

# epydoc doc format
__docformat__ = "javadoc en"

######
# imports
######
import Blender, sys, os, math, string
if KEEP_SETTINGS:
	try:
		import pickle
	except ImportError:
		Blender.Draw.PupMenu("Can't import pickle module!%t|Permanent settings disabled.")
		KEEP_SETTINGS = 0

######
# namespaces
######
from Blender import Draw
from Blender import Mathutils
from Blender.BGL import *

######
# Classes
######
class ReplacementScrollbar:
	"""Scrollbar replacement for Draw.Scrollbar
	   <ul>
	   <li> import Blender
	   <li> call eventFilter and buttonFilter in registered callbacks
	   </ul>
	   
	   @author Michael Reimpell
	"""
	def __init__(self, initialValue, minValue, maxValue, buttonUpEvent, buttonDownEvent):
		"""Constructor   
		   
		   @param initialValue    inital value
		   @param minValue        minimum value
		   @param maxValue        maxium value
		   @param buttonUpEvent   unique event number
		   @param buttonDownEvent unique event number
		"""
		self.currentValue = initialValue
		self.minValue = minValue
		if maxValue > minValue:
			self.maxValue = maxValue
		else:
			self.maxValue = self.minValue
			self.minValue = maxValue
		self.buttonUpEvent = buttonUpEvent
		self.buttonDownEvent = buttonDownEvent
		# private
		self.guiRect = [0,0,0,0]
		self.positionRect = [0,0,0,0]
		self.markerRect = [0,0,0,0]
		self.mousePressed = 0
		self.mouseFocusX = 0
		self.mouseFocusY = 0
		self.markerFocusY = 0
		self.mousePositionY = 0
		return
	
	def getCurrentValue(self):
		"""current marker position
		"""
		return self.currentValue
		
	def up(self, steps=1):
		"""move scrollbar up
		"""
		if (steps > 0):
			if ((self.currentValue - steps) > self.minValue):
				self.currentValue -= steps
			else:
				self.currentValue = self.minValue
		return
	
	def down(self, steps=1):
		"""move scrollbar down
		"""
		if (steps > 0):
			if ((self.currentValue + steps) < self.maxValue): 
				self.currentValue += steps
			else:
				self.currentValue = self.maxValue
		return
	
	def draw(self, x, y, width, height):
		"""draw scrollbar
		"""
		# get size of the GUI window to translate MOUSEX and MOUSEY events
		guiRectBuffer = Blender.BGL.Buffer(GL_FLOAT, 4)
		Blender.BGL.glGetFloatv(Blender.BGL.GL_SCISSOR_BOX, guiRectBuffer)
		self.guiRect = [int(guiRectBuffer.list[0]), int(guiRectBuffer.list[1]), \
		                int(guiRectBuffer.list[2]), int(guiRectBuffer.list[3])]
		# relative position
		self.positionRect = [ x, y, x + width, y + height]
		# check minimal size:
		# 2 square buttons,4 pixel borders and 1 pixel inside for inner and marker rectangles
		if ((height > (2*(width+5))) and (width > 2*5)):
			# keep track of remaining area
			remainRect = self.positionRect[:]
			# draw square buttons
			Blender.Draw.Button("/\\", self.buttonUpEvent, x, y + (height-width), width, width, "scroll up") 
			remainRect[3] -=  width + 2
			Blender.Draw.Button("\\/", self.buttonDownEvent, x, y, width, width, "scroll down") 
			remainRect[1] +=  width + 1
			# draw inner rectangle
			Blender.BGL.glColor3f(0.13,0.13,0.13) # dark grey
			Blender.BGL.glRectf(remainRect[0], remainRect[1], remainRect[2], remainRect[3])
			remainRect[0] += 1
			remainRect[3] -= 1
			Blender.BGL.glColor3f(0.78,0.78,0.78) # light grey
			Blender.BGL.glRectf(remainRect[0], remainRect[1], remainRect[2], remainRect[3])
			remainRect[1] += 1
			remainRect[2] -= 1
			Blender.BGL.glColor3f(0.48,0.48,0.48) # grey
			Blender.BGL.glRectf(remainRect[0], remainRect[1], remainRect[2], remainRect[3])
			# draw marker rectangle
			# calculate marker rectangle
			innerHeight = remainRect[3]-remainRect[1]
			markerHeight = innerHeight/(self.maxValue-self.minValue+1.0)
			# markerRect 
			self.markerRect[0] = remainRect[0]
			self.markerRect[1] = remainRect[1] + (self.maxValue - self.currentValue)*markerHeight
			self.markerRect[2] = remainRect[2]
			self.markerRect[3] = self.markerRect[1] + markerHeight
			# clip markerRect to innerRect (catch all missed by one errors)
			if self.markerRect[1] > remainRect[3]:
				self.markerRect[1] = remainRect[3]
			if self.markerRect[3] > remainRect[3]:
				self.markerRect[3] = remainRect[3]
			# draw markerRect
			remainRect = self.markerRect
			Blender.BGL.glColor3f(0.78,0.78,0.78) # light grey
			Blender.BGL.glRectf(remainRect[0], remainRect[1], remainRect[2], remainRect[3])
			remainRect[0] += 1
			remainRect[3] -= 1
			Blender.BGL.glColor3f(0.13,0.13,0.13) # dark grey
			Blender.BGL.glRectf(remainRect[0], remainRect[1], remainRect[2], remainRect[3])
			remainRect[1] += 1
			remainRect[2] -= 1
			# check if marker has foucs
			if (self.mouseFocusX and self.mouseFocusY and (self.mousePositionY > self.markerRect[1]) and (self.mousePositionY < self.markerRect[3])):
				Blender.BGL.glColor3f(0.64,0.64,0.64) # marker focus grey
			else:
				Blender.BGL.glColor3f(0.60,0.60,0.60) # marker grey
			Blender.BGL.glRectf(remainRect[0], remainRect[1], remainRect[2], remainRect[3])
		else:
			print "scrollbar draw size too small!"
		return
		
	def eventFilter(self, event, value):
		"""event filter for keyboard and mouse input events
		   call it inside the registered event function
		"""
		if (value != 0):
			# Buttons
			if (event == Blender.Draw.PAGEUPKEY):
				self.up(3)
				Blender.Draw.Redraw(1)
			elif (event == Blender.Draw.PAGEDOWNKEY):
				self.down(3)
				Blender.Draw.Redraw(1)
			elif (event == Blender.Draw.UPARROWKEY):
				self.up(1)
				Blender.Draw.Redraw(1)
			elif (event == Blender.Draw.DOWNARROWKEY):
				self.down(1)
				Blender.Draw.Redraw(1)
			# Mouse
			elif (event == Blender.Draw.MOUSEX):
				# check if mouse is inside positionRect
				if (value >= (self.guiRect[0] + self.positionRect[0])) and (value <= (self.guiRect[0] + self.positionRect[2])):
					# redraw if marker got focus
					if (not self.mouseFocusX) and self.mouseFocusY:
						Blender.Draw.Redraw(1)
					self.mouseFocusX = 1
				else:
					# redraw if marker lost focus
					if self.mouseFocusX and self.mouseFocusY:
						Blender.Draw.Redraw(1)
					self.mouseFocusX = 0
			elif (event == Blender.Draw.MOUSEY):
				# check if mouse is inside positionRect
				if (value >= (self.guiRect[1] + self.positionRect[1])) and (value <= (self.guiRect[1] + self.positionRect[3])):
					self.mouseFocusY = 1
					# relative mouse position
					self.mousePositionY = value - self.guiRect[1]
					if ((self.mousePositionY > self.markerRect[1]) and (self.mousePositionY < self.markerRect[3])):
						# redraw if marker got focus
						if self.mouseFocusX and (not self.markerFocusY):
							Blender.Draw.Redraw(1)
						self.markerFocusY = 1
					else:
						# redraw if marker lost focus
						if self.mouseFocusX and self.markerFocusY:
							Blender.Draw.Redraw(1)
						self.markerFocusY = 0
					# move marker
					if (self.mousePressed == 1):
						# calculate step from distance to marker
						if (self.mousePositionY > self.markerRect[3]):
							# up
							self.up(1)
							Blender.Draw.Draw()
						elif (self.mousePositionY < self.markerRect[1]):
							# down
							self.down(1)
							Blender.Draw.Draw()
						# redraw if marker lost focus
						if self.mouseFocusX and self.mouseFocusY:
							Blender.Draw.Redraw(1)
				else:
					# redraw if marker lost focus
					if self.mouseFocusX and self.markerFocusY:
						Blender.Draw.Redraw(1)
					self.markerFocusY = 0
					self.mouseFocusY = 0
			elif ((event == Blender.Draw.LEFTMOUSE) and (self.mouseFocusX == 1) and (self.mouseFocusY == 1)):
				self.mousePressed = 1
				# move marker
				if (self.mousePositionY > self.markerRect[3]):
					# up
					self.up(1)
					Blender.Draw.Redraw(1)
				elif (self.mousePositionY < self.markerRect[1]):
					# down
					self.down(1)
					Blender.Draw.Redraw(1)
			elif (Blender.Get("version") >= 234):
				if (event == Blender.Draw.WHEELUPMOUSE):
					self.up(1)
					Blender.Draw.Redraw(1)
				elif (event == Blender.Draw.WHEELDOWNMOUSE):
					self.down(1)
					Blender.Draw.Redraw(1)
		else: # released keys and buttons
			if (event == Blender.Draw.LEFTMOUSE):
				self.mousePressed = 0
				
		return
		
	def buttonFilter(self, event):
		"""button filter for Draw Button events
		   call it inside the registered button function
		"""
		if (event  == self.buttonUpEvent):
			self.up()
			Blender.Draw.Redraw(1)
		elif (event == self.buttonDownEvent):
			self.down()
			Blender.Draw.Redraw(1)
		return

class ArmatureAction:
	"""Resembles Blender's actions
	   <ul>
	   <li> import Blender, string
	   </ul>
	   
	   @author Michael Reimpell
	"""
	def __init__(self, name="", ipoDict=None):
		"""Constructor
		
		   @param name    the action name
		   @param ipoDict a dictionary with bone names as keys and action Ipos as values
		"""
		self.firstKeyFrame = None
		self.lastKeyFrame = None
		self.name = name
		# ipoDict[boneName] = Blender.Ipo
		if ipoDict is None:
			self.ipoDict = {}
		else:
			self.ipoDict = ipoDict
			self._updateKeyFrameRange()
		return
	
	# private method	
	def _updateKeyFrameRange(self):
		"""Updates firstKeyFrame and lastKeyFrame considering the current IpoCurves.
		"""
		self.firstKeyFrame = None
		self.lastKeyFrame = None
		if self.ipoDict is not None:
			# check all bone Ipos
			for ipo in self.ipoDict.values():
				# check all IpoCurves
				for ipoCurve in ipo.getCurves():
					# check first and last keyframe
					for bezTriple in ipoCurve.getPoints():
						iFrame = bezTriple.getPoints()[0]
						if ((iFrame < self.firstKeyFrame) or (self.firstKeyFrame is None)):
							self.firstKeyFrame = iFrame
						if ((iFrame > self.lastKeyFrame) or (self.lastKeyFrame is None)):
							self.lastKeyFrame = iFrame
		return
	
	# static method
	def createArmatureActionDict(object):
		"""Creates a dictionary of possible actions belonging to an armature.
		   Static call with: ArmatureAction.createArmatureActionDict(object)
		   
		   @param object a Blender.Object of type Armature
		   @return a dictionary of ArmatureAction objects with name as key and ArmatureAction as value
		"""
		# create bone dict
		boneQueue = object.getData().getBones()
		boneDict = {}
		while (len(boneQueue) > 0):
			# get all bones of the armature
			currentBone = boneQueue.pop(0)
			boneDict[currentBone.getName()] = currentBone
			children = currentBone.getChildren()
			if (len(children) > 0):
				for child in children:
					boneQueue.append(child)
		boneNameList = boneDict.keys()
		# check for available actions
		armatureActionDict = {}
		## get linked action first
		linkedAction = object.getAction()
		if linkedAction is not None:
			# check all bones
			linkedActionIpoDict = linkedAction.getAllChannelIpos()
			hasValidChannel = 0 # false
			iBone = 0
			while ((not hasValidChannel) and (iBone < len(boneNameList))):
				if (linkedActionIpoDict.keys().count(boneNameList[iBone]) == 1):
					hasValidChannel = 1 # true
				else:
					iBone += 1
			if hasValidChannel:
				# add action
				armatureActionDict[linkedAction.getName()] = ArmatureAction(linkedAction.getName(), linkedActionIpoDict)
		## get other actions != linked action
		actionDict = Blender.Armature.NLA.GetActions()
		for actionName in actionDict.keys():
			# check if action is not linked action
			if actionDict[actionName] is not linkedAction:
				# check all bones
				actionIpoDict = actionDict[actionName].getAllChannelIpos()
				hasValidChannel = 0 # false
				iBone = 0
				while ((not hasValidChannel) and (iBone < len(boneNameList))):
					if (actionIpoDict.keys().count(boneNameList[iBone]) == 1):
						hasValidChannel = 1 # true
					else:
						iBone += 1
				if hasValidChannel:
					# add action
					armatureActionDict[actionName] = ArmatureAction(actionName, actionIpoDict)
		return armatureActionDict
	createArmatureActionDict = staticmethod(createArmatureActionDict)

class ArmatureActionActuator:
	"""Resembles Blender's action actuators.
	
	   @author Michael Reimpell
	"""
	def __init__(self, name, startFrame, endFrame, armatureAction):
		"""Constructor
		   
		   @param name           Animation name
		   @param startFrame     first frame of the animation
		   @param endFrame       last frame of the animation
		   @param armatureAction ArmatureAction object of the animation
		"""
		self.name = name
		self.startFrame = startFrame
		self.endFrame = endFrame
		self.armatureAction = armatureAction
		return

class ArmatureActionActuatorListView:
	"""Mangages a list of ArmatureActionActuators.
	   <ul>
	   <li> import Blender
	   <li> call eventFilter and buttonFilter in registered callbacks
	   </ul>
	   
	   @author Michael Reimpell
	"""
	def __init__(self, armatureActionDict, maxActuators, buttonEventRangeStart, armatureAnimationDictList=None):
		"""Constructor.
		   
		   @param armatureActionDict        possible armature actuator actions
		   @param maxActuators              maximal number of actuator list elements
		   @param buttonEventRangeStart     first button event number.
		                                    The number of used event numbers is (3 + maxActuators*5)
		   @param armatureAnimationDictList list of armature animations (see getArmatureAnimationDictList())
		"""
		self.armatureActionDict = armatureActionDict
		self.maxActuators = maxActuators
		self.buttonEventRangeStart = buttonEventRangeStart
		self.armatureActionActuatorList = []
		self.armatureActionMenuList = []
		self.startFrameNumberButtonList = []
		self.endFrameNumberButtonList = []
		self.animationNameStringButtonList = []
		# scrollbar values:
		#   0:(len(self.armatureActionActuatorList)-1) = listIndex
		#   len(self.armatureActionActuatorList) = addbuttonline
		self.scrollbar = ReplacementScrollbar(0,0,0, self.buttonEventRangeStart+1, self.buttonEventRangeStart+2)
		if armatureAnimationDictList is not None:
			# rebuild ArmatureActionActuators for animationList animations
			for animationDict in armatureAnimationDictList:
				# check if Action is available
				if self.armatureActionDict.has_key(animationDict['actionKey']):
					armatureActionActuator = ArmatureActionActuator(animationDict['name'], \
					                                                animationDict['startFrame'], \
					                                                animationDict['endFrame'], \
					                                                self.armatureActionDict[animationDict['actionKey']])
					self._addArmatureActionActuator(armatureActionActuator)
		else:
			# create default ArmatureActionActuators
			for armatureAction in self.armatureActionDict.values():
				# add default action
				armatureActionActuator = ArmatureActionActuator(armatureAction.name, armatureAction.firstKeyFrame, armatureAction.lastKeyFrame, armatureAction)
				self._addArmatureActionActuator(armatureActionActuator)
		return
		
	def refresh(self, armatureActionDict):
		"""Delete ArmatureActionActuators for removed Actions.
		    
		   @param armatureActionDict possible ArmatureActuator actions
		"""
		self.armatureActionDict = armatureActionDict
		# delete ArmatureActionActuators for removed Actions
		for armatureActionActuator in self.armatureActionActuatorList[:]:
			# check if action is still available
			if not self.armatureActionDict.has_key(armatureActionActuator.armatureAction.name):
				# remove armatureActionActuator from lists
				listIndex = self.armatureActionActuatorList.index(armatureActionActuator)
				self._deleteArmatureActionActuator(listIndex)
		Blender.Draw.Redraw(1)
		return
		
	def draw(self, x, y, width, height):
		"""draw actuatorList
		   use scrollbar if needed
		"""
		# black border
		minX = x
		minY = y
		maxX = x + width
		maxY = y + height
		minWidth = 441
		if ((width - 5) > minWidth):
			glColor3f(0.0,0.0,0.0)
			glRectf(minX, minY, maxX - 22, maxY)
			glColor3f(0.6,0.6,0.6) # Background: grey
			glRectf(minX + 1, minY + 1, maxX - 23, maxY - 1)
			x += 3
			y += 3
			width -= 5
			height -= 6
		else:
			print "ArmatureActionActuatorListView draw size to small!"
			glColor3f(0.0,0.0,0.0)
			glRectf(minX, minY, maxX, maxY)
			glColor3f(0.6,0.6,0.6) # Background: grey
			glRectf(minX + 1, minY + 1, maxX, maxY - 1)
			x += 3
			y += 3
			width -= 5
			height -= 6
		# Layout:
		# |---- 105 ---|2|----80---|2|---80---|2|---- >80 ----|2|---60---|2|----20---|
		# actionName   | startFrame | endFrame | animationName | [delete] | scrollbar
		# [ add ]                                                         | scrollbar
		if (len(self.armatureActionDict.keys()) > 0):
			# construct actionMenu name
			menuValue = 0
			menuName = ""
			for key in self.armatureActionDict.keys():
				menuName += key + " %x" + ("%d" % menuValue) + "|"
				menuValue +=1
			# first line
			lineY = y + height - 20
			lineX = x
			listIndex = self.scrollbar.getCurrentValue()
			while ((listIndex < len(self.armatureActionActuatorList)) and (lineY >= y)):
				# still armatureActionActuators left to draw
				lineX = x
				armatureActionActuator = self.armatureActionActuatorList[listIndex]
				# draw actionMenu
				event = self.buttonEventRangeStart + 3 + listIndex
				menuValue = self.armatureActionDict.keys().index(armatureActionActuator.armatureAction.name)
				self.armatureActionMenuList[listIndex] = Blender.Draw.Menu(menuName,event, x, lineY, 105, 20, menuValue, "Action name")
				lineX += 107
				# draw startFrameNumberButton
				event = self.buttonEventRangeStart + 3 + self.maxActuators + listIndex
				self.startFrameNumberButtonList[listIndex] = Blender.Draw.Number("Sta: ", event, lineX, lineY, 80, 20, \
				                                         armatureActionActuator.startFrame, -18000, 18000, "Start frame")
				lineX += 82
				# draw endFrameNumberButton
				event = self.buttonEventRangeStart + 3 + 2*self.maxActuators + listIndex
				self.endFrameNumberButtonList[listIndex] = Blender.Draw.Number("End: ", event, lineX, lineY, 80, 20, \
				                                         armatureActionActuator.endFrame, -18000, 18000, "End frame")
				lineX += 82
				# compute animationNameWidht
				animationNameWidth = width - 271 - 85
				if (animationNameWidth < 80):
					animationNameWidth = 80
				# draw animationNameStringButton
				event = self.buttonEventRangeStart + 3 + 3*self.maxActuators + listIndex
				self.animationNameStringButtonList[listIndex] = Blender.Draw.String("",event, lineX, lineY, animationNameWidth, 20, \
				                                                armatureActionActuator.name, 1000, "Animation export name") 
				lineX += animationNameWidth + 2
				# draw deleteButton
				event = self.buttonEventRangeStart + 3 + 4*self.maxActuators + listIndex
				Draw.Button("Delete", event, lineX, lineY, 60, 20, "Delete export animation")
				lineX += 62
				# inc line
				lineY -= 22
				listIndex += 1
			# draw add button
			if (lineY >= y):
				Draw.Button("Add", self.buttonEventRangeStart, x, lineY, 60, 20, "Add new export animation")
		# draw scrollbar
		if (width > minWidth):
			# align left
			self.scrollbar.draw(maxX - 20, minY, 20, (maxY - minY))
		return
	
	def eventFilter(self, event, value):
		"""event filter for keyboard and mouse input events
		   call it inside the registered event function
		"""
		self.scrollbar.eventFilter(event,value)
		return
		
	def buttonFilter(self, event):
		"""button filter for Draw Button events
		   call it inside the registered button function
		"""
		# button numbers = self.buttonEventRangeStart + buttonNumberOffset
		# buttonNumberOffsets:
		# addButton: 0 
		# scrollbar: 1 and 2
		# actionMenu range: 3 <= event < 3 + maxActuators
		# startFrameNumberButton range:  3 + maxActuators <= event < 3 + 2*maxActuators
		# endFrameNumberButton range: 3 + 2*maxActuators <= event < 3 + 3*maxActuators
		# animationNameStringButton range: 3 + 3*maxActuators <= event < 3 + 4*maxActuators
		# deleteButton range: 3 + 4*maxActuators <= event < 3 + 5*maxActuators
		self.scrollbar.buttonFilter(event)
		relativeEvent = event - self.buttonEventRangeStart
		if (relativeEvent == 0):
			# add button pressed
			if (len(self.armatureActionDict.keys()) > 0):
				# add default ArmatureActionActuator
				armatureAction = self.armatureActionDict[self.armatureActionDict.keys()[0]]
				armatureActionActuator = ArmatureActionActuator(armatureAction.name, armatureAction.firstKeyFrame, armatureAction.lastKeyFrame, armatureAction)
				self._addArmatureActionActuator(armatureActionActuator)
				Blender.Draw.Redraw(1)
		elif ((3 <= relativeEvent) and (relativeEvent < (3 + self.maxActuators))):
			# actionMenu
			listIndex = relativeEvent - 3
			armatureActionActuator = self.armatureActionActuatorList[listIndex]
			# button value is self.actionDict.keys().index
			keyIndex = self.armatureActionMenuList[listIndex].val
			key = self.armatureActionDict.keys()[keyIndex]
			armatureActionActuator.armatureAction = self.armatureActionDict[key]
			armatureActionActuator.startFrame = self.armatureActionDict[key].firstKeyFrame
			armatureActionActuator.endFrame = self.armatureActionDict[key].lastKeyFrame
			armatureActionActuator.name = self.armatureActionDict[key].name
			self.armatureActionActuatorList[listIndex] = armatureActionActuator
			Blender.Draw.Redraw(1)
		elif (((3 + self.maxActuators) <= relativeEvent) and (relativeEvent < (3 + 2*self.maxActuators))):
			# startFrameNumberButton
			listIndex = relativeEvent - (3 + self.maxActuators)
			armatureActionActuator = self.armatureActionActuatorList[listIndex]
			armatureActionActuator.startFrame = self.startFrameNumberButtonList[listIndex].val
			self.armatureActionActuatorList[listIndex] = armatureActionActuator
		elif (((3 + 2*self.maxActuators) <= relativeEvent) and (relativeEvent < (3 + 3*self.maxActuators))):
			# endFrameNumberButton
			listIndex = relativeEvent - (3 + 2*self.maxActuators)
			armatureActionActuator = self.armatureActionActuatorList[listIndex]
			armatureActionActuator.endFrame = self.endFrameNumberButtonList[listIndex].val
			self.armatureActionActuatorList[listIndex] = armatureActionActuator
		elif (((3 + 3*self.maxActuators) <= relativeEvent) and (relativeEvent < (3 + 4*self.maxActuators))):
			# animationNameStringButton
			listIndex = relativeEvent - (3 + 3*self.maxActuators)
			armatureActionActuator = self.armatureActionActuatorList[listIndex]
			armatureActionActuator.name = self.animationNameStringButtonList[listIndex].val
			self.armatureActionActuatorList[listIndex] = armatureActionActuator
		elif (((3 + 4*self.maxActuators) <= relativeEvent) and (relativeEvent < (3 + 5*self.maxActuators))):
			# deleteButton
			listIndex = relativeEvent - (3 + 4*self.maxActuators)
			self._deleteArmatureActionActuator(listIndex)
			Blender.Draw.Redraw(1)
		return
		
	def getArmatureAnimationDictList(self):
		"""serialize the armatureActionActuatorList into a pickle storable list
		   Each item of the returned list is a dictionary with key-value pairs:
		   <ul>
		   <li>	name - ArmatureActionActuator.name
		   <li>	startFrame - ArmatureActionActuator.startFrame
		   <li>	endFrame - ArmatureActionActuator.endFrame
		   <li>	armatureActionKey - ArmatureActionActuator.armatureAction.name
		   </ul>
		   
		   @return serialized actionActuatorList
		"""
		animationDictList = []
		for armatureActionActuator in self.armatureActionActuatorList:
			# create animationDict
			animationDict = {}
			animationDict['name'] = armatureActionActuator.name
			animationDict['startFrame'] = armatureActionActuator.startFrame
			animationDict['endFrame'] = armatureActionActuator.endFrame
			animationDict['actionKey'] = armatureActionActuator.armatureAction.name
			animationDictList.append(animationDict)
		return animationDictList
		
	def setAnimationDictList(self, animationDictList):
		"""loads old AnimationDictList with actionKey = (ipoPrefix, ipoPostfix)
		   
		   @see #getArmatureAnimationDictList()
		"""
		# rebuild ArmatureActionActuators for animationList animations
		for animationDict in animationDictList:
			# check if Action is available
			prefix, postfix = animationDict['actionKey']
			armatureActionName = prefix+postfix
			if self.armatureActionDict.has_key(armatureActionName):
				armatureActionActuator = ArmatureActionActuator(animationDict['name'], \
				                                                animationDict['startFrame'], \
				                                                animationDict['endFrame'], \
				                                                self.armatureActionDict[armatureActionName])
				self._addArmatureActionActuator(armatureActionActuator)
		return
		
	# private methods
	def _addArmatureActionActuator(self, armatureActionActuator):
		"""adds an ArmatureActionActuator to the list
		   <ul>
		   <li> call Blender.Draw.Redraw(1) afterwards
		   </ul>
		"""
		if (len(self.armatureActionActuatorList) < self.maxActuators):
			# check if armatureActionActuator.action is available
			if armatureActionActuator.armatureAction.name in self.armatureActionDict.keys():
				# create armatureActionMenu
				# get ArmatureAction index in armatureActionDict.keys() list
				armatureActionMenu = Draw.Create(self.armatureActionDict.keys().index(armatureActionActuator.armatureAction.name))
				self.armatureActionMenuList.append(armatureActionMenu)
				# create startFrameNumberButton
				startFrameNumberButton = Draw.Create(int(armatureActionActuator.startFrame))
				self.startFrameNumberButtonList.append(startFrameNumberButton)
				# create endFrameNumberButton
				endFrameNumberButton = Draw.Create(int(armatureActionActuator.endFrame))
				self.endFrameNumberButtonList.append(endFrameNumberButton)
				# create animationNameStringButton
				animationNameStringButton = Draw.Create(armatureActionActuator.name)
				self.animationNameStringButtonList.append(animationNameStringButton)
				# append to armatureActionActuatorList
				self.armatureActionActuatorList.append(armatureActionActuator)
				# adjust scrollbar
				scrollbarPosition = self.scrollbar.getCurrentValue()
				self.scrollbar = ReplacementScrollbar(scrollbarPosition,0,len(self.armatureActionActuatorList), self.buttonEventRangeStart+1, self.buttonEventRangeStart+2)
				# TODO: change scrollbarPosition in a way, such that the new actuator is visible
			else:
				print "Error: Could not add ArmatureActionActuator because ArmatureAction is not available!"
		return
		
	def _deleteArmatureActionActuator(self, listIndex):
		"""removes an ArmatureActionActuator from the list
		   <ul>
		   <li> call Blender.Draw.Redraw(1) afterwards
		   </ul>
		"""
		# check listIndex
		if ((len(self.armatureActionActuatorList) > 0) and (listIndex >= 0) and (listIndex < len(self.armatureActionActuatorList))):
			# remove armatureActionMenu
			self.armatureActionMenuList.pop(listIndex)
			# remove startFrameNumberButton
			self.startFrameNumberButtonList.pop(listIndex)
			# remove endFrameNumberButton
			self.endFrameNumberButtonList.pop(listIndex)
			# remove animationNameStringButton
			self.animationNameStringButtonList.pop(listIndex)
			# remove armatureActionActuator
			self.armatureActionActuatorList.pop(listIndex)
			# adjust scrollbar
			scrollbarPosition = self.scrollbar.getCurrentValue()
			if (scrollbarPosition > len(self.armatureActionActuatorList)):
				scrollbarPosition = len(self.armatureActionActuatorList)
			self.scrollbar = ReplacementScrollbar(scrollbarPosition,0,len(self.armatureActionActuatorList), self.buttonEventRangeStart+1, self.buttonEventRangeStart+2)
			return

class Logger:
	"""Logs messages and status.
	
	   Logs messages as a list of strings and keeps track of the status.
	   Possible status values are info, warning and error.
	   
	   @cvar INFO info status
	   @cvar WARNING warning status
	   @cvar ERROR error status
	"""
	INFO, WARNING, ERROR = range(3)
	def __init__(self):
		"""Constructor.
		"""
		self.messageList = []
		self.status = Logger.INFO
		return
	def logInfo(self, message):
		"""Logs an info message.
		
		   @param message message string
		"""
		self.messageList.append((Logger.INFO, message))
		return		
	def logWarning(self, message):
		"""Logs a warning message.
		
		   The status is set to <code>Logger.WARNING</code> if it is not already <code>Logger.ERROR</code>.
		   
		   @param message message string
		"""
		self.messageList.append((Logger.WARNING, "Warning: "+message))
		if not self.status == Logger.ERROR:
			self.status = Logger.WARNING
		return
	def logError(self, message):
		"""Logs an error message.
		
		   The status is set to <code>Logger.ERROR</code>.
		   
		   @param message message string
		"""
		self.messageList.append((Logger.ERROR, "Error: "+message))
		self.status = Logger.ERROR
		return
	def getStatus(self):
		"""Gets the current status.
		
		   The status can be
		   <ul>
		   <li><code>Logger.INFO</code>
		   <li><code>Logger.WARNING</code>
		   <li><code>Logger.ERROR</code>
		   </ul>
		   
		   @return status
		"""
		return self.status
	def getMessageList(self):
		"""Returns the list of log messages.
		
		   @return list of tuples (status, message)
		"""
		return self.messageList

class LogInterface:
	def __init__(self):
		self.loggerList = []
	def addLogger(self, logger):
		self.loggerList.append(logger)
		return
	def removeLogger(self, logger):
		self.loggerList.remove(logger)
		return
	# protected
	def _logInfo(self, message):
		for logger in self.loggerList:
			logger.logInfo(message)
		return
	def _logWarning(self, message):
		for logger in self.loggerList:
			logger.logWarning(message)
		return
	def _logError(self, message):
		for logger in self.loggerList:
			logger.logWarning(message)
		return

class PathName(LogInterface):
	"""Splits a pathname independent of the underlying os.
	
	   Blender saves pathnames in the os specific manner. Using os.path may result in problems
	   when the export is done on a different os than the creation of the .blend file.	   
	"""
	def __init__(self, pathName):
		self.pathName = pathName
		LogInterface.__init__(self)
		return
	def dirname(self):
		return os.path.dirname(self.pathName) 
	def basename(self):
		baseName = os.path.basename(self.pathName)
		# split from non-os directories
		# \\
		baseName = baseName.split('\\').pop()
		# /
		baseName = baseName.split('/').pop()
		if (baseName != baseName.replace(' ','_')):
			# replace whitespace with underscore
			self._logWarning("Whitespaces in filename \"%s\" replaced with underscores." % baseName)
			baseName = baseName.replace(' ','_')
		return baseName
	def path(self):
		return self.pathName

class ExportOptions:
	"""Encapsulates export options common to all objects.
	"""
	# TODO: Model for GUI
	def __init__(self, rotXAngle, rotYAngle, rotZAngle, scale, useWorldCoordinates, colouredAmbient, exportPath, materialFilename):
		"""Constructor.
		"""
		# floating point accuracy
		self.accuracy = 1e-6
		# export transformation
		self.rotXAngle = rotXAngle
		self.rotYAngle = rotYAngle
		self.rotZAngle = rotZAngle
		self.scale = scale
		self.useWorldCoordinates = useWorldCoordinates
		self.colouredAmbient = colouredAmbient
		# file settings
		self.exportPath = exportPath
		self.materialFilename = materialFilename
		return
	
	def transformationMatrix(self):
		"""Returns the matrix representation for the additional transformation on export.
		"""
		rotationMatrix = Mathutils.RotationMatrix(self.rotXAngle,4,'x')
		rotationMatrix *= Mathutils.RotationMatrix(self.rotYAngle,4,'y')
		rotationMatrix *= Mathutils.RotationMatrix(self.rotZAngle,4,'z')
		scaleMatrix = Mathutils.Matrix([self.scale,0,0],[0,self.scale,0],[0,0,self.scale])
		scaleMatrix.resize4x4()
		return rotationMatrix*scaleMatrix

class ObjectExporter:
	"""Interface. Exports a Blender object to Ogre.
	"""
	def __init__(self, object):
		"""Constructor.
		   
		   @param object Blender object to export.
		"""
		self.object = object
		return
	
	def getName(self):
		"""Returns the name of the object.
		"""
		return self.object.getName()
	
	def getObjectMatrix(self):
		"""Returns the object matrix in worldspace.
		"""
		return self.object.getMatrix('worldspace') 
	
class MeshExporter(ObjectExporter):
	"""
	"""
	def getName(self):
		return self.object.getData().name
		
class ArmatureExporter:
	"""Exports an armature of a mesh.
	"""
	# TODO: Provide bone ids for vertex influences.
	def __init__(self, meshObject, armatureObject):
		"""Constructor.
		
		  @param meshObject ObjectExporter.
		  @param armatureObject Blender armature object.
		"""
		self.meshObject = meshObject
		self.armatureObject = armatureObject
		self.skeleton = None
		return
	
	def export(self, actionActuatorList, exportOptions, logger):
		"""Exports the armature.
		
		   @param actionActuatorList list of animations to export.
		   @param exportOptions global export options.
		   @param logger Logger Logger for log messages. 		   
		"""
		# convert Armature into Skeleton
		name = None
		if exportOptions.useWorldCoordinates:
			name = self.armatureObject.getData().getName()
		else:
			name = self.meshObject.getName() + "-" + self.armatureObject.getData().getName()
		skeleton = Skeleton(name)
		skeleton = self._convertRestpose(skeleton, exportOptions, logger)
		
		# convert ActionActuators into Animations
		self._convertAnimations(skeleton, actionActuatorList, exportOptions, exportLogger)
		
		# write to file
		self._toFile(skeleton, exportOptions, exportLogger)
		
		self.skeleton = skeleton
		return
	
	def _convertAnimations(self, skeleton, armatureActionActuatorList, exportOptions, exportLogger):
		"""Converts ActionActuators to Ogre animations.
		"""
		# frames per second
		fps = Blender.Scene.GetCurrent().getRenderingContext().framesPerSec()
		# map armatureActionActuatorList to skeleton.animationsDict
		for armatureActionActuator in armatureActionActuatorList:
			# map armatureActionActuator to animation
			if (not skeleton.animationsDict.has_key(armatureActionActuator.name)):
				# create animation
				animation = Animation(armatureActionActuator.name)
				# map bones to tracks
				for boneName in armatureActionActuator.armatureAction.ipoDict.keys():
					if (not(animation.tracksDict.has_key(boneName))):
						# get bone object
						if skeleton.bonesDict.has_key(boneName):
							# create track
							track = Track(animation, skeleton.bonesDict[boneName])
							# map ipocurves to keyframes
							# get ipo for that bone
							ipo = armatureActionActuator.armatureAction.ipoDict[boneName]
							# map curve names to curvepos
							curveId = {}
							index = 0
							have_quat = 0
							for curve in ipo.getCurves():
								try:
									name = curve.getName()
									if (name == "LocX" or name == "LocY" or name == "LocZ" or \
									name == "SizeX" or name == "SizeY" or name == "SizeZ" or \
									name == "QuatX" or name == "QuatY" or name == "QuatZ" or name == "QuatW"):
										curveId[name] = index
										index += 1
									else:
									# bug: 2.28 does not return "Quat*"...
										if not have_quat:
											curveId["QuatX"] = index
											curveId["QuatY"] = index+1
											curveId["QuatZ"] = index+2
											curveId["QuatW"] = index+3
											index += 4
											have_quat = 1
								except TypeError:
									# blender 2.32 does not implement IpoCurve.getName() for action Ipos
									if not have_quat:
										# no automatic assignments so far
										# guess Ipo Names       
										nIpoCurves = ipo.getNcurves()
										if nIpoCurves in [4,7,10]:
											exportLogger.logWarning("IpoCurve.getName() not available!")
											exportLogger.logWarning("The exporter tries to guess the IpoCurve names.")
											if (nIpoCurves >= 7):
												# not only Quats
												# guess: Quats and Locs
												curveId["LocX"] = index
												curveId["LocY"] = index+1
												curveId["LocZ"] = index+2
												index += 3      
											if (nIpoCurves == 10):
												# all possible Action IpoCurves
												curveId["SizeX"] = index
												curveId["SizeY"] = index+1
												curveId["SizeZ"] = index+2
												index += 3
											if (nIpoCurves >= 4):
												# at least 4 IpoCurves
												# guess: 4 Quats
												curveId["QuatX"] = index
												curveId["QuatY"] = index+1
												curveId["QuatZ"] = index+2
												curveId["QuatW"] = index+3
												index += 4
											have_quat = 1
										else:
											exportLogger.logError("IpoCurve.getName() not available!")
											exportLogger.logError("Could not guess the IpoCurve names. Other Blender versions may work.")
							# get all frame numbers between startFrame and endFrame where this ipo has a point in one of its curves
							frameNumberDict = {}
							for curveIndex in range(ipo.getNcurves()):
								for bez in range(ipo.getNBezPoints(curveIndex)):
									frame = int(ipo.getCurveBeztriple(curveIndex, bez)[3])
									frameNumberDict[frame] = frame
							frameNumberDict[armatureActionActuator.startFrame] = armatureActionActuator.startFrame
							frameNumberDict[armatureActionActuator.endFrame] = armatureActionActuator.endFrame
							# remove frame numbers not in the startFrame endFrame range
							if (armatureActionActuator.startFrame > armatureActionActuator.endFrame):
								minFrame = armatureActionActuator.endFrame
								maxFrame = armatureActionActuator.startFrame
							else:
								minFrame = armatureActionActuator.startFrame
								maxFrame = armatureActionActuator.endFrame
							for frameNumber in frameNumberDict.keys()[:]:
								if ((frameNumber < minFrame) or (frameNumber > maxFrame)):
									del frameNumberDict[frameNumber]
							frameNumberList = frameNumberDict.keys()
							# convert frame numbers to seconds
							# frameNumberDict: key = export time, value = frame number
							frameNumberDict = {}
							for frameNumber in frameNumberList:
								if  (armatureActionActuator.startFrame <= armatureActionActuator.endFrame):
									# forward animation
									time = float(frameNumber-armatureActionActuator.startFrame)/fps
								else:
									# backward animation
									time = float(armatureActionActuator.endFrame-frameNumber)/fps
								# update animation duration
								if animation.duration < time:
									animation.duration = time
								frameNumberDict[time] = frameNumber
							# create key frames
							timeList = frameNumberDict.keys()
							timeList.sort()
							for time in timeList:
								# Blender's ordering of transformation is deltaR*deltaS*deltaT
								# in the bones coordinate system.
								frame = frameNumberDict[time]
								loc = ( 0.0, 0.0, 0.0 )
								rotQuat = Mathutils.Quaternion([1.0, 0.0, 0.0, 0.0])
								sizeX = 1.0
								sizeY = 1.0
								sizeZ = 1.0
								blenderLoc = [0, 0, 0]
								hasLocKey = 0 #false
								if curveId.has_key("LocX"):
									blenderLoc[0] = ipo.EvaluateCurveOn(curveId["LocX"], frame)
									hasLocKey = 1 #true
								if curveId.has_key("LocY"):
									blenderLoc[1] = ipo.EvaluateCurveOn(curveId["LocY"], frame)
									hasLocKey = 1 #true
								if curveId.has_key("LocZ"):
									blenderLoc[2] = ipo.EvaluateCurveOn(curveId["LocZ"], frame)
									hasLocKey = 1 #true
								if hasLocKey:
									# Ogre's deltaT is in the bone's parent coordinate system
									loc = point_by_matrix(blenderLoc, skeleton.bonesDict[boneName].conversionMatrix)
								if curveId.has_key("QuatX") and curveId.has_key("QuatY") and curveId.has_key("QuatZ") and curveId.has_key("QuatW"):
									if not (Blender.Get("version") == 234):
										rot = [ ipo.EvaluateCurveOn(curveId["QuatW"], frame), \
												ipo.EvaluateCurveOn(curveId["QuatX"], frame), \
												ipo.EvaluateCurveOn(curveId["QuatY"], frame), \
												ipo.EvaluateCurveOn(curveId["QuatZ"], frame) ]
										rotQuat = Mathutils.Quaternion(rot)
									else:
										# Blender 2.34 quaternion naming bug
										rot = [ ipo.EvaluateCurveOn(curveId["QuatX"], frame), \
												ipo.EvaluateCurveOn(curveId["QuatY"], frame), \
												ipo.EvaluateCurveOn(curveId["QuatZ"], frame), \
												ipo.EvaluateCurveOn(curveId["QuatW"], frame) ]
										rotQuat = Mathutils.Quaternion(rot)
								if curveId.has_key("SizeX"):
									sizeX = ipo.EvaluateCurveOn(curveId["SizeX"], frame)
								if curveId.has_key("SizeY"):
									sizeY = ipo.EvaluateCurveOn(curveId["SizeY"], frame)
								if curveId.has_key("SizeZ"):
									sizeZ = ipo.EvaluateCurveOn(curveId["SizeZ"], frame)
								size = (sizeX, sizeY, sizeZ)
								KeyFrame(track, time, loc, rotQuat, size)
							# append track
							animation.tracksDict[boneName] = track
						else:
							# ipo name contains bone but armature doesn't
							exportLogger.logWarning("Unused action channel \"%s\" in action \"%s\" for skeleton \"%s\"." \
											 % (boneName, armatureActionActuator.armatureAction.name, skeleton.name))
					else:
						# track for that bone already exists
						exportLogger.logError("Ambiguous bone name \"%s\", track already exists." % boneName)
				# append animation
				skeleton.animationsDict[armatureActionActuator.name] = animation
			else:
				# animation export name already exists
				exportLogger.logError("Ambiguous animation name \"%s\"." % armatureActionActuator.name)
		return
	
	def _convertRestpose(self, skeleton, exportOptions, logger):
		"""Calculate inital bone positions and rotations.
		"""
		obj = self.armatureObject
		stack = []
		matrix = None
		if exportOptions.useWorldCoordinates:
			# world coordinates
			matrix = obj.getMatrix("worldspace")
		else:
			# local mesh coordinates
			armatureMatrix = obj.getMatrix("worldspace")
			inverseMeshMatrix = self.meshObject.getObjectMatrix()
			inverseMeshMatrix.invert()
			matrix = armatureMatrix*inverseMeshMatrix
		# apply additional export transformation
		matrix = matrix*exportOptions.transformationMatrix()
		loc = [ 0.0, 0, 0 ]
		matrix_one = Mathutils.Matrix([1.0, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1])
		parent = None
		
		# get parent bones
		boneList = obj.getData().getBones()
		boneDict = {} 
		for bone in boneList:
			if not bone.hasParent():
				boneDict[bone.getName()] = bone
		for bbone in boneDict.values():  
			stack.append([bbone, parent, matrix, loc, 0, matrix_one])
		
		while len(stack):
			bbone, parent, accu_mat, parent_pos, parent_ds, invertedOgreTransformation = stack.pop()
			# preconditions: (R : rotation, T : translation, S : scale, M: general transformation matrix)
			#   accu_mat
			#     points to the tail of the parents bone, i.e. for root bones
			#     accu_mat = M_{object}*R_{additional on export}
			#     and for child bones
			#     accu_mat = T_{length of parent}*R_{parent}*T_{to head of parent}*M_{parent's parent}
			#  invertedOgreTransformation
			#    inverse of transformation done in Ogre so far, i.e. identity for root bones,
			#    M^{-1}_{Ogre, parent's parent}*T^{-1}_{Ogre, parent}*R^{-1}_{Ogre, parent} for child bones.
			
			head = bbone.getHead()
			tail = bbone.getTail()
			
			# get the restmat 
			R_bmat = bbone.getRestMatrix('bonespace').rotationPart()
			R_bmat.resize4x4()
			
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
			# tmp_mat = R_{bone}*T_{to head}*M_{parent}
			accu_mat = matrix_multiply(accu_mat, T_len)
			
			# local rotation and distance from parent bone
			if parent:
				rotQuat = bbone.getRestMatrix('bonespace').toQuat()
			
			else:
				rotQuat = (bbone.getRestMatrix('bonespace')*matrix).toQuat()
			
			x, y, z = pos
			# pos = loc * M_{Ogre}
			loc = point_by_matrix([x, y, z], invertedOgreTransformation)
			x, y, z = loc
			ogreTranslationMatrix = [[ 1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [x, y, z, 1]]
			
			# R_{Ogre} is either
			# the rotation part of R_{bone}*T_{to_head}*M_{parent} for root bones or
			# the rotation part of R_{bone}*T_{to_head} of child bones
			ogreRotationMatrix = rotQuat.toMatrix()
			ogreRotationMatrix.resize4x4()
			invertedOgreTransformation = matrix_multiply(matrix_invert(ogreTranslationMatrix), invertedOgreTransformation)
			parent = Bone(skeleton, parent, bbone.getName(), loc, rotQuat, matrix_multiply(invertedOgreTransformation, tmp_mat))
			# matrix_multiply(invertedOgreTransformation, tmp_mat) is R*T*M_{parent} M^{-1}_{Ogre}T^{-1}_{Ogre}.
			# Necessary, since Ogre's delta location is in the Bone's parent coordinate system, i.e.
			# delatT_{Blender}*R*T*M = deltaT_{Ogre}*T_{Ogre}*M_{Ogre}
			invertedOgreTransformation = matrix_multiply(matrix_invert(ogreRotationMatrix), invertedOgreTransformation)
			for child in bbone.getChildren():
				stack.append([child, parent, accu_mat, pos, ds, invertedOgreTransformation])
		return skeleton
		
	def _toFile(self, skeleton, exportOptions, exportLogger):
		"""Writes converted skeleton to file.
		"""
		file = skeleton.name+".skeleton.xml"
		exportLogger.logInfo("Skeleton \"%s\"" % file)
		f = open(os.path.join(exportOptions.exportPath, file), "w")
		f.write(tab(0)+"<skeleton>\n")
		f.write(tab(1)+"<bones>\n")
		for bone in skeleton.bones:
			f.write(tab(2)+"<bone id=\"%d\" name=\"%s\">\n" % (bone.id, bone.name))

			x, y, z = bone.loc
			f.write(tab(3)+"<position x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" % (x, y, z))

			f.write(tab(3)+"<rotation angle=\"%.6f\">\n" % (bone.rotQuat.angle/360*2*math.pi))
			f.write(tab(4)+"<axis x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" % tuple(bone.rotQuat.axis))
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
			name = animation.name
			
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
					
					f.write(tab(7)+"<rotate angle=\"%.6f\">\n" % (keyframe.rotQuat.angle/360*2*math.pi))
					f.write(tab(8)+"<axis x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" % tuple(keyframe.rotQuat.axis))
					f.write(tab(7)+"</rotate>\n")
					
					f.write(tab(7)+"<scale x=\"%f\" y=\"%f\" z=\"%f\"/>\n" % keyframe.scale)
					
					f.write(tab(6)+"</keyframe>\n")
				
				f.write(tab(5)+"</keyframes>\n")
				f.write(tab(4)+"</track>\n");
			
			f.write(tab(3)+"</tracks>\n");
			f.write(tab(2)+"</animation>\n")
			
		f.write(tab(1)+"</animations>\n")
		f.write(tab(0)+"</skeleton>\n")
		f.close()
		convertXMLFile(os.path.join(exportOptions.exportPath, file))
		return
		
class ArmatureMeshExporter(ObjectExporter):
	"""Exports an armature object as mesh.
	
	   Converts a Blender armature into an animated Ogre mesh.
	"""
	# TODO:	Use observer pattern for progress bar.
	# TODO: Get bone ids from skeletonExporter class.
	def __init__(self, armatureObject):
		"""Constructor.
		
		   @param armatureObject armature object to export.
		"""
		# call base class constructor
		ObjectExporter.__init__(self, armatureObject)
		self.skeleton = None
		return
	
	def export(self, materialsDict, actionActuatorList, exportOptions, logger):
		"""Exports the mesh object.
		   
		   @param materialsDict dictionary that contains already existing materials.
		   @param actionActuatorList list of animations to export.
		   @param exportOptions global export options.
		   @param logger Logger for log messages.
		   @return materialsDict with the new materials added.
		"""
		# export skeleton
		armatureExporter = ArmatureExporter(self, self.object)
		armatureExporter.export(actionActuatorList, exportOptions, logger)
		self.skeleton = armatureExporter.skeleton
		self._convertToMesh(materialsDict, exportOptions, logger)
		return materialsDict
	def getName(self):
		return self.object.getData().getName()
	
	def _convertToMesh(self, materialsDict, exportOptions, logger):
		"""Creates meshes in form of the armature bones.
		"""
		obj = self.object
		stack = []
		# list of bone data (boneName, startPosition, endPosition)
		boneMeshList = []		
		matrix = None
		if exportOptions.useWorldCoordinates:
			# world coordinates
			matrix = obj.getMatrix("worldspace")
		else:
			# local mesh coordinates
			armatureMatrix = obj.getMatrix("worldspace")
			inverseMeshMatrix = self.getObjectMatrix()
			inverseMeshMatrix.invert()
			matrix = armatureMatrix*inverseMeshMatrix
		# apply additional export transformation
		matrix = matrix*exportOptions.transformationMatrix()
		loc = [ 0.0, 0, 0 ]
		parent = None
		
		# get parent bones
		boneList = obj.getData().getBones()
		boneDict = {} 
		for bone in boneList:
			if not bone.hasParent():
				boneDict[bone.getName()] = bone
		for bbone in boneDict.values():  
			stack.append([bbone, parent, matrix, loc, 0])
		
		while len(stack):
			bbone, parent, accu_mat, parent_pos, parent_ds = stack.pop()
			# preconditions: (R : rotation, T : translation, S : scale, M: general transformation matrix)
			#   accu_mat
			#     points to the tail of the parents bone, i.e. for root bones
			#     accu_mat = M_{object}*R_{additional on export}
			#     and for child bones
			#     accu_mat = T_{length of parent}*R_{parent}*T_{to head of parent}*M_{parent's parent}
			#  invertedOgreTransformation
			#    inverse of transformation done in Ogre so far, i.e. identity for root bones,
			#    M^{-1}_{Ogre, parent's parent}*T^{-1}_{Ogre, parent}*R^{-1}_{Ogre, parent} for child bones.
			
			head = bbone.getHead()
			tail = bbone.getTail()
			
			# get the restmat 
			R_bmat = bbone.getRestMatrix('bonespace').rotationPart()
			R_bmat.resize4x4()
			
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
			# tmp_mat = R_{bone}*T_{to head}*M_{parent}
			accu_mat = matrix_multiply(accu_mat, T_len)
			pos2 = point_by_matrix([ 0, 0, 0 ], accu_mat)
			boneMeshList.append([bbone.getName(), pos, pos2])
			for child in bbone.getChildren():
				stack.append([child, parent, accu_mat, pos, ds])
		self._createMeshFromBoneList(materialsDict, boneMeshList)
		return

	def _makeFace(self, submesh, name, p1, p2, p3):
		normal = vector_normalize(vector_crossproduct(
				[ p3[0] - p2[0], p3[1] - p2[1], p3[2] - p2[2] ],
				[ p1[0] - p2[0], p1[1] - p2[1], p1[2] - p2[2] ]))
		v1 = Vertex(submesh, XMLVertex(p1, normal))
		v2 = Vertex(submesh, XMLVertex(p2, normal))
		v3 = Vertex(submesh, XMLVertex(p3, normal))

		id = self.skeleton.bonesDict[name]
		v1.influences.append(Influence(id, 1.0))
		v2.influences.append(Influence(id, 1.0))
		v3.influences.append(Influence(id, 1.0))

		Face(submesh, v1, v2, v3)
		return
	
	def _createMeshFromBoneList(self, materialsDict, boneMeshList):
		matName = "SkeletonMaterial"
		material = materialsDict.get(matName)
		if not material:
			material = Material(matName, None, None)
			materialsDict[matName] = material

		submesh = SubMesh(material)
		for name, p1, p2 in boneMeshList:
			axis = blender_bone2matrix(p1, p2, 0)
			axis = matrix_translate(axis, p1)
			dx, dy, dz = p1[0] - p2[0], p1[1] - p2[1], p1[2] - p2[2]
			ds = math.sqrt(dx*dx + dy*dy + dz*dz)
			d = 0.1 + 0.2 * (ds / 10.0)
			c1 = point_by_matrix([-d, 0,-d], axis)
			c2 = point_by_matrix([-d, 0, d], axis)
			c3 = point_by_matrix([ d, 0, d], axis)
			c4 = point_by_matrix([ d, 0,-d], axis)
			
			self._makeFace(submesh, name, p2, c1, c2)
			self._makeFace(submesh, name, p2, c2, c3)
			self._makeFace(submesh, name, p2, c3, c4)
			self._makeFace(submesh, name, p2, c4, c1)
			self._makeFace(submesh, name, c3, c2, c1)
			self._makeFace(submesh, name, c1, c4, c3)
		file = self.getName()
		write_mesh(file, [submesh], self.skeleton)
		return
	
######
# global variables
######
uvToggle = Draw.Create(1)
armatureToggle = Draw.Create(1)
worldCoordinatesToggle = Draw.Create(0)
ambientToggle = Draw.Create(0)
pathString = Draw.Create(os.path.dirname(Blender.Get('filename')))
materialString = Draw.Create(Blender.Scene.GetCurrent().getName()+".material")
scaleNumber = Draw.Create(1.0)
fpsNumber = Draw.Create(25)
# first rotation, around X-axis
rotXNumber = Draw.Create(-90.0)
# second rotation, around Y-axis
rotYNumber = Draw.Create(0.0)
# third rotation, around Z-axis
rotZNumber = Draw.Create(0.0)
selectedObjectsList = Blender.Object.GetSelected()
selectedObjectsMenu = Draw.Create(0)
scrollbar = ReplacementScrollbar(0,0,0,0,0)
# key: objectName, value: armatureName
armatureDict = {}
# key: armatureName, value: armatureActionActuatorListView
# does only contain keys for the current selected objects
armatureActionActuatorListViewDict = {}
# key: armatureName, value: animationDictList
armatureAnimationDictListDict = {}
MAXACTUATORS = 100

# button event numbers:
BUTTON_EVENT_OK = 101
BUTTON_EVENT_QUIT = 102
BUTTON_EVENT_EXPORT = 103
BUTTON_EVENT_UVTOGGLE = 104
BUTTON_EVENT_ARMATURETOGGLE = 105
BUTTON_EVENT_WORLDCOORDINATESTOGGLE = 106
BUTTON_EVENT_AMBIENTTOGGLE = 107
BUTTON_EVENT_PATHSTRING = 108
BUTTON_EVENT_PATHBUTTON = 109
BUTTON_EVENT_MATERIALSTRING = 1010
BUTTON_EVENT_SCALENUMBER = 1011
BUTTON_EVENT_ROTXNUMBER = 1012
BUTTON_EVENT_ROTYNUMBER = 1013
BUTTON_EVENT_ROTZNUMBER = 1014
BUTTON_EVENT_FPSNUMBER = 1015
BUTTON_EVENT_SCROLLBAR = 1016
BUTTON_EVENT_SCROLLBARUP = 1017
BUTTON_EVENT_SRCROLLBARDOWN = 1018
BUTTON_EVENT_UPDATEBUTTON = 1019
BUTTON_EVENT_SELECTEDOBJECTSMENU = 1020
BUTTON_EVENT_ACTUATOR_RANGESTART = 1021

exportLogger = Logger()

OGRE_LOGO = Buffer(GL_BYTE, [48,4*77],[[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,11,25,4,2,2,8,74,1,1,5,114,1,1,4,123,1,1,4,124,1,1,4,126,0,0,4,124,0,0,5,109,2,2,9,66,11,10,26,7,56,56,56,0,9,8,28,13,8,8,19,31,2,1,12,46,5,5,16,34,43,43,43,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,4,14,25,3,3,6,119,4,4,2,127,5,5,3,127,5,5,3,127,4,4,2,127,4,4,2,127,2,2,1,127,0,0,0,127,0,0,0,127,1,1,3,127,3,3,5,127,3,3,3,127,2,2,1,127,1,2,1,127,0,0,0,127,0,0,3,125,0,0,4,109,2,1,7,79,6,5,16,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,8,22,25,2,2,9,60,2,2,7,82,2,1,8,71,6,6,19,36,6,6,6,125,10,11,6,127,12,13,8,127,11,12,7,127,9,10,6,127,7,8,4,127,5,6,3,127,3,3,2,127,1,2,1,127,7,8,4,127,10,11,6,127,11,12,7,127,11,12,7,127,10,11,7,127,9,10,6,127,6,6,4,127,4,4,3,127,3,3,2,127,1,1,1,127,0,0,1,127,0,0,4,120,2,1,8,78,9,8,25,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,20,20,0,5,4,11,62,4,4,6,123,4,5,3,127,3,4,2,127,3,3,2,127,1,1,1,127,8,8,5,127,16,18,10,127,19,21,12,127,17,19,11,127,17,19,10,127,13,15,8,127,10,11,6,127,9,10,6,127,8,8,5,127,10,11,6,127,14,15,9,127,17,19,11,127,20,22,13,127,19,20,12,127,20,21,12,127,17,18,11,127,13,14,9,127,10,11,6,127,7,7,4,127,6,6,4,127,4,4,2,127,3,3,2,127,1,1,0,127,0,0,2,127,1,0,5,112,2,2,9,68,2,2,13,41,9,9,22,23,16,15,35,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,6,27,3,5,6,9,108,11,12,7,127,12,13,7,127,13,15,8,127,10,11,6,127,8,9,5,127,7,8,4,127,18,20,11,127,25,27,15,127,26,29,16,127,26,28,16,127,22,24,14,127,19,20,12,127,15,16,9,127,15,16,9,127,14,16,9,127,24,26,15,127,27,30,17,127,30,32,19,127,29,31,18,127,27,29,17,127,24,26,15,127,22,24,14,127,20,21,13,127,16,17,10,127,12,13,8,127,9,10,5,127,7,8,4,127,5,6,3,127,3,3,2,127,3,3,2,127,2,2,1,127,1,1,0,127,1,1,0,127,0,0,1,127,0,0,2,127,0,0,3,124,1,0,5,115,2,2,9,71,19,18,35,3,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,9,12,104,18,19,11,127,20,22,12,127,22,24,13,127,20,22,12,127,18,19,11,127,16,18,10,127,18,19,11,127,27,30,17,127,31,34,19,127,35,38,22,127,30,33,19,127,27,30,17,127,24,26,16,127,23,25,15,127,22,24,13,127,24,26,15,127,30,33,19,127,35,38,22,127,33,36,21,127,31,33,19,127,34,37,22,127,35,37,22,127,35,37,23,127,33,35,21,127,24,26,16,127,21,22,13,127,13,14,8,127,9,10,5,127,8,9,5,127,7,8,4,127,5,5,3,127,5,5,3,127,4,5,3,127,4,5,3,127,5,5,3,127,5,5,3,127,4,4,2,127,2,2,1,127,1,1,1,127,1,0,4,123,3,3,10,50,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,10,13,89,23,24,14,127,27,30,17,127,29,31,17,127,27,30,16,127,26,28,16,127,24,25,14,127,25,27,15,127,23,26,14,127,32,35,20,127,34,37,22,127,39,42,25,127,43,45,29,127,31,34,20,127,28,30,19,127,27,30,17,127,28,31,17,127,34,37,21,127,36,39,23,127,47,50,31,127,51,54,35,127,50,53,34,127,45,48,31,127,44,47,29,127,43,46,28,127,39,42,26,127,30,31,19,127,19,20,12,127,17,18,11,127,11,12,7,127,10,11,6,127,8,8,5,127,9,10,5,127,9,9,5,127,10,11,6,127,10,11,6,127,10,11,7,127,10,11,6,127,9,10,6,127,8,9,5,127,5,5,3,127,2,2,1,127,0,0,3,124,9,8,22,5,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,11,27,12,3,3,13,45,2,2,7,74,3,3,7,95,0,0,5,99,0,0,4,100,1,1,5,96,24,25,18,127,33,35,20,127,35,38,22,127,37,40,23,127,39,42,25,127,37,40,24,127,34,37,22,127,36,39,23,127,29,32,18,127,36,40,23,127,42,45,27,127,48,51,32,127,41,44,27,127,35,37,22,127,28,30,17,127,33,35,21,127,38,41,24,127,41,44,26,127,38,41,24,127,46,49,31,127,55,58,38,127,48,51,32,127,49,51,33,127,46,49,31,127,39,42,25,127,43,45,28,127,31,34,20,127,24,26,15,127,20,21,13,127,15,16,9,127,10,10,6,127,12,13,8,127,14,15,9,127,14,16,9,127,16,18,11,127,16,17,10,127,17,18,11,127,16,17,10,127,14,15,9,127,10,11,6,127,8,9,5,127,5,6,3,127,1,2,1,127,2,1,8,83,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,45,45,45,0,4,4,11,65,5,6,6,123,5,6,4,127,3,4,1,127,4,5,2,127,3,3,1,127,2,2,1,127,2,3,1,127,3,3,2,127,28,30,18,127,40,43,26,127,38,42,24,127,40,43,25,127,38,41,24,127,38,40,24,127,39,42,25,127,40,43,26,127,31,34,19,127,35,38,22,127,39,43,25,127,42,46,28,127,39,42,25,127,34,37,22,127,23,25,14,127,31,34,19,127,38,41,24,127,37,40,23,127,37,40,23,127,43,47,28,127,45,48,30,127,40,43,27,127,39,42,26,127,37,39,25,127,32,35,21,127,29,31,19,127,24,25,15,127,17,19,10,127,15,16,9,127,12,13,8,127,18,19,11,127,20,22,12,127,23,24,15,127,21,22,13,127,22,24,14,127,21,23,13,127,20,21,12,127,18,19,11,127,15,16,9,127,13,15,8,127,12,13,8,127,9,10,5,127,5,6,3,127,0,0,3,126,8,8,22,13],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,13,41,5,6,7,118,8,10,4,127,11,13,5,127,10,12,5,127,8,10,4,127,11,13,5,127,14,17,7,127,11,13,5,127,8,9,4,127,9,9,5,127,26,28,16,127,39,42,25,127,38,41,23,127,37,40,22,127,36,39,22,127,36,38,22,127,37,40,23,127,31,34,20,127,27,29,16,127,32,35,20,127,39,43,26,127,36,39,23,127,33,36,21,127,25,27,16,127,18,20,11,127,13,14,8,127,12,14,7,127,16,17,10,127,20,21,12,127,19,21,12,127,22,23,14,127,18,19,11,127,17,19,11,127,18,19,12,127,16,17,11,127,18,19,12,127,17,18,11,127,17,19,11,127,20,21,13,127,25,26,16,127,25,27,16,127,29,31,18,127,31,33,20,127,30,31,19,127,30,32,20,127,26,27,16,127,23,25,15,127,21,22,13,127,19,21,13,127,16,18,10,127,14,16,9,127,10,11,6,127,9,10,5,127,5,5,3,127,1,1,6,92],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,29,28,48,2,15,14,32,11,6,5,25,28,5,4,23,31,15,14,34,15,18,17,35,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,9,11,85,14,17,8,127,16,20,8,127,16,20,8,127,15,18,7,127,17,22,8,127,16,20,8,127,13,16,6,127,13,17,6,127,15,18,8,127,15,16,10,127,13,14,8,127,20,22,12,127,27,30,17,127,27,29,17,127,25,27,15,127,22,24,13,127,17,18,10,127,11,12,7,127,9,10,5,127,9,10,6,127,12,14,8,127,13,15,8,127,11,13,7,127,10,11,6,127,12,13,7,127,13,14,8,127,15,17,10,127,16,17,10,127,17,18,11,127,21,22,14,127,22,24,14,127,24,25,15,127,25,26,16,127,22,24,13,127,24,26,15,127,24,25,15,127,24,26,15,127,26,28,17,127,24,26,15,127,24,26,15,127,30,32,19,127,29,31,19,127,33,35,21,127,34,36,22,127,30,32,19,127,30,32,20,127,29,31,18,127,26,28,16,127,27,28,17,127,23,25,15,127,19,20,12,127,16,18,10,127,13,14,8,127,10,11,6,127,5,6,3,127,1,1,5,112],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,5,24,31,4,3,23,72,4,3,22,106,2,2,20,123,1,0,16,127,2,1,18,127,1,0,19,127,1,1,17,127,3,2,19,125,2,1,23,115,2,1,14,104,1,0,15,91,7,8,12,101,17,21,9,127,19,24,9,127,21,26,10,127,18,23,9,127,22,28,11,127,27,33,13,127,28,34,13,127,30,36,16,127,27,34,14,127,23,26,13,127,24,26,15,127,19,21,12,127,16,18,10,127,16,18,10,127,17,18,10,127,17,19,10,127,15,17,9,127,16,17,10,127,16,17,9,127,17,19,10,127,20,22,13,127,22,24,13,127,27,29,17,127,24,26,15,127,25,27,16,127,24,26,15,127,26,28,16,127,28,30,18,127,31,33,20,127,34,36,22,127,37,39,24,127,36,38,24,127,31,33,20,127,37,39,25,127,31,33,20,127,31,34,20,127,30,32,19,127,31,33,20,127,28,31,18,127,28,30,18,127,29,31,19,127,27,29,17,127,32,34,21,127,31,33,19,127,30,33,19,127,31,33,19,127,33,35,22,127,29,32,19,127,31,33,20,127,28,30,18,127,25,27,15,127,19,21,11,127,19,21,12,127,14,15,8,127,10,10,6,127,4,4,2,127,2,2,7,84],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,6,26,45,6,5,28,116,6,5,21,127,6,5,18,127,7,6,17,127,4,3,12,127,4,4,13,127,8,7,21,127,4,4,18,127,4,3,17,127,2,2,13,127,2,2,12,127,3,2,17,127,8,9,12,127,24,30,12,127,32,39,15,127,31,38,16,127,32,38,17,127,27,34,13,127,33,41,16,127,34,41,16,127,41,49,23,127,57,66,36,127,47,55,28,127,27,30,17,127,27,30,17,127,27,29,17,127,25,27,16,127,25,27,15,127,26,28,16,127,26,28,15,127,27,30,16,127,29,31,18,127,31,33,19,127,30,33,18,127,34,37,22,127,35,38,22,127,36,39,23,127,39,42,25,127,37,40,24,127,40,43,26,127,44,47,29,127,48,51,33,127,44,47,30,127,40,42,26,127,40,43,27,127,37,39,24,127,36,39,23,127,32,35,20,127,30,33,19,127,32,34,20,127,30,32,19,127,31,33,20,127,30,32,19,127,28,30,18,127,29,31,18,127,30,32,19,127,33,35,22,127,33,35,21,127,33,36,21,127,37,39,24,127,34,36,22,127,31,34,20,127,34,36,22,127,31,33,20,127,24,27,15,127,20,22,12,127,19,21,12,127,14,15,8,127,8,9,5,127,2,3,3,127,6,5,18,20],
[0,0,0,0,0,0,0,0,0,0,0,0,7,6,26,3,11,10,30,88,8,7,24,126,11,11,22,127,6,5,20,127,2,2,15,127,5,5,19,127,3,2,16,127,11,11,30,127,8,7,23,127,3,3,15,127,4,4,15,127,7,6,18,127,1,0,5,127,2,3,7,127,20,25,11,127,28,34,14,127,32,40,16,127,31,39,16,127,33,41,17,127,34,42,17,127,40,49,21,127,46,55,24,127,61,70,37,127,67,77,43,127,53,60,33,127,27,29,17,127,29,32,19,127,30,32,19,127,29,32,19,127,30,33,18,127,32,35,20,127,32,35,20,127,35,38,21,127,35,38,21,127,36,40,22,127,38,41,24,127,41,44,26,127,44,47,28,127,44,47,29,127,47,51,32,127,49,52,33,127,52,55,36,127,50,53,35,127,45,48,31,127,50,52,34,127,48,51,33,127,45,47,30,127,42,44,28,127,38,40,25,127,36,38,23,127,33,36,21,127,30,33,18,127,30,33,19,127,31,33,20,127,32,34,20,127,29,31,18,127,29,31,18,127,30,32,19,127,33,35,21,127,34,36,22,127,35,38,23,127,35,37,22,127,35,38,23,127,35,37,23,127,35,37,23,127,29,32,18,127,26,29,16,127,22,24,13,127,18,20,11,127,13,14,7,127,7,8,4,127,3,3,6,115,0,0,0,0],
[0,0,0,0,0,0,0,0,16,16,34,2,11,10,29,96,21,21,36,127,10,10,25,127,6,5,18,127,3,2,12,127,0,0,3,127,6,5,8,127,13,12,26,127,3,2,8,127,7,7,16,127,5,4,11,127,0,0,0,127,3,3,6,127,2,2,4,127,9,11,8,127,28,35,14,127,42,50,23,127,52,61,31,127,51,60,31,127,43,52,24,127,38,47,19,127,50,60,28,127,49,59,28,127,50,59,29,127,46,55,27,127,40,46,23,127,30,32,19,127,31,34,20,127,32,34,20,127,29,32,18,127,31,34,19,127,34,37,21,127,35,38,21,127,37,40,23,127,36,40,22,127,39,42,24,127,44,47,28,127,46,50,30,127,44,47,29,127,46,50,30,127,52,55,35,127,50,54,34,127,63,66,45,127,51,55,35,127,50,54,34,127,48,51,32,127,51,53,34,127,50,53,34,127,43,46,29,127,38,41,25,127,34,37,22,127,36,38,23,127,30,33,19,127,32,34,21,127,33,35,22,127,30,32,19,127,29,32,18,127,26,29,16,127,28,30,17,127,29,32,18,127,31,33,19,127,36,38,23,127,37,40,24,127,35,38,23,127,36,38,24,127,34,36,22,127,31,33,20,127,26,29,16,127,22,24,13,127,16,18,10,127,12,13,7,127,9,10,6,127,5,5,11,63,0,0,0,0],
[0,0,0,0,0,0,0,0,9,8,22,63,32,31,42,127,18,17,32,127,10,10,23,127,7,7,17,127,12,11,21,127,10,9,20,127,11,10,17,127,10,9,20,127,4,4,9,127,0,0,0,127,6,6,12,127,0,0,0,127,0,0,0,127,7,7,13,127,17,20,14,127,41,49,22,127,45,54,24,127,61,70,38,127,41,48,25,127,21,25,11,127,22,27,11,127,20,24,10,127,20,25,10,127,24,29,12,127,33,39,18,127,46,54,27,127,33,36,21,127,33,36,21,127,33,35,21,127,29,31,18,127,31,34,19,127,35,38,21,127,36,40,22,127,41,45,26,127,41,44,25,127,40,43,24,127,45,49,29,127,45,49,29,127,51,55,34,127,56,60,38,127,51,54,34,127,54,58,37,127,54,58,38,127,57,61,40,127,55,58,37,127,52,54,35,127,52,55,36,127,47,50,32,127,48,50,32,127,42,45,28,127,36,39,23,127,33,36,21,127,34,36,21,127,34,36,22,127,36,38,24,127,32,34,21,127,29,31,18,127,28,30,17,127,29,31,18,127,33,35,21,127,31,33,19,127,35,37,22,127,35,38,23,127,35,37,23,127,34,36,23,127,34,36,22,127,28,31,18,127,25,28,16,127,20,22,12,127,12,13,7,127,25,28,15,127,15,16,13,122,12,12,12,1,0,0,0,0],
[0,0,0,0,0,0,0,0,10,9,21,104,43,42,55,127,26,25,41,127,18,18,28,127,9,8,17,127,17,16,33,127,8,8,18,127,11,10,20,127,7,6,12,127,0,0,0,127,0,0,0,127,7,7,13,127,5,5,8,127,10,9,16,127,23,22,33,127,10,12,10,127,26,31,14,127,17,21,9,127,12,14,6,127,14,17,7,127,20,25,10,127,29,36,14,127,33,41,17,127,37,44,19,127,47,55,28,127,54,62,33,127,56,65,35,127,46,52,29,127,33,36,21,127,34,36,21,127,31,33,19,127,30,33,18,127,35,38,21,127,36,39,22,127,41,44,26,127,40,44,25,127,43,47,27,127,47,51,31,127,52,55,34,127,57,61,39,127,54,57,36,127,55,58,37,127,61,64,42,127,63,67,44,127,58,62,40,127,58,61,40,127,57,60,39,127,54,57,37,127,48,51,31,127,41,44,26,127,39,42,25,127,37,40,23,127,34,37,22,127,34,37,22,127,33,35,21,127,34,36,22,127,30,33,19,127,27,30,17,127,28,30,18,127,33,35,21,127,32,34,20,127,30,33,19,127,34,36,22,127,35,38,23,127,34,36,22,127,34,36,22,127,29,31,19,127,25,28,16,127,25,26,15,127,41,39,23,127,19,18,11,127,25,28,15,127,6,6,10,82,0,0,0,0,0,0,0,0],
[0,0,0,0,18,17,38,6,24,24,37,126,38,37,50,127,33,33,41,127,25,24,36,127,8,7,18,127,11,10,19,127,11,11,18,127,0,0,0,127,8,7,13,127,0,0,0,127,5,4,8,127,18,18,27,127,5,5,8,127,9,9,14,127,9,10,13,127,17,21,11,127,20,25,10,127,25,31,12,127,27,34,13,127,31,39,16,127,38,46,20,127,41,50,22,127,46,55,26,127,57,65,36,127,61,69,39,127,61,70,39,127,61,69,39,127,60,68,38,127,33,36,21,127,34,37,22,127,31,33,19,127,31,34,19,127,36,39,22,127,39,42,24,127,39,43,24,127,41,45,25,127,41,45,25,127,46,49,29,127,52,55,34,127,53,57,36,127,60,64,41,127,62,66,43,127,67,71,48,127,67,70,47,127,63,67,44,127,65,69,46,127,58,61,40,127,53,57,36,127,46,48,30,127,42,44,27,127,39,42,25,127,36,38,23,127,34,37,22,127,34,36,22,127,35,37,23,127,29,32,18,127,29,32,19,127,27,30,17,127,27,29,17,127,29,31,18,127,30,32,19,127,31,34,20,127,33,35,21,127,32,34,20,127,29,31,18,127,28,30,18,127,21,23,13,127,48,46,26,127,99,92,54,127,75,70,40,127,26,24,14,127,14,15,11,127,9,8,22,14,0,0,0,0,0,0,0,0],
[0,0,0,0,16,15,33,7,26,25,38,124,21,21,29,127,14,13,30,127,20,19,29,127,11,11,19,127,0,0,0,127,8,7,13,127,0,0,0,127,11,11,16,127,10,10,16,127,5,4,8,127,0,0,0,127,11,10,17,127,11,10,18,127,18,22,10,127,27,34,13,127,31,39,15,127,34,42,17,127,43,52,23,127,49,58,28,127,50,59,28,127,50,59,28,127,51,60,30,127,53,61,32,127,54,62,33,127,57,65,35,127,60,68,38,127,61,70,39,127,52,59,34,127,31,33,20,127,32,34,20,127,32,34,20,127,34,37,21,127,39,42,24,127,41,45,26,127,43,47,27,127,42,46,26,127,46,50,29,127,47,51,30,127,52,56,35,127,58,61,39,127,62,66,43,127,61,65,42,127,65,69,45,127,65,69,46,127,60,63,41,127,56,59,38,127,54,57,36,127,46,49,30,127,43,46,28,127,36,39,23,127,32,35,20,127,35,38,23,127,31,33,19,127,31,33,20,127,31,34,20,127,29,31,18,127,27,30,18,127,28,30,17,127,28,30,18,127,30,32,19,127,28,30,18,127,29,31,18,127,27,30,17,127,22,24,14,127,14,15,9,127,38,36,21,127,101,94,54,127,108,100,57,127,78,73,42,127,28,26,15,127,4,4,8,95,0,0,0,0,0,0,0,0,0,0,0,0],
[12,11,26,28,9,9,24,116,20,19,30,127,14,13,23,127,7,7,11,127,8,8,14,127,0,0,0,127,0,0,0,127,8,8,14,127,9,8,14,127,8,8,13,127,9,8,14,127,0,0,0,127,0,0,0,127,10,10,16,127,33,33,41,127,29,36,14,127,35,43,17,127,38,47,18,127,38,47,19,127,44,54,24,127,50,59,28,127,50,59,28,127,50,59,28,127,51,60,29,127,53,62,33,127,53,61,33,127,53,61,32,127,53,61,32,127,56,64,34,127,60,69,37,127,46,53,27,127,31,34,19,127,29,31,18,127,36,39,23,127,38,42,24,127,40,43,24,127,43,46,27,127,45,49,29,127,44,48,27,127,47,51,31,127,54,58,37,127,59,63,41,127,62,66,43,127,59,63,41,127,61,65,42,127,63,67,44,127,57,60,38,127,53,56,36,127,50,53,34,127,46,49,30,127,43,46,28,127,40,43,26,127,36,38,22,127,31,33,19,127,33,36,21,127,30,32,19,127,30,32,19,127,28,30,18,127,27,29,17,127,27,29,17,127,28,30,18,127,25,27,15,127,26,28,16,127,22,24,14,127,16,17,10,127,11,12,7,127,10,11,6,127,36,34,20,127,94,87,51,127,106,98,58,127,80,75,44,127,25,23,15,127,10,9,22,17,0,0,0,0,0,0,0,0,0,0,0,0],
[22,21,35,97,9,8,16,127,8,7,14,127,0,0,0,127,0,0,0,127,4,3,6,127,5,5,8,127,8,7,12,127,16,16,24,127,4,4,7,127,0,0,0,127,9,8,14,127,1,1,2,127,7,7,11,127,10,10,15,127,35,35,41,127,29,36,14,127,35,43,17,127,37,46,18,127,38,47,19,127,43,52,23,127,50,59,28,127,50,59,29,127,50,60,29,127,51,60,30,127,54,62,33,127,54,63,33,127,54,63,33,127,54,63,33,127,54,63,33,127,56,65,34,127,57,66,35,127,54,62,32,127,38,43,24,127,32,35,20,127,40,43,25,127,42,45,26,127,41,45,25,127,43,47,27,127,42,46,26,127,48,52,31,127,51,55,34,127,59,63,41,127,55,59,37,127,60,64,41,127,65,68,46,127,61,65,42,127,58,62,40,127,50,53,33,127,47,51,31,127,46,49,30,127,39,42,25,127,42,44,27,127,36,38,23,127,32,35,21,127,33,36,21,127,31,34,20,127,28,30,17,127,25,27,15,127,26,28,16,127,24,26,15,127,24,26,16,127,20,21,12,127,19,20,12,127,17,19,11,127,12,13,7,127,11,12,6,127,10,11,6,127,34,32,18,127,97,90,52,127,105,97,56,127,74,69,39,127,23,21,16,125,17,15,31,2,0,0,0,0,0,0,0,0,0,0,0,0],
[13,12,25,110,0,0,0,127,8,8,13,127,0,0,0,127,1,1,2,127,9,8,14,127,19,18,28,127,2,2,3,127,0,0,0,127,10,10,16,127,0,0,0,127,2,2,3,127,25,25,37,127,6,6,11,127,10,9,18,127,15,14,26,127,29,35,14,127,35,43,17,127,37,46,18,127,38,48,19,127,43,52,23,127,54,63,31,127,56,65,33,127,58,67,34,127,60,69,36,127,55,63,34,127,54,63,33,127,54,63,33,127,54,63,33,127,54,63,32,127,54,63,33,127,57,66,35,127,57,66,35,127,57,65,34,127,38,43,23,127,35,38,21,127,40,44,25,127,40,43,24,127,40,44,24,127,41,45,25,127,43,47,27,127,50,54,33,127,54,58,36,127,54,58,36,127,53,56,35,127,56,59,37,127,57,60,39,127,51,54,34,127,47,50,31,127,44,47,28,127,46,49,30,127,40,43,25,127,38,41,24,127,35,37,22,127,35,37,23,127,29,31,18,127,29,31,18,127,27,29,17,127,23,25,14,127,21,23,13,127,24,25,15,127,22,24,14,127,24,26,15,127,22,23,14,127,19,21,13,127,12,13,8,127,11,12,6,127,11,12,6,127,34,32,18,127,95,88,52,127,98,91,53,127,62,58,33,127,55,40,17,127,35,19,10,102,0,0,0,0,0,0,0,0,0,0,0,0],
[13,12,27,101,0,0,0,127,14,14,22,127,9,8,15,127,7,7,12,127,0,0,0,127,9,9,15,127,0,0,0,127,0,0,0,127,2,2,3,127,15,15,23,127,10,10,17,127,11,10,19,127,20,20,31,127,10,10,22,127,12,11,30,127,33,40,21,127,37,45,18,127,40,49,19,127,41,51,20,127,43,52,22,127,61,70,37,127,60,70,36,127,59,69,36,127,60,69,36,127,56,65,34,127,54,62,33,127,53,62,32,127,53,61,32,127,53,62,32,127,53,62,32,127,55,64,33,127,49,58,28,127,42,51,23,127,40,47,21,127,33,36,20,127,35,39,22,127,37,40,22,127,38,41,23,127,39,42,23,127,39,43,24,127,42,46,27,127,43,47,27,127,42,46,27,127,46,49,31,127,43,46,27,127,40,44,25,127,41,44,26,127,40,43,25,127,39,42,25,127,37,39,23,127,34,37,21,127,32,34,20,127,29,32,19,127,27,28,17,127,23,25,15,127,26,28,17,127,27,29,17,127,29,32,19,127,30,33,19,127,29,31,19,127,27,29,17,127,25,27,16,127,21,23,13,127,20,21,13,127,13,14,9,127,11,12,7,127,12,13,7,127,43,40,23,127,91,84,47,127,93,86,49,127,50,46,27,127,67,43,15,127,42,20,8,124,0,0,0,0,0,0,0,0,0,0,0,0],
[17,16,30,91,11,10,18,127,4,4,7,127,7,7,11,127,0,0,0,127,0,0,0,127,0,0,1,127,10,9,15,127,6,6,9,127,10,9,16,127,8,8,13,127,19,18,29,127,12,11,22,127,8,8,22,127,15,15,31,127,15,15,25,127,34,37,32,127,35,43,17,127,39,48,19,127,41,51,20,127,41,51,20,127,55,65,32,127,62,72,38,127,62,72,38,127,61,70,37,127,58,67,35,127,54,62,33,127,52,60,31,127,48,56,28,127,44,52,25,127,40,48,21,127,39,47,20,127,40,48,21,127,44,51,23,127,90,85,47,127,52,50,28,127,28,30,18,127,31,34,19,127,33,36,20,127,31,33,19,127,31,34,19,127,31,34,19,127,32,34,20,127,35,38,22,127,38,40,25,127,40,43,26,127,38,41,25,127,40,43,27,127,42,45,29,127,43,45,29,127,43,45,29,127,40,42,26,127,43,46,29,127,44,46,29,127,40,43,26,127,38,41,24,127,36,39,23,127,32,35,20,127,33,35,21,127,29,32,18,127,32,34,21,127,27,29,18,127,25,27,16,127,21,23,14,127,18,19,11,127,14,15,9,127,11,13,7,127,13,14,8,127,50,47,27,127,94,88,50,127,81,76,44,127,43,35,18,127,93,63,23,127,43,23,12,120,0,0,0,0,0,0,0,0,0,0,0,0],
[16,15,28,62,8,8,14,127,0,0,0,127,9,9,15,127,0,0,0,127,2,2,4,127,9,9,14,127,18,17,27,127,5,5,9,127,2,2,3,127,9,9,15,127,14,14,25,127,20,19,34,127,17,16,27,127,4,4,7,127,23,22,33,127,16,17,18,127,31,39,16,127,39,48,19,127,41,51,20,127,41,51,21,127,48,58,26,127,61,71,37,127,58,68,35,127,55,65,32,127,51,60,29,127,40,49,22,127,37,46,19,127,37,45,19,127,37,45,19,127,36,44,18,127,36,44,18,127,37,45,19,127,49,55,27,127,99,92,54,127,61,57,33,127,29,31,19,127,35,38,22,127,39,42,25,127,40,43,25,127,39,43,24,127,44,47,27,127,43,47,27,127,49,53,32,127,57,61,39,127,64,67,45,127,65,68,45,127,67,71,48,127,65,68,46,127,66,69,46,127,61,64,43,127,55,59,38,127,50,54,34,127,49,52,33,127,42,46,27,127,40,43,25,127,35,38,22,127,37,39,23,127,33,36,21,127,32,34,20,127,30,32,19,127,26,28,16,127,24,26,15,127,21,22,13,127,17,18,11,127,13,15,8,127,13,15,8,127,13,15,8,127,61,56,32,127,93,86,48,127,61,57,33,127,75,56,26,127,94,65,30,127,35,20,13,95,0,0,0,0,0,0,0,0,0,0,0,0],
[23,23,37,12,16,15,26,124,1,1,3,127,15,14,22,127,13,12,20,127,6,6,10,127,0,0,0,127,6,6,9,127,22,22,32,127,12,11,22,127,15,14,26,127,10,9,17,127,1,1,6,127,21,20,33,127,7,7,11,127,2,2,4,127,9,9,15,127,36,40,29,127,36,45,18,127,39,48,19,127,40,49,19,127,40,49,20,127,43,53,22,127,44,53,23,127,44,53,23,127,43,52,22,127,36,44,18,127,32,40,16,127,31,38,15,127,36,43,19,127,49,57,29,127,61,70,39,127,71,80,47,127,64,73,41,127,85,81,46,127,62,58,33,127,24,24,14,127,33,36,21,127,37,40,23,127,39,42,24,127,40,44,25,127,43,47,27,127,44,48,28,127,49,52,31,127,57,61,38,127,63,66,43,127,64,68,45,127,68,72,49,127,66,70,47,127,65,69,46,127,61,64,42,127,55,59,38,127,52,56,36,127,46,49,30,127,42,45,27,127,40,43,26,127,35,37,22,127,34,37,22,127,33,36,21,127,31,34,20,127,29,31,18,127,26,28,16,127,22,23,13,127,19,20,12,127,16,17,10,127,13,15,8,127,13,14,8,127,14,15,9,127,72,67,39,127,84,78,45,127,54,45,23,127,116,99,66,127,95,70,36,127,26,17,21,25,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,21,20,34,89,15,15,24,127,0,0,0,127,16,15,24,127,14,13,23,127,13,13,23,127,22,21,40,127,17,16,33,127,9,8,26,127,11,10,21,127,9,8,20,127,11,10,19,127,11,11,17,127,5,5,8,127,10,10,16,127,20,20,29,127,49,48,55,127,35,40,28,127,30,37,17,127,30,37,15,127,28,35,18,127,31,38,16,127,30,37,15,127,29,35,14,127,30,37,15,127,34,42,17,127,44,53,25,127,51,61,29,127,68,77,43,127,74,83,49,127,74,83,49,127,74,83,48,127,66,75,42,127,73,75,43,127,76,71,42,127,16,15,9,127,35,39,22,127,37,40,24,127,39,42,24,127,42,45,25,127,42,46,26,127,42,46,26,127,47,50,29,127,59,63,40,127,60,64,41,127,61,65,42,127,70,73,50,127,67,71,48,127,61,64,42,127,59,62,41,127,55,58,37,127,49,53,33,127,43,46,28,127,41,44,26,127,39,42,25,127,35,38,22,127,34,37,21,127,32,35,20,127,31,34,20,127,27,29,17,127,25,27,15,127,20,22,13,127,17,19,11,127,15,17,9,127,14,16,9,127,12,13,7,127,17,18,10,127,85,79,45,127,63,56,31,127,98,78,48,127,123,111,86,127,58,44,29,95,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,19,18,32,6,21,20,34,91,21,20,37,111,8,8,23,119,10,9,30,119,26,25,40,125,11,10,26,127,8,7,18,127,8,7,21,127,19,19,30,127,2,2,3,127,5,5,9,127,27,26,37,127,20,20,30,127,9,9,14,127,13,13,21,127,58,58,64,127,58,57,68,127,38,37,49,127,12,12,26,127,10,10,15,127,41,45,34,127,35,43,17,127,39,48,19,127,41,51,21,127,44,54,23,127,55,64,32,127,55,65,32,127,65,74,41,127,72,81,47,127,72,81,47,127,71,80,46,127,68,77,44,127,58,65,35,127,85,80,46,127,29,31,16,127,34,40,19,127,35,38,22,127,38,41,23,127,38,42,23,127,45,49,29,127,43,47,28,127,46,50,29,127,56,60,37,127,60,64,41,127,60,64,41,127,66,70,46,127,63,67,44,127,59,63,40,127,54,57,36,127,50,54,34,127,45,48,29,127,41,44,26,127,38,42,24,127,36,39,23,127,38,41,25,127,34,36,21,127,32,35,20,127,29,32,18,127,27,29,16,127,23,26,14,127,21,23,13,127,18,20,11,127,17,18,10,127,14,16,9,127,13,15,8,127,15,16,9,127,89,78,43,127,95,72,36,127,125,114,81,127,82,71,54,117,17,12,25,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,19,35,93,13,13,21,127,18,17,27,127,2,2,3,127,8,7,12,127,18,17,24,127,9,9,14,127,7,7,11,127,12,12,19,127,12,11,19,127,30,29,39,127,53,52,59,127,50,49,62,127,31,31,46,127,2,1,6,127,19,18,27,127,42,44,43,127,34,43,17,127,38,47,19,127,41,51,21,127,43,52,22,127,54,64,31,127,54,64,31,127,59,68,35,127,68,77,44,127,67,77,43,127,62,71,38,127,59,69,36,127,49,58,28,127,56,61,31,127,36,43,18,127,34,41,17,127,33,36,21,127,36,39,22,127,40,43,25,127,43,47,28,127,43,47,28,127,44,48,28,127,51,55,34,127,59,63,40,127,59,63,40,127,63,67,44,127,59,63,40,127,54,58,37,127,50,54,33,127,47,50,31,127,45,48,29,127,40,44,25,127,39,42,25,127,37,40,23,127,35,38,22,127,34,37,22,127,32,34,20,127,31,33,19,127,27,30,17,127,23,26,14,127,21,23,13,127,20,22,13,127,17,19,11,127,16,17,10,127,13,14,8,127,18,16,9,127,102,78,41,127,122,102,68,127,76,66,48,124,37,32,40,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,25,40,64,15,14,23,127,11,11,16,127,10,9,16,127,15,14,24,127,10,10,17,127,12,12,20,127,11,10,16,127,19,18,27,127,0,0,0,127,9,9,16,127,54,53,56,127,63,63,64,127,30,29,38,127,10,10,16,127,17,17,28,127,35,35,44,127,34,42,18,127,37,46,18,127,41,51,20,127,43,53,22,127,52,62,29,127,54,64,31,127,52,62,30,127,54,64,32,127,48,57,26,127,52,61,30,127,59,69,36,127,50,59,29,127,39,47,21,127,40,48,22,127,43,51,24,127,35,39,21,127,33,36,20,127,39,42,25,127,41,44,26,127,41,45,26,127,41,45,25,127,48,52,31,127,58,62,40,127,60,64,41,127,57,60,38,127,53,57,35,127,51,55,34,127,48,51,31,127,44,48,29,127,44,47,29,127,38,41,24,127,37,40,23,127,37,40,23,127,35,38,22,127,33,36,21,127,32,34,20,127,30,32,19,127,26,28,16,127,24,26,15,127,22,24,14,127,22,24,14,127,18,20,11,127,16,17,10,127,10,11,6,127,41,34,17,127,101,84,54,127,57,52,36,127,5,5,7,116,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,25,41,8,26,25,41,122,10,9,17,127,9,9,15,127,11,11,18,127,17,16,24,127,26,25,37,127,9,9,14,127,19,18,26,127,10,9,15,127,21,20,29,127,24,24,29,127,9,9,12,127,10,9,15,127,0,0,0,127,16,16,27,127,11,10,20,127,34,37,32,127,36,45,18,127,40,50,20,127,42,51,21,127,44,54,23,127,46,55,24,127,45,55,23,127,46,55,24,127,52,61,30,127,57,67,34,127,55,64,32,127,48,57,27,127,43,52,24,127,43,51,24,127,43,51,24,127,40,48,22,127,32,34,19,127,34,37,21,127,38,41,24,127,43,46,28,127,44,48,29,127,45,49,29,127,51,55,33,127,56,60,37,127,56,60,38,127,52,55,34,127,47,51,30,127,45,48,29,127,41,45,26,127,39,42,25,127,38,42,24,127,36,39,23,127,35,38,22,127,34,37,22,127,32,35,20,127,29,32,18,127,28,31,17,127,26,29,16,127,24,27,15,127,23,26,15,127,21,23,13,127,19,21,12,127,14,16,9,127,7,8,4,127,27,30,17,127,36,39,24,127,19,20,11,127,8,8,9,106,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,22,39,74,18,18,29,127,16,15,23,127,23,23,34,127,8,8,14,127,23,22,32,127,13,12,21,127,30,30,40,127,10,10,17,127,18,18,26,127,3,3,4,127,7,7,12,127,13,13,20,127,11,10,16,127,19,19,31,127,18,18,32,127,23,23,35,127,31,38,19,127,40,49,20,127,42,51,21,127,42,51,21,127,45,55,24,127,44,54,23,127,44,53,23,127,46,55,25,127,46,55,24,127,47,56,25,127,47,56,25,127,39,47,20,127,38,46,20,127,40,49,21,127,39,47,20,127,34,41,19,127,32,35,20,127,34,37,21,127,39,42,25,127,42,45,27,127,43,47,28,127,46,50,29,127,55,59,37,127,55,59,37,127,48,51,31,127,44,48,29,127,41,45,26,127,40,43,25,127,37,40,23,127,36,39,22,127,35,38,22,127,33,36,21,127,33,36,21,127,32,35,20,127,32,35,20,127,28,31,18,127,27,30,17,127,25,27,16,127,24,26,15,127,21,23,13,127,17,19,10,127,10,11,6,127,11,12,7,127,31,34,20,127,32,35,20,127,22,24,14,127,7,7,8,110,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,25,40,7,21,20,37,116,14,13,21,127,17,16,26,127,10,9,16,127,16,15,23,127,26,25,38,127,15,15,23,127,10,10,18,127,18,18,25,127,6,6,9,127,15,14,20,127,2,2,4,127,7,7,12,127,9,9,18,127,12,11,21,127,18,17,30,127,42,42,50,127,38,46,19,127,43,53,21,127,42,52,21,127,44,53,22,127,44,53,23,127,42,51,21,127,41,51,21,127,42,51,21,127,43,52,22,127,41,50,21,127,38,47,20,127,41,50,21,127,40,48,20,127,38,47,19,127,38,47,19,127,31,36,18,127,32,35,19,127,32,35,20,127,36,39,23,127,38,42,24,127,40,44,25,127,58,62,40,127,55,59,37,127,45,49,29,127,42,45,27,127,39,42,25,127,36,39,23,127,35,38,22,127,33,36,21,127,34,37,21,127,33,36,21,127,32,34,19,127,33,36,20,127,31,34,20,127,28,31,18,127,27,29,16,127,26,28,16,127,23,25,14,127,21,22,13,127,13,15,8,127,11,12,7,127,14,15,8,127,29,31,18,127,52,55,35,127,32,35,21,127,15,17,12,127,14,13,28,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,25,40,52,30,29,43,127,3,3,6,127,23,22,34,127,12,11,20,127,25,25,38,127,26,26,38,127,23,23,34,127,24,23,34,127,9,8,14,127,18,17,26,127,8,8,14,127,17,16,26,127,22,22,35,127,18,17,27,127,34,33,42,127,50,49,59,127,26,30,27,127,41,51,21,127,42,51,21,127,40,50,20,127,39,48,19,127,39,49,20,127,39,48,19,127,39,48,19,127,38,47,19,127,38,47,18,127,38,47,19,127,39,48,20,127,37,46,19,127,38,47,19,127,38,47,20,127,36,44,19,127,30,32,18,127,33,36,20,127,34,38,21,127,31,35,19,127,35,38,22,127,38,41,25,127,41,44,28,127,39,42,25,127,38,41,24,127,33,36,20,127,34,36,21,127,32,35,20,127,33,36,21,127,36,39,22,127,36,39,22,127,35,38,22,127,33,36,21,127,31,34,20,127,28,30,17,127,26,28,16,127,24,27,15,127,22,23,13,127,16,18,10,127,11,12,7,127,16,18,10,127,20,22,13,127,13,14,8,127,53,56,36,127,42,45,28,127,22,24,14,127,9,10,12,77,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,22,32,26,31,30,45,125,38,37,49,127,24,23,33,127,27,27,38,127,33,33,41,127,34,33,42,127,28,27,35,127,22,22,34,127,17,17,24,127,13,13,18,127,0,0,0,127,25,24,32,127,31,30,39,127,13,13,21,127,26,26,39,127,38,37,48,127,17,17,33,127,31,37,19,127,49,58,27,127,40,49,20,127,39,48,19,127,38,47,19,127,39,48,19,127,42,51,22,127,45,54,24,127,44,54,24,127,46,56,25,127,45,54,24,127,43,52,23,127,42,51,22,127,40,49,21,127,38,46,19,127,24,28,13,127,20,22,12,127,26,28,16,127,27,30,17,127,31,34,21,127,18,20,11,127,14,15,9,127,23,25,14,127,32,34,20,127,8,9,5,127,24,26,14,127,29,32,18,127,30,32,19,127,28,31,17,127,25,28,16,127,21,23,13,127,17,19,11,127,14,16,9,127,15,16,9,127,20,22,12,127,20,22,13,127,19,21,12,127,14,15,8,127,14,15,9,127,18,20,11,127,31,33,20,127,20,21,13,127,40,43,27,127,47,50,31,127,21,23,13,127,14,15,13,120,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24,24,32,104,40,39,48,127,33,32,43,127,50,49,57,127,28,27,40,127,27,27,41,127,22,21,34,127,22,21,33,127,26,26,34,127,23,22,33,127,19,18,28,127,12,11,17,127,33,32,42,127,35,34,46,127,17,16,24,127,27,26,37,127,13,13,21,127,23,24,30,127,15,18,7,127,53,61,33,127,18,22,9,127,19,24,10,127,36,44,18,127,34,41,18,127,26,32,14,127,25,30,12,127,27,33,15,127,32,38,18,127,27,33,14,127,33,40,18,127,46,54,26,127,47,56,27,127,40,48,21,127,29,33,17,127,32,34,19,127,33,36,21,127,21,23,13,127,24,26,15,127,41,45,28,127,31,34,21,127,11,12,7,127,15,16,9,127,8,8,5,127,27,29,18,127,30,33,19,127,33,36,22,127,33,36,21,127,31,33,19,127,27,29,17,127,25,27,16,127,20,22,13,127,12,14,8,127,9,10,5,127,16,18,10,127,16,17,10,127,12,13,7,127,16,18,10,127,20,22,12,127,25,27,15,127,22,23,14,127,40,42,27,127,49,52,33,127,28,31,17,127,24,26,15,127,9,9,16,38,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,36,36,48,92,37,36,49,127,28,27,41,127,51,50,57,127,33,33,47,127,30,30,40,127,36,35,46,127,20,20,29,127,32,31,46,127,20,20,27,127,28,27,37,127,11,10,17,127,32,31,44,127,24,23,36,127,19,18,28,127,2,2,4,127,26,25,36,127,22,27,17,127,23,29,12,127,31,37,17,127,27,34,14,127,24,29,12,127,26,32,13,127,38,46,21,127,50,58,29,127,55,64,34,127,55,63,34,127,55,64,34,127,45,53,27,127,25,30,13,127,20,24,10,127,42,49,24,127,40,48,22,127,33,37,20,127,55,50,37,127,51,49,33,127,42,45,28,127,41,45,26,127,53,57,36,127,58,61,40,127,38,42,24,127,22,24,13,127,26,28,17,127,50,53,34,127,48,51,32,127,46,48,30,127,56,56,38,127,49,48,31,127,48,44,34,127,42,42,30,127,31,32,20,127,22,24,14,127,8,9,5,127,11,12,7,127,12,13,7,127,8,9,5,127,13,15,8,127,22,24,13,127,22,24,13,127,19,21,12,127,44,46,30,127,48,52,32,127,26,29,15,127,28,30,18,127,9,10,10,105,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,22,44,4,26,25,38,73,27,26,39,124,18,18,26,127,43,43,50,127,28,28,40,127,20,19,30,127,35,34,44,127,25,24,35,127,16,16,26,127,19,19,26,127,11,10,18,127,24,23,34,127,18,17,32,127,32,32,43,127,30,29,43,127,18,17,25,127,24,25,27,127,40,49,21,127,36,44,18,127,26,32,13,127,31,38,17,127,44,53,23,127,47,57,26,127,44,53,24,127,48,56,28,127,48,56,28,127,50,58,30,127,50,59,30,127,49,57,29,127,27,33,14,127,19,23,9,127,33,40,18,127,37,40,23,127,59,55,39,127,55,48,29,127,73,68,53,127,27,29,17,127,36,40,23,127,36,40,23,127,29,32,18,127,19,21,12,127,28,30,18,127,33,35,22,127,59,58,42,127,82,74,62,127,66,59,44,127,27,25,1,127,57,49,38,127,54,50,40,127,43,41,32,127,29,30,18,127,15,16,9,127,5,6,3,127,9,10,6,127,4,4,2,127,16,18,10,127,21,22,13,127,16,17,10,127,14,16,8,127,52,55,36,127,53,56,36,127,29,32,17,127,30,34,18,127,18,19,13,127,8,8,20,17,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,52,52,52,1,28,27,37,123,10,9,24,127,8,8,21,127,14,13,30,127,28,28,42,127,40,40,50,127,39,38,44,127,22,21,30,127,19,18,30,127,20,19,31,127,15,14,22,127,20,20,35,127,25,25,40,127,26,25,42,127,19,18,35,127,26,25,39,127,19,20,27,127,28,34,13,127,22,26,11,127,14,17,7,127,11,13,5,127,24,29,12,127,43,51,23,127,52,61,30,127,58,67,35,127,54,62,33,127,40,48,23,127,46,54,27,127,45,53,27,127,21,26,10,127,24,29,12,127,25,29,14,127,62,58,45,127,60,55,39,127,31,29,21,127,28,30,19,127,29,31,18,127,38,41,24,127,35,38,23,127,28,30,19,127,13,15,8,127,10,11,6,127,11,11,8,127,43,41,32,127,70,66,50,127,55,50,28,127,67,59,46,127,64,60,47,127,53,50,39,127,30,30,21,127,18,19,11,127,3,3,2,127,5,5,3,127,5,6,3,127,4,5,3,127,10,11,6,127,10,11,6,127,6,6,3,127,42,45,28,127,44,47,30,127,32,35,20,127,37,40,23,127,21,24,12,127,6,7,9,90,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,35,34,51,6,28,27,40,125,20,20,32,127,8,8,16,127,11,10,23,127,9,9,22,127,12,12,23,127,16,15,30,127,14,13,27,127,9,9,22,127,17,16,28,127,17,17,25,127,15,15,23,127,20,19,29,127,24,23,32,127,22,21,33,127,24,23,35,127,19,21,21,127,30,37,14,127,30,37,15,127,31,37,16,127,23,28,12,127,11,13,5,127,6,8,3,127,12,15,6,127,20,24,9,127,27,32,14,127,27,33,14,127,24,29,13,127,30,35,17,127,15,18,7,127,8,9,4,127,4,4,2,127,2,3,1,127,16,18,10,127,33,35,21,127,38,41,24,127,43,47,29,127,49,53,33,127,45,49,30,127,53,57,38,127,37,39,24,127,25,28,16,127,14,16,9,127,11,12,7,127,3,3,2,127,11,10,8,127,26,25,19,127,26,25,19,127,22,20,16,127,15,15,11,127,5,5,3,127,2,2,1,127,1,1,0,127,6,7,4,127,11,12,7,127,17,18,10,127,19,21,12,127,15,16,10,127,12,13,7,127,24,27,15,127,31,34,20,127,26,29,15,127,33,36,21,127,15,16,12,126,12,11,27,9,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24,23,34,48,29,28,39,126,18,18,28,127,16,15,24,127,16,16,26,127,5,5,10,127,23,22,34,127,27,26,41,127,15,15,28,127,19,18,30,127,16,16,25,127,13,13,21,127,19,19,26,127,32,31,47,127,37,36,50,127,30,29,46,127,27,31,27,127,35,43,17,127,36,44,18,127,45,53,25,127,41,49,23,127,32,39,17,127,24,29,12,127,16,19,8,127,11,13,5,127,12,15,6,127,14,17,7,127,15,19,7,127,16,20,8,127,15,19,7,127,12,14,7,127,13,15,8,127,18,20,11,127,31,33,18,127,38,42,24,127,47,50,30,127,65,68,46,127,53,57,36,127,54,58,37,127,61,64,43,127,55,58,39,127,42,45,28,127,29,32,19,127,22,24,14,127,16,17,10,127,11,12,7,127,6,7,4,127,8,9,5,127,8,8,5,127,10,11,6,127,5,6,3,127,6,7,4,127,2,2,1,127,7,7,4,127,20,22,12,127,32,34,20,127,30,32,18,127,29,32,18,127,15,17,9,127,9,10,5,127,25,27,16,127,33,36,21,127,31,34,19,127,25,28,16,127,9,9,12,85,0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,35,34,46,91,33,33,46,127,24,23,37,127,18,18,26,127,7,7,13,127,46,45,59,127,32,31,44,127,23,23,38,127,18,18,26,127,15,14,28,127,19,19,30,127,14,13,23,127,12,12,22,127,12,11,26,127,27,26,41,127,23,23,30,127,34,41,19,127,36,45,18,127,42,51,21,127,51,60,29,127,56,65,34,127,49,58,29,127,45,53,26,127,40,47,22,127,34,41,18,127,31,38,17,127,28,34,14,127,28,34,14,127,23,29,11,127,21,23,12,127,27,30,16,127,33,36,20,127,42,45,26,127,42,46,26,127,48,52,32,127,53,57,35,127,59,62,40,127,48,52,31,127,58,62,40,127,61,65,42,127,58,61,39,127,48,51,32,127,37,41,24,127,31,34,20,127,25,28,16,127,20,23,13,127,18,20,11,127,21,23,14,127,14,15,8,127,13,15,8,127,13,14,8,127,6,7,4,127,6,7,6,127,23,25,16,127,37,41,24,127,39,43,26,127,43,46,29,127,37,40,23,127,20,22,12,127,10,11,6,127,16,17,10,127,24,26,15,127,18,20,11,127,10,10,9,125,19,18,33,4,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,18,17,33,77,17,17,28,127,20,20,29,127,37,37,45,127,34,33,40,127,24,24,35,127,28,27,35,127,7,6,13,127,2,2,4,127,6,6,12,127,3,2,7,127,5,5,13,127,12,11,25,122,11,10,25,51,17,17,30,62,25,26,37,123,35,43,21,127,39,48,19,127,39,48,19,127,42,51,22,127,51,60,29,127,61,70,37,127,67,76,43,127,64,73,40,127,58,66,36,127,50,58,30,127,44,52,26,127,33,39,17,127,27,31,15,127,34,37,21,127,38,42,24,127,42,45,27,127,44,48,29,127,42,46,27,127,43,47,27,127,46,50,30,127,49,52,32,127,44,48,29,127,44,48,28,127,44,47,28,127,42,45,26,127,42,46,27,127,40,44,26,127,35,39,22,127,32,35,20,127,30,33,19,127,28,30,18,127,25,27,15,127,21,24,13,127,17,19,11,127,10,11,6,127,4,3,11,65,19,19,31,12,18,19,18,77,24,26,19,119,42,45,28,127,46,49,30,127,33,36,21,127,32,35,20,127,15,16,10,127,10,11,6,127,13,14,8,127,12,13,7,127,6,6,12,63,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,12,27,42,18,17,31,116,8,8,15,127,14,14,23,127,8,8,16,127,18,17,29,127,6,6,12,127,6,5,12,127,10,10,20,127,6,5,14,124,6,6,17,96,11,10,28,25,0,0,0,0,0,0,0,0,17,16,33,17,18,19,24,104,36,44,18,127,44,53,23,127,53,63,31,127,54,63,32,127,45,54,25,127,43,52,23,127,48,57,27,127,51,60,30,127,50,59,29,127,43,51,24,127,30,37,15,127,24,30,12,127,29,32,17,127,33,37,20,127,35,38,22,127,38,41,24,127,43,47,28,127,43,47,27,127,46,50,30,127,49,53,33,127,49,53,32,127,51,55,34,127,43,48,28,127,39,42,24,127,37,41,23,127,37,40,23,127,37,41,23,127,36,39,22,127,35,38,21,127,33,36,21,127,32,35,20,127,27,29,17,127,21,23,13,127,7,7,10,111,15,14,29,2,0,0,0,0,0,0,0,0,0,0,0,0,18,19,21,48,26,28,21,116,38,41,25,127,44,47,29,127,30,32,18,127,24,26,15,127,6,6,4,127,13,14,8,127,9,10,10,115,0,0,0,0],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,21,41,4,12,12,28,44,13,12,24,83,11,10,20,107,11,11,21,104,10,10,18,91,11,10,23,78,13,13,28,52,16,16,31,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,36,36,36,1,19,22,17,82,33,41,17,127,37,46,18,127,43,52,22,127,49,59,28,127,51,60,29,127,38,46,19,127,35,43,17,127,32,40,15,127,32,40,16,127,25,31,12,127,20,24,9,127,17,21,9,127,10,12,10,115,20,21,19,100,34,38,21,127,42,46,27,127,43,47,28,127,43,47,27,127,47,51,31,127,44,48,28,127,43,47,28,127,43,47,27,127,41,45,26,127,38,41,23,127,37,41,23,127,36,39,22,127,33,36,20,127,33,36,21,127,28,31,18,127,24,25,16,127,15,17,14,107,10,11,13,74,12,11,27,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,12,17,46,24,26,19,113,41,44,27,127,47,50,31,127,33,35,21,127,14,16,9,127,10,11,8,127,9,9,20,25],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,17,16,45,21,26,15,108,30,37,17,127,35,43,17,127,34,42,16,127,33,41,16,127,32,40,15,127,31,39,15,127,26,32,13,127,16,19,10,127,10,13,10,108,8,8,15,41,0,0,0,0,34,34,34,0,19,20,18,93,33,36,21,127,39,42,25,127,39,42,24,127,40,44,25,127,38,42,24,127,44,48,29,127,42,46,27,127,41,45,26,127,37,41,23,127,34,38,21,127,32,36,20,127,29,33,18,127,23,26,14,127,19,20,13,127,8,9,12,69,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,17,22,34,21,22,18,99,30,33,23,125,35,38,22,127,21,22,13,127,6,6,9,92],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,15,26,10,15,17,15,68,17,21,13,103,18,22,13,110,17,21,13,100,14,17,13,80,9,11,16,44,18,18,30,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,15,19,33,22,24,18,114,32,35,21,127,34,38,21,127,37,40,23,127,35,39,22,127,38,41,24,127,36,39,22,127,34,38,22,127,32,35,20,127,29,31,18,127,21,23,15,127,19,21,15,124,24,29,15,127,10,11,19,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,14,32,3,16,17,21,52,12,12,12,96,6,6,10,106],
[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,14,24,24,15,16,17,65,17,19,16,98,21,23,17,118,23,25,19,125,22,24,18,122,19,21,16,116,16,17,14,106,14,14,14,87,12,12,22,21,0,0,0,0,15,17,23,25,17,19,24,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]])

#######################################################################################
## math functions

def matrix_translate(m, v):
  m[3][0] += v[0]
  m[3][1] += v[1]
  m[3][2] += v[2]
  return m

def matrix_multiply(b, a):
  """ matrix_multiply(b, a) = a*b
  """
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

def point_by_matrix(p, m):
  return [p[0] * m[0][0] + p[1] * m[1][0] + p[2] * m[2][0] + m[3][0],
          p[0] * m[0][1] + p[1] * m[1][1] + p[2] * m[2][1] + m[3][1],
          p[0] * m[0][2] + p[1] * m[1][2] + p[2] * m[2][2] + m[3][2]]

def vector_by_matrix(p, m):
  return [p[0] * m[0][0] + p[1] * m[1][0] + p[2] * m[2][0],
          p[0] * m[0][1] + p[1] * m[1][1] + p[2] * m[2][1],
          p[0] * m[0][2] + p[1] * m[1][2] + p[2] * m[2][2]]

def vector_normalize(v):
  global exportLogger
  l = math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])
  if l <= 0.000001:
    exportLogger.logError("error in normalize")
    return [0 , l, 0]
  return [v[0] / l, v[1] / l, v[2] / l]

def normal_by_matrix(n, m):
  m = matrix_transpose(matrix_invert(m))
  return vector_normalize(vector_by_matrix(n, m))


def vector_dotproduct(v1, v2):
  return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]

def vector_crossproduct(v1, v2):
  return [
    v1[1] * v2[2] - v1[2] * v2[1],
    v1[2] * v2[0] - v1[0] * v2[2],
    v1[0] * v2[1] - v1[1] * v2[0],
    ]


#######################################################################################
## data structures

class Material:
  def __init__(self, name, mat, texname, mode=Blender.NMesh.FaceTranspModes["SOLID"]):
    self.name = name
    self.mat = mat
    self.texture = texname
    self.mode = mode

class SubMesh:
  def __init__(self, material):
    self.material   = material
    self.vertices   = []
    self.faces      = []

  def rename_vertices(self, new_vertices):
    # Rename (change ID) of all vertices, such as self.vertices == new_vertices.
    for i in range(len(new_vertices)): new_vertices[i].id = i
    self.vertices = new_vertices

class XMLVertex:
	"""Vertex in Ogre.
	
	   @cvar threshold Floating point precicsion.
	"""
	threshold = 1e-6
	def __init__(self, position=None, normal=None, colourDiffuse=None, colourSpecular=None, texcoordList=None):
		"""Constructor.
		
		   @param position       list with x, y and z coordinate of the position
		   @param normal         list with x, y and z coordinate of the normal vector
		   @param colourDiffuse  list with RGBA floats
		   @param colourSpecular list with RGBA floats
		   @param texcoordList   list of list with u and v texture coordinates.
		"""
		self.elementDict = {}
		if position:
			self.elementDict['position'] = position
		if normal:
			self.elementDict['normal'] = normal
		if colourDiffuse:
			self.elementDict['colourDiffuse'] = colourDiffuse
		if colourSpecular:
			self.elementDict['colourSpecular'] = colourSpecular
		if texcoordList:
			self.elementDict['texcoordList'] = texcoordList
		return
	def hasPosition(self):
		return self.elementDict.has_key('position')
	def hasNormal(self):
		return self.elementDict.has_key('normal')
	def hasVertexColour(self):
		return self.elementDict.has_key('colourDiffuse') or self.elementDict.has_key('colourSpecular')
	def hasDiffuseColour(self):
		return self.elementDict.has_key('colourDiffuse')
	def hasSpecularColour(self):
		return self.elementDict.has_key('colourSpecular')
	def nTextureCoordinates(self):
		nTextureCoordinates = 0
		if self.elementDict.has_key('texcoordList'):
			nTextureCoordinates = len(self.elementDict['texcoordList'])
		return nTextureCoordinates
	def __getitem__(self, key):
		return self.elementDict[key]
	def __ne__(self, other):
		"""Tests if it differs from another Vertex.
		   
		   @param other the XMLVertex to compare with
		   @return <code>true</code> if they differ, else <code>false</code>
		"""
		return not self.__eq__(other)
	def __eq__(self, other):
		"""Tests if it is equal to another Vertex.
		
		   @param other the XMLVertex to compare with
		   @return <code>true</code> if they are equal, else <code>false</code>
		"""
		areEqual = 0
		if (self.getElements() == other.getElements()):
			compared = 0
			itemIterator = self.elementDict.iteritems()
			while (not compared):
				try:
					(element, value) = itemIterator.next()
					if element == 'position' or element == 'normal':
						otherValue = other[element]
						if ((math.fabs(value[0] - otherValue[0]) > XMLVertex.threshold) or
							(math.fabs(value[1] - otherValue[1]) > XMLVertex.threshold) or
							(math.fabs(value[2] - otherValue[2]) > XMLVertex.threshold)):
							# test fails
							compared = 1
					elif element == 'colourDiffuse' or element == 'colourSpecular':
						otherValue = other[element]
						if ((math.fabs(value[0] - otherValue[0]) > XMLVertex.threshold) or
							(math.fabs(value[1] - otherValue[1]) > XMLVertex.threshold) or
							(math.fabs(value[2] - otherValue[2]) > XMLVertex.threshold) or
							(math.fabs(value[3] - otherValue[3]) > XMLVertex.threshold)):
							# test fails
							compared = 1
					elif element == 'texcoordList':
						otherValue = other[element]
						if len(value) == len(otherValue):
							for uv, otherUV in zip(value, otherValue):
								if ((math.fabs(uv[0] - otherUV[0]) > XMLVertex.threshold) or
									(math.fabs(uv[1] - otherUV[1]) > XMLVertex.threshold)):
									# test fails
									compared = 1
						else:
							# test fails
							compared = 1
					else:
						# test fails, unknown element
						compared = 1
				except StopIteration:
					# objects are equal
					areEqual = 1
					compared = 1
		return areEqual
	# getter and setter
	def getElements(self):
		return self.elementDict.keys()
	def getPosition(self):
		return self.elementDict['position']
	def getNormal(self):
		return self.elementDict['normal']
	def getColourDiffuse(self):
		return self.elementDict['colourDiffuse']
	def getColourSpecular(self):
		return self.elementDict['colourSpecular']
	def getTextureCoordinatesList(self):
		return self.elementDict['texcoordList']
	def setPosition(self, position):
		if position:
			self.elementDict['position'] = position
		else:
			del self.elementDict['position']
		return
	def setNormal(self, normal):
		if normal:
			self.elementDict['normal'] = normal
		else:
			del self.elementDict['normal']
		return
	def setColourDiffuse(self, colourDiffuse):
		if colourDiffuse:
			self.elementDict['colourDiffuse'] = colourDiffuse
		else:
			del self.colourDiffuse
		return
	def setColourSpecular(self, colourSpecular):
		if colourSpecular:
			self.elementDict['colourSpecular'] = colourSpecular
		else:
			del self.elementDict['colourSpecular']
		return
	# special getter and setter
	def appendTextureCoordinates(self, uvList):
		"""Appends new texture coordinate.
		
		   @param uvList list with u and v coordinate
		   @return list index
		"""
		if self.elementDict.has_key('texcoordList'):
			self.elementDict['texcoordList'].append(uvList)
		else:
			self.elementDict['texcoordList'] = [uvList]
		return (len(self.elementDict['texcoordList']) -1 )
	def setTextureCorrdinates(self, index, uvList):
		self.elementDict['texcoordList'][index] = uvList
		return
	def getTextureCoordinates(self, index=None):
		return self.elementDict['texcoordList'][index]
	def deleteTextureCoordinates(self, index=None):
		"""Delete texture coordinates.
		
		   Delete a pair or all texture coordinates of the vertex.
		   
		   @param index the index of the texture coordinates in the vertex's list of
		                texture coordinates. If <code>None</code> the complete list
		                is deleted.
		"""
		if (index != None):
			del self.elementDict['texcoordList'][index]
		else:
			del self.elementDict['texcoordList']
		return

class XMLVertexStringView:
	"""Viewer class for textual representation of a XMLVertex. 
	
	   @see XMLVertex
	"""
	def __init__(self, xmlVertex):
		if isinstance(xmlVertex, XMLVertex):
			self.xmlVertex = xmlVertex
		return
	def __str__(self):
		return self.toString()
	def toString(self, indent=0, keyList=None):
		"""Returns textual representations of its XMLVertex.
		
		   @param indent Indentation level of the string.
		   @param keyList List of keys of elements to represent in the string.
		   @return string String representation of the XMLVertex.
		   @see XMLVertex#__init__
		"""
		if not keyList:
			keyList = self.xmlVertex.getElements()
		else:
			# remove unavailable elements
			keyList = [key for key in keyList if key in self.xmlVertex.getElements()]
		s = self._indent(indent) + "<vertex>\n"
		if keyList.count('position'):
			position = self.xmlVertex.getPosition()
			s += self._indent(indent+1)+"<position x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" % tuple(position)
		if keyList.count('normal'):
			normal = self.xmlVertex.getNormal()
			s += self._indent(indent+1)+"<normal x=\"%.6f\" y=\"%.6f\" z=\"%.6f\"/>\n" % tuple(normal)
		if keyList.count('colourDiffuse'):
			colourDiffuse = self.xmlVertex.getColourDiffuse()
			s += self._indent(indent+1)+"<colour_diffuse value=\"%.6f %.6f %.6f %.6f\"/>\n" % tuple(colourDiffuse)
		if keyList.count('colourSpecular'):
			colourSpecular = self.xmlVertex.getColourSpecular()
			s += self._indent(indent+1)+"<colour_specular value=\"%.6f %.6f %.6f %.6f\"/>\n" % tuple(colourSpecular)
		if keyList.count('texcoordList'):
			for uv in self.xmlVertex.getTextureCoordinatesList():
				s+=self._indent(indent+1)+"<texcoord u=\"%.6f\" v=\"%.6f\"/>\n" % tuple(uv)
		s += self._indent(indent) + "</vertex>\n"
		return s
	def _indent(self, indent):
		return "  "*indent

class Vertex:
  def __init__(self, submesh, xmlVertex):
    self.xmlVertex = xmlVertex
    self.influences = []
    
    self.cloned_from = None
    self.clones      = []
    self.submesh = submesh
    self.id = len(submesh.vertices)
    submesh.vertices.append(self)

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
  def __init__(self, skeleton, parent, name, loc, rotQuat, conversionMatrix):
    self.parent = parent
    self.name   = name
    self.loc = loc # offset from parent bone
    self.rotQuat = rotQuat # axis as quaternion
    self.children = []
    self.conversionMatrix = conversionMatrix # converts Blender's local bone coordinates into Ogre's local bone coordinates

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
  def __init__(self, track, time, loc, rotQuat, scale):
    self.time = time
    self.loc  = loc
    self.rotQuat  = rotQuat
    self.scale = scale
    
    self.track = track
    track.keyframes.append(self)

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

#######################################################################################
## Mesh stuff

# remap vertices for faces
def process_face(face, submesh, mesh, matrix, skeleton=None):
	"""Process a face of a mesh.
	
	   @param face Blender.NMesh.NMFace.
	   @param submesh SubMesh the face belongs to.
	   @param mesh Blender.NMesh.NMesh the face belongs to.
	   @param matrix Export translation.
	   @param skeleton Skeleton of the mesh (if any).
	"""
	global verticesDict
	global exportLogger
	# threshold to compare floats
	threshold = 1e-6
	if len(face.v) in [ 3, 4 ]:
		if not face.smooth:
			# calculate the face normal.
			p1 = face.v[0].co
			p2 = face.v[1].co
			p3 = face.v[2].co
			faceNormal = vector_crossproduct(
				[p3[0] - p2[0], p3[1] - p2[1], p3[2] - p2[2]],
				[p1[0] - p2[0], p1[1] - p2[1], p1[2] - p2[2]],
				)
			faceNormal = normal_by_matrix(faceNormal, matrix)

		face_vertices = [ 0, 0, 0, 0]
		for i in range(len(face.v)):
			# position
			position  = point_by_matrix (face.v[i].co, matrix)
			# Blender separates normal, uv coordinates and colour from vertice coordinates.
			# normal
			if face.smooth:
				normal = normal_by_matrix(face.v[i].no, matrix)
			else:
				normal = faceNormal
			xmlVertex = XMLVertex(position, normal)
			# uv coordinates
			if submesh.material.texture:
				uv = [0,0]
				if mesh.hasVertexUV():
					# mesh has sticky/per vertex uv coordinates
					uv[0] = face.v[i].uvco[0]
					# origin is now in the top-left (Ogre v0.13.0)
					uv[1] = 1 - face.v[i].uvco[1]
				else:
					# mesh has per face vertex uv coordinates
					uv[0] = face.uv[i][0]
					# origin is now in the top-left (Ogre v0.13.0)
					uv[1] = 1 - face.uv[i][1]
				xmlVertex.appendTextureCoordinates(uv)
			# vertex colour
			if submesh.material.mat:
				if (submesh.material.mat.mode & Blender.Material.Modes["VCOL_PAINT"]):
					colour = face.col[i]
					xmlVertex.setColourDiffuse([colour.r/255.0, colour.g/255.0, colour.b/255.0, colour.a/255.0])
			# check if an equal xmlVertex already exist
			# get vertex 
			if verticesDict.has_key(face.v[i].index):
				# vertex already exists
				vertex = verticesDict[face.v[i].index]
				# compare xmlVertex to vertex and its clones
				if (vertex.xmlVertex != xmlVertex):
					vertexFound = 0
					iClone = 0
					while ((iClone < len(vertex.clones)) and (not vertexFound)):
						clone = vertex.clones[iClone]
						if (clone.xmlVertex == xmlVertex):
							vertexFound = 1
							vertex = clone
						iClone += 1
					if not vertexFound:
						# create new clone
						clone = Vertex(submesh, xmlVertex)
						clone.cloned_from = vertex
						clone.influences = vertex.influences
						vertex.clones.append(clone)
						# write back to dictionary
						verticesDict[face.v[i].index] = vertex
						vertex = clone
			else:
				# vertex does not exist yet
				# create vertex
				vertex = Vertex(submesh, xmlVertex)
				# set bone influences
				if skeleton:
					influences = mesh.getVertexInfluences(face.v[i].index)
					if not influences:
						exportLogger.logError("Vertex in skinned mesh without influence, check your mesh!")
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
				verticesDict[face.v[i].index] = vertex
			# postcondition: vertex is current vertex
			face_vertices[i] = vertex
		
		if len(face.v) == 3:
			Face(submesh, face_vertices[0], face_vertices[1], face_vertices[2])
		elif len(face.v) == 4:
			# Split faces with 4 vertices on the shortest edge
			differenceVectorList = [[0,0,0],[0,0,0]]
			for indexOffset in range(2):
				for coordinate in range(3):
					differenceVectorList[indexOffset][coordinate] = face_vertices[indexOffset].xmlVertex.getPosition()[coordinate] \
					                                              - face_vertices[indexOffset+2].xmlVertex.getPosition()[coordinate]
			if Mathutils.Vector(differenceVectorList[0]).length < Mathutils.Vector(differenceVectorList[1]).length:
				Face(submesh, face_vertices[0], face_vertices[1], face_vertices[2])
				Face(submesh, face_vertices[2], face_vertices[3], face_vertices[0])
			else:
				Face(submesh, face_vertices[0], face_vertices[1], face_vertices[3])
				Face(submesh, face_vertices[3], face_vertices[1], face_vertices[2])
	else:
		exportLogger.logWarning("Ignored face with %d edges." % len(face.v))
	return

def export_mesh(object, exportOptions):
	global uvToggle, armatureToggle
	global verticesDict
	global skeletonsDict
	global materialsDict
	global exportLogger
	
	if (object.getType() == "Mesh"):
		# is this mesh attached to an armature?
		skeleton = None
		if armatureToggle.val:
			parent = object.getParent()
			#if parent and parent.getType() == "Armature" and (not skeletonsDict.has_key(parent.getName())):
			if (parent and (parent.getType() == "Armature")):
				if armatureActionActuatorListViewDict.has_key(parent.getName()):
					actionActuatorList = armatureActionActuatorListViewDict[parent.getName()].armatureActionActuatorList
					armatureExporter = ArmatureExporter(MeshExporter(object), parent)
					armatureExporter.export(actionActuatorList, exportOptions, exportLogger)
					skeleton = armatureExporter.skeleton
					#export_skeleton(parent, object)
					#skeleton = skeletonsDict[parent.getName()]

		#NMesh of the object
		data = object.getData()
		matrix = None
		if worldCoordinatesToggle.val:
			matrix = object.getMatrix("worldspace")
		else:
			matrix = Mathutils.Matrix([1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1])
		matrix = matrix*BASE_MATRIX
		# materials of the object
		# note: ogre assigns different textures and different facemodes
		#       to different materials
		objectMaterialDict = {}
		# faces assign to objectMaterial keys
		objectMaterialFacesDict = {}

		# note: these are blender materials. Evene if nMaterials = 0
		#       the face can still have a texture (see above)
		meshMaterialList = data.getMaterials(1)
		# note: material slots may be empty, resp. meshMaterialList entries may be None
		nMaterials = len(meshMaterialList)

		# create ogre materials
		for face in data.faces:
			if not(face.mode & Blender.NMesh.FaceModes["INVISIBLE"]):
				# face is visible
				hasTexture = 0
				# texture image name
				textureFile = None
				if ((uvToggle.val) and (data.hasFaceUV()) and (face.mode & Blender.NMesh.FaceModes["TEX"])):
					if face.image:
						textureFile = face.image.filename
						hasTexture = 1
					else:
						# check if image texture is assigend as material texture
						if (nMaterials > 0):
							# check if non-empty material slot
							if meshMaterialList[face.materialIndex]:
								for materialTexture in meshMaterialList[face.materialIndex].getTextures():
									if ((materialTexture is not None) \
										and (materialTexture.mapto & Blender.Texture.MapTo['COL']) \
										and (materialTexture.texco & Blender.Texture.TexCo['UV']) \
										and (materialTexture.tex.type == Blender.Texture.Types.IMAGE)):
											# use image as material.texture
											textureFile = materialTexture.tex.image.filename
											hasTexture = 1
						if not hasTexture:
							exportLogger.logError("Face is textured but has no image assigned!")
				if ((nMaterials > 0 ) or (hasTexture == 1)):
					# create material of the face:
					# blenders material name / FaceTranspMode / texture image name
					# blenders material name
					materialName = ""
					# blenders material name
					faceMaterial = None
					if (nMaterials > 0):
						faceMaterial = meshMaterialList[face.materialIndex]
						if faceMaterial:
							materialName += faceMaterial.getName() +"/"
					# FaceTranspMode
					# default: solid
					blendMode = Blender.NMesh.FaceTranspModes["SOLID"]
					if (face.transp == Blender.NMesh.FaceTranspModes["ALPHA"]):
						materialName += "ALPHA/"
						blendMode = Blender.NMesh.FaceTranspModes["ALPHA"]
					elif (face.transp == Blender.NMesh.FaceTranspModes["ADD"]):
						materialName += "ADD/"
						blendMode = Blender.NMesh.FaceTranspModes["ADD"]
					else:
						materialName += "SOLID/"
					if hasTexture:
						materialName += PathName(textureFile).basename()
					# insert into Dicts
					material = objectMaterialDict.get(materialName)
					if not material:
						if hasTexture:
							# log texture filename problems
							pathName = PathName(textureFile)
							pathName.addLogger(exportLogger)
							pathName.basename()
						material = Material(materialName, faceMaterial, textureFile, blendMode)
						objectMaterialDict[materialName] = material
						# faces
						objectMaterialFacesDict[materialName] = [face]
					else:
						# append faces
						faceList = objectMaterialFacesDict[materialName]
						faceList.append(face)
						objectMaterialFacesDict[materialName] = faceList
		# process faces
		submeshes = []
		for materialKey in objectMaterialDict.keys():
			submesh = SubMesh(objectMaterialDict[materialKey])
			verticesDict = {}
			for face in objectMaterialFacesDict[materialKey]:
				process_face(face, submesh, data, matrix, skeleton)
			if len(submesh.faces):
				submeshes.append(submesh)
				# update global materialsDict
				material = materialsDict.get(materialKey)
				if not material:
					materialsDict[materialKey] = objectMaterialDict[materialKey]
		# write mesh
		if len(submeshes) == 0:
			# no submeshes
			exportLogger.logWarning("Mesh %s has no visible faces!" % data.name)
		else:
			# write mesh
			write_mesh(data.name, submeshes, skeleton)
	return

#######################################################################################
## file output

def tab(tabsize):
    return "\t" * tabsize
    
def clamp(val):
    if val < 0.0:
        val = 0.0
    if val > 1.0:
        val = 1.0
    return val

def convertXMLFile(filename):
	"""Calls the OgreXMLConverter on a file.
	   
	   If the script variable <code>OGRE_XML_CONVERTER</code> is nonempty, the
	   OgreXMLConverter is called to convert the given file.
	   
	   @param filename filename of the XML file to convert.
	"""
	global exportLogger
	if OGRE_XML_CONVERTER != '':
		commandLine = OGRE_XML_CONVERTER + ' "' + filename + '"'
		exportLogger.logInfo("Running OgreXMLConverter: " + commandLine)
		xmlConverter = os.popen(commandLine, 'r')
		if xmlConverter == None:
			exportLogger.logError('Could not run OgreXMLConverter!')
		else:
			for line in xmlConverter:
				exportLogger.logInfo("OgreXMLConverter: " + line)
			xmlConverter.close()
	return

def write_mesh(name, submeshes, skeleton):
  global pathString, exportLogger
  file = name+".mesh.xml"
  exportLogger.logInfo("Mesh \"%s\"" % file)

  f = open(os.path.join(pathString.val, file), "w")
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

    if (armatureToggle.val):
      # use seperate vertexbuffer for position and normals when animated
      f.write(tab(4)+"<vertexbuffer positions=\"true\" normals=\"true\">\n")
      for v in submesh.vertices:
        f.write(XMLVertexStringView(v.xmlVertex).toString(5, ['normal','position']))
      f.write(tab(4)+"</vertexbuffer>\n")
      if submesh.material.mat:
        # Blender material
        if submesh.material.texture or (submesh.material.mat.mode & Blender.Material.Modes["VCOL_PAINT"]):
          f.write(tab(4)+"<vertexbuffer")
          if submesh.material.texture:
              f.write(" texture_coord_dimensions_0=\"2\" texture_coords=\"1\"")
          if (submesh.material.mat.mode & Blender.Material.Modes["VCOL_PAINT"]):
              f.write(" colours_diffuse=\"true\"")
          f.write(">\n")
          for v in submesh.vertices:
            f.write(XMLVertexStringView(v.xmlVertex).toString(5, ['texcoordList','colourDiffuse']))
          f.write(tab(4)+"</vertexbuffer>\n")
      elif submesh.material.texture:
        # texture only
        f.write(tab(4)+"<vertexbuffer texture_coord_dimensions_0=\"2\" texture_coords=\"1\">\n")
        for v in submesh.vertices:
          f.write(XMLVertexStringView(v.xmlVertex).toString(5, ['texcoordList','colourDiffuse']))
        f.write(tab(4)+"</vertexbuffer>\n")
    else:
      # use only one vertex buffer if mesh is not animated
      f.write(tab(4)+"<vertexbuffer ")
      f.write("positions=\"true\" ")
      f.write("normals=\"true\"")
      if submesh.material.texture:
        f.write(" texture_coord_dimensions_0=\"2\" texture_coords=\"1\"")
      if (submesh.material.mat.mode & Blender.Material.Modes["VCOL_PAINT"]):
        f.write(" colours_diffuse=\"true\"")
      f.write(">\n")
      for v in submesh.vertices:
        f.write(XMLVertexStringView(v.xmlVertex).toString(5))
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
  convertXMLFile(os.path.join(pathString.val, file))
  return

def writeNormalMapMaterial(file, colorImage, norImage):
	file.write("""	technique
	{
		pass
		{
			ambient 1 1 1
			diffuse 0 0 0 
			specular 0 0 0 0
			vertex_program_ref Ogre/BasicVertexPrograms/AmbientOneTexture
			{
				param_named_auto worldViewProj worldviewproj_matrix
				param_named_auto ambient ambient_light_colour
			}
		}
		pass
		{
			ambient 0 0 0 
			iteration once_per_light
			scene_blend add
			vertex_program_ref Examples/BumpMapVPSpecular
			{
				param_named_auto lightPosition light_position_object_space 0
				param_named_auto eyePosition camera_position_object_space
				param_named_auto worldViewProj worldviewproj_matrix
			}
			fragment_program_ref Examples/BumpMapFPSpecular
			{
				param_named_auto lightDiffuse light_diffuse_colour 0 
				param_named_auto lightSpecular light_specular_colour 0
			}
			texture_unit
			{
				texture %s
				colour_op replace
			}
			texture_unit
			{
				cubic_texture nm.png combinedUVW
				tex_coord_set 1
				tex_address_mode clamp
			}
			texture_unit
			{
				cubic_texture nm.png combinedUVW
				tex_coord_set 2
				tex_address_mode clamp
			}
		}
		pass
		{
			lighting off
			vertex_program_ref Ogre/BasicVertexPrograms/AmbientOneTexture
			{
				param_named_auto worldViewProj worldviewproj_matrix
				param_named ambient float4 1 1 1 1
			}
			scene_blend dest_colour zero
			texture_unit
			{
				texture %s
			}
		}
	}
	technique
	{
		pass
		{
			ambient 1 1 1
			diffuse 0 0 0 
			specular 0 0 0 0
			vertex_program_ref Ogre/BasicVertexPrograms/AmbientOneTexture
			{
				param_named_auto worldViewProj worldviewproj_matrix
				param_named_auto ambient ambient_light_colour
			}
		}
		pass
		{
			ambient 0 0 0 
			iteration once_per_light
			scene_blend add
			vertex_program_ref Examples/BumpMapVP
			{
				param_named_auto lightPosition light_position_object_space 0
				param_named_auto eyePosition camera_position_object_space
				param_named_auto worldViewProj worldviewproj_matrix
			}
			texture_unit
			{
				texture %s
				colour_op replace
			}
			texture_unit
			{
				cubic_texture nm.png combinedUVW
				tex_coord_set 1
				tex_address_mode clamp
				colour_op_ex dotproduct src_texture src_current
				colour_op_multipass_fallback dest_colour zero
			}
		}
		pass
		{
			lighting off
			vertex_program_ref Ogre/BasicVertexPrograms/AmbientOneTexture
			{
				param_named_auto worldViewProj worldviewproj_matrix
				param_named ambient float4 1 1 1 1
			}
			scene_blend dest_colour zero
			texture_unit
			{
				texture %s
			}
		}
	}
}
""" % (colorImage, norImage, colorImage, norImage))	
	return

def write_materials():
	global ambientToggle, pathString, materialString, exportLogger
	global materialsDict
	file = materialString.val
	exportLogger.logInfo("Materials \"%s\"" % file)

	f = open(os.path.join(pathString.val, file), "w")
	for name, material in materialsDict.items():
		# material
		f.write("material %s\n" % name)
		f.write("{\n")
		# receive_shadows <- SHADOW
		if material.mat:
			if (material.mat.mode & Blender.Material.Modes["SHADOW"]):
				f.write(tab(1)+"receive_shadows on\n")
			else:
				f.write(tab(1)+"receive_shadows off\n")
		# check material texture maps
		hasNorMap = 0
		iMaterialTexture = 0
		if material.mat and material.texture:
			while ((not hasNorMap) and (iMaterialTexture < 8)):
				materialTexture = material.mat.getTextures()[iMaterialTexture]
				iMaterialTexture += 1
				if ((materialTexture is not None) \
					and (materialTexture.mapto & Blender.Texture.MapTo['NOR']) \
					and (materialTexture.texco & Blender.Texture.TexCo['UV']) \
					and (materialTexture.tex.type == Blender.Texture.Types.IMAGE)):
						# face has uv texture and material has enabled image texture, map input uv, map to nor
						writeNormalMapMaterial(f, PathName(materialTexture.tex.image.filename).basename(), PathName(material.texture).basename())
						hasNorMap = 1
		if not hasNorMap:
			# without material texture maps
			# technique
			f.write(tab(1)+"technique\n")
			f.write(tab(1)+"{\n")
			# pass
			f.write(tab(2)+"pass\n")
			f.write(tab(2)+"{\n")
			if material.mat:
				# pass attributes
				mat = material.mat
				## ambient
				if (not(mat.mode & Blender.Material.Modes["TEXFACE"])
					and not(mat.mode & Blender.Material.Modes["VCOL_PAINT"])
					and (ambientToggle.val)):
					ambientRGBList = mat.rgbCol
				else:
					ambientRGBList = [1.0, 1.0, 1.0]
				# ambient <- amb * ambient RGB
				ambR = clamp(mat.amb * ambientRGBList[0])
				ambG = clamp(mat.amb * ambientRGBList[1])
				ambB = clamp(mat.amb * ambientRGBList[2])
				f.write(tab(3)+"ambient %f %f %f\n" % (ambR, ambG, ambB))
				## diffuse
				if (not(mat.mode & Blender.Material.Modes["TEXFACE"])
					and not(mat.mode & Blender.Material.Modes["VCOL_PAINT"])):
					# diffuse <- rgbCol
					diffR = clamp(mat.rgbCol[0])
					diffG = clamp(mat.rgbCol[1])
					diffB = clamp(mat.rgbCol[2])
					f.write(tab(3)+"diffuse %f %f %f\n" % (diffR, diffG, diffB))
				## specular
				# specular <- spec * specCol, hard
				specR = clamp(mat.spec * mat.specCol[0])
				specG = clamp(mat.spec * mat.specCol[1])
				specB = clamp(mat.spec * mat.specCol[2])
				specShine = mat.hard
				f.write(tab(3)+"specular %f %f %f %f\n" % (specR, specG, specB, specShine))
				## emissive
				if(not(mat.mode & Blender.Material.Modes["TEXFACE"])
					and not(mat.mode & Blender.Material.Modes["VCOL_PAINT"])):
					# emissive <-emit * rgbCol
					emR = clamp(mat.emit * mat.rgbCol[0])
					emG = clamp(mat.emit * mat.rgbCol[1])
					emB = clamp(mat.emit * mat.rgbCol[2])
					f.write(tab(3)+"emissive %f %f %f\n" % (emR, emG, emB))
				# depth_func  <- ZINVERT; ENV
				if (mat.mode & Blender.Material.Modes["ENV"]):
					f.write(tab(3)+"depth_func always_fail\n")
				elif (mat.mode & Blender.Material.Modes["ZINVERT"]):
					f.write(tab(3)+"depth_func greater_equal\n")
				# lighting <- SHADELESS
				if (mat.mode & Blender.Material.Modes["SHADELESS"]):
					f.write(tab(3)+"lighting off\n")
				# fog_override <- NOMIST
				if (mat.mode & Blender.Material.Modes["NOMIST"]):
					f.write(tab(3)+"fog_override true\n")
			elif not(material.texture):
				# default material
				f.write(tab(3)+"ambient 0.5 0.22 0.5\n")
				f.write(tab(3)+"diffuse 1.0 0.44 0.1\n")
				f.write(tab(3)+"specular 0.5 0.22 0.5 50.0\n")
				f.write(tab(3)+"emissive 0.0 0.0 0.0\n")
			# scene_blend <- transp
			if (material.mode == Blender.NMesh.FaceTranspModes["ALPHA"]):
				f.write(tab(3)+"scene_blend alpha_blend \n")
			elif (material.mode == Blender.NMesh.FaceTranspModes["ADD"]):
				f.write(tab(3)+"scene_blend add\n")
			if material.texture:
				f.write(tab(3)+"texture_unit\n")
				f.write(tab(3)+"{\n")
				f.write(tab(4)+"texture %s\n" % PathName(material.texture).basename())
				f.write(tab(3)+"}\n") # texture_unit
			f.write(tab(2)+"}\n") # pass
			f.write(tab(1)+"}\n") # technique
		f.write("}\n\n") # material
	f.close()

#######################################################################################
## main export

def export(selectedObjectsList):
    global pathString, scaleNumber, rotXNumber, rotYNumber, rotZNumber
    global materialsDict
    global skeletonsDict
    global BASE_MATRIX
    global exportLogger
    
    materialsDict = {}
    skeletonsDict = {}

    # default: set matrix to 90 degree rotation around x-axis
    # rotation order: x, y, z
    # WARNING: Blender uses left multiplication!
    rotationMatrix = Mathutils.RotationMatrix(rotXNumber.val,4,'x')
    rotationMatrix *= Mathutils.RotationMatrix(rotYNumber.val,4,'y')
    rotationMatrix *= Mathutils.RotationMatrix(rotZNumber.val,4,'z')
    scaleMatrix = Mathutils.Matrix([scaleNumber.val,0,0],[0,scaleNumber.val,0],[0,0,scaleNumber.val])
    scaleMatrix.resize4x4()
    BASE_MATRIX = rotationMatrix*scaleMatrix

    exportOptions = ExportOptions(rotXNumber.val, rotYNumber.val, rotZNumber.val, scaleNumber.val,
        worldCoordinatesToggle.val, ambientToggle.val, pathString.val, materialString.val)

    if not os.path.exists(pathString.val):
      exportLogger.logError("Invalid path: "+pathString.val)
    else:
      exportLogger.logInfo("Exporting selected objects into \"" + pathString.val + "\":")
      n = 0
      for obj in selectedObjectsList:
          if obj:
              if obj.getType() == "Mesh":
                  exportLogger.logInfo("Exporting object \"%s\":" % obj.getName())
                  export_mesh(obj, exportOptions)
                  n = 1
              elif obj.getType() == "Armature":
                  exportLogger.logInfo("Exporting object \"%s\":" % obj.getName())
                  actionActuatorList = armatureActionActuatorListViewDict[obj.getName()].armatureActionActuatorList
                  armatureMeshExporter = ArmatureMeshExporter(obj)
                  armatureMeshExporter.export(materialsDict, actionActuatorList, exportOptions, exportLogger)
      if n == 0:
          exportLogger.logWarning("No mesh objects selected!")
      elif len(materialsDict) == 0:
          exportLogger.logWarning("No materials or textures defined!")
      else:
          write_materials()
      
      exportLogger.logInfo("Finished.")
    return exportLogger.getStatus()
    
#######################################################################################
## GUI

######
# global variables
######
# see above

######
# methods
######
def saveSettings():
	"""Save all exporter settings of selected and unselected objects into a blender text object.
	
	   Settings are saved to the text 'ogreexport.cfg' inside the current .blend file. Settings
	   belonging to removed objects in the .blend file will not be saved.
	   
	   @return <code>true</code> on success, else <code>false</code>
	"""
	global uvToggle
	global armatureToggle
	global worldCoordinatesToggle
	global ambientToggle
	global pathString
	global materialString
	global scaleNumber
	global rotXNumber, rotYNumber, rotZNumber
	global fpsNumber
	global selectedObjectsList
	global armatureDict
	global armatureActionActuatorListViewDict
	global armatureAnimationDictListDict
	settingsDict = {}
	success = 0
	# save general settings
	settingsDict['uvToggle'] = uvToggle.val
	settingsDict['armatureToggle'] = armatureToggle.val
	settingsDict['worldCoordinatesToggle'] = worldCoordinatesToggle.val
	settingsDict['ambientToggle'] = ambientToggle.val
	settingsDict['pathString'] = pathString.val
	settingsDict['materialString'] = materialString.val
	settingsDict['scaleNumber'] = scaleNumber.val
	settingsDict['rotXNumber'] = rotXNumber.val
	settingsDict['rotYNumber'] = rotYNumber.val
	settingsDict['rotZNumber'] = rotZNumber.val
	if (Blender.Get("version") < 233):
		settingsDict['fpsNumber'] = fpsNumber.val
	else:
		# get blender's "scene->format->frames per second" setting
		settingsDict['fpsNumber'] = Blender.Scene.GetCurrent().getRenderingContext().framesPerSec()
	# save object specific settings
	# check if armature exists (I think this is cleaner than catching NameError exceptions.)
	# create list of valid armature names
	armatureNameList = []
	for object in Blender.Object.Get():
		if (object.getType() == "Armature"):
			armatureNameList.append(object.getName())
	for armatureName in armatureAnimationDictListDict.keys():
		if not(armatureName in armatureNameList):
			# remove obsolete settings
			del armatureAnimationDictListDict[armatureName]
	# update settings
	for armatureName in armatureActionActuatorListViewDict.keys():
		armatureAnimationDictListDict[armatureName] = armatureActionActuatorListViewDict[armatureName].getArmatureAnimationDictList()
	settingsDict['armatureAnimationDictListDict'] = armatureAnimationDictListDict
		
	configTextName = 'ogreexport.cfg'
	# remove old configuration text
	if configTextName in [text.getName() for text in Blender.Text.Get()]:
		oldConfigText = Blender.Text.Get(configTextName)
		oldConfigText.setName('ogreexport.old')
		Blender.Text.unlink(oldConfigText)
	# write new configuration text
	configText = Blender.Text.New(configTextName)
	configText.write('Ogreexport configuration file.\n\nThis file is automatically created. Please don\'t edit this file directly.\n\n')
	try:
		# pickle
		configText.write(pickle.dumps(settingsDict))
	except (PickleError):
		pass
	else:
		success = 1
	return success

def loadSettings(filename):
	"""Load all exporter settings from text or file.
	
	   Settings are loaded from a text object called 'ogreexport.cfg'.
	   If it is not found, settings are loaded from the file with the given filename.
	   <p>
	   You have to create armatureActionActuatorListViews with the new
	   armatuerAnimationDictListDict if you want the animation settings
	   to take effect.
	
	   @param filename where to store the settings
	   @return <code>true</code> on success, else <code>false</code>
	"""
	global uvToggle
	global armatureToggle
	global worldCoordinatesToggle
	global ambientToggle
	global pathString
	global materialString
	global scaleNumber
	global rotXNumber, rotYNumber, rotZNumber
	global fpsNumber
	global selectedObjectsList
	global armatureDict
	global armatureAnimationDictListDict
	settingsDict = {}
	success = 0
	# try open 'ogreexport.cfg' text
	configTextName = 'ogreexport.cfg'
	if configTextName in [text.getName() for text in Blender.Text.Get()]:
		configText = Blender.Text.Get(configTextName)
		# compose string from text and unpickle
		try:
			# unpickle
			settingsDict = pickle.loads(string.join(configText.asLines()[4:],'\n'))
		except (PickleError):
			pass
		else:
			success = 1		
	# else try open filename
	if not success and os.path.isfile(filename):
		# open file
		try:
			fileHandle = open(filename,'r')
		except IOError, (errno, strerror):
			print "I/O Error(%s): %s" % (errno, strerror)
		else:
			try:
				# load settings
				unpickler = pickle.Unpickler(fileHandle) 
				settingsDict = unpickler.load()
				# close file
				fileHandle.close()
			except EOFError:
				print "EOF Error"
			else:
				success = 1
	# set general settings
	if settingsDict.has_key('uvToggle'):
		uvToggle = Blender.Draw.Create(settingsDict['uvToggle'])
	if settingsDict.has_key('armatureToggle'):
		armatureToggle = Blender.Draw.Create(settingsDict['armatureToggle'])
	if settingsDict.has_key('worldCoordinatesToggle'):
		worldCoordinatesToggle = Blender.Draw.Create(settingsDict['worldCoordinatesToggle'])
	if settingsDict.has_key('ambientToggle'):
		ambientToggle = Blender.Draw.Create(settingsDict['ambientToggle'])
	elif settingsDict.has_key('armatureMeshToggle'):
		# old default was export in world coordinates
		worldCoordinatesToggle = Blender.Draw.Create(1)
	if settingsDict.has_key('pathString'):
		pathString = Blender.Draw.Create(settingsDict['pathString'])
	if settingsDict.has_key('materialString'):
		materialString = Blender.Draw.Create(settingsDict['materialString'])
	if settingsDict.has_key('scaleNumber'):
		scaleNumber = Blender.Draw.Create(settingsDict['scaleNumber'])
	if settingsDict.has_key('rotXNumber'):
		rotXNumber = Blender.Draw.Create(settingsDict['rotXNumber'])
	if settingsDict.has_key('rotYNumber'):
		rotYNumber = Blender.Draw.Create(settingsDict['rotYNumber'])
	if settingsDict.has_key('rotZNumber'):
		rotZNumber = Blender.Draw.Create(settingsDict['rotZNumber'])
	if settingsDict.has_key('fpsNumber'):
		fpsNumber = Blender.Draw.Create(settingsDict['fpsNumber'])
	# set object specific settings
	if settingsDict.has_key('armatureAnimationDictListDict'):
		armatureAnimationDictListDict = settingsDict['armatureAnimationDictListDict']
	elif settingsDict.has_key('animationDictListDict'):
		# convert old animationDictListDict
		## create list of valid armature names
		armatureNameList = []
		for object in Blender.Object.Get():
			if (object.getType() == "Armature"):
				armatureNameList.append(object.getName())
		# create armatureAnimationDictListDict
		armatureAnimationDictListDict = {}
		animationDictListDict = settingsDict['animationDictListDict']
		for armatureName in armatureNameList:
			if animationDictListDict.has_key(armatureName):
				# convert animationDictList
				armatureActionDict = ArmatureAction.createArmatureActionDict(Blender.Object.Get(armatureName))
				armatureActionActuatorListView = ArmatureActionActuatorListView(armatureActionDict, MAXACTUATORS, BUTTON_EVENT_ACTUATOR_RANGESTART,{})
				armatureActionActuatorListView.setAnimationDictList(animationDictListDict[armatureName])
				armatureAnimationDictListDict[armatureName] = armatureActionActuatorListView.getArmatureAnimationDictList()
	return success
	
def refreshGUI():
	"""refresh GUI after export and selection change
	"""
	global exportLogger
	global selectedObjectsList, armatureToggle, armatureDict
	global armatureActionActuatorListViewDict
	global armatureAnimationDictListDict
	# export settings
	exportLogger = Logger()
	# synchronize armatureAnimationDictListDict
	for armatureName in armatureActionActuatorListViewDict.keys():
		armatureAnimationDictListDict[armatureName] = armatureActionActuatorListViewDict[armatureName].getArmatureAnimationDictList()
	selectedObjectsList = Blender.Object.GetSelected()
	if not selectedObjectsList:
		# called from command line
		selectedObjectsList = []
	armatureDict = {}
	# create fresh armatureDict
	for object in selectedObjectsList:
		if (object.getType() == "Armature"):
			# add armature to armatureDict
			armatureDict[object.getName()] = object.getName()
		elif (object.getType() == "Mesh"):
			parent = object.getParent()
			if parent and parent.getType() == "Armature":
				# add armature to armatureDict
				armatureDict[object.getName()] = parent.getName()
	# refresh ArmatureActionActuatorListViews
	for armatureName in armatureDict.values():
		# create armatureActionDict
		armatureActionDict = ArmatureAction.createArmatureActionDict(Blender.Object.Get(armatureName))
		# get animationDictList
		armatureAnimationDictList = None
		if armatureAnimationDictListDict.has_key(armatureName):
			armatureAnimationDictList = armatureAnimationDictListDict[armatureName]
		if armatureActionActuatorListViewDict.has_key(armatureName):
			# refresh armatureActionActuators
			armatureActionActuatorListViewDict[armatureName].refresh(armatureActionDict)
		else:
			# create armatureActionActuatorListView
			armatureActionActuatorListViewDict[armatureName] = ArmatureActionActuatorListView(armatureActionDict, MAXACTUATORS, BUTTON_EVENT_ACTUATOR_RANGESTART, armatureAnimationDictList)
	return

def initGUI():
	"""initialization of the GUI
	"""
	global armatureActionActuatorListViewDict
	if KEEP_SETTINGS:
		# load exporter settings
		loadSettings(Blender.Get('filename')+".ogre")
	armatureActionActuatorListViewDict = {}
	refreshGUI()
	return

def exitGUI():
	if KEEP_SETTINGS:
		# save exporter settings
		saveSettings()
	Blender.Draw.Exit()
	return

def pathSelectCallback(fileName):
	"""handles FileSelector output
	"""
	global pathString
	# strip path from fileName
	pathString = Blender.Draw.Create(os.path.dirname(fileName))
	return
	
def eventCallback(event,value):
	"""handles keyboard and mouse events
	   <p>	
	   exits on ESCKEY<br>
	   exits on QKEY
	"""
	global scrollbar
	global selectedObjectsList, selectedObjectsMenu, armatureActionActuatorListViewDict, armatureDict
	# eventFilter for current ArmatureActionActuatorListView
	if (len(selectedObjectsList) > 0):
		selectedObjectsListIndex = selectedObjectsMenu.val
		selectedObjectName = selectedObjectsList[selectedObjectsListIndex].getName()
		if armatureDict.has_key(selectedObjectName):
			armatureName = armatureDict[selectedObjectName]
			armatureActionActuatorListViewDict[armatureName].eventFilter(event, value)
	scrollbar.eventFilter(event, value)
	if (value != 0):
		# pressed
		if (event == Draw.ESCKEY):
			exitGUI()
		if (event == Draw.QKEY):
			exitGUI()
	return

def buttonCallback(event):
	"""handles button events
	"""
	global materialString, doneMessageBox, eventCallback, buttonCallback, scrollbar
	global selectedObjectsList, selectedObjectsMenu, armatureActionActuatorListViewDict, armatureDict
	global fpsNumber
	# buttonFilter for current ArmatureActionActuatorListView
	if (len(selectedObjectsList) > 0):
		selectedObjectsListIndex = selectedObjectsMenu.val
		selectedObjectName = selectedObjectsList[selectedObjectsListIndex].getName()
		if armatureDict.has_key(selectedObjectName):
			armatureName = armatureDict[selectedObjectName]
			armatureActionActuatorListViewDict[armatureName].buttonFilter(event)
	scrollbar.buttonFilter(event)
	if (event == BUTTON_EVENT_OK): # Ok
		# restart
		refreshGUI()
		Draw.Register(gui, eventCallback, buttonCallback)
	elif (event == BUTTON_EVENT_UPDATEBUTTON):
		# update list of selected objects
		refreshGUI()
		Draw.Redraw(1)
	elif (event == BUTTON_EVENT_SELECTEDOBJECTSMENU):
		# selected object changed
		Draw.Redraw(1)
	elif (event  == BUTTON_EVENT_QUIT): # Quit
		exitGUI()
	elif (event == BUTTON_EVENT_ARMATURETOGGLE): # armatureToggle
		Draw.Redraw(1)
	elif (event == BUTTON_EVENT_PATHBUTTON): # pathButton
		Blender.Window.FileSelector(pathSelectCallback, "Export Directory", pathString.val)
		Draw.Redraw(1)
	elif (event == BUTTON_EVENT_MATERIALSTRING): # materialString
		materialString = Blender.Draw.Create(PathName(materialString.val).basename())
		if (len(materialString.val) == 0):
			materialString = Blender.Draw.Create(Blender.Scene.GetCurrent().getName() + ".material")
		Draw.Redraw(1)
	elif (event == BUTTON_EVENT_SCROLLBAR): # scrollbar
		Draw.Redraw(1)
	elif (event == BUTTON_EVENT_EXPORT): # export
		Draw.Register(exportMessageBox, None, None)
		Draw.Draw()
		# export
		if (Blender.Get("version") >= 233):
			# get blender's current "scene->format->frames per second" setting
			fpsNumber = Draw.Create(Blender.Scene.GetCurrent().getRenderingContext().framesPerSec())
		export(selectedObjectsList)
		# set donemessage
		scrollbar = ReplacementScrollbar(0,0,len(exportLogger.getMessageList())-1,BUTTON_EVENT_SCROLLBARUP,BUTTON_EVENT_SRCROLLBARDOWN)
		Draw.Register(doneMessageBox, eventCallback, buttonCallback)
		Draw.Redraw(1)
	return

def frameDecorator(x, y, width):
	"""draws title and logo onto the frame
	
		@param x upper left x coordinate
		@param y upper left y coordinate
		@param width screen width to use
		@return used height
	"""
	# title
	glColor3f(0, 0.2, 0)
	glRectf(x,y-36,x+width,y-16)
	glColor3f(1.0,1.0,0)
	glRasterPos2i(x+85, y-30)
	Draw.Text("OGRE Exporter 0.15.0", "normal")

	# logo
	glRasterPos2i(x+1, y-48)	
	glEnable(GL_BLEND)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
	glDrawPixels(77, 48, GL_RGBA, GL_BYTE, OGRE_LOGO)
	glColor3f(0,0,0)	
	return 50

def gui():
	"""draws the screen
	"""
	global uvToggle, armatureToggle, worldCoordinatesToggle, ambientToggle, pathString, materialString, \
		scaleNumber, fpsNumber, scrollbar, rotXNumber, rotYNumber, rotZNumber
	global selectedObjectsList, selectedObjectsMenu, armatureActionActuatorListViewDict, armatureDict
	# get size of the window
	guiRectBuffer = Buffer(GL_FLOAT, 4)
	glGetFloatv(GL_SCISSOR_BOX, guiRectBuffer)
	guiRect = [0, 0, int(guiRectBuffer.list[2]), int(guiRectBuffer.list[3])]
	
	remainRect = guiRect[:]
	remainRect[0] += 10
	remainRect[1] += 10
	remainRect[2] -= 10
	remainRect[3] -= 10
	
	# clear background
	glClearColor(0.6,0.6,0.6,1) # Background: grey
	glClear(GL_COLOR_BUFFER_BIT)
	
	remainRect[3] -= frameDecorator(remainRect[0], remainRect[3], remainRect[2]-remainRect[0])
	
	# export settings
	remainRect[3] -= 5
	# first row
	materialString = Draw.String("Material File: ", BUTTON_EVENT_MATERIALSTRING, \
			remainRect[0],remainRect[3]-25, 450, 20, \
			materialString.val, 255,"all material definitions go in this file (relative to the save path)")
	remainRect[3] -= 25
	# second row
	uvToggle = Draw.Toggle("Export Textures", BUTTON_EVENT_UVTOGGLE, \
				remainRect[0], remainRect[3]-25, 220, 20, \
				uvToggle.val, "export uv coordinates and texture names, if available")
	# scale settings
	scaleNumber = Draw.Number("Mesh Scale Factor: ", BUTTON_EVENT_SCALENUMBER, \
			remainRect[0]+230, remainRect[3]-25, 220, 20, \
			scaleNumber.val, 0.0, 1000.0, "scale factor")
	remainRect[3] -= 25
	# third row	
	armatureToggle = Draw.Toggle("Export Armature", BUTTON_EVENT_ARMATURETOGGLE, \
				remainRect[0], remainRect[3]-25, 220, 20, \
				armatureToggle.val, "export skeletons and bone weights in meshes")
	rotXNumber = Draw.Number("RotX: ", BUTTON_EVENT_ROTXNUMBER, \
			remainRect[0]+230, remainRect[3]-25, 220, 20, \
			rotXNumber.val, -360.0, 360.0, "angle of the first rotation, around the x-axis")
	remainRect[3] -= 25
	# fourth row
	worldCoordinatesToggle = Draw.Toggle("World Coordinates", BUTTON_EVENT_WORLDCOORDINATESTOGGLE, \
			remainRect[0], remainRect[3]-25, 220, 20, \
			worldCoordinatesToggle.val, "use world coordinates instead of object coordinates")
	rotYNumber = Draw.Number("RotY: ", BUTTON_EVENT_ROTYNUMBER, \
			remainRect[0]+230, remainRect[3]-25, 220, 20, \
			rotYNumber.val, -360.0, 360.0, "angle of the second rotation, around the y-axis")
	remainRect[3] -= 25
	# fifth row
	ambientToggle = Draw.Toggle("Coloured Ambient", BUTTON_EVENT_AMBIENTTOGGLE, \
			remainRect[0], remainRect[3]-25, 220, 20, \
			ambientToggle.val, "use Amb factor times diffuse colour as ambient instead of Amb factor times white")
	rotZNumber = Draw.Number("RotZ: ", BUTTON_EVENT_ROTZNUMBER, \
			remainRect[0]+230, remainRect[3]-25, 220, 20, \
			rotZNumber.val, -360.0, 360.0, "angle of the third rotation, around the z-axis")
	# sixth row
	if ((armatureToggle.val == 1) and (Blender.Get("version") < 233)):
		remainRect[3] -= 25
		fpsNumber = Draw.Number("Frs/Sec: ", BUTTON_EVENT_FPSNUMBER, \
				remainRect[0]+230, remainRect[3]-25, 220, 20, \
				fpsNumber.val, 1, 120, "animation speed in frames per second")
	remainRect[3] -= 35
	
	# Path setting
	pathString = Draw.String("Path: ", BUTTON_EVENT_PATHSTRING, \
			10, 50, guiRect[2]-91, 20, \
			pathString.val, 255, "the directory where the exported files are saved")
	Draw.Button("Select", BUTTON_EVENT_PATHBUTTON, guiRect[2]-80, 50, 70, 20, "select the export directory")
	# button panel
	Draw.Button("Export", BUTTON_EVENT_EXPORT,10,10,100,30,"export selected objects")
	# Draw.Button("Help",BUTTON_EVENT_HELP ,(guiRect[2])/2-50,10,100,30,"notes on usage")	
	Draw.Button("Quit", BUTTON_EVENT_QUIT,guiRect[2]-110,10,100,30,"quit without exporting")
	remainRect[1] += 70
	
	# rename animation part	
	#if (armatureToggle.val == 1):
	animationText = "Animation settings of"
	xOffset = Draw.GetStringWidth(animationText) + 5
	selectedObjectsMenuName = ""
	selectedObjectsMenuIndex = 0
	if (len(selectedObjectsList) > 0):
		for object in selectedObjectsList:
			# add menu string
			selectedObjectsMenuName += object.getName() + " %x" + ("%d" % selectedObjectsMenuIndex) + "|"
			selectedObjectsMenuIndex += 1
	else:
		selectedObjectsMenuName = "No objects selected! %t"
	selectedObjectsMenu = Draw.Menu(selectedObjectsMenuName, BUTTON_EVENT_SELECTEDOBJECTSMENU, \
	                      remainRect[0]+xOffset, remainRect[3]-20, 140, 20, \
	                      selectedObjectsMenu.val, "Objects selected for export")
	xOffset += 141
	# update button
	Draw.Button("Update", BUTTON_EVENT_UPDATEBUTTON, remainRect[0]+xOffset, remainRect[3]-20, 60, 20, "update list of selected objects")
	remainRect[3] -= 25
	if (armatureToggle.val == 1):
		# draw armatureActionActuator
		if (len(selectedObjectsList) > 0):
			selectedObjectsListIndex = selectedObjectsMenu.val
			selectedObjectName = selectedObjectsList[selectedObjectsListIndex].getName()
			if armatureDict.has_key(selectedObjectName):
				glRasterPos2i(remainRect[0],remainRect[3]+10)
				Draw.Text(animationText)
				armatureName = armatureDict[selectedObjectName]
				armatureActionActuatorListViewDict[armatureName].draw(remainRect[0], remainRect[1], remainRect[2]-remainRect[0], remainRect[3]-remainRect[1])
	return

def exportMessageBox():
	"""informs on the export progress
	"""
	# get size of the window
	guiRectBuffer = Buffer(GL_FLOAT, 4)
	glGetFloatv(GL_SCISSOR_BOX, guiRectBuffer)
	guiRect = [0, 0, int(guiRectBuffer.list[2]), int(guiRectBuffer.list[3])]
	
	remainRect = guiRect[:]
	remainRect[0] += 10
	remainRect[1] += 10
	remainRect[2] -= 10
	remainRect[3] -= 10

	# clear background
	glClearColor(0.6,0.6,0.6,1) # Background: grey
	glClear(GL_COLOR_BUFFER_BIT)
	
	remainRect[3] -= frameDecorator(remainRect[0], remainRect[3], remainRect[2]-remainRect[0])
	
	# export information
	## center view
	exportMessage = "Exporting, please wait!"
	exportMessageWidth = Draw.GetStringWidth(exportMessage, 'normal')
	textPosition = [0, 0]
	textPosition[0] = (remainRect[0] + remainRect[2] - exportMessageWidth)/2
	textPosition[1] = (remainRect[1] + remainRect[3])/2
	glRasterPos2i(textPosition[0], textPosition[1]) 
	glColor3f(0,0,0) # Defaul color: black
	Draw.Text(exportMessage, "normal")
	return
	
def doneMessageBox():
	"""displays export message and log
	"""
	global exportLogger
	EXPORT_SUCCESS_MESSAGE = "Successfully exported!"
	EXPORT_WARNING_MESSAGE = "Exported with warnings!"
	EXPORT_ERROR_MESSAGE = "Error in export!"	
	# get size of the window
	guiRectBuffer = Buffer(GL_FLOAT, 4)
	glGetFloatv(GL_SCISSOR_BOX, guiRectBuffer)
	guiRect = [0, 0, int(guiRectBuffer.list[2]), int(guiRectBuffer.list[3])]
		
	remainRect = guiRect[:]
	remainRect[0] += 10
	remainRect[1] += 10
	remainRect[2] -= 10
	remainRect[3] -= 10
	
	# clear background
	glClearColor(0.6,0.6,0.6,1) # Background: grey
	glClear(GL_COLOR_BUFFER_BIT)
	
	remainRect[3] -= frameDecorator(remainRect[0], remainRect[3], remainRect[2]-remainRect[0])
	
	# OK button
	Draw.Button("OK", BUTTON_EVENT_OK,10,10,100,30,"return to export settings")
	Draw.Button("Quit", BUTTON_EVENT_QUIT,guiRect[2]-110,10,100,30,"quit export script")
	remainRect[1] += 40
	
	# message
	status = exportLogger.getStatus()
	doneMessage = ''
	if (status == Logger.INFO):
		doneMessage= EXPORT_SUCCESS_MESSAGE
	elif (status == Logger.WARNING):
		doneMessage = EXPORT_WARNING_MESSAGE
		glColor3f(1.0,1.0,0.0)
		Blender.BGL.glRectf(remainRect[0], remainRect[3]-24, remainRect[0]+Draw.GetStringWidth(doneMessage), remainRect[3]-7)
	elif (status == Logger.ERROR):
		doneMessage = EXPORT_ERROR_MESSAGE
		glColor3f(1.0,0.0,0.0)
		Blender.BGL.glRectf(remainRect[0], remainRect[3]-24, remainRect[0]+Draw.GetStringWidth(doneMessage), remainRect[3]-7)
	remainRect[3] -= 20
	glColor3f(0.0,0.0,0.0) # Defaul color: black
	glRasterPos2i(remainRect[0],remainRect[3])
	Draw.Text(doneMessage,"normal")
	
	remainRect[3] -= 20
	glColor3f(0.0,0.0,0.0) # Defaul color: black
	glRasterPos2i(remainRect[0],remainRect[3])
	Draw.Text("Export Log:","small")
	remainRect[3] -= 4
	
	# black border
	logRect = remainRect[:]
	logRect[2] -= 22
	glColor3f(0,0,0) # Defaul color: black
	glRectf(logRect[0],logRect[1],logRect[2],logRect[3])
	logRect[0] += 1
	logRect[1] += 1
	logRect[2] -= 1
	logRect[3] -= 1
	glColor3f(0.662,0.662,0.662) # Background: grey
	glRectf(logRect[0],logRect[1],logRect[2],logRect[3])
	
	# display export log
	exportLog = exportLogger.getMessageList()
	scrollPanelRect = remainRect[:]
	loglineiMax = len(exportLog)
	loglinei = scrollbar.getCurrentValue()
	while (((logRect[3]-logRect[1]) >= 20) and ( loglinei < loglineiMax )):
		logRect[3] -= 16
		(status, message) = exportLog[loglinei]
		if (status == Logger.WARNING):
			glColor3f(1.0,1.0,0.0)
			glRecti(logRect[0],logRect[3]-4,logRect[2],logRect[3]+13)
		elif (status == Logger.ERROR):
			glColor3f(1.0,0.0,0.0)
			glRecti(logRect[0],logRect[3]-4,logRect[2],logRect[3]+13)
		glColor3f(0,0,0)
		glRasterPos2i(logRect[0]+4,logRect[3])
		Draw.Text(message)
		loglinei += 1
	# clip log text
	glColor3f(0.6,0.6,0.6) # Background: grey
	glRectf(scrollPanelRect[2]-22,scrollPanelRect[1], guiRect[2],scrollPanelRect[3])
	# draw scrollbar
	scrollbar.draw(scrollPanelRect[2]-20, scrollPanelRect[1], 20, scrollPanelRect[3]-scrollPanelRect[1])
	return

######
# Main
######
initGUI()
Draw.Register(gui, eventCallback, buttonCallback)
