#!BPY

"""
Name: 'Ogre XML'
Blender: 232
Group: 'Export'
Tooltip: 'Export Mesh and Armature to Ogre'
"""

# Blender to Ogre Mesh and Skeleton Exporter v0.13.1
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
#
# TODO:
#          - TWOSIDE face mode, TWOSIDED mesh mode
#          - SUBSURF mesh mode
#          - load/save animation export settings
#          - help button
#          - code cleanup
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

# KEEP_SETTINGS (enable = 1, disable = 0)
#  transparently load and save settings to a file named after the current
#  .blend file with suffix ".ogre".
KEEP_SETTINGS = 1

#######################################################################################
## Code starts here.

######
# imports
######
import Blender, sys, os, math, string, pickle

######
# namespaces
######
from Blender import Draw
from Blender.BGL import *

######
# Classes
######
class ReplacementScrollbar:
	def __init__(self, initialValue, minValue, maxValue, buttonUpEvent, buttonDownEvent):
		"""Scrollbar replacement for Draw.Scrollbar
		   
		   - import Blender
		   - call eventFilter and buttonFilter in registered callbacks
		      
		   Parameters:
		   	initialValue -  inital value
		   	minValue - minimum value
		   	maxValue - maxium value
		   	buttonUpEvent - unique event number
		   	buttonDownEvent - unique event number
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
		self.guiRect =  guiRectBuffer.list
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
			print "scrollbar draw size to small!"
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
				# check if mouse is inside postionRect
				if (value >= (self.guiRect[0] + self.positionRect[0])) and (value <= (self.guiRect[0] + self.positionRect[2])):
					self.mouseFocusX = 1
				else:
					self.mouseFocusX = 0
			elif (event == Blender.Draw.MOUSEY):
				# check if mouse is inside positionRect
				if (value >= (self.guiRect[1] + self.positionRect[1])) and (value <= (self.guiRect[1] + self.positionRect[3])):
					self.mouseFocusY = 1
					# relative mouse position
					self.mousePositionY = value - self.guiRect[1]
					# move marker
					if (self.mousePressed == 1):
						# calculate step from distance to marker
						if (self.mousePositionY > self.markerRect[3]):
							# up
							self.up(1)
						elif (self.mousePositionY < self.markerRect[1]):
							# down
							self.down(1)
					Blender.Draw.Redraw(1)
				else:
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
class Action:
	def __init__(self, ipoPrefix="", ipoPostfix= "", boneList=None, firstKeyFrame=None, lastKeyFrame=None):
		"""Resemble Blender's actions.
		   
		   - import Blender, string
		   
		   Parameters:
		   	ipoPrefix - prefix of the corresponding action ipo name, e.g. "Action.BAKED
		   	ipoPostfix - postfix of the corresponding action ipo name, e.g. ".001"
		   	boneList - list of bone names belonging to that action
		   	firstKeyFrame - first keyframe of that action
		   	lastKeyFrame - last keyframe of that action
		"""
		self.ipoPrefix = ipoPrefix
		self.ipoPostfix = ipoPostfix
		if boneList is None:
			self.boneList = []
		else:
			self.boneList = boneList
		if firstKeyFrame is None:
			self.firstKeyFrame = firstKeyFrame
		else:
			self.firstKeyFrame = int(firstKeyFrame)
		if lastKeyFrame is None:
			self.lastKeyFrame = lastKeyFrame
		else:
			self.lastKeyFrame = int(lastKeyFrame)
		return
	
	def addBone(self, bone, firstKeyFrame=None, lastKeyFrame=None):
		"""Add bone to the bone list of that action.
		   
		   Paraneters:
		   	bone - name of the new bone
			firstKeyFrame - first keyframe of that bone
			lastKeyFrame - last keyframe of that bone
		"""
		self.boneList.append(bone)
		if (((firstKeyFrame < self.firstKeyFrame) or (self.firstKeyFrame is None)) and (firstKeyFrame is not None)):
			self.firstKeyFrame = int(firstKeyFrame)
		if (((lastKeyFrame > self.lastKeyFrame) or (self.lastKeyFrame is None)) and (lastKeyFrame is not None)):
			self.lastKeyFrame = int(lastKeyFrame)
		return
	
	def createActionDict(self, armature):
		"""Creates a dictionary of possible actions belonging to an armature.
		   Static call with: Action.createActionDict(Action(), armature)
		   
		   Note: There is no direct way to get Action Ipos of a
		         Blender.Armature. Therefore this method is based on
		         name comparison between Ipos and Bones.
		   
		   Parameters:
		   	armature - a Blender.Armature object
		   Return:
		   	a dictionary of Action objects with keys (ipoPrefix, ipoPostfix)
		"""
		actionDict = {}
		# get all bone names
		boneNameList = []
		boneQueue = armature.getBones()
		while (len(boneQueue) > 0):
			# get all bones of the armature
			currentBone = boneQueue.pop(0)
			boneNameList.append(currentBone.getName())
			children = currentBone.getChildren()
			if (len(children) > 0):
				for child in children:
					boneQueue.append(child)
		# check all ipos:
		#   if prefix = Action.<action name>.
		#      postfix = [.<3 digits> | ""]
		#      infix = <bone name>:
		#     actionName = prefix.postfix
		#     if action does not allready exist
		#       create action
		#     append bone
		for ipo in Blender.Ipo.Get():
			ipoName = ipo.getName()
			prefix = "Action."
			if ipoName[0:len(prefix)] == prefix:
				# Action Ipo
				# check postfix for .<3 digits>
				postfix = ipoName[-4:]
				if ((postfix[0] == ".") and \
				(postfix[1] in string.digits) and \
				(postfix[2] in string.digits) and \
				(postfix[3] in string.digits)):
					ipoPostfix = postfix
				else:
					ipoPostfix = ""
				# check if ipoName contains a bone name
				for boneName in boneNameList:
					if (len(ipoName) >= (len(prefix) + len(boneName) + len(ipoPostfix))):
						# ipoName is long enough
						if (len(ipoPostfix) > 0):
							infix = ipoName[-(len(boneName) + len(ipoPostfix) + 1):-len(ipoPostfix)]
						else:
							infix = ipoName[-(len(boneName) + 1):]
						if (infix == "." + boneName):
							# Action Ipo name contains bone name
							ipoPrefix = ipoName[:-(len(infix)+len(ipoPostfix))]
							if (actionDict.has_key((ipoPrefix,ipoPostfix))):
								# action already exists
								action = actionDict[(ipoPrefix,ipoPostfix)]
							else:
								# create action
								action = Action(ipoPrefix, ipoPostfix)
							# append bone
							# get first and last keyframe
							frameList = []
							for ipoCurve in ipo.getCurves():
								for bezTriple in ipoCurve.getPoints():
									frameList.append(bezTriple.getPoints()[0])
							frameList.sort()
							action.addBone(boneName, frameList.pop(0), frameList.pop())
							actionDict[(ipoPrefix,ipoPostfix)]= action
		return actionDict

class ActionActuator:
	def __init__(self, name, startFrame, endFrame, action):
		"""Resemble Blender's action actuators.
		   
		   Parameters:
		   	name - Animation name
		   	startFrame - first frame of the animation
		   	endFrame - last frame of the animation
		   	action - Action object of the animation
		"""
		self.name = name
		self.startFrame = startFrame
		self.endFrame = endFrame
		self.action = action
		return

class ActionActuatorListView:
	def __init__(self, actionDict, maxActuators, buttonEventRangeStart, animationDictList=None):
		"""Mangages a list of ActionActuators.
		   
		   - import Blender
		   - call eventFilter and buttonFilter in registered callbacks
		      
		   Parameters:
		   	actionDict - possible actuator actions
		   	maxActuators - maximal number of actuator list elements
		   	buttonEventRangeStart - first button event number
		   		number of used event numbers is (3 + maxActuators*5)
			animationDictList - list of animations (see getAnimationDictList())
		"""
		self.actionDict = actionDict
		self.maxActuators = maxActuators
		self.buttonEventRangeStart = buttonEventRangeStart
		self.actionActuatorList = []
		self.actionMenuList = []
		self.startFrameNumberButtonList = []
		self.endFrameNumberButtonList = []
		self.animationNameStringButtonList = []
		# scrollbar values:
		#   0:(len(self.actionActuatorList)-1) = listIndex
		#   len(self.actionActuatorList) = addbuttonline
		self.scrollbar = ReplacementScrollbar(0,0,0, self.buttonEventRangeStart+1, self.buttonEventRangeStart+2)
		if not(animationDictList is None):
			# rebuild ActionActuators for animationList animations
			for animationDict in animationDictList:
				# check if Action is available
				if self.actionDict.has_key(animationDict['actionKey']):
					actionActuator = ActionActuator(animationDict['name'], \
					                                animationDict['startFrame'], \
					                                animationDict['endFrame'], \
					                                self.actionDict[animationDict['actionKey']])
					self.addActionActuator(actionActuator)
		else:
			# create default ActionActuators
			for actionKey in self.actionDict.keys():
				# add default action
				action = self.actionDict[actionKey]
				actionActuator = ActionActuator(action.ipoPrefix + action.ipoPostfix, action.firstKeyFrame, action.lastKeyFrame, action)
				self.addActionActuator(actionActuator)
		return
		
	def refresh(self, actionDict):
		"""Delete ActionActuators for removed Actions.
		    
		   Parameters:
		   	actionDict - possible actuator actions
		"""
		self.actionDict = actionDict
		# delete ActionActuators for removed Actions
		for actionActuator in self.actionActuatorList[:]:
			key = (actionActuator.action.ipoPrefix, actionActuator.action.ipoPostfix)
			# check if action is still available
			if not self.actionDict.has_key(key):
				# remove actionActuator from lists
				listIndex = self.actionActuatorList.index(actionActuator)
				self.deleteActionActuator(listIndex)
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
			print "ActionActuatorListView draw size to small!"
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
		# actionPrefix | startFrame | endFrame | animationName | [delete] | scrollbar
		# [ add ]                                                         | scrollbar
		if (len(self.actionDict.keys()) > 0):
			# construct actionMenu name
			menuValue = 0
			menuName = ""
			for key in self.actionDict.keys():
				menuName += self.actionDict[key].ipoPrefix + self.actionDict[key].ipoPostfix + " %x" + ("%d" % menuValue) + "|"
				menuValue +=1
			# first line
			lineY = y + height - 20
			lineX = x
			listIndex = self.scrollbar.getCurrentValue()
			while ((listIndex < len(self.actionActuatorList)) and (lineY >= y)):
				# still actionActuators left to draw
				lineX = x
				actionActuator = self.actionActuatorList[listIndex]
				# draw actionMenu
				event = self.buttonEventRangeStart + 3 + listIndex
				menuValue = self.actionDict.keys().index((actionActuator.action.ipoPrefix, actionActuator.action.ipoPostfix))
				self.actionMenuList[listIndex] = Blender.Draw.Menu(menuName,event, x, lineY, 105, 20, menuValue, "Action name")
				lineX += 107
				# draw startFrameNumberButton
				event = self.buttonEventRangeStart + 3 + self.maxActuators + listIndex
				self.startFrameNumberButtonList[listIndex] = Blender.Draw.Number("Sta: ", event, lineX, lineY, 80, 20, \
				                                         actionActuator.startFrame, -18000, 18000, "Start frame")
				lineX += 82
				# draw endFrameNumberButton
				event = self.buttonEventRangeStart + 3 + 2*self.maxActuators + listIndex
				self.endFrameNumberButtonList[listIndex] = Blender.Draw.Number("End: ", event, lineX, lineY, 80, 20, \
				                                         actionActuator.endFrame, -18000, 18000, "End frame")
				lineX += 82
				# compute animationNameWidht
				animationNameWidth = width - 271 - 85
				if (animationNameWidth < 80):
					animationNameWidth = 80
				# draw animationNameStringButton
				event = self.buttonEventRangeStart + 3 + 3*self.maxActuators + listIndex
				self.animationNameStringButtonList[listIndex] = Blender.Draw.String("",event, lineX, lineY, animationNameWidth, 20, \
				                                                actionActuator.name, 1000, "Animation export name") 
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
			if (len(self.actionDict.keys()) > 0):
				# add default ActionActuator
				action = self.actionDict[self.actionDict.keys()[0]]
				actionActuator = ActionActuator(action.ipoPrefix + action.ipoPostfix, action.firstKeyFrame, action.lastKeyFrame, action)
				self.addActionActuator(actionActuator)
				Blender.Draw.Redraw(1)
		elif ((3 <= relativeEvent) and (relativeEvent < (3 + self.maxActuators))):
			# actionMenu
			listIndex = relativeEvent - 3
			actionActuator = self.actionActuatorList[listIndex]
			# button value is self.actionDict.keys().index
			keyIndex = self.actionMenuList[listIndex].val
			key = self.actionDict.keys()[keyIndex]
			actionActuator.action = self.actionDict[key]
			self.actionActuatorList[listIndex] = actionActuator
		elif (((3 + self.maxActuators) <= relativeEvent) and (relativeEvent < (3 + 2*self.maxActuators))):
			# startFrameNumberButton
			listIndex = relativeEvent - (3 + self.maxActuators)
			actionActuator = self.actionActuatorList[listIndex]
			actionActuator.startFrame = self.startFrameNumberButtonList[listIndex].val
			self.actionActuatorList[listIndex] = actionActuator
		elif (((3 + 2*self.maxActuators) <= relativeEvent) and (relativeEvent < (3 + 3*self.maxActuators))):
			# endFrameNumberButton
			listIndex = relativeEvent - (3 + 2*self.maxActuators)
			actionActuator = self.actionActuatorList[listIndex]
			actionActuator.endFrame = self.endFrameNumberButtonList[listIndex].val
			self.actionActuatorList[listIndex] = actionActuator
		elif (((3 + 3*self.maxActuators) <= relativeEvent) and (relativeEvent < (3 + 4*self.maxActuators))):
			# animationNameStringButton
			listIndex = relativeEvent - (3 + 3*self.maxActuators)
			actionActuator = self.actionActuatorList[listIndex]
			actionActuator.name = self.animationNameStringButtonList[listIndex].val
			self.actionActuatorList[listIndex] = actionActuator
		elif (((3 + 4*self.maxActuators) <= relativeEvent) and (relativeEvent < (3 + 5*self.maxActuators))):
			# deleteButton
			listIndex = relativeEvent - (3 + 4*self.maxActuators)
			self.deleteActionActuator(listIndex)
			Blender.Draw.Redraw(1)
		return
		
	def getAnimationDictList(self):
		"""serialize the actionActuatorList into a pickle storable list
		   Each item of the returned list is a dictionary with key-value pairs:
		   	name - ActionActuator.name
		   	startFrame - ActionActuator.startFrame
		   	endFrame - ActionActuator.endFrame
		   	actionKey - (ActionActuator.Action.ipoPrefix, ActionActuator.Action.ipoPostfix)
		   
		   Return:
		   	serialized actionActuatorList
		"""
		animationDictList = []
		for actionActuator in self.actionActuatorList:
			# create animationDict
			animationDict = {}
			animationDict['name'] = actionActuator.name
			animationDict['startFrame'] = actionActuator.startFrame
			animationDict['endFrame'] = actionActuator.endFrame
			animationDict['actionKey'] = (actionActuator.action.ipoPrefix, actionActuator.action.ipoPostfix)
			animationDictList.append(animationDict)
		return animationDictList
		
	# private methods
	def addActionActuator(self, actionActuator):
		"""adds an ActionActuator to the list
		   - call Blender.Draw.Redraw(1) afterwards
		"""
		if (len(self.actionActuatorList) < self.maxActuators):
			# check if actionActuator.action is available
			if (actionActuator.action.ipoPrefix, actionActuator.action.ipoPostfix) in self.actionDict.keys():
				# create actionMenu
				# get Action index in actionDict.keys() list
				actionMenu = Draw.Create(self.actionDict.keys().index((actionActuator.action.ipoPrefix, actionActuator.action.ipoPostfix)))
				self.actionMenuList.append(actionMenu)
				# create startFrameNumberButton
				startFrameNumberButton = Draw.Create(int(actionActuator.startFrame))
				self.startFrameNumberButtonList.append(startFrameNumberButton)
				# create endFrameNumberButton
				endFrameNumberButton = Draw.Create(int(actionActuator.endFrame))
				self.endFrameNumberButtonList.append(endFrameNumberButton)
				# create animationNameStringButton
				animationNameStringButton = Draw.Create(actionActuator.name)
				self.animationNameStringButtonList.append(animationNameStringButton)
				# append to actionActuatorList
				self.actionActuatorList.append(actionActuator)
				# adjust scrollbar
				scrollbarPosition = self.scrollbar.getCurrentValue()
				self.scrollbar = ReplacementScrollbar(scrollbarPosition,0,len(self.actionActuatorList), self.buttonEventRangeStart+1, self.buttonEventRangeStart+2)
				# TODO: change scrollbarPosition in a way, such that the new actuator is visible
			else:
				print "Error: Could not add ActionActuator because Action is not available!"
		return
		
	def deleteActionActuator(self, listIndex):
		"""removes an ActionActuator from the list
		   - call Blender.Draw.Redraw(1) afterwards
		"""
		# check listIndex
		if ((len(self.actionActuatorList) > 0) and (listIndex >= 0) and (listIndex < len(self.actionActuatorList))):
			# remove actionMenu
			self.actionMenuList.pop(listIndex)
			# remove startFrameNumberButton
			self.startFrameNumberButtonList.pop(listIndex)
			# remove endFrameNumberButton
			self.endFrameNumberButtonList.pop(listIndex)
			# remove animationNameStringButton
			self.animationNameStringButtonList.pop(listIndex)
			# remove actionActuator
			self.actionActuatorList.pop(listIndex)
			# adjust scrollbar
			scrollbarPosition = self.scrollbar.getCurrentValue()
			if (scrollbarPosition > len(self.actionActuatorList)):
				scrollbarPosition = len(self.actionActuatorList)
			self.scrollbar = ReplacementScrollbar(scrollbarPosition,0,len(self.actionActuatorList), self.buttonEventRangeStart+1, self.buttonEventRangeStart+2)
			return

######
# global variables
######
uvToggle = Draw.Create(1)
armatureToggle = Draw.Create(1)
armatureMeshToggle = Draw.Create(0)
pathString = Draw.Create(os.path.dirname(Blender.Get('filename')))
materialString = Draw.Create("export.material")
scaleNumber = Draw.Create(1.0)
fpsNumber = Draw.Create(25)
selectedObjectsList = Blender.Object.GetSelected()
selectedObjectsMenu = Draw.Create(0)
scrollbar = ReplacementScrollbar(0,0,0,0,0)
doneMessage = ""
# key: objectName, value: armatureName
armatureDict = {}
# key: armatureName, value: actionActuatorListView
# does only contain keys for the current selected objects
actionActuatorListViewDict = {}
# key: armatureName, value: animationDictList
animationDictListDict = {}
MAXACTUATORS = 100

# button event numbers:
BUTTON_EVENT_OK = 101
BUTTON_EVENT_QUIT = 102
BUTTON_EVENT_EXPORT = 103
BUTTON_EVENT_UVTOGGLE = 104
BUTTON_EVENT_ARMATURETOGGLE = 105
BUTTON_EVENT_ARMATUREMESHTOGGLE = 106
BUTTON_EVENT_PATHSTRING = 107
BUTTON_EVENT_PATHBUTTON = 108
BUTTON_EVENT_MATERIALSTRING = 109
BUTTON_EVENT_SCALENUMBER = 1010
BUTTON_EVENT_FPSNUMBER = 1011
BUTTON_EVENT_SCROLLBAR = 1012
BUTTON_EVENT_SCROLLBARUP = 1013
BUTTON_EVENT_SRCROLLBARDOWN = 1014
BUTTON_EVENT_UPDATEBUTTON = 1015
BUTTON_EVENT_SELECTEDOBJECTSMENU = 1016
BUTTON_EVENT_ACTUATOR_RANGESTART = 1017

# error indication:
EXPORT_SUCCESS = 0
EXPORT_SUCCESS_MESSAGE = "Successfully exported!"
EXPORT_WARNING = 1
EXPORT_WARNING_MESSAGE = "Exported with warnings!"
EXPORT_ERROR = 2
EXPORT_ERROR_MESSAGE = "Error in export!"
exportStatus = EXPORT_SUCCESS
exportLog = []

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
  global exportStatus, exportLog
  l = math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])
  if l <= 0.000001:
    exportLog.append("error in normalize")
    exportStatus = EXPORT_ERROR
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
  def __init__(self, name, mat, texname, mode):
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
  def __init__(self, u=None, v=None):
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
    # exportLog.append("neg")
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


def export_skeleton(object):
	global armatureToggle, fpsNumber, armatureMeshToggle, actionActuatorListViewDict
	global skeletonsDict
	global exportStatus, exportLog
	
	if ((armatureToggle.val == 1) and (not skeletonsDict.has_key(object.getName())) and (object.getType() == "Armature")):
		skeleton = Skeleton(object.name)
		skeletonsDict[object.name] = skeleton

		testskel = None
		if armatureMeshToggle.val:
			testskel = TestSkel(skeleton)

		convert_armature(skeleton, object, testskel)

		if testskel:
			export_testskel(testskel)

		# get scale from obj matrix (for loc keys)
		matrix = matrix_multiply(BASE_MATRIX, object.getMatrix())
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
		# export animations
		if actionActuatorListViewDict.has_key(object.getName()):
			actionActuatorList = actionActuatorListViewDict[object.getName()].actionActuatorList
			# map actionActuatorList to skeleton.animationsDict
			for actionActuator in actionActuatorList:
				# map actionActuator to animation
				if (not skeleton.animationsDict.has_key(actionActuator.name)):
					# create animation
					animation = Animation(actionActuator.name)
					# map bones to tracks
					for boneName in actionActuator.action.boneList:
						if (not animation.tracksDict.has_key(boneName)):
							# get bone object
							if boneDict.has_key(boneName):
								# create track
								track = Track(animation, boneDict[boneName])
								# map ipocurves to keyframes
								# get ipo for that bone
								ipoName = actionActuator.action.ipoPrefix + "." + boneName + actionActuator.action.ipoPostfix
								try:
									ipo = Blender.Ipo.Get(ipoName)
									# map curve names to curvepos
									curveId = {}
									id = 0
									have_quat = 0
									for curve in ipo.getCurves():
										try:
											name = curve.getName()
											if (name == "LocX" or name == "LocY" or name == "LocZ" or \
											name == "SizeX" or name == "SizeY" or name == "SizeZ" or \
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
										except TypeError:
											# blender 2.32 does not implement IpoCurve.getName() for action Ipos
											if not have_quat:
												# no automatic assignments so far
												# guess Ipo Names       
												nIpoCurves = ipo.getNcurves()
												if nIpoCurves in [4,7,10]:
													exportLog.append("Warning: IpoCurve.getName() not available!")
													exportLog.append("         The exporter tries to guess the IpoCurve names.")
													exportStatus = EXPORT_WARNING
													if (nIpoCurves >= 7):
														# not only Quats
														# guess: Quats and Locs
														curveId["LocX"] = id
														curveId["LocY"] = id+1
														curveId["LocZ"] = id+2
														id += 3      
													if (nIpoCurves == 10):
														# all possible Action IpoCurves
														curveId["SizeX"] = id
														curveId["SizeY"] = id+1
														curveId["SizeZ"] = id+2
														id += 3
													if (nIpoCurves >= 4):
														# at least 4 IpoCurves
														# guess: 4 Quats
														curveId["QuatX"] = id
														curveId["QuatY"] = id+1
														curveId["QuatZ"] = id+2
														curveId["QuatW"] = id+3
														id += 4
													have_quat = 1
												else:
													exportLog.append("Error: IpoCurve.getName() not available!")
													exportLog.append("       Could not guess the IpoCurve names. Other Blender versions may work.")
													exportStatus = EXPORT_ERROR
									# get all frame numbers between startFrame and endFrame where this ipo has a point in one of its curves
									frameNumberDict = {}
									for curveIndex in range(ipo.getNcurves()):
										for bez in range(ipo.getNBezPoints(curveIndex)):
											frame = int(ipo.getCurveBeztriple(curveIndex, bez)[3])
											frameNumberDict[frame] = frame
									frameNumberDict[actionActuator.startFrame] = actionActuator.startFrame
									frameNumberDict[actionActuator.endFrame] = actionActuator.endFrame
									# remove frame numbers not in the startFrame endFrame range
									if (actionActuator.startFrame > actionActuator.endFrame):
										minFrame = actionActuator.endFrame
										maxFrame = actionActuator.startFrame
									else:
										minFrame = actionActuator.startFrame
										maxFrame = actionActuator.endFrame
									for frameNumber in frameNumberDict.keys()[:]:
										if ((frameNumber < minFrame) or (frameNumber > maxFrame)):
											del frameNumberDict[frameNumber]
									frameNumberList = frameNumberDict.keys()
									# convert frame numbers to seconds
									# frameNumberDict: key = export time, value = frame number
									frameNumberDict = {}
									for frameNumber in frameNumberList:
										if  (actionActuator.startFrame <= actionActuator.endFrame):
											# forward animation
											time = float(frameNumber-actionActuator.startFrame)/fpsNumber.val
										else:
											# backward animation
											time = float(actionActuator.endFrame-frameNumber)/fpsNumber.val
										# update animation duration
										if animation.duration < time:
											animation.duration = time
										frameNumberDict[time] = frameNumber
									# create key frames
									timeList = frameNumberDict.keys()
									timeList.sort()
									for time in timeList:
										frame = frameNumberDict[time]
										loc = [ 0.0, 0.0, 0.0 ]
										rot = [ 0.0, 0.0, 0.0, 1.0 ]
										size = 1.0
										if curveId.has_key("LocX"):
											loc = [ ipo.EvaluateCurveOn(curveId["LocX"], frame) * -1.0 * scale_x, \
												ipo.EvaluateCurveOn(curveId["LocY"], frame) * -1.0 * scale_y, \
												ipo.EvaluateCurveOn(curveId["LocZ"], frame) *  1.0 * scale_z ]
										if curveId.has_key("QuatX"):
											rot = [ ipo.EvaluateCurveOn(curveId["QuatX"], frame), \
												ipo.EvaluateCurveOn(curveId["QuatY"], frame), \
												ipo.EvaluateCurveOn(curveId["QuatZ"], frame), \
												ipo.EvaluateCurveOn(curveId["QuatW"], frame) ]
										if curveId.has_key("SizeX"):
											sx = ipo.EvaluateCurveOn(curveId["SizeX"], frame)
											sy = ipo.EvaluateCurveOn(curveId["SizeY"], frame)
											sz = ipo.EvaluateCurveOn(curveId["SizeZ"], frame)
											size = sx
											size = max(size, sy)
											size = max(size, sz)
										KeyFrame(track, time, loc, rot, size)
									# append track
									animation.tracksDict[boneName] = track
								except NameError:
									# there is no ipo called ipoName
									exportLog.append("error: Unknown Ipo \"%s\" ." % ipoName)
									exportStatus = EXPORT_ERROR
							else:
								# ipo name contains bone but armature doesn't
								exportLog.append("error: ambiguous bone name \"%s\" ." % boneName)
								exportStatus = EXPORT_ERROR
						else:
							# track for that bone already exists
							exportLog.append("error: ambiguous bone name \"%s\" ." % boneName)
							exportStatus = EXPORT_ERROR
					# append animation
					skeleton.animationsDict[actionActuator.name] = animation
				else:
					# animation export name already exists
					exportLog.append("error: ambiguous animation name \"%s\" ." % actionActuator.name)
					exportStatus = EXPORT_ERROR
		else:
			# armature has no actionActuatorListView
			exportLog.append("error: No animation settings for armature \"%s\" ." % object.getName())
			exportStatus = EXPORT_ERROR

		write_skeleton(skeleton)
	return

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
def process_face(face, submesh, mesh, matrix, skeleton):
	"""Process a face of a mesh
	
	   Parameters:
	   	face - Blender.NMesh.NMFace
	   	submesh - SubMesh the face belongs to
	   	mesh - Blender.NMesh.NMesh the face belongs to
	   	matrix - export translation
	   	skeleton - skeleton of the mesh (if any)
	"""
	global verticesDict
	global exportStatus, exportLog
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
			# normal
			if face.smooth:
				normal = normal_by_matrix(face.v[i].no, matrix)
			else:
				normal = faceNormal
			# get vertex 
			if verticesDict.has_key(face.v[i].index):
				# vertex already exists
				vertex = verticesDict[face.v[i].index]
				# uv
				if submesh.material.texture:
					# compare uv and normal
					uv = UVMap()
					if mesh.hasVertexUV():
						# mesh has sticky/per vertex uv coordinates
						uv.u = face.v[i].uvco[0]
						# origin is now in the top-left (Ogre v0.13.0)
						uv.v = 1 - face.v[i].uvco[1]
					else:
						# mesh has per face vertex uv coordinates
						uv.u = face.uv[i][0]
						# origin is now in the top-left (Ogre v0.13.0)
						uv.v = 1 - face.uv[i][1]
					vertexFound = 0
					# check if it has uv
					if (len(vertex.uvmaps) > 0):
						# check if same uv and normal
						if ((math.fabs(vertex.normal[0] - normal[0]) > threshold) or
						    (math.fabs(vertex.normal[1] - normal[1]) > threshold) or
						    (math.fabs(vertex.normal[2] - normal[2]) > threshold) or
						    (math.fabs(vertex.uvmaps[0].u - uv.u) > threshold) or
						    (math.fabs(vertex.uvmaps[0].v - uv.v) > threshold)):
							# check clones
							iClone = 0
							while ((iClone < len(vertex.clones)) and (not vertexFound)):
								# check clone uv and normal
								clone = vertex.clones[iClone]
								if ((math.fabs(clone.normal[0] - normal[0]) < threshold) and
								    (math.fabs(clone.normal[1] - normal[1]) < threshold) and
								    (math.fabs(clone.normal[2] - normal[2]) < threshold) and
								    (math.fabs(clone.uvmaps[0].u - uv.u) < threshold) and
								    (math.fabs(clone.uvmaps[0].v - uv.v) < threshold)):
									vertexFound = 1
									vertex = clone
								iClone += 1
						else:
							# same vertex
							vertexFound = 1
							# vertex = vertex
					if not vertexFound:
						# create new clone
						clone = Vertex(submesh, vertex.loc, normal)
						clone.cloned_from = vertex
						clone.influences = vertex.influences
						clone.uvmaps.append(uv)
						vertex.clones.append(clone)
						# write back to dictionary
						verticesDict[face.v[i].index] = vertex
						vertex = clone

				else:
					# compare normal (no uv coordinates)
					vertexFound = 0
					# check if same normal
					if ((math.fabs(vertex.normal[0] - normal[0]) > threshold) or
					    (math.fabs(vertex.normal[1] - normal[1]) > threshold) or
					    (math.fabs(vertex.normal[2] - normal[2]) > threshold)):
						# check clones
						iClone = 0
						while ((iClone < len(vertex.clones)) and (not vertexFound)):
							# check clone normal
							clone = vertex.clones[iClone]
							if ((math.fabs(clone.normal[0] - normal[0]) < threshold) and
							    (math.fabs(clone.normal[1] - normal[1]) < threshold) and
							    (math.fabs(clone.normal[2] - normal[2]) < threshold)):
								vertexFound = 1
								vertex = clone
							iClone += 1
					else:
						# same vertex
						vertexFound = 1
						# vertex = vertex
					if not vertexFound:
						# create new clone
						clone = Vertex(submesh, vertex.loc, normal)
						clone.cloned_from = vertex
						clone.influences = vertex.influences
						vertex.clones.append(clone)
						# write back to dictionary
						verticesDict[face.v[i].index] = vertex
						vertex = clone

			else:
				# vertex does not exist yet
				# coordinates
				coord  = point_by_matrix (face.v[i].co, matrix)
				# create vertex
				vertex = Vertex(submesh, coord, normal)
				if submesh.material.texture:
					uv = UVMap()
					if mesh.hasVertexUV():
						# mesh has sticky/per vertex uv coordinates
						uv.u = face.v[i].uvco[0]
						# origin is now in the top-left (Ogre v0.13.0)
						uv.v = 1 - face.v[i].uvco[1]
					else:
						# mesh has per face vertex uv coordinates
						uv.u = face.uv[i][0]
						# origin is now in the top-left (Ogre v0.13.0)
						uv.v = 1 - face.uv[i][1]
					vertex.uvmaps.append(uv)
				# set bone influences
				if skeleton:
					influences = mesh.getVertexInfluences(face.v[i].index)
					if not influences:
						exportLog.append("Error: vertex in skinned mesh without influence! check your mesh")
						exportStatus = EXPORT_ERROR

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
		# Split faces with more than 3 vertices
		Face(submesh, face_vertices[0], face_vertices[1], face_vertices[2])
		if len(face.v) == 4:
			Face(submesh, face_vertices[2], face_vertices[3], face_vertices[0])
	else:
		exportLog.append("ignored face with %d edges" % len(face.v))
		exportStatus = EXPORT_WARNING
	return

def export_mesh(object):
	global uvToggle, armatureToggle
	global verticesDict
	global materialsDict
	global exportStatus, exportLog
	
	if (object.getType() == "Mesh"):
		# is this mesh attached to an armature?
		skeleton = 0
		if armatureToggle.val:
			parent = object.getParent()
			if parent and parent.getType() == "Armature":
				export_skeleton(parent)
				skeleton = skeletonsDict[parent.name]

		#NMesh of the object
		data = object.getData()
		matrix = matrix_multiply(BASE_MATRIX, object.getMatrix())

		# materials of the object
		# note: ogre assigns different textures and different facemodes
		#       to different materials
		objectMaterialDict = {}
		# faces assign to objectMaterial keys
		objectMaterialFacesDict = {}

		# note: these are blender materials. Evene if nMaterials = 0
		#       the face can still have a texture (see above)
		nMaterials = len(data.materials)

		# create ogre materials
		for face in data.faces:
			if not(face.mode & Blender.NMesh.FaceModes["INVISIBLE"]):
				# face is visible
				hasTexture = 0
				if ((uvToggle.val) and (data.hasFaceUV()) and (face.mode & Blender.NMesh.FaceModes["TEX"])):
					if face.image:
						hasTexture = 1
					else:
						exportLog.append("Error: Face is textured but has no image assigned!")
						exportStatus = EXPORT_ERROR
				if ((nMaterials > 0 ) or (hasTexture == 1)):
					# create material of the face:
					# blenders material name / FaceTranspMode / texture image name
					# blenders material name
					materialName = ""
					# blenders material name
					faceMaterial = None
					if (nMaterials > 0):
						faceMaterial = data.materials[face.materialIndex]
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
					# texture image name
					textureFile = None
					if hasTexture:
						textureFile = face.image.filename
						materialName += os.path.basename(textureFile)
					# insert into Dicts
					material = objectMaterialDict.get(materialName)
					if not material:
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
		write_mesh(object.getName(), submeshes, skeleton)
	return

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
  global pathString, exportLog
  file = skeleton.name+".skeleton.xml"
  exportLog.append("skeleton  \"%s\"" % file)

  f = open(os.path.join(pathString.val, file), "w")
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
  global pathString, exportLog
  file = name+".mesh.xml"
  exportLog.append("mesh      \"%s\"" % file)

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
	global pathString, materialString, exportLog
	global materialsDict
	file = materialString.val
	exportLog.append("materials \"%s\"" % file)

	f = open(os.path.join(pathString.val, file), "w")
	for name, material in materialsDict.items():
		# material
		f.write("material %s\n" % name)
		f.write("{\n")
		# technique
		f.write(tab(1)+"technique\n")
		f.write(tab(1)+"{\n")
		# pass
		f.write(tab(2)+"pass\n")
		f.write(tab(2)+"{\n")
		if material.mat:
			# pass attributes
			mat = material.mat
			if (not(mat.mode & Blender.Material.Modes["TEXFACE"])):
				# ambient <- amb * rgbCol
				ambR = clamp(mat.amb * mat.rgbCol[0])
				ambG = clamp(mat.amb * mat.rgbCol[1])
				ambB = clamp(mat.amb * mat.rgbCol[2])
				f.write(tab(3)+"ambient %f %f %f\n" % (ambR, ambG, ambB))
				# diffuse <- rgbCol
				diffR = clamp(mat.rgbCol[0])
				diffG = clamp(mat.rgbCol[1])
				diffB = clamp(mat.rgbCol[2])
				f.write(tab(3)+"diffuse %f %f %f\n" % (diffR, diffG, diffB))
				# specular <- spec * specCol, hard
				specR = clamp(mat.spec * mat.specCol[0])
				specG = clamp(mat.spec * mat.specCol[1])
				specB = clamp(mat.spec * mat.specCol[2])
				specShine = mat.hard
				f.write(tab(3)+"specular %f %f %f %f\n" % (specR, specG, specB, specShine))
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
		# scene_blend <- transp
		if (material.mode == Blender.NMesh.FaceTranspModes["ALPHA"]):
			f.write(tab(3)+"scene_blend alpha_blend \n")
		elif (material.mode == Blender.NMesh.FaceTranspModes["ADD"]):
			f.write(tab(3)+"scene_blend add\n")
		if material.texture:
			f.write(tab(3)+"texture_unit\n")
			f.write(tab(3)+"{\n")
			f.write(tab(4)+"texture %s\n" % os.path.basename(material.texture))
			f.write(tab(3)+"}\n") # texture_unit
		f.write(tab(2)+"}\n") # pass
		f.write(tab(1)+"}\n") # technique
		f.write("}\n\n") # material
	f.close()
  
#######################################################################################
## main export

def export(selectedObjectsList):
    global pathString, scaleNumber
    global materialsDict
    global skeletonsDict
    global BASE_MATRIX
    global exportStatus, exportLog
    
    materialsDict = {}
    skeletonsDict = {}

    # set matrix to 90 degree rotation around x-axis and scale
    rot_mat = matrix_rotate_x(-math.pi / 2.0)
    scale_mat = matrix_scale(scaleNumber.val, scaleNumber.val, scaleNumber.val)
    BASE_MATRIX = matrix_multiply(scale_mat, rot_mat)
    
    if not os.path.exists(pathString.val):
      exportLog.append("invalid path: "+pathString.val)
      exportStatus = EXPORT_ERROR
      return

    exportLog.append("exporting selected objects:")    
    n = 0
    for obj in selectedObjectsList:
      if not obj:
        continue
      
      if obj.getType() == "Mesh":
        export_mesh(obj)
        n = 1

      elif obj.getType() == "Armature":
        export_skeleton(obj)

    if n == 0:
        exportLog.append("no mesh objects selected!")
        exportStatus = EXPORT_WARNING

    if len(materialsDict):
      write_materials()

    exportLog.append("finished.")
    return exportStatus
    
#######################################################################################
## GUI

######
# global variables
######
# see above

######
# methods
######
def saveSettings(filename):
	"""Save all exporter settings of selected and unselected objects to a file.
	
	   Settings belonging to removed objects in the .blend file will not be saved.
	   
	   Parameters:
	   	filename - where to store the settings
	   Return:
	   	 true on success, else false
	"""
	global uvToggle
	global armatureToggle
	global armatureMeshToggle
	global pathString
	global materialString
	global scaleNumber
	global fpsNumber
	global selectedObjectsList
	global armatureDict
	global actionActuatorListViewDict
	global animationDictListDict
	settingsDict = {}
	success = 0
	# to open file
	try:
		fileHandle = open(filename,'w')
		# save general settings
		settingsDict['uvToggle'] = uvToggle.val
		settingsDict['armatureToggle'] = armatureToggle.val
		settingsDict['armatureMeshToggle'] = armatureMeshToggle.val
		settingsDict['pathString'] = pathString.val
		settingsDict['materialString'] = materialString.val
		settingsDict['scaleNumber'] = scaleNumber.val
		settingsDict['fpsNumber'] = fpsNumber.val
		# save object specific settings
		# check if armature exists (I think this is cleaner than catching NameError exceptions.)
		# create list of valid armature names
		armatureNameList = []
		for object in Blender.Object.Get():
			if (object.getType() == "Armature"):
				armatureNameList.append(object.getName())
		for armatureName in animationDictListDict.keys():
			if not(armatureName in armatureNameList):
				# remove obsolete settings
				del animationDictListDict[armatureName]
		# update settings
		for armatureName in actionActuatorListViewDict.keys():
			animationDictListDict[armatureName] = actionActuatorListViewDict[armatureName].getAnimationDictList()
		settingsDict['animationDictListDict'] = animationDictListDict
		pickler = pickle.Pickler(fileHandle)
		pickler.dump(settingsDict)
		# close file
		fileHandle.close()
		success = 1
	except IOError, (errno, strerror):
		print "I/O Error(%s): %s" % (errno, strerror)
	return success

def loadSettings(filename):
	"""Load all exporter settings from a file.
	
	   You have to create actionActuatorListViews with the new
	   animationDictListDict if you want the animation settings
	   to take effect.
	
	   Parameters:
	   	filename - where to store the settings
	   Return:
	   	 true on success, else false
	"""
	global uvToggle
	global armatureToggle
	global armatureMeshToggle
	global pathString
	global materialString
	global scaleNumber
	global fpsNumber
	global selectedObjectsList
	global armatureDict
	global actionActuatorListViewDict
	global animationDictListDict
	success = 0
	if os.path.isfile(filename) :
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
				# set general settings
				if settingsDict.has_key('uvToggle'):
					uvToggle = Blender.Draw.Create(settingsDict['uvToggle'])
				if settingsDict.has_key('armatureToggle'):
					armatureToggle = Blender.Draw.Create(settingsDict['armatureToggle'])
				if settingsDict.has_key('armatureMeshToggle'):
					armatureMeshToggle = Blender.Draw.Create(settingsDict['armatureMeshToggle'])
				if settingsDict.has_key('pathString'):
					pathString = Blender.Draw.Create(settingsDict['pathString'])
				if settingsDict.has_key('materialString'):
					materialString = Blender.Draw.Create(settingsDict['materialString'])
				if settingsDict.has_key('scaleNumber'):
					scaleNumber = Blender.Draw.Create(settingsDict['scaleNumber'])
				if settingsDict.has_key('fpsNumber'):
					fpsNumber = Blender.Draw.Create(settingsDict['fpsNumber'])
				# set object specific settings
				if settingsDict.has_key('animationDictListDict'):
					animationDictListDict = settingsDict['animationDictListDict']
				success = 1
	return success
	
def refreshGUI():
	"""refresh GUI after export and selection change
	"""
	global exportStatus, exportLog
	global selectedObjectsList, armatureToggle, armatureDict, actionActuatorListViewDict
	global animationDictListDict
	# export settings
	exportStatus = EXPORT_SUCCESS
	exportLog = []
	# synchronize animationDictListDict
	for armatureName in actionActuatorListViewDict.keys():
		animationDictListDict[armatureName] = actionActuatorListViewDict[armatureName].getAnimationDictList()
	selectedObjectsList = Blender.Object.GetSelected()
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
	# refresh ActionActuatorListViews
	for armatureName in armatureDict.values():
		# create actionDict
		actionDict = Action.createActionDict(Action(), Blender.Object.Get(armatureName).data)
		# get animationDictList
		animationDictList = None
		if animationDictListDict.has_key(armatureName):
			animationDictList = animationDictListDict[armatureName]
		if actionActuatorListViewDict.has_key(armatureName):
			# refresh actionActuators
			actionActuatorListViewDict[armatureName].refresh(actionDict)
		else:
			# create actionActuatorListView
			actionActuatorListViewDict[armatureName] = ActionActuatorListView(actionDict, MAXACTUATORS, BUTTON_EVENT_ACTUATOR_RANGESTART, animationDictList)
	return

def initGUI():
	"""initialization of the GUI
	"""
	global actionActuatorListViewDict
	if KEEP_SETTINGS:
		# load exporter settings
		loadSettings(Blender.Get('filename')+".ogre")
	actionActuatorListViewDict = {}
	refreshGUI()
	return

def exitGUI():
	if KEEP_SETTINGS:
		# save exporter settings
		saveSettings(Blender.Get('filename')+".ogre")
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
	
	exit on ESCKEY
	exit on QKEY
	"""
	global scrollbar
	global selectedObjectsList, selectedObjectsMenu, actionActuatorListViewDict, armatureDict
	# eventFilter for current ActionActuatorListView
	if (len(selectedObjectsList) > 0):
		selectedObjectsListIndex = selectedObjectsMenu.val
		selectedObjectName = selectedObjectsList[selectedObjectsListIndex].getName()
		if armatureDict.has_key(selectedObjectName):
			armatureName = armatureDict[selectedObjectName]
			actionActuatorListViewDict[armatureName].eventFilter(event, value)
	scrollbar.eventFilter(event, value)
	if not(value == 0):
		# pressed
		if (event == Draw.ESCKEY):
			exitGUI()
		if (event == Draw.QKEY):
			exitGUI()
	return

