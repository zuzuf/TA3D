#!/usr/bin/lua

local socket = require("socket")

local client, err = socket.connect(arg[1], 4240)

if err then
    print("socket error : " .. err)
    os.exit()
end

client:settimeout(0)

while true do
    local msg = client:receive()
    
    if msg ~= nil then
        print("server: ", msg)
    end
    
    io.stdout:write("> ")
    local line = io.stdin:read("*l")
    if line == "exit" then
        os.exit()
    end
    if line ~= "" then
        print("sending `" .. line .. "`")
        client:send(line .. "\n")
    end
end
