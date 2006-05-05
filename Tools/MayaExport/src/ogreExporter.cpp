
#include "OgreExporter.h"

namespace OgreMayaExporter
{
	void OgreExporter::exit()
	{
		// Restore active selection list
		MGlobal::setActiveSelectionList(m_selList);
		// Restore current time
		MAnimControl::setCurrentTime(m_curTime);
		// Free memory
		if (m_pMesh)
			delete m_pMesh;
		if (m_pMaterialSet)
			delete m_pMaterialSet;
		// Close output files
		m_params.closeFiles();
		std::cout.flush();
	}

	MStatus OgreExporter::doIt(const MArgList& args)
	{
		// Parse the arguments.
		m_params.parseArgs(args);
		
		// Create output files
		m_params.openFiles();

		// Create a new empty mesh
		m_pMesh = new Mesh();

		// Create a new empty material set
		m_pMaterialSet = new MaterialSet();

		// Save current time for later restore
		m_curTime = MAnimControl::currentTime();

		// Save active selection list for later restore
		MGlobal::getActiveSelectionList(m_selList);

		/**************************** LOAD DATA **********************************/
		if (m_params.exportAll)
		{	// we are exporting the whole scene
			std::cout << "Export the whole scene\n";
			std::cout.flush();
			MItDag dagIter;
			MFnDagNode worldDag (dagIter.root());
			MDagPath worldPath;
			worldDag.getPath(worldPath);
			stat = translateNode(worldPath);
		}
		else
		{	// we are translating a selection
			std::cout << "Export selected objects\n";
			std::cout.flush();
			// get the selection list
			MSelectionList activeList;
			stat = MGlobal::getActiveSelectionList(activeList);
			if (MS::kSuccess != stat)
			{
				std::cout << "Error retrieving selection list\n";
				std::cout.flush();
				exit();
				return MS::kFailure;
			}
			MItSelectionList iter(activeList);

			for ( ; !iter.isDone(); iter.next())
			{								
				MDagPath dagPath;
				stat = iter.getDagPath(dagPath);	
				stat = translateNode(dagPath); 
			}							
		}

		// load skeleton animation (do it now, so we have loaded all needed joints)
		if (m_pMesh->getSkeleton() && m_params.exportSkelAnims)
		{
			// Restore skeleton to correct pose
			m_pMesh->getSkeleton()->restorePose();
			// Load skeleton animations
			m_pMesh->getSkeleton()->loadAnims(m_params);
		}

		// load vertex animations
		if (m_params.exportVertAnims)
			m_pMesh->loadAnims(m_params);

		/**************************** WRITE DATA **********************************/
		// write mesh data
		if (m_params.exportMesh)
		{
			std::cout << "Writing mesh data to xml file...\n";
			std::cout.flush();
			stat  = m_pMesh->writeXML(m_params);
			if (stat == MS::kSuccess)
			{
				std::cout << "OK\n";
				std::cout.flush();
			}
			else
			{
				std::cout << "Error writing mesh to XML\n";
				std::cout.flush();
				exit();
				return MS::kFailure;
			}
		}
		// write skeleton data
		if (m_params.exportSkeleton)
		{
			std::cout << "Writing skeleton data to xml file...\n";
			std::cout.flush();
			if (m_pMesh->getSkeleton())
			{
				stat = m_pMesh->getSkeleton()->writeXML(m_params);
				if (stat == MS::kSuccess)
				{
					std::cout << "OK\n";
					std::cout.flush();
				}
				else
				{
					std::cout << "Error writing skeleton to XML\n";
					std::cout.flush();
					exit();
					return MS::kFailure;
				}
			}
			else
			{
				std::cout << "Mesh has no linked skeleton, creating an empty skeleton file\n";
				std::cout.flush();
			}
		}
		// write materials data
		if (m_params.exportMaterial)
		{
			std::cout << "Writing materials data...\n";
			std::cout.flush();
			stat  = m_pMaterialSet->writeXML(m_params);
			if (stat == MS::kSuccess)
			{
				std::cout << "OK\n";
				std::cout.flush();
			}
			else
			{
				std::cout << "Error writing materials file\n";
				std::cout.flush();
				exit();
				return MS::kFailure;
			}
		}

		exit();

		return MS::kSuccess;
	}


