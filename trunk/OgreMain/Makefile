files := $(shell echo src/*.cpp src/*.c)
objects := $(filter %.o, $(files:src/%.cpp=obj/%.o) ) $(filter %.o, $(files:src/%.c=obj/%.o) )
incdirs := -Iinclude -I$(STLPORT_DIR) $(CUSTOM_INCS)
defines :=
libraries := $(LIBS)

all: $(objects)
	ar rcs lib/libOgreMain.a $(objects)
	$(CXX) -shared -Wl,-soname,lib/libOgreMain.so -o lib/libOgreMain.so $(libraries) $(LDFLAGS) $(objects)

$(objects): obj/%.o: src/%.cpp
	$(CXX) -fPIC -Wall -c $(CFLAGS) $(defines) $(incdirs) $< -o $@

$(objects): obj/%.o: src/%.c
	$(CC) -fPIC -Wall -c $(CFLAGS) $(defines) $(incdirs) $< -o $@

clean:
	@-rm $(objects)

#-----------------------------------------------------------------------------
# This is the CVS log of the file. Do NOT modify beyond this point as this
# may cause inconsistencies between the actual log and what's written here.
# (for more info, see http://www.cvshome.org/docs/manual/cvs_12.html#SEC103 )
#
# $Log: not supported by cvs2svn $
# Revision 1.2  2002/08/22 11:39:06  cearny
# Highly modified, now using make functions for better protability (who knows?) - still got to work on it.
#
#-----------------------------------------------------------------------------