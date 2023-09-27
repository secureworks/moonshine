#!/usr/bin/env luajit

local mcompress = require 'mcompress'

local description = [=[
Usage: bin2c.lua <filename> <name> [prefix] [suffix]
]=]

if not arg or not arg[2] then
  io.stderr:write(description)
  return
end

local filename = arg[1]
local name = arg[2]
local prefix = arg[3] ~= nil and "_" .. arg[3] or ""
local suffix = arg[4] ~= nil and "_" .. arg[4] or ""
local content = assert(io.open(filename,"rb")):read"*a"

local dump do
  local numtab={}; for i=0,255 do numtab[string.char(i)]=("%3d,"):format(i) end
  function dump(str)
    return (str:gsub(".", numtab):gsub(("."):rep(80), "%0\n  "))
  end
end

local compressed = mcompress.compress(content)
local code_array = dump(compressed)
local code_size = #compressed

local template = [[
#ifndef {{prefix}}{{name}}{{suffix}}_h_
#define {{prefix}}{{name}}{{suffix}}_h_

inline const unsigned char {{prefix}}{{name}}{{suffix}}_array[{{code_size}}] = {
  {{code_array}}
};
inline const unsigned int {{prefix}}{{name}}{{suffix}}_size = {{code_size}};

#endif // {{prefix}}{{name}}{{suffix}}_h_
]]

local output = template:gsub("{{prefix}}", prefix):gsub("{{suffix}}", suffix):gsub("{{name}}", name)
output = output:gsub("{{code_array}}", code_array):gsub("{{code_size}}", code_size)

io.write(output)