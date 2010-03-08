/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2009 Torus Knot Software Ltd
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/
#import "Select_Ogre_Install_LocationPane.h"


@implementation Select_Ogre_Install_LocationPane

- (id)initWithSection:(id)parent
{
    if((self = [super initWithSection:parent]))
        validSDKChosen = NO;
    
    return self;
}

- (NSString *)title
{
	return [[NSBundle bundleForClass:[self class]] localizedStringForKey:@"InstallerPaneTitle" value:nil table:nil];
}

- (BOOL)shouldExitPane:(InstallerSectionDirection)dir
{
    if(!validSDKChosen)
    {
        NSAlert *alert = [[[NSAlert alloc] init] autorelease];
        [alert addButtonWithTitle:@"OK"];
        [alert setMessageText:@"No SDK chosen!"];
        [alert setInformativeText:@"Please choose the location of your OGRE SDK."];
        [alert setAlertStyle:NSWarningAlertStyle];
        [alert beginSheetModalForWindow:[ogreLocationLabel window]
                          modalDelegate:self
                         didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
                            contextInfo:nil];
        return NO;
    }
    return YES;
}

- (void)alertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    [[alert window] orderOut:nil];
}

- (IBAction)chooseOgreSDKLocation:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    [openPanel setDelegate:self];
    [openPanel setCanChooseFiles:NO];
    [openPanel setCanChooseDirectories:YES];
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setMessage:@"Select where the OGRE SDK is installed."];

    [openPanel beginSheetForDirectory:nil
                                 file:nil
                                types:nil
                       modalForWindow:[ogreLocationLabel window]
                        modalDelegate:self
                       didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
                          contextInfo:nil];
}

- (void)openPanelDidEnd:(NSOpenPanel *)panel returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    if(NSOKButton == returnCode) {
        NSString *ogreDirectory = [[panel filenames] objectAtIndex:0];

        // Validate that this folder contains a valid SDK
        
        // Files/folders to look for:
        //
        // OS X:
        // lib/release/Ogre.framework
        //
        // iPhone:
        // lib/release/libOgreMainStatic.a
        // include/OGRE/Ogre.h
        BOOL isDir;
        NSString *frameworkPath = [ogreDirectory stringByAppendingString:@"/lib/release/Ogre.framework"];
        NSString *iPhoneLibPath = [ogreDirectory stringByAppendingString:@"/lib/release/libOgreMainStatic.a"];
        NSString *iPhoneHeaderPath = [ogreDirectory stringByAppendingString:@"/include/OGRE/Ogre.h"];

        if(([[NSFileManager defaultManager] fileExistsAtPath:frameworkPath isDirectory:&isDir] && isDir) ||
            ([[NSFileManager defaultManager] fileExistsAtPath:iPhoneLibPath]) ||
            ([[NSFileManager defaultManager] fileExistsAtPath:iPhoneHeaderPath])) {
            // Set the flag to show that will be ok to move on to the next installer section
            validSDKChosen = YES;
        }
        else
        {
            validSDKChosen = NO;
        }

        if(validSDKChosen)
        {
            // Pull the version info out of config.h
            NSMutableString *configFilePath = [ogreDirectory mutableCopy];
            NSString *fileContents = nil;
            NSArray *lines = nil;
            [configFilePath appendString:@"/lib/release/Ogre.framework/Headers/config.h"];
            fileContents = [NSString stringWithContentsOfFile:configFilePath];
            lines = [fileContents componentsSeparatedByString:@"\n"];
            
            for(int i = 0; i < [lines count]; i++)
            {
                if([[lines objectAtIndex:i] rangeOfString:@"#define VERSION"].length > 0)
                {
                    NSArray *components = [[lines objectAtIndex:i] componentsSeparatedByString:@"\""];

                    [ogreInfoLabel setStringValue:[NSString stringWithFormat:@"%@%@", @"Found Ogre version ", [components objectAtIndex:1]]];
                }
            }
            
            // Update the GUI
            [ogreLocationLabel setStringValue:ogreDirectory];

            // Replace the placeholder string in the project files with the SDK root chosen by the user
            NSMutableString *projectFileContents = [NSMutableString stringWithContentsOfFile:@"/Library/Application Support/Developer/Shared/Xcode/Project Templates/Ogre/Mac OS X/___PROJECTNAME___.xcodeproj/project.pbxproj"];
            [projectFileContents replaceOccurrencesOfString:@"_OGRESDK_ROOT_"
                                                 withString:ogreDirectory
                                                    options:NSLiteralSearch
                                                      range:NSMakeRange(0, [projectFileContents length])];
            [projectFileContents writeToFile:@"/Library/Application Support/Developer/Shared/Xcode/Project Templates/Ogre/Mac OS X/___PROJECTNAME___.xcodeproj/project.pbxproj" atomically:YES];

            projectFileContents = [NSMutableString stringWithContentsOfFile:@"/Library/Application Support/Developer/Shared/Xcode/Project Templates/Ogre/iPhone OS/___PROJECTNAME___.xcodeproj/project.pbxproj"];
            [projectFileContents replaceOccurrencesOfString:@"_OGRESDK_ROOT_"
                                                 withString:ogreDirectory
                                                    options:NSLiteralSearch
                                                      range:NSMakeRange(0, [projectFileContents length])];
            [projectFileContents writeToFile:@"/Library/Application Support/Developer/Shared/Xcode/Project Templates/Ogre/iPhone OS/___PROJECTNAME___.xcodeproj/project.pbxproj" atomically:YES];

            // Register the file type from extension.  Create a Uniform Type Identifier for each one
            CFStringRef particleUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("particle"), kUTTypePlainText);
            CFStringRef materialUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("material"), kUTTypePlainText);
            CFStringRef compositorUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("compositor"), kUTTypePlainText);
            CFStringRef fontdefUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("fontdef"), kUTTypePlainText);
            CFStringRef overlayUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("overlay"), kUTTypePlainText);
            CFStringRef programUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("program"), kUTTypePlainText);
            CFStringRef genericScriptUTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("os"), kUTTypePlainText);

            // Register Xcode as the default editor
            LSSetDefaultRoleHandlerForContentType(particleUTI, (kLSRolesEditor | kLSRolesViewer), CFSTR("com.apple.Xcode"));
            LSSetDefaultRoleHandlerForContentType(materialUTI, (kLSRolesEditor | kLSRolesViewer), CFSTR("com.apple.Xcode"));
            LSSetDefaultRoleHandlerForContentType(compositorUTI, (kLSRolesEditor | kLSRolesViewer), CFSTR("com.apple.Xcode"));
            LSSetDefaultRoleHandlerForContentType(fontdefUTI, (kLSRolesEditor | kLSRolesViewer), CFSTR("com.apple.Xcode"));
            LSSetDefaultRoleHandlerForContentType(overlayUTI, (kLSRolesEditor | kLSRolesViewer), CFSTR("com.apple.Xcode"));
            LSSetDefaultRoleHandlerForContentType(programUTI, (kLSRolesEditor | kLSRolesViewer), CFSTR("com.apple.Xcode"));
            LSSetDefaultRoleHandlerForContentType(genericScriptUTI, (kLSRolesEditor | kLSRolesViewer), CFSTR("com.apple.Xcode"));
        }
        [panel close];
    }
}

@end
