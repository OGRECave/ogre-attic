/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/
#ifndef _Material_H__
#define _Material_H__

#include "OgrePrerequisites.h"

#include "OgreColourValue.h"
#include "OgreException.h"
#include "OgreTextureManager.h"
#include "OgreBlendMode.h"
#include "OgreRoot.h"
#include "OgreSceneManagerEnumerator.h"
#include "OgreCommon.h"
#include "OgreMatrix4.h"


namespace Ogre {

    /** Class encapsulates rendering properties of an object.
        @remarks
            To optimise rendering, changes in render state must be
            minimised. One of the most frequent render state changes are
            changes to materials, mostly to textures. An object may
            use multiple materials for different parts of it's sub-mesh,
            although it is normally advisable to restrict objects to
            one material if possible.
        @par
            Ogre's material class encapsulates ALL aspects of the material,
            rather like 3D Studio's material. It also includes other flags which 
            might not be traditionally thought of as material properties such as 
            culling modes and depth buffer settings, but these affect the 
            appearance of the rendered object and are convenient to attach to the 
            material since it keeps all the settings in one place. This is 
            different to Direct3D which treats a material as just the colour 
            components (diffuse, specular) and not texture maps etc. An Ogre 
            Material can be thought of as equivalent to a 'Shader'.
        @par
            Ogre comes configured with a number of default settings for a newly 
            created material. These can be changed if you wish by retrieving the 
            default material settings through 
            SceneManager::getDefaultMaterialSettings. Any changes you make to the 
            Material returned from this method will apply to any materials created 
            from this point onward.
        @todo
            Add bump-mapping support.
    */
    class _OgreExport Material : public Resource
    {
        friend class SceneManager;
        friend class MaterialManager;
    public:

        /** Nested class representing a single texture layer of a material.
        */
        class _OgreExport TextureLayer
        {
            friend class RenderSystem;
        public:
            /** Definition of the broad types of texture effect you can apply to a texture layer.
            */
            enum TextureEffectType
            {
                /// Bump mapping
                ET_BUMP_MAP,
                /// Generate all texture coords based on angle between camera and vertex
                ET_ENVIRONMENT_MAP,
                /// Constant u/v scrolling effect
                ET_SCROLL,
                /// Constant rotation
                ET_ROTATE,
                /// More complex transform
                ET_TRANSFORM

            };

            /** Enumeration to specify type of envmap.
            */
            enum EnvMapType
            {
                /// Envmap based on vector from camera to vertex position, good for planar geometry
                ENV_PLANAR,
                /// Envmap based on dot of vector from camera to vertex and vertex normal, good for curves
                ENV_CURVED
            };

            /** Useful enumeration when dealing with procedural transforms.
            */
            enum TextureTransformType
            {
                TT_TRANSLATE_U,
                TT_TRANSLATE_V,
                TT_SCALE_U,
                TT_SCALE_V,
                TT_ROTATE
            };

            /** Texture addressing modes - default is TAM_WRAP.
            */
            enum TextureAddressingMode
            {
                /// Texture wraps at values over 1.0
                TAM_WRAP,
                /// Texture mirrors (flips) at joins over 1.0
                TAM_MIRROR,
                /// Texture clamps at 1.0
                TAM_CLAMP
            };

            /** Enum identifying the frame indexes for faces of a cube map (not the composite 3D type.
            */
            enum TextureCubeFace
            {
                CUBE_FRONT = 0,
                CUBE_BACK = 1,
                CUBE_LEFT = 2,
                CUBE_RIGHT = 3,
                CUBE_UP = 4,
                CUBE_DOWN = 5
            };

            /** Internal structure defining a texture effect.
            */
            struct TextureEffect {
                TextureEffectType type;
                int subtype;
                Real arg1, arg2;
                WaveformType waveType;
                Real base;
                Real frequency;
                Real phase;
                Real amplitude;
                Controller* controller;
            };


            /** Default constructor.
            */
            TextureLayer(bool deferLoad = false);

            TextureLayer( const TextureLayer& oth );

            TextureLayer & operator = ( const TextureLayer& oth );

            /** Default destructor.
            */
            ~TextureLayer();

            /** Name-based constructor.
                @note
                    Explicit to stop implicit conversion from char, causes ambiguity problems on addTextureLayer(char*).
                @param
                    name The basic name of the texture e.g. brickwall.jpg, stonefloor.png
                @param
                    texCoordSet The index of the texture coordinate set to use.
            */
            explicit TextureLayer( const String& texName, int texCoordSet = 0, bool deferLoad = false );

            /** Mostly internal method to indicate a deferred load texture layer.
            */
            void setDeferredLoad( bool defer );

            /** Get the name of current texture image for this layer.
                @remarks
                    This will either always be a single name for this layer,
                    or will be the name of the current frame for an animated
                    or otherwise multi-frame texture.
            */
            const String& getTextureName(void) const;

            /** Sets this texture layer to use a single texture, given the
                name of the texture to use on this layer.
            */
            void setTextureName( const String& name );

