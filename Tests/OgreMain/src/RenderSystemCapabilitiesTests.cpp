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

#include "RenderSystemCapabilitiesTests.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreStringConverter.h"


// Regsiter the suite
CPPUNIT_TEST_SUITE_REGISTRATION( RenderSystemCapabilitiesTests );

void RenderSystemCapabilitiesTests::setUp()
{
}
void RenderSystemCapabilitiesTests::tearDown()
{
}

void RenderSystemCapabilitiesTests::testIsShaderProfileSupported(void)
{
	// create a new RSC
	Ogre::RenderSystemCapabilities rsc;

	// check that no shader profile is supported
	CPPUNIT_ASSERT(!rsc.isShaderProfileSupported("vs_1"));
	CPPUNIT_ASSERT(!rsc.isShaderProfileSupported("ps_1_1"));
	CPPUNIT_ASSERT(!rsc.isShaderProfileSupported("fp1"));

	rsc.addShaderProfile("vs_1");
	rsc.addShaderProfile("fp1");

	// check that the added shader profiles are supported
	CPPUNIT_ASSERT(rsc.isShaderProfileSupported("vs_1"));
	CPPUNIT_ASSERT(rsc.isShaderProfileSupported("fp1"));


	// check that non added profile is not supported
	CPPUNIT_ASSERT(!rsc.isShaderProfileSupported("ps_1_1"));


	// check that empty string is not supported
	CPPUNIT_ASSERT(!rsc.isShaderProfileSupported(""));
}

void RenderSystemCapabilitiesTests::testHasCapability()
{
	 using namespace Ogre;

	 RenderSystemCapabilities rsc;

	 // check that no caps (from 2 categories) are supported
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_AUTOMIPMAP));
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_BLENDING));
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_FRAGMENT_PROGRAM));
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_TWO_SIDED_STENCIL));
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_MIPMAP_LOD_BIAS));
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_TEXTURE_COMPRESSION));
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_TEXTURE_COMPRESSION_VTC));
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_GLEW1_5_NOVBO));
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_FBO_ATI));
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_PBUFFER));
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_GLEW1_5_NOHWOCCLUSION));

	 // add support for few caps from each category
	 rsc.setCapability(RSC_AUTOMIPMAP);
	 rsc.setCapability(RSC_FRAGMENT_PROGRAM);
	 rsc.setCapability(RSC_TEXTURE_COMPRESSION);
	 rsc.setCapability(RSC_FBO_ATI);
	 rsc.setCapability(RSC_GLEW1_5_NOHWOCCLUSION);

	 // check that the newly set caps are supported
	 CPPUNIT_ASSERT(rsc.hasCapability(RSC_AUTOMIPMAP));
	 CPPUNIT_ASSERT(rsc.hasCapability(RSC_FRAGMENT_PROGRAM));
	 CPPUNIT_ASSERT(rsc.hasCapability(RSC_TEXTURE_COMPRESSION));
	 CPPUNIT_ASSERT(rsc.hasCapability(RSC_FBO_ATI));
	 CPPUNIT_ASSERT(rsc.hasCapability(RSC_GLEW1_5_NOHWOCCLUSION));

	 // check that the non-set caps are NOT supported
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_BLENDING));
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_TWO_SIDED_STENCIL));
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_MIPMAP_LOD_BIAS));
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_TEXTURE_COMPRESSION_VTC));
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_GLEW1_5_NOVBO));
	 CPPUNIT_ASSERT(!rsc.hasCapability(RSC_PBUFFER));
	 
}
