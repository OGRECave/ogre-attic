include ../../../platform-settings

INCLUDES = -I../include

OBJFILES := \
		$(foreach file, $(wildcard *.cpp), $(subst .cpp,.o,$(file)))

all: $(OBJFILES)
	$(CXX) -shared -o BSPSceneManager.so $(OBJFILES) $(LDFLAGS) $(LIBS)

.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

clean:
	rm -f $(OBJFILES) OgreBSDSceneManager.so
