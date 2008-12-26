bin_PROGRAMS=ta3d 3dmeditor
ta3d_SOURCES= \
	ta3d.cpp ta3d.h \
	main.cpp \
	backtrace.cpp backtrace.h \
	hpiview.cpp \
	install.cpp \
	i18n.cpp i18n.h \
	cTA3D_Engine.cpp cTA3D_Engine.h \
	cTAFileParser.cpp cTAFileParser.h \
	restore.cpp restore.h \
	gfx.cpp gfx.h \
	pathfinding.cpp pathfinding.h \
	menu.cpp menu.h \
	3do.cpp 3do.h \
	cob.cpp cob.h \
	console.cpp console.h \
	EngineClass.cpp EngineClass.h \
	UnitEngine.cpp UnitEngine.h \
	fbi.cpp fbi.h \
	gaf.cpp gaf.h \
	glfunc.cpp glfunc.h \
	gui.cpp gui.h \
	intro.cpp intro.h \
	particles.cpp particles.h \
	lzw.cpp lzw.h \
	tdf.cpp tdf.h \
	tnt.cpp tnt.h \
	weapons.cpp weapons.h \
	fx.cpp fx.h \
	script.cpp script.h \
	ai.cpp ai.h \
	taconfig.cpp taconfig.h \
	icon.cpp \
	stdafx.cpp stdafx.h \
	TA3D_Network.cpp TA3D_Network.h \
	TA3D_Audio.cpp TA3D_Audio.h \
	TA3D_Exception.cpp TA3D_Exception.h \
	hash_table.h \
	TA3D_hpi.cpp TA3D_hpi.h \
	TA3D_NameSpace.cpp TA3D_NameSpace.h \
	cThread.cpp cThread.h \
	cCriticalSection.cpp cCriticalSection.h \
	cInterface.cpp cInterface.h \
	cLogger.cpp cLogger.h \
	cError.cpp cError.h \
	jpeg/decode.cpp jpeg/encode.cpp jpeg/io.cpp jpeg/ta3d_jpg.cpp \
	jpeg/dct.h jpeg/internal.h jpeg/ta3d_jpg.h \
	SocketClass.cpp SocketClass.h \
	udpsock.cpp udpsock.h \
	ta3dsock.cpp ta3dsock.h \
	broadcastsock.cpp broadcastsock.h \
	superqueue.cpp superqueue.h \
	network.cpp network.h \
	TA3D_Platform.h matrix.h ta3dbase.h vector.h thread.h


#	SocketClass.cpp SocketClass.h \
#	ta3dsock.cpp ta3dsock.h \
#	superqueue.cpp superqueue.h \
#	network.cpp network.h

3dmeditor_SOURCES= \
	3dmeditor.cpp ta3d.h \
	3dmeditor_sub.cpp 3dmeditor.h \
	backtrace.cpp backtrace.h \
	cTAFileParser.cpp cTAFileParser.h \
	3ds.cpp 3ds.h \
	i18n.cpp i18n.h \
	gfx.cpp gfx.h \
	3do.cpp 3do.h \
	console.cpp console.h \
	gaf.cpp gaf.h \
	glfunc.cpp glfunc.h \
	gui.cpp gui.h \
	particles.cpp particles.h \
	taconfig.cpp taconfig.h \
	stdafx.cpp stdafx.h \
	TA3D_Audio.cpp TA3D_Audio.h \
	TA3D_Exception.cpp TA3D_Exception.h \
	hash_table.h \
	TA3D_hpi.cpp TA3D_hpi.h \
	TA3D_NameSpace.cpp TA3D_NameSpace.h \
	cThread.cpp cThread.h \
	cCriticalSection.cpp cCriticalSection.h \
	cInterface.cpp cInterface.h \
	cLogger.cpp cLogger.h \
	cError.cpp cError.h \
	jpeg/decode.cpp jpeg/encode.cpp jpeg/io.cpp jpeg/ta3d_jpg.cpp \
	jpeg/dct.h jpeg/internal.h jpeg/ta3d_jpg.h \
	TA3D_Platform.h matrix.h ta3dbase.h vector.h thread.h

INSTALL=../install-sh

all:
	!(test -e ta3d) || mv ta3d ../ta3d-bin
	!(test -e ta3d.exe) || mv ta3d.exe ../ta3d-bin.exe
	!(test -e 3dmeditor) || mv 3dmeditor ../3dmeditor-bin
	!(test -e 3dmeditor.exe) || mv 3dmeditor.exe ../