def buttonCallback(event):
	"""handles button events
	"""
	global materialString, doneMessage, doneMessageBox, eventCallback, buttonCallback, scrollbar
	global selectedObjectsList, selectedObjectsMenu, actionActuatorListViewDict, armatureDict
	# buttonFilter for current ActionActuatorListView
	if (len(selectedObjectsList) > 0):
		selectedObjectsListIndex = selectedObjectsMenu.val
		selectedObjectName = selectedObjectsList[selectedObjectsListIndex].getName()
		if armatureDict.has_key(selectedObjectName):
			armatureName = armatureDict[selectedObjectName]
			actionActuatorListViewDict[armatureName].buttonFilter(event)
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
		Blender.Window.FileSelector(pathSelectCallback, "Export Directory")
		Draw.Redraw(1)
	elif (event == BUTTON_EVENT_MATERIALSTRING): # materialString
		materialString = Blender.Draw.Create(os.path.basename(materialString.val))
		if (len(materialString.val) == 0):
			materialString = Blender.Draw.Create("export.material")
		Draw.Redraw(1)
	elif (event == BUTTON_EVENT_SCROLLBAR): # scrollbar
		Draw.Redraw(1)
	elif (event == BUTTON_EVENT_EXPORT): # export
		# export
		status = export(selectedObjectsList)
		if (status == EXPORT_SUCCESS):
			doneMessage = EXPORT_SUCCESS_MESSAGE
		elif (status == EXPORT_WARNING):
			doneMessage = EXPORT_WARNING_MESSAGE
		elif (status == EXPORT_ERROR):
			doneMessage = EXPORT_ERROR_MESSAGE
		# set donemessage
		scrollbar = ReplacementScrollbar(0,0,len(exportLog)-1,BUTTON_EVENT_SCROLLBARUP,BUTTON_EVENT_SRCROLLBARDOWN)
		Draw.Register(doneMessageBox, eventCallback, buttonCallback)
	return

