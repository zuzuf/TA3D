add_executable(ta3d-bin ta3d.cpp ta3d.h main.cpp backtrace.cpp backtrace.h hpiview.cpp install.cpp i18n.cpp i18n.h cTA3D_Engine.cpp cTA3D_Engine.h cTAFileParser.cpp cTAFileParser.h restore.cpp restore.h gfx.cpp gfx.h pathfinding.cpp pathfinding.h menu.cpp menu.h 3do.cpp 3do.h cob.cpp cob.h console.cpp console.h EngineClass.cpp EngineClass.h UnitEngine.cpp UnitEngine.h fbi.cpp fbi.h gaf.cpp gaf.h glfunc.cpp glfunc.h gui.cpp gui.h intro.cpp intro.h particles.cpp particles.h lzw.cpp lzw.h tdf.cpp tdf.h tnt.cpp tnt.h weapons.cpp weapons.h script.cpp script.h ai.cpp ai.h taconfig.cpp taconfig.h icon.cpp stdafx.cpp stdafx.h TA3D_Network.cpp TA3D_Network.h TA3D_Audio.cpp TA3D_Audio.h TA3D_Exception.cpp TA3D_Exception.h hash_table.h TA3D_hpi.cpp TA3D_hpi.h TA3D_NameSpace.cpp TA3D_NameSpace.h cThread.cpp cThread.h cCriticalSection.cpp cCriticalSection.h cInterface.cpp cInterface.h cLogger.cpp cLogger.h cError.cpp cError.h jpeg/decode.cpp jpeg/encode.cpp jpeg/io.cpp jpeg/ta3d_jpg.cpp jpeg/dct.h jpeg/internal.h jpeg/ta3d_jpg.h SocketClass.cpp SocketClass.h udpsock.cpp udpsock.h ta3dsock.cpp ta3dsock.h broadcastsock.cpp broadcastsock.h superqueue.cpp superqueue.h network.cpp network.h TA3D_Platform.h matrix.h ta3dbase.h vector.h thread.h )

if( WIN32 OR CMAKE_CROSSCOMPILING )
	SET( 3DMEDITOR 3dmeditor )
else( WIN32 OR CMAKE_CROSSCOMPILING )
	SET( 3DMEDITOR 3dmeditor-bin )
endif( WIN32 OR CMAKE_CROSSCOMPILING )

add_executable(${3DMEDITOR} 3dmeditor.cpp ta3d.h 3dmeditor_sub.cpp 3dmeditor.h backtrace.cpp backtrace.h cTAFileParser.cpp cTAFileParser.h 3ds.cpp 3ds.h i18n.cpp i18n.h gfx.cpp gfx.h 3do.cpp 3do.h console.cpp console.h gaf.cpp gaf.h glfunc.cpp glfunc.h gui.cpp gui.h particles.cpp particles.h taconfig.cpp taconfig.h stdafx.cpp stdafx.h TA3D_Audio.cpp TA3D_Audio.h TA3D_Exception.cpp TA3D_Exception.h hash_table.h TA3D_hpi.cpp TA3D_hpi.h TA3D_NameSpace.cpp TA3D_NameSpace.h cThread.cpp cThread.h cCriticalSection.cpp cCriticalSection.h cInterface.cpp cInterface.h cLogger.cpp cLogger.h cError.cpp cError.h jpeg/decode.cpp jpeg/encode.cpp jpeg/io.cpp jpeg/ta3d_jpg.cpp jpeg/dct.h jpeg/internal.h jpeg/ta3d_jpg.h TA3D_Platform.h matrix.h ta3dbase.h vector.h thread.h)

target_link_libraries(ta3d-bin ${LIBS} )
target_link_libraries(${3DMEDITOR} ${LIBS} )

find_library( VAR_LIB_FMODEX   fmodex )
find_library( VAR_LIB_FMODEX64 fmodex64 )

if( VAR_LIB_FMODEX )
	target_link_libraries(ta3d-bin fmodex)
	target_link_libraries(${3DMEDITOR} fmodex)
else( VAR_LIB_FMODEX64 )
	target_link_libraries(ta3d-bin fmodex64)
	target_link_libraries(${3DMEDITOR} fmodex64)
else( VAR_LIB_FMODEX )
	message( FATAL_ERROR "FMODex is required, please install FMODex" )
endif( VAR_LIB_FMODEX )
