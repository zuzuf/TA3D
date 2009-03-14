-- Arm Energy Storage Facility

piece( "base", "texture" )

SMOKEPIECE1 = base
#include "smokeunit.h"
#include "EXPtype.h"

function Create()
	dont_cache( texture )
	dont_shade( texture )
	start_script( SmokeUnit )
end

function SweetSpot()
	return base
end

function Killed( severity )
	if severity <= 25 then
		explode( base, BITMAPONLY + BITMAP1 )
		explode( texture, BITMAPONLY + BITMAP2 )
		return 1
	end

	if severity <= 50 then
		explode( base, BITMAPONLY + BITMAP1 )
		explode( texture, SHATTER + BITMAP2 )
		return 2
	end

	if severity <= 99 then
	    explode( base, BITMAPONLY + BITMAP1 )
	    explode( texture, SHATTER + BITMAP2 )
		return 3
	end
	explode( base, BITMAPONLY + BITMAP1 )
	explode( texture, SHATTER + EXPLODE_ON_HIT + BITMAP2 )
	return 3
end
