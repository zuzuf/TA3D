-- keep trying to open building's yard until we succeed
function OpenYard()
	set( YARD_OPEN, true )

	while not get( YARD_OPEN ) do
		set( BUGGER_OFF, true )
		sleep( 1.500 )
		set( YARD_OPEN, true )
	end

	set( BUGGER_OFF, false )
end


-- keep trying to close building's yard until we succeed
function CloseYard()
	set( YARD_OPEN, false )

	while get( YARD_OPEN ) do
		set( BUGGER_OFF, true )
		sleep( 1.500 )
		set( YARD_OPEN, false )
	end

	set( BUGGER_OFF, false )
end


