files := $(shell echo src/*.cpp src/*.c)
objects := $(filter %.o, $(files:src/%.cpp=obj/%.o) ) $(filter %.o, $(files:src/%.c=obj/%.o) )
include := -Iinclude -I$(STLPORT_DIR)

all: $(objects)
        echo $(objects)

$(objects): obj/%.o: src/%.cpp
        $(CC) -c $(CFLAGS) -Iinclude -I../STLport-4.5.3/stlport $< -o $@

clean:
	@-rm $(objects)

#SUBDIRS = src
#all:
#	for sd in $(SUBDIRS); do \
#		echo "Building in $$sd"; \
#		(cd $$sd && make all) || exit 1; \
#	done
#clean:
#	for sd in $(SUBDIRS); do \
#		echo "Cleaning in $$sd"; \
#		(cd $$sd && make clean) || exit 1; \
#	done
#

#-----------------------------------------------------------------------------
# This is the CVS log of the file. Do NOT modify beyond this point as this
# may cause inconsistencies between the actual log and what's written here.
# (for more info, see http://www.cvshome.org/docs/manual/cvs_12.html#SEC103 )
#
# $Log: not supported by cvs2svn $
#-----------------------------------------------------------------------------