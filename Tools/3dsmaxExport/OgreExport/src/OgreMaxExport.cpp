#include "windows.h"
#include "max.h"
#include "plugapi.h"
#include "stdmat.h"
#include "impexp.h"
#include "CS/bipexp.h"
#include "CS/phyexp.h"
#include "OgreExport.h"
#include "resource.h"

#include <string>
#include <fstream>
#include <list>
#include <queue>

static OgreMaxExport* _exp = 0;

INT_PTR CALLBACK ExportPropertiesDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {

	std::string filename;

	switch(message) {
		case WM_INITDIALOG:
			_exp = (OgreMaxExport*) lParam;

			if (_exp == 0) {
				MessageBox(NULL, "Error: Cannot initialize exporter options dialog, aborting", "Error", MB_ICONEXCLAMATION);
				EndDialog(hDlg, 0);
				return TRUE;
			}

			_exp->m_hWndDlgExport = hDlg;
    		
			CenterWindow(hDlg,GetParent(hDlg));

			// initialize controls on the dialog
			EnableWindow(GetDlgItem(hDlg, IDC_CHK_SHARE_SKELETON), FALSE);
			CheckDlgButton(hDlg, IDC_RADIO_EXPORT_SUBMESHES, BST_CHECKED);
			CheckDlgButton(hDlg, IDC_CHK_SHARE_SKELETON, BST_CHECKED);
			CheckDlgButton(hDlg, IDC_CHK_REBUILD_NORMALS, BST_CHECKED);
			CheckDlgButton(hDlg, IDC_RADIO_UV, BST_CHECKED);

			SendMessage(GetDlgItem(hDlg, IDC_TXT_SCALE), WM_SETTEXT, 0, (LPARAM)_T("1.0"));
			SendMessage(GetDlgItem(hDlg, IDC_TXT_DEFAULT_MATERIAL), WM_SETTEXT, 0, (LPARAM)_T("DefaultMaterial"));

			// populate the output directory box
			filename = _exp->m_filename;
			_exp->m_exportPath = filename.substr(0, filename.find_last_of("\\"));
			_exp->m_exportFilename = filename.substr(filename.find_last_of("\\") + 1);

			_exp->m_materialFilename = _exp->m_exportFilename;
			_exp->m_materialFilename = _exp->m_materialFilename.substr(0, _exp->m_materialFilename.find(".mesh.xml")) + ".material";
			SendMessage(GetDlgItem(hDlg, IDC_TXT_MATERIAL_FILENAME), WM_SETTEXT, 0, (LPARAM)_exp->m_materialFilename.c_str());

			SendMessage(GetDlgItem(hDlg, IDC_TXT_EXPORT_DIR), WM_SETTEXT, 0, (LPARAM)_exp->m_exportPath.c_str());
			EnableWindow(GetDlgItem(hDlg, IDC_TXT_EXPORT_DIR), FALSE);
			return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_SELECT_EXPORT_DIR:
					break;
				case IDC_RADIO_EXPORT_FILES:
				case IDC_RADIO_EXPORT_SUBMESHES:
					_exp->updateExportOptions(hDlg);
					break;
				case IDOK:
				case IDC_EXPORT:
					if (_exp->export())
						EndDialog(hDlg, 1);
					else
						EndDialog(hDlg, 2);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, 0);
					return TRUE;
			}
			break;
	}
	return FALSE;

}

