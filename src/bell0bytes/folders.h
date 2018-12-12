#pragma once

#include "stringifiedEnum.h"

namespace fileSystem
{
	// define the game commands
	ENUM_WITH_STRING(DataFolders,	(Data)\
									(Artwork)\
									(Music)\
									(EndFolders)\
									(Buttons)\
									(Cursors)\
									(Logos)\
									(EndArtworkSubFolders)\
									(End))
}