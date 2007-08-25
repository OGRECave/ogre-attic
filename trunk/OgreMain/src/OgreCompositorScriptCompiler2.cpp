/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#include "OgreStableHeaders.h"
#include "OgreCompositorScriptCompiler2.h"
#include "OgreCommon.h"


namespace Ogre
{

	CompositorScriptCompilerListener::CompositorScriptCompilerListener()
	{
	}

	bool CompositorScriptCompilerListener::processNode(ScriptNodeList::iterator &iter, ScriptNodeList::iterator &end, CompositorScriptCompiler2*)
	{
		return false;
	}

	CompositorScriptCompiler2::CompositorScriptCompiler2()
		:mListener(0), mCompositor(0)
	{
		mWordIDs["compositor"] = ID_COMPOSITOR;
		mWordIDs["technique"] = ID_TECHNIQUE;
		mWordIDs["target"] = ID_TARGET;
		mWordIDs["target_output"] = ID_TARGET_OUTPUT;
		mWordIDs["pass"] = ID_PASS;

		mWordIDs["texture"] = ID_TEXTURE;
		mWordIDs["input"] = ID_INPUT;
			mWordIDs["none"] = ID_NONE;
			mWordIDs["previous"] = ID_PREVIOUS;
		mWordIDs["only_initial"] = ID_ONLY_INITIAL;
		mWordIDs["visibility_mask"] = ID_VISIBILITY_MASK;
		mWordIDs["lod_bias"] = ID_LOD_BIAS;
		mWordIDs["material_scheme"] = ID_MATERIAL_SCHEME;

		mWordIDs["clear"] = ID_CLEAR;
		mWordIDs["stencil"] = ID_STENCIL;
		mWordIDs["render_scene"] = ID_RENDER_SCENE;
		mWordIDs["render_quad"] = ID_RENDER_QUAD;
		mWordIDs["material"] = ID_MATERIAL;
		mWordIDs["identifier"] = ID_IDENTIFIER;
		mWordIDs["first_render_queue"] = ID_FIRST_RENDER_QUEUE;
		mWordIDs["last_render_queue"] = ID_LAST_RENDER_QUEUE;

		mWordIDs["buffers"] = ID_BUFFERS;
			mWordIDs["colour"] = ID_COLOUR;
			mWordIDs["depth"] = ID_DEPTH;
		mWordIDs["colour_value"] = ID_COLOUR_VALUE;
		mWordIDs["depth_value"] = ID_DEPTH_VALUE;
		mWordIDs["stencil_value"] = ID_STENCIL_VALUE;

		mWordIDs["check"] = ID_CHECK;
		mWordIDs["comp_func"] = ID_COMP_FUNC;
			mWordIDs["always_fail"] = ID_ALWAYS_FAIL;
			mWordIDs["always_pass"] = ID_ALWAYS_PASS;
			mWordIDs["less_equal"] = ID_LESS_EQUAL;
			mWordIDs["less"] = ID_LESS;
			mWordIDs["equal"] = ID_EQUAL;
			mWordIDs["not_equal"] = ID_NOT_EQUAL;
			mWordIDs["greater_equal"] = ID_GREATER_EQUAL;
			mWordIDs["greater"] = ID_GREATER;
		mWordIDs["ref_value"] = ID_REF_VALUE;
		mWordIDs["mask"] = ID_MASK;
		mWordIDs["fail_op"] = ID_FAIL_OP;
			mWordIDs["keep"] = ID_KEEP;
			mWordIDs["zero"] = ID_ZERO;
			mWordIDs["replace"] = ID_REPLACE;
			mWordIDs["increment"] = ID_INCREMENT;
			mWordIDs["decrement"] = ID_DECREMENT;
			mWordIDs["increment_wrap"] = ID_INCREMENT_WRAP;
			mWordIDs["decrement_wrap"] = ID_DECREMENT_WRAP;
			mWordIDs["invert"] = ID_INVERT;
		mWordIDs["depth_fail_op"] = ID_DEPTH_FAIL_OP;
		mWordIDs["pass_op"] = ID_PASS_OP;
		mWordIDs["two_sided"] = ID_TWO_SIDED;
	}

	void CompositorScriptCompiler2::setListener(CompositorScriptCompilerListener *listener)
	{
		mListener = listener;
	}

	bool CompositorScriptCompiler2::compileImpl(ScriptNodeListPtr nodes)
	{
		ScriptNodeList::iterator i = nodes->begin();
		while(i != nodes->end())
		{
			if(!processNode(i, nodes->end()))
			{
				if((*i)->wordID == ID_COMPOSITOR)
				{

				}
				++i;
			}
		}
		return mErrors.empty();
	}

	bool CompositorScriptCompiler2::processNode(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		return mListener ? mListener->processNode(i, end, this) : false;
	}

	ScriptNodeListPtr CompositorScriptCompiler2::loadImportPath(const Ogre::String &name)
	{
		ScriptNodeListPtr nodes;

		// Try the listener
		if(mListener)
			nodes = mListener->importFile(name);

		// Try the base version
		if(nodes.isNull())
			nodes = ScriptCompiler::loadImportPath(name);

		// If we got any AST loaded, do the necessary pre-processing steps
		if(!nodes.isNull())
		{
			// Expand all imports
			processImports(nodes);
		}

		return nodes;
	}

	void CompositorScriptCompiler2::preParse()
	{
		if(mListener)
			mListener->preParse(mWordIDs);
	}

	bool CompositorScriptCompiler2::errorRaised(const ScriptCompilerErrorPtr &error)
	{
		return mListener ? mListener->errorRaised(error) : true;
	}

}

