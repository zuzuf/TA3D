
macro(YMESSAGE msg)
	message(STATUS "{yuni} -- ${msg}")
endmacro()

macro(YERROR msg)
	message(SEND_ERROR "{yuni} -- ${msg}")
endmacro()

macro(YFATAL msg)
	message(FATAL_ERROR "{yuni} -- ${msg}")
endmacro()


