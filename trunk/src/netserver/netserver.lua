#!/usr/bin/lua

if log_file == nil then
    log_file = io.open("netserver.log","w")
end

function log_debug(...)
    local msg = os.date() .. " [debug] " .. table.concat({...}," ")
    print(msg)
    log_file:write(msg)
    log_file:write("\n")
    log_file:flush()
end

function log_error(...)
    local msg = os.date() .. " [error] " .. table.concat({...}," ")
    print(msg)
    log_file:write(msg)
    log_file:write("\n")
    log_file:flush()
end

function copyFile(src, dst)
    local file_src = io.open(src,"r")

    if file_src == nil then
        log_error("copyFile failed: could not open ", src)
        return
    end

    local file_dst = io.open(dst,"w")

    if file_dst == nil then
        file_src:close()
        log_error("copyFile failed: could not open ", dst)
        return
    end
    
    local data = file_src:read("*a")
    file_dst:write(data)
    file_dst:flush()
    data = nil
    
    file_src:close()
    file_dst:close()
end

SERVER_VERSION = "TA3D netserver 0.0.1"

STATE_CONNECTING = 0
STATE_CONNECTED = 1
STATE_DISCONNECTED = 2

-- initialize Mysql stuffs

require("luasql.mysql")
if luasql == nil or luasql.mysql == nil then
    log_error("luasql not found")
    os.exit()
end

mysql = luasql.mysql()
if mysql == nil then
    log_error("could not initialize luasql.mysql")
    os.exit()
end

netserver_db = mysql:connect("netserver", "user", "password")
if netserver_db == nil then
    log_error("could not connect to database")
    os.exit()
end

-- do not erase clients data if set (to allow live reloading of server code)
-- the login table (since it's a hash table it's much faster than going through the whole clients table)
if clients_login == nil then
    clients_login = {}
end
-- the socket table ==> this will prevent going trough all sockets if they are not being used
if socket_table == nil then
    socket_table = {}
end
if socket_list == nil then
    socket_list = {}
end
-- the chan table
if chans == nil then
    chans = {}
end
if chans_len == nil then
    chans_len = {}
end

function fixSQL(str)
    local safe_str, n = string.gsub(str, "['\"\\]", "\\%1")
    return safe_str
end

function removeSocket(sock)
    socket_table[sock] = nil
    for i, s in ipairs(socket_list) do
        if s == sock then
            table.remove(socket_list, i)
            break
        end
    end
end

-- Tell everyone on client's chan that client is there
function joinChan(client)
    if client.state ~= STATE_CONNECTED then
        return
    end

    -- identify * to nil
    if client.chan == nil then
        client.chan = "*"
    end
    if chans[client.chan] == nil then
        chans[client.chan] = {}
        chans_len[client.chan] = 0
        sendAll("CHAN " .. client.chan)
    end
    chans_len[client.chan] = chans_len[client.chan] + 1
    chans[client.chan][client.login] = true
    for c, v in pairs(chans[client.chan]) do
        if c ~= client.login then               -- I guess the client knows he's joining the chan ... (also would crash at login time)
            clients_login[c]:send("USER " .. client.login)
        end
    end
end

-- Tell everyone on client's chan that client is leaving chan
function leaveChan(client)
    if client.state ~= STATE_CONNECTED then
        return
    end

    if client.chan == nil then
        client.chan = "*"
    end
    if chans[client.chan] ~= nil and chans_len[client.chan] == 1 then
        chans[client.chan] = nil
        chans_len[client.chan] = nil
        sendAll("DECHAN " .. client.chan)
    else
        chans_len[client.chan] = chans_len[client.chan] - 1
        chans[client.chan][client.login] = nil
        for c, v in pairs(chans[client.chan]) do
            if c ~= client.login then           -- I guess the client knows he's leaving the chan ...
                clients_login[c]:send("LEAVE " .. client.login)
            end
        end
    end
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
    if clients_login[login] then
        clients_login[login]:send("MESSAGE you have been banned")
        clients_login[login]:disconnect()
    end
end

-- Unban a client
function unbanClient(login)
    netserver_db:execute("UPDATE `clients` SET `banned` = '0' WHERE `login`='" .. fixSQL(login) .. "'")
end

-- Kill a client
function killClient(login)
    netserver_db:execute("DELETE FROM `clients` WHERE `login`='" .. fixSQL(login) .. "'")
    if clients_login[login] then
        clients_login[login]:send("MESSAGE you have been killed")
        clients_login[login]:disconnect()
    end