            /** Sets this texture layer to use a combination of 6 texture maps, each one relating to a face of a cube.
                @remarks
                    Cubic textures are made up of 6 separate texture images. Each one of these is an orthoganal view of the
                    world with a FOV of 90 degrees and an aspect ratio of 1:1. You can generate these from 3D Studio by
                    rendering a scene to a reflection map of a transparent cube and saving the output files.
                @par
                    Cubic maps can be used either for skyboxes (complete wrap-around skies, like space) or as environment
                    maps to simulate reflections. The system deals with these 2 scenarios in different ways:
                    <ol>
                        <li>
                            <p>
                            for cubic environment maps, the 6 textures are combined into a single 'cubic' texture map which
                            is then addressed using 3D texture coordinates. This is required because you don't know what
                            face of the box you're going to need to address when you render an object, and typically you
                            need to reflect more than one face on the one object, so all 6 textures are needed to be
                            'active' at once. Cubic environment maps are enabled by calling this method with the forUVW
                            parameter set to true, and then calling setEnvironmentMap(true).
                            </p>
                            <p>
                            Note that not all cards support cubic environment mapping.
                            </p>
                        </li>
                        <li>
                            <p>
                            for skyboxes, the 6 textures are kept separate and used independently for each face of the skybox.
                            This is done because not all cards support 3D cubic maps and skyboxes do not need to use 3D
                            texture coordinates so it is simpler to render each face of the box with 2D coordinates, changing
                            texture between faces.
                            </p>
                            <p>
                            Skyboxes are created by calling SceneManager::setSkyBox.
                            </p>
                        </li>
                    </ul>
                @param
                    name The basic name of the texture e.g. brickwall.jpg, stonefloor.png. There must be 6 versions
                    of this texture with the suffixes _fr, _bk, _up, _dn, _lf, and _rt (before the extension) which
                    make up the 6 sides of the box. The textures must all be the same size and be powers of 2 in width & height.
                    If you can't make your texture names conform to this, use the alternative method of the same name which takes
                    an array of texture names instead.
                @param
                    forUVW Set to true if you want a single 3D texture addressable with 3D texture coordinates rather than
                    6 separate textures. Useful for cubic environment mapping.
            */
            void setCubicTextureName( const String& name, bool forUVW = false );

            /** Sets this texture layer to use a combination of 6 texture maps, each one relating to a face of a cube.
                @remarks
                    Cubic textures are made up of 6 separate texture images. Each one of these is an orthoganal view of the
                    world with a FOV of 90 degrees and an aspect ratio of 1:1. You can generate these from 3D Studio by
                    rendering a scene to a reflection map of a transparent cube and saving the output files.
                @par
                    Cubic maps can be used either for skyboxes (complete wrap-around skies, like space) or as environment
                    maps to simulate reflections. The system deals with these 2 scenarios in different ways:
                    <ol>
                        <li>
                            <p>
                            for cubic environment maps, the 6 textures are combined into a single 'cubic' texture map which
                            is then addressed using 3D texture coordinates. This is required because you don't know what
                            face of the box you're going to need to address when you render an object, and typically you
                            need to reflect more than one face on the one object, so all 6 textures are needed to be
                            'active' at once. Cubic environment maps are enabled by calling this method with the forUVW
                            parameter set to true, and then calling setEnvironmentMap(true).
                            </p>
                            <p>
                            Note that not all cards support cubic environment mapping.
                            </p>
                        </li>
                        <li>
                            <p>
                            for skyboxes, the 6 textures are kept separate and used independently for each face of the skybox.
                            This is done because not all cards support 3D cubic maps and skyboxes do not need to use 3D
                            texture coordinates so it is simpler to render each face of the box with 2D coordinates, changing
                            texture between faces.
                            </p>
                            <p>
                            Skyboxes are created by calling SceneManager::setSkyBox.
                            </p>
                        </li>
                    </ul>
                @param
                    name The basic name of the texture e.g. brickwall.jpg, stonefloor.png. There must be 6 versions
                    of this texture with the suffixes _fr, _bk, _up, _dn, _lf, and _rt (before the extension) which
                    make up the 6 sides of the box. The textures must all be the same size and be powers of 2 in width & height.
                    If you can't make your texture names conform to this, use the alternative method of the same name which takes
                    an array of texture names instead.
                @param
                    forUVW Set to true if you want a single 3D texture addressable with 3D texture coordinates rather than
                    6 separate textures. Useful for cubic environment mapping.
            */
            void setCubicTextureName( const String* const names, bool forUVW = false );

            /** Sets the names of the texture images for an animated texture.
                @remarks
                    Animated textures are just a series of images making up the frames of the animation. All the images
                    must be the same size, and their names must have a frame number appended before the extension, e.g.
                    if you specify a name of "wall.jpg" with 3 frames, the image names must be "wall_1.jpg" and "wall_2.jpg".
                @par
                    You can change the active frame on a texture layer by calling the setCurrentFrame method.
                @note
                    If you can't make your texture images conform to the naming standard layed out here, you
                    can call the alternative setAnimatedTextureName method which takes an array of names instead.
                @param
                    name The base name of the textures to use e.g. wall.jpg for frames wall_1.jpg, wall_2.jpg etc.
                @param
                    numFrames The number of frames in the sequence.
                @param
                    duration The length of time it takes to display the whole animation sequence, in seconds.
                    If 0, no automatic transition occurs.
            */
            void setAnimatedTextureName( const String& name, int numFrames, Real duration = 0 );

            /** Sets the names of the texture images for an animated texture.
                @remarks
                    This an alternative method to the one where you specify a single name and let the system derive
                    the names of each frame, incase your images can't conform to this naming standard.
                @par
                    Animated textures are just a series of images making up the frames of the animation. All the images
                    must be the same size, and you must provide their names as an array in the first parameter.
                    You can change the active frame on a texture layer by calling the setCurrentFrame method.
                @note
                    If you can make your texture images conform to a naming standard of basicName_frame.ext, you
                    can call the alternative setAnimatedTextureName method which just takes a base name instead.
                @param
                    names Pointer to array of names of the textures to use, in frame order.
                @param
                    numFrames The number of frames in the sequence.
                @param
                    duration The length of time it takes to display the whole animation sequence, in seconds.
                    If 0, no automatic transition occurs.
            */
            void setAnimatedTextureName( const String* const names, int numFrames, Real duration = 0 );

            /** Changes the active frame in an animated or multi-image texture.
                @remarks
                    An animated texture (or a cubic texture where the images are not combined for 3D use) is made up of
                    a number of frames. This method sets the active frame.
            */
            void setCurrentFrame( int frameNumber );

            /** Gets the active frame in an animated or multi-image texture layer.
            */
            int getCurrentFrame(void) const;

            /** Gets the name of the texture associated with a frame.
            */
            const String& getFrameTextureName(int frameNumber) const;

            /** Gets the number of frames for a texture.
            */
            int getNumFrames(void) const;

            /** Returns true if this texture layer is a cubic texture, false otherwise.
            */
            bool isCubic(void) const;

            /** Returns true if this texture layer uses a composite 3D cubic texture.
            */
            bool is3D(void) const;

            /** Gets the index of the set of texture co-ords this layer uses.
            */
            int getTextureCoordSet(void) const;

            /** Sets the index of the set of texture co-ords this layer uses.
                @note
                    Default is 0 for all layers. Only change this if you have provided multiple texture co-ords per
                    vertex.
            */
            void setTextureCoordSet(int set);

            /** Sets a matrix used to transform any texture coordinates on this layer.
                @remarks
                    Texture coordinates can be modified on a texture layer to create effects like scrolling
                    textures. A texture transform can either be applied to a layer which takes the source coordinates
                    from a fixed set in the geometry, or to one which generates them dynamically (e.g. environment mapping).
                @par
                    It's obviously a bit impractical to create scrolling effects by calling this method manually since you
                    would have to call it every framw with a slight alteration each time, which is tedious. Instead
                    you can use the ControllerManager class to create a Controller object which will manage the
                    effect over time for you. See the ControllerManager::createTextureScroller and it's sibling methods for details.<BR>
                    In addition, if you want to set the individual texture transformations rather than concatenating them
                    yourself, use setTextureScroll, setTextureScale and setTextureRotate.
            */
            void setTextureTransform(const Matrix4& xform);

            /** Gets the current texture transformation matrix.
                @remarks
                    Causes a reclaculation of the matrix if any parameters have been changed via
                    setTextureScroll, setTextureScale and setTextureRotate.
            */
            const Matrix4& getTextureTransform(void);

            /** Sets the translation offset of the texture, ie scrolls the texture.
                @remarks
                    This method sets the translation element of the texture transformation, and is easier to use than setTextureTransform if
                    you are combining translation, scaling and rotation in your texture transformation. Again if you want
                    to animate these values you need to use a Controller
                @param u The amount the texture should be moved horizontally (u direction).
                @param v The amount the texture should be moved vertically (v direction).
                @see
                    ControllerManager, Controller
            */
            void setTextureScroll(Real u, Real v);

            /** As setTextureScroll, but sets only U value.
            */

            void setTextureUScroll(Real value);

            /** As setTextureScroll, but sets only V value.
            */
            void setTextureVScroll(Real value);

            /** As setTextureScale, but sets only U value.
            */
            void setTextureUScale(Real value);

            /** As setTextureScale, but sets only V value.
            */
            void setTextureVScale(Real value);

            /** Sets the scaling factor applied to texture coordinates.
                @remarks
                    This method sets the scale element of the texture transformation, and is easier to use than
                    setTextureTransform if you are combining translation, scaling and rotation in your texture transformation. Again if you want
                    to animate these values you need to use a Controller (see ControllerManager and it's methods for
                    more information).
                @param
                    uScale The value by which the texture is to be scaled horizontally.
                @param
                    vScale The value by which the texture is to be scaled vertically.
            */
            void setTextureScale(Real uScale, Real vScale);

            /** Sets the anticlockwise rotation factor applied to texture coordinates.
                @remarks
                    This sets a fixed rotation angle - if you wish to animate this, see the
                    ControllerManager::createTextureRotater method.
                @param
                    degrees The angle of rotation in degrees (anticlockwise).
            */
            void setTextureRotate(Real degrees);

            /** Gets the texture addressing mode, i.e. what happens at uv values above 1.0.
                @note
                    The default is TAM_WRAP i.e. the texture repeats over values of 1.0.
            */
            TextureAddressingMode getTextureAddressingMode(void) const;

            /** Sets the texture addressing mode, i.e. what happens at uv values above 1.0.
                @note
                    The default is TAM_WRAP i.e. the texture repeats over values of 1.0.
            */
            void setTextureAddressingMode( TextureAddressingMode tam);

            /** Setting advanced blending options.
                @remarks
                    This is an extended version of the TextureLayer::setColourOperation method which allows
                    extremely detailed control over the blending applied between this and earlier layers.
                    See the IMPORTANT note below about the issues between mulitpass and multitexturing that
                    using this method can create.
                @par
                    Texture colour operations determine how the final colour of the surface appears when
                    rendered. Texture units are used to combine colour values from various sources (ie. the
                    diffuse colour of the surface from lighting calculations, combined with the colour of
                    the texture). This method allows you to specify the 'operation' to be used, ie. the
                    calculation such as adds or multiplies, and which values to use as arguments, such as
                    a fixed value or a value from a previous calculation.
                @par
                    The defaults for each layer are:
                    <ul>
                        <li>op = LBX_MODULATE</li>
                        <li>source1 = LBS_TEXTURE</li>
                        <li>source2 = LBS_CURRENT</li>
                    </ul>
                    ie. each layer takes the colour results of the previous layer, and multiplies them
                    with the new texture being applied. Bear in mind that colours are RGB values from
                    0.0 - 1.0 so multiplying them together will result in values in the same range,
                    'tinted' by the multiply. Note however that a straight multiply normally has the
                    effect of darkening the textures - for this reason there are brightening operations
                    like LBO_MODULATE_X2. See the LayerBlendOperation and LayerBlendSource enumerated
                    types for full details.
                @note
                    Because of the limitations on some underlying APIs (Direct3D included)
                    the LBS_TEXTURE argument can only be used as the first argument, not the second.
                @par
                    The final 3 parameters are only required if you decide to pass values manually
                    into the operation, i.e. you want one or more of the inputs to the colour calculation
                    to come from a fixed value that you supply. Hence you only need to fill these in if
                    you supply LBS_MANUAL to the corresponding source, or use the LBX_BLEND_MANUAL
                    operation.
                @warning
                    Ogre tries to use multitexturing hardware to blend texture layers
                    together. However, if it runs out of texturing units (e.g. 2 of a GeForce2, 4 on a
                    GeForce3) it has to fall back on multipass rendering, i.e. rendering the same object
                    multiple times with different textures. This is both less efficient and there is a smaller
                    range of blending operations which can be performed. For this reason, if you use this method
                    you MUST also call TextureLayer::setColourOpMultipassFallback to specify which effect you
                    want to fall back on if sufficient hardware is not available.
                @param
                    If you wish to avoid having to do this, use the simpler TextureLayer::setColourOperation method
                    which allows less flexible blending options but sets up the multipass fallback automatically,
                    since it only allows operations which have direct multipass equivalents.
                @param
                    op The operation to be used, e.g. modulate (multiply), add, subtract
                @param
                    source1 The source of the first colour to the operation e.g. texture colour
                @param
                    source2 The source of the second colour to the operation e.g. current surface colour
                @param
                    arg1 Manually supplied colour value (only required if source1 = LBS_MANUAL)
                @param
                    arg2 Manually supplied colour value (only required if source2 = LBS_MANUAL)
                @param
                    manualBlend Manually supplied 'blend' value - only required for operations
                    which require manual blend e.g. LBX_BLEND_MANUAL
            */
            void setColourOperationEx(
                LayerBlendOperationEx op,
                LayerBlendSource source1 = LBS_TEXTURE,
                LayerBlendSource source2 = LBS_CURRENT,

                const ColourValue& arg1 = ColourValue::White,
                const ColourValue& arg2 = ColourValue::White,

                Real manualBlend = 0.0);

