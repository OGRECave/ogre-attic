include ../../../platform-settings

INCLUDES = -I../include
OBJFILES := \
		$(foreach file, $(wildcard *.cpp), $(subst .cpp,.o,$(file))) \
		$(foreach file, $(wildcard *.c), $(subst .c,.o,$(file)))

all: $(OBJFILES)
	$(CXX) -shared -o FileSystem.so $(OBJFILES) $(LDFLAGS) $(LIBS)

.cpp.o:
	$(CXX) -g $(CFLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

clean:
	rm -f $(OBJFILES)
