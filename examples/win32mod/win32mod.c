#include <lua.h>
#include <lauxlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define LUA_WIN32_DEFAULT_CODE_PAGE CP_UTF8

static UINT codePage = LUA_WIN32_DEFAULT_CODE_PAGE;

static WCHAR *decode_string(const char *s) {
  DWORD size;
  WCHAR *ws = NULL;
  if (s != NULL) {
    size = MultiByteToWideChar(codePage, 0, s, -1, 0, 0);
    ws = (WCHAR *)GlobalAlloc(GMEM_FIXED, sizeof(WCHAR) * size);
    if (ws != NULL) {
      MultiByteToWideChar(codePage, 0, s, -1, ws, size);
    }
  }
  return ws;
}

static char *encode_string(WCHAR *ws) {
  int n;
  char *s = NULL;
  if (ws != NULL) {
    n = WideCharToMultiByte(codePage, 0, ws, -1, NULL, 0, NULL, NULL);
    s = (char *)GlobalAlloc(GMEM_FIXED, n);
    if (s != NULL) {
      WideCharToMultiByte(codePage, 0, ws, -1, s, n, NULL, NULL);
    }
  }
  return s;
}

static int win32mod_MessageBox(lua_State *l) {
  HWND hwndOwner = NULL;
  WCHAR *text = decode_string(luaL_optstring(l, 1, NULL));
  WCHAR *caption = decode_string(luaL_optstring(l, 2, NULL));
  unsigned int type = luaL_optinteger(l, 3, MB_OK);
  int result = MessageBoxW(hwndOwner, text, caption, type);
  lua_pushinteger(l, result);
  return 1;
}

int __declspec(dllexport) luaopen_win32mod(lua_State *L) {
  lua_newtable(L);
  lua_pushcfunction (L, win32mod_MessageBox);
  lua_setfield (L, -2, "MessageBox");
  return 1;
}