            /** Determines how this texture layer is combined with the one below it (or the diffuse colour of
                the geometry if this is layer 0).
                @remarks
                    This method is the simplest way to blend tetxure layers, because it requires only one parameter,
                    gives you the most common blending types, and automatically sets up 2 blending methods: one for
                    if single-pass multitexturing hardware is available, and another for if it is not and the blending must
                    be achieved through multiple rendering passes. It is, however, quite limited and does not expose
                    the more flexible multitexturing operations, simply because these can't be automatically supported in
                    multipass fallback mode. If want to use the fancier options, use TextureLayer::setColourOperationEx,
                    but you'll either have to be sure that enough multitexturing units will be available, or you should
                    explicitly set a fallback using TextureLayer::setColourOpMultipassFallback.
                @note
                    The default method is LBO_MODULATE for all layers.
                @param
                    op One of the LayerBlendOperation enumerated blending types.
            */
            void setColourOperation( const LayerBlendOperation op);

            /** Sets the multipass fallback operation for this layer, if you used TextureLayer::setColourOperationEx
                and not enough multitexturing hardware is available.
                @remarks
                    Because some effects exposed using TextureLayer::setColourOperationEx are only supported under
                    multitexturing hardware, if the hardware is lacking the system must fallback on multipass rendering,
                    which unfortunately doesn't support as many effects. This method is for you to specify the fallback
                    operation which most suits you.
                @par
                    You'll notice that the interface is the same as the Material::setSceneBlending method; this is
                    because multipass rendering IS effectively scene blending, since each layer is rendered on top
                    of the last using the same mechanism as making an object transparent, it's just being rendered
                    in the same place repeatedly to get the multitexture effect.
                @par
                    If you use the simpler (and hence less flexible) TextureLayer::setColourOperation method you
                    don't need to call this as the system sets up the fallback for you.
            */
            void setColourOpMultipassFallback( const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor);

            /** Get multitexturing colour blending mode.
            */
            LayerBlendModeEx getColourBlendMode(void) const;

            /** Get multitexturing alpha blending mode.
            */
            LayerBlendModeEx getAlphaBlendMode(void) const;

            /** Get the multipass fallback for colour blending operation source factor.
            */
            SceneBlendFactor getColourBlendFallbackSrc(void) const;

            /** Get the multipass fallback for colour blending operation destination factor.
            */
            SceneBlendFactor getColourBlendFallbackDest(void) const;

            /** Sets the alpha operation to be applied to this texture.
                @remarks
                    This works in exactly the same way as setColourOperation, except
                    that the effect is applied to the level of alpha (i.e. transparency)
                    of the texture rather than its colour. When the alpha of a texel (a pixel
                    on a texture) is 1.0, it is opaque, wheras it is fully transparent if the
                    alpha is 0.0. Please refer to the setColourOperation method for more info.
                @param
                    op The operation to be used, e.g. modulate (multiply), add, subtract
                @param
                    source1 The source of the first alpha value to the operation e.g. texture alpha
                @param
                    source2 The source of the second alpha value to the operation e.g. current surface alpha
                @param
                    arg1 Manually supplied alpha value (only required if source1 = LBS_MANUAL)
                @param
                    arg2 Manually supplied alpha value (only required if source2 = LBS_MANUAL)
                @param
                    manualBlend Manually supplied 'blend' value - only required for operations
                    which require manual blend e.g. LBX_BLEND_MANUAL
                @see
                    setColourOperation
            */
            void setAlphaOperation(LayerBlendOperationEx op,
                LayerBlendSource source1 = LBS_TEXTURE,
                LayerBlendSource source2 = LBS_CURRENT,
                Real arg1 = 1.0,
                Real arg2 = 1.0,
                Real manualBlend = 0.0);

            /** Generic method for setting up texture effects.
                @remarks
                    Allows you to specify effects directly by using the TextureEffectType enumeration. The
                    arguments that go with it depend on the effect type. Only one effect of
                    each type can be applied to a texture layer.
                @par
                    This method is used internally by Ogre but it is better generally for applications to use the
                    more intuitive specialised methods such as setEnvironmentMap and setScroll.
            */
            void addEffect(TextureEffect& effect);

            /** Turns on/off texture coordinate effect that makes this layer an environment map.
                @remarks
                    Environment maps make an object look reflective by using the object's vertex normals relative
                    to the camera view to generate texture coordinates.
                @par
                    The vectors generated can either be used to address a single 2D texture which
                    is a 'fish-eye' lens view of a scene, or a 3D cubic environment map which requires 6 textures
                    for each side of the inside of a cube. The type depends on what texture you set up - if you use the
                    setTextureName method then a 2D fisheye lens texture is required, whereas if you used setCubicTextureName
                    then a cubic environemnt map will be used.
                @par
                    This effect works best if the object has lots of gradually changing normals. The texture also
                    has to be designed for this effect - see the example spheremap.png included with the sample
                    application for a 2D environment map; a cubic map can be generated by rendering 6 views of a
                    scene to each of the cube faces with orthoganal views.
                @note
                    Enabling this disables any other texture coordinate generation effects.
                    However it can be combined with texture coordinate modification functions, which then operate on the
                    generated coordinates rather than static model texture coordinates.
                @param
                    enable True to enable, false to disable
                @param
                    planar If set to true, instead of being based on normals the environment effect is based on
                    vertex positions. This is good for planar surfaces.
            */
            void setEnvironmentMap(bool enable, bool planar = false);

