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

void PassPropertyGridPage::populate()
{	
	createGeneralCategory();
	createReflectanceCategory();
	createPointCategory();
	createSceneBlendingCategory();
	createDepthCategory();
	createCullingCategory();
	createIlluminationCategory();
	createFogCategory();
	createAlphaRejectionCategory();
	createMiscCategory();
}

void PassPropertyGridPage::createGeneralCategory()
{
	const Pass* pass = mController->getPass();

	Append(wxPropertyCategory(wxT("General")));

	// Name
	wxPGId propNameId = Append(wxStringProperty(wxT("Name"), wxPG_LABEL, pass->getName()));
	SetPropertyHelpString(propNameId, wxT("Name of this Pass"));
}

void PassPropertyGridPage::createReflectanceCategory()
{
	const Pass* pass = mController->getPass();

	Append(wxPropertyCategory(wxT("Reflectance")));

	// Ambient
	ColourValue ambient = pass->getAmbient();
	wxPGId propAmbientId = Append(wxColourProperty(wxT("Ambient"), wxPG_LABEL, wxColour((int)(255 * ambient.r), (int)(255 * ambient.g), (int)(255 * ambient.b))));
	SetPropertyHelpString(propAmbientId, wxT("Ambient colour reflectance"));

	// Diffuse
	ColourValue diffuse = pass->getDiffuse();
	wxPGId propDiffuseId = Append(wxColourProperty(wxT("Diffuse"), wxPG_LABEL, wxColour((int)(255 * diffuse.r), (int)(255 * diffuse.g), (int)(255 * diffuse.b))));
	SetPropertyHelpString(propDiffuseId, wxT("Diffuse colour reflectance"));

	// Specular
	ColourValue specular = pass->getSpecular();
	wxPGId propSpecularId = Append(wxColourProperty(wxT("Specular"), wxPG_LABEL, wxColour((int)(255 * specular.r), (int)(255 * specular.g), (int)(255 * specular.b))));
	SetPropertyHelpString(propSpecularId, wxT("Specular colour reflectance"));

	// Shininess
	wxPGId propShininessId = Append(wxFloatProperty(wxT("Shininess"), wxPG_LABEL, pass->getShininess()));
	SetPropertyHelpString(propShininessId, wxT("Shininess, affecting the size of specular highlights"));
}


void PassPropertyGridPage::createPointCategory()
{
	const Pass* pass = mController->getPass();

	Append(wxPropertyCategory(wxT("Point"), wxPG_LABEL));

	// Point Size
	wxPGId pointSize = Append(wxParentProperty(wxT("Size"), wxPG_LABEL));
	wxPGId propPointSizeId = AppendIn(pointSize, wxFloatProperty(wxT("Size"), wxPG_LABEL, pass->getPointSize()));
	SetPropertyHelpString(propPointSizeId, wxT("Point size, affecting the size of points when rendering a point list, or a list of point sprites"));

	// Point Sprites
	wxPGId propPointSpritesId = Append(wxBoolProperty(wxT("Point Sprites"), wxPG_LABEL, pass->getPointSpritesEnabled()));

	// Point Attenuation
	wxPGId attenuation = Append(wxParentProperty(wxT("Attenuation"),wxPG_LABEL)); 
	SetPropertyHelpString(attenuation, wxT("Determines how points are attenuated with distance"));
	wxPGId propPointAttenuationId = AppendIn(attenuation, wxBoolProperty(wxT("Enabled"), wxPG_LABEL, pass->isPointAttenuationEnabled()));
	wxPGId propPointMinSizeId = AppendIn(attenuation, wxFloatProperty(wxT("Min"), wxPG_LABEL, pass->getPointMinSize()));
	SetPropertyHelpString(propPointMinSizeId, wxT("Minimum point size, when point attenuation is in use"));
	wxPGId propPointMaxSizeId = AppendIn(attenuation, wxFloatProperty(wxT("Max"), wxPG_LABEL, pass->getPointMaxSize()));
	SetPropertyHelpString(propPointMaxSizeId, wxT("Maximum point size, when point attenuation is in use"));
	wxPGId propPointAttenuationConstantId = AppendIn(attenuation, wxFloatProperty(wxT("Constant"), wxPG_LABEL, pass->getPointAttenuationConstant()));
	SetPropertyHelpString(propPointAttenuationConstantId, wxT("Constant coefficient of the point attenuation"));
	wxPGId propPointAttenuationLinearId = AppendIn(attenuation, wxFloatProperty(wxT("Linear"), wxPG_LABEL, pass->getPointAttenuationLinear()));
	SetPropertyHelpString(propPointAttenuationLinearId, wxT("Linear coefficient of the point attenuation"));
	wxPGId propPointAttenuationQuadraticId = AppendIn(attenuation, wxFloatProperty(wxT("Quadratic"), wxPG_LABEL, pass->getPointAttenuationQuadratic()));
	SetPropertyHelpString(propPointAttenuationQuadraticId, wxT("Quadratic coefficient of the point attenuation"));
}