def gui():
	"""draws the screen
	"""
	global uvToggle, armatureToggle, armatureMeshToggle, pathString, materialString, \
		scaleNumber, fpsNumber, scrollbar
	global selectedObjectsList, selectedObjectsMenu, actionActuatorListViewDict, armatureDict
	# get size of the window
	guiRectBuffer = Buffer(GL_FLOAT, 4)
	glGetFloatv(GL_SCISSOR_BOX, guiRectBuffer)
	guiRect =  guiRectBuffer.list
	guiRect[0] = 0;
	guiRect[1] = 0;
	
	remainRect = guiRect[:]
	remainRect[0] += 10
	remainRect[1] += 10
	remainRect[2] -= 10
	remainRect[3] -= 10
	
	# clear background
	glClearColor(0.6,0.6,0.6,1) # Background: grey
	glClear(GL_COLOR_BUFFER_BIT)
	
	# title
	glColor3f(52.0/255,154.0/255,52.0/255)
	glRectf(remainRect[0],remainRect[3]-20,remainRect[2],remainRect[3])
	remainRect[3] -= 20
	glColor3f(0,0,0) # Defaul color: black
	glRasterPos2i(remainRect[0]+4,remainRect[3]+7)
	Draw.Text("Ogre Exporter 0.13.1","normal")
	
	# export settings
	remainRect[3] -= 5
	# first row
	uvToggle = Draw.Toggle("Export Textures", BUTTON_EVENT_UVTOGGLE, \
				remainRect[0], remainRect[3]-25, 220, 20, \
				uvToggle.val, "export uv coordinates and texture names, if available")
	# Material file
	materialString = Draw.String("Material File: ", BUTTON_EVENT_MATERIALSTRING, \
			remainRect[0]+230,remainRect[3]-25, 220, 20, \
			materialString.val, 255,"all material definitions go in this file (relative to the save path)")

	remainRect[3] -= 25
	# second row
	armatureToggle = Draw.Toggle("Export Armature", BUTTON_EVENT_ARMATURETOGGLE, \
				remainRect[0], remainRect[3]-25, 220, 20, \
				armatureToggle.val, "export skeletons and bone weights in meshes")
	# Scale and FPS settings
	scaleNumber = Draw.Number("Mesh Scale Factor: ", BUTTON_EVENT_SCALENUMBER, \
			remainRect[0]+230, remainRect[3]-25, 220, 20, \
			scaleNumber.val, 0.0, 1000.0, "scale factor")
	remainRect[3] -= 25
	# third row
	if (armatureToggle.val == 1):
		armatureMeshToggle = Draw.Toggle("Export Armature as Mesh", BUTTON_EVENT_ARMATUREMESHTOGGLE, \
				remainRect[0], remainRect[3]-25, 220, 20, \
				armatureMeshToggle.val, "create an extra mesh with the form of the skeleton")
	if (armatureToggle.val == 1):
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
	if (armatureToggle.val == 1):
		animationText = "Animation settings of"
		glRasterPos2i(remainRect[0],remainRect[3]-15)
		Draw.Text(animationText)
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
		                      selectedObjectsMenu.val, "choose one of the selected objects")
		xOffset += 141
		# update button
		Draw.Button("Update", BUTTON_EVENT_UPDATEBUTTON, remainRect[0]+xOffset, remainRect[3]-20, 60, 20, "update list of selected objects")
		remainRect[3] -= 25
		# draw actionActuator
		if (len(selectedObjectsList) > 0):
			selectedObjectsListIndex = selectedObjectsMenu.val
			selectedObjectName = selectedObjectsList[selectedObjectsListIndex].getName()
			if armatureDict.has_key(selectedObjectName):
				armatureName = armatureDict[selectedObjectName]
				actionActuatorListViewDict[armatureName].draw(remainRect[0], remainRect[1], remainRect[2]-remainRect[0], remainRect[3]-remainRect[1])
	return

