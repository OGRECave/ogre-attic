ifeq ("$(shell uname)", "Linux") 
  OS="Linux"
endif

ifeq ("$(shell uname)", "Darwin")
  OS="MacOSX"
endif

# XP
ifeq ("$(shell uname)", "CYGWIN_NT-5.1")
  OS="Windows_NT"
endif

ifndef OS
  $(error Unsupported operating system)
endif 

MAIN=../libnvparse.a
CC=g++
CFLAGS= -DYY_NO_UNPUT -Wall -g \
		-I../../../../OgreMain/include \
		-I../../include \
		-DEXT_HASH -DGCC_3_1

SRCS = nvparse.cpp nvparse_errors.cpp rc1.0_combiners.cpp rc1.0_final.cpp rc1.0_general.cpp ts1.0_inst.cpp ts1.0_inst_list.cpp _ts1.0_parser.cpp _ts1.0_lexer.cpp _rc1.0_lexer.cpp _rc1.0_parser.cpp avp1.0_impl.cpp

ifeq ($(OS), "Linux")
  CFLAGS += -DUNIX
  SRCS += _vs1.0_lexer.cpp _vs1.0_parser.cpp _ps1.0_lexer.cpp _ps1.0_parser.cpp vs1.0_inst.cpp  vs1.0_inst_list.cpp ps1.0_program.cpp vsp1.0_impl.cpp vcp1.0_impl.cpp vp1.0_impl.cpp 
endif

ifeq ($(OS), "Windows_NT")
  CFLAGS += -mno-cygwin 
  SRCS += _vs1.0_lexer.cpp _vs1.0_parser.cpp _ps1.0_lexer.cpp _ps1.0_parser.cpp vs1.0_inst.cpp  vs1.0_inst_list.cpp ps1.0_program.cpp vsp1.0_impl.cpp vcp1.0_impl.cpp vp1.0_impl.cpp 
endif

ifeq ($(OS), "MacOSX")
  CFLAGS += -DMACOS -I../../../../Mac/XCode/OgreFramework $(OTHER_CFLAGS) $(OTHER_CXXFLAGS)
endif

OBJS = $(SRCS:.cpp=.o)

ifeq ($(OS), "Linux")
$(MAIN):
	-bison -d -o _ps1.0_parser.c -p ps10_ ps1.0_grammar.y
	-mv -f _ps1.0_parser.c _ps1.0_parser.cpp
	-bison -d -o _rc1.0_parser.c -p rc10_ rc1.0_grammar.y
	-mv -f _rc1.0_parser.c _rc1.0_parser.cpp
	-bison -d -o _ts1.0_parser.c -p ts10_ ts1.0_grammar.y
	-mv -f _ts1.0_parser.c _ts1.0_parser.cpp
	-bison -d -o _vs1.0_parser.c -p vs10_ vs1.0_grammar.y
	-mv -f _vs1.0_parser.c _vs1.0_parser.cpp
	-flex -Prc10_ -o_rc1.0_lexer.cpp rc1.0_tokens.l
	-flex -Pps10_ -o_ps1.0_lexer.cpp ps1.0_tokens.l
	-flex -Pts10_ -o_ts1.0_lexer.cpp ts1.0_tokens.l
	-flex -Pvs10_ -o_vs1.0_lexer.cpp vs1.0_tokens.l
	make compile
endif

ifeq ($(OS), "Windows_NT")
$(MAIN):
	-bison -d -o _ps1.0_parser.c -p ps10_ ps1.0_grammar.y
	-mv -f _ps1.0_parser.c _ps1.0_parser.cpp
	-bison -d -o _rc1.0_parser.c -p rc10_ rc1.0_grammar.y
	-mv -f _rc1.0_parser.c _rc1.0_parser.cpp
	-bison -d -o _ts1.0_parser.c -p ts10_ ts1.0_grammar.y
	-mv -f _ts1.0_parser.c _ts1.0_parser.cpp
	-bison -d -o _vs1.0_parser.c -p vs10_ vs1.0_grammar.y
	-mv -f _vs1.0_parser.c _vs1.0_parser.cpp
	-flex -Prc10_ -o_rc1.0_lexer.cpp rc1.0_tokens.l
	-flex -Pps10_ -o_ps1.0_lexer.cpp ps1.0_tokens.l
	-flex -Pts10_ -o_ts1.0_lexer.cpp ts1.0_tokens.l
	-flex -Pvs10_ -o_vs1.0_lexer.cpp vs1.0_tokens.l
	make compile
endif

ifeq ($(OS), "MacOSX")
$(MAIN):
	-bison -d -o _rc1.0_parser.c -p rc10_ rc1.0_grammar.y
	-mv -f _rc1.0_parser.c _rc1.0_parser.cpp
	-bison -d -o _ts1.0_parser.c -p ts10_ ts1.0_grammar.y
	-mv -f _ts1.0_parser.c _ts1.0_parser.cpp
	-flex -Prc10_ -o_rc1.0_lexer.cpp rc1.0_tokens.l
	-flex -Pts10_ -o_ts1.0_lexer.cpp ts1.0_tokens.l
	make compile
endif

all: compile

compile: $(OBJS)
	$(AR) -rs $(MAIN) $(OBJS)

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	rm -f *.o *~ $(MAIN)

install:
