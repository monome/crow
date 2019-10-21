#include "repl.h"

#include <stdlib.h> // malloc(), free()
#include <string.h> // memcpy()
#include <stdbool.h>

#include <stm32f7xx_hal.h> // HAL_Delay()
#include "lib/flash.h"     // Flash_write_(), Flash_which_userscript(), Flash_read()
#include "lib/caw.h"       // Caw_send_raw(), Caw_send_luachunk(), Caw_send_luaerror()

// types
typedef enum{ REPL_normal
            , REPL_reception
            , REPL_discard
} L_repl_mode;

// global variables
lua_State*  Lua;
L_repl_mode repl_mode = REPL_normal;
char*       new_script;
uint16_t    new_script_len;

// prototypes
static bool REPL_new_script_buffer( uint32_t len );
static void REPL_receive_script( char* buf, uint32_t len, ErrorHandler_t errfn );
static char* REPL_script_name_from_mem( char* dest, char* src, int max_len );

// public interface
void REPL_init( lua_State* lua )
{
    Lua = lua;

    switch( Flash_which_user_script() ){
        case USERSCRIPT_Default:
            Lua_load_default_script();
            break;
        case USERSCRIPT_User:
            REPL_new_script_buffer( Flash_read_user_scriptlen() );
            if( Flash_read_user_script( new_script )
             || Lua_eval( Lua, new_script
                             , Flash_read_user_scriptlen() // must call to flash lib!
                             , Caw_send_luaerror
                             ) ){
                printf("failed to load user script\n");
                Caw_send_luachunk("failed to load user script");
            }
            free(new_script);
            break;
        case USERSCRIPT_Clear:
            // Do nothing!
            break;
    }
}

void REPL_begin_upload( void )
{
    Lua_Reset(); // free up memory
    if( REPL_new_script_buffer( USER_SCRIPT_SIZE ) ){
        repl_mode = REPL_reception;
    } else {
        repl_mode = REPL_discard;
    }
}

void REPL_upload( int flash )
{
    if( repl_mode == REPL_discard ){
        Caw_send_luachunk("upload failed, returning to normal mode");
    } else {
        if( !Lua_eval( Lua, new_script
                          , new_script_len
                          , Caw_send_luaerror
                          ) ){ // successful load
            if( flash ){
                // TODO if we're setting init() should check it doesn't crash
                if( Flash_write_user_script( new_script
                                           , new_script_len
                                           ) ){
                    printf("flash write failed\n");
                    Caw_send_luachunk("User script upload failed!");
                } else {
                    printf("script saved, len: %i\n", new_script_len);
                    Caw_send_luachunk("User script updated.");
                    REPL_print_script_name(NULL);
                    Lua_crowbegin();
                }
            } else {
                REPL_print_script_name(new_script);
                Lua_crowbegin();
            }
        } else {
            Caw_send_luachunk("User script evaluation failed.");
        }
        free(new_script);
    }
    repl_mode = REPL_normal;
}

void REPL_clear_script( void )
{
    Lua_Reset();
    Flash_clear_user_script();
    Caw_send_luachunk("User script cleared.");
    REPL_print_script_name(NULL);
    Lua_crowbegin();
}

void REPL_default_script( void )
{
    Lua_Reset();
    Flash_default_user_script();
    Caw_send_luachunk("Using default script.");
    REPL_print_script_name(NULL);
    Lua_load_default_script();
    Lua_crowbegin();
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
    if( Flash_which_user_script() == USERSCRIPT_User ){
        uint16_t length = Flash_read_user_scriptlen();
        char* addr = Flash_read_user_scriptaddr();
        const int chunk = 0x200;
        while( length > chunk ){
            Caw_send_raw( (uint8_t*)addr, chunk );
            length -= chunk;
            addr += chunk;
            HAL_Delay(3); // wait for usb tx
        }
        Caw_send_raw( (uint8_t*)addr, length );
    } else {
        REPL_print_script_name(NULL);
    }
}

void REPL_print_script_name( char* buffer )
{
    switch( Flash_which_user_script() ){
        case USERSCRIPT_Default:
            Caw_send_luachunk("Running: First.lua");
            break;
        case USERSCRIPT_User:
            buffer[0] = buffer[0]; // satisfy switch
            char script[32];
            memset( script, '\0', 32 );
            strcpy( script, "Running: " );
            REPL_script_name_from_mem( &script[9]
                                     , buffer ? buffer
                                              : (char*)(USER_SCRIPT_LOCATION+4 )
                                     , 22
                                     );
            Caw_send_luachunk(script);
            break;
        case USERSCRIPT_Clear:
            Caw_send_luachunk("No user script.");
            break;
    }
}

// private funcs
static void REPL_receive_script( char* buf, uint32_t len, ErrorHandler_t errfn )
{
    if( new_script_len + len >= USER_SCRIPT_SIZE ){
        Caw_send_luachunk("!ERROR! Script is too long.");
        repl_mode = REPL_discard;
        free(new_script);
    } else {
        memcpy( &new_script[new_script_len], buf, len );
        new_script_len += len;
    }
}

static bool REPL_new_script_buffer( uint32_t len )
{
    new_script = malloc(len);
    if( new_script == NULL ){
        printf("malloc failed. REPL\n");
        Caw_send_luachunk("!ERROR! Out of memory.");
        return false;
    }
    memset(new_script, 0, len);
    new_script_len = 0;
    return true;
}

static char* REPL_script_name_from_mem( char* dest, char* src, int max_len )
{
    while( *src == '-' ){ src++; } // skip commments
    while( *src == ' ' ){ src++; } // skip spaces
    char* linebreak = strchr( src, '\n' );
    if( linebreak ){ // print until newline
        int len = (int)(linebreak - src);
        if( len > max_len ){ len = max_len; }
        strncpy( dest, src, len );
    } else { // can't find a new line, so just fill the buffer
        strncpy( dest, src, max_len );
    }
    return dest;
}