            /** Sets up an animated scroll for the texture layer.
                @note
                    Useful for creating constant scrolling effects on a texture layer (for varying scrolls, see setTransformAnimation).
                @param
                    uSpeed The number of horizontal loops per second (+ve=moving right, -ve = moving left)
                @param
                    vSpeed The number of vertical loops per second (+ve=moving up, -ve= moving down)
            */
            void setScrollAnimation(Real uSpeed, Real vSpeed);

            /** Sets up an animated texture rotation for this layer.
                @note
                    Useful for constant rotations (for varying rotations, see setTransformAnimation).
                @param
                    speed The number of complete anticlockwise revolutions per second (use -ve for clockwise)
            */
            void setRotateAnimation(Real speed);

            /** Sets up a general time-relative texture modification effect.
                @note
                    This can be called multiple times for different values of ttype, but only the latest effect
                    applies if called multiple time for the same ttype.
                @param
                    ttype The type of transform, either translate (scroll), scale (stretch) or rotate (spin)
                @param
                    waveType The shape of the wave, see WaveformType enum for details
                @param
                    base The base value for the function (range of output = {base, base + amplitude})
                @param
                    frequency The speed of the wave in cycles per second
                @param
                    phase The offset of the start of the wave, e.g. 0.5 to start half-way through the wave
                @param
                    amplitude Scales the output so that instead of lying within 0..1 it lies within 0..1*amplitude for exaggerated effects
            */
            void setTransformAnimation( const TextureTransformType ttype,
                const WaveformType waveType, Real base = 0, Real frequency = 1, Real phase = 0, Real amplitude = 1 );

            /** Sets the way the layer will have use alpha to totally reject pixels from the pipeline.
                @note
                    The default is CMPF_ALWAYS_PASS i.e. alpha is not used to reject pixels.
                @param
                    func The comparison which must pass for the pixel to be written.
                @param
                    value 1 byte value against which alpha values will be tested. Recommended 0 or 128 for compatibility.
            */
            void setAlphaRejectSettings( const CompareFunction func, unsigned char value );

            /** Gets the alpha reject function. See setAlphaRejectSettings for more information.
            */
            CompareFunction getAlphaRejectFunction(void) const;

            /** Gets the alpha reject value. See setAlphaRejectSettings for more information.
            */
            unsigned char getAlphaRejectValue(void) const;

            /** Removes all effects applied to this texture layer.
            */
            void removeAllEffects(void);

            /** Removes a single effect applied to this texture layer.
                @note
                    Because you can only have 1 effect of each type (e.g. 1 texture coordinate generation) applied
                    to a layer, only the effect type is required.
            */
            void removeEffect( const TextureEffectType type );

            /** Determines if this texture layer is currently blank.
                @note
                    This can happen if a texture fails to load or some other non-fatal error. Worth checking after
                    setting texture name.
            */
            bool isBlank(void) const;

            /** Sets this texture layer to be blank.
            */
            void setBlank(void);

            /** Internal method for dealing with deferred texture loading.
            */
            void _load(void);

            protected:
                // State
                #define MAX_FRAMES 32

                /// Number of frames of animation, or frames making up cubic
                int mNumFrames;        
                /// The xurrent animation frame.
                int mCurrentFrame;
                // String mFrames[MAX_FRAMES] is at the end of the class                

                /// Duration of animation in seconds
                Real mAnimDuration;            
                Controller* mAnimController;
                bool mCubic;

                int textureCoordSetIndex;
                TextureAddressingMode mAddressMode;                

                LayerBlendModeEx colourBlendMode;
                SceneBlendFactor colourBlendFallbackSrc;
                SceneBlendFactor colourBlendFallbackDest;

                LayerBlendModeEx alphaBlendMode;
                bool mIsBlank;
                bool mDeferLoad;

                bool mRecalcTexMatrix;
                Real mUMod, mVMod;
                Real mUScale, mVScale;
                Real mRotate;
                Matrix4 mTexModMatrix;
                CompareFunction mAlphaRejectFunc;
                unsigned char mAlphaRejectVal;

                // Animation, will be set up as Controllers
                Real mUScrollAnim, mVScrollAnim;
                Real mRotateAnim;

    //-----------------------------------------------------------------------------
    // Complex members (those that can't be copied using memcpy) are at the end to 
    // allow for fast copying of the basic members.
    //
                String mFrames[MAX_FRAMES]; // Names of frames

                typedef std::multimap<TextureEffectType, TextureEffect> EffectMap;
                EffectMap mEffects;
    //-----------------------------------------------------------------------------



                /** Internal method for calculating texture matrix.
                */
                void recalcTextureMatrix(void);

                /** Internal method for creating animation controller.
                */
                void createAnimController(void);

                /** Internal method for creating texture effect controller.
                */
                void createEffectController(TextureEffect& effect);


        };

    //-----------------------------------------------------------------------------
    // Add only basic data types here (ie those that can be memcpy()'d)
    //
    protected:
        //-------------------------------------------------------------------------
        // Colour properties
        ColourValue mAmbient;
        ColourValue mDiffuse;
        ColourValue mSpecular;    
        ColourValue mEmissive;
        Real mShininess;
        //-------------------------------------------------------------------------

        /// Default material settings - set up by SceneManager
        static Material* mDefaultSettings;

        /** Numerical handle (constant).
            @remarks
                This is assigned automatically, and is useful where small system 
                indexes to materials are needed.
        */
        int mHandle;    

        //-------------------------------------------------------------------------
        // Blending factors
        SceneBlendFactor mSourceBlendFactor;    
        SceneBlendFactor mDestBlendFactor;
        //-------------------------------------------------------------------------
        
