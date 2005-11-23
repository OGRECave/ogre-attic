@node Animation
@chapter Animation

OGRE supports a prety flexible animation system that allows you to script animation for several different purposes:

@table @asis
@item @ref{Skeletal Animation}
Mesh animation using a skeletal structure to determine how the mesh deforms. @*
@item @ref{Vertex Animation}
Mesh animation using snapshots of vertex data to determine how the shape of the mesh changes.@*
@item @ref{SceneNode Animation}
Animating SceneNodes automatically to create effects like camera sweeps, objects following predefined paths, etc.@*
@item @ref{Numeric Value Animation}
Using OGRE's extensible class structure to animate any value.
@end table

@node Skeletal Animation
@section Skeletal Animation

Skeletal animation is a process of animating a mesh by moving a set of hierarchical bones within the mesh, which in turn moves the vertices of the model according to the bone assignments stored in each vertex. An alternative term for this approach is 'skinning'. The usual way of creating these animations is with a modelling tool such as Softimage XSI, Milkshape 3D, Blender, 3D Studio or Maya among others. OGRE provides exporters to allow you to get the data out of these modellers and into the engine @xref{Exporters}.@*@*

There are many grades of skeletal animation, and not all engines (or modellers for that matter) support all of them. OGRE supports the following features:
@itemize @bullet
@item Each mesh can be linked to a single skeleton
@item Unlimited bones per skeleton
@item Hierarchical forward-kinematics on bones
@item Multiple named animations per skeleton (e.g. 'Walk', 'Run', 'Jump', 'Shoot' etc)
@item Unlimited keyframes per animation
@item Linear or spline-based interpolation between keyframes
@item A vertex can be assigned to multiple bones and assigned weightings for smoother skinning
@item Multiple animations can be applied to a mesh at the same time, again with a blend weighting
@end itemize
@*
Skeletons and the animations which go with them are held in .skeleton files, which are produced by the OGRE exporters. These files are loaded automatically when you create an Entity based on a Mesh which is linked to the skeleton in question. You then use @ref{Animation State} to set the use of animation on the entity in question.

Skeletal animation can be performed in software, or implemented in shaders (hardware skinning). Clearly the latter is preferable, since it takes some of the work away from the CPU and gives it to the graphics card, and also means that the vertex data does not need to be re-uploaded every frame. This is especially important for large, detailed models. You should try to use hardware skinning wherever possible; this basically means assigning a material which has a vertex program powered technique. See @ref{Skeletal Animation in Vertex Programs} for more details. Skeletal animation can be combined with vertex animation, @xref{Combining Skeletal and Vertex Animation}.

@node Animation State
@section Animation State

When an entity containing animation of any type is created, it is given an 'animation state' object per animation to allow you to specify the animation state of that single entity (you can animate multiple entities using the same animation definitions, OGRE sorts the reuse out internally).@*@*

You can retrieve a pointer to the AnimationState object by calling Entity::getAnimationState. You can then call methods on this returned object to update the animation, probably in the frameStarted event. Each AnimationState needs to be enabled using the setEnabled method before the animation it refers to will take effect, and you can set both the weight and the time position (where appropriate) to affect the application of the animation using correlating methods. AnimationState also has a very simple method 'addTime' which allows you to alter the animation position incrementally, and it will automatically loop for you. addTime can take positive or negative values (so you can reverse the animation if you want).@*@*

@node Vertex Animation
@section Vertex Animation
Vertex animation is about using information about the movement of vertices directly to animate the mesh. Each track in a vertex animation targets a single VertexData instance. Vertex animation is stored inside the .mesh file since it is tightly linked to the vertex structure of the mesh.

There are actually two subtypes of vertex animation, for reasons which will be discussed in a moment.

@table @asis
@item @ref{Morph Animation}
Morph animation is about interpolating many mesh snapshots along a keyframe timeline. Morph animation has a direct correlation to old-skool character animation techniques used before skeletal animation was widely used.@*
@item @ref{Pose Animation}
Pose animation is about blending multiple discrete poses, expressed as offsets to the base vertex data, with different weights to provide a final result. Pose animation's most obvious use is facial animation.
@end table

