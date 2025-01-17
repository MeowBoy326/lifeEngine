#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>
#include <SDL.h>

#include "Misc/Types.h"
#include "Misc/CoreGlobals.h"
#include "Misc/Guid.h"
#include "Containers/String.h"
#include "Containers/StringConv.h"
#include "Logger/LoggerMacros.h"
#include "EngineLoop.h"
#include "D3D11RHI.h"
#include "WindowsLogger.h"
#include "WindowsFileSystem.h"
#include "WindowsWindow.h"
#include "WindowsImGUI.h"
#include "WindowsStackWalker.h"
#include "WindowsGlobals.h"

// ----
// Platform specific globals variables
// ----

CBaseLogger*         g_Log			= new CWindowsLogger();
CBaseFileSystem*     g_FileSystem	= new CWindowsFileSystem();
CBaseWindow*         g_Window		= new CWindowsWindow();
CBaseRHI*            g_RHI			= new CD3D11RHI();
CEngineLoop*         g_EngineLoop	= new CEngineLoop();
EPlatformType        g_Platform		= PLATFORM_Windows;

// ---
// Classes
// ---

/**
 * @ingroup WindowsPlatform
 * Stack walker for Windows
 */
class CStackWalker : public StackWalker
{
public:
	/**
	 * Constructor
	 */
	CStackWalker() : StackWalker()
	{}

	/**
	 * Get buffer
	 * @return Return call stack buffer
	 */
	FORCEINLINE const std::wstring& GetBuffer() const
	{
		return buffer;
	}

protected:
	virtual void OnOutput( LPCSTR InText ) override
	{
		buffer += ANSI_TO_TCHAR( InText );
	}

	virtual void OnSymInit( LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName )  override
	{}

	virtual void OnLoadModule( LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion ) override 
	{}

	virtual void OnDbgHelpErr( LPCSTR szFuncName, DWORD gle, DWORD64 addr ) override
	{}

private:
	std::wstring	buffer;		/**< Output buffer of call stack */
};

// ----
// Platform specific functions
// ----

/*
==================
Sys_GetVarArgs
==================
*/
int Sys_GetVarArgs( tchar* InOutDest, uint32 InDestSize, uint32 InCount, const tchar*& InFormat, va_list InArgPtr )
{
    return vswprintf( InOutDest, InCount, InFormat, InArgPtr );
}

/*
==================
Sys_GetVarArgsAnsi
==================
*/
int Sys_GetVarArgsAnsi( achar* InOutDest, uint32 InDestSize, uint32 InCount, const achar*& InFormat, va_list InArgPtr )
{
    return vsnprintf( InOutDest, InCount, InFormat, InArgPtr );
}