        //-------------------------------------------------------------------------    
        // Depth buffer settings
        bool mDepthCheck;
        bool mDepthWrite;
        CompareFunction mDepthFunc;
        //-------------------------------------------------------------------------    

        //-------------------------------------------------------------------------
        // Culling mode
        CullingMode mCullMode;
        ManualCullingMode mManualCullMode;
        //-------------------------------------------------------------------------

        /// Lighting enabled?
        bool mLightingEnabled;

        /// Shading options
        ShadeOptions mShadeOptions;

        /// Texture filtering
        TextureFilterOptions mTextureFiltering;

        //-------------------------------------------------------------------------    
        // Fog
        bool mFogOverride;
        FogMode mFogMode;
        ColourValue mFogColour;
        Real mFogStart;
        Real mFogEnd;
        Real mFogDensity;
        //-------------------------------------------------------------------------    


        /** If true, loading of textures and setting up controllers is deferred 
            until the 'load' method is called.
        */
        bool mDeferLoad;

        /// Number of texture layers
        int mNumTextureLayers;
    //-----------------------------------------------------------------------------

    //-----------------------------------------------------------------------------
    // Add only complex data types here (ie the ones that can't be memcpy()'d
    //
    protected:
        TextureLayer mTextureLayers[OGRE_MAX_TEXTURE_LAYERS];        
    //-----------------------------------------------------------------------------

    //-----------------------------------------------------------------------------
    // Add protected member functions here
    //
    protected:
        /** Internal method which sets the material up from the default settings.
        */
        void applyDefaults(void);

        /** Method for retrieving next handle (internal only) */
        void assignNextHandle(void);
    //-----------------------------------------------------------------------------

    public:

        /** Mandatory constructor - you must supply a name for the material.
            @note
                Normally you create materials by calling the relevant SceneManager since that is responsible for
                managing all scene state including materials.
            @param
                name A unique mateiral name
            @param
                deferLoad If true, defers loading of texture resources etc until Material::load is called
        */
        Material( const String& name, bool deferLoad = false );

        /** Default constructor - uses a generated material name.
        */
        Material();

        /** Assignment operator to allow easy copying between materials.
        */
        Material& operator=( const Material& rhs );

        /** Gets the material's name (note - not a texture name).
        */
        const String& getName(void) const;

        /** Returns the numerical handle for this material, if a more compact refence is required.
            @note
                This handle will only be assigned after adding the material to the SceneManager.
                Before this time it will be zero.
        */
        int getHandle(void) const;

        /** Sets the ambient colour reflectance properties of this material.
            @remarks
                The base colour of a material is determined by how much red, green and blue light is reflects
                (provided texture layer #0 has a blend mode other than LBO_REPLACE). This property determines how
                much ambient light (directionless global light) is reflected. The default is full white, meaning
                objects are completely globally illuminated. Reduce this if you want to see diffuse or specular light
                effects, or change the blend of colours to make the object have a base colour other than white.
            @note
                This setting has no effect if dynamic lighting is disabled (see Material::setLightingEnabled).
        */
        void setAmbient(Real red, Real green, Real blue);

        /** Sets the ambient colour reflectance properties of this material.
            @remarks
                The base colour of a material is determined by how much red, green and blue light is reflects
                (provided texture layer #0 has a blend mode other than LBO_REPLACE). This property determines how
                much ambient light (directionless global light) is reflected. The default is full white, meaning
                objects are completely globally illuminated. Reduce this if you want to see diffuse or specular light
                effects, or change the blend of colours to make the object have a base colour other than white.
            @note
                This setting has no effect if dynamic lighting is disabled (see Material::setLightingEnabled).
        */

        void setAmbient(const ColourValue& ambient);

        /** Sets the diffuse colour reflectance properties of this material.
            @remarks
                The base colour of a material is determined by how much red, green and blue light is reflects
                (provided texture layer #0 has a blend mode other than LBO_REPLACE). This property determines how
                much diffuse light (light from instances of the Light class in the scene) is reflected. The default
                is full white, meaning objects reflect the maximum white light they can from Light objects.
            @note
                This setting has no effect if dynamic lighting is disabled (see Material::setLightingEnabled).
        */
        void setDiffuse(Real red, Real green, Real blue);

        /** Sets the diffuse colour reflectance properties of this material.
            @remarks
                The base colour of a material is determined by how much red, green and blue light is reflects
                (provided texture layer #0 has a blend mode other than LBO_REPLACE). This property determines how
                much diffuse light (light from instances of the Light class in the scene) is reflected. The default
                is full white, meaning objects reflect the maximum white light they can from Light objects.
            @note
                This setting has no effect if dynamic lighting is disabled (see Material::setLightingEnabled).
        */
        void setDiffuse(const ColourValue& diffuse);

        /** Sets the specular colour reflectance properties of this material.
            @remarks
                The base colour of a material is determined by how much red, green and blue light is reflects
                (provided texture layer #0 has a blend mode other than LBO_REPLACE). This property determines how
                much specular light (highlights from instances of the Light class in the scene) is reflected.
                The default is to reflect no specular light.
            @note
                The size of the specular highlights is determined by the separate 'shininess' property.
            @par
                This setting has no effect if dynamic lighting is disabled (see Material::setLightingEnabled).
        */
        void setSpecular(Real red, Real green, Real blue);

        /** Sets the specular colour reflectance properties of this material.
            @remarks
                The base colour of a material is determined by how much red, green and blue light is reflects
                (provided texture layer #0 has a blend mode other than LBO_REPLACE). This property determines how
                much specular light (highlights from instances of the Light class in the scene) is reflected.
                The default is to reflect no specular light.
            @note
                The size of the specular highlights is determined by the separate 'shininess' property.
            @par
                This setting has no effect if dynamic lighting is disabled (see Material::setLightingEnabled).
        */
        void setSpecular(const ColourValue& specular);

        /** Sets the shininess of the material, affecting the size of specular highlights.
        */
        void setShininess(Real val);

