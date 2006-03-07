@node Compositor Scripts 
@section Compositor Scripts 

The compositor framework is a subsection of the OGRE API that allows you to easily define full screen post-processing effects. Compositor scripts offer you the ability to define compositor effects in a script which can be reused and modified easily, rather than having to use the API to define them. You still need to use code to instantiate a compositor against one of your visible viewports, but this is a much simpler process than actually defining the compositor itself. 

@heading Compositor Fundamentals 

Performing post-processing effects generally involves first rendering the scene to a texture, either in addition to or instead of the main window. Once the scene is in a texture, you can then pull the scene image into a fragment program and perform operations on it by rendering it through full screen quad. The target of this post processing render can be the main result (e.g. a window), or it can be another render texture so that you can perform multi-stage convolutions on the image. You can even 'ping-pong' the render back and forth between a couple of render textures to perform convolutions which require many iterations, without using a separate texture for each stage. Eventually you'll want to render the result to the final output, which you do with a full screen quad. This might replace the whole window (thus the main window doesn't need to render the scene itself), or it might be a combinational effect. @*@* 

So that we can discuss how to implement these techniques efficiently, a number of definitions are required:@*@* 
@table @asis 
@item Compositor 
Definition of a fullscreen effect that can be applied to a user viewport. 
@item Compositor Instance 
An instance of a compositor as applied to a single viewport. 
@item Compositor Chain 
It is possible to enable more than one compositor instance on a viewport at the same time, with one compositor taking the results of the previous one as input. This is known as a compositor chain. 
@item Target 
This is a RenderTarget, ie the place where the result of a series of render operations is sent. A target may be the final output, or it may be an intermediate render texture. A target which is not the output target has a defined size and pixel format which you can control. 
@item Output Target 
As Target, but this is the single final result of all operations. The size and pixel format of this target cannot be controlled by the compositor since it is defined by the application using it. 
@item Target Pass 
A Target may be rendered to many times in the course of a composition effect. In particular if you 'ping pong' a convolution between a couple of textures, you will have more than one Target Pass per Target. 
@item Pass 
Within a Target Pass, there are one or more individual Pass instances, which perform a very specific action, such as rendering the original scene (or pulling the result from the previous compositor in the chain), rendering a fullscreen quad, or clearing one or more buffers. Typically within a single target pass you will use the either a 'render scene' pass or a 'render quad' pass, not both. Clear can be used with either type. 
@end table 

@heading Loading scripts

Compositor scripts are loaded at initialisation time by the system: by default it looks in all common resource locations (see Root::addResourceLocation) for files with the '.compositor' extension and parses them. If you want to parse files with a different extension, use the CompositorManager::getSingleton().parseAllSources method with your own extension, or if you want to parse an individual file, use CompositorManager::getSingleton().parseScript.@*@*