/*
==================
Sys_CreateProc
==================
*/
void* Sys_CreateProc( const tchar* InPathToProcess, const tchar* InParams, bool InLaunchDetached, bool InLaunchHidden, bool InLaunchReallyHidden, int32 InPriorityModifier, uint64* OutProcessId /* = nullptr */ )
{
    Logf( TEXT( "CreateProc %s %s\n" ), InPathToProcess, InParams );

	std::wstring				commandLine = CString::Format( TEXT( "%s %s" ), InPathToProcess, InParams );

	PROCESS_INFORMATION			procInfo;
	SECURITY_ATTRIBUTES			attributes;
	attributes.nLength					= sizeof( SECURITY_ATTRIBUTES );
	attributes.lpSecurityDescriptor		= nullptr;
	attributes.bInheritHandle			= true;

	uint64						createFlags = NORMAL_PRIORITY_CLASS;
	if ( InPriorityModifier < 0 )
	{
		if ( InPriorityModifier == -1 )
		{
			createFlags = BELOW_NORMAL_PRIORITY_CLASS;
		}
		else
		{
			createFlags = IDLE_PRIORITY_CLASS;
		}
	}
	else if ( InPriorityModifier > 0 )
	{
		if ( InPriorityModifier == 1 )
		{
			createFlags = ABOVE_NORMAL_PRIORITY_CLASS;
		}
		else
		{
			createFlags = HIGH_PRIORITY_CLASS;
		}
	}
	if ( InLaunchDetached )
	{
		createFlags |= DETACHED_PROCESS;
	}

	uint64			flags = 0;
	uint32			showWindowFlags = SW_HIDE;
	if ( InLaunchReallyHidden )
	{
		flags = STARTF_USESHOWWINDOW;

		showWindowFlags = SW_HIDE;			// Epic's code doesn't properly hide the window, so let's do that
		if ( InLaunchDetached )				// if really hiding the window, and running detached, create a new console
		{
			createFlags = CREATE_NEW_CONSOLE;
		}
	}
	else if ( InLaunchHidden )
	{
		flags = STARTF_USESHOWWINDOW;
		showWindowFlags = SW_SHOWMINNOACTIVE;
	}

	STARTUPINFO		startupInfo = 
	{ 
		sizeof( STARTUPINFO ),				NULL,						NULL,						NULL,
		( DWORD )CW_USEDEFAULT,				( DWORD )CW_USEDEFAULT,		( DWORD )CW_USEDEFAULT,		( DWORD )CW_USEDEFAULT,
		NULL,								NULL,						NULL,						( DWORD )flags,
		( WORD )showWindowFlags,			NULL,						NULL,						NULL,
		NULL,								NULL
	};

	if ( !CreateProcessW( NULL, ( LPWSTR )commandLine.c_str(), &attributes, &attributes, TRUE, ( DWORD )createFlags, NULL, ( LPCWSTR )g_FileSystem->GetCurrentDirectory().c_str(), ( LPSTARTUPINFOW ) &startupInfo, &procInfo ) )
	{
		if ( OutProcessId )
		{
			*OutProcessId = 0;
		}

		return nullptr;
	}

	if ( OutProcessId )
	{
		*OutProcessId = procInfo.dwProcessId;
	}

	return ( void* )procInfo.hProcess;
}

/*
==================
Sys_GetProcReturnCode
==================
*/
bool Sys_GetProcReturnCode( void* InProcHandle, int32* OutReturnCode )
{
    return GetExitCodeProcess( ( HANDLE )InProcHandle, ( DWORD* )OutReturnCode ) && *( ( DWORD* )OutReturnCode ) != STILL_ACTIVE;
}

/*
==================
Sys_ShowMessageBox
==================
*/
void Sys_ShowMessageBox( const tchar* InTitle, const tchar* InMessage, EMessageBox Intype )
{
	uint32		flags = 0;
	switch ( Intype )
	{
	case MB_Info:		flags = SDL_MESSAGEBOX_INFORMATION;	break;
	case MB_Warning:	flags = SDL_MESSAGEBOX_WARNING;		break;
	case MB_Error:		flags = SDL_MESSAGEBOX_ERROR;		break;
	}

	SDL_ShowSimpleMessageBox( flags, TCHAR_TO_ANSI( InTitle ), TCHAR_TO_ANSI( InMessage ), nullptr );
}

/*
==================
Sys_DumpCallStack
==================
*/
void Sys_DumpCallStack( std::wstring& OutCallStack )
{
	CStackWalker	stackWalker;
	stackWalker.ShowCallstack();
	OutCallStack = stackWalker.GetBuffer();
}

/*
==================
Sys_RequestExit
==================
*/
void Sys_RequestExit( bool InForce )
{
	if ( InForce )
	{
		// Force immediate exit
		// Dangerous because config code isn't flushed, global destructors aren't called, etc
		// Suppress abort message and MS reports
		_set_abort_behavior( 0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT );
		abort();
	}
	else
	{
		// Tell the platform specific code we want to exit cleanly from the main loop.
		PostQuitMessage( 0 );
		g_IsRequestingExit = true;
	}
}

