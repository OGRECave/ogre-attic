# Import the Ogre-to-Python bridge
import Ogre

class FL(Ogre.FrameListener, Ogre.KeyListener):
    def __init__(self, win, cam):
        Ogre.FrameListener.__init__(self)
        Ogre.KeyListener.__init__(self)
        self.input_device = Ogre.PlatformManager.getSingleton().createInputReader()
        self.input_device.initialise(win, 1, 1)
        self.camera = cam
        self.window = win

    def cleanup(self):
        Ogre.PlatformManager.getSingleton().destroyInputReader(self.input_device)
    def frameStarted(self, evt):
        self.input_device.capture()
        if self.input_device.isKeyDown(Ogre.KeyCode.KC_ESCAPE):
            self.cleanup()
            return 0
        return 1

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
    # Create a camera, set details
    cam = sm.createCamera("TestCam")
    cam.setPosition(0,0,500)
    cam.lookAt(0,0,-300)
    cam.setNearClipDistance(5)
    # Add a viewport, we could add multiple ones if required
    vp = win.addViewport(cam)
    vp.setBackgroundColour(Ogre.ColourValue(0.0, 0.0, 0.0))
    # Get the RenderSystem to set the ambient light
    sm.setAmbientLight(Ogre.ColourValue(0.5, 0.5, 0.5))
    # add a light and an environment mapped entity
    l = sm.createLight("MainLight")
    l.setPosition(20, 80, 50)
    ent = sm.createEntity("head", "ogrehead.mesh")
    ent.setMaterialName("Examples/EnvMappedRustySteel")

    # Get the root scene node & attach the entity
    node = Ogre.castNodeToSceneNode(sm.getRootSceneNode().createChild())
    node.attachObject(ent)
    # Get an example frame listener
    #list = Ogre.ExampleFrameListener(win,cam)
    # Get our overridden frame listener
    list = FL(win, cam)
    root.addFrameListener(list)
    root.showDebugOverlay(1)
    # Start infinite rendering loop
    root.startRendering()