@heading Why two subtypes?
So, why two subtypes of vertex animation? Couldn't both be implemented using the same system? The short answer is yes, but for very good reasons we decided to specialise them in order to optimise the implementation of each discrete type. If you don't care about the reasons why these are implemented differently, you can skip to the next part.@*@*

With morph animation, we have a whole series of poses which must be interpolated, e.g. a running animation implemented as morph targets. We choose not to support blending between multiple morph animations - this is beccause we like to support all features in hardware if possible, and blending of multiple morph animations requires (2*animations + 1) position vertex buffers. This is because to support blending all the positions have to be stored as offsets rather than snapshots, so to interpolate you need the previous keyframe, the next keyframe and the original vertex data. This clearly becomes infeasible very quickly when trying to implement this in a vertex shader - and really if you're wanting to do blended animation with multiple sets of tracks you should be using skeletal animation. By only supporting one active morph animation at once, the buffer requirements reduce to just 2 - snapshots of 2 keyframes of absolute position data. @*@*

Pose animation is different - it is not a sequence of keyframes, but a single target pose per track. For simplicity this is implemented as a single keyframe, but the data inside it is stored as an offset to the  base vertex data rather than as absolute data. This is because the primary reason for pose animation is to be able to blend multiple weighted poses - for example multiple expressions in facial animation. Whilst each track doesn't need interpolation within itself, it will be blended with other tracks for the same submesh. Since there is only one keyframe, the vertex buffer requirements for hardware interpolation are only (animations + 1), which is more manageable.  @*@*

So, by partitioning the vertex animation approaches into 2, we keep the techniques viable for hardware acceleration whilst still allowing all the useful techniques to be available. Note that morph animation cannot be blended with other types of vertex animation on the same vertex data (pose animation or other morph animation); pose animation can be blended with other pose animation though, and both types can be combined with skeletal animation. This combination limitation applies per set of vertex data though, not globally across the mesh (see below).

@heading Subtype applies per track
It's important to note that the subtype in question is held at a track level, not at the animation or mesh level. Since tracks map onto VertexData instances, this means that if your mesh is split into SubMeshes, each with their own dedicated geometry, you can have one SubMesh animated using pose animation, and others animated with morph animation (or not vertex animated at all). @*@*

For example, a common set-up for a complex character which needs both skeletal and facial animation might be to split the head into a separate SubMesh with its own geometry, then apply skeletal animation to both submeshes, and pose animation to just the head. @*@*

To see how to apply vertex animation, @xref{Animation State}.

@node Morph Animation
@subsection Morph Animation
Morph animation works by storing snapshots of the absolute vertex positions in each keyframe, and interpolating between them. Morph animation is mainly useful for animating objects which could not be adequately handled using skeletal animation; this is mostly objects that have to radically change structure and shape as part of the animation such that a skeletal structure isn't appropriate. @*@*

Because absolute positions are used, it is not possible to blend more than one morph animation on the same vertex data; you should use skeletal animation if you want to include animation blending since it is much more efficient. If you activate more than one animation which includes morph tracks for the same vertex data, only the last one will actually take effect. This also means that the 'weight' option on the animation state is not used for morph animation. @*@*

Morph animation can be combined with skeletal animation if required @xref{Combining Skeletal and Vertex Animation}. Morph animation can also be implemented in hardware using vertex shaders, @xref{Morph Animation in Vertex Programs}.

@node Pose Animation
@subsection Pose Animation
Pose animation allows you to blend together potentially multiple vertex poses at different weights into final vertex state. A common use for this is facial animation, where each facial expression is placed in a separate animation, and weights used to either blend from one expression to another, or to combine full expressions if each pose only affects part of the face.@*@*

In order to do this, pose animation uses offsets to the original vertex data, and does not require that every vertex has an offset - those that don't are left alone. When blending in software these vertices are completely skipped - when blending in hardware (which requires a vertex entry for every vertex), zero offsets for vertices which are not mentioned are automatically created for you.@*@*

Pose animation tracks only have a single keyframe, ie the final pose. For this reason the 'time' property of the animation state has no effect, only the weight changes the effect. Unlike morph animation you can have multiple animations with tracks affecting the same vertex data active at once, since this is how blending is performed.@*@*

You can combine pose animation with skeletal animation, @xref{Combining Skeletal and Vertex Animation}, and you can also hardware accelerate the application of the blend with a vertex shader, @xref{Pose Animation in Vertex Programs}.

