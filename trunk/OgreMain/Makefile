files := $(shell echo src/*.cpp src/*.c)
cxx_objects := $(filter %.o, $(files:src/%.cpp=obj/%.o) ) 
cc_objects := $(filter %.o, $(files:src/%.c=obj/%.o) )
headers := $(shell echo include/*.h)
incdirs := -Iinclude $(CUSTOM_INCS)
defines :=
libraries := $(LIBS)

all: $(cc_objects) $(cxx_objects)
	ar rcs lib/libOgreMain.a $(objects)
	$(CXX) -shared -Wl,-soname,lib/libOgreMain.so -o lib/libOgreMain.so $(libraries) $(LDFLAGS) $(cxx_objects) $(cc_objects)

$(cxx_objects): obj/%.o: src/%.cpp $(headers)
	$(CXX) -fPIC -Wall -c $(CFLAGS) $(defines) $(incdirs) $< -o $@

$(cc_objects): obj/%.o: src/%.c $(headers)
	$(CC) -fPIC -Wall -c $(CFLAGS) $(defines) $(incdirs) $< -o $@

clean:
	@-rm $(cxx_objects) $(cc_objects)

#-----------------------------------------------------------------------------
# This is the CVS log of the file. Do NOT modify beyond this point as this
# may cause inconsistencies between the actual log and what's written here.
# (for more info, see http://www.cvshome.org/docs/manual/cvs_12.html#SEC103 )
#
# $Log: not supported by cvs2svn $
# Revision 1.3  2002/08/22 12:49:07  cearny
# Some more Linux work.
#
# Revision 1.2  2002/08/22 11:39:06  cearny
# Highly modified, now using make functions for better protability (who knows?) - still got to work on it.
#
#-----------------------------------------------------------------------------