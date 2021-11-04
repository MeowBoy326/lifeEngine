#include <qapplication.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <string>
#include <vector>
#include <sstream>

#include "Containers/StringConv.h"
#include "Logger/LoggerMacros.h"
#include "System/Config.h"
#include "System/SplashScreen.h"
#include "System/EditorEngine.h"
#include "Misc/EngineGlobals.h"
#include "Misc/LaunchGlobals.h"
#include "Misc/WorldEdGlobals.h"
#include "Misc/Misc.h"
#include "EngineLoop.h"
#include "WorldEd.h"

void QtMessageOutput( QtMsgType InType, const QMessageLogContext& InContext, const QString& InMessage )
{
	switch ( InType )
	{
	case QtDebugMsg:
	case QtInfoMsg:
		LE_LOG( LT_Log, LC_Editor, InMessage.toStdWString().c_str() );
		break;

	case QtWarningMsg:
		LE_LOG( LT_Warning, LC_Editor, InMessage.toStdWString().c_str() );
		break;

	case QtCriticalMsg:
		LE_LOG( LT_Error, LC_Editor, InMessage.toStdWString().c_str() );
		break;

	case QtFatalMsg:
		appErrorf( InMessage.toStdWString().c_str() );
		break;
	}
}

int32 WorldEdEntry( const tchar* InCmdLine )
{
	// Parse arguments of start for create QApplication
	std::stringstream		strStream;
	std::vector< achar* >	arguments;
	strStream << TCHAR_TO_ANSI( InCmdLine );

	while ( !strStream.eof() )
	{
		std::string		tempString;
		strStream >> tempString;

		achar*		newArgument = new achar[ tempString.size()+1 ];
		strcpy( newArgument, tempString.data() );
		arguments.push_back( newArgument );
	}

	LE_LOG( LT_Log, LC_Init, TEXT( "Qt version: %s" ), ANSI_TO_TCHAR( qVersion() ) );

	// Create object of QApplication
	qInstallMessageHandler( QtMessageOutput );
	int32					argumentsCount = ( int32 )arguments.size();	
	QApplication			application( argumentsCount, arguments.data() );

	// Load styles and init engine
	appShowSplash( GEditorConfig.GetValue( TEXT( "Editor.Editor" ), TEXT( "Splash" ) ).GetString().c_str() );

	{
		QFile			file( TCHAR_TO_ANSI( ( appBaseDir() + TEXT( "Engine/Editor/Styles/Dark/Style.css" ) ).c_str() ) );
		if ( file.exists() )
		{
			file.open( QFile::ReadOnly | QFile::Text );
			QTextStream			textStream( &file );
			application.setStyleSheet( textStream.readAll() );
		}
	}

	GEditorEngine = GEngine->Cast< LEditorEngine >();
	checkMsg( GEditorEngine, TEXT( "Class of engine for editor must be inherited from LEditorEngine" ) );

	int32		result = GEngineLoop->Init();
	check( result == 0 );

	// Free allocated memory for arguments
	for ( uint32 index = 0, count = argumentsCount; index < argumentsCount; ++index )
	{
		delete arguments[ index ];
	}
	arguments.clear();

	// Start Qt execute
	appHideSplash();
	return application.exec();
}