	/**************************** TRANSLATE A NODE **********************************/
	// Method for iterating over nodes in a dependency graph from top to bottom
	MStatus OgreExporter::translateNode(MDagPath& dagPath)
	{
		if (m_params.exportAnimCurves)
		{
			MItDependencyGraph animIter( dagPath.node(),
				MFn::kAnimCurve,
				MItDependencyGraph::kUpstream,
				MItDependencyGraph::kDepthFirst,
				MItDependencyGraph::kNodeLevel,
				&stat );

			if (stat)
			{
				for (; !animIter.isDone(); animIter.next())
				{
					MObject anim = animIter.thisNode(&stat);
					MFnAnimCurve animFn(anim,&stat);
					std::cout << "Found animation curve: " << animFn.name().asChar() << "\n";
					std::cout << "Translating animation curve: " << animFn.name().asChar() << "...\n";
					std::cout.flush();
					stat = writeAnim(animFn);
					if (MS::kSuccess == stat)
					{
						std::cout << "OK\n";
						std::cout.flush();
					}
					else
					{
						std::cout << "Error, Aborting operation\n";
						std::cout.flush();
						return MS::kFailure;
					}
				}
			}
		}
		if (dagPath.hasFn(MFn::kMesh)&&(m_params.exportMesh||m_params.exportMaterial||m_params.exportSkeleton)
			&& (dagPath.childCount() == 0))
		{	// we have found a mesh shape node, it can't have any children, and it contains
			// all the mesh geometry data
			MDagPath meshDag = dagPath;
			MFnMesh meshFn(meshDag);
			if (!meshFn.isIntermediateObject())
			{
				std::cout << "Found mesh node: " << meshDag.fullPathName().asChar() << "\n";
				std::cout << "Loading mesh node " << meshDag.fullPathName().asChar() << "...\n";
				std::cout.flush();
				stat = m_pMesh->load(meshDag,m_params);
				if (MS::kSuccess == stat)
				{
					std::cout << "OK\n";
					std::cout.flush();
				}
				else
				{
					std::cout << "Error, mesh skipped\n";
					std::cout.flush();
				}
			}
		}
		else if (dagPath.hasFn(MFn::kCamera)&&(m_params.exportCameras) && (!dagPath.hasFn(MFn::kShape)))
		{	// we have found a camera shape node, it can't have any children, and it contains
			// all information about the camera
			MFnCamera cameraFn(dagPath);
			if (!cameraFn.isIntermediateObject())
			{
				std::cout <<  "Found camera node: "<< dagPath.fullPathName().asChar() << "\n";
				std::cout <<  "Translating camera node: "<< dagPath.fullPathName().asChar() << "...\n";
				std::cout.flush();
				stat = writeCamera(cameraFn);
				if (MS::kSuccess == stat)
				{
					std::cout << "OK\n";
					std::cout.flush();
				}
				else
				{
					std::cout << "Error, Aborting operation\n";
					std::cout.flush();
					return MS::kFailure;
				}
			}
		}
		else if ( ( dagPath.apiType() == MFn::kParticle ) && m_params.exportParticles )
		{	// we have found a set of particles
			MFnDagNode fnNode(dagPath);
			if (!fnNode.isIntermediateObject())
			{
				std::cout <<  "Found particles node: "<< dagPath.fullPathName().asChar() << "\n";
				std::cout <<  "Translating particles node: "<< dagPath.fullPathName().asChar() << "...\n";
				std::cout.flush();
				Particles particles;
				particles.load(dagPath,m_params);
				stat = particles.writeToXML(m_params);
				if (MS::kSuccess == stat)
				{
					std::cout << "OK\n";
					std::cout.flush();
				}
				else
				{
					std::cout << "Error, Aborting operation\n";
					std::cout.flush();
					return MS::kFailure;
				}
			}
		}
		// look for meshes and cameras within the node's children
		for (unsigned int i=0; i<dagPath.childCount(); i++)
		{
			MObject child = dagPath.child(i);
			MDagPath childPath;
			stat = MDagPath::getAPathTo(child,childPath);
			if (MS::kSuccess != stat)
			{
				std::cout << "Error retrieving path to child " << i << " of: " << dagPath.fullPathName().asChar();
				std::cout.flush();
				return MS::kFailure;
			}
			stat = translateNode(childPath);
			if (MS::kSuccess != stat)
				return MS::kFailure;
		}
		return MS::kSuccess;
	}



