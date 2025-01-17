/**
 * @file
 * @addtogroup Core Core
 *
 * Copyright BSOD-Games, All Rights Reserved.
 * Authors: Yehor Pohuliaka (zombiHello)
 */

#ifndef Archive_H
#define Archive_H

#include <string>
#include <vector>
#include <unordered_map>

#include "Core.h"
#include "Misc/Types.h"
#include "Misc/Misc.h"

/**
 * @ingroup Core
 * Enumeration of type archive
 */
enum EArchiveType
{
	AT_TextFile,		/**< Archive is text file */
	AT_ShaderCache,		/**< Archive contains shader cache */
	AT_TextureCache,	/**< Archive contains texture cache */
	AT_World,			/**< Archive contains world */
	AT_Package,			/**< Archive contains assets */
	AT_BinaryFile,		/**< Archive is unknown binary file */
};

/**
 * @ingroup Core
 * @brief The base class for work with archive
 */
class CArchive
{
public:
	/**
	 * Constructor
	 * 
	 * @param InPath Path to archive
	 */
	CArchive( const std::wstring& InPath );

	/**
	 * @brief Destructor
	 */
	virtual ~CArchive() {}

	/**
	 * @brief Serialize data
	 * 
	 * @param[in] InBuffer Pointer to buffer for serialize
	 * @param[in] InSize Size of buffer
	 */
	virtual void Serialize( void* InBuffer, uint32 InSize ) {}

	/**
	 * @brief Serialize compression data
	 * 
	 * @param[in] InBuffer Pointer to buffer for serialize
	 * @param[in] InSize Size of buffer
	 * @param[in] InFlags Compression flags (see ECompressionFlags)
	 */
	void SerializeCompressed( void* InBuffer, uint32 InSize, ECompressionFlags InFlags );

	/**
	 * @brief Serialize archive header
	 */
	void SerializeHeader();

	/**
	 * @brief Get current position in archive
	 * @return Current position in archive
	 */
	virtual uint32 Tell() { return 0; };

	/**
	 * @brief Set current position in archive
	 * 
	 * @param[in] InPosition New position in archive
	 */
	virtual void Seek( uint32 InPosition ) {}

	/**
	 * @brief Flush data
	 */
	virtual void Flush() {}

	/**
	 * Set archive type
	 * 
	 * @param[in] InType Archive type
	 */
	FORCEINLINE void SetType( EArchiveType InType )
	{
		arType = InType;
	}

	/**
	 * @brief Is saving archive
	 * @return True if archive saving, false if archive loading
	 */
	virtual bool IsSaving() const { return false; }

	/**
	 * @breif Is loading archive
	 * @return True if archive loading, false if archive saving
	 */
	virtual bool IsLoading() const { return false; }

	/**
	 * Is end of file
	 * @return Return true if end of file, else return false
	 */
	virtual bool IsEndOfFile() { return false; }

	/**
	 * @brief Get size of archive
	 * @return Size of archive
	 */
	virtual uint32 GetSize() { return 0; }

	/**
	 * Get archive version
	 * @return Return archive version
	 */
	FORCEINLINE uint32 Ver() const
	{
		return arVer;
	}

	/**
	 * Get archive type
	 * @return Return type archive
	 */
	FORCEINLINE EArchiveType Type() const
	{
		return arType;
	}

	/**
	 * Get path to archive
	 * @return Return path to archive
	 */
	FORCEINLINE const std::wstring& GetPath() const
	{
		return arPath;
	}

protected:
	uint32					arVer;			/**< Archive version (look ELifeEnginePackageVersion) */
	EArchiveType			arType;			/**< Archive type */
	std::wstring			arPath;			/**< Path to archive */
};

/**
 * @ingroup Core
 * Helper structure for compression support, containing information on compressed
 * and uncompressed size of a chunk of data.
 */
struct CompressedChunkInfo
{
	uint32		compressedSize;			/**< Compressed size of data */
	uint32		uncompressedSize;		/**< Uncompresses size of data */
};

//
// Overloaded operators for serialize in archive
//