/*
==================
Sys_CreateGuid
==================
*/
CGuid Sys_CreateGuid()
{
	CGuid	guid;
	HRESULT result = CoCreateGuid( ( GUID* ) &guid );
	Assert( result == S_OK );
	return guid;
}

/*
==================
Sys_ComputerName
==================
*/
std::wstring Sys_ComputerName()
{
	static tchar	result[256] = TEXT( "" );
	if ( !result[0] )
	{
		DWORD	size = ARRAY_COUNT( result );
		GetComputerNameW( result, &size );
	}
	return result;
}

/*
==================
Sys_UserName
==================
*/
std::wstring Sys_UserName()
{
	static tchar	result[256] = TEXT( "" );
	if ( !result[0] )
	{
		DWORD	size = ARRAY_COUNT( result );
		GetUserNameW( result, &size );
	}
	return result;
}

#if WITH_EDITOR
#include "Windows/FileDialog.h"

/*
==================
Sys_ShowFileInExplorer
==================
*/
void Sys_ShowFileInExplorer( const std::wstring& InPath )
{
	CFilename		filename( g_FileSystem->ConvertToAbsolutePath( InPath ) );
	Sys_CreateProc( TEXT( "explorer.exe" ), g_FileSystem->IsDirectory( filename.GetFullPath() ) ? filename.GetFullPath().c_str() : filename.GetPath().c_str(), true, false, false, 0 );
}

