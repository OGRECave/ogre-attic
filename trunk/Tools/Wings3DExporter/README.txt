
WHAT

This is wings-ogre, my Wings3D to OGRE converter written in Python.

Please send feedback, suggestions and bug reports to:

Attila Tajti attis@spacehawks.hu


USAGE

Copy the .wings file to the directory containing the python files. Then
execute ./w2o.py <files> from the command line, eg:

	% tar -xzf wings-ogre-0.9.tar.gz
	% cd wings-ogre-0.9
	% cp ~/models/test.wings
	% ./w2o.py test.wings

This will result in a .mesh.xml file. It has to be converted to a .mesh 
via the the XML converter:

	% OgreXMLConverter test.mesh.xml

The w2o.py program will also export internal Wings textures, using the name
of the material (not the name of the image) as filename.


REQUIREMENTS

(These should be available on a recent Linux system)

Python 2.2
Python Imaging Library (for Image export)
libxml2 (see link below)


LINKS

http://www.wings3d.org - Wings 3D homepage

http://www.python.org - Python homepage

http://www.pythonware.com/products/pil/ - Python Imaging Library homepage

http://users.skynet.be/sbi/libxml-python/ - Windows libxml binaries

