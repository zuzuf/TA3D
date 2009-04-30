#!/usr/bin/lua

local socket = require("socket")

if arg[1] == nil then
    arg[1] = "localhost"
end

local client, err = socket.connect(arg[1], 4240)

if err then
    print("socket error : " .. err)
    os.exit()
end

client:settimeout(0)

client:send("CLIENT NetClient\n")

while true do
    local msg = client:receive()
    
    if msg ~= nil then
        print("server: ", msg)
        
        if msg == "CLOSE" then
            print("leaving ...")
            client:close()
            os.exit()
        end
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
