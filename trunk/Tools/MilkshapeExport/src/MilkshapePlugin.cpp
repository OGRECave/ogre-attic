/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 The OGRE Team
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

#include "MilkshapePlugin.h"
#include "Ogre.h"
#include "msLib.h"
#include "resource.h"


//---------------------------------------------------------------------
MilkshapePlugin::MilkshapePlugin ()
{
    strcpy(mTitle, "OGRE Mesh / Skeleton...");

}
//---------------------------------------------------------------------
MilkshapePlugin::~MilkshapePlugin ()
{
    // do nothing
}
//---------------------------------------------------------------------
int MilkshapePlugin::GetType ()
{
    return cMsPlugIn::eTypeExport;
}
//---------------------------------------------------------------------
const char* MilkshapePlugin::GetTitle ()
{
    return mTitle;
}
//---------------------------------------------------------------------
int MilkshapePlugin::Execute (msModel* pModel)
{
    // Do nothing if no model selected
    if (!pModel)
        return -1;

    //
    // check, if we have something to export
    //
    if (msModel_GetMeshCount (pModel) == 0)
    {
        ::MessageBox (NULL, "The model is empty!  Nothing exported!", "OGRE Export", MB_OK | MB_ICONWARNING);
        return 0;
    }

    showOptions();

    if (exportMesh)
    {
        doExportMesh(pModel);
    }


    return 0;

}
//---------------------------------------------------------------------
MilkshapePlugin *plugin;
BOOL MilkshapePlugin::DlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndDlgItem;

    switch (iMsg)
    {

    case WM_INITDIALOG:
        // Center myself
        int x, y, screenWidth, screenHeight;
        RECT rcDlg;
        GetWindowRect(hDlg, &rcDlg);
        screenWidth = GetSystemMetrics(SM_CXFULLSCREEN);
        screenHeight = GetSystemMetrics(SM_CYFULLSCREEN);

        x = (screenWidth / 2) - ((rcDlg.right - rcDlg.left) / 2);
        y = (screenHeight / 2) - ((rcDlg.bottom - rcDlg.top) / 2);

        MoveWindow(hDlg, x, y, (rcDlg.right - rcDlg.left),
            (rcDlg.bottom - rcDlg.top), TRUE);

        // Check mesh export
        hwndDlgItem = GetDlgItem(hDlg, IDC_EXPORT_MESH);
        SendMessage(hwndDlgItem, BM_SETCHECK, BST_CHECKED,0);

        // Check skeleton export
        hwndDlgItem = GetDlgItem(hDlg, IDC_EXPORT_SKEL);
        SendMessage(hwndDlgItem, BM_SETCHECK, BST_CHECKED,0);

        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            case IDOK:
                // Set options
                hwndDlgItem = GetDlgItem(hDlg, IDC_EXPORT_MESH);
                plugin->exportMesh = (SendMessage(hwndDlgItem, BM_GETCHECK, 0, 0) == BST_CHECKED) ? true : false;

                hwndDlgItem = GetDlgItem(hDlg, IDC_EXPORT_SKEL);
                plugin->exportSkeleton = (SendMessage(hwndDlgItem, BM_GETCHECK, 0, 0) == BST_CHECKED) ? true : false;

                hwndDlgItem = GetDlgItem(hDlg, IDC_EXPORT_MATERIALS);
                plugin->exportMaterials = (SendMessage(hwndDlgItem, BM_GETCHECK, 0, 0) == BST_CHECKED) ? true : false;
                
                hwndDlgItem = GetDlgItem(hDlg, IDC_SPLIT_ANIMATION);
                plugin->splitAnimations = (SendMessage(hwndDlgItem, BM_GETCHECK, 0, 0) == BST_CHECKED) ? true : false;

                EndDialog(hDlg, TRUE);
                return TRUE;
            case IDCANCEL:
                EndDialog(hDlg, FALSE);
                return TRUE;
        }
    }

    return FALSE;

}

//---------------------------------------------------------------------
void MilkshapePlugin::showOptions(void)
{
    int i;
    HINSTANCE hInst = GetModuleHandle("msOGREExporter.dll");
    plugin = this;
    exportMesh = true;
    exportSkeleton = false;
    exportMaterials = false;
    i = DialogBox(hInst, MAKEINTRESOURCE(IDD_OPTIONS), NULL, DlgProc);


    

}

