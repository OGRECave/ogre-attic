#ifndef BISON__PS1_0_PARSER_H
# define BISON__PS1_0_PARSER_H

#ifndef YYSTYPE
typedef union 
{
	int ival;
	float fval;
	
	string * sval;
	constdef * cdef;
	vector<constdef> * consts;
	vector<string> * line;
	list<vector<string> > * lines;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	HEADER	257
# define	NEWLINE	258
# define	NUMBER	259
# define	REG	260
# define	DEF	261
# define	ADDROP	262
# define	BLENDOP	263


extern YYSTYPE ps10_lval;

#endif /* not BISON__PS1_0_PARSER_H */
