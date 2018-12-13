#pragma once

// include enums with automatic string conversion
#include "stringifiedEnum.h"

namespace fileSystem
{
	// define the data folders for the application
	// the first folders are root folders
	// the folders after "EndFolders* are subfolders
	ENUM_WITH_STRING(DataFolders,	(Data)\
									(Artwork)\
									(Audio)\
									(EndFolders)\
									(Bars)\
									(Buttons)\
									(Cursors)\
									(Entities)\
									(Icons)\
									(Logos)\
									(EndArtworkSubFolders)\
									(Music)\
									(Sounds)\
									(EndAudioSubFolders)\
									(End))
}