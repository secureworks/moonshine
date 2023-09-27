#ifndef MOON_OUTPUT_H_
#define MOON_OUTPUT_H_

#include "lua.h"

#include <stdarg.h>

typedef void (*output_callback)(lua_State *L, const void* data, size_t length, void *userdata);
/*LUA_API*/ void moon_output_cb(lua_State *L, output_callback callback, void *userdata);
int moon_print (lua_State *L);

void push_output(lua_State *L, const char *str);
void push_raw_output(lua_State *L, const void *data, size_t len);
void push_char_output(lua_State *L, int i);
void push_printf_output(lua_State *L, char *fmt, ...);

/* print a string */
#define moon_writestring(lua_state, string) \
        do { push_output((lua_state), (string)); } while (0)

#define moon_writestringlength(lua_state, string, length) \
        do { push_raw_output((lua_state), (string), ((length) * sizeof(char))); } while (0)

#define moon_writechar(lua_state, i) \
        do { push_char_output((lua_state), (i)); } while (0)

/* print a newline and flush the output */
#define moon_writeline(lua_state) \
        moon_writestringlength(lua_state, "\n", 1)

/* print an error message */
#define moon_writestringerror(lua_state, s, p) \
        do { push_printf_output(lua_state, (s), (p)); } while (0)

/* }====================================================== */

#endif //MOON_OUTPUT_H_
