INCLUDES=-I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Tools/XMLConverter/include
bin_PROGRAMS=OgreXMLConverter
OgreXMLConverter_SOURCES= OgreXMLMeshSerializer.cpp \
                      OgreXMLSkeletonSerializer.cpp \
                      main.cpp \
                      tinystr.cpp \
                      tinyxml.cpp \
                      tinyxmlerror.cpp \
                      tinyxmlparser.cpp
OgreXMLConverter_LDFLAGS= -L$(top_srcdir)/OgreMain/src
OgreXMLConverter_LDADD= -lOgreMain
