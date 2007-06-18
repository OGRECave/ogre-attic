/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "PassPropertyGridPage.h"

#include <wx/propgrid/advprops.h>

#include "OgreBlendMode.h"
#include "OgreCommon.h"
#include "OgrePass.h"
#include "OgrePrerequisites.h"

#include "PassController.h"

using namespace Ogre;

BEGIN_EVENT_TABLE(PassPropertyGridPage, wxPropertyGridPage)
	EVT_PG_CHANGED(1, PassPropertyGridPage::propertyChanged)
END_EVENT_TABLE()

PassPropertyGridPage::PassPropertyGridPage(PassController* controller)
: mController(controller)
{
}

PassPropertyGridPage::~PassPropertyGridPage()
{
}

void PassPropertyGridPage::createPage()
{
	const Pass* pass = mController->getPass();
	
	// Name
	Append(wxStringProperty(wxT("Name"), wxPG_LABEL, pass->getName()));
	
	//
	// Reflectance (parent)
	//
	wxPGId reflectance = Append(wxParentProperty(wxT("Reflectance"), wxPG_LABEL));
	
	// Ambient
	ColourValue ambient = pass->getAmbient();
	AppendIn(reflectance, wxColourProperty(wxT("Ambient"), wxPG_LABEL, wxColour((int)(255 * ambient.r), (int)(255 * ambient.g), (int)(255 * ambient.b))));
	
	// Diffuse
	ColourValue diffuse = pass->getDiffuse();
	AppendIn(reflectance, wxColourProperty(wxT("Diffuse"), wxPG_LABEL, wxColour((int)(255 * diffuse.r), (int)(255 * diffuse.g), (int)(255 * diffuse.b))));
	
	// Specular
	ColourValue specular = pass->getSpecular();
	AppendIn(reflectance, wxColourProperty(wxT("Specular"), wxPG_LABEL, wxColour((int)(255 * specular.r), (int)(255 * specular.g), (int)(255 * specular.b))));
	
	// Shininess
	AppendIn(reflectance, wxFloatProperty(wxT("Shininess"), wxPG_LABEL, pass->getShininess()));
	
	// Self Illumination
	ColourValue selfIllum = pass->getSelfIllumination();
	Append(wxColourProperty(wxT("Self Illumination"), wxPG_LABEL, wxColour((int)(255 * selfIllum.r), (int)(255 * selfIllum.g), (int)(255 * selfIllum.b))));
	
	// Track Vertex Colour Type
	wxPGChoices vertexColourTypeChoices;
	vertexColourTypeChoices.Add(wxT("None"), TrackVertexColourEnum::TVC_NONE);
	vertexColourTypeChoices.Add(wxT("Ambient"), TrackVertexColourEnum::TVC_AMBIENT);
	vertexColourTypeChoices.Add(wxT("Diffuse"), TrackVertexColourEnum::TVC_DIFFUSE);
	vertexColourTypeChoices.Add(wxT("Specular"), TrackVertexColourEnum::TVC_SPECULAR);
	vertexColourTypeChoices.Add(wxT("Emissive"), TrackVertexColourEnum::TVC_EMISSIVE);
	
	Append(wxEnumProperty(wxT("Track Vertex Colour Type"), wxPG_LABEL, vertexColourTypeChoices, pass->getVertexColourTracking()));
	
	// Point
	wxPGId point = Append(wxParentProperty(wxT("Point"), wxPG_LABEL));
	
	// Point Size
	wxPGId pointSize = AppendIn(point, wxParentProperty(wxT("Size"), wxPG_LABEL));
	AppendIn(pointSize, wxFloatProperty(wxT("Size"), wxPG_LABEL, pass->getPointSize()));
	AppendIn(pointSize, wxFloatProperty(wxT("Min"), wxPG_LABEL, pass->getPointMinSize()));
	AppendIn(pointSize, wxFloatProperty(wxT("Max"), wxPG_LABEL, pass->getPointMaxSize()));
	
	// Point Sprites
	Append(wxBoolProperty(wxT("Point Sprites"), wxPG_LABEL, pass->getPointSpritesEnabled()));
	
	// Point Attenuation
	wxPGId attenuation = Append(wxParentProperty(wxT("Attenuation"),wxPG_LABEL)); 
	AppendIn(attenuation, wxBoolProperty(wxT("Enabled"), wxPG_LABEL, pass->isPointAttenuationEnabled()));
	AppendIn(attenuation, wxFloatProperty(wxT("Constant"), wxPG_LABEL, pass->getPointAttenuationConstant()));
	AppendIn(attenuation, wxFloatProperty(wxT("Linear"), wxPG_LABEL, pass->getPointAttenuationLinear()));
	AppendIn(attenuation, wxFloatProperty(wxT("Quadratic"), wxPG_LABEL, pass->getPointAttenuationQuadratic()));
	
	//
	// Scene Blending
	//
	wxPGId sceneBlending = Append(wxParentProperty(wxT("Scene Blending"), wxPG_LABEL));
	
	wxPGChoices sceneBlendTypeChoices;
	sceneBlendTypeChoices.Add(wxT("Transparent Alpha"), SceneBlendType::SBT_TRANSPARENT_ALPHA);
	sceneBlendTypeChoices.Add(wxT("Transparent Colour"), SceneBlendType::SBT_TRANSPARENT_COLOUR);
	sceneBlendTypeChoices.Add(wxT("Add"), SceneBlendType::SBT_ADD);
	sceneBlendTypeChoices.Add(wxT("Modulate"), SceneBlendType::SBT_MODULATE);
	sceneBlendTypeChoices.Add(wxT("Replace"), SceneBlendType::SBT_REPLACE);
	
	//
	// Blending ???
	//
	
	// Scene Blend Type
	//AppendIn(sceneBlending, wxEnumProperty(wxT("Type"), wxPG_LABEL, sceneBlendTypeChoices, pass->getScene);
	
	// Source Scene Blend Type
	//AppendIn(sceneBlending, wxEnumProperty(wxT("Source Type"), wxPG_LABEL, sceneBlendTypeChoices, pass->getSourceBlendFactor());
	
	// Destination Scene Blend Type
	//AppendIn(sceneBlending, wxEnumProperty(wxT("Destination Type"), wxPG_LABEL, sceneBlendTypeChoices, pass->getSceneBlendType());
	
	//
	// Depth ???
	//
	
	// Depth Check
	Append(wxBoolProperty(wxT("Depth Check"), wxPG_LABEL, pass->getDepthCheckEnabled()));
		
	// Depth Write
	Append(wxBoolProperty(wxT("Depth Write"), wxPG_LABEL, pass->getDepthWriteEnabled()));	
	
	//  Depth Function
	wxPGChoices compareFuncChoices;
	compareFuncChoices.Add(wxT("Fail"), CompareFunction::CMPF_ALWAYS_FAIL);
	compareFuncChoices.Add(wxT("Pass"), CompareFunction::CMPF_ALWAYS_PASS);
	compareFuncChoices.Add(wxT("<"), CompareFunction::CMPF_LESS);
	compareFuncChoices.Add(wxT("<="), CompareFunction::CMPF_LESS_EQUAL);
	compareFuncChoices.Add(wxT("=="), CompareFunction::CMPF_EQUAL);
	compareFuncChoices.Add(wxT("!="), CompareFunction::CMPF_NOT_EQUAL);
	compareFuncChoices.Add(wxT(">="), CompareFunction::CMPF_GREATER_EQUAL);
	compareFuncChoices.Add(wxT(">"), CompareFunction::CMPF_GREATER);
	
	Append(wxEnumProperty(wxT("Depth Function"), wxPG_LABEL, compareFuncChoices, pass->getDepthFunction()));
	
	// Colour Write
	Append(wxBoolProperty(wxT("Colour Write"), wxPG_LABEL, pass->getColourWriteEnabled()));	
	
	//
	// Culing ???
	//
	
	// Culling Mode
	wxPGChoices cullingModeChoices;
	cullingModeChoices.Add(wxT("None"), CullingMode::CULL_NONE);
	cullingModeChoices.Add(wxT("Clockwise"), CullingMode::CULL_CLOCKWISE);
	cullingModeChoices.Add(wxT("Counterclockwise"), CullingMode::CULL_ANTICLOCKWISE);
	
	Append(wxEnumProperty(wxT("Culling Mode"), wxPG_LABEL, cullingModeChoices, pass->getDepthFunction()));
	
	// Manual Culling Mode
	wxPGChoices manualCullingModeChoices;
	manualCullingModeChoices.Add(wxT("None"), ManualCullingMode::MANUAL_CULL_NONE);
	manualCullingModeChoices.Add(wxT("Back"), ManualCullingMode::MANUAL_CULL_BACK);
	manualCullingModeChoices.Add(wxT("Front"), ManualCullingMode::MANUAL_CULL_FRONT);
	
	Append(wxEnumProperty(wxT("Manual Culling Mode"), wxPG_LABEL, manualCullingModeChoices, pass->getManualCullingMode()));
	
	// Lighting
	Append(wxBoolProperty(wxT("Lighting"), wxPG_LABEL, pass->getLightingEnabled()));	
	
	// Max Simultaneous Lights
	Append(wxIntProperty(wxT("Max Lights"), wxPG_LABEL, pass->getMaxSimultaneousLights()));
	
	// Start Light
	Append(wxIntProperty(wxT("Start Light"), wxPG_LABEL, pass->getStartLight()));
	
	// Shading Mode
	wxPGChoices shadingModeChoices;
	shadingModeChoices.Add(wxT("Flat"), ShadeOptions::SO_FLAT);
	shadingModeChoices.Add(wxT("Gouraud"), ShadeOptions::SO_GOURAUD);
	shadingModeChoices.Add(wxT("Phong"), ShadeOptions::SO_PHONG);
	
	Append(wxEnumProperty(wxT("Shading Mode"), wxPG_LABEL, shadingModeChoices, pass->getShadingMode()));
	
	// Polygon Mode
	wxPGChoices polygonModeChoices;
	polygonModeChoices.Add(wxT("Points"), PolygonMode::PM_POINTS);
	polygonModeChoices.Add(wxT("Wireframe"), PolygonMode::PM_WIREFRAME);
	polygonModeChoices.Add(wxT("Solid"), PolygonMode::PM_SOLID);
	
	Append(wxEnumProperty(wxT("Polygon Mode"), wxPG_LABEL, polygonModeChoices, pass->getPolygonMode()));
	
	//
	// Fog
	//
	wxPGId fog = Append(wxParentProperty(wxT("Fog"), wxPG_LABEL));
	
	// Fog Enabled
	AppendIn(fog, wxBoolProperty(wxT("Override Scene"), wxPG_LABEL, pass->getFogOverride()));	
	
	// Fog Mode
	wxPGChoices fogModeChoices;
	fogModeChoices.Add(wxT("None"), FogMode::FOG_NONE);
	fogModeChoices.Add(wxT("EXP"), FogMode::FOG_EXP);
	fogModeChoices.Add(wxT("EXP2"), FogMode::FOG_EXP2);
	fogModeChoices.Add(wxT("Linear"), FogMode::FOG_LINEAR);
	
	AppendIn(fog, wxEnumProperty(wxT("Fog Mode"), wxPG_LABEL, fogModeChoices, pass->getFogMode()));
	
	//
	// Depth Bias
	//
	wxPGId depthBias = Append(wxParentProperty(wxT("Depth Bias"), wxPG_LABEL));
	
	// Constant Bias
	AppendIn(depthBias, wxFloatProperty(wxT("Constant"), wxPG_LABEL, pass->getDepthBiasConstant()));
	
	// Slope Bias
	AppendIn(depthBias, wxFloatProperty(wxT("Slope Scale"), wxPG_LABEL, pass->getDepthBiasSlopeScale()));
	
	//
	// Alpha Reject
	//
	wxPGId alphaReject = Append(wxParentProperty(wxT("Alpha Reject"), wxPG_LABEL));
		
	// Alpha Reject Func
	AppendIn(alphaReject, wxEnumProperty(wxT("Function"), wxPG_LABEL, compareFuncChoices, pass->getAlphaRejectFunction()));
	
	// Alpha Reject Value
	AppendIn(alphaReject, wxIntProperty(wxT("Value"), wxPG_LABEL, pass->getAlphaRejectValue()));
	
	//
	// Light Iteration
	//
	wxPGId lightIteration = wxParentProperty(wxT("Light Iteration"), wxPG_LABEL);
	AppendIn(lightIteration, wxBoolProperty(wxT("Enabled"), wxPG_LABEL, pass->getIteratePerLight()));
	
	
}

void PassPropertyGridPage::propertyChanged(wxPropertyGridEvent& event)
{
}
















