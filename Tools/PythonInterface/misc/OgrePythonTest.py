# Import the Ogre-to-Python bridge
import OgrePython
# Get the root object; MUST BE FIRST
root = OgrePython.Root()
# Show the config dialog, easy way to get settings
if root.showConfigDialog() == 1:
	# Initialise system, auto-create a window
	win = root.initialise(1)
	# Set mipmap level
	root.getTextureManager().setDefaultNumMipMaps(5)
	# Add resource location
	root.addResourceLocation("ogrepython.zip", "Zip", OgrePython.RESTYPE_ALL)
	# Get a default SceneManager
	sm = root.getSceneManager(OgrePython.ST_GENERIC)
	# Set up a new material for a skydome
	skyMat = sm.createMaterial("SkyDomeMat")
	skyMat.setLightingEnabled(0)
	tl = skyMat.addTextureLayer("clouds.jpg",0)
	tl.setScrollAnimation(0.15,0)
	# Set up a Quaternion identity for the skydome call
	# Dunno how to do optional params in Python
	qID = OgrePython.Quaternion(1,0,0,0)
	sm.setSkyDome(1, "SkyDomeMat", 5, 8, 4000, 1, qID)
	# Create a camera, set details
	cam = sm.createCamera("TestCam")
	cam.setPosition(0,0,500)
	cam.lookAt(0,0,-300)
	# Add a viewport, we could add multiple ones if required
	win.addViewport(cam,0,0,0,100,100)
	# Get the RenderSystem to set the ambient light
	rend = root.getRenderSystem()
	rend.setAmbientLight(0.4,0.4,0.4)
	# Create an entity based on a model file
	ent = sm.createEntity("Ent1","knot.oof")
	# Override the material a bit
	mat = sm.getMaterial("2 - Default")
	tl = mat.getTextureLayer(0)
	tl.setTextureName("RustySteel.jpg")
	tl = mat.addTextureLayer("spheremap.png",0)
	tl.setEnvironmentMap(1,0)
	tl.setColourOperation(OgrePython.LBO_ADD)
	# Get the root scene node & attach the entity
	rootnode = sm.getRootSceneNode()
	rootnode.attachObject(ent)
	# Get an example frame listener
	list = OgrePython.ExampleFrameListener(win,cam)
	root.addFrameListener(list)
	# Start infinite rendering loop
	root.startRendering()
