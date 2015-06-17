AC_DEFUN([PIAPI_CHECK_DEBUG],[

    AC_ARG_ENABLE(
        [debug],
        [AS_HELP_STRING([--enable-debug], 
            [Enable debug features of the PowerInsight API])]
    )

    AS_IF(
        [test "x$enable_debug" = "xyes"],
        [AC_DEFINE([USE_DEBUG],[1], 
            [Set to 1 to use debugging features of the PowerInsight API])]    
    ) 
])

AM_CONDITIONAL(USE_STATIC, [test "x$USE_STATIC" = xyes])