FORCEINLINE CArchive& operator<<( CArchive& InArchive, int8& InValue )
{
	InArchive.Serialize( &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const int8& InValue )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* ) &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, uint8& InValue )
{
	InArchive.Serialize( &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const uint8& InValue )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* ) &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, int16& InValue )
{
	InArchive.Serialize( &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const int16& InValue )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* ) &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, uint16& InValue )
{
	InArchive.Serialize( &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const uint16& InValue )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* ) &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, int32& InValue )
{
	InArchive.Serialize( &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const int32& InValue )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* ) &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, uint32& InValue )
{
	InArchive.Serialize( &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const uint32& InValue )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* ) &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, int64& InValue )
{
	InArchive.Serialize( &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const int64& InValue )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* ) &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, uint64& InValue )
{
	InArchive.Serialize( &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const uint64& InValue )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* ) &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, bool& InValue )
{
	InArchive.Serialize( &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const bool& InValue )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* ) &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, float& InValue )
{
	InArchive.Serialize( &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const float& InValue )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* ) &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, double& InValue )
{
	InArchive.Serialize( &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const double& InValue )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* )&InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, EArchiveType& InValue )
{
	InArchive.Serialize( &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const EArchiveType& InValue )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* ) &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, CompressedChunkInfo& InValue )
{
	InArchive.Serialize( &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const CompressedChunkInfo& InValue )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* ) &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, achar& InValue )
{
	InArchive.Serialize( &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const achar& InValue )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* ) &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, tchar& InValue )
{
	InArchive.Serialize( &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const tchar& InValue )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* ) &InValue, sizeof( InValue ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const tchar* InStringC )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* )InStringC, ( uint32 )wcslen( InStringC ) * 2 );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const achar* InStringC )
{
	Assert( InArchive.IsSaving() );
	InArchive.Serialize( ( void* )InStringC, ( uint32 )strlen( InStringC ) );
	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, std::string& InValue )
{
	// If we serialize text file
	if ( InArchive.Type() == AT_TextFile )
	{
		InArchive.Serialize( InValue.data(), InValue.size() );
	}
	// Else we serialize binary archive
	else
	{
		uint32		stringSize = InValue.size();
		InArchive << stringSize;

		if ( stringSize > 0 )
		{
			if ( InArchive.IsLoading() )
			{
				InValue.resize( stringSize );
			}

			InArchive.Serialize( InValue.data(), stringSize );
		}
	}

	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const std::string& InValue )
{
	Assert( InArchive.IsSaving() );

	// If we serialize text file
	if ( InArchive.Type() == AT_TextFile )
	{
		InArchive.Serialize( ( void* )InValue.data(), InValue.size() );
	}
	// Else we serialize binary archive
	else
	{
		uint32		stringSize = InValue.size();
		InArchive << stringSize;

		if ( stringSize > 0 )
		{
			InArchive.Serialize( ( void* )InValue.data(), stringSize );
		}
	}

	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, std::wstring& InValue )
{	
	// If we serialize text file
	if ( InArchive.Type() == AT_TextFile )
	{
		InArchive.Serialize( InValue.data(), InValue.size() * sizeof( std::wstring::value_type ) );
	}
	// Else we serialize binary archive
	else
	{
		uint32		stringSize = InValue.size() * sizeof( std::wstring::value_type );
		InArchive << stringSize;

		if ( stringSize > 0 )
		{
			if ( InArchive.IsLoading() )
			{
				InValue.resize( stringSize / sizeof( std::wstring::value_type ) );
			}

			InArchive.Serialize( InValue.data(), stringSize );
		}
	}

	return InArchive;
}

FORCEINLINE CArchive& operator<<( CArchive& InArchive, const std::wstring& InValue )
{
	Assert( InArchive.IsSaving() );

	// If we serialize text file
	if ( InArchive.Type() == AT_TextFile )
	{
		InArchive.Serialize( ( void* )InValue.data(), InValue.size() * sizeof( std::wstring::value_type ) );
	}
	// Else we serialize binary archive
	else
	{
		uint32		stringSize = InValue.size() * sizeof( std::wstring::value_type );
		InArchive << stringSize;

		if ( stringSize > 0 )
		{
			InArchive.Serialize( ( void* )InValue.data(), stringSize );
		}
	}

	return InArchive;
}

template< typename TType >
FORCEINLINE CArchive& operator<<( CArchive& InArchive, std::vector< TType >& InValue )
{
	if ( InArchive.IsLoading() && InArchive.Ver() < VER_StaticMesh )
	{
		return InArchive;
	}

	uint32		arraySize = InValue.size();
	InArchive << arraySize;

	if ( arraySize > 0 )
	{
		if ( InArchive.IsLoading() )
		{
			InValue.resize( arraySize );
		}

		for ( uint32 index = 0; index < arraySize; ++index )
		{
			InArchive << InValue[ index ];
		}
	}

	return InArchive;
}

template< typename TType >
FORCEINLINE CArchive& operator<<( CArchive& InArchive, const std::vector< TType >& InValue )
{
	Assert( InArchive.IsSaving() );

	uint32		arraySize = InValue.size();
	InArchive << arraySize;

	if ( arraySize > 0 )
	{
		for ( uint32 index = 0; index < arraySize; ++index )
		{
			InArchive << InValue[ index ];
		}
	}

	return InArchive;
}

template< typename TKey, typename TValue, typename THasher = std::hash<TKey> >
FORCEINLINE CArchive& operator<<( CArchive& InArchive, std::unordered_map< TKey, TValue, THasher >& InValue )
{
	if ( InArchive.IsLoading() && InArchive.Ver() < VER_ShaderMap )
	{
		return InArchive;
	}

	uint32		arraySize = InValue.size();
	InArchive << arraySize;

	if ( arraySize > 0 )
	{
		if ( InArchive.IsLoading() )
		{
			for ( uint32 index = 0; index < arraySize; ++index )
			{
				TKey		key;
				TValue		value;

				InArchive << key;
				InArchive << value;
				InValue[ key ] = value;
			}
		}
		else
		{
			for ( auto itValue = InValue.begin(), itValueEnd = InValue.end(); itValue != itValueEnd; ++itValue )
			{
				InArchive << itValue->first;
				InArchive << itValue->second;
			}
		}		
	}

	return InArchive;
}

template< typename TKey, typename TValue, typename THasher = std::hash<TKey> >
FORCEINLINE CArchive& operator<<( CArchive& InArchive, const std::unordered_map< TKey, TValue, THasher >& InValue )
{
	Assert( InArchive.IsSaving() );

	uint32		arraySize = InValue.size();
	InArchive << arraySize;

	if ( arraySize > 0 )
	{
		for ( auto itValue = InValue.begin(), itValueEnd = InValue.end(); itValue != itValueEnd; ++itValue )
		{
			InArchive << itValue->first;
			InArchive << itValue->second;
		}
	}

	return InArchive;
}

#endif // !Archive_H