void PassPropertyGridPage::createSceneBlendingCategory()
{
	const Pass* pass = mController->getPass();

	Append(wxPropertyCategory(wxT("Scene Blending")));

	wxPGChoices sbtChoices;
	sbtChoices.Add(wxT("N/A") -1);
	sbtChoices.Add(wxT("Transparent Alpha"), SBT_TRANSPARENT_ALPHA);
	sbtChoices.Add(wxT("Transparent Colour"), SBT_TRANSPARENT_COLOUR);
	sbtChoices.Add(wxT("Add"), SBT_ADD);
	sbtChoices.Add(wxT("Modulate"), SBT_MODULATE);
	sbtChoices.Add(wxT("Replace"), SBT_REPLACE);

	wxPGChoices sbfChoices;
	sbfChoices.Add(wxT("One"), SBF_ONE);
	sbfChoices.Add(wxT("Zero"), SBF_ZERO);
	sbfChoices.Add(wxT("Dest Colour"), SBF_DEST_COLOUR);
	sbfChoices.Add(wxT("Src Colour"), SBF_SOURCE_COLOUR);
	sbfChoices.Add(wxT("One Minus Dest Colour"), SBF_ONE_MINUS_DEST_COLOUR);
	sbfChoices.Add(wxT("One Minus Src Colour"), SBF_ONE_MINUS_SOURCE_COLOUR);
	sbfChoices.Add(wxT("Dest Alpha"), SBF_DEST_ALPHA);
	sbfChoices.Add(wxT("Source Alpha"), SBF_SOURCE_ALPHA);
	sbfChoices.Add(wxT("One Minus Dest Alpha"), SBF_ONE_MINUS_DEST_ALPHA);
	sbfChoices.Add(wxT("One Minus Source Alpha"), SBF_ONE_MINUS_SOURCE_ALPHA);

	// Scene Blend Type
	bool type = true;
	SceneBlendType blendType;
	SceneBlendFactor srcFactor = pass->getSourceBlendFactor();
	SceneBlendFactor destFactor = pass->getDestBlendFactor();
	if(srcFactor == SBF_SOURCE_ALPHA && destFactor == SBF_ONE_MINUS_SOURCE_ALPHA)
		blendType = SBT_TRANSPARENT_ALPHA;
	else if(srcFactor == SBF_SOURCE_COLOUR && destFactor == SBF_ONE_MINUS_SOURCE_COLOUR)
		blendType = SBT_TRANSPARENT_COLOUR;
	else if(srcFactor == SBF_DEST_COLOUR && destFactor == SBF_ZERO)
		blendType = SBT_MODULATE;
	else if(srcFactor == SBF_ONE && destFactor == SBF_ONE)
		blendType = SBT_ADD;
	else if(srcFactor == SBF_ONE && destFactor == SBF_ZERO)
		blendType= SBT_REPLACE;
	else type = false;

	Append(wxEnumProperty(wxT("Type"), wxPG_LABEL, sbtChoices, (type) ? blendType : 0));

	// Source Scene Blend Type
	Append(wxEnumProperty(wxT("Src Factor"), wxPG_LABEL, sbfChoices, srcFactor));

	// Destination Scene Blend Type
	Append(wxEnumProperty(wxT("Dest Factor"), wxPG_LABEL, sbfChoices, destFactor));
}


