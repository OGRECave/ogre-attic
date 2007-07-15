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
#include "OgreRenderSystemCapabilitiesManager.h"
#include "OgreStringConverter.h"


// Regsiter the suite
CPPUNIT_TEST_SUITE_REGISTRATION( RenderSystemCapabilitiesTests );

void RenderSystemCapabilitiesTests::setUp()
{
    using namespace Ogre;

    // we need to be able to create FileSystem archives to load .rendercaps
    mFileSystemArchiveFactory = new FileSystemArchiveFactory();

    mArchiveManager = new ArchiveManager();
    ArchiveManager::getSingleton().addArchiveFactory( mFileSystemArchiveFactory );

    mRenderSystemCapabilitiesManager = new RenderSystemCapabilitiesManager();
    // actual parsing happens here. test methods confirm parse results only
    mRenderSystemCapabilitiesManager->parseCapabilitiesFromArchive("../Media/CustomCapabilities", "FileSystem", true);
}

void RenderSystemCapabilitiesTests::tearDown()
{
    delete mRenderSystemCapabilitiesManager;
    delete mArchiveManager;
    delete mFileSystemArchiveFactory;

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

void RenderSystemCapabilitiesTests::testSerializeBlank()
{
    using namespace Ogre;

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps Blank");

    // if we have a non-NULL it's good enough
    CPPUNIT_ASSERT(rsc != 0);
}

void RenderSystemCapabilitiesTests::testSerializeEnumCapability()
{
    using namespace Ogre;

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps enum Capabilities");
    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rsc != 0);

    // confirm that the contents are rhe same as in .rendercaps file
    CPPUNIT_ASSERT(rsc->hasCapability(RSC_AUTOMIPMAP));
    CPPUNIT_ASSERT(rsc->hasCapability(RSC_FBO_ARB));
}


void RenderSystemCapabilitiesTests::testSerializeStringCapability()
{
    using namespace Ogre;

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps set String");
    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rsc != 0);

    CPPUNIT_ASSERT(rsc->getMaxVertexProgramVersion() == "vs99");
}

void RenderSystemCapabilitiesTests::testSerializeBoolCapability()
{
    using namespace Ogre;

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rscTrue = rscManager->loadParsedCapabilities("TestCaps set bool (true)");
    RenderSystemCapabilities* rscFalse = rscManager->loadParsedCapabilities("TestCaps set bool (false)");
    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rscTrue != 0);
    CPPUNIT_ASSERT(rscFalse != 0);

    CPPUNIT_ASSERT(rscTrue->getVertexTextureUnitsShared() == true);
    CPPUNIT_ASSERT(rscFalse->getVertexTextureUnitsShared() == false);
}

void RenderSystemCapabilitiesTests::testSerializeIntCapability()
{
    using namespace Ogre;

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps set int");
    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rsc != 0);

    // TODO: why no get?
    CPPUNIT_ASSERT(rsc->getNumMultiRenderTargets() == 99);
}

void RenderSystemCapabilitiesTests::testSerializeRealCapability()
{
    using namespace Ogre;

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps set Real");
    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rsc != 0);

    CPPUNIT_ASSERT(rsc->getMaxPointSize() == 99.5);
}

void RenderSystemCapabilitiesTests::testSerializeShaderCapability()
{
    using namespace Ogre;

    RenderSystemCapabilitiesManager* rscManager = RenderSystemCapabilitiesManager::getSingletonPtr();

    RenderSystemCapabilities* rsc = rscManager->loadParsedCapabilities("TestCaps addShaderProfile");
    // confirm that RSC was loaded
    CPPUNIT_ASSERT(rsc != 0);

    CPPUNIT_ASSERT(rsc->isShaderProfileSupported("vp1"));
    CPPUNIT_ASSERT(rsc->isShaderProfileSupported("vs_1_1"));
    CPPUNIT_ASSERT(rsc->isShaderProfileSupported("ps_99"));
}

