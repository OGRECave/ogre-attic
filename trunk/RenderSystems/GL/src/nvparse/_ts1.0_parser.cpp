
/*  A Bison parser, made from ts1.0_grammar.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse ts10_parse
#define yylex ts10_lex
#define yyerror ts10_error
#define yylval ts10_lval
#define yychar ts10_char
#define yydebug ts10_debug
#define yynerrs ts10_nerrs
#define	floatValue	257
#define	gequal	258
#define	less	259
#define	texVariable	260
#define	expandString	261
#define	openParen	262
#define	closeParen	263
#define	semicolon	264
#define	comma	265
#define	nop	266
#define	texture_1d	267
#define	texture_2d	268
#define	texture_rectangle	269
#define	texture_3d	270
#define	texture_cube_map	271
#define	cull_fragment	272
#define	pass_through	273
#define	offset_2d_scale	274
#define	offset_2d	275
#define	offset_rectangle_scale	276
#define	offset_rectangle	277
#define	dependent_ar	278
#define	dependent_gb	279
#define	dot_product_2d_1of2	280
#define	dot_product_2d_2of2	281
#define	dot_product_rectangle_1of2	282
#define	dot_product_rectangle_2of2	283
#define	dot_product_depth_replace_1of2	284
#define	dot_product_depth_replace_2of2	285
#define	dot_product_3d_1of3	286
#define	dot_product_3d_2of3	287
#define	dot_product_3d_3of3	288
#define	dot_product_cube_map_1of3	289
#define	dot_product_cube_map_2of3	290
#define	dot_product_cube_map_3of3	291
#define	dot_product_reflect_cube_map_eye_from_qs_1of3	292
#define	dot_product_reflect_cube_map_eye_from_qs_2of3	293
#define	dot_product_reflect_cube_map_eye_from_qs_3of3	294
#define	dot_product_reflect_cube_map_const_eye_1of3	295
#define	dot_product_reflect_cube_map_const_eye_2of3	296
#define	dot_product_reflect_cube_map_const_eye_3of3	297
#define	dot_product_cube_map_and_reflect_cube_map_eye_from_qs_1of3	298
#define	dot_product_cube_map_and_reflect_cube_map_eye_from_qs_2of3	299
#define	dot_product_cube_map_and_reflect_cube_map_eye_from_qs_3of3	300
#define	dot_product_cube_map_and_reflect_cube_map_const_eye_1of3	301
#define	dot_product_cube_map_and_reflect_cube_map_const_eye_2of3	302
#define	dot_product_cube_map_and_reflect_cube_map_const_eye_3of3	303

#line 2 "ts1.0_grammar.y"

void yyerror(char* s);
int yylex ( void );

#ifdef _WIN32
# include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "ts1.0_inst.h"
#include "ts1.0_inst_list.h"
#include "nvparse_errors.h"
#include "nvparse_externs.h"


#line 17 "ts1.0_grammar.y"
typedef union {
  float fval;
  InstPtr inst;
  InstListPtr instList;
  MappedVariablePtr variable;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		218
#define	YYFLAG		-32768
#define	YYNTBASE	50

#define YYTRANSLATE(x) ((unsigned)(x) <= 303 ? yytranslate[x] : 55)

static const char yytranslate[] = {     0,
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
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     6,     9,    14,    16,    20,    24,    28,    32,
    36,    40,    51,    55,    72,    85,   102,   115,   120,   125,
   130,   135,   140,   145,   150,   155,   160,   165,   170,   175,
   180,   185,   190,   195,   200,   211,   216,   221,   226,   231,
   236,   247,   252,   257,   259
};

static const short yyrhs[] = {    51,
     0,    51,    53,    10,     0,    53,    10,     0,     7,     8,
     6,     9,     0,     6,     0,    12,     8,     9,     0,    13,
     8,     9,     0,    14,     8,     9,     0,    15,     8,     9,
     0,    16,     8,     9,     0,    17,     8,     9,     0,    18,
     8,    54,    11,    54,    11,    54,    11,    54,     9,     0,
    19,     8,     9,     0,    20,     8,     6,    11,     3,    11,
     3,    11,     3,    11,     3,    11,     3,    11,     3,     9,
     0,    21,     8,     6,    11,     3,    11,     3,    11,     3,
    11,     3,     9,     0,    22,     8,     6,    11,     3,    11,
     3,    11,     3,    11,     3,    11,     3,    11,     3,     9,
     0,    23,     8,     6,    11,     3,    11,     3,    11,     3,
    11,     3,     9,     0,    24,     8,     6,     9,     0,    25,
     8,     6,     9,     0,    26,     8,    52,     9,     0,    27,
     8,    52,     9,     0,    28,     8,    52,     9,     0,    29,
     8,    52,     9,     0,    30,     8,    52,     9,     0,    31,
     8,    52,     9,     0,    32,     8,    52,     9,     0,    33,
     8,    52,     9,     0,    34,     8,    52,     9,     0,    35,
     8,    52,     9,     0,    36,     8,    52,     9,     0,    37,
     8,    52,     9,     0,    38,     8,    52,     9,     0,    39,
     8,    52,     9,     0,    40,     8,    52,     9,     0,    41,
     8,    52,    11,     3,    11,     3,    11,     3,     9,     0,
    42,     8,    52,     9,     0,    43,     8,    52,     9,     0,
    44,     8,    52,     9,     0,    45,     8,    52,     9,     0,
    46,     8,    52,     9,     0,    47,     8,    52,    11,     3,
    11,     3,    11,     3,     9,     0,    48,     8,    52,     9,
     0,    49,     8,    52,     9,     0,     4,     0,     5,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    72,    80,    86,    96,   102,   110,   114,   118,   122,   126,
   130,   134,   138,   142,   146,   150,   154,   158,   162,   166,
   171,   176,   181,   186,   191,   196,   201,   206,   211,   216,
   221,   226,   231,   236,   241,   246,   251,   256,   261,   266,
   271,   276,   281,   288,   292
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","floatValue",
"gequal","less","texVariable","expandString","openParen","closeParen","semicolon",
"comma","nop","texture_1d","texture_2d","texture_rectangle","texture_3d","texture_cube_map",
"cull_fragment","pass_through","offset_2d_scale","offset_2d","offset_rectangle_scale",
"offset_rectangle","dependent_ar","dependent_gb","dot_product_2d_1of2","dot_product_2d_2of2",
"dot_product_rectangle_1of2","dot_product_rectangle_2of2","dot_product_depth_replace_1of2",
"dot_product_depth_replace_2of2","dot_product_3d_1of3","dot_product_3d_2of3",
"dot_product_3d_3of3","dot_product_cube_map_1of3","dot_product_cube_map_2of3",
"dot_product_cube_map_3of3","dot_product_reflect_cube_map_eye_from_qs_1of3",
"dot_product_reflect_cube_map_eye_from_qs_2of3","dot_product_reflect_cube_map_eye_from_qs_3of3",
"dot_product_reflect_cube_map_const_eye_1of3","dot_product_reflect_cube_map_const_eye_2of3",
"dot_product_reflect_cube_map_const_eye_3of3","dot_product_cube_map_and_reflect_cube_map_eye_from_qs_1of3",
"dot_product_cube_map_and_reflect_cube_map_eye_from_qs_2of3","dot_product_cube_map_and_reflect_cube_map_eye_from_qs_3of3",
"dot_product_cube_map_and_reflect_cube_map_const_eye_1of3","dot_product_cube_map_and_reflect_cube_map_const_eye_2of3",
"dot_product_cube_map_and_reflect_cube_map_const_eye_3of3","WholeEnchilada",
"InstListDesc","MappedVariableDesc","InstDesc","CondDesc", NULL
};
#endif

static const short yyr1[] = {     0,
    50,    51,    51,    52,    52,    53,    53,    53,    53,    53,
    53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
    53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
    53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
    53,    53,    53,    54,    54
};

static const short yyr2[] = {     0,
     1,     3,     2,     4,     1,     3,     3,     3,     3,     3,
     3,    10,     3,    16,    12,    16,    12,     4,     4,     4,
     4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
     4,     4,     4,     4,    10,     4,     4,     4,     4,     4,
    10,     4,     4,     1,     1
};

static const short yydefact[] = {     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     1,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     3,
     6,     7,     8,     9,    10,    11,    44,    45,     0,    13,
     0,     0,     0,     0,     0,     0,     5,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     2,     0,     0,     0,     0,     0,    18,    19,
     0,    20,    21,    22,    23,    24,    25,    26,    27,    28,
    29,    30,    31,    32,    33,    34,     0,    36,    37,    38,
    39,    40,     0,    42,    43,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     4,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    12,     0,     0,     0,     0,    35,    41,     0,
     0,     0,     0,     0,    15,     0,    17,     0,     0,     0,
     0,     0,     0,    14,    16,     0,     0,     0
};

static const short yydefgoto[] = {   216,
    39,    99,    40,    89
};

static const short yypact[] = {   -11,
    31,    35,    39,    40,    41,    42,    43,    44,    45,    46,
    71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
    81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
    91,    92,    93,    94,    95,    96,    97,    98,   -11,    36,
    99,   100,   101,   102,   103,   104,    37,   105,   109,   110,
   111,   112,   113,   114,    38,    38,    38,    38,    38,    38,
    38,    38,    38,    38,    38,    38,    38,    38,    38,    38,
    38,    38,    38,    38,    38,    38,    38,    38,   115,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   116,-32768,
   117,   118,   119,   120,   123,   124,-32768,   126,   127,   128,
   129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
   139,   140,   141,   142,   143,   145,   146,   147,   148,   149,
   150,   152,-32768,    37,   121,   155,   159,   160,-32768,-32768,
   158,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   162,-32768,-32768,-32768,
-32768,-32768,   163,-32768,-32768,   156,   157,   161,   164,   165,
   168,   167,   169,    37,   166,   170,   171,   176,-32768,   178,
   179,   172,   173,   174,   175,   177,   180,   181,    37,   184,
   186,   187,   190,   191,   192,   188,   185,   189,   193,   194,
   197,   198,-32768,   195,   196,   199,   200,-32768,-32768,   201,
   202,   203,   204,   205,-32768,   206,-32768,   207,   208,   212,
   213,   211,   214,-32768,-32768,   107,   122,-32768
};

static const short yypgoto[] = {-32768,
-32768,     0,   182,  -124
};


#define	YYLAST		223


static const short yytable[] = {   156,
     1,     2,     3,     4,     5,     6,     7,     8,     9,    10,
    11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
    31,    32,    33,    34,    35,    36,    37,    38,    41,   172,
    87,    88,    42,    97,    98,    80,    43,    44,    45,    46,
    47,    48,    49,    50,   186,   100,   101,   102,   103,   104,
   105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
   115,   116,   117,   118,   119,   120,   121,   122,    51,    52,
    53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
    63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
    73,    74,    75,    76,    77,    78,   217,    81,    82,    83,
    84,    85,    86,    90,    91,    92,    93,    94,    95,    96,
     0,   218,     0,   157,   123,     0,   124,   125,   126,   127,
   128,   129,   130,   131,     0,   132,   133,   134,   135,   136,
   137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     0,   148,   147,   149,   150,   151,   152,   158,   154,   153,
   155,   159,   160,   161,   162,   163,   164,   165,   173,     0,
     0,   166,   174,   175,   167,   168,   169,   170,   176,   171,
   177,   178,   179,   180,   181,   182,   187,   183,   188,   189,
   184,   185,   190,   191,   192,   194,   193,   200,   201,   195,
     0,   202,   203,   196,   197,   198,   199,   208,   209,     0,
   205,   204,   207,   206,   212,   213,     0,   210,   211,   214,
    79,     0,   215
};

static const short yycheck[] = {   124,
    12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
    32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
    42,    43,    44,    45,    46,    47,    48,    49,     8,   164,
     4,     5,     8,     6,     7,    10,     8,     8,     8,     8,
     8,     8,     8,     8,   179,    56,    57,    58,    59,    60,
    61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
    71,    72,    73,    74,    75,    76,    77,    78,     8,     8,
     8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
     8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
     8,     8,     8,     8,     8,     8,     0,     9,     9,     9,
     9,     9,     9,     9,     6,     6,     6,     6,     6,     6,
    -1,     0,    -1,     3,    10,    -1,    11,    11,    11,    11,
    11,     9,     9,     8,    -1,     9,     9,     9,     9,     9,
     9,     9,     9,     9,     9,     9,     9,     9,     9,     9,
    -1,     9,    11,     9,     9,     9,     9,     3,     9,    11,
     9,     3,     3,     6,     3,     3,    11,    11,     3,    -1,
    -1,    11,     3,     3,    11,    11,     9,    11,     3,    11,
     3,     3,    11,    11,    11,    11,     3,    11,     3,     3,
    11,    11,     3,     3,     3,    11,     9,     3,     3,    11,
    -1,     3,     3,    11,    11,     9,     9,     3,     3,    -1,
     9,    11,     9,    11,     3,     3,    -1,    11,    11,     9,
    39,    -1,     9
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

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

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
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
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
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

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 73 "ts1.0_grammar.y"
{
			yyvsp[0].instList->Validate();
			yyvsp[0].instList->Invoke();
		    delete yyvsp[0].instList;
		;
    break;}
case 2:
#line 81 "ts1.0_grammar.y"
{
		    *(yyvsp[-2].instList) += yyvsp[-1].inst;
			delete yyvsp[-1].inst;
		    yyval.instList = yyvsp[-2].instList;
		;
    break;}
case 3:
#line 87 "ts1.0_grammar.y"
{
		    InstListPtr instList = new InstList;
		    *instList += yyvsp[-1].inst;
			delete yyvsp[-1].inst;
		    yyval.instList = instList;
		;
    break;}
case 4:
#line 97 "ts1.0_grammar.y"
{
			yyval.variable = new MappedVariable;
			yyval.variable->var = yyvsp[-1].fval;
			yyval.variable->expand = true;
		;
    break;}
case 5:
#line 103 "ts1.0_grammar.y"
{
			yyval.variable = new MappedVariable;
			yyval.variable->var = yyvsp[0].fval;
			yyval.variable->expand = false;
		;
    break;}
case 6:
#line 111 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_NOP);
		;
    break;}
case 7:
#line 115 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_TEXTURE_1D);
		;
    break;}
case 8:
#line 119 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_TEXTURE_2D);
		;
    break;}
case 9:
#line 123 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_TEXTURE_RECTANGLE);
		;
    break;}
case 10:
#line 127 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_TEXTURE_3D);
		;
    break;}
case 11:
#line 131 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_TEXTURE_CUBE_MAP);
		;
    break;}
case 12:
#line 135 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_CULL_FRAGMENT, yyvsp[-7].fval, yyvsp[-5].fval, yyvsp[-3].fval, yyvsp[-1].fval);
		;
    break;}
case 13:
#line 139 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_PASS_THROUGH);
		;
    break;}
case 14:
#line 143 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_OFFSET_2D_SCALE, yyvsp[-13].fval, yyvsp[-11].fval, yyvsp[-9].fval, yyvsp[-7].fval, yyvsp[-5].fval, yyvsp[-3].fval, yyvsp[-1].fval);
		;
    break;}
case 15:
#line 147 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_OFFSET_2D, yyvsp[-9].fval, yyvsp[-7].fval, yyvsp[-5].fval, yyvsp[-3].fval, yyvsp[-1].fval);
		;
    break;}
case 16:
#line 151 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_OFFSET_RECTANGLE_SCALE, yyvsp[-13].fval, yyvsp[-11].fval, yyvsp[-9].fval, yyvsp[-7].fval, yyvsp[-5].fval, yyvsp[-3].fval, yyvsp[-1].fval);
		;
    break;}
case 17:
#line 155 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_OFFSET_RECTANGLE, yyvsp[-9].fval, yyvsp[-7].fval, yyvsp[-5].fval, yyvsp[-3].fval, yyvsp[-1].fval);
		;
    break;}
case 18:
#line 159 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DEPENDENT_AR, yyvsp[-1].fval);
		;
    break;}
case 19:
#line 163 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DEPENDENT_GB, yyvsp[-1].fval);
		;
    break;}
case 20:
#line 167 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_2D_1_OF_2, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 21:
#line 172 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_2D_2_OF_2, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 22:
#line 177 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_RECTANGLE_1_OF_2, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 23:
#line 182 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_RECTANGLE_2_OF_2, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 24:
#line 187 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_DEPTH_REPLACE_1_OF_2, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 25:
#line 192 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_DEPTH_REPLACE_2_OF_2, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 26:
#line 197 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_3D_1_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 27:
#line 202 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_3D_2_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 28:
#line 207 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_3D_3_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 29:
#line 212 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_1_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 30:
#line 217 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_2_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 31:
#line 222 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_3_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 32:
#line 227 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_EYE_FROM_QS_1_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 33:
#line 232 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_EYE_FROM_QS_2_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 34:
#line 237 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_EYE_FROM_QS_3_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 35:
#line 242 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_CONST_EYE_1_OF_3, yyvsp[-7].variable, yyvsp[-5].fval, yyvsp[-3].fval, yyvsp[-1].fval);
			delete yyvsp[-7].variable;
		;
    break;}
case 36:
#line 247 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_CONST_EYE_2_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 37:
#line 252 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_REFLECT_CUBE_MAP_CONST_EYE_3_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 38:
#line 257 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_EYE_FROM_QS_1_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 39:
#line 262 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_EYE_FROM_QS_2_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 40:
#line 267 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_EYE_FROM_QS_3_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 41:
#line 272 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_CONST_EYE_1_OF_3, yyvsp[-7].variable, yyvsp[-5].fval, yyvsp[-3].fval, yyvsp[-1].fval);
			delete yyvsp[-7].variable;
		;
    break;}
case 42:
#line 277 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_CONST_EYE_2_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 43:
#line 282 "ts1.0_grammar.y"
{
		    yyval.inst = new Inst(TSP_DOT_PRODUCT_CUBE_MAP_AND_REFLECT_CUBE_MAP_CONST_EYE_3_OF_3, yyvsp[-1].variable);
			delete yyvsp[-1].variable;
		;
    break;}
case 44:
#line 289 "ts1.0_grammar.y"
{
			yyval.fval = yyvsp[0].fval;
		;
    break;}
case 45:
#line 293 "ts1.0_grammar.y"
{
			yyval.fval = yyvsp[0].fval;
		;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/share/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

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

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 299 "ts1.0_grammar.y"

void yyerror(char* s)
{
     errors.set("unrecognized token");
}
