#ifndef _MATERIALEDITORAPP_H_
#define _MATERIALEDITORAPP_H_

#include <wx/wx.h>

#include "OgreRoot.h"

using Ogre::Root;

class MaterialEditorApp : public wxApp
{
public:
	virtual ~MaterialEditorApp();

private:
	virtual bool OnInit();

	Root* mRoot;
};

IMPLEMENT_APP(MaterialEditorApp)

#endif // _MATERIALEDITORAPP_H_