void MilkshapePlugin::doExportMesh(msModel* pModel)
{


    // Create singletons
    Ogre::MaterialManager matMgr;
    Ogre::SkeletonManager skelMgr;
    Ogre::LogManager logMgr;

    
    logMgr.createLog("msOgreExporter.log");
    logMgr.logMessage("OGRE Milkshape Exporter Log");
    logMgr.logMessage("---------------------------");
    //
    // choose filename
    //
    OPENFILENAME ofn;
    memset (&ofn, 0, sizeof (OPENFILENAME));
    
    char szFile[MS_MAX_PATH];
    char szFileTitle[MS_MAX_PATH];
    char szDefExt[32] = "mesh";
    char szFilter[128] = "OGRE .mesh Files (*.mesh)\0*.mesh\0All Files (*.*)\0*.*\0\0";
    szFile[0] = '\0';
    szFileTitle[0] = '\0';

    ofn.lStructSize = sizeof (OPENFILENAME);
    ofn.lpstrDefExt = szDefExt;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MS_MAX_PATH;
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = MS_MAX_PATH;
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle = "Export to OGRE Mesh";

    if (!::GetSaveFileName (&ofn))
        return /*0*/;

    logMgr.logMessage("Creating Mesh object...");
    Ogre::Mesh* ogreMesh = new Ogre::Mesh("export");
    logMgr.logMessage("Mesh object created.");

    bool foundBoneAssignment = false;

    // No shared geometry
    int i, j;
    for (i = 0; i < msModel_GetMeshCount (pModel); i++)
    {
        msMesh *pMesh = msModel_GetMeshAt (pModel, i);


        logMgr.logMessage("Creating SubMesh object...");
        Ogre::SubMesh* ogreSubMesh = ogreMesh->createSubMesh();
        logMgr.logMessage("SubMesh object created.");
        // Set material
        logMgr.logMessage("Getting SubMesh Material...");
        int matIdx = msMesh_GetMaterialIndex(pMesh);

        if (matIdx == -1)
        {
            // No material, use blank
            ogreSubMesh->setMaterialName("BaseWhite");
            logMgr.logMessage("No Material, using default 'BaseWhite'.");
        }
        else
        {

            msMaterial *pMat = msModel_GetMaterialAt(pModel, matIdx);
            ogreSubMesh->setMaterialName(pMat->szName);
            logMgr.logMessage("SubMesh Material Done.");
        }

        
        logMgr.logMessage("Setting up geometry...");
        // Set up mesh geometry
        // Always 1 texture layer, 2D coords
        ogreSubMesh->geometry.numTexCoords = 1;
        ogreSubMesh->geometry.numTexCoordDimensions[0] = 2;
        ogreSubMesh->geometry.hasNormals = true;
        ogreSubMesh->geometry.hasColours = false;
        ogreSubMesh->geometry.vertexStride = 0;
        ogreSubMesh->geometry.texCoordStride[0] = 0;
        ogreSubMesh->geometry.normalStride = 0;
        ogreSubMesh->useSharedVertices = false;
        ogreSubMesh->useTriStrips = false;

        ogreSubMesh->geometry.numVertices = msMesh_GetVertexCount (pMesh);
        ogreSubMesh->geometry.pVertices = new Ogre::Real[ogreSubMesh->geometry.numVertices * 3];
        ogreSubMesh->geometry.pNormals = new Ogre::Real[ogreSubMesh->geometry.numVertices * 3];
        ogreSubMesh->geometry.pTexCoords[0] = new Ogre::Real[ogreSubMesh->geometry.numVertices * 2];
        for (j = 0; j < ogreSubMesh->geometry.numVertices; ++j)
        {
            msVertex *pVertex = msMesh_GetVertexAt (pMesh, j);
            msVec3 Vertex;
            msVec2 uv;

            msVertex_GetVertex (pVertex, Vertex);
            msVertex_GetTexCoords (pVertex, uv);

            ogreSubMesh->geometry.pVertices[j*3] = Vertex[0];
            ogreSubMesh->geometry.pVertices[(j*3)+1] = Vertex[1];
            ogreSubMesh->geometry.pVertices[(j*3)+2] = Vertex[2];

            ogreSubMesh->geometry.pTexCoords[0][j*2] = uv[0];
            // Invert the 'v' texture coordinate, Milkshape appears to treat 0 as the TOP of the texture
            //   like D3D, OGRE uses the reverse
            ogreSubMesh->geometry.pTexCoords[0][(j*2)+1] = 1 - uv[1];

            int boneIdx = msVertex_GetBoneIndex(pVertex);
            if (boneIdx != -1)
            {
                foundBoneAssignment = true;
                Ogre::VertexBoneAssignment vertAssign;
                vertAssign.boneIndex = boneIdx;
                vertAssign.vertexIndex = j;
                vertAssign.weight = 1.0; // Milkshape only supports single assignments
                ogreSubMesh->addBoneAssignment(vertAssign);
            }


        }
        // Aargh, Milkshape uses stupid separate normal indexes for the same vertex like 3DS
        // Normals aren't described per vertex but per triangle vertex index
        // Pain in the arse, we have to do vertex duplication again if normals differ at a vertex (non smooth)
        // WHY don't people realise this format is a pain for passing to 3D APIs in vertex buffers?
        ogreSubMesh->numFaces = msMesh_GetTriangleCount (pMesh);
        ogreSubMesh->faceVertexIndices = new unsigned short[ogreSubMesh->numFaces * 3];
        for (j = 0; j < ogreSubMesh->numFaces; j++)
        {
            msTriangle *pTriangle = msMesh_GetTriangleAt (pMesh, j);
            
            word nIndices[3];
            msTriangle_GetVertexIndices (pTriangle, nIndices);

            msVec3 Normal;
            int k, normIdx, vertIdx;
            for (k = 0; k < 3; ++k)
            {
                vertIdx = nIndices[k];
                // Face index
                ogreSubMesh->faceVertexIndices[(j*3)+k] = vertIdx;

                // Vertex normals
                // For the moment, ignore any discrepancies per vertex
                normIdx = pTriangle->nNormalIndices[k];
                msMesh_GetVertexNormalAt (pMesh, normIdx, Normal);

                ogreSubMesh->geometry.pNormals[(vertIdx*3)] = Normal[0];
                ogreSubMesh->geometry.pNormals[(vertIdx*3)+1] = Normal[1];
                ogreSubMesh->geometry.pNormals[(vertIdx*3)+2] = Normal[2];

            }


        } // Faces

        logMgr.logMessage("Geometry done.");
    } // SubMesh


    if (exportMaterials)
    {
        doExportMaterials(pModel);
    }

    // Keep hold of a Skeleton pointer for deletion later
    // Mesh uses Skeleton pointer for skeleton name
    Ogre::Skeleton* pSkel = 0;

    if (exportSkeleton && foundBoneAssignment)
    {
        // export skeleton, also update mesh to point to it
        pSkel = doExportSkeleton(pModel, ogreMesh);
    }
    else if (!exportSkeleton && foundBoneAssignment)
    {
        // We've found bone assignments, but skeleton is not to be exported
        // Prompt the user to find the skeleton 
        if (!locateSkeleton(ogreMesh))
            return;

    }
    
    // Export
    logMgr.logMessage("Creating MeshSerializer..");
    Ogre::MeshSerializer serializer;
    logMgr.logMessage("MeshSerializer created.");

    // Export, no materials for now
    Ogre::String msg;
    msg << "Exporting mesh data to file '" << szFile << "'";
    logMgr.logMessage(msg);
    serializer.exportMesh(ogreMesh, szFile, false);
    logMgr.logMessage("Export successful");

    delete ogreMesh;
    if (pSkel)
        delete pSkel;
}

