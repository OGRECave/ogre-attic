include ../../../platform-settings

OBJFILES := \
		$(foreach file, $(wildcard *.cpp), $(subst .cpp,.o,$(file)))

all: $(OBJFILES)
	$(CXX) -shared -o OgreSDLEngine.so $(OBJFILES) $(LDFLAGS) $(LIBS) `sdl-config --libs` -lGL -lGLU

.cpp.o:
	$(CXX) $(CFLAGS) `sdl-config --cflags` $(DEFINES) -c $< -o $@

clean:
	rm -f $(OBJFILES) OgreSDLEngine.so
