--
-- exptype.h -- Explosion Type values for Lua scripts
--

SHATTER         = 1     -- The piece will shatter instead of remaining whole
EXPLODE_ON_HIT  = 2     -- The piece will explode when it hits the ground
FALL            = 4     -- The piece will fall due to gravity instead of just flying off
SMOKE           = 8     -- A smoke trail will follow the piece through the air
FIRE            = 16    -- A fire trail will follow the piece through the air
BITMAPONLY      = 32    -- The piece will not fly off or shatter or anything.  Only a bitmap explosion will be rendered.


-- Bitmap types

BITMAP1         = 256
BITMAP2         = 512
BITMAP3         = 1024
BITMAP4         = 2048
BITMAP5         = 4096
BITMAPNUKE      = 8192

BITMAPMASK      = 16128    -- Mask of the possible bitmap bits

-- Indices for set/get value, most of these values are here for backward compatibility with OTA script style
ACTIVATION          = 1     -- set or get
STANDINGMOVEORDERS  = 2     -- set or get
STANDINGFIREORDERS  = 3     -- set or get
HEALTH              = 4     -- get (0-100%)
INBUILDSTANCE       = 5     -- set or get
BUSY                = 6     -- set or get (used by misc. special case missions like transport ships)
PIECE_XZ            = 7     -- get
PIECE_Y             = 8     -- get
UNIT_XZ             = 9     -- get
UNIT_Y              = 10    -- get
UNIT_HEIGHT         = 11    -- get
XZ_ATAN             = 12    -- get atan of packed x,z coords
XZ_HYPOT            = 13    -- get hypot of packed x,z coords
ATAN                = 14    -- get ordinary two-parameter atan
HYPOT               = 15    -- get ordinary two-parameter hypot
GROUND_HEIGHT       = 16    -- get
BUILD_PERCENT_LEFT  = 17    -- get 0 = unit is built and ready, 1-100 = How much is left to build
YARD_OPEN           = 18    -- set or get (change which plots we occupy when building opens and closes)
BUGGER_OFF          = 19    -- set or get (ask other units to clear the area)
ARMORED             = 20    -- set or get

MIN_ID                  = 69      -- returns the lowest valid unit ID number
MAX_ID                  = 70      -- returns the highest valid unit ID number
MY_ID                   = 71      -- returns ID of current unit
UNIT_TEAM               = 72      -- returns team(player ID in TA) of unit given with parameter
UNIT_BUILD_PERCENT_LEFT = 73      -- basically BUILD_PERCENT_LEFT, but comes with a unit parameter
UNIT_ALLIED             = 74      -- is unit given with parameter allied to the unit of the current COB script. 0=not allied, not zero allied
UNIT_IS_ON_THIS_COMP    = 75      -- indicates if the 1st parameter(a unit ID) is local to this computer
VETERAN_LEVEL           = 32      -- gets kills * 100

x_axis = 0
y_axis = 1
z_axis = 2

