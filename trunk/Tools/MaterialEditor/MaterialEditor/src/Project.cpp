#include "Project.h"

#include "OgreSceneManager.h"
#include "OgreMaterial.h"
#include "OgreMaterialManager.h"

using Ogre::MaterialManager;

Project::Project() : mActiveMaterial(NULL)
{

}

Project::~Project()
{

}

const String& Project::getName() const
{
	return mName;
}

void Project::setName(const String& name)
{
	mName = name;
}

Material* Project::getActiveMaterial() const
{
	return mActiveMaterial;
}

void Project::setActiveMaterial(Material* material)
{
	if(material == mActiveMaterial) return;

	mActiveMaterial = material;
}

void Project::setActiveMaterial(const String& name)
{
	//Material* m = (Material*)MaterialManager::getSingletonPtr()->getByName(name);
	//if(m == NULL) return; // ERROR

	//mActiveMaterial = m;
}

void Project::open()
{

}

void Project::close()
{

}

void Project::generateScene(Ogre::SceneManager* sceneManager)
{

}
