#include "OgrePrerequisites.h"
#include "OgreCommon.h"

#include <IL/il.h>

namespace Ogre 
{
	PixelFormat ilFormat2OgreFormat( int ImageFormat, int BytesPerPixel )
	{
		switch( BytesPerPixel )
		{
		case 1:
			return PF_L8;

		case 2:
			switch( ImageFormat )
			{
			case IL_BGR:
				return PF_B5G6R5;
			case IL_RGB:
				return PF_R5G6B5;
			case IL_BGRA:
				return PF_B4G4R4A4;
			case IL_RGBA:
				return PF_A4R4G4B4;
			}

		case 3:
			switch( ImageFormat )
			{
			case IL_BGR:
				return PF_B8G8R8;
			case IL_RGB:
				return PF_R8G8B8;
			}

		case 4:
			switch( ImageFormat )
			{
			case IL_BGRA:
				return PF_B8G8R8A8;
			case IL_RGBA:
				return PF_A8R8G8B8;
			}

		default:
			return PF_UNKNOWN;
		}

	}

	std::pair< int, int > OgreFormat2ilFormat( PixelFormat format )
	{
		std::pair< int, int > ret_data;

		switch( format )
		{
		case PF_L8:
		case PF_A8:
			return std::pair< int, int >( IL_LUMINANCE, 1 );
		case PF_R5G6B5:
			return std::pair< int, int >( IL_RGB, 2 );
		case PF_B5G6R5:
			return std::pair< int, int >( IL_BGR, 2 );
		case PF_A4R4G4B4:
			return std::pair< int, int >( IL_RGBA, 2 );
		case PF_B4G4R4A4:
			return std::pair< int, int >( IL_BGRA, 2 );
		case PF_R8G8B8:
			return std::pair< int, int >( IL_RGB, 3 );
		case PF_B8G8R8:
			return std::pair< int, int >( IL_BGR, 3 );
		case PF_A8R8G8B8:
			return std::pair< int, int >( IL_RGBA, 4 );
		case PF_B8G8R8A8:
			return std::pair< int, int >( IL_BGRA, 4 );
		case PF_UNKNOWN:
		case PF_A4L4:
		case PF_L4A4:
		case PF_A2R10G10B10:
		case PF_B10G10R10A2:
		default:
			return std::pair< int, int >( -1, -1 );
		}
	}
}
