#!/usr/bin/env moonlight

local server = 255
local server_public_key = 45
local script = 5000

if #arg == 3 then
  server = math.floor(tonumber(arg[1]))
  server_public_key = math.floor(tonumber(arg[2]))
  script = math.floor(tonumber(arg[3]))
elseif #arg ~= 0 then
  error("Expecting 3 arguments, got " .. #arg)
end

print([[
#ifndef MOONSHINE_IMPLANT_EXECUTABLE_PATCH_H_
#define MOONSHINE_IMPLANT_EXECUTABLE_PATCH_H_
]])
print("const char* server = \"".. string.rep("A", server) .. "\";")
print("const char* server_public_key = \"".. string.rep("B", server_public_key) .. "\";")
print("const char* script = \"".. string.rep("C", script) .. "\";")
print("")
print([[
#endif //MOONSHINE_IMPLANT_EXECUTABLE_PATCH_H_
]])