@node Combining Skeletal and Vertex Animation
@subsection Combining Skeletal and Vertex Animation
Skeletal animation and vertex animation (of either subtype) can both be enabled on the same entity at the same time (@xref{Animation State}). The effect of this is that vertex animation is applied first to the base mesh, then skeletal animation is applied to the result. This allows you, for example, to facially animate a character using pose vertex animation, whilst performing the main movement animation using skeletal animation.@*@*

Combining the two is, from a user perspective, as simple as just enabling both animations at the same time. When it comes to using this feature efficiently though, there are a few points to bear in mind:

@itemize bullet
@item @ref{Combined Hardware Skinning}
@item @ref{Submesh Splits}
@end itemize

@anchor{Combined Hardware Skinning}
@heading Combined Hardware Skinning
For complex characters it is a very good idea to implement hardware skinning by including a technique in your materials which has a vertex program which can perform the kinds of animation you are using in hardware. See @ref{Skeletal Animation in Vertex Programs}, @ref{Morph Animation in Vertex Programs}, @ref{Pose Animation in Vertex Programs}. @*@*

When combining animation types, your vertex programs must support both types of animation that the combined mesh needs, otherwise hardware skinning will be disabled. You should implement the animation in the same way that OGRE does, ie perform vertex animation first, then apply skeletal animation to the result of that. Remember that the implementation of morph animation passes 2 absolute snapshot buffers of the from & to keyframes, along with a single parametric, which you have to linearly interpolate, whilst pose animation passes the base vertex data plus 'n' pose offset buffers, and 'n' parametric weight values. @*@*

@anchor{Submesh Splits}
@heading Submesh Splits

If you only need to combine vertex and skeletal animation for a small part of your mesh, e.g. the face, you could split your mesh into 2 parts, one which needs the combination and one which does not, to reduce the calculation overhead. Note that it will also reduce vertex buffer usage since vertex keyframe / pose buffers will also be smaller. Note that if you use hardware skinning you should then implement 2 separate vertex programs, one which does only skeletal animation, and the other which does skeletal and vertex animation.

@node SceneNode Animation
@section SceneNode Animation

SceneNode animation is created from the SceneManager in order to animate the movement of SceneNodes, to make any attached objects move around automatically. You can see this performing a camera swoop in Demo_CameraTrack, or controlling how the fish move around in the pond in Demo_Fresnel.@*@*

At it's heart, scene node animation is mostly the same code which animates the underlying skeleton in skeletal animation. After creating the main Animation using SceneManager::createAnimation you can create a NodeAnimationTrack per SceneNode that you want to animate, and create keyframes which control its position, orientation and scale which can be interpolated linearly or via splines. You use @ref{Animation State} in the same way as you do for skeletal/vertex animation, except you obtain the state from SceneManager instead of from an individual Entity.Animations are applied automatically every frame, or the state can be applied manually in advance using the _applySceneAnimations() method on SceneManager. See the API reference for full details of the interface for configuring scene animations.@*@*

@node Numeric Value Animation
@section Numeric Value Animation
Apart from the specific animation types which may well comprise the most common uses of the animation framework, you can also use animations to alter any value which is exposed via the @ref{AnimableObject} interface. @*@*

@anchor{AnimableObject}
@heading AnimableObject 
AnimableObject is an abstract interface that any class can extend in order to provide access to a number of @ref{AnimableValue}s. It holds a 'dictionary' of the available animable properties which can be enumerated via the getAnimableValueNames method, and when its createAnimableValue method is called, it returns a reference to a value object which forms a bridge between the generic animation interfaces, and the underlying specific object property.@*@*

One example of this is the Light class. It extends AnimableObject and provides AnimableValues for properties such as "diffuseColour" and "attenuation". Animation tracks can be created for these values and thus properties of the light can be scripted to change. Other objects, including your custom objects, can extend this interface in the same way to provide animation support to their properties.

@anchor{AnimableValue}
@heading AnimableValue 

When implementing custom animable properties, you have to also implement a number of methods on the AnimableValue interface - basically anything which has been marked as unimplemented. These are not pure virtual methods simply because you only have to implement the methods required for the type of value you're animating. Again, see the examples in Light to see how this is done.