void MilkshapePlugin::doExportMaterials(msModel* pModel)
{
    Ogre::MaterialManager& matMgr = Ogre::MaterialManager::getSingleton();

    int matCount = msModel_GetMaterialCount(pModel);
    for (int i = 0; i < matCount; ++i)
    {
        msMaterial *pMat = msModel_GetMaterialAt(pModel, i);
        // Create deferred material so no load
        Ogre::Material* ogreMat = (Ogre::Material*)matMgr.createDeferred(pMat->szName);

        msVec4 vec4;
        msMaterial_GetAmbient (pMat, vec4);
        ogreMat->setAmbient(vec4[0], vec4[1], vec4[2]);
        msMaterial_GetDiffuse (pMat, vec4);
        ogreMat->setDiffuse(vec4[0], vec4[1], vec4[2]);
        msMaterial_GetSpecular (pMat, vec4);
        ogreMat->setSpecular(vec4[0], vec4[1], vec4[2]);
        ogreMat->setShininess(msMaterial_GetShininess(pMat));

        char szTexture[MS_MAX_PATH];
        msMaterial_GetDiffuseTexture (pMat, szTexture, MS_MAX_PATH);
        if (strlen(szTexture) > 0)
        {
            // Diffuse texture only
            ogreMat->addTextureLayer(szTexture);
        }

    }
}

