#pragma once

#include <stm32f7xx.h>

/*
 * caw is the usb<->lua layer which allows usb driver to interact
 * directly with the lua engine through a tty modem emulation acting
 * as a direct serial port.
 *
 * there is no repl built-in anyway, instead all we have is a direct
 * stream of chars into crow from the usb (typed in minicom, or streamed
 * from a norns app / laptop), then we send a stream of chars back.
 *
 * when crow is receing, how do we know when to call the Lua function?
 * there is no defined 'enter' command, or 'execute'. but we don't want
 * to exec the lua command every character.
 *
 * data sent to crow:
 *      single chars from minicom
 *      strings from a stream of lua code from a future app / norns
 *      command sequences saying "exec the chunk" or similar
 * nb: these commands may cause data to be returned, but the caw layer is unaware!!
 *
 * point is, there is only a single callback into lua, with the data,
 * but the data will only be sent once the input stream is recognized as complete
 *
 * we can switch on the input type:
 *      if a single char is received
 *          check if it's \r (or \n?) and 'exec' the buffer
 *          else add the char to the buffer (and echo to sender)
 *      if a multi-char is received
 *          check if last char is \r (\n) and exec if so
 *          else add to buffer
 *              then check if buffer ends in escape sequence (<!<) to exec chunk
 * nb: while receiving a chunk, set a status bit so no echo occurs, but instead
 *     a status report is returned saying a chunk of size(x) was received
 *
 * when crow sending it will send data of types:
 *      echo'd chars / linefeed / prompt in repl mode
 *      debug info from lua (errors etc)
 *      value (preceded by special char '#')
 *      lua chunk (surrounded in escape sequences "<<<" ">>>")
 *
 * */

uint8_t Caw_Init( void );
void Caw_DeInit( void );

void Caw_send_rawtext( char* text, uint32_t len );
void Caw_send_luachunk( char* text );
void Caw_send_value( uint8_t type, float value ); // enum the type

// Call from main scheduler to process USB buffer
void Caw_try_receive( void );

// Callback. Define in top-level program
void Caw_receive_callback( uint8_t* buf, uint32_t len );
//__weak void Caw_receive_callback( uint8_t* buf, uint32_t len );
