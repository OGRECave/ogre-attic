INCLUDES = -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Tests/OgreMain/include $(CPPUNIT_CFLAGS)

TESTS = TestSuite
check_PROGRAMS  = TestSuite

TestSuite_SOURCES = main.cpp \
                    ../OgreMain/src/EdgeBuilderTests.cpp \
                    ../OgreMain/src/FileSystemArchiveTests.cpp \
                    ../OgreMain/src/StringTests.cpp \
                    ../OgreMain/src/ZipArchiveTests.cpp

TestSuite_LDFLAGS = -L$(top_srcdir)/OgreMain/src $(CPPUNIT_LIBS)
TestSuite_LDADD = -lOgreMain
