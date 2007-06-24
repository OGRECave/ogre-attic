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

#include "OgreRenderSystemCapabilitiesSerializer.h"
#include "OgreRenderSystemCapabilitiesManager.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreRenderSystemCapabilities.h"

namespace Ogre
{

    //-----------------------------------------------------------------------
    // Internal parser methods
    //-----------------------------------------------------------------------
    void RenderSystemCapabilitiesSerializer::logParseError(const String& error)
    {
        // log material name only if filename not specified
        if (mCurrentLine != 0 && !mCurrentStream.isNull())
        {
            LogManager::getSingleton().logMessage(
                "Error in .rendercaps " + mCurrentStream->getName() + ":" + StringConverter::toString(mCurrentLineNumber) +
                " : " + error);
        }
        else if (!mCurrentStream.isNull())
        {
            LogManager::getSingleton().logMessage(
                "Error in .rendercaps " + mCurrentStream->getName() +
                " : " + error);
        }
    }

    RenderSystemCapabilitiesSerializer::RenderSystemCapabilitiesSerializer()
    {
        // initialiaze parsing data to NULL
        mCurrentLineNumber = 0;
        mCurrentLine = 0;
        mCurrentStream.setNull();
        mCurrentCapabilities = 0;

    }
    //-----------------------------------------------------------------------
    void RenderSystemCapabilitiesSerializer::parseScript(DataStreamPtr& stream)
    {
        // reset parsing data to NULL
        mCurrentLineNumber = 0;
        mCurrentLine = 0;
        mCurrentStream.setNull();
        mCurrentCapabilities = 0;

        mCurrentStream = stream;

        // parser operating data
        String line;
        ParseAction parseAction = PARSE_HEADER;
        StringVector tokens;
        bool parsedAtLeastOneRSC = false;

        // collect capabilities lines (i.e. everything that is not header, "{", "}",
        // comment or empty line) for further processing
        StringVector capabilitiesLines;


        // TODO: build a smarter tokenizer so that "{" and "}"
        // don't need separate lines
        while (!stream->eof())
        {
            line = stream->getLine();

            // keep track of parse position
            mCurrentLine = &line;
            mCurrentLineNumber++;

            tokens = StringUtil::split(line);

            // skip empty and comment lines
            // TODO: handle end of line comments
            if (tokens.size() == 0 || tokens[0].substr(0,2) == "//")
                continue;

            switch (parseAction)
            {
                // header line must look like this:
                // render_system_capabilities "Vendor Card Name Version xx.xxx"

                case PARSE_HEADER:

                    if(tokens[0] != "render_system_capabilities")
                    {
                        logParseError("The first keyword must be render_system_capabilities. RenderSystemCapabilities NOT created!");
                        return;
                    }
                    else
                    {
                        // the rest of the tokens are irrevelant, beause everything between "..." is one name
                        String rscName = line.substr(tokens[0].size());
                        StringUtil::trim(rscName);

                        // the second argument must be a "" delimited string
                        if (StringUtil::match(rscName, "\"*\""))
                        {
                            logParseError("The argument to render_system_capabilities must be a quote delimited (\"...\") string. RenderSystemCapabilities NOT created!");
                            return;
                        }
                        else
                        {
                            // we have a valid header

                            // remove quotes
                            rscName = rscName.substr(1);
                            rscName = rscName.substr(0, rscName.size() - 1);

                            // create RSC
                            mCurrentCapabilities = new RenderSystemCapabilities();
                            // RSCManager is responsible for deleting mCurrentCapabilities
                            RenderSystemCapabilitiesManager::getSingleton()._addRenderSystemCapabilities(rscName, mCurrentCapabilities);

                            LogManager::getSingleton().logMessage("Create RenderSystemCapabilities" + rscName);

                            // do next action
                            parseAction = FIND_OPEN_BRACE;
                            parsedAtLeastOneRSC = true;
                        }
                    }

                break;

                case FIND_OPEN_BRACE:
                    if (tokens[0] != "{" || tokens.size() != 1)
                    {
                        logParseError("Expected '{' got: " + line + ". Continuing to next line.");
                    }
                    else
                    {
                        parseAction = COLLECT_LINES;
                    }

                break;

                case COLLECT_LINES:
                    if (tokens[0] == "}")
                    {
                        // this render_system_capabilities section is over
                        // let's process the data and look for the next one
                        parseCapabilitiesLines(capabilitiesLines);
                        parseAction = PARSE_HEADER;

                    }
                    else
                        capabilitiesLines.push_back(line);


                break;

            }
        }

        // Datastream is empty
        // if we are still looking for header, this means that we have either
        // finished reading one, or this is an empty file
        if(parseAction == PARSE_HEADER && parsedAtLeastOneRSC == false)
        {
            logParseError ("The file is empty");
        }
        if(parseAction == FIND_OPEN_BRACE)
        {
            logParseError ("Bad .rendercaps file. Were not able to find a '{'");
        }
        if(parseAction == COLLECT_LINES)
        {
            logParseError ("Bad .rendercaps file. Were not able to find a '}'");
        }


    }

    void RenderSystemCapabilitiesSerializer::parseCapabilitiesLines(StringVector& lines)
    {
    }

}


