INCLUDES=-I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Tools/XMLConverter/include
bin_PROGRAMS=XMLConverter
XMLConverter_SOURCES= OgreXMLMeshSerializer.cpp \
                      OgreXMLSkeletonSerializer.cpp \
                      main.cpp \
                      tinystr.cpp \
                      tinyxml.cpp \
                      tinyxmlerror.cpp \
                      tinyxmlparser.cpp
XMLConverter_LDFLAGS= -L$(top_srcdir)/OgreMain/src
XMLConverter_LDADD= -lOgreMain
