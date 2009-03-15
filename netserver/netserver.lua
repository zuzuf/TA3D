#!/usr/bin/lua

function log_debug(...)
    print(...)
end

SERVER_VERSION = "TA3D netserver 0.0.0"

STATE_CONNECTING = 0
STATE_CONNECTED = 1
STATE_DISCONNECTED = 2

-- initialize Mysql stuffs

require("luasql.mysql")
if luasql == nil or luasql.mysql == nil then
    log_debug("error: luasql not found")
    os.exit()
end

mysql = luasql.mysql()
if mysql == nil then
    log_debug("error: could not initialize luasql.mysql")
    os.exit()
end

netserver_db = mysql:connect("netserver", "user", "password")
if netserver_db == nil then
    log_debug("error: could not connect to database")
    os.exit()
end

-- the client table, contains all client structures
clients = {}

function fixSQL(str)
    local safe_str, n = string.gsub(str, "['\"\\]", "\\%1")
    return safe_str
end

-- Identify a client, connect it if password and login match
function identifyClient(client, password)
    cur = netserver_db:execute("SELECT * FROM `clients` WHERE `login`='" .. fixSQL(client.login) .. "' AND `password`='" .. fixSQL(password) .. "' AND `banned`=0")
    if cur == nil or cur == 0 or cur:numrows() ~= 1 then
        return false
    end
    row = cur:fetch({}, "a")
    client.ID = tonumber( row.ID )
    client.admin = tonumber( row.admin )
    return true
end

-- Register a new client if there is no account with the same name
function registerClient(client, password)
    cur = netserver_db:execute("SELECT * FROM `clients` WHERE `login`='" .. fixSQL(client.login) .. "'")
    if cur == nil or cur == 0 or cur:numrows() ~= 0 then
        return false
    end
    
    cur = netserver_db:execute("INSERT INTO clients(`login`, `password`) VALUES('" .. fixSQL(client.login) .. "','" .. fixSQL(password) .. "')")
    if cur == nil or cur == 0 then
        return false
    end
    
    return identifyClient(client, password)
end

-- Ban a client
function banClient(login)
    netserver_db:execute("UPDATE `clients` SET `banned` = '1' WHERE `login`='" .. fixSQL(login) .. "'")
    if _G[login] then
        _G[login]:send("MESSAGE you have been banned")
        _G[login]:disconnect()
    end
end

-- Unban a client
function unbanClient(login)
    netserver_db:execute("UPDATE `clients` SET `banned` = '0' WHERE `login`='" .. fixSQL(login) .. "'")
end

-- Kill a client
function killClient(login)
    netserver_db:execute("DELETE FROM `clients` WHERE `login`='" .. fixSQL(login) .. "'")
    if _G[login] then
        _G[login]:send("MESSAGE you have been killed")
        _G[login]:disconnect()
    end
end

function getValue(name)
    cur = netserver_db:execute("SELECT value FROM `info` WHERE name='" .. fixSQL(name) .. "'")
    if cur == nil or cur == 0 or cur:numrows() ~= 1 then
        return nil
    end
    row = cur:fetch({}, "a")
    return row.value
end

