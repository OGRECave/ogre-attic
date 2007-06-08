#include "Workspace.h"

#include "Project.h"

template<> Workspace* Ogre::Singleton<Workspace>::ms_Singleton = 0;

Workspace& Workspace::getSingleton(void)
{  
	assert( ms_Singleton );  return ( *ms_Singleton );  
}

Workspace* Workspace::getSingletonPtr(void)
{
	return ms_Singleton;
}

Workspace::Workspace()
{

}

Workspace::~Workspace()
{
	ProjectList::iterator it;
	for(it = mProjects.begin(); it != mProjects.end(); ++it)
	{
		delete *it;
	}
}

void Workspace::addProject(Project* project)
{
	mProjects.push_back(project);
}

void Workspace::removeProject(Project* project)
{
	mProjects.remove(project);
}

Project* Workspace::getProject(const String& name)
{
	Project* p;
	ProjectList::iterator it;
	for(it = mProjects.begin(); it != mProjects.end(); ++it)
	{
		p = *it;
		if(p->getName() == name) return p;
	}

	return NULL;
}

const ProjectList* Workspace::getProjects() const
{
	return &mProjects;
}