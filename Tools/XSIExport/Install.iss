
[Setup]
AppName=OGRE XSI Exporter
AppVersion=1.0.7
AppVerName=OGRE XSI Exporter 1.0.7
DefaultDirName=c:\SoftImage\XSI_4.2
DefaultGroupName=OGRE XSI Exporter
Compression=lzma
SolidCompression=yes
EnableDirDoesntExistWarning=yes

[Files]
Source: "bin\Release\OgreXSIExporter.dll"; DestDir: "{app}\Application\Plugins"
Source: "..\Common\bin\release\OgreMain.dll"; DestDir: "{app}\Application\bin"
Source: "..\Common\bin\release\devil.dll"; DestDir: "{app}\Application\bin"
Source: "..\Common\bin\release\ilu.dll"; DestDir: "{app}\Application\bin"
Source: "..\Common\bin\release\ilut.dll"; DestDir: "{app}\Application\bin"
Source: "..\Common\bin\release\zlib1.dll"; DestDir: "{app}\Application\bin"
Source: "extras\STLport_VC746.DLL"; DestDir: "{sys}"
Source: "extras\msvcp70.DLL"; DestDir: "{sys}"
Source: "extras\msvcr70.DLL"; DestDir: "{sys}"

Source: "OGREXSI_Readme.html"; DestDir: "{app}"; Flags: isreadme
Source: "page1.gif"; DestDir: "{app}"
Source: "page2.gif"; DestDir: "{app}"
Source: "page3.gif"; DestDir: "{app}"

[Icons]
Name: "{group}\README"; Filename: "{app}\OGREXSI_Readme.html"
Name: "{group}\Uninstall"; Filename: "{uninstallexe}"

[Messages]
WizardSelectDir=Select XSI Folder
SelectDirDesc=Please identify the location of your XSI folder (e.g. c:\SoftImage\XSI_4.2)
SelectDirLabel3=Setup will install the exporter into your existing XSI location
DirExistsTitle=Confirm XSI Folder
DirExists=Are you sure this is your XSI folder?
DirDoesntExistTitle=Folder Does Not Exist
DirDoesntExist=The folder:%n%n%1%n%ndoes not exist. You are strongly advised to click 'No' and pick your correct XSI folder.

