# Configure paths for FreeType2
# Marcelo Magallon 2001-10-26, based on gtk.m4 by Owen Taylor

dnl AC_CHECK_FT2([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for FreeType2, and define FT2_CFLAGS and FT2_LIBS
dnl
AC_DEFUN(AC_CHECK_FT2,
[dnl
dnl Get the cflags and libraries from the freetype-config script
dnl
AC_ARG_WITH(ft-prefix,
[  --with-ft-prefix=PREFIX
                          Prefix where FreeType is installed (optional)],
            ft_config_prefix="$withval", ft_config_prefix="")
AC_ARG_WITH(ft-exec-prefix,
[  --with-ft-exec-prefix=PREFIX
                          Exec prefix where FreeType is installed (optional)],
            ft_config_exec_prefix="$withval", ft_config_exec_prefix="")
AC_ARG_ENABLE(freetypetest,
[  --disable-freetypetest  Do not try to compile and run
                          a test FreeType program],
              [], enable_fttest=yes)

if test x$ft_config_exec_prefix != x ; then
  ft_config_args="$ft_config_args --exec-prefix=$ft_config_exec_prefix"
  if test x${FT2_CONFIG+set} != xset ; then
    FT2_CONFIG=$ft_config_exec_prefix/bin/freetype-config
  fi
fi
if test x$ft_config_prefix != x ; then
  ft_config_args="$ft_config_args --prefix=$ft_config_prefix"
  if test x${FT2_CONFIG+set} != xset ; then
    FT2_CONFIG=$ft_config_prefix/bin/freetype-config
  fi
fi
AC_PATH_PROG(FT2_CONFIG, freetype-config, no)

min_ft_version=ifelse([$1], ,9.1.0,$1)
AC_MSG_CHECKING(for FreeType - version >= $min_ft_version)
no_ft=""
if test "$FT2_CONFIG" = "no" ; then
  no_ft=yes
else
  FT2_CFLAGS=`$FT2_CONFIG $ft_config_args --cflags`
  FT2_LIBS=`$FT2_CONFIG $ft_config_args --libs`
  ft_config_major_version=`$FT2_CONFIG $ft_config_args --version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
  ft_config_minor_version=`$FT2_CONFIG $ft_config_args --version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
  ft_config_micro_version=`$FT2_CONFIG $ft_config_args --version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
  ft_min_major_version=`echo $min_ft_version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
  ft_min_minor_version=`echo $min_ft_version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
  ft_min_micro_version=`echo $min_ft_version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
  if test x$enable_fttest = xyes ; then
    ft_config_is_lt=""
    if test $ft_config_major_version -lt $ft_min_major_version ; then
      ft_config_is_lt=yes
    else
      if test $ft_config_major_version -eq $ft_min_major_version ; then
        if test $ft_config_minor_version -lt $ft_min_minor_version ; then
          ft_config_is_lt=yes
        else
          if test $ft_config_minor_version -eq $ft_min_minor_version ; then
            if test $ft_config_micro_version -lt $ft_min_micro_version ; then
              ft_config_is_lt=yes
            fi
          fi
        fi
      fi
    fi
    if test x$ft_config_is_lt = xyes ; then
      no_ft=yes
    else
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $FT2_CFLAGS"
      LIBS="$FT2_LIBS $LIBS"
dnl
dnl Sanity checks for the results of freetype-config to some extent
dnl
      AC_TRY_RUN([
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <stdlib.h>

int
main()
{
  FT_Library library;
  FT_Error error;

  error = FT_Init_FreeType(&library);

  if (error)
    return 1;
  else
  {
    FT_Done_FreeType(library);
    return 0;
  }
}
],, no_ft=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
      CFLAGS="$ac_save_CFLAGS"
      LIBS="$ac_save_LIBS"
    fi             # test $ft_config_version -lt $ft_min_version
  fi               # test x$enable_fttest = xyes
fi                 # test "$FT2_CONFIG" = "no"
if test x$no_ft = x ; then
   AC_MSG_RESULT(yes)
   ifelse([$2], , :, [$2])
else
   AC_MSG_RESULT(no)
   if test "$FT2_CONFIG" = "no" ; then
     echo "*** The freetype-config script installed by FreeType 2 could not be found."
     echo "*** If FreeType 2 was installed in PREFIX, make sure PREFIX/bin is in"
     echo "*** your path, or set the FT2_CONFIG environment variable to the"
     echo "*** full path to freetype-config."
   else
     if test x$ft_config_is_lt = xyes ; then
       echo "*** Your installed version of the FreeType 2 library is too old."
       echo "*** If you have different versions of FreeType 2, make sure that"
       echo "*** correct values for --with-ft-prefix or --with-ft-exec-prefix"
       echo "*** are used, or set the FT2_CONFIG environment variable to the"
       echo "*** full path to freetype-config."
     else
       echo "*** The FreeType test program failed to run.  If your system uses"
       echo "*** shared libraries and they are installed outside the normal"
       echo "*** system library path, make sure the variable LD_LIBRARY_PATH"
       echo "*** (or whatever is appropiate for your system) is correctly set."
     fi
   fi
   FT2_CFLAGS=""
   FT2_LIBS=""
   ifelse([$3], , :, [$3])
fi
AC_SUBST(FT2_CFLAGS)
AC_SUBST(FT2_LIBS)
])

AC_DEFUN([OGRE_USE_STLPORT],
[AC_ARG_WITH(stlport, 
             AC_HELP_STRING([--with-stlport=PATH],
                           [the path to STLPort.]),
             ac_cv_use_stlport=$withval,
             ac_cv_use_stlport=no)
 AC_CACHE_CHECK([whether to use STLPort], ac_cv_use_stlport,
                ac_cv_use_stlport=no)
 if test x$ac_cv_use_stlport != xno; then
     STLPORT_CFLAGS="-I$ac_cv_use_stlport/stlport"
     STLPORT_LIBS="-L$ac_cv_use_stlport/lib -lstlport"
 fi
 AC_SUBST(STLPORT_CFLAGS)
 AC_SUBST(STLPORT_LIBS)
])

AC_DEFUN([OGRE_GET_CONFIG_TOOLKIT],
[OGRE_CFGTK=cli
 AC_ARG_WITH(cfgtk, 
             AC_HELP_STRING([--with-cfgtk=TOOLKIT],
                            [the toolkit for the config gui, currently cli or gtk]),
             OGRE_CFGTK=$withval,
             OGRE_CFGTK=cli)

 
  CFGTK_DEPS_CFLAGS=""
  CFGTK_DEPS_LIBS=""

  dnl Do the extra checks per type here
  case $OGRE_CFGTK in 
    gtk)
      PKG_CHECK_MODULES(CFGTK_DEPS, gtkmm-2.0 libglademm-2.0);;
  esac

  AC_SUBST(CFGTK_DEPS_CFLAGS)
  AC_SUBST(CFGTK_DEPS_LIBS)
  AC_SUBST(OGRE_CFGTK)
])

AC_DEFUN([OGRE_BUILD_PYTHON_LINK],
[AC_ARG_ENABLE(python-link,
              AC_HELP_STRING([--enable-python-link],
                             [Build the python extension]),
              [
              build_py=true
              AC_CONFIG_FILES([Tools/PythonInterface/Makefile \
                               Tools/PythonInterface/src/Makefile \
                               Tools/PythonInterface/misc/Makefile \
                               Tools/PythonInterface/include/Makefile], sh python_debug.sh $ac_file)],
              [build_py=false])

AC_ARG_ENABLE(python-debug,
              AC_HELP_STRING([--enable-python-debug],
                             [Enable C++ debug for the python extension, might be compile time consumming]),
              [ rm -f python_debug.sh ; ln -s Scripts/null.sh python_debug.sh ],
              [ rm -f python_debug.sh ; ln -s Scripts/remove_debug.sh python_debug.sh ])

AM_CONDITIONAL(BUILD_PYTHON_INTERFACE, test x$build_py = xtrue)
boost_inc_dir=""
boost_lib_dir=""
AC_ARG_WITH(boost-includes, AC_HELP_STRING([--with-boost-includes=PATH],
                            [Directory that has the boost headers.]),
            boost_inc_dir=$withval)
AC_ARG_WITH(boost-libs, AC_HELP_STRING([--with-boost-libs=PATH],
                            [Directory that has the boost libraries]),
            boost_lib_dir=$withval)

BOOST_CFLAGS=""
BOOST_LIBS="-lboost_python"

if test -n "$boost_inc_dir"; then
    AC_MSG_CHECKING(boost include dir)
    if test -e $boost_inc_dir/boost/python.hpp; then
        BOOST_CFLAGS="-I$boost_inc_dir";
    else
        AC_MSG_ERROR("Unable to find boost/python.hpp in $boost_inc_dir")
    fi
    AC_MSG_RESULT(done)
fi

if test -n "$boost_lib_dir"; then
    AC_MSG_CHECKING(boost library dir)
    if test -e $boost_lib_dir/libboost_python.so; then
        BOOST_LIBS="-L$boost_lib_dir $BOOST_LIBS";
    else
        AC_MSG_ERROR("Unable to find libboost_python.so in $boost_lib_dir")
    fi
    AC_MSG_RESULT(done)
fi

AC_SUBST(BOOST_CFLAGS)
AC_SUBST(BOOST_LIBS)

pylibdir="" dnl Default just means we use /usr/lib
pyincdir="" dnl Default just means we use /usr/include
pyversion="2.2"
AC_ARG_WITH(python-version, AC_HELP_STRING([--with-python-version=VER],
                            [The version of python to use, default 2.2]),
            pyversion=$withval)
AC_ARG_WITH(python-libs, AC_HELP_STRING([--with-python-libs=PATH],
                            [Directory that has the python library]),
            pylibdir=$withval)
AC_ARG_WITH(python-includes, AC_HELP_STRING([--with-python-includes=PATH],
                            [Directory that has the python headers]),
            pyincdir=$withval)

PYTHON_CFLAGS="-I/usr/include/python$pyversion"
PYTHON_LIBS="-lpython$pyversion"
if test -n "$pyincdir"; then
    AC_MSG_CHECKING(for python headers in $pyincdir/python$pyversion)
    if test -e $pyincdir/python$pyversion/Python.h; then
        PYTHON_CFLAGS="-I$pyincdir/python$pyversion";
    else
        AC_MSG_ERROR("Unable to locate")
    fi
    AC_MSG_RESULT(found)
fi

if test -n "$pylibdir"; then
    AC_MSG_CHECKING(for python library version $pyversion in $pylibdir)
    if test -e $pylibdir/libpython$pyversion.so -o -e $pylibdir/libpython$pyversion.a ; then
        PYTHON_LIBS="-L$pylibdir $PYTHON_LIBS";
    else
        AC_MSG_ERROR("Unable to locate")
    fi
    AC_MSG_RESULT(found)
fi

AC_SUBST(PYTHON_CFLAGS)
AC_SUBST(PYTHON_LIBS)

])

AC_DEFUN([OGRE_SETUP_FOR_HOST],
[case $target in
powerpc-apple-darwin*)
	AC_SUBST(SHARED_FLAGS, "-bundle -undefined suppress -flat_namespace")
	AC_SUBST(GL_LIBS,,"-framework OpenGL")
	;;
*) dnl default to standard linux
	AC_SUBST(SHARED_FLAGS, "-shared")
	AC_SUBST(GL_LIBS, "-lGL -lGLU")
;;
esac
])


AC_DEFUN([OGRE_DETECT_ENDIAN],
[AC_TRY_RUN([
		int main()
		{
			short s = 1;
			short* ptr = &s;
			unsigned char c = *((char*)ptr);
			return c;
		}
	]
	,[AC_DEFINE(CONFIG_BIG_ENDIAN,,[Big endian machine])]
	,[AC_DEFINE(CONFIG_LITTLE_ENDIAN,,[Little endian machine])])
])
