#ifndef PROJECT_H
#define PROJECT_H

#include "OgreString.h"

namespace Ogre
{
	class Material;
	class SceneManager;
}

using Ogre::Material;
using Ogre::String;

typedef std::list<Material*> MaterialList;

class Project
{
public:
	Project();
	virtual ~Project();

	const String& getName() const;
	void setName(const String& name);

	Material* getActiveMaterial() const;
	void setActiveMaterial(Material* material);
	void setActiveMaterial(const String& name);

	void open();
	void close();

	bool isOpen();
	bool isClosed();

	void generateScene(Ogre::SceneManager* sceneManager);

protected:
	String mName;
	bool mOpen;
	Material* mActiveMaterial;
};

#endif // PROJECT_H