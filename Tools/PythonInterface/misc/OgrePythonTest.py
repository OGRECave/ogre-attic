# Import the Ogre-to-Python bridge
import Ogre
# Get the root object; MUST BE FIRST
root = Ogre.Root()

root.addResourceLocation("../../../Samples/Media/", "FileSystem", Ogre.ResourceType.RESTYPE_ALL)
root.addResourceLocation("../../../Samples/Media/knot.zip", "Zip", Ogre.ResourceType.RESTYPE_ALL)
root.addResourceLocation("../../../Samples/Media/skybox.zip", "Zip", Ogre.ResourceType.RESTYPE_ALL)
root.addResourceLocation("../../../Samples/Media/dragon.zip", "Zip", Ogre.ResourceType.RESTYPE_ALL)
# Show the config dialog, easy way to get settings
if root.showConfigDialog() == 1:
	# Initialise system, auto-create a window
	win = root.initialise(1)
	# Set mipmap level
	root.getTextureManager().setDefaultNumMipMaps(5)
	# Get a default SceneManager
	sm = root.getSceneManager(Ogre.SceneType.ST_GENERIC)
	# Set up a new material for a skydome
	skyMat = sm.createMaterial("SkyDomeMat")
	skyMat.setLightingEnabled(0)
	tl = skyMat.addTextureLayer("clouds.jpg",0)
	tl.setScrollAnimation(0.15,0)
	# Set up a Quaternion identity for the skydome call
	# Dunno how to do optional params in Python
	qID = Ogre.Quaternion(1,0,0,0)
	sm.setSkyDome(1, "SkyDomeMat", 5, 8, 4000, 1, qID)
	# Create a camera, set details
	cam = sm.createCamera("TestCam")
	cam.setPosition(0,0,500)
	cam.lookAt(0,0,-300)
	# Add a viewport, we could add multiple ones if required
	win.addViewport(cam,0,0,0,100,100)
	# Get the RenderSystem to set the ambient light
	sm.setAmbientLight(Ogre.ColourValue(0.4,0.4,0.4,0.4))
	# Create an entity based on a model file
	ent = sm.createEntity("Ent1","knot.mesh")
	# Override the material a bit
	mat = sm.getMaterial("2 - Default")
	tl = mat.getTextureLayer(0)
	tl.setTextureName("RustySteel.jpg")
	tl = mat.addTextureLayer("spheremap.png",0)
	tl.setEnvironmentMap(1,0)
	tl.setColourOperation(Ogre.LayerBlendOperation.LBO_ADD)
	# Get the root scene node & attach the entity
	rootnode = sm.getRootSceneNode()
	rootnode.attachObject(ent)
	# Get an example frame listener
	list = Ogre.ExampleFrameListener(win,cam)
	root.addFrameListener(list)
	# Start infinite rendering loop
	root.startRendering()
