@node Copying Materials
@subsection Copying Materials

When creating new materials scripts that are only slight variations of another material script, the script writer resorts to copy and pasting materials within scripts.  For simple materials, the copy/paste method is acceptable.  Complex materials that use multiple passes along with vertex and pixel shaders become very tedious to edit when many variations are needed.  To reduce the tediousness of making new materials that are just slight variations of previously defined materials, new materials can copy from previously defined materials and modify specific techniques, passes, texture units or add new ones.@*@*

To make a new material that is based on one previously defined, add a @emph{colon} @strong{:} after the new material name followed by the name of the material that is to be copied.@*@*

Format: material <NewUniqueChildName> : <ReferanceParentMaterial>@*@*

The only caveat is that a parent material must have been defined/parsed prior to the child material script being parsed.  The easiest way to achieve this is to place parents at the begining of the material script file.  After material scripts are loaded into Ogre, materials no longer maintain their copy inheritance structure.  If a parent material is modified through code at runtime, the changes have no effect on child materials that were copied from it in the script.@*@*

Material copying within the script alleviates some drudgery from copy/paste but having the ability to identify specific techniques, passes, and texture units to modify makes material copying easier.  Techniques, passes, texture units can be identified directly in the child material without having to layout previous techniques, passes, texture units by associating a name with them,  Techniques and passes can take a name and texture units can be numbered within the material script.@*@*

@strong{NOTE:  Using names is optional so older scripts will still work just fine and you can inherit from them as well.}@*@*

Names become very usefull in materials that copy from other materials.  In order to overide values they must be in the correct technique, pass, texture unit etc.  The script could be layed out using the sequence of techniques, passes, texture units in the child material but if only one paramter needs to change in say the 5th pass then the first four passes prior to the fith would have to be placed in the script:@*@*

Here is an example:
@example
material test2 : test1
{
  technique
  {
    pass
    {
    }

    pass
    {
    }

    pass
    {
    }

    pass
    {
    }

    pass
    {
	  ambient 0.5 0.7 0.3 1.0
    }
  }
}
@end example

This method is tedious for materials that only have slight variations to their parent.
An easier way is to name the pass directly without listing the previous passes:@*

@example
material test2 : test1
{
  technique 0
  {
    pass 4
    {
	  ambient 0.5 0.7 0.3 1.0
    }
  }
}
@end example

The parent pass name must be known and the pass must be in the correct technique in order for this to work correctly.  Specifying the technique name and the pass name is the best method.  If the parent technique/pass are not named then use their index values for their name as done in the example.@*@*

@subheading Adding new Techniques, Passes, to copied materials:

If a new technique or pass needs to be added to a copied material then use a unique name for the technique or pass that does not exist in the parent material.  Using an index for the name that is one greater than the last index in the parent will do the same thing.  The new technique/pass will be added to the end of the techniques/passes copied from the parent material.@*@*

Note:  if passes or techniques aren't given a name, they will take on a default name based on their index.  For example the first pass has index 0 so its name will be 0.

@subheading Dealing with Program References and Shader Parameters:

For a child material that only needs to change some shader parameters within a pass, do the following:@*@*

@example
material furtest2 : furtest1
{
  technique ati8500
  {
    pass fur3
    {
      fragment_program_ref
      {
        param_named furlength float 0.5
      }
    }
  }
}
@end example

There is no requirement to give the name of the fragment program reference since the parent material has already done this and the same applies to a vertex program reference, but dropping the program reference name only applies if one is defined in the pass copied from the parent material.  If the parent didn't have a program reference then an exception is generated since the material parser has no clue as to what the script intended.@*@*

Program references can be added to copied passes that had no program references.  The whole program reference will have to be defined with in the pass.@*@*

If a different program ref is to be used in the copied pass then just put in the name of the different vertex/fragment program reference.  This is usefull when you want to inherit from a parent material but want to use a different shader program for one of the passes.@*@*

@subheading Identifying Texture Units to overide values

A specific texture unit state (TUS) can be specified in the pass without declaring previous texture units.  Using this for parent materials helps with readability only and is optional.  If the index number of the TUS is wrong in the parent material then the parser will just ignore the number and generate the proper index along with putting a warning message in the ogre log.  This feature is more useful for child materials that need to modify a specific TUS or need to add a new TUS.@*@*

@example
material BumpMap2 : BumpMap1
{
  technique ati8500
  {
    pass 0
    {
      texture_unit 1
      {
        texture BumpyMetalNM.png
      }
    }
  }
}
@end example

If a texture unit index is greater than what already exits in the pass, a new texture unit will be created after the last existing TUS in the current pass. if the new TUS index does not match the index specified in the script then an warning message will be placed in the log and the index value corrected.