Ogre::Skeleton* MilkshapePlugin::doExportSkeleton(msModel* pModel, Ogre::Mesh* mesh)
{
    Ogre::LogManager &logMgr = Ogre::LogManager::getSingleton();
    Ogre::String msg;

    //
    // choose filename
    //
    OPENFILENAME ofn;
    memset (&ofn, 0, sizeof (OPENFILENAME));
    
    char szFile[MS_MAX_PATH];
    char szFileTitle[MS_MAX_PATH];
    char szDefExt[32] = "skeleton";
    char szFilter[128] = "OGRE .skeleton Files (*.skeleton)\0*.skeleton\0All Files (*.*)\0*.*\0\0";
    szFile[0] = '\0';
    szFileTitle[0] = '\0';

    ofn.lStructSize = sizeof (OPENFILENAME);
    ofn.lpstrDefExt = szDefExt;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MS_MAX_PATH;
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = MS_MAX_PATH;
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle = "Export to OGRE Skeleton";

    if (!::GetSaveFileName (&ofn))
        return 0;

    // Strip off the path
    Ogre::String skelName = szFile;
    size_t lastSlash = skelName.find_last_of("\\");
    skelName = skelName.substr(lastSlash+1);

    // Set up
    logMgr.logMessage("Trying to create Skeleton object");
    Ogre::Skeleton *ogreskel = new Ogre::Skeleton(skelName);
    logMgr.logMessage("Skeleton object created");

    // Complete the details
    
    // Do the bones
    int numBones = msModel_GetBoneCount(pModel);
    msg = "Number of bones: ";
    msg << numBones;
    logMgr.logMessage(msg);

    int i;
    // Create all the bones in turn
    for (i = 0; i < numBones; ++i)
    {
        msBone* bone = msModel_GetBoneAt(pModel, i);
        Ogre::Bone* ogrebone = ogreskel->createBone(bone->szName);

        msVec3 msBonePos, msBoneRot;
        msBone_GetPosition(bone, msBonePos);
        msBone_GetRotation(bone, msBoneRot);

        Ogre::Vector3 bonePos(msBonePos[0], msBonePos[1], msBonePos[2]);
        ogrebone->setPosition(bonePos);
        // Hmm, Milkshape has chosen a Euler angle representation of orientation which is not smart
        // Rotation Matrix or Quaternion would have been the smarter choice
        // Might we have Gimbal lock here? What order are these 3 angles supposed to be applied?
        // Grr, we'll try our best anyway...
        Ogre::Quaternion qx, qy, qz, qfinal;
        qx.FromAngleAxis(msBoneRot[0], Ogre::Vector3::UNIT_X);
        qy.FromAngleAxis(msBoneRot[1], Ogre::Vector3::UNIT_Y);
        qz.FromAngleAxis(msBoneRot[2], Ogre::Vector3::UNIT_Z);

        // Assume rotate by x then y then z
        qfinal = qz * qy * qx;
        ogrebone->setOrientation(qfinal);

        msg = "";
        msg << "Bone #" << i << ": " <<
            "Name='" << bone->szName << "' " <<
            "Position: " << bonePos << " " <<
            "Ms3d Rotation: {" << msBoneRot[0] << ", " << msBoneRot[1] << ", " << msBoneRot[2] << "} " <<
            "Orientation: " << qfinal;
        logMgr.logMessage(msg);

        
    }
    // Now we've created all the bones, link them up
    logMgr.logMessage("Establishing bone hierarchy..");
    for (i = 0; i < numBones; ++i)
    {
        msBone* bone = msModel_GetBoneAt(pModel, i);

        if (strlen(bone->szParentName) == 0)
        {
            // Root bone
            msg = "Root bone detected: Name='";
            msg << bone->szName << "' Index=" << i;
            logMgr.logMessage(msg);
        }
        else
        {
            Ogre::Bone* ogrechild = ogreskel->getBone(bone->szName);
            Ogre::Bone* ogreparent = ogreskel->getBone(bone->szParentName);

            if (ogrechild == 0)
            {
                msg = "Error: could not locate child bone '";
                msg << bone->szName << "'";
                logMgr.logMessage(msg);
                continue;
            }
            if (ogreparent == 0)
            {
                msg = "Error: could not locate parent bone '";
                msg << bone->szParentName << "'";
                logMgr.logMessage(msg);
                continue;
            }
            // Make child
            ogreparent->addChild(ogrechild);
        }


    }
    logMgr.logMessage("Bone hierarchy established.");

    // Create the Animation(s)
    doExportAnimations(pModel, ogreskel);



    // Create skeleton serializer & export
    Ogre::SkeletonSerializer serializer;
    msg = "";
    msg << "Exporting skeleton to " << szFile;
    logMgr.logMessage(msg);
    serializer.exportSkeleton(ogreskel, szFile);
    logMgr.logMessage("Skeleton exported");


    msg << "Linking mesh to skeleton file '" << skelName << "'";
    Ogre::LogManager::getSingleton().logMessage(msg);
    
    mesh->_notifySkeleton(ogreskel);

    return ogreskel;

}

