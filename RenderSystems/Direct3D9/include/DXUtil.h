#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
/*
	static bool OgreFormatRequiresEndianFlipping( PixelFormat format )
	{
		switch( format )
		{
		case PF_L4A4:
		case PF_B5G6R5:
		case PF_B4G4R4A4:
		case PF_B8R8G8:
		case PF_B8G8R8A8:
		case PF_B10G10R10A2:
			return true;

		case PF_L8:
		case PF_A8:
		case PF_R5G6B5:
		case PF_A4R4G4B4:
		case PF_R8G8B8:
		case PF_A8R8G8B8:
		case PF_A2R10G10B10:
		case PF_A4L4:
		case PF_UNKNOWN:
		default:
			return false;
		}
	}

	static D3DFORMAT OgreFormat_to_D3D9FORMAT( PixelFormat format )
	{
        switch( format )
        {
        case PF_L8:
            return D3DFMT_L8;
        case PF_A8:
            return D3DFMT_A8;
        case PF_R5G6B5:
        case PF_B5G6R5:
            return D3DFMT_R5G6B5;
        case PF_A4R4G4B4:
        case PF_B4G4R4A4:
            return D3DFMT_A4R4G4B4;
        case PF_R8G8B8:
        case PF_B8R8G8:
            return D3DFMT_R8G8B8;
        case PF_A8R8G8B8:
        case PF_B8G8R8A8:
            return D3DFMT_A8R8G8B8;
        case PF_UNKNOWN:
        case PF_A4L4:
        case PF_L4A4:
        case PF_A2R10G10B10:
        case PF_B10G10R10A2:
        default:
            return D3DFMT_UNKNOWN;
        }
	}
*/

		/*
        SDDataChunk encoded;
        if( !ArchiveManager::getSingleton()._findResourceData( 
			Texture::mName, 
            encoded ) )
        {
            Except(
            Exception::ERR_INVALIDPARAMS, 
            "Unable to find image file '" + Texture::mName + "'.",
            "D3D9Texture::loadImage" );
        }

		HRESULT hr;
		if (mUsage == TU_DEFAULT)
		{
			if( FAILED( hr = D3DXCreateTextureFromFileInMemoryEx(	mpD3DDevice, 
																	(LPCVOID)encoded.getPtr(), 
																	(UINT)encoded.getSize(), 
																	D3DX_DEFAULT,		//width
																	D3DX_DEFAULT,		//height
																	getNumMipMaps() ? getNumMipMaps() : 1,	//mip maps level
																	D3DUSAGE_DYNAMIC,						//usage
																	OgreFormat_to_D3D9FORMAT(mFormat),		//format
																	D3DPOOL_DEFAULT,	//memory pool
																	D3DX_DEFAULT,		//filter
																	D3DX_DEFAULT,		//mip map filter
																	0xFF000000,			//color key
																	NULL,
																	NULL,
																	&mpTexture ) ) )
			{
				Except( hr, "Failed to load texture", "D3D9Texture::loadImage" );
			}
		}
		else //render target
		{
			IDirect3DSurface9 * tempSurf;
			D3DSURFACE_DESC tempDesc;
			mpD3DDevice->GetRenderTarget(0,&tempSurf);
			tempSurf->GetDesc(&tempDesc);
			tempSurf->Release();
			mNumMipMaps = 0;

			if( FAILED( hr = D3DXCreateTextureFromFileInMemoryEx(	mpD3DDevice, 
																	(LPCVOID)encoded.getPtr(), 
																	(UINT)encoded.getSize(), 
																	D3DX_DEFAULT,			//width
																	D3DX_DEFAULT,			//height
																	0,						//mip maps level
																	D3DUSAGE_RENDERTARGET,	//usage
																	tempDesc.Format,		//format
																	D3DPOOL_DEFAULT,		//memory pool
																	D3DX_DEFAULT,			//filter
																	D3DX_DEFAULT,			//mip filter
																	0xFF000000,				//color key
																	NULL,
																	NULL,
																	&mpTexture ) ) )
			{
				Except( hr, "Failed to load texture, RENDER_TARGET", "D3D9Texture::loadImage" );
			}
		}
		
		D3DSURFACE_DESC desc;
		if( FAILED( hr = mpTexture->GetLevelDesc(0, &desc) ) )
			Except( hr, "Failed to get texture desc", "D3D9Texture::loadImage" );

		if( desc.Width != mSrcWidth || desc.Height != mSrcHeight )
		{
			char msg[255];
			sprintf(msg, "Surface dimensions for texture %s have been altered by "
				"the renderer.", Texture::mName.c_str());
			LogManager::getSingleton().logMessage(msg);

			sprintf(msg,"   Requested: %dx%d Actual: %dx%d",
				mSrcWidth, mSrcHeight, desc.Width, desc.Height);
			LogManager::getSingleton().logMessage(msg);

			LogManager::getSingleton().logMessage("   Likely cause is that requested dimensions are not a power of 2, "
				"or device requires square textures.");
		}

		mWidth = desc.Width;
		mHeight = desc.Height;
		getFormatDesc(desc.Format, mHasAlpha, &mFinalBpp);
		//Image::applyGamma( pTempData, mGamma, uint( img.getSize() ), mSrcBpp );
		*/
