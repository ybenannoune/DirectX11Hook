#include "Log.h"

HANDLE		g_hOutConsole = nullptr;

namespace Log
{	
	void CreateConsole( char* consoleName )
	{
		if ( !AllocConsole( ) )
			return;

		SetConsoleTitleA( consoleName );

		g_hOutConsole = GetStdHandle( STD_OUTPUT_HANDLE );
		if ( g_hOutConsole == false )
			return;

		// Redirect STDOUT to the console
		long lStdHandle = ( long )GetStdHandle( STD_OUTPUT_HANDLE );
		short int hConHandle = _open_osfhandle( lStdHandle, _O_TEXT );
		FILE* fp = _fdopen( hConHandle, "w" );
		*stdout = *fp;
		setvbuf( stdout, nullptr, _IONBF, 0 );


		// Redirect STDIN to the console
		lStdHandle = ( long )GetStdHandle( STD_INPUT_HANDLE );
		hConHandle = _open_osfhandle( lStdHandle, _O_TEXT );
		fp = _fdopen( hConHandle, "r" );
		*stdin = *fp;
		setvbuf( stdin, nullptr, _IONBF, 0 );


		// Redirect STDERR to the console
		lStdHandle = ( long )GetStdHandle( STD_ERROR_HANDLE );
		hConHandle = _open_osfhandle( lStdHandle, _O_TEXT );
		fp = _fdopen( hConHandle, "w" );
		*stderr = *fp;
		setvbuf( stderr, nullptr, _IONBF, 0 );

		SetColorConsole( ConsoleColor::WHITE );
	}
	
	void ResizeConsole( int size )
	{
		CONSOLE_SCREEN_BUFFER_INFO coninfo;
		GetConsoleScreenBufferInfo( g_hOutConsole, &coninfo );
		coninfo.dwSize.Y = size;
		coninfo.dwSize.X = coninfo.dwSize.X * 2;
		SetConsoleScreenBufferSize( g_hOutConsole, coninfo.dwSize );
	}
	
	void SetColorConsole( DWORD color )
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if ( GetConsoleScreenBufferInfo( g_hOutConsole, &csbi ) )
		{
			WORD wColor = ( csbi.wAttributes & 0xF0 ) + ( color & 0x0F );
			SetConsoleTextAttribute( g_hOutConsole, wColor );
		}
		return;
	}

	void ClearConsole( )
	{
		COORD coordScreen = { 0, 0 };    // home for the cursor 
		DWORD cCharsWritten;
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		DWORD dwConSize;

		// Get the number of character cells in the current buffer. 
		if ( !GetConsoleScreenBufferInfo( g_hOutConsole, &csbi ) )
		{
			return;
		}
		dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

		// Fill the entire screen with blanks.
		if ( !FillConsoleOutputCharacter( g_hOutConsole,        // Handle to console screen buffer 
			( TCHAR ) ' ',     // Character to write to the buffer
			dwConSize,       // Number of cells to write 
			coordScreen,     // Coordinates of first cell 
			&cCharsWritten ) )// Receive number of characters written
		{
			return;
		}

		// Get the current text attribute.
		if ( !GetConsoleScreenBufferInfo( g_hOutConsole, &csbi ) )
		{
			return;
		}

		// Set the buffer's attributes accordingly.

		if ( !FillConsoleOutputAttribute( g_hOutConsole,         // Handle to console screen buffer 
			csbi.wAttributes, // Character attributes to use
			dwConSize,        // Number of cells to set attribute 
			coordScreen,      // Coordinates of first cell 
			&cCharsWritten ) ) // Receive number of characters written
		{
			return;
		}

		// Put the cursor at its home coordinates.

		SetConsoleCursorPosition( g_hOutConsole, coordScreen );
	}	
	
	void Add( char *szText, ... )
	{
		if ( !szText || strlen( szText ) >= 256 )
			return;

		va_list va_alist;
		char logbuf[ 256 ];

		va_start( va_alist, szText );
		_vsnprintf_s( logbuf, sizeof( logbuf ), szText, va_alist );
		va_end( va_alist );

		//Get current time
		SYSTEMTIME	time;
		GetLocalTime( &time );

		char szLine[ 512 ];
		sprintf_s( szLine, "%02d:%02d:%02d:%03d \t%s\n",
			time.wHour, time.wMinute, time.wSecond, time.wMilliseconds, logbuf );

		printf( szLine );
	}

	void NewLine( )
	{
		printf( "\n" );
	}

	char* GetDirectoryFile( char *filename )
	{
		static char exedir[ 320 ];
		static char path[ 320 ];
		strcpy_s( path, exedir );
		strcat_s( path, filename );
		return path;
	}

	void Flog( char* szFormat, ... )
	{
		if ( !szFormat || strlen( szFormat ) >= 256 )
			return;

		FILE * fp;
		char logbuf[ 256 ] = { 0 };
		va_list va_alist;
		va_start( va_alist, szFormat );
		_vsnprintf_s( logbuf, sizeof( logbuf ), szFormat, va_alist );
		va_end( va_alist );

		if ( ( fopen_s( &fp, GetDirectoryFile( "Log.txt" ), "a" ) ) != NULL )
		{
			fprintf( fp, "%s", logbuf );
			fclose( fp );
		}
	}

	BOOL CloseConsole( )
	{
		return ( FreeConsole( ) );
	}
}












