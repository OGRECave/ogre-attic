!define TARGET_COMPILER_DESCRIPTION "Visual C++ 6.0"
!define TARGET_COMPILER "VC6"
!include ogresdk.nsh

Section -Samples
  ; We assume copysamples.sh has been run recently enough for these files to be available
  SetOutPath "$INSTDIR\samples\scripts"
  SetOverwrite try
  File ".\Samples\scripts\*.dsp"
  File ".\ReferenceApplication\BspCollision\scripts\*.dsp"
  SetOutPath "$INSTDIR\samples\src"
  SetOverwrite try
  File /r /x CVS /x CEGUIRenderer "..\..\Samples\*.cpp"
  File "..\..\ReferenceApplication\BspCollision\src\*.cpp"
  SetOutPath "$INSTDIR\samples\include"
  SetOverwrite try
  File /r /x CVS /x CEGUIRenderer "..\..\Samples\*.h"
  File "..\..\ReferenceApplication\BspCollision\include\*.h"

  ; Refapp
  SetOutPath "$INSTDIR\samples\refapp\scripts"
  SetOverwrite try
  File ".\ReferenceApplication\scripts\*.dsp"
  SetOutPath "$INSTDIR\samples\refapp\src"
  SetOverwrite try
  File "..\..\ReferenceApplication\ReferenceAppLayer\src\*.cpp"
  SetOutPath "$INSTDIR\samples\refapp\include"
  SetOverwrite try
  File "..\..\ReferenceApplication\ReferenceAppLayer\include\*.h"


  SetOutPath "$INSTDIR\samples"
  SetOverwrite try
  File ".\Samples\Samples.dsw"
  File ".\Samples\Samples.opt"


SectionEnd