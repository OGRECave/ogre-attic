!define TARGET_COMPILER_DESCRIPTION "Visual C++.Net 2002"
!define TARGET_COMPILER "VC70"
!include ogresdk.nsh

Section -Samples
  ; We assume copysamples.sh has been run recently enough for these files to be available
  SetOutPath "$INSTDIR\Samples\scripts"
  SetOverwrite try
  File ".\Samples\scripts\*.vcproj"
  SetOutPath "$INSTDIR\Samples\src"
  SetOverwrite try
  File /r /x CVS /x CEGUIRenderer "..\..\Samples\*.cpp"
  SetOutPath "$INSTDIR\Samples\include"
  SetOverwrite try
  File /r /x CVS /x CEGUIRenderer "..\..\Samples\*.h"

  ; TODO: reference app source & bspcollision source




SectionEnd