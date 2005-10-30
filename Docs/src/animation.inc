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
Skeletons and the animations which go with them are held in .skeleton files, which are produced by the OGRE exporters. These files are loaded automatically when you create an Entity based on a Mesh which is linked to the skeleton in question. The entity is then given an 'animation state' object per animation on the skeleton to allow you to specify the animation state of that single entity (you can animate multiple entities using the same skeleton, OGRE sorts the reuse out internally).@*@*

You can retrieve a pointer to the AnimationState object by calling Entity::getAnimationState. You can then call methods on this returned object to update the animation, probably in the frameStarted event. AnimationState has a very simple method 'addTime' which allows you to alter the animation position incrementally, and it will automatically loop for you. addTime can take positive or negative values (so you can reverse the animation if you want).@*@*

Skeletal animation can be performed in software, or implemented in shaders (hardware skinning). Clearly the latter is preferable, since it takes some of the work away from the CPU and gives it to the graphics card, and also means that the vertex data does not need to be re-uploaded every frame. This is especially important for large, detailed models. You should try to use hardware skinning wherever possible; this basically means assigning a material which has a vertex program powered technique. See @ref{Skeletal Animation in Vertex Programs} for more details. Skeletal animation can be combined with vertex animation, @xref{Combining Skeletal and Vertex Animation}.

@node Vertex Animation
@section Vertex Animation
Vertex animation is about using information about the movement of vertices directly to animate the mesh. Each track in a vertex animation targets a single VertexData instance.

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

For example, a common set-up for a complex character which needs both skeletal and facial animation might be to split the head into a separate SubMesh with its own geometry, then apply skeletal animation to both submeshes, and pose animation to just the head. 

@node Morph Animation
@subsection Morph Animation
TODO

@node Pose Animation
@subsection Pose Animation
TODO


@node Combining Skeletal and Vertex Animation
@subsection Combining Skeletal and Vertex Animation
TODO

@node SceneNode Animation
@section SceneNode Animation
TODO

@node Numeric Value Animation
@section Numeric Value Animation

TODO