end

-- Get a value from the mysql database (info table)
function getValue(name)
    cur = netserver_db:execute("SELECT value FROM `info` WHERE name='" .. fixSQL(name) .. "'")
    if cur == nil or cur == 0 or cur:numrows() ~= 1 then
        return nil
    end
    row = cur:fetch({}, "a")
    return row.value
end

-- Broadcast a message to all admins
function sendAdmins(msg)
    for id, s in ipairs(socket_list) do
        local c = socket_table[s]
        if c ~= nil and c.state == STATE_CONNECTED and c.admin == 1 then
            c:send(msg)
        end
    end
end

-- Send all
function sendAll(msg)
    for id, s in ipairs(socket_list) do
        local c = socket_table[s]
        if c ~= nil and c.state == STATE_CONNECTED then
            c:send(msg)
        end
    end
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
            if client.login == nil then
                log_debug(client.sock:getpeername() .. " (nil) sent: ", msg)
            else
                log_debug(client.sock:getpeername() .. " (" .. client.login .. ") sent: ", msg)
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
                        if clients_login[client.login] ~= nil then
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

                -- GET USER LIST : client is asking for the client list (clients on the same chan)
                if args[1] == "GET" and #args >= 3 and args[2] == "USER" and args[3] == "LIST" then
                    if client.chan == nil then
                        client.chan = "*"
                    end
                    for c, v in pairs(chans[client.chan]) do
                        client:send("USER " .. c)
                    end
                -- GET CHAN LIST : client is asking for the chan list
                elseif args[1] == "GET" and #args >= 3 and args[2] == "CHAN" and args[3] == "LIST" then
                    for c, v in pairs(chans) do
                        client:send("CHAN " .. c)
                    end
                -- GET CLIENT LIST : list ALL clients
                elseif args[1] == "GET" and #args >= 3 and args[2] == "CLIENT" and args[3] == "LIST" then
                    for id, s in ipairs(socket_list) do
                        local c = socket_table[s]
                        if c ~= nil and c.state == STATE_CONNECTED then
                            client:send("CLIENT " .. c.login)
                        end
                    end
                -- SEND to msg : client is sending a message to another client
                elseif args[1] == "SEND" and #args >= 3 then
                    if args[2] ~= nil and args[2] ~= client.login and clients_login[args[2]] ~= nil and clients_login[args[2]].state == STATE_CONNECTED then
                        clients_login[args[2]]:send("MSG " .. client.login .. " " .. table.concat(args, " ", 3))
                    end
                -- SENDALL msg : client is sending a message to other clients in the same chan
                elseif args[1] == "SENDALL" and #args >= 2 then
                    if client.chan == nil then
                        client.chan = "*"
                    end
                    if chans[client.chan] ~= nil then
                        for c, v in pairs(chans[client.chan]) do
                            if c ~= client.login then
                                clients_login[c]:send("MSG " .. client.login .. " " .. table.concat(args, " ", 2))
                            end
                        end
                    end
                -- KICK user : admin privilege, disconnect someone (self kick works)
                elseif args[1] == "KICK" and #args == 2 then
                    if client.admin == 1 then
                        if clients_login[args[2]] ~= nil then
                            clients_login[args[2]]:send("MESSAGE You have been kicked")
                            clients_login[args[2]]:disconnect()
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
                -- RELOAD SERVER : admin privilege, live reload of server code (update server without closing any connection)
                elseif args[1] == "RELOAD" and args[2] == "SERVER" then
                    if client.admin == 1 then
                        _G.reload = true
                    else
                        client:send("ERROR you don't have the right to do that")
                    end
                -- SHUTDOWN SERVER : admin privilege, stop server (closes all connections)
                elseif args[1] == "SHUTDOWN" and args[2] == "SERVER" then
                    if client.admin == 1 then
                        for id = #socket_list, 1, -1 do
                            local s = socket_list[id]
                            if s ~= nil then
                                local c = socket_table[s]
                                if c ~= nil then
                                    c:send("MESSAGE Server is being shut down for maintenance, sorry for the inconvenience")
                                    c:disconnect()
                                end
                            end
                        end
                        log_debug("Server is being shut down")
                        os.exit()
                    else
                        client:send("ERROR you don't have the right to do that")
                    end
                -- CRASH SERVER: admin privilege, crash the server (thus resuming previous server version ... be careful with that)
                elseif args[1] == "CRASH" and args[2] == "SERVER" then
                    if client.admin == 1 then
                        error(table.concat(args," ",3))
                    else
                        client:send("ERROR you don't have the right to do that")
                    end
                -- CHAN chan_name: change chan for client
                elseif args[1] == "CHAN" then
                    leaveChan(client)
                    client.chan = args[2]
                    joinChan(client)
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
                                    clients_login[this.login] = this           -- we need this to do fast look ups
                                    this.state = STATE_CONNECTED
                                    this:send("CONNECTED")
                                    joinChan(this)
                                end,
                    disconnect = function(this)
                                    if this.login ~= nil then       -- allow garbage collection
                                        clients_login[this.login] = nil
                                    end
                                    removeSocket(this.sock)
                                    leaveChan(this)                 -- don't forget to leave the chan!
                                    this:send("CLOSE")
                                    this.sock:close()
                                    this.state = STATE_DISCONNECTED
                                end}
    client.sock:settimeout(0)
    client:send("SERVER " .. SERVER_VERSION)
    socket_table[client.sock] = client
    table.insert(socket_list, client.sock)
