/* getRawPower9.5.c   (version 2)
 * For use with Carrier Board 10016423 Rev E8 (= Rev A)
 * Sandia Q195871 configuration 
 */

#include "pidev.h"
#include "powerInsight.h"
#include "piglobal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/* The Lua state */
static lua_State * L = NULL ;

/* Shared space */
static char buffer[1024];

static int pidev_init( void )
{
   int ret ;

   /* Create a Lua instance */
   L = luaL_newstate( );
   if( L == NULL ) {
      fprintf( stderr, "%s: Memory allocation error creating Lua instance.\n", ARGV0 );
      return -1;
   }

   /* Load initial state */
   luaL_openlibs( L );  /* Standard libraries */

   /* Register Power Insight library */
   pi_register( L );

   /* Finish initialization with Lua code */
   strcpy( buffer, libexecdir );
   strcat( buffer, "/init_final.lc" );
   ret = luaL_loadfile( L, buffer );
   if( ret != 0 || (ret = lua_pcall( L, 0, 0, 0 )) ) {
      strcpy( buffer, "Load/run " );
      strcat( buffer, libexecdir );
      strcat( buffer, "/init_final.lc" );
      luaPI_doerror( L, ret, buffer );
   }

   /* Now read the config file */
   ret = luaL_loadfile( L, configfile );
   if( ret != 0 || (ret = lua_pcall( L, 0, LUA_MULTRET, 0 )) ) {
      strcpy( buffer, "Processing config file " );
      strcat( buffer, configfile );
      luaPI_doerror( L, ret, buffer );
   }
   if( (debug & DBG_LUA) && lua_gettop( L ) ) {
      fprintf( stderr, "Config file returned %d values. Ignored\n", lua_gettop( L ) );
   }
   lua_pop( L, lua_gettop( L ));

   /* Post-configfile initialization with Lua code */
   strcpy( buffer, libexecdir );
   strcat( buffer, "/post_conf.lc" );
   ret = luaL_loadfile( L, buffer );
   if( ret != 0 || (ret = lua_pcall( L, 0, 0, 0 )) ) {
      strcpy( buffer, "Load/run " );
      strcat( buffer, libexecdir );
      strcat( buffer, "/post_conf.lc" );
      luaPI_doerror( L, ret, buffer );
   }

   /* Push all args and Run "App" */
/*   if( ! lua_checkstack( L, argc - optind +2 ) ) {
      fprintf( stderr, "%s: Too many arguments\n", ARGV0 );
      exit( 1 );
   }
   lua_getfield( L, LUA_GLOBALSINDEX, "App" );
   for( i = optind ; i < argc ; ++i ) {
      lua_pushstring( L, argv[i] );
   }
   ret = lua_pcall( L, argc - optind, 0, 0 );
   if( ret != 0 ) {
      luaPI_doerror( L, ret, "Running application 'App'" );
   }
*/
   return 0 ;
}

void pidev_read(int portNumber, reading_t *sample)
{
}

void pidev_open(void)
{
   static int initialized = 0;

   if( !initialized ) {
      if( !pidev_init( ) ) {
         fprintf( stderr, "Unable to initialize powerinsight device" );
         return;
      }
      else
         initialized = 1;
   }
}

void pidev_close(void)
{
}

