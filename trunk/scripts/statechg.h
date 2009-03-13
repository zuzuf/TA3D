-- StateChg.h -- Generic State Change support for units that activate and deactivate or whatever

-- Due to limitiations of the scripting language, this file must be included twice.  The
-- first time must be where the static variables are declared.  The second time must be
-- where the functions are defined (and of course before they are called.)

-- The Following macros must be defined:  ACTIVATECMD and DEACTIVATECMD.  They are the commands
-- to run when the units is actiavted or deactivated.

-- State variables

-- The states that can be requested

ACTIVE      = 0
INACTIVE    = 1

-- State change request functions

function InitState()
    -- Initial state
    statechg_DesiredState = INACTIVE
    statechg_StateChanging = false
end

function RequestState( requestedstate )
    local actualstate

    -- Is it busy?
    if statechg_StateChanging then
        -- Then just tell it how we want to end up.  A script is already running and will take care of it.
        statechg_DesiredState = requestedstate
        return 0
    end

    -- Keep everybody else out
    statechg_StateChanging = true

    -- Since nothing was running, the actual state is the current desired state
    actualstate = statechg_DesiredState

    -- State our desires
    statechg_DesiredState = requestedstate

    -- Process until everything is right and decent
    while statechg_DesiredState ~= actualstate do
        -- Change the state

        if statechg_DesiredState == ACTIVE then
            ACTIVATECMD()
            actualstate = ACTIVE
        end

        if statechg_DesiredState == INACTIVE then
            DEACTIVATECMD()
            actualstate = INACTIVE
        end
        
        yield()
    end

    -- Okay, we are finshed
    statechg_StateChanging = false
end