void OgreMaxExport::updateExportOptions(HWND hDlg) {

	// ***************************************************************************
	// adjust enabled state of share-skeleton checkbox if the user chose to 
	// export each mesh to an individual file -- this ultimately will instruct the exporter
	// not to create a .skeleton.xml file for each .mesh.xml, and instead assign a
	// common .skeleton filename to each exported .mesh.xml

	HWND hChk = GetDlgItem(hDlg, IDC_CHK_SHARE_SKELETON);
	EnableWindow(hChk, IsDlgButtonChecked(hDlg, IDC_RADIO_EXPORT_FILES));

	m_exportMultipleFiles = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO_EXPORT_FILES));
	m_useSingleSkeleton = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHK_SHARE_SKELETON));
	m_rebuildNormals = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHK_REBUILD_NORMALS));
	m_invertNormals = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHK_INVERT_NORMALS));
	m_flipYZ = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHK_FLIP_YZ));
	m_exportVertexColors = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHK_VERTEX_COLORS));

	if (SendMessage(GetDlgItem(hDlg, IDC_TXT_MATERIAL_FILENAME), WM_GETTEXTLENGTH, 0, 0) == 0) {
		m_exportMaterial = false;
	}
	else {
		m_exportMaterial = true;
	}

	if (SendMessage(GetDlgItem(hDlg, IDC_TXT_SCALE), WM_GETTEXTLENGTH, 0, 0) == 0) 
		m_scale = 1.0f;
	else {
		char buf[16];
		SendMessage(GetDlgItem(hDlg, IDC_TXT_SCALE), WM_GETTEXT, 16, (LPARAM)buf);
		m_scale = atof(buf);

		if (m_scale == 0.0f)
			m_scale = 1.0f;
	}

	TCHAR defMatName[256];
	if (SendMessage(GetDlgItem(hDlg, IDC_TXT_DEFAULT_MATERIAL), WM_GETTEXT, 256, (LPARAM)defMatName) > 0)
		m_defaultMaterialName = defMatName;
	else
		m_defaultMaterialName = _T("DefaultMaterial");

	if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO_UV))
		m_2DTexCoord = UV;
	if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO_VW))
		m_2DTexCoord = VW;
	if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO_WU))
		m_2DTexCoord = WU;
}

OgreMaxExport::OgreMaxExport(HINSTANCE hInst) : m_exportPath(""), m_exportFilename("") {
	m_hInstance = hInst;
	m_hWndDlgExport = 0;
	m_ei = 0;
	m_i = 0;

	m_exportMultipleFiles = true;		// default is to export a file per mesh object in the scene
	m_useSingleSkeleton = true;			// default for multiple meshes is to reference a single .skeleton file where applicable
	m_rebuildNormals = false;			// rebuild the normals before exporting mesh data

	m_exportMaterial = true;			// default is to export material scripts
	m_defaultMaterialName = "DefaultMaterial";
	m_2DTexCoord = UV;					// default is UV interpretation of 2D tex coords

	m_exportOnlySelectedNodes = false;	// this corresponds to the "Export..." vs "Export Selected..." menu items
	m_invertNormals = false;			// flip normals; will also reorder triangle vertex indices
	m_flipYZ = false;					// swap X and Z axes, so that Y becomes the One True Up Vector
	m_exportVertexColors = false;		// useful for vertex painting
	m_scale = 1.0f;						// export at normal size (scale) -- all vertices get multiplied by this
}

OgreMaxExport::~OgreMaxExport() {
}

int OgreMaxExport::ExtCount() {
	// only support one filename extension in this plugin
	return 1;
}

const TCHAR * OgreMaxExport::Ext(int n) {
	switch (n) {
		case 0:
			return _T("xml");
			break;
		default:
			return 0;
			break;
	}
}

const TCHAR * OgreMaxExport::LongDesc() { 
	return _T("Ogre Mesh/Animation/Material Exporter");
}

const TCHAR * OgreMaxExport::ShortDesc() {
	return _T("Ogre XML");
}

const TCHAR * OgreMaxExport::AuthorName() { 
	return _T("Gregory Junker");
}

const TCHAR * OgreMaxExport::CopyrightMessage() { 
	return _T("Clash of Steel (c) 2006");
}

const TCHAR * OgreMaxExport::OtherMessage1() { 
	return 0;
}

const TCHAR * OgreMaxExport::OtherMessage2() { 
	return 0;
}

unsigned int OgreMaxExport::Version() { 
	return 100;
}

void OgreMaxExport::ShowAbout(HWND hWnd) {
	MessageBox(hWnd, "Ogre (Dagon) Mesh, Material and Animation Exporter", "About", 0);
}

int	OgreMaxExport::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options) {

	// massage the filename -- if it does not end with .mesh.xml, make it do so
	m_filename = name;
	if (m_filename.find(".mesh.xml") == std::string::npos) {
		m_filename = m_filename.substr(0, m_filename.find(".XML"));
		m_filename += ".mesh.xml";
	}

	m_ei = ei;
	m_i = i;

	// Max will supply a nonzero (specifically, SCENE_EXPORT_SELECTED) value for options if the user
	// chose "Export Selected..." instead of "Export..." from the File menu
	m_exportOnlySelectedNodes = (options == SCENE_EXPORT_SELECTED);

	int result = DialogBoxParam(m_hInstance,
									MAKEINTRESOURCE(IDD_EXPORT),
									GetActiveWindow(),
									ExportPropertiesDialogProc,
									(LPARAM) this);

	switch (result) {
		case 0:
			return IMPEXP_CANCEL;
			break;
		case 1:
			MessageBox(GetActiveWindow(), "Export Succeeded", "Sucessful Export", MB_ICONINFORMATION);
			return IMPEXP_SUCCESS;
			break;
		default:
			return IMPEXP_FAIL;
			break;
	}
}