        /** Sets the amount of self-illumination an object has.
            @remarks
                If an object is self-illuminating, it does not need external sources to light it, ambient or
                otherwise. It's like the object has it's own personal ambient light. This property is rarely useful since
                you can already specify per-material ambient light, but is here for completeness.
        */
        void setSelfIllumination(Real red, Real green, Real blue);

        /** Sets the amount of self-illumination an object has.
            @remarks
                If an object is self-illuminating, it does not need external sources to light it, ambient or
                otherwise. It's like the object has it's own personal ambient light. This property is rarely useful since
                you can already specify per-material ambient light, but is here for completeness.
        */
        void setSelfIllumination(const ColourValue& selfIllum);

        /** Gets the ambient colour reflectance of the material.
        */
        const ColourValue& getAmbient(void) const;

        /** Gets the diffuse colour reflectance of the material.
        */
        const ColourValue& getDiffuse(void) const;

        /** Gets the specular colour reflectance of the material.
        */
        const ColourValue& getSpecular(void) const;

        /** Gets the self illumination colour of the material.
        */
        const ColourValue& getSelfIllumination(void) const;

        /** Gets the 'shininess' property of the material (affects specular highlights).
        */
        Real getShininess(void) const;

        /** Inserts a new texture layer in the material using primitives.
            @remarks
                This layer is added on top of all previous layers. Previously this method allowed insertion
                at any point in the texture layer stack but this feature has been removed for efficiency.
                Note that the Material will automatically load the texture requested, with
                the default number of mipmaps and priority. If you wish to have more control,
                load the Texture manually yourself using the TextureManager::load method before
                you add the texture layer (textures already loaded will keep their settings).
            @param
                name The basic name of the texture e.g. brickwall.jpg, stonefloor.png
            @param
                texCoordSet The index of the texture coordinate set to use.
        */
        TextureLayer* addTextureLayer( const String& textureName, int texCoordSet = 0);

        /** Retrieves a pointer to a texture layer so it may be modified.
        */
        TextureLayer* getTextureLayer(int index) const;

        /** Removes the topmost texture layer from the material.
        */
        void removeTextureLayer();

        /** Removes all texture layers.
        */
        void removeAllTextureLayers(void);

        /** Returns the number of texture layers.
        */
        int getNumTextureLayers(void) const
        {
            return mNumTextureLayers;
        }

        /** Sets the kind of blending this material has with the existing contents of the scene.
            @remarks
                Wheras the texture blending operations seen in the TextureLayer class are concerned with
                blending between texture layers, this blending is about combining the output of the material
                as a whole with the existing contents of the rendering target. This blending therefore allows
                object transparency and other special effects.
            @par
                This method allows you to select one of a number of predefined blending types. If you require more
                control than this, use the alternative version of this method which allows you to specify source and
                destination blend factors.
            @param
                sbt One of the predefined SceneBlendType blending types
        */
        void setSceneBlending( const SceneBlendType sbt );

        /** Allows very fine control of blending this material with the existing contents of the scene.
            @remarks
                Wheras the texture blending operations seen in the TextureLayer class are concerned with
                blending between texture layers, this blending is about combining the output of the material
                as a whole with the existing contents of the rendering target. This blending therefore allows
                object transparency and other special effects.
            @par
                This version of the method allows complete control over the blending operation, by specifying the
                source and destination blending factors. The result of the blending operation is:
                <span align="center">
                    final = (texture * sourceFactor) + (pixel * destFactor)
                </span>
            @par
                Each of the factors is specified as one of a number of options, as specified in the SceneBlendFactor
                enumerated type.
            @param
                sourceFactor The source factor in the above calculation, i.e. multiplied by the texture colour components.
            @param
                destFactor The destination factor in the above calculation, i.e. multiplied by the pixel colour components.
        */
        void setSceneBlending( const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor);

        /** Retrieves the source blending factor for the material (as set using Materiall::setSceneBlending).
        */
        SceneBlendFactor getSourceBlendFactor() const;

        /** Retrieves the destination blending factor for the material (as set using Materiall::setSceneBlending).
        */
        SceneBlendFactor getDestBlendFactor() const;

        /** Determines if the material has any transparency with the rest of the scene (derived from scene blending op).
        */
        bool isTransparent(void) const;

        /** Sets whether or not this material renders with depth-buffer checking on or not.
            @remarks
                If depth-buffer checking is on, whenever a pixel is about to be written to the frame buffer
                the depth buffer is checked to see if the pixel is in front of all other pixels written at that
                point. If not, the pixel is not written.
            @par
                If depth checking is off, pixels are written no matter what has been rendered before.
                Also see setDepthFunction for more advanced depth check configuration.
            @see
                setDepthFunction
        */
        void setDepthCheckEnabled(bool enabled);

        /** Returns whether or not this material renders with depth-buffer checking on or not.
            @see
                setDepthCheckEnabled
        */
        bool getDepthCheckEnabled(void) const;

        /** Sets whether or not this material renders with depth-buffer writing on or not.
            @remarks
                If depth-buffer writing is on, whenever a pixel is written to the frame buffer
                the depth buffer is updated with the depth value of that new pixel, thus affecting future
                rendering operations if future pixels are behind this one.
            @par
                If depth writing is off, pixels are written without updating the depth buffer Depth writing should
                normally be on but can be turned off when rendering static backgrounds or when rendering a collection
                of transparent objects at the end of a scene so that they overlap each other correctly.
        */
        void setDepthWriteEnabled(bool enabled);

        /** Returns whether or not this material renders with depth-buffer writing on or not.
            @see
                setDepthWriteEnabled
        */
        bool getDepthWriteEnabled(void) const;

        /** Sets the function used to compare depth values when depth checking is on.
            @remarks
                If depth checking is enabled (see setDepthCheckEnabled) a comparison occurs between the depth
                value of the pixel to be written and the current contents of the buffer. This comparison is
                normally CMPF_LESS_EQUAL, i.e. the pixel is written if it is closer (or at the same distance)
                than the current contents. If you wish you can change this comparison using this method.
        */
        void setDepthFunction( CompareFunction func );
        /** Returns the function used to compare depth values when depth checking is on.
            @see
                setDepthFunction
        */
        CompareFunction getDepthFunction(void) const;

