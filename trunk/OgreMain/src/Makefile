include ../../platform-settings

INCLUDES = -I../include/
OBJFILES := \
		$(foreach file, $(wildcard *.cpp), $(subst .cpp,.o,$(file))) \
		$(foreach file, $(wildcard *.c), $(subst .c,.o,$(file))) 

all: $(OBJFILES)
	ar rcs libOgreMain.a $(OBJFILES) && (mv libOgreMain.a ../lib)
	$(CXX) -shared -Wl,-soname,libOgreMain.so -o libOgreMain.so $(LIBS) $(LDFLAGS) $(OBJFILES) && (mv libOgreMain.so ../lib)

.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDES) -c $(DEFINES) $< -o $@
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $(DEFINES) $< -o $@
clean:
	-rm -f $(OBJFILES)