/*
==================
Sys_ShowOpenFileDialog
==================
*/
bool Sys_ShowOpenFileDialog( const CFileDialogSetup& InSetup, OpenFileDialogResult& OutResult )
{
	OPENFILENAME		fileDialogSettings;
	Sys_Memzero( &fileDialogSettings, sizeof( OPENFILENAME ) );
	fileDialogSettings.lStructSize = sizeof( OPENFILENAME );

	// Flags
	fileDialogSettings.hwndOwner	= g_Window ? ( HWND )g_Window->GetHandle() : nullptr;
	fileDialogSettings.Flags		|= ( InSetup.IsMultiselection() ) ? OFN_ALLOWMULTISELECT : 0;
	fileDialogSettings.Flags		|= OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_NONETWORKBUTTON | OFN_PATHMUSTEXIST | OFN_EXTENSIONDIFFERENT;

	// File name buffer
	std::wstring		fileNameBuffer;
	fileNameBuffer.resize( 64 * 1024 );
	fileDialogSettings.lpstrFile	= fileNameBuffer.data();
	fileDialogSettings.nMaxFile		= fileNameBuffer.size();

	// Format filters
	std::wstring		filterBuffer;
	const std::vector<CFileDialogSetup::FileNameFilter>		fileNameFilters = InSetup.GetFormats();
	if ( fileNameFilters.empty() )
	{
		fileDialogSettings.nFilterIndex		= 0;
		fileDialogSettings.lpstrFilter		= TEXT( "All Formats (*.*)" );
	}
	else	// We are going to filter only those files with the same exact filename
	{
		std::wstring		allSupportedFormats;
		for ( uint32 index = 0, count = fileNameFilters.size(); index < count; ++index )
		{
			const CFileDialogSetup::FileNameFilter&	fileNameFilter = fileNameFilters[index];
			if ( index > 0 )
			{
				allSupportedFormats += TEXT( "; " );
			}
			allSupportedFormats += fileNameFilter.filter;
			
			filterBuffer		+= CString::Format( TEXT( "%s (%s)" ), fileNameFilter.description.c_str(), fileNameFilter.filter.c_str() );
			filterBuffer.push_back( TEXT( '\0' ) );
			filterBuffer		+= fileNameFilter.filter;
			filterBuffer.push_back( TEXT( '\0' ) );
		}
		filterBuffer			+= CString::Format( TEXT( "All Supported Formats (%s)" ), allSupportedFormats.c_str() );
		filterBuffer.push_back( TEXT( '\0' ) );
		filterBuffer			+= allSupportedFormats;
		filterBuffer.push_back( TEXT( '\0' ) );

		fileDialogSettings.nFilterIndex		= 0;
		fileDialogSettings.lpstrFilter		= filterBuffer.data();
	}

	// Default file extension
	std::wstring		defaultFileExtension = InSetup.GetDefaultExtension();
	if ( !defaultFileExtension.empty() )
	{
		fileDialogSettings.lpstrDefExt = defaultFileExtension.data();
	}

	// Title text
	std::wstring		title = InSetup.GetTitle();
	if ( title.empty() )
	{
		fileDialogSettings.lpstrTitle	= InSetup.IsMultiselection() ? TEXT( "Open Files" ) : TEXT( "Open File" );
	}
	else
	{
		fileDialogSettings.lpstrTitle	= title.data();
	}

	// Preserve the directory around the calls
	std::wstring		absolutePathToDirectory = g_FileSystem->ConvertToAbsolutePath( InSetup.GetDirectory() );
	fileDialogSettings.lpstrInitialDir	= absolutePathToDirectory.data();

	// Open file dialog
	std::wstring		originalCurrentDir = g_FileSystem->GetCurrentDirectory();
	if ( !GetOpenFileNameW( &fileDialogSettings ) )
	{
		return false;
	}
	SetCurrentDirectoryW( originalCurrentDir.c_str() );

	// Extract file paths
	{
		std::vector<std::wstring>		parts;

		// Parse file names
		tchar*	pPos	= fileNameBuffer.data();
		tchar*	pStart	= fileNameBuffer.data();
		while ( true )
		{
			if ( *pPos == 0 )
			{
				if ( pPos == pStart )
				{
					break;
				}
				
				parts.push_back( std::wstring( pStart ) );
				pStart	= pPos + 1;
				pPos	+= 1;
			}
			else
			{
				++pPos;
			}
		}

		if ( !parts.empty() )
		{
			// If single paths
			if ( parts.size() == 1 )
			{
				OutResult.files.push_back( parts[0] );
			}
			// When multiple paths are selected the first entry in the list is the directory
			else
			{
				std::wstring	basePath = parts[0];
				for ( uint32 index = 1, count = parts.size(); index < count; ++index )
				{
					OutResult.files.push_back( basePath + PATH_SEPARATOR + parts[index] );
				}
			}
		}
	}

	return true;
}

