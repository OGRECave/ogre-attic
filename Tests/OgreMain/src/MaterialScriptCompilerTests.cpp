/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
-----------------------------------------------------------------------------
*/
#include "MaterialScriptCompilerTests.h"
#include "OgreMaterialScriptCompiler.h"

// Regsiter the suite
CPPUNIT_TEST_SUITE_REGISTRATION( MaterialScriptCompilerTests );

void MaterialScriptCompilerTests::executeTokenAction(const size_t tokenID)
{

}

void MaterialScriptCompilerTests::setUp()
{
}
void MaterialScriptCompilerTests::tearDown()
{
}

void MaterialScriptCompilerTests::testPositionToNextSymbol()
{
  struct test1result{
    const char character;
    const int line;
  };

  const String TestStr1 = "   \n\r  //c  \n\r// test\n\r  \t  c   - \n\r ,  e";
  const test1result test1results[] = {
    {'c', 4},
    {'-', 4},
    {',', 5},
    {'e', 5}
  };

  // first test: see if positionToNextSymbol can find a valid Symbol
  mSource = &TestStr1;
  mCharPos = 0;
  size_t resultID = 0;
  mCurrentLine = 1;
  mEndOfSource = mSource->length();
  while (positionToNextLexeme()) {
      CPPUNIT_ASSERT_MESSAGE( "test " + StringConverter::toString(resultID) + "  character found: " + (*mSource)[mCharPos] +
          "  Line:%d  : " + StringConverter::toString(mCurrentLine)
          , ((*mSource)[mCharPos] == test1results[resultID].character) && (mCurrentLine==test1results[resultID].line) );
    resultID++;
    mCharPos++;
  }

}

void MaterialScriptCompilerTests::testIsFloatValue(void)
{
  struct testfloatresult{
    const String teststr;
    const float fvalue;
    const size_t charsize;
  };

  const testfloatresult testfloatresults[] = {
    {"1 test", 1.0f, 1},
    {"2.3f test", 2.3f, 3},
    {"-0.5 test", -0.5f, 4},
    {" 23.6 test", 23.6f, 5},
    {"  -0.021 test", -0.021f, 8},
    {"12 test", 12.0f, 2},
    {"3test", 3.0f, 1}
  };

  float fvalue = 0;
  size_t charsize = 0;
  mCharPos = 0;
  size_t testsize = ARRAYSIZE(testfloatresults);
  for(size_t resultID=0; resultID<testsize; resultID++)
  {
    mSource = &testfloatresults[resultID].teststr;
    isFloatValue(fvalue, charsize);
    CPPUNIT_ASSERT_MESSAGE( "test " + StringConverter::toString(resultID) + " value returned: " + StringConverter::toString(fvalue)
        , fvalue == testfloatresults[resultID].fvalue);
    CPPUNIT_ASSERT_MESSAGE( "test " + StringConverter::toString(resultID) + " char size returned: " + StringConverter::toString(charsize)
        , charsize == testfloatresults[resultID].charsize);
  }

}

void MaterialScriptCompilerTests::testIsLexemeMatch(void)
{
  const String TestStr = "material test";
  const String TestSymbols = "material";

  mSource = &TestStr;
  mCharPos = 0;
  CPPUNIT_ASSERT(isLexemeMatch(TestSymbols));
  mCharPos = 1;
  CPPUNIT_ASSERT(!isLexemeMatch(TestSymbols));

}