bool MilkshapePlugin::locateSkeleton(Ogre::Mesh* mesh)
{
    //
    // choose filename
    //
    OPENFILENAME ofn;
    memset (&ofn, 0, sizeof (OPENFILENAME));
    
    char szFile[MS_MAX_PATH];
    char szFileTitle[MS_MAX_PATH];
    char szDefExt[32] = "skeleton";
    char szFilter[128] = "OGRE .skeleton Files (*.skeleton)\0*.skeleton\0All Files (*.*)\0*.*\0\0";
    szFile[0] = '\0';
    szFileTitle[0] = '\0';

    ofn.lStructSize = sizeof (OPENFILENAME);
    ofn.lpstrDefExt = szDefExt;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MS_MAX_PATH;
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = MS_MAX_PATH;
    ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrTitle = "Locate OGRE Skeleton (since you're not exporting it)";

    if (!::GetOpenFileName (&ofn))
        return false;

    // Strip off the path
    Ogre::String skelName = szFile;
    size_t lastSlash = skelName.find_last_of("\\");
    skelName = skelName.substr(lastSlash+1);

    Ogre::String msg;
    msg << "Linking mesh to skeleton file '" << skelName << "'";
    Ogre::LogManager::getSingleton().logMessage(msg);
    
    // Create a dummy skeleton for Mesh to link to (saves it trying to load it)
    Ogre::Skeleton* pSkel = (Ogre::Skeleton*)Ogre::SkeletonManager::getSingleton().create(skelName);
    Ogre::LogManager::getSingleton().logMessage("Dummy Skeleton object created for link.");

    mesh->_notifySkeleton(pSkel);

    return true;

}

struct SplitAnimationStruct
{
    int start;
    int end;
    Ogre::String name;
};

