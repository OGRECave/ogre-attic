/************************************************************************
	filename: 	MaterialControls.h
	created:	05 Aug 2005
	author:		Jeff Doyle (nfz)
	
	modified:
        05 Aug 2005 - file created

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*************************************************************************/

#ifndef __MaterialControls_H__
#define __MaterialControls_H__

#include "CEGUI/CEGUIForwardRefs.h"
#include "OgreString.h"

enum ShaderValType
{
	GPU_VERTEX, GPU_FRAGMENT, MAT_SPECULAR, MAT_DIFFUSE, MAT_AMBIENT, MAT_SHININESS, MAT_EMISSIVE
};

//---------------------------------------------------------------------------
struct ShaderControl
{
    Ogre::String Name;
	Ogre::String ParamName;
	ShaderValType ValType;
	float MinVal;
	float MaxVal;
	size_t ElementIndex;
	mutable size_t ConstantIndex;

	float getRange(void) const { return MaxVal - MinVal; }
	float convertParamToScrollPosition(const float val) const { return val - MinVal; }
	float convertScrollPositionToParam(const float val) const { return val + MinVal; }
};

typedef std::vector<ShaderControl> ShaderControlsContainer;
typedef ShaderControlsContainer::iterator ShaderControlIterator;
// used for materials that have user controls

//---------------------------------------------------------------------------
class MaterialControls
{
public:
    MaterialControls(const Ogre::String& displayName, const Ogre::String& materialName)
        : mDisplayName(displayName)
        , mMaterialName(materialName)
    {
    };

    ~MaterialControls(void){}

    const Ogre::String& getDisplayName(void) const { return mDisplayName; }
    const Ogre::String& getMaterialName(void) const { return mMaterialName; }
    size_t getShaderControlCount(void) const { return mShaderControlsContainer.size(); }
    const ShaderControl& getShaderControl(const size_t idx) const
    {
        assert( idx < mShaderControlsContainer.size() );
        return mShaderControlsContainer[idx];
    }
    /** add a new control by passing a string parameter

    @param
      params is a string using the following format:
        "<Control Name>, <Shader parameter name>, <Parameter Type>, <Min Val>, <Max Val>, <Parameter Sub Index>"

        <Control Name> is the string displayed for the control name on screen
        <Shader parameter name> is the name of the variable in the shader
        <Parameter Type> can be GPU_VERTEX, GPU_FRAGMENT
        <Min Val> minimum value that parameter can be
        <Max Val> maximum value that parameter can be
        <Parameter Sub Index> index into the the float array of the parameter.  All GPU parameters are assumed to be float[4].

    */
    void addControl(const Ogre::String& params);

protected:

    Ogre::String mDisplayName;
    Ogre::String mMaterialName;

    ShaderControlsContainer mShaderControlsContainer;
};

typedef std::vector<MaterialControls> MaterialControlsContainer;
typedef MaterialControlsContainer::iterator MaterialControlsIterator;


//---------------------------------------------------------------------------
struct ShaderControlGUIWidget
{
	CEGUI::StaticText* TextWidget;
	CEGUI::StaticText* NumberWidget;
	CEGUI::Scrollbar*  ScrollWidget;

	ShaderControlGUIWidget() : TextWidget(NULL), ScrollWidget(NULL), NumberWidget(NULL) {}
};

//---------------------------------------------------------------------------
/** loads material shader controls from a configuration file
    A .controls file is made up of the following:

    [<material display name>]
    material = <material name>
    control = <Control Name>, <Shader parameter name>, <Parameter Type>, <Min Val>, <Max Val>, <Parameter Sub Index>

    <material display name> is what is displayed in the material combo box.
    <material name> is the name of the material in the material script.
    control is the shader control associated with the material. The order
    of the contol definions in the .controls file determins their order
    when displayed in the controls window.

    you can have multiple .controls files or put them all in one.
*/
void loadMaterialControlsFile(MaterialControlsContainer& controlsContainer, const Ogre::String& filename);
/** load all control files found in resource paths
*/
void loadAllMaterialControlFiles(MaterialControlsContainer& controlsContainer);

#endif // __MaterialControls_H__