def doneMessageBox():
	"""displays export message and log
	"""
	global doneMessage, exportLog
	# get size of the window
	guiRectBuffer = Buffer(GL_FLOAT, 4)
	glGetFloatv(GL_SCISSOR_BOX, guiRectBuffer)
	guiRect =  guiRectBuffer.list
	guiRect[0] = 0;
	guiRect[1] = 0;
	
	remainRect = guiRect[:]
	remainRect[0] += 10
	remainRect[1] += 10
	remainRect[2] -= 10
	remainRect[3] -= 10
	
	# clear background
	glClearColor(0.6,0.6,0.6,1) # Background: grey
	glClear(GL_COLOR_BUFFER_BIT)
	
	# title
	glColor3f(52.0/255,154.0/255,52.0/255)
	glRectf(remainRect[0],remainRect[3]-20,remainRect[2],remainRect[3])
	remainRect[3] -= 20
	glColor3f(0,0,0) # Defaul color: black
	glRasterPos2i(remainRect[0]+4,remainRect[3]+7)
	Draw.Text("Ogre Exporter","normal")

	# Ok button
	Draw.Button("OK", BUTTON_EVENT_OK,10,10,100,30,"return to export settings")
	Draw.Button("Quit", BUTTON_EVENT_QUIT,guiRect[2]-110,10,100,30,"quit export script")
	remainRect[1] += 40
	
	# message
	remainRect[3] -= 20
	glRasterPos2i(remainRect[0],remainRect[3])
	glColor3f(0,0,0) # Defaul color: black
	Draw.Text(doneMessage,"normal")
	remainRect[3] -= 20
	glRasterPos2i(remainRect[0],remainRect[3])
	glColor3f(0,0,0) # Defaul color: black
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
	glColor3f(0.6,0.6,0.6) # Background: grey
	glRectf(logRect[0],logRect[1],logRect[2],logRect[3])
	
	# display exportLog
	scrollPanelRect = remainRect[:]
	loglineiMax = len(exportLog)
	loglinei = scrollbar.getCurrentValue()
	glColor3f(0,0,0)
	while (((logRect[3]-logRect[1]) >= 20) and ( loglinei < loglineiMax )):
		logRect[3] -= 16
		glRasterPos2i(logRect[0]+4,logRect[3])
		Draw.Text(exportLog[loglinei])
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
