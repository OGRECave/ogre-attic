#ifndef __RenderTargetTexture_H__
#define __RenderTargetTexture_H__

#include "OgrePrerequisites.h"
#include "OgreTexture.h"
#include "OgreRenderTarget.h"

namespace Ogre
{
    class _OgrePrivate RenderTargetTexture : public Texture, public RenderTarget
    {
    public:
        /// @copydoc Texture::isRenderTarget
        virtual bool isRenderTarget() const { return false; }
    };
}

#endif
