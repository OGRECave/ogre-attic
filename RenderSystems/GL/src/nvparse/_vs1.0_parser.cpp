/* A Bison parser, made from vs1.0_grammar.y
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse vs10_parse
#define yylex vs10_lex
#define yyerror vs10_error
#define yylval vs10_lval
#define yychar vs10_char
#define yydebug vs10_debug
#define yynerrs vs10_nerrs
# define	VERTEX_SHADER	257
# define	ADD_INSTR	258
# define	DP3_INSTR	259
# define	DP4_INSTR	260
# define	DST_INSTR	261
# define	EXP_INSTR	262
# define	EXPP_INSTR	263
# define	FRC_INSTR	264
# define	LIT_INSTR	265
# define	LOG_INSTR	266
# define	LOGP_INSTR	267
# define	M3X2_INSTR	268
# define	M3X3_INSTR	269
# define	M3X4_INSTR	270
# define	M4X3_INSTR	271
# define	M4X4_INSTR	272
# define	MAD_INSTR	273
# define	MAX_INSTR	274
# define	MIN_INSTR	275
# define	MOV_INSTR	276
# define	MUL_INSTR	277
# define	NOP_INSTR	278
# define	RCP_INSTR	279
# define	RSQ_INSTR	280
# define	SGE_INSTR	281
# define	SLT_INSTR	282
# define	SUB_INSTR	283
# define	ILLEGAL	284
# define	UNKNOWN_STRING	285
# define	INTVAL	286
# define	REGISTER	287
# define	XYZW_MODIFIER	288
# define	COMMENT	289

#line 2 "vs1.0_grammar.y"

void yyerror(char *s);
int yylex(void);

#include <math.h>
#include <string>

#include <stdlib.h>
#include "vs1.0_inst_list.h"
#include "nvparse_errors.h"
#include "nvparse_externs.h"

//extern bool gbTempInsideMacro;
//extern unsigned int &base_linenumber;
void LexError(char *format, ...);
extern int line_incr;

#define do_linenum_incr()		{ line_number+=line_incr; line_incr = 0; }
//int get_linenum()			{ return( gbTempInsideMacro ? base_linenumber : line_number ); }
int get_linenum()			{ return( line_number ); }

#define YYDEBUG 1


#line 26 "vs1.0_grammar.y"
#ifndef YYSTYPE
typedef union {
  int ival;
  unsigned int lval;
  float fval;
  char mask[4];
  char *comment;
  VS10Reg reg;
  VS10InstPtr inst;
  VS10InstListPtr instList;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		103
#define	YYFLAG		-32768
#define	YYNTBASE	44

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 289 ? yytranslate[x] : 61)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      36,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    43,    37,    38,    39,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    41,     2,    42,     2,     2,     2,     2,     2,    40,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     2,     5,     7,     9,    11,    13,    15,    17,
      19,    21,    23,    25,    27,    32,    37,    41,    44,    46,
      48,    50,    55,    62,    71,    76,    83,    89,    97,   105,
     115,   120,   125,   132,   141,   143,   145,   147,   149,   151,
     153,   155,   157,   159,   161,   163,   165,   167,   169,   171,
     173,   175,   177,   179,   181,   183,   185,   187,   189
};
static const short yyrhs[] =
{
      45,     0,    45,    46,     0,    46,     0,    47,     0,    36,
       0,    48,     0,    52,     0,    53,     0,    54,     0,    55,
       0,    24,     0,    35,     0,     3,     0,    56,    49,    37,
      49,     0,    38,    50,    39,    34,     0,    50,    39,    34,
       0,    38,    50,     0,    50,     0,    33,     0,    51,     0,
      40,    41,    32,    42,     0,    40,    41,    33,    39,    34,
      42,     0,    40,    41,    33,    39,    34,    43,    32,    42,
       0,    40,    41,    33,    42,     0,    40,    41,    33,    43,
      32,    42,     0,    40,    41,    38,    33,    42,     0,    40,
      41,    38,    33,    43,    32,    42,     0,    40,    41,    38,
      33,    39,    34,    42,     0,    40,    41,    38,    33,    39,
      34,    43,    32,    42,     0,    57,    49,    37,    49,     0,
      58,    49,    37,    49,     0,    59,    49,    37,    49,    37,
      49,     0,    60,    49,    37,    49,    37,    49,    37,    49,
       0,    22,     0,    11,     0,    25,     0,    26,     0,     8,
       0,     9,     0,    12,     0,    13,     0,    10,     0,    23,
       0,     4,     0,     5,     0,     6,     0,     7,     0,    21,
       0,    20,     0,    28,     0,    27,     0,    14,     0,    15,
       0,    16,     0,    17,     0,    18,     0,    29,     0,    19,
       0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,    95,   103,   109,   121,   126,   133,   134,   135,   136,
     137,   138,   142,   146,   152,   158,   168,   178,   188,   199,
     199,   203,   210,   245,   281,   286,   291,   296,   301,   306,
     313,   319,   325,   331,   338,   342,   348,   352,   356,   360,
     364,   368,   374,   380,   384,   388,   392,   396,   400,   404,
     408,   412,   416,   420,   424,   428,   432,   436,   442
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "VERTEX_SHADER", "ADD_INSTR", "DP3_INSTR", 
  "DP4_INSTR", "DST_INSTR", "EXP_INSTR", "EXPP_INSTR", "FRC_INSTR", 
  "LIT_INSTR", "LOG_INSTR", "LOGP_INSTR", "M3X2_INSTR", "M3X3_INSTR", 
  "M3X4_INSTR", "M4X3_INSTR", "M4X4_INSTR", "MAD_INSTR", "MAX_INSTR", 
  "MIN_INSTR", "MOV_INSTR", "MUL_INSTR", "NOP_INSTR", "RCP_INSTR", 
  "RSQ_INSTR", "SGE_INSTR", "SLT_INSTR", "SUB_INSTR", "ILLEGAL", 
  "UNKNOWN_STRING", "INTVAL", "REGISTER", "XYZW_MODIFIER", "COMMENT", 
  "'\\n'", "','", "'-'", "'.'", "'c'", "'['", "']'", "'+'", "VS10Program", 
  "InstSequence", "InstLine", "Instruction", "VECTORopinstruction", 
  "genericReg", "genReg", "constantReg", "SCALARopinstruction", 
  "UNARYopinstruction", "BINopinstruction", "TRIopinstruction", 
  "VECTORop", "SCALARop", "UNARYop", "BINop", "TRIop", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    44,    45,    45,    46,    46,    47,    47,    47,    47,
      47,    47,    47,    47,    48,    49,    49,    49,    49,    50,
      50,    51,    51,    51,    51,    51,    51,    51,    51,    51,
      52,    53,    54,    55,    56,    56,    57,    57,    57,    57,
      57,    57,    58,    59,    59,    59,    59,    59,    59,    59,
      59,    59,    59,    59,    59,    59,    59,    59,    60
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     4,     4,     3,     2,     1,     1,
       1,     4,     6,     8,     4,     6,     5,     7,     7,     9,
       4,     4,     6,     8,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       0,    13,    44,    45,    46,    47,    38,    39,    42,    35,
      40,    41,    52,    53,    54,    55,    56,    58,    49,    48,
      34,    43,    11,    36,    37,    51,    50,    57,    12,     5,
       1,     3,     4,     6,     7,     8,     9,    10,     0,     0,
       0,     0,     0,     2,    19,     0,     0,     0,    18,    20,
       0,     0,     0,     0,    17,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,    16,    30,    31,
       0,     0,    15,    21,     0,    24,     0,     0,     0,     0,
       0,     0,     0,    26,     0,    32,     0,    22,     0,    25,
       0,     0,     0,     0,    28,     0,    27,    33,    23,     0,
      29,     0,     0,     0
};

static const short yydefgoto[] =
{
     101,    30,    31,    32,    33,    47,    48,    49,    34,    35,
      36,    37,    38,    39,    40,    41,    42
};

static const short yypact[] =
{
      51,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
      51,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   -29,   -29,
     -29,   -29,   -29,-32768,-32768,   -28,   -35,   -23,   -21,-32768,
     -13,    -5,    -4,    -3,    -2,   -25,   -29,   -24,   -29,   -29,
     -29,   -29,     1,    -6,   -16,     5,-32768,-32768,-32768,-32768,
       4,     6,-32768,-32768,     8,-32768,    12,   -14,   -29,   -29,
     -27,     3,    13,-32768,    14,-32768,    11,-32768,    17,-32768,
     -12,     9,   -29,    10,-32768,    18,-32768,-32768,-32768,    39,
  -32768,    82,    83,-32768
};

static const short yypgoto[] =
{
  -32768,-32768,    54,-32768,-32768,   -39,    40,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768
};


#define	YYLAST		87


static const short yytable[] =
{
      50,    51,    52,    53,    44,    44,    55,    63,    64,    45,
      67,    46,    46,    65,    56,    87,    88,    66,    57,    68,
      69,    70,    71,    74,    58,    82,    75,    76,    83,    84,
      94,    95,    59,    60,    61,    72,    73,    62,    77,    85,
      86,    78,    80,    79,    81,    89,    91,    90,    92,    93,
      99,    96,    98,    97,     1,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,   100,   102,   103,    43,    54,    28,    29
};

static const short yycheck[] =
{
      39,    40,    41,    42,    33,    33,    41,    32,    33,    38,
      34,    40,    40,    38,    37,    42,    43,    56,    39,    58,
      59,    60,    61,    39,    37,    39,    42,    43,    42,    43,
      42,    43,    37,    37,    37,    34,    42,    39,    33,    78,
      79,    37,    34,    37,    32,    42,    32,    34,    37,    32,
      32,    42,    42,    92,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    42,     0,     0,    30,    45,    35,    36
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#ifdef YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif
#endif

#line 315 "/usr/share/bison/bison.simple"


/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 1:
#line 96 "vs1.0_grammar.y"
{
	yyvsp[0].instList->Validate();
	yyvsp[0].instList->Translate();
	delete yyvsp[0].instList;
	;
    break;}
case 2:
#line 104 "vs1.0_grammar.y"
{
		*(yyvsp[-1].instList) += yyvsp[0].inst;
		delete yyvsp[0].inst;
		yyval.instList = yyvsp[-1].instList
	;
    break;}
case 3:
#line 110 "vs1.0_grammar.y"
{
 		VS10InstListPtr instList = new VS10InstList;
		if ( yyvsp[0].inst != NULL )
			{
			*instList += yyvsp[0].inst;
			delete yyvsp[0].inst;
			}
		yyval.instList = instList;
	;
    break;}
case 4:
#line 122 "vs1.0_grammar.y"
{
		yyval.inst = yyvsp[0].inst;
		do_linenum_incr();
	;
    break;}
case 5:
#line 127 "vs1.0_grammar.y"
{
		yyval.inst = new VS10Inst( get_linenum() );
		do_linenum_incr();
	;
    break;}
case 11:
#line 139 "vs1.0_grammar.y"
{
		   yyval.inst = new VS10Inst( get_linenum(), VS10_NOP );
		   ;
    break;}
case 12:
#line 143 "vs1.0_grammar.y"
{
		   yyval.inst = new VS10Inst( get_linenum(), VS10_COMMENT, yyvsp[0].comment );
		   ;
    break;}
case 13:
#line 147 "vs1.0_grammar.y"
{
		   yyval.inst = new VS10Inst( get_linenum(), VS10_HEADER );
		   ;
    break;}
case 14:
#line 153 "vs1.0_grammar.y"
{
		yyval.inst = new VS10Inst( get_linenum(), yyvsp[-3].ival, yyvsp[-2].reg, yyvsp[0].reg );
	;
    break;}
case 15:
#line 159 "vs1.0_grammar.y"
{
		   VS10Reg reg;
		   reg = yyvsp[-2].reg;
		   reg.sign = -1;
		   reg.type = yyvsp[-2].reg.type;
		   reg.index = yyvsp[-2].reg.index;
		   for ( int i = 0; i < 4; i++ ) reg.mask[i] = yyvsp[0].mask[i];
		   yyval.reg = reg;
		   ;
    break;}
case 16:
#line 169 "vs1.0_grammar.y"
{
		   VS10Reg reg;
		   reg = yyvsp[-2].reg;
		   reg.sign = 1;
		   reg.type = yyvsp[-2].reg.type;
		   reg.index = yyvsp[-2].reg.index;
		   for ( int i = 0; i < 4; i++ ) reg.mask[i] = yyvsp[0].mask[i];
		   yyval.reg = reg;
		   ;
    break;}
case 17:
#line 179 "vs1.0_grammar.y"
{
		   VS10Reg reg;
		   reg = yyvsp[0].reg;
		   reg.sign = -1;
		   reg.type = yyvsp[0].reg.type;
		   reg.index = yyvsp[0].reg.index;
		   for ( int i = 0; i < 4; i++ ) reg.mask[i] = 0;
		   yyval.reg = reg;
		   ;
    break;}
case 18:
#line 189 "vs1.0_grammar.y"
{
		   VS10Reg reg;
		   reg = yyvsp[0].reg;
		   reg.sign = 1;
		   reg.type = yyvsp[0].reg.type;
		   reg.index = yyvsp[0].reg.index;
		   for ( int i = 0; i < 4; i++ ) reg.mask[i] = 0;
		   yyval.reg = reg;
		   ;
    break;}
case 20:
#line 200 "vs1.0_grammar.y"
{
	  ;
    break;}
case 21:
#line 204 "vs1.0_grammar.y"
{
		   VS10Reg reg;
		   reg.type = TYPE_CONSTANT_MEM_REG;
		   reg.index = yyvsp[-1].ival;
		   yyval.reg = reg;
		   ;
    break;}
case 22:
#line 211 "vs1.0_grammar.y"
{
		   // Register is valid only if
		   //	type = TYPE_ADDRESS_REG
		   //	index = 0
		   //	len(mask) = 1
		   //	mask[0] = 'x'
		   VS10Reg reg;
		   yyval.reg.type = TYPE_CONSTANT_A0_REG;
		   if ( yyvsp[-3].reg.type != TYPE_ADDRESS_REG )
		       {
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
			   }
		       else if ( yyvsp[-3].reg.index != 0 )
			   {
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
			   }
			   else
			   {
			       int len = 0;
				   while ( len < 2 )
				   {
				       if ( yyvsp[-1].mask[len] == 0 )
				           break;
					   len++;
				   }
				   if ( len != 1 || yyvsp[-1].mask[0] != 'x' )
				   {
			       LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
				   }

				   reg.type = TYPE_CONSTANT_A0_REG;
				   yyval.reg = reg;
			   }
		   ;
    break;}
case 23:
#line 246 "vs1.0_grammar.y"
{
		   // Register is valid only if
		   //	type = TYPE_ADDRESS_REG
		   //	index = 0
		   //	len(mask) = 1
		   //	mask[0] = 'x'
		   VS10Reg reg;
		   yyval.reg.type = TYPE_CONSTANT_A0_OFFSET_REG;
		   if ( yyvsp[-5].reg.type != TYPE_ADDRESS_REG )
		       {
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
			   }
		       else if ( yyvsp[-5].reg.index != 0 )
			   {
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
			   }
			   else
			   {
			       int len = 0;
				   while ( len < 2 )
				   {
				       if ( yyvsp[-3].mask[len] == 0 )
				           break;
					   len++;
				   }
				   if ( len != 1 || yyvsp[-3].mask[0] != 'x' )
				   {
			       LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
				   }

				   reg.type = TYPE_CONSTANT_A0_OFFSET_REG;
				   reg.index = yyvsp[-1].ival;
				   yyval.reg = reg;
			   }
		   ;
    break;}
case 24:
#line 282 "vs1.0_grammar.y"
{
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;
    break;}
case 25:
#line 287 "vs1.0_grammar.y"
{
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;
    break;}
case 26:
#line 292 "vs1.0_grammar.y"
{
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;
    break;}
case 27:
#line 297 "vs1.0_grammar.y"
{
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;
    break;}
case 28:
#line 302 "vs1.0_grammar.y"
{
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;
    break;}
case 29:
#line 307 "vs1.0_grammar.y"
{
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;
    break;}
case 30:
#line 314 "vs1.0_grammar.y"
{
		yyval.inst = new VS10Inst( get_linenum(), yyvsp[-3].ival, yyvsp[-2].reg, yyvsp[0].reg );
	;
    break;}
case 31:
#line 320 "vs1.0_grammar.y"
{
		yyval.inst = new VS10Inst( get_linenum(), yyvsp[-3].ival, yyvsp[-2].reg, yyvsp[0].reg );
	;
    break;}
case 32:
#line 326 "vs1.0_grammar.y"
{
		yyval.inst = new VS10Inst( get_linenum(), yyvsp[-5].ival, yyvsp[-4].reg, yyvsp[-2].reg, yyvsp[0].reg );
	;
    break;}
case 33:
#line 333 "vs1.0_grammar.y"
{
		yyval.inst = new VS10Inst( get_linenum(), yyvsp[-7].ival, yyvsp[-6].reg, yyvsp[-4].reg, yyvsp[-2].reg, yyvsp[0].reg );
	;
    break;}
case 34:
#line 339 "vs1.0_grammar.y"
{
		yyval.ival = VS10_MOV;
	;
    break;}
case 35:
#line 343 "vs1.0_grammar.y"
{
		yyval.ival = VS10_LIT;
	;
    break;}
case 36:
#line 349 "vs1.0_grammar.y"
{
	yyval.ival = VS10_RCP;
	;
    break;}
case 37:
#line 353 "vs1.0_grammar.y"
{
	yyval.ival = VS10_RSQ;
	;
    break;}
case 38:
#line 357 "vs1.0_grammar.y"
{
	yyval.ival = VS10_EXP;
	;
    break;}
case 39:
#line 361 "vs1.0_grammar.y"
{
	yyval.ival = VS10_EXPP;
	;
    break;}
case 40:
#line 365 "vs1.0_grammar.y"
{
	yyval.ival = VS10_LOG;
	;
    break;}
case 41:
#line 369 "vs1.0_grammar.y"
{
	yyval.ival = VS10_LOGP;
	;
    break;}
case 42:
#line 375 "vs1.0_grammar.y"
{
	yyval.ival = VS10_FRC;
	;
    break;}
case 43:
#line 381 "vs1.0_grammar.y"
{
	yyval.ival = VS10_MUL;
	;
    break;}
case 44:
#line 385 "vs1.0_grammar.y"
{
	yyval.ival = VS10_ADD;
	;
    break;}
case 45:
#line 389 "vs1.0_grammar.y"
{
	yyval.ival = VS10_DP3;
	;
    break;}
case 46:
#line 393 "vs1.0_grammar.y"
{
	yyval.ival = VS10_DP4;
	;
    break;}
case 47:
#line 397 "vs1.0_grammar.y"
{
	yyval.ival = VS10_DST;
	;
    break;}
case 48:
#line 401 "vs1.0_grammar.y"
{
	yyval.ival = VS10_MIN;
	;
    break;}
case 49:
#line 405 "vs1.0_grammar.y"
{
	yyval.ival = VS10_MAX;
	;
    break;}
case 50:
#line 409 "vs1.0_grammar.y"
{
	yyval.ival = VS10_SLT;
	;
    break;}
case 51:
#line 413 "vs1.0_grammar.y"
{
	yyval.ival = VS10_SGE;
	;
    break;}
case 52:
#line 417 "vs1.0_grammar.y"
{
	yyval.ival = VS10_M3X2;
	;
    break;}
case 53:
#line 421 "vs1.0_grammar.y"
{
	yyval.ival = VS10_M3X3;
	;
    break;}
case 54:
#line 425 "vs1.0_grammar.y"
{
	yyval.ival = VS10_M3X4;
	;
    break;}
case 55:
#line 429 "vs1.0_grammar.y"
{
	yyval.ival = VS10_M4X3;
	;
    break;}
case 56:
#line 433 "vs1.0_grammar.y"
{
	yyval.ival = VS10_M4X4;
	;
    break;}
case 57:
#line 437 "vs1.0_grammar.y"
{
	yyval.ival = VS10_SUB;
	;
    break;}
case 58:
#line 443 "vs1.0_grammar.y"
{
	yyval.ival = VS10_MAD;
	;
    break;}
}

#line 705 "/usr/share/bison/bison.simple"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
yyerrhandle:
  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 449 "vs1.0_grammar.y"

void yyerror(char* s)
{
    LexError( "Syntax Error.\n" );
    //errors.set(s);
    //errors.set("unrecognized token");
}