	/********************************************************************************************************
	*                       Method to translate a single animation curve                                   *
	********************************************************************************************************/
	MStatus OgreExporter::writeAnim(MFnAnimCurve& anim)
	{
		m_params.outAnim << "anim " << anim.name().asChar() << "\n";
		m_params.outAnim <<"{\n";
		m_params.outAnim << "\t//Time   /    Value\n";

		for (unsigned int i=0; i<anim.numKeys(); i++)
			m_params.outAnim << "\t" << anim.time(i).as(MTime::kSeconds) << "\t" << anim.value(i) << "\n";

		m_params.outAnim << "}\n\n";
		return MS::kSuccess;
	}



	/********************************************************************************************************
	*                           Method to translate a single camera                                        *
	********************************************************************************************************/
	MStatus OgreExporter::writeCamera(MFnCamera& camera)
	{
		int i;
		MPlug plug;
		MPlugArray srcplugarray;
		double dist;
		MAngle angle;
		MFnTransform* cameraTransform = NULL;
		MFnAnimCurve* animCurve = NULL;
		// get camera transform
		for (i=0; i<camera.parentCount(); i++)
		{
			if (camera.parent(i).hasFn(MFn::kTransform))
			{
				cameraTransform = new MFnTransform(camera.parent(i));
				continue;
			}
		}
		// start camera description
		m_params.outCameras << "camera " << cameraTransform->partialPathName().asChar() << "\n";
		m_params.outCameras << "{\n";

		//write camera type
		m_params.outCameras << "\ttype ";
		if (camera.isOrtho())
			m_params.outCameras << "ortho\n";
		else
			m_params.outCameras << "persp\n";

		// write translation data
		m_params.outCameras << "\ttranslation\n";
		m_params.outCameras << "\t{\n";
		m_params.outCameras << "\t\tx ";
		plug = cameraTransform->findPlug("translateX");
		if (plug.isConnected() && m_params.exportCamerasAnim)
		{
			plug.connectedTo(srcplugarray,true,false,&stat);
			for (i=0; i < srcplugarray.length(); i++)
			{
				if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
				{
					if (animCurve)
						delete animCurve;
					animCurve = new MFnAnimCurve(srcplugarray[i].node());
					continue;
				}
				else if (i == srcplugarray.length()-1)
				{
					std::cout << "Invalid link to translateX attribute\n";
					return MS::kFailure;
				}
			}
			m_params.outCameras << "anim " << animCurve->name().asChar() << "\n";
		}
		else
		{
			plug.getValue(dist);
			m_params.outCameras << "= " << dist << "\n";
		}
		m_params.outCameras << "\t\ty ";
		plug = cameraTransform->findPlug("translateY");
		if (plug.isConnected() && m_params.exportCamerasAnim)
		{
			plug.connectedTo(srcplugarray,true,false,&stat);
			for (i=0; i< srcplugarray.length(); i++)
			{
				if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
				{
					if (animCurve)
						delete animCurve;
					animCurve = new MFnAnimCurve(srcplugarray[i].node());
					continue;
				}
				else if (i == srcplugarray.length()-1)
				{
					std::cout << "Invalid link to translateY attribute\n";
					return MS::kFailure;
				}
			}
			m_params.outCameras << "anim " << animCurve->name().asChar() << "\n";
		}
		else
		{
			plug.getValue(dist);
			m_params.outCameras << "= " << dist << "\n";
		}
		m_params.outCameras << "\t\tz ";
		plug = cameraTransform->findPlug("translateZ");
		if (plug.isConnected() && m_params.exportCamerasAnim)
		{
			plug.connectedTo(srcplugarray,true,false,&stat);
			for (i=0; i< srcplugarray.length(); i++)
			{
				if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
				{
					if (animCurve)
						delete animCurve;
					animCurve = new MFnAnimCurve(srcplugarray[i].node());
					continue;
				}
				else if (i == srcplugarray.length()-1)
				{
					std::cout << "Invalid link to translateZ attribute\n";
					return MS::kFailure;
				}
			}
			m_params.outCameras << "anim " << animCurve->name().asChar() << "\n";
		}
		else
		{
			plug.getValue(dist);
			m_params.outCameras << "= " << dist << "\n";
		}
		m_params.outCameras << "\t}\n";

		// write rotation data
		m_params.outCameras << "\trotation\n";
		m_params.outCameras << "\t{\n";
		m_params.outCameras << "\t\tx ";
		plug = cameraTransform->findPlug("rotateX");
		if (plug.isConnected() && m_params.exportCamerasAnim)
		{
			plug.connectedTo(srcplugarray,true,false,&stat);
			for (i=0; i< srcplugarray.length(); i++)
			{
				if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
				{
					if (animCurve)
						delete animCurve;
					animCurve = new MFnAnimCurve(srcplugarray[i].node());
					continue;
				}
				else if (i == srcplugarray.length()-1)
				{
					std::cout << "Invalid link to rotateX attribute\n";
					return MS::kFailure;
				}
			}
			m_params.outCameras << "anim " << animCurve->name().asChar() << "\n";
		}
		else
		{
			plug.getValue(angle);
			m_params.outCameras << "= " << angle.asDegrees() << "\n";
		}
		m_params.outCameras << "\t\ty ";
		plug = cameraTransform->findPlug("rotateY");
		if (plug.isConnected() && m_params.exportCamerasAnim)
		{
			plug.connectedTo(srcplugarray,true,false,&stat);
			for (i=0; i< srcplugarray.length(); i++)
			{
				if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
				{
					if (animCurve)
						delete animCurve;
					animCurve = new MFnAnimCurve(srcplugarray[i].node());
					continue;
				}
				else if (i == srcplugarray.length()-1)
				{
					std::cout << "Invalid link to rotateY attribute\n";
					return MS::kFailure;
				}
			}
			m_params.outCameras << "anim " << animCurve->name().asChar() << "\n";
		}
		else
		{
			plug.getValue(angle);
			m_params.outCameras << "= " << angle.asDegrees() << "\n";
		}
		m_params.outCameras << "\t\tz ";
		plug = cameraTransform->findPlug("rotateZ");
		if (plug.isConnected() && m_params.exportCamerasAnim)
		{
			plug.connectedTo(srcplugarray,true,false,&stat);
			for (i=0; i< srcplugarray.length(); i++)
			{
				if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
				{
					if (animCurve)
						delete animCurve;
					animCurve = new MFnAnimCurve(srcplugarray[i].node());
					continue;
				}
				else if (i == srcplugarray.length()-1)
				{
					std::cout << "Invalid link to rotateZ attribute\n";
					return MS::kFailure;
				}
			}
			m_params.outCameras << "anim " << animCurve->name().asChar() << "\n";
		}
		else
		{
			plug.getValue(angle);
			m_params.outCameras << "= " << angle.asDegrees() << "\n";
		}
		m_params.outCameras << "\t}\n";

		// end camera description
		m_params.outCameras << "}\n\n";
		if (cameraTransform != NULL)
			delete cameraTransform;
		if (animCurve != NULL)
			delete animCurve;
		return MS::kSuccess;
	}

} // end namespace