BOOL OgreMaxExport::SupportsOptions(int ext, DWORD options) {

	// currently, only SCENE_EXPORT_SELECTED is passed to this; we support exporting
	// of selected files only, so return TRUE (if they ever add anything later, we'll 
	// either support it too, or check what they are asking and return accordingly).
	return TRUE;
}

// pulled directly from the Sparks site: 
// http://sparks.discreet.com/Knowledgebase/sdkdocs_v8/prog/cs/cs_physique_export.html
// Also available in the SDK docs. Used to find out if this node has a physique modifier or not.
// If it does, it returns a pointer to the modifier, and if not, returns NULL. This can be used to 
// determine whether a node is bone or mesh -- mesh nodes will have Physique modifiers, bone nodes
// will not.
Modifier* OgreMaxExport::FindPhysiqueModifier (INode* nodePtr)
{
	// Get object from node. Abort if no object.
	Object* ObjectPtr = nodePtr->GetObjectRef();

	if (!ObjectPtr) return NULL;

	// Is derived object ?
	while (ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID && ObjectPtr)
	{
		// Yes -> Cast.
		IDerivedObject *DerivedObjectPtr = (IDerivedObject *)(ObjectPtr);
						
		// Iterate over all entries of the modifier stack.
		int ModStackIndex = 0;
		while (ModStackIndex < DerivedObjectPtr->NumModifiers())
		{
			// Get current modifier.
			Modifier* ModifierPtr = DerivedObjectPtr->GetModifier(ModStackIndex);

			// Is this Physique ?
			if (ModifierPtr->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B))
			{
				// Yes -> Exit.
				return ModifierPtr;
			}

			// Next modifier stack entry.
			ModStackIndex++;
		}
		ObjectPtr = DerivedObjectPtr->GetObjRef();
	}

	// Not found.
	return NULL;
}

