include ../../../platform-settings

INCLUDES = -I../include

OBJFILES := \
		$(foreach file, $(wildcard *.cpp), $(subst .cpp,.o,$(file)))

all: $(OBJFILES)
	$(CXX) -shared -o OgreSDLPlatform.so $(OBJFILES) $(LDFLAGS) $(LIBS) `sdl-config --libs`

.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDES) `sdl-config --cflags` $(DEFINES) -c $< -o $@

clean:
	rm -f $(OBJFILES) OgreSDLPlatform.so
