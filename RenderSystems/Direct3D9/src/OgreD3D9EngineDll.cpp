#include "OgreD3D9RenderSystem.h"
#include "OgreRoot.h"

namespace Ogre 
{
	D3D9RenderSystem* d3dRendPlugin;

	extern "C" void dllStartPlugin(void) throw()
	{
		// Create the DirectX 8 rendering api
		HINSTANCE hInst = GetModuleHandle( "OgreD3D9Engine.dll" );
		d3dRendPlugin = new D3D9RenderSystem( hInst );
		// Register the render system
		Root::getSingleton().addRenderSystem( d3dRendPlugin );
	}

	extern "C" void dllStopPlugin(void)
	{
		delete d3dRendPlugin;
	}
}