end

--************************************************************************--
--                                                                        --
--                              TA3D Netserver                            --
--                                                                        --
--************************************************************************--

-- This is the server monitor, it is responsible for warm restart and crash management
if _G.reload == nil then
    log_debug("Server monitor started")
    local chunks = {}
    while true do
        local reloadFile = _G.reload or #chunks == 0
        if reloadFile then
            if _G.reload then
                log_debug()
                log_debug("--- Warm restart ---")
                log_debug()
            end
            local chunk = loadfile("netserver.lua")
            if chunk ~= nil then
                if _G.reload then
                    sendAdmins("MESSAGE Warm restart in progress")
                end
                table.insert(chunks, chunk)
            else
                if _G.reload then
                    sendAdmins("MESSAGE Warm restart failed, resuming current version")
                    log_error("could not load netserver.lua! Warm restart failed")
                    log_debug("resuming current server version")
                else
                    log_error("could not load netserver.lua! Server start failed")
                end
            end
        end

        -- run the last available chunk (in case it crashes, it'll remove the last one and try with the previous one :) )
        if #chunks > 0 then
            local chunk = chunks[ #chunks ]
            _G.reload = true
            local success, msg = pcall(chunk)
            -- on crash
            if not success then
                log_error(msg)
                log_error("server crashed, resuming previous working version")
                table.remove(chunks)
                _G.crashRecovery = true
            -- on exit (on reload request we just loop and load the new version)
            elseif _G.reload == nil then
                break
            end
        end
    end
    -- we're in monitor mode so don't run the server on exit :p
    os.exit()
end

-- Normal server code

log_debug("Starting " .. SERVER_VERSION)

local socket = require("socket")

if socket == nil then
    log_debug("error: luasocket not found")
    os.exit()
end

-- loads the luasocket library
if server == nil then
    server = socket.bind("*", 4240)

    if server == nil then
        log_debug("error: could not create a TCP server socket :(")
        os.exit()
    end
end

server:settimeout(0.001)

-- If server has been restarted, then update coroutines
if _G.reload and #socket_list > 0 then
    log_debug("warm restart detected, updating clients coroutines and socket_table")
    for s, c in pairs(socket_table) do
        c.serve = coroutine.wrap(processClient)
    end
    sendAdmins("MESSAGE Warm restart successful")
end
socket_list[1] = server

-- Wow, we've just recovered from a crash :s
if _G.crashRecovery then
    _G.crashRecovery = nil
    sendAdmins("MESSAGE Server just recovered from a crash")
end


-- prevent it from restarting in an infinite loop
_G.reload = nil

while true do
    local read, write, err = socket.select( socket_list, nil, 10 )
    print(#read,"/",#socket_list, err)
    local incoming = server:accept()
    if incoming ~= nil then
        log_debug("incoming connection from ", incoming:getpeername() )
        -- ok, we have a new client, we add it to the client list
        newClient(incoming)
    end
    
    for i, s in ipairs(read) do
        if s ~= server then
            local c = socket_table[s]
            if c == nil then
                if socket_list[s] ~= nil then
                    table.remove(socket_list, socket_list[s])
                end
                socket_list[s] = nil
            else
                if c.state == STATE_DISCONNECTED then
                    -- If the client has disconnected, remove it from the clients table
                    removeSocket(c.sock)
                else
                    -- Serve all clients
                    c:serve()
                end
            end
        end
    end
   
    -- in case we want to reload the server, just return and let the monitor do the job
    if _G.reload == true then
        return
    end
end
