#include "repl.h"

#include <stdlib.h> // malloc(), free()
#include <string.h> // memcpy()

#include "lib/flash.h"     // Flash_write_(), Flash_is_userscript(), Flash_read()
#include "lib/caw.h"       // Caw_send_raw(), Caw_send_luachunk(), Caw_send_luaerror()

// global variables
lua_State*  Lua;
L_repl_mode repl_mode = REPL_normal;
char*       new_script;
uint16_t    new_script_len;

// prototypes
static void REPL_new_script_buffer( uint32_t len );
static void REPL_receive_script( char* buf, uint32_t len, ErrorHandler_t errfn );

// public interface
void REPL_init( lua_State* lua )
{
    Lua = lua;

    if( Flash_is_user_script() ){
        printf("loaded! %i\n", Flash_read_user_scriptlen() );
        REPL_new_script_buffer( Flash_read_user_scriptlen() );
        if( Flash_read_user_script( new_script )
         || Lua_eval( Lua, new_script
                         , Flash_read_user_scriptlen() // must call to flash lib!
                         , Caw_send_luaerror
                         ) ){
            printf("failed to load user script\n");
            Lua_load_default_script(); // fall back to default
        }
        free(new_script);
    } else { Lua_load_default_script(); } // no user script
}

void REPL_mode( L_repl_mode mode )
{
    repl_mode = mode;
    if( repl_mode == REPL_reception ){ // begin a new transmission
        REPL_new_script_buffer( USER_SCRIPT_SIZE );
    } else { // end of a transmission
        if( !Lua_eval( Lua, new_script
                          , new_script_len
                          , Caw_send_luaerror
                          ) ){ // successful load
            // TODO if we're setting init() should check it doesn't crash
            if( Flash_write_user_script( new_script
                                       , new_script_len
                                       ) ){
                printf("flash write failed\n");
                Caw_send_luachunk("flash write failed");
            }
            printf("script saved\n");
        } else { printf("new user script failed test\n"); }
        free(new_script); // cleanup memory
    }
}

void REPL_eval( char* buf, uint32_t len, ErrorHandler_t errfn )
{
    if( repl_mode == REPL_normal ){
        if(Lua_eval( Lua, buf
                     , len
                     , errfn
                   )){
            printf("!eval\n");
        }
    } else { // REPL_reception
        REPL_receive_script( buf, len, errfn );
    }
}

void REPL_print_script( void )
{
    if( Flash_is_user_script() ){
        REPL_new_script_buffer( Flash_read_user_scriptlen() );
        Flash_read_user_script( new_script );
        uint16_t send_len = Flash_read_user_scriptlen();
        uint8_t page_count = 0;
        while( send_len > 0x200 ){
            Caw_send_raw( (uint8_t*)&new_script[(page_count++)*0x200], 0x200 );
            send_len -= 0x200;
        }
        Caw_send_raw( (uint8_t*)&new_script[page_count*0x200], send_len );
        free(new_script);
    } else {
        Caw_send_luachunk("no user script.");
    }
}

// private funcs
static void REPL_receive_script( char* buf, uint32_t len, ErrorHandler_t errfn )
{
    memcpy( &new_script[new_script_len], buf, len );
    new_script_len += len;
}

static void REPL_new_script_buffer( uint32_t len )
{
    // TODO call to Lua to free resources from current script
    new_script = malloc(len);
    if(new_script == NULL){
        printf("out of mem\n");
        Caw_send_luachunk("!script: out of memory");
        //(*errfn)("!script: out of memory");
        return; // how to deal with this situation?
        // FIXME: should respond over usb stating out of memory?
        //        try allocating a smaller amount and hope it fits?
        //        retry?
    }
    new_script_len = 0;
}
