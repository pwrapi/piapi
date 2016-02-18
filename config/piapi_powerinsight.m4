
AC_DEFUN([PIAPI_CHECK_POWERINSIGHT], [
	AC_ARG_WITH([powerinsight], [AS_HELP_STRING([--with-powerinsight@<:@=DIR@:>@], [Use the powerinsight binding in directory specified (DIR).])])

	piapi_check_powerinsight_happy="yes"

	CFLAGS_saved="$CFLAGS"
  	LDFLAGS_saved="$LDFLAGS"
	LIBS_saved="$LIBS"
	POWERINSIGHT_LIBS=""

	AS_IF([test -z "$with_powerinsight"],
		[POWERINSIGHT_CFLAGS=
                 POWERINSIGHT_LDFLAGS=
                 POWERINSIGHT_LIBS="-lpidev"
                 LIBS="$LIBS $POWERINSIGHT_LIBS"],
		[	AS_IF([test "x$with_powerinsight" = "xyes"],
				[POWERINSIGHT_CFLAGS=
           		 POWERINSIGHT_LDFLAGS=
           		 POWERINSIGHT_LIBS="-lpidev"
           		 LIBS="$LIBS $POWERINSIGHT_LIBS"],
				[POWERINSIGHT_CFLAGS="-I$with_powerinsight/include"
                 CFLAGS="$POWERINSIGHT_CFLAGS $CFLAGS"
                 POWERINSIGHT_LDFLAGS="-L$with_powerinsight/lib"
                 LDFLAGS="$POWERINSIGHT_LDFLAGS $LDFLAGS"
                 POWERINSIGHT_LIBS="-lpidev"
                 LIBS="$LIBS $POWERINSIGHT_LIBS"]
		)]
	)

	AC_LANG_SAVE
	AC_LANG_CPLUSPLUS

	AC_CHECK_HEADERS([pidev.h], [], [piapi_check_powerinsight_happy="no"])
	AC_LINK_IFELSE([AC_LANG_PROGRAM([], [
			int a;
		])], [piapi_check_powerinsight_lib_happy="yes"],
		[piapi_check_powerinsight_lib_happy="no"])

	AS_IF([test "x$piapi_check_powerinsight_lib_happy" = "xno"],
		[piapi_check_powerinsight_happy="no"])

	AC_LANG_RESTORE

	CFLAGS="$CFLAGS_saved"
	LDFLAGS="$LDFLAGS_saved"
	LIBS="$LIBS_saved"

	AC_SUBST([POWERINSIGHT_CFLAGS])
	AC_SUBST([POWERINSIGHT_LDFLAGS])
	AC_SUBST([POWERINSIGHT_LIBS])

	AM_CONDITIONAL([HAVE_POWERINSIGHT], [test "x$piapi_check_powerinsight_happy" = "xyes"])
	AS_IF([test "x$piapi_check_powerinsight_happy" = "xyes"], 
		[AC_DEFINE([HAVE_POWERINSIGHT], [1], [Set to 1 if powerinsight is found during configuration])])

	AS_IF([test "x$piapi_check_powerinsight_happy" = "xyes"], [$1], [$2])
])
