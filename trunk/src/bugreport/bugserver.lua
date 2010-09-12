#!/usr/bin/lua

if log_file == nil then
    log_file = io.open("bugserver.log","w")
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

local STATE_CONNECTING = 0
local STATE_CONNECTED = 1
local STATE_DISCONNECTED = 2
local SERVER_VERSION = "TA3D bugserver 0.0.0"

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

cfg_file = io.open("db.cfg")
if cfg_file ~= nil then
    db_host = cfg_file:read()
    db_user = cfg_file:read()
    db_pass = cfg_file:read()
    cfg_file:close()
else
    db_host = "localhost"
    db_user = "user"
    db_pass = "password"
end
bugserver_db = mysql:connect("bugserver", db_user, db_pass, db_host)
if bugserver_db == nil then
    log_error("could not connect to database")
    os.exit()
end

function mysql_reconnect()        -- when connection to MySQL database is closed, let's reopen it :D
    log_debug("reconnecting to MySQL server")
    bugserver_db:close()
    log_debug("connection to MySQL server closed")
    bugserver_db = mysql:connect("bugserver", db_user, db_pass, db_host)
    if bugserver_db then
        log_debug("connected to MySQL server")
    else
        log_error("impossible to connect to MySQL server!")
    end
end

function mysql_safe_request(req)
    local cur, err = bugserver_db:execute(req)
    if err ~= nil then
        log_error( err )
        log_debug( "I am going to reconnect to MySQL server and retry request : '" .. req .. "'")
        mysql_reconnect()
        cur, err = bugserver_db:execute(req)
    end
    return cur, err
end

function fixSQL(str)
    local safe_str, n = string.gsub(str, "['\"\\]", "\\%1")
    return safe_str
end

function escape(str)
    local escaped_str, n = string.gsub(str, "[\"\\]", "\\%1")
    escaped_str, n = string.gsub(escaped_str, "\n", "\\n")
    return escaped_str
end

-- this is where the magic takes place
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
            log_debug("msg: ", msg)

            if msg == "DISCONNECT" then
                client:disconnect()
                return
            end

            if client.state == STATE_CONNECTING then        -- Client is not connected
                -- login command: BUG REPORT
                if msg == "BUG REPORT" then
                    client.state = STATE_CONNECTED
                    client.report = ""
                else
                    client:send("ERROR could not parse request")
                    client:disconnect()
                end
            elseif client.state == STATE_CONNECTED then     -- Client is connected, msg is a part of the bug report
                client.report = client.report .. msg .. "\n"
            end
        end
    
        -- let the others access the server too
        coroutine.yield()
    end
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

function processReport(report)
    if report == nil then
        return
    end
    local cur, err = mysql_safe_request("INSERT INTO bugreports(`report`) VALUES('" .. fixSQL(report) .. "')")
    if cur == nil or cur == 0 then
        log_error( err )
        return
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
                    serve = coroutine.wrap(processClient),
                    report = nil,
                    disconnect = function(this)
                                    removeSocket(this.sock)
                                    this:send("CLOSE")
                                    this.sock:close()
                                    this.state = STATE_DISCONNECTED
                                    -- do we have something to put in the database ?
                                    processReport(this.report)
                                end}
    client.sock:settimeout(0)
    client:send("SERVER \"" .. escape(SERVER_VERSION) .. "\"")
    socket_table[client.sock] = client
    table.insert(socket_list, client.sock)
end

--************************************************************************--
--                                                                        --
--                              TA3D Bugserver                            --
--                                                                        --
--************************************************************************--

log_debug("Starting " .. SERVER_VERSION)

local socket = require("socket")

if socket == nil then
    log_debug("error: luasocket not found")
    os.exit()
end

-- loads the luasocket library
server = socket.bind("*", 1905)	-- bug = (2 * 26 + 21) * 26 + 7 = 1905

if server == nil then
    log_debug("error: could not create a TCP server socket :(")
    os.exit()
end

server:settimeout(0.001)

socket_table = {}
socket_list = {}
socket_list[1] = server;

while true do
    local read, write, err = socket.select( socket_list, nil, 10 )
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
end