        /** Sets the culling mode for this material  based on the 'vertex winding'.
            @remarks
                A typical way for the rendering engine to cull triangles is based on the 'vertex winding' of
                triangles. Vertex winding refers to the direction in which the vertices are passed or indexed
                to in the rendering operation as viewed from the camera, and will wither be clockwise or
                anticlockwise (that's 'counterclockwise' for you Americans out there ;) The default is
                CULL_CLOCKWISE i.e. that only triangles whose vertices are passed/indexed in anticlockwise order
                are rendered - this is a common approach and is used in 3D studio models for example. You can
                alter this culling mode if you wish but it is not advised unless you know what you are doing.
            @par
                You may wish to use the CULL_NONE option for mesh data that you cull yourself where the vertex
                winding is uncertain.
        */
        void setCullingMode( CullingMode mode );

        /** Returns the culling mode for geometry rendered with this material. See setCullingMode for more information.
        */
        CullingMode getCullingMode(void) const;

        /** Sets the manual culling mode, performed by CPU rather than hardware.
            @pemarks
                In some situations you want to use manual culling of triangles rather than sending the
                triangles to the hardware and letting it cull them. This setting only takes effect on SceneManager's
                that use it (since it is best used on large groups of planar world geometry rather than on movable
                geometry since this would be expensive), but if used can cull geometry before it is sent to the
                hardware.
            @note
                The default for this setting is MANUAL_CULL_BACK.
            @param
                mode The mode to use - see enum ManualCullingMode for details

        */
        void setManualCullingMode( ManualCullingMode mode );

        /** Retrieves the manual culling mode for this material
            @see
                setManualCullingMode
        */
        ManualCullingMode getManualCullingMode(void) const;

        /** Sets whether or not dynamic lighting is enabled.
            @param
                enabled
                If true, dynamic lighting is performed on geometry with normals supplied, geometry without
                normals will not be displayed.
            @par
                If false, no lighting is applied and all geometry will be full brightness.
        */
        void setLightingEnabled(bool enabled);

        /** Returns whether or not dynamic lighting is enabled.
        */
        bool getLightingEnabled(void) const;

        /** Sets the type of light shading required
            @note
                The default shading method is Gouraud shading.
        */
        void setShadingMode( ShadeOptions mode );

        /** Returns the type of light shading to be used.
        */
        ShadeOptions getShadingMode(void) const;

        /** Sets the type of texture filtering used when rendering.
            @remarks
                This method sets the kind of texture filtering applied when rendering textures onto
                primitives. Filtering covers how the effects of minification and magnification are
                disguised by resampling.
            */
        void setTextureFiltering( TextureFilterOptions mode );

        /** Returns the type of texture filtering used when rendering.
        */
        TextureFilterOptions getTextureFiltering(void) const;

        /** Sets the fogging mode applied to this material.
            @remarks
                Fogging is an effect that is applied as polys are rendered. Sometimes, you want
                fog to be applied to an entire scene. Other times, you want it to be applied to a few
                polygons only. This material-level specification of fog parameters lets you easily manage
                both.
            @par
                The SceneManager class also has a setFog method which applies scene-level fog. This method
                lets you change the fog behaviour for this material compared to the standard scene-level fog.
            @param
                overrideScene If true, you authorise this material to override the scene's fog params with it's own settings.
                If you specify false, so other parameters are necessary, and this is the default behaviour for materials.
            @param
                mode Only applicable if overrideScene is true. You can disable fog which is turned on for the
                rest of the scene by specifying FOG_NONE. Otherwise, set a material-specific fog mode as
                defined in the enum FogMode.
            @param
                colour The colour of the fog. Either set this to the same as your viewport background colour,
                or to blend in with a skydome or skybox.
            @param
                expDensity The density of the fog in FOG_EXP or FOG_EXP2 mode, as a value between 0 and 1. The default is 1. i.e. completely opaque, lower values can mean
                that fog never completely obscures the scene.
            @param
                linearStart Distance at which linear fog starts to encroach. The distance must be passed
                as a parametric value between 0 and 1, with 0 being the near clipping plane, and 1 being the far clipping plane. Only applicable if mode is FOG_LINEAR.
            @param
                linearEnd Distance at which linear fog becomes completely opaque.The distance must be passed
                as a parametric value between 0 and 1, with 0 being the near clipping plane, and 1 being the far clipping plane. Only applicable if mode is FOG_LINEAR.
        */
        void setFog(
            bool overrideScene,
            FogMode mode = FOG_NONE,
            const ColourValue& colour = ColourValue::White,
            Real expDensity = 1.0, Real linearStart = 0.0, Real linearEnd = 1.0 );

        /** Returns true if this material is to override the scene fog settings.
        */
        bool getFogOverride(void) const;

        /** Returns the fog mode for this material.
            @note
                Only valid if getFogOverride is true.
        */
        FogMode getFogMode(void) const;

        /** Returns the fog colour for the scene.
        */
        const ColourValue& getFogColour(void) const;

        /** Returns the fog start distance for this material.
            @note
                Only valid if getFogOverride is true.
        */
        Real getFogStart(void) const;

        /** Returns the fog end distance for this material.
            @note
                Only valid if getFogOverride is true.
        */
        Real getFogEnd(void) const;

        /** Returns the fog density for this material.
            @note
                Only valid if getFogOverride is true.
        */
        Real getFogDensity(void) const;

        /** Overridden from Resource.
            @remarks
                By default, Material's load their resources on demand, ie. when you add a texture layer with
                a texture name, that texture is loaded straight away. However, the Material also supports deferred
                loading for scripts ie. a Material can have a full definition, but has not loaded any textures yet
                or set up any Controller objects for animation etc. In this case the 'load' method does all these
                things.
        */
        void load(void);

        /** Overridden from Resource.
            @see
                Resource
        */
        void unload(void);

        /** Utility method for comparing material surface params.
            @returns
                True if identical, false otherwise.
        */
        bool _compareSurfaceParams( const Material& cmp ) const;
    };

} //namespace 

#endif
