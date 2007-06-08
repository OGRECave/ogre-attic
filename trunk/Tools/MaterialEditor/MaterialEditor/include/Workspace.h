#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <list>

#include "OgreSingleton.h"

class Project;

typedef std::list<Project*> ProjectList;

class Workspace : public Ogre::Singleton<Workspace>
{
public:
	Workspace();
	virtual ~Workspace();

	void addProject(Project* project);
	void removeProject(Project* project);

	Project* getProject(const Ogre::String& name);
	const ProjectList* getProjects() const;

	/** Override standard Singleton retrieval.
	@remarks
	Why do we do this? Well, it's because the Singleton
	implementation is in a .h file, which means it gets compiled
	into anybody who includes it. This is needed for the
	Singleton template to work, but we actually only want it
	compiled into the implementation of the class based on the
	Singleton, not all of them. If we don't change this, we get
	link errors when trying to use the Singleton-based class from
	an outside dll.
	@par
	This method just delegates to the template version anyway,
	but the implementation stays in this single compilation unit,
	preventing link errors.
	*/
	static Workspace& getSingleton(void);
	/** Override standard Singleton retrieval.
	@remarks
	Why do we do this? Well, it's because the Singleton
	implementation is in a .h file, which means it gets compiled
	into anybody who includes it. This is needed for the
	Singleton template to work, but we actually only want it
	compiled into the implementation of the class based on the
	Singleton, not all of them. If we don't change this, we get
	link errors when trying to use the Singleton-based class from
	an outside dll.
	@par
	This method just delegates to the template version anyway,
	but the implementation stays in this single compilation unit,
	preventing link errors.
	*/
	static Workspace* getSingletonPtr(void);

protected:
	ProjectList mProjects;
};

#endif // WORKSPACE_H