void PassPropertyGridPage::createDepthCategory()
{
	const Pass* pass = mController->getPass();

	Append(wxPropertyCategory(wxT("Depth")));

	// Depth Check
	Append(wxBoolProperty(wxT("Depth Check"), wxPG_LABEL, pass->getDepthCheckEnabled()));

	// Depth Write
	Append(wxBoolProperty(wxT("Depth Write"), wxPG_LABEL, pass->getDepthWriteEnabled()));	

	//  Depth Function
	wxPGChoices compareFuncChoices;
	compareFuncChoices.Add(wxT("Fail"), CMPF_ALWAYS_FAIL);
	compareFuncChoices.Add(wxT("Pass"), CMPF_ALWAYS_PASS);
	compareFuncChoices.Add(wxT("<"), CMPF_LESS);
	compareFuncChoices.Add(wxT("<="), CMPF_LESS_EQUAL);
	compareFuncChoices.Add(wxT("=="), CMPF_EQUAL);
	compareFuncChoices.Add(wxT("!="), CMPF_NOT_EQUAL);
	compareFuncChoices.Add(wxT(">="), CMPF_GREATER_EQUAL);
	compareFuncChoices.Add(wxT(">"), CMPF_GREATER);

	Append(wxEnumProperty(wxT("Depth Function"), wxPG_LABEL, compareFuncChoices, pass->getDepthFunction()));

	wxPGId depthBias = Append(wxParentProperty(wxT("Depth Bias"), wxPG_LABEL));

	// Constant Bias
	AppendIn(depthBias, wxFloatProperty(wxT("Constant"), wxPG_LABEL, pass->getDepthBiasConstant()));

	// Slope Bias
	AppendIn(depthBias, wxFloatProperty(wxT("Slope Scale"), wxPG_LABEL, pass->getDepthBiasSlopeScale()));
}

void PassPropertyGridPage::createCullingCategory()
{
	const Pass* pass = mController->getPass();

	Append(wxPropertyCategory(wxT("Culling")));

	// Culling Mode
	wxPGChoices cullingModeChoices;
	cullingModeChoices.Add(wxT("None"), CULL_NONE);
	cullingModeChoices.Add(wxT("Clockwise"), CULL_CLOCKWISE);
	cullingModeChoices.Add(wxT("Counterclockwise"), CULL_ANTICLOCKWISE);

	Append(wxEnumProperty(wxT("Culling Mode"), wxPG_LABEL, cullingModeChoices, pass->getDepthFunction()));

	// Manual Culling Mode
	wxPGChoices manualCullingModeChoices;
	manualCullingModeChoices.Add(wxT("None"), MANUAL_CULL_NONE);
	manualCullingModeChoices.Add(wxT("Back"), MANUAL_CULL_BACK);
	manualCullingModeChoices.Add(wxT("Front"), MANUAL_CULL_FRONT);

	Append(wxEnumProperty(wxT("Manual Culling Mode"), wxPG_LABEL, manualCullingModeChoices, pass->getManualCullingMode()));
}

void PassPropertyGridPage::createIlluminationCategory()
{
	const Pass* pass = mController->getPass();

	Append(wxPropertyCategory(wxT("Illumination")));

	Append(wxBoolProperty(wxT("Lighting"), wxPG_LABEL, pass->getLightingEnabled()));	

	// Max Simultaneous Lights
	Append(wxIntProperty(wxT("Max Lights"), wxPG_LABEL, pass->getMaxSimultaneousLights()));

	// Start Light
	Append(wxIntProperty(wxT("Start Light"), wxPG_LABEL, pass->getStartLight()));

	// Light Iteration
	Append(wxBoolProperty(wxT("Iteration"), wxPG_LABEL, pass->getIteratePerLight()));

	// Shading Mode
	wxPGChoices shadingModeChoices;
	shadingModeChoices.Add(wxT("Flat"), SO_FLAT);
	shadingModeChoices.Add(wxT("Gouraud"), SO_GOURAUD);
	shadingModeChoices.Add(wxT("Phong"), SO_PHONG);

	Append(wxEnumProperty(wxT("Shading Mode"), wxPG_LABEL, shadingModeChoices, pass->getShadingMode()));

	// Self Illumination
	ColourValue selfIllum = pass->getSelfIllumination();
	Append(wxColourProperty(wxT("Self Illumination"), wxPG_LABEL, wxColour((int)(255 * selfIllum.r), (int)(255 * selfIllum.g), (int)(255 * selfIllum.b))));

}

void PassPropertyGridPage::createFogCategory()
{
	const Pass* pass = mController->getPass();

	Append(wxPropertyCategory(wxT("Fog")));

	// Fog Enabled
	Append(wxBoolProperty(wxT("Override Scene"), wxPG_LABEL, pass->getFogOverride()));	

	// Fog Mode
	wxPGChoices fogModeChoices;
	fogModeChoices.Add(wxT("None"), FOG_NONE);
	fogModeChoices.Add(wxT("EXP"), FOG_EXP);
	fogModeChoices.Add(wxT("EXP2"), FOG_EXP2);
	fogModeChoices.Add(wxT("Linear"), FOG_LINEAR);

	Append(wxEnumProperty(wxT("Fog Mode"), wxPG_LABEL, fogModeChoices, pass->getFogMode()));
}

// Possibly better as a wxParentProperty within Misc?
void PassPropertyGridPage::createAlphaRejectionCategory()
{
	const Pass* pass = mController->getPass();
	Append(wxPropertyCategory(wxT("Alpha Rejection"), wxPG_LABEL));
		
	// Alpha Reject Func
	wxPGChoices compareFuncChoices;
	compareFuncChoices.Add(wxT("Fail"), CMPF_ALWAYS_FAIL);
	compareFuncChoices.Add(wxT("Pass"), CMPF_ALWAYS_PASS);
	compareFuncChoices.Add(wxT("<"), CMPF_LESS);
	compareFuncChoices.Add(wxT("<="), CMPF_LESS_EQUAL);
	compareFuncChoices.Add(wxT("=="), CMPF_EQUAL);
	compareFuncChoices.Add(wxT("!="), CMPF_NOT_EQUAL);
	compareFuncChoices.Add(wxT(">="), CMPF_GREATER_EQUAL);
	compareFuncChoices.Add(wxT(">"), CMPF_GREATER);
	Append(wxEnumProperty(wxT("Function"), wxPG_LABEL, compareFuncChoices, pass->getAlphaRejectFunction()));
	
	// Alpha Reject Value
	Append(wxIntProperty(wxT("Value"), wxPG_LABEL, pass->getAlphaRejectValue()));


}

void PassPropertyGridPage::createMiscCategory()
{
	const Pass* pass = mController->getPass();

	Append(wxPropertyCategory(wxT("Misc")));

	// Colour Write
	Append(wxBoolProperty(wxT("Colour Write"), wxPG_LABEL, pass->getColourWriteEnabled()));	

	// Polygon Mode
	wxPGChoices polygonModeChoices;
	polygonModeChoices.Add(wxT("Points"), PM_POINTS);
	polygonModeChoices.Add(wxT("Wireframe"), PM_WIREFRAME);
	polygonModeChoices.Add(wxT("Solid"), PM_SOLID);

	Append(wxEnumProperty(wxT("Polygon Mode"), wxPG_LABEL, polygonModeChoices, pass->getPolygonMode()));

	// Track Vertex Colour Type
	wxPGChoices vertexColourTypeChoices;
	vertexColourTypeChoices.Add(wxT("None"), TVC_NONE);
	vertexColourTypeChoices.Add(wxT("Ambient"), TVC_AMBIENT);
	vertexColourTypeChoices.Add(wxT("Diffuse"), TVC_DIFFUSE);
	vertexColourTypeChoices.Add(wxT("Specular"), TVC_SPECULAR);
	vertexColourTypeChoices.Add(wxT("Emissive"), TVC_EMISSIVE);

	Append(wxEnumProperty(wxT("Track Vertex Colour Type"), wxPG_LABEL, vertexColourTypeChoices, pass->getVertexColourTracking()));

}

void PassPropertyGridPage::propertyChanged(wxPropertyGridEvent& event)
{

}
















