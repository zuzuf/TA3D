function sleep(delay)
    coroutine.yield(delay, 1)
end

function wait()
    coroutine.yield(0, 2)
end

function end_thread()
    coroutine.yield(0, 4)
end

function yield()
    coroutine.yield()
end

function set_signal_mask(sig)
    coroutine.yield(sig, 3)
end

function vector()
    local vec = {x=0, y=0, z=0}
    return setmetatable(vec, __vector_metatable)
end

-- conservative functions that don't overwrite "permanent" objects but produce a temporary one
function __vec_add(v0,v1)
    return setmetatable({x=v0.x + v1.x, y=v0.y + v1.y, z=v0.z + v1.z}, __fast_vector_metatable)
end

function __vec_sub(v0,v1)
    return setmetatable({x=v0.x - v1.x, y=v0.y - v1.y, z=v0.z - v1.z}, __fast_vector_metatable)
end

function __vec_mul(m,v)
    return setmetatable({x=m * v.x, y=m * v.y, z=m * v.z}, __fast_vector_metatable)
end

function __vec_dot(v0,v1)
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
end

function __vec_cross(v0,v1)
    return setmetatable({x=v0.y * v1.z - v0.z * v1.y, y=v0.z * v1.x - v0.x * v1.z, z=v0.x * v1.y - v0.y * v1.x}, __fast_vector_metatable)
end

-- convert a temporary object into a "permanent" one
function fix_vector(v)
    return setmetatable(v, __vector_metatable)
end

-- use fast functions for temporary objects (we don't need to create new tables since we already have one we won't reuse)
function __fast_vec_add(v0,v1)
    v0.x = v0.x + v1.x
    v0.y = v0.y + v1.y
    v0.z = v0.z + v1.z
    return v0
end

function __fast_vec_sub(v0,v1)
    v0.x = v0.x - v1.x
    v0.y = v0.y - v1.y
    v0.z = v0.z - v1.z
    return v0
end

function __fast_vec_mul(m,v)
    v.x = m * v.x
    v.y = m * v.y
    v.z = m * v.z
    return v
end

__vector_metatable = {__add=__vec_add, __sub=__vec_sub, __mul=__vec_mul, __mod=__vec_dot, __pow=__vec_cross}
__fast_vector_metatable = {__add=__fast_vec_add, __sub=__fast_vec_sub, __mul=__fast_vec_mul, __mod=__vec_dot, __pow=__vec_cross}

-- normalize v, modifies the content of v
function normalize(v)
    local l = 1.0 / math.sqrt( v * v )
    v.x = l * v.x
    v.y = l * v.y
    v.z = l * v.z
    return v;
end

-- returns the normalized version of v, doesn't change v
function normalized(v)
    local l = 1.0 / math.sqrt( v * v )
    local tmp = vector()
    tmp.x = l * v.x
    tmp.y = l * v.y
    tmp.z = l * v.z
    return tmp;
end

function game_signal( sig )
    coroutine.yield(sig)
end

function set_cam_mode( mode )
    if mode == CAM_NORMAL_MODE then
        game_signal( SIGNAL_CAM_DEF )
    else
        if mode == CAM_FREE_MODE then
            game_signal( SIGNAL_FREECAM )
        end
    end
end

function wait_for_move(obj, axis)
    while is_moving(obj, axis) do
        coroutine.yield()
    end
end

function wait_for_turn(obj, axis)
    while is_turning(obj, axis) do
        coroutine.yield()
    end
end
