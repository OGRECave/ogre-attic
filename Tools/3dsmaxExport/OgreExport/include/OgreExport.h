#pragma once
#include <string>
#include <iosfwd>
#include <map>
#include <queue>
#include <list>

class Modifier;

INT_PTR CALLBACK ExportPropertiesDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

class OgreMaxExport : public SceneExport, public ITreeEnumProc { 

	typedef enum {
		UV, 
		VW, 
		WU
	} Tex2D;

	friend INT_PTR CALLBACK ExportPropertiesDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

public:
	OgreMaxExport(HINSTANCE hInst);
	virtual ~OgreMaxExport();
	virtual int ExtCount();					// Number of extemsions supported
	virtual const TCHAR * Ext(int n);					// Extension #n (i.e. "3DS")
	virtual const TCHAR * LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
	virtual const TCHAR * ShortDesc();				// Short ASCII description (i.e. "3D Studio")
	virtual const TCHAR * AuthorName();				// ASCII Author name
	virtual const TCHAR * CopyrightMessage();			// ASCII Copyright message
	virtual const TCHAR * OtherMessage1();			// Other message #1
	virtual const TCHAR * OtherMessage2();			// Other message #2
	virtual unsigned int Version();					// Version number * 100 (i.e. v3.01 = 301)
	virtual void ShowAbout(HWND hWnd);		// Show DLL's "About..." box
	virtual int	DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);	// Export file
	virtual BOOL SupportsOptions(int ext, DWORD options); // Returns TRUE if all option bits set are supported for this extension

	// ITreeEnumProc methods
	int callback(INode *node);
	
protected:
	bool m_exportMultipleFiles;
	bool m_useSingleSkeleton;
	bool m_rebuildNormals;
	bool m_exportMaterial; // alernate is to use default material
	std::string m_defaultMaterialName;
	bool m_invertNormals;
	bool m_flipYZ;
	bool m_exportVertexColors;
	float m_scale;

	Tex2D m_2DTexCoord;
	std::string m_materialFilename;
	std::queue< std::string > m_submeshNames;
	std::map< std::string, Mtl * > m_materialMap;
	std::list <INode *> m_nodeList;

	HINSTANCE m_hInstance;
	HWND m_hWndDlgExport;
	std::string m_filename;
	std::string m_exportPath;
	std::string m_exportFilename;
	bool m_exportOnlySelectedNodes;
	ExpInterface *m_ei;
	Interface *m_i;

	// utility methods
	void updateExportOptions(HWND hDlg);
	bool export();

	bool streamFileHeader(std::ostream &of);
	bool streamFileFooter(std::ostream &of);
	bool streamSubmesh(std::ostream &of, TriObject *tri, std::string &mtlName);
	bool streamMaterial(std::ostream &of);
	bool streamPass(std::ostream &of, Mtl *mtl);
	Modifier *FindPhysiqueModifier (INode* nodePtr);
};

class OgreMaxExportClassDesc : public ClassDesc {
public:
	int IsPublic();
	void * Create(BOOL loading = FALSE);
	const TCHAR * ClassName();
	SClass_ID SuperClassID();
	Class_ID ClassID();
	const TCHAR* Category();
};