void MilkshapePlugin::doExportAnimations(msModel* pModel, Ogre::Skeleton* ogreskel)
{

    Ogre::LogManager& logMgr = Ogre::LogManager::getSingleton();
    std::vector<SplitAnimationStruct> splitInfo;
    Ogre::String msg;

    int numFrames = msModel_GetTotalFrames(pModel);
    msg = "Number of frames: ";
    msg << numFrames;
    logMgr.logMessage(msg);

    if (splitAnimations)
    {
        // Explain
        msg = "You have chosen to create multiple discrete animations by splitting up the frames in ";
        msg << "the animation sequence. In order to do this, you must supply a simple text file "
            << "describing the separate animations, which has a single line per animation in the format: \n\n" 
            << "startFrame,endFrame,animationName\n\n" << "For example: \n\n"
            << "1,20,Walk\n21,35,Run\n36,40,Shoot\n\n" 
            << "..creates 3 separate animations (the frame numbers are inclusive). You must browse to this file in the next dialog.";
        MessageBox(0,msg.c_str(), "Splitting Animations",MB_ICONINFORMATION | MB_OK);
        // Prompt for a file which contains animation splitting info
        OPENFILENAME ofn;
        memset (&ofn, 0, sizeof (OPENFILENAME));
        
        char szFile[MS_MAX_PATH];
        char szFileTitle[MS_MAX_PATH];
        char szDefExt[32] = "skeleton";
        char szFilter[128] = "All Files (*.*)\0*.*\0\0";
        szFile[0] = '\0';
        szFileTitle[0] = '\0';

        ofn.lStructSize = sizeof (OPENFILENAME);
        ofn.lpstrDefExt = szDefExt;
        ofn.lpstrFilter = szFilter;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MS_MAX_PATH;
        ofn.lpstrFileTitle = szFileTitle;
        ofn.nMaxFileTitle = MS_MAX_PATH;
        ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        ofn.lpstrTitle = "Open animation split configuration file";

        if (!::GetOpenFileName (&ofn))
        {
            msg = "Splitting aborted, generating a single animation called 'Default'";
            MessageBox(0, msg.c_str(), "Info", MB_OK | MB_ICONWARNING);
            SplitAnimationStruct split;
            split.start = 1;
            split.end = numFrames;
            split.name = "Default";
            splitInfo.push_back(split);
        }
        else
        {
            // Read file
            Ogre::String sline;
            char line[256];
            SplitAnimationStruct newSplit;

            std::ifstream istr;
            istr.open("c:\\temp\\test.txt");

            while (!istr.eof())
            {
                istr.getline(line, 256);
                sline = line;

                // Ignore blanks & comments
                if (sline == "" || sline.substr(0,2) == "//")
                    continue;

                // Split on ','
                std::vector<Ogre::String> svec = sline.split(",\n");

                // Basic validation on number of elements
                if (svec.size() != 3)
                {
                    MessageBox(0, "Warning: corrupt animation details in file. You should look into this. ",
                        "Corrupt animations file", MB_ICONWARNING | MB_OK);
                    continue;
                }
                // Remove any embedded spaces
                svec[0].trim();
                svec[1].trim();
                svec[2].trim();
                // Create split info
                newSplit.start = atoi(svec[0].c_str());
                newSplit.end = atoi(svec[1].c_str());
                newSplit.name = svec[2];
                splitInfo.push_back(newSplit);

            }


        }

    }
    else
    {
        // No splitting
        SplitAnimationStruct split;
        split.start = 1;
        split.end = numFrames;
        split.name = "Default";
        splitInfo.push_back(split);
    }

    // Get animation length
    // Map frames -> seconds, this can be changed in speed of animation anyway



    int numBones = msModel_GetBoneCount(pModel);

    std::vector<SplitAnimationStruct>::iterator animsIt;
    for (animsIt = splitInfo.begin(); animsIt != splitInfo.end(); ++animsIt)
    {
        SplitAnimationStruct& currSplit = *animsIt;

        // Create animation
        msg = "Trying to create Animation object for animation ";
        msg <<  currSplit.name << " For Frames " << currSplit.start << " to "
            << currSplit.end << " inclusive.";
        logMgr.logMessage(msg);
        Ogre::Animation *ogreanim = 
            ogreskel->createAnimation(currSplit.name, (currSplit.end - currSplit.start) + 1);
        logMgr.logMessage("Animation object created.");

        int i;
        // Create all the animation tracks
        for (i = 0; i < numBones; ++i)
        {

            msBone* bone = msModel_GetBoneAt(pModel, i);
            Ogre::Bone* ogrebone = ogreskel->getBone(bone->szName);

            // Create animation tracks
            msg = "";
            msg << "Creating AnimationTrack for bone " << i;
            logMgr.logMessage(msg);

            Ogre::AnimationTrack *ogretrack = ogreanim->createTrack(i, ogrebone);
            logMgr.logMessage("Animation track created.");

            // OGRE uses keyframes which are both position and rotation
            // Milkshape separates them, so create merged OGRE keyframes
            int numPosKeys = msBone_GetPositionKeyCount(bone);
            int numRotKeys = msBone_GetRotationKeyCount(bone);

            msg = "";
            msg << "Number of keyframes: Pos: " << numPosKeys << " Rot: " << numRotKeys;
            logMgr.logMessage(msg);

            int currPosIdx, currRotIdx;
            msPositionKey* currPosKey;
            msRotationKey* currRotKey;
            currPosKey = msBone_GetPositionKeyAt(bone, 0);
            currRotKey = msBone_GetRotationKeyAt(bone, 0);
            int ogreKeyFrameCount = 0;
            for (currPosIdx = 0, currRotIdx = 0; 
                currPosIdx < numPosKeys && currRotIdx < numRotKeys;
                ++ogreKeyFrameCount )
            {
                Ogre::Real time;
                if (currPosKey->fTime > currRotKey->fTime)
                {
                    time = currPosKey->fTime;
                }
                else
                {
                    time = currRotKey->fTime;
                }

                // Make sure keyframe is in current time frame (for splitting)
                if (time >= currSplit.start && time <= currSplit.end)
                {

                    msg = "";
                    msg << "Creating combined (pos & rot) KeyFrame #" << ogreKeyFrameCount <<
                        " for bone #" << i;
                    logMgr.logMessage(msg);
                    // Create keyframe
                    // Adjust for start time, and for the fact that frames are numbered from 1
                    Ogre::KeyFrame *ogrekey = ogretrack->createKeyFrame(time - currSplit.start);
                    logMgr.logMessage("KeyFrame created");

                    Ogre::Vector3 kfPos(currPosKey->Position[0], currPosKey->Position[1], currPosKey->Position[2]);
                    Ogre::Quaternion qx, qy, qz, kfQ;

                    ogrekey->setTranslate(kfPos);
                    qx.FromAngleAxis(currRotKey->Rotation[0], Ogre::Vector3::UNIT_X);
                    qy.FromAngleAxis(currRotKey->Rotation[1], Ogre::Vector3::UNIT_Y);
                    qz.FromAngleAxis(currRotKey->Rotation[2], Ogre::Vector3::UNIT_Z);
                    kfQ = qz * qy * qx;
                    ogrekey->setRotation(kfQ);

                    msg = "";
                    msg << "KeyFrame details: Time=" << time << " Position=" << kfPos << " " <<
                        "Ms3d Rotation= {" << currRotKey->Rotation[0] << ", " << currRotKey->Rotation[1] << ", " << currRotKey->Rotation[2] << "} " <<
                        "Orientation=" << kfQ;
                    logMgr.logMessage(msg);
                } // keyframe creation

                // Check to see which one is next
                if (currPosIdx+1 == numPosKeys && currRotIdx+1 == numRotKeys)
                    break; // were done

                if (currPosIdx+1 == numPosKeys)
                {
                    // Use next rotation index since position indexes have run out
                    currRotIdx++;
                }
                else if (currRotIdx+1 == numRotKeys)
                {
                    // Use next position index since rotation indexes have run out
                    currPosIdx++;
                }
                else
                {
                    // Neither have run out, time to compare the times of the next of each
                    // We want the lowest time of the next of each
                    msPositionKey* possPosKey = msBone_GetPositionKeyAt(bone, currPosIdx+1);
                    msRotationKey* possRotKey = msBone_GetRotationKeyAt(bone, currRotIdx+1);

                    if (possPosKey->fTime == possRotKey->fTime)
                    {
                        // Increment both if equal
                        currPosIdx++;
                        currRotIdx++;
                    }
                    else if (possPosKey->fTime < possRotKey->fTime)
                    {
                        currPosIdx++;
                    }
                    else
                    {
                        currRotIdx++;
                    }
                }
                
                // Get the keys
                currPosKey = msBone_GetPositionKeyAt(bone, currPosIdx);
                currRotKey = msBone_GetRotationKeyAt(bone, currRotIdx);

            } // keys
        } //Bones
    } // Animations




}

