#!/usr/bin/python

from erlang_ext import *
import types
import pprint

import io3d_wings
import io3d_ogre

def conv(infile, outfile):
	obj = io3d_wings.read_wings(infile)
	obj.scale(10)
	io3d_ogre.write_ogre(obj, outfile)

if __name__ == "__main__":

	if len(sys.argv) > 1:

		for srcname in sys.argv[1:]:

			if srcname[-6:] == ".wings":
				dstname = srcname[:-6]
			else:
				dstname = srcname
			dstname += ".mesh.xml"
			conv(srcname, dstname)

	else:
		conv("uv-cube.wings", "uv-cube.mesh.xml")

