s/\.\.\\\.\.\\\.\.\\OgreMain\\include/$(OGRE_HOME)\\include/i
s/\.\.\\\.\.\\\.\.\\Dependencies\\include//i
s/\.\.\\\.\.\\\.\.\\OgreMain\\lib\\\$(ConfigurationName)/$(OGRE_HOME)\\lib/i
s/\"\.\.\\\.\.\\\.\.\\Dependencies\\lib\\\$(ConfigurationName)\"//i
s/\.\.\\lib/\.\.\\\.\.\\lib/g
s/\.\.\\\.\.\\\.\.\\OgreMain\\lib\\Debug/$(OGRE_HOME)\\lib/i
s/\.\.\\\.\.\\\.\.\\OgreMain\\lib\\Release/$(OGRE_HOME)\\lib/i
s/CommandLine=\"copy.*\"/CommandLine=\"copy $(OutDir)\\$(TargetFileName) $(OGRE_HOME)\\bin\\$(ConfigurationName)\\"/i
s/PostBuild_Cmds=copy \.\.\\\.\.\\lib\\Release.*/PostBuild_Cmds=copy \.\.\\lib\\Release\\\*\.dll $(OGRE_HOME)\\bin\\Release/i
s/PostBuild_Cmds=copy \.\.\\\.\.\\lib\\Debug.*/PostBuild_Cmds=copy \.\.\\lib\\Debug\\\*\.dll $(OGRE_HOME)\\bin\\Debug/i