-- this is where the magic tooks place
function processClient(client)
    while true do
        local msg, err = client:receive()
        -- if an error is detected, close the connection
        if err ~= nil and err ~= "timeout" then
            log_debug("socket error:", err)
            client:disconnect()
            return
        end
        -- what is client telling us ?
        if msg ~= nil then
            log_debug(client.sock:getpeername(), " sent: ", msg)
            -- this is temporary
            if msg == "end" then
                os.exit()
            end

            if msg == "DISCONNECT" then
                client:disconnect()
                return
            end

            -- parse words
            args = {}
            for w in string.gmatch(msg, "[^%s]+") do
                table.insert(args, w)
            end

            -- parsing error: let the main loop restart this function for us            
            if #args == 0 then
                return
            end
            
            if client.state == STATE_CONNECTING then        -- Client is not connected
                -- login command: CLIENT VERSION"
                if args[1] == "CLIENT" then
                    client.version = table.concat(args, " ", 2)
                    log_debug("client version ", client.version, " registered")
                    if client.version ~= getValue("LAST_VERSION") then
                        log_debug("new version available, sending update notification")
                        client:send("MESSAGE " .. getValue("UPDATE_NOTIFICATION"))
                    end

                -- login command: LOGIN USER PASSWORD"
                elseif args[1] == "LOGIN" and #args == 3 then
                    if client.version == nil then
                        client:send("ERROR client version unknown, send version first")
                    else
                        client.login = args[2]
                        if _G[client.login] ~= nil then
                            client.login = nil
                            client:send("ERROR session already opened")
                        else
                            local password = args[3]
                            local success = identifyClient(client, password)
                            if success then
                                client:connect()
                            else
                                client.login = nil
                                client:send("ERROR login or password incorrect")
                            end
                        end
                    end
                -- login command: REGISTER USER PASSWORD"
                elseif args[1] == "REGISTER" and #args == 3 then
                    if client.version == nil then
                        client:send("ERROR client version unknown, send version first")
                    else
                        client.login = args[2]
                        local password = args[3]
                        local success = registerClient(client, password)
                        if success then
                            client:connect()
                        else
                            client.login = nil
                            client:send("ERROR login already used")
                        end
                    end
                else
                    client:send("ERROR could not parse request")
                end
            elseif client.state == STATE_CONNECTED then     -- Client is connected

                -- GET CLIENT LIST : client is asking for the client list
                if args[1] == "GET" and #args >= 3 and args[2] == "CLIENT" and args[3] == "LIST" then
                    for id, c in ipairs(clients) do
                        if c.state == STATE_CONNECTED then
                            client:send("USER " .. c.login)
                        end
                    end
                -- SEND to msg : client is sending a message to another client
                elseif args[1] == "SEND" and #args >= 3 then
                    if args[2] ~= nil and args[2] ~= client.login and _G[args[2]] ~= nil and _G[args[2]].state == STATE_CONNECTED then
                        _G[args[2]]:send("MSG " .. client.login .. " " .. table.concat(args, " ", 3))
                    end
                -- KICK user : admin privilege, disconnect someone (self kick works)
                elseif args[1] == "KICK" and #args == 2 then
                    if client.admin == 1 then
                        if _G[args[2]] ~= nil then
                            _G[args[2]]:send("MESSAGE You have been kicked")
                            _G[args[2]]:disconnect()
                        end
                    else
                        client:send("ERROR you don't have the right to do that")
                    end
                -- BAN user : admin privilege, disconnect someone and prevent him from reconnecting (self ban works)
                elseif args[1] == "BAN" and #args == 2 then
                    if client.admin == 1 then
                        banClient(args[2])
                    else
                        client:send("ERROR you don't have the right to do that")
                    end
                -- UNBAN user : admin privilege, remove ban flag on a user (self unban works ... but you've been banned you don't get here)
                elseif args[1] == "UNBAN" and #args == 2 then
                    if client.admin == 1 then
                        unbanClient(args[2])
                    else
                        client:send("ERROR you don't have the right to do that")
                    end
                -- KILL user : admin privilege, remove a user account (self kill doesn't work)
                elseif args[1] == "KILL" and #args == 2 then
                    if client.admin == 1 and client.login ~= args[2] then
                        killClient(args[2])
                    else
                        client:send("ERROR you don't have the right to do that")
                    end
                else
                    client:send("ERROR could not parse request")
                end
            else -- Duhh oO, what are we doing here ?
                client:disconnect()
            end
        end
    
        -- let the others access the server too
        coroutine.yield()
    end
end

-- create a new client structure linked to the incoming socket
function newClient(incoming)
    local client = {sock = incoming,
                    state = STATE_CONNECTING,
                    send =  function(this,msg)
                                this.sock:send(msg .. "\n")
                            end,
                    receive =   function(this)
                                    return this.sock:receive()
                                end,
                    ID = nil,
                    login = nil,
                    version = nil,
                    serve = coroutine.wrap(processClient),
                    connect =   function(this)
                                    _G[this.login] = this           -- we need this to do fast look ups
                                    this.state = STATE_CONNECTED
                                    this:send("CONNECTED")
                                end,
                    disconnect = function(this)
                                    if this.login ~= nil then       -- allow garbage collection
                                        _G[this.login] = nil
                                    end
                                    this:send("CLOSE")
                                    this.sock:close()
                                    this.state = STATE_DISCONNECTED
                                end}
    client.sock:settimeout(0)
    client:send("SERVER " .. SERVER_VERSION)
    return client
end

--************************************************************************--
--                                                                        --
--                              TA3D Netserver                            --
--                                                                        --
--************************************************************************--

log_debug("Starting " .. SERVER_VERSION)

local socket = require("socket")

if socket == nil then
    log_debug("error: luasocket not found")
    os.exit()
end

-- loads the luasocket library
local server = socket.bind("*", 4240)

if server == nil then
    log_debug("error: could not create a TCP server socket :(")
    os.exit()
end

server:settimeout(0.001)

while true do
    local incoming = server:accept()
    if incoming ~= nil then
        log_debug("incoming connection from ", incoming:getpeername() )
        -- ok, we have a new client, we add it to the client list
        table.insert(clients, newClient(incoming))
    end
    
    for i = 1, #clients do
        if clients[i] == nil then
            break
        end
        if clients[i].state == STATE_DISCONNECTED then
            -- If the client has disconnected, remove it from the clients table
            table.remove(clients, i)
        else
            -- Serve all clients
            clients[i]:serve()
        end
    end
end