/*
==================
Sys_ShowSaveFileDialog
==================
*/
bool Sys_ShowSaveFileDialog( const CFileDialogSetup& InSetup, SaveFileDialogResult& OutResult )
{
	OPENFILENAME		fileDialogSettings;
	Sys_Memzero( &fileDialogSettings, sizeof( OPENFILENAME ) );
	fileDialogSettings.lStructSize = sizeof( OPENFILENAME );

	// Flags
	fileDialogSettings.hwndOwner	= g_Window ? ( HWND )g_Window->GetHandle() : nullptr;
	fileDialogSettings.Flags		|= OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_NONETWORKBUTTON | OFN_PATHMUSTEXIST | OFN_NOTESTFILECREATE | OFN_OVERWRITEPROMPT | OFN_EXTENSIONDIFFERENT;

	// File name buffer
	std::wstring		fileNameBuffer;
	fileNameBuffer.resize( 64 * 1024 );
	fileDialogSettings.lpstrFile	= fileNameBuffer.data();
	fileDialogSettings.nMaxFile		= fileNameBuffer.size();

	// Format filters
	std::wstring		filterBuffer;
	const std::vector<CFileDialogSetup::FileNameFilter>		fileNameFilters = InSetup.GetFormats();
	if ( fileNameFilters.empty() )
	{
		fileDialogSettings.nFilterIndex		= 0;
		fileDialogSettings.lpstrFilter		= TEXT( "All Formats (*.*)" );
	}
	else	// We are going to filter only those files with the same exact filename
	{
		std::wstring		allSupportedFormats;
		for ( uint32 index = 0, count = fileNameFilters.size(); index < count; ++index )
		{
			const CFileDialogSetup::FileNameFilter&	fileNameFilter = fileNameFilters[index];
			if ( index > 0 )
			{
				allSupportedFormats += TEXT( "; " );
			}
			allSupportedFormats += fileNameFilter.filter;
			
			filterBuffer		+= CString::Format( TEXT( "%s (%s)" ), fileNameFilter.description.c_str(), fileNameFilter.filter.c_str() );
			filterBuffer.push_back( TEXT( '\0' ) );
			filterBuffer		+= fileNameFilter.filter;
			filterBuffer.push_back( TEXT( '\0' ) );
		}
		filterBuffer			+= CString::Format( TEXT( "All Supported Formats (%s)" ), allSupportedFormats.c_str() );
		filterBuffer.push_back( TEXT( '\0' ) );
		filterBuffer			+= allSupportedFormats;
		filterBuffer.push_back( TEXT( '\0' ) );

		fileDialogSettings.nFilterIndex		= 0;
		fileDialogSettings.lpstrFilter		= filterBuffer.data();
	}

	// Default file extension
	std::wstring		defaultFileExtension = InSetup.GetDefaultExtension();
	if ( !defaultFileExtension.empty() )
	{
		fileDialogSettings.lpstrDefExt = defaultFileExtension.data();
	}

	// Title text
	std::wstring		title = InSetup.GetTitle();
	if ( title.empty() )
	{
		fileDialogSettings.lpstrTitle	= InSetup.IsMultiselection() ? TEXT( "Save Files" ) : TEXT( "Save File" );
	}
	else
	{
		fileDialogSettings.lpstrTitle	= title.data();
	}

	// Preserve the directory around the calls
	std::wstring		absolutePathToDirectory = g_FileSystem->ConvertToAbsolutePath( InSetup.GetDirectory() );
	fileDialogSettings.lpstrInitialDir	= absolutePathToDirectory.data();

	// Open file dialog
	std::wstring		originalCurrentDir = g_FileSystem->GetCurrentDirectory();
	if ( !GetSaveFileNameW( &fileDialogSettings ) )
	{
		return false;
	}
	SetCurrentDirectoryW( originalCurrentDir.c_str() );

	// Get selected format
	if ( fileDialogSettings.nFilterIndex && fileDialogSettings.nFilterIndex <= fileNameFilters.size() )
	{
		OutResult.selectedFormat = fileNameFilters[fileDialogSettings.nFilterIndex - 1];
	}
	else if ( !fileNameFilters.empty() )
	{
		OutResult.selectedFormat = fileNameFilters[0];
	}

	// Extract file paths
	{
		std::vector<std::wstring>		parts;

		// Parse file names
		tchar*	pPos	= fileNameBuffer.data();
		tchar*	pStart	= fileNameBuffer.data();
		while ( true )
		{
			if ( *pPos == 0 )
			{
				if ( pPos == pStart )
				{
					break;
				}
				
				parts.push_back( std::wstring( pStart ) );
				pStart	= pPos + 1;
				pPos	+= 1;
			}
			else
			{
				++pPos;
			}
		}

		if ( !parts.empty() )
		{
			// If single paths
			if ( parts.size() == 1 )
			{
				OutResult.files.push_back( parts[0] );
			}
			// When multiple paths are selected the first entry in the list is the directory
			else
			{
				std::wstring	basePath = parts[0];
				for ( uint32 index = 1, count = parts.size(); index < count; ++index )
				{
					OutResult.files.push_back( basePath + PATH_SEPARATOR + parts[index] );
				}
			}
		}
	}

	return true;
}
#endif // WITH_EDITOR