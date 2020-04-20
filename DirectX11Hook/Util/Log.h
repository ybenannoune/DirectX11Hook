#ifndef _LOGFILE_H
#define _LOGFILE_H

#include <Windows.h>
#include <TlHelp32.h>
#include <io.h>
#include <iostream>

#define MAX_CONSOLE_LINES 500
#define _O_TEXT 0x4000

#ifdef _DEBUG
#define DEBUGOUT( X, ... ) Log::Add( X, __VA_ARGS__ )
#else
#define DEBUGOUT( X, ... )
#endif

#define LOG_STRING(sz)              "[LOG] : %s  : %s ", #sz ,  sz
#define LOG_INT(val)                "[LOG] : %s  : %d ",#val,val
#define LOG_FLOAT(val)              "[LOG] : %s  : %f ",#val,val
#define LOG_HEX(hex)                "[LOG] : %s  : 0x%X",#hex,hex

#define LOG_ANGLES(ang)             "[LOG] : %s  (%5.5f | %5.5f)",#ang,ang.yaw,ang.pitch
#define LOG_VEC2(vec2)              "[LOG] : %s  (%5.1f | %5.1f)",#vec2,vec2.x,vec2.y
#define LOG_VEC3(vec3)              "[LOG] : %s  (%5.1f | %5.1f | %5.1f)",#vec3,vec3.x,vec3.y,vec3.z
#define LOG_VEC4(vec4)              "[LOG] : %s  (%5.1f | %5.1f | %5.1f | %5.1f) ",#vec4,vec4.x,vec4.y,vec4.z,vec4.w

/*
#define LOG_MATRIX(mat)             "[LOG] : Matrix : %s" #mat \
									"[LOG] : (%5.1f | %5.1f | %5.1f | %5.1f) ", mat._11, mat._12, mat._13, mat._14 \
	                                "[LOG] : (%5.1f | %5.1f | %5.1f | %5.1f) ", mat._21, mat._22, mat._23, mat._24 \
									"[LOG] : (%5.1f | %5.1f | %5.1f | %5.1f) ", mat._31, mat._32, mat._33, mat._34 \
									"[LOG] : (%5.1f | %5.1f | %5.1f | %5.1f) ", mat._41, mat._43, mat._43, mat._44 \
*/

enum ConsoleColor
{
	BLACK,
	DARK_BLUE,
	DARK_GREEN,
	DARK_CYAN,
	DARK_RED,
	DARK_PINK,
	DARK_YELLOW,
	DARK_WHITE,
	GREY,
	BLUE,
	GREEN,
	CYAN,
	RED,
	PINK,
	YELLOW,
	WHITE,
};

namespace Log
{	
	void CreateConsole( char* consoleName );
	void ResizeConsole( int size );
	BOOL CloseConsole( );

	void SetColorConsole( DWORD color );
	void ClearConsole( );

	void Add( char *szText, ... );
	void NewLine( );

	void Flog( char* szFormat, ... );
	char* GetDirectoryFile( char *filename );
}


#endif