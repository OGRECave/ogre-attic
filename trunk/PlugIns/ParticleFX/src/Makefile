include ../../../platform-settings

OBJFILES := \
		$(foreach file, $(wildcard *.cpp), $(subst .cpp,.o,$(file))) \
		$(foreach file, $(wildcard *.c), $(subst .c,.o,$(file))) 

all: $(OBJFILES)
	$(CXX) -shared -o ParticleFX.so $(OBJFILES) $(LDFLAGS) $(LIBS)

.cpp.o:
	$(CXX) $(CFLAGS) -c $(DEFINES) $< -o $@
.c.o:
	$(CC) $(CFLAGS) -c $(DEFINES) $< -o $@
clean:
	rm -f $(OBJFILES) ParticleFX.so