// "callback" is called in response to the EnumTree() call made below. That call visits every node in the 
// scene and calls this procedure for each one. 
int OgreMaxExport::callback(INode *node) {

	// ignore nodes that do not have a Physique modifier - those will be non-mesh nodes
//	if (NULL == FindPhysiqueModifier(node))
//		return TREE_CONTINUE;

	// ignore nodes that are Biped controllers
	Control *c = node->GetTMController();
	if ((c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
		(c->ClassID() == BIPBODY_CONTROL_CLASS_ID) ||
		(c->ClassID() == FOOTPRINT_CLASS_ID)) {
			return TREE_CONTINUE;
	}
	// if the node cannot be converted to a TriObject (mesh), ignore it
	Object *obj = node->EvalWorldState(m_i->GetTime()).obj;
	if (!obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
		return TREE_CONTINUE;

	// create a list of nodes to process
	if (m_exportOnlySelectedNodes) {
		if (node->Selected())
			m_nodeList.push_back(node);
	}
	else {
		m_nodeList.push_back(node);
	}

	return TREE_CONTINUE;
}

bool OgreMaxExport::export() {

	// make sure we have the latest options
	updateExportOptions(m_hWndDlgExport);

	try {
		// all options have been set when actions were taken, so we can just start exporting stuff here
		std::ofstream of;
		bool rtn = true;

		m_nodeList.clear();
		while (!m_submeshNames.empty())
			m_submeshNames.pop();

		m_ei->theScene->EnumTree(this);

		// check to see if there's anything to export
		if (m_nodeList.size() == 0) {
			MessageBox(GetActiveWindow(), "No nodes available to export, aborting...", "Nothing To Export", MB_ICONINFORMATION);
			return false;
		}

		std::string fileName;
		// if we are writing everything to one file, use the name provided when the user first started the export
		if (!m_exportMultipleFiles)
			fileName = m_filename;
		else {
			fileName = m_exportPath + "\\";
			INode *n = *(m_nodeList.begin());
			fileName += n->GetName();
			fileName += ".mesh.xml";
		}

		of.open(fileName.c_str(), std::ios::out);

		if (of.is_open())
			streamFileHeader(of);
		else {
			std::string msg("Could not open output file");
			msg += fileName;
			msg += ", aborting...";
			MessageBox(GetActiveWindow(), msg.c_str(), "File Output Error", MB_ICONEXCLAMATION);
			return false;
		}

		std::list<INode *>::iterator it = m_nodeList.begin();

		Mtl *nodeMtl = (*it)->GetMtl();
		if (nodeMtl == NULL) {
			std::string mtlName;
			mtlName = m_defaultMaterialName;
			m_materialMap.insert(std::map< std::string, Mtl * >::value_type(mtlName, NULL));
		}

		while (it != m_nodeList.end()) {

			// we already filtered out nodes that had NULL materials, and those that could
			// not be converted to TriObjects, so now we know everything we have is good

			INode *node = *it;
			std::string mtlName;
			
			Mtl *mtl = node->GetMtl();
			if (mtl == NULL)
				mtlName = m_defaultMaterialName;
			else
				mtlName = mtl->GetName();

			// duplicate map keys will cause an exception; ignore it and keep going
			try {
				// map a material name to its Mtl pointer so that we can retrieve these later
				std::string::size_type pos;

				// clean out any spaces the user left in their material name
				while ((pos = mtlName.find_first_of(' ')) != std::string::npos)
					mtlName.replace(pos, 1, _T("_"));

				m_materialMap.insert(std::map< std::string, Mtl * >::value_type(mtlName, mtl));
			} catch (...) {}

			Object *obj = node->EvalWorldState(m_i->GetTime()).obj;
			TriObject *tri = (TriObject *) obj->ConvertToType(m_i->GetTime(), Class_ID(TRIOBJ_CLASS_ID, 0));

			if (streamSubmesh(of, tri, mtlName))
				m_submeshNames.push(std::string((*it)->GetName()));

			if (obj != tri)
				delete tri;

			it++;

			// if we are doing one mesh per file, then close this one and open a new one
			if (m_exportMultipleFiles || it == m_nodeList.end()) {
				streamFileFooter(of);
				of.close();

				if (it != m_nodeList.end()) {
					fileName = m_exportPath + "\\";
					INode *n = *it;
					fileName += n->GetName();
					fileName += ".mesh.xml";

					of.open(fileName.c_str(), std::ios::out);

					if (of.is_open())
						streamFileHeader(of);
					else {
						std::string msg("Could not open output file");
						msg += fileName;
						msg += ", aborting...";
						MessageBox(GetActiveWindow(), msg.c_str(), "File Output Error", MB_ICONEXCLAMATION);
						return false;
					}
				}
			}
		}

		// stream material file(s)
		TCHAR fName[256];
		HWND hWnd = GetDlgItem(m_hWndDlgExport, IDC_TXT_MATERIAL_FILENAME);

		SendMessage(hWnd, WM_GETTEXT, 256, (LPARAM)fName);
		
		of.open(fName, std::ios::out);
		of.precision(6);
		of << std::fixed;
		streamMaterial(of);
		of.close();

		return rtn;
	}
	catch (...) {
		MessageBox(GetActiveWindow(), "An unexpected error has occurred while trying to export, aborting", "Error", MB_ICONEXCLAMATION);
		return false;
	}
}

bool OgreMaxExport::streamFileHeader(std::ostream &of) {

	// write the XML header tags
	of << "<?xml version=\"1.0\"?>" << std::endl;
	of << "<mesh>" << std::endl;

	// *************** Export Submeshes ***************
	of << "\t<submeshes>" << std::endl;

	of.precision(6);
	of << std::fixed;

	return true;
}

bool OgreMaxExport::streamFileFooter(std::ostream &of) {

	of << "\t</submeshes>" << std::endl;
	// *************** End Submeshes Export ***********

	// *************** Export Submesh Names ***************
	of << "\t<submeshnames>" << std::endl;

	int idx = 0;
	while (!m_submeshNames.empty()) {
		of << "\t\t<submeshname name=\"" << m_submeshNames.front() << "\" index=\"" << idx << "\" />" << std::endl;
		idx++;
		m_submeshNames.pop();
	}

	of << "\t</submeshnames>" << std::endl;
	// *************** End Submesh Names Export ***********

	of << "</mesh>" << std::endl;

	return true;
}

bool OgreMaxExport::streamPass(std::ostream &of, Mtl *mtl) {
	of << "\t\tpass" << std::endl;
	of << "\t\t{" << std::endl;

	BMM_Color_32 amb32, diff32, spec32, em32;
	ZeroMemory(&amb32, sizeof(BMM_Color_32));
	ZeroMemory(&diff32, sizeof(BMM_Color_32));
	ZeroMemory(&spec32, sizeof(BMM_Color_32));
	ZeroMemory(&em32, sizeof(BMM_Color_32));

	if (mtl != NULL) {
		Color ambient = mtl->GetAmbient();
		amb32 = BMM_Color_32(ambient);

		Color diffuse = mtl->GetDiffuse();
		diff32 = BMM_Color_32(diffuse);

		Color specular = mtl->GetSpecular();
		spec32 = BMM_Color_32(specular);

		Color emissive = mtl->GetSelfIllumColor();
		em32 = BMM_Color_32(emissive);
	}

	of << "\t\t\tambient " << (float)amb32.r/255.0f << " " << (float)amb32.g/255.0f << " " << (float)amb32.b/255.0f << " " << (float)amb32.a/255.0f << std::endl;
	of << "\t\t\tdiffuse " << (float)diff32.r/255.0f << " " << (float)diff32.g/255.0f << " " << (float)diff32.b/255.0f << " " << (float)diff32.a/255.0f << std::endl;
	of << "\t\t\tspecular " << (float)spec32.r/255.0f << " " << (float)spec32.g/255.0f << " " << (float)spec32.b/255.0f << " " << (float)spec32.a/255.0f << " 0.0" << std::endl;
	of << "\t\t\temissive " << (float)em32.r/255.0f << " " << (float)em32.g/255.0f << " " << (float)em32.b/255.0f << " " << (float)em32.a/255.0f << std::endl;

	if (mtl != NULL) {
		// check for diffuse texture
		Texmap *tMap = mtl->GetSubTexmap(ID_DI);
		if (tMap) {
			if (tMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {

				BitmapTex *bmt = (BitmapTex*) tMap;
				std::string mapName(bmt->GetMapName());
				mapName = mapName.substr(mapName.find_last_of('\\') + 1);

				of << "\t\t\ttexture_unit " << std::endl;
				of << "\t\t\t{" << std::endl;
				of << "\t\t\t\ttexture " << mapName << std::endl;
				of << "\t\t\t}" << std::endl;
			}
		}
	}

	of << "\t\t}" << std::endl;

	return true;
}

bool OgreMaxExport::streamMaterial(std::ostream &of) {

	// serialize this information to the material file
	std::map< std::string, Mtl * >::iterator it = m_materialMap.begin();

	while (it != m_materialMap.end()) {
		std::string matName(it->first);
		Mtl *mtl = it->second;

		of << "material " << matName << std::endl;
		of << std::showpoint;
		of << "{" << std::endl;

		of << "\ttechnique" << std::endl;
		of << "\t{" << std::endl;

		int numSubMtl = 0;
		
		if (mtl != NULL) {
			numSubMtl = mtl->NumSubMtls();

			if (numSubMtl > 0) {
				int i;
				for (i=0; i<numSubMtl; i++) {
					streamPass(of, mtl->GetSubMtl(i));
				}
			}
			else
				streamPass(of, mtl);
		}
		else {
			streamPass(of, mtl);
		}

		of << "\t}" << std::endl;
		of << "}" << std::endl;

		it++;
	}

	m_materialMap.clear();


	return true;
}

bool OgreMaxExport::streamSubmesh(std::ostream &of, TriObject *tri, std::string &mtlName) {
	
	int i;
	Mesh mesh = tri->GetMesh();
	
	int vertCount = mesh.getNumVerts();
	int faceCount = mesh.getNumFaces();

	of << "\t\t<submesh ";
	
	if (mtlName.length() > 0)
		of << "material=\"" << mtlName << "\" ";
	
	of << "usesharedvertices=\"false\" use32bitindexes=\"false\">" << std::endl;


	// *************** Export Face List ***************
	of << "\t\t\t<faces count=\"" << faceCount << "\">" << std::endl;
	
	for (i=0; i<faceCount; i++) {
		int v1 = mesh.faces[i].v[0];
		int v2 = mesh.faces[i].v[1];
		int v3 = mesh.faces[i].v[2];

		if (m_invertNormals) {
			int tmp = v2;
			v2 = v3;
			v3 = tmp;
		}

		of << "\t\t\t\t<face v1=\"" << v1 << "\" v2=\"" << v2 << "\" v3=\"" << v3 << "\" />" << std::endl;
	}

	of << "\t\t\t</faces>" << std::endl;
	// *************** End Export Face List ***************


	// *************** Export Geometry ***************
	of << "\t\t\t<geometry vertexcount=\"" << vertCount << "\">" << std::endl;

	// *************** Export Vertex Buffer ***************
	if (m_rebuildNormals) {
		mesh.buildNormals();
	}

	bool exportNormals = (mesh.normalsBuilt > 0);

	of << std::boolalpha;

	// TODO: get the actual number and dimemsion of tex maps from Max -- for now, fake it
	// NB: we don't export tex coords unless we are exporting a material as well
	int numTexMaps = m_exportMaterial ? 1 : 0;
	of << "\t\t\t\t<vertexbuffer positions=\"true\" normals=\"" << exportNormals << "\" colours_diffuse=\"" << m_exportVertexColors << "\" texture_coords=\"" << numTexMaps << "\"";
	
	for (i=0; i<numTexMaps; i++)
		of << " texture_coords_dimensions_" << i << "=\"2\"";
	
	of << ">" << std::endl;

	for (i=0; i<vertCount; i++) {
		Point3 v = mesh.getVert(i);

		Point3 vc;
		vc.x = 0.0f;
		vc.y = 0.0f;
		vc.z = 0.0f;

		if (mesh.vertCol != 0) {
			vc = mesh.vertCol[i];
		}

		of << "\t\t\t\t\t<vertex>" << std::endl;
		of << std::showpoint;

		float x = v.x * m_scale;
		float y = v.y * m_scale;
		float z = v.z * m_scale;

		if (m_flipYZ) {
			float tmp = y;
			y = z;
			z = tmp;
		}

		of << "\t\t\t\t\t\t<position x=\"" << x << "\" y=\"" << y << "\" z=\"" << z << "\" />" << std::endl;

		if (m_exportVertexColors)
			of << "\t\t\t\t\t\t<colour_diffuse value=\"\t" << vc.x << "\t" << vc.y << "\t" << vc.z << "\" />" << std::endl;
		
		if (exportNormals) {
			Point3 n = mesh.getNormal(i);

			float x = n.x;
			float y = n.y;
			float z = n.z;

			if (m_flipYZ) {
				float tmp = y;
				y = z;
				z = tmp;
			}
			
			if (m_invertNormals)
				of << "\t\t\t\t\t\t<normal x=\"" << -x << "\" y=\"" << -y << "\" z=\"" << -z << "\" />" << std::endl;
			else
				of << "\t\t\t\t\t\t<normal x=\"" << x << "\" y=\"" << y << "\" z=\"" << z << "\" />" << std::endl;
		}

		if (i < mesh.getNumTVerts()) {
			for (int t=0; t<numTexMaps; t++) {

				UVVert uv = mesh.getTVert(i);

				switch (m_2DTexCoord) {
					case UV:
						of << "\t\t\t\t\t\t<texcoord u=\"" << uv.x << "\" v=\"" << uv.y << "\" />" << std::endl; 
						break;
					case VW:
						of << "\t\t\t\t\t\t<texcoord v=\"" << uv.y << "\" w=\"" << uv.z << "\" />" << std::endl; 
						break;
					case WU:
						of << "\t\t\t\t\t\t<texcoord w=\"" << uv.z << "\" u=\"" << uv.x << "\" />" << std::endl; 
						break;
				}
			}
		}
		
		of << std::noshowpoint;
		of << "\t\t\t\t\t</vertex>" << std::endl;
	}

	of << "\t\t\t\t</vertexbuffer>" << std::endl;
	// *************** End Export Vertex Buffer ***************

	of << "\t\t\t</geometry>" << std::endl;
	// *************** End Export Geometry ***********

	of << "\t\t</submesh>" << std::endl;

	return true;
}