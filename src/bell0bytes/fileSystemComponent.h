#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		26/06/2018 - Lenningen - Luxembourg
*
* Desc:		file system components of the DirectXApp class:
* Hist:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// c++ includes
#include <string>

// folder data
#include "folders.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////
namespace core
{
	class DirectXApp;
}

namespace util
{
	template<class T>
	class Expected;
}

namespace fileSystem
{
	class FileSystemComponent
	{
	private:
		// folder paths (documents)
		std::wstring pathToMyDocuments;				// path to the My Documents folder
		std::wstring pathToLogFiles;				// path to the folder containing log files
		std::wstring pathToUserConfigurationFiles;	// path to the folder containing the configuration files visible to the user
		std::wstring pathToDataFolder;				// path to the main data folder
		std::wstring pathToArtworkFolder;			// path to the main artwork folder
		std::wstring pathToAudioFolder;				// path to the main music folder

		// folder paths (application)
		std::wstring pathToLocalAppData;			// data bound to the user, the machine and the application (FOLDERID_LocalAppData)
		std::wstring pathToRoamingAppData;			// data bound to the user and the application (FOLDERID_RoamingAppData)
		std::wstring pathToProgramData;				// data bound to the machine and the application (FOLDERID_ProgramData)

		// application data subfolders
		const std::wstring manufacturerName;		// the manufacturer, i.e. bell0bytes
		const std::wstring applicationName;			// the game name, i.e. Tetris
		const std::wstring applicationVersion;		// the version number of the application, i.e. 0.1

		// configuration file names
		const std::wstring userPrefFile;			// configuration file editable by the user
		
		// key binding file names
		std::wstring keyBindingsFileKeyboard;		// game input configuration file for keyboard input
		std::wstring keyBindingsFileJoystick;		// game input configuration file for joystick input
		std::wstring keyBindingsFileGamepad;		// game input configuration file for gamepad input

		// booleans to keep track whether important files exist or not
		bool validUserConfigurationFile;			// true iff there was a valid user configuration file at startup
		bool activeFileLogger;						// true iff the logging service was successfully registered

		// private members to get folder paths and check configuration files
		const bool getPathToMyDocuments();			// stores the path to the My Documents folder in the appropriate member variable
		const bool getPathToApplicationDataFolders();// stores the paths to the application data folders
		const bool checkConfigurationFile();		// checks the state of the configuration file, creates the file if it does not exist yet
				
		// file logger
		void createLoggingService();				// creates the file logger and registers it as a service
				
	public:
		// the constructor gets some variables as input and then gets and sets all folder paths
		FileSystemComponent(const std::wstring& manufacturerName, const std::wstring& applicationName, const std::wstring& applicationVersion);
		~FileSystemComponent();

		// get file name depending on data folder
		const std::wstring openFile(const DataFolders&, const std::wstring&) const;	// gets the correct path to a given filename in a specified data folder
		
		// write resolution and fullscreen state to lua file
		util::Expected<void> saveConfiguration(const unsigned int width, const unsigned int height, const unsigned int index, const bool fullscreen, const bool enableJoystick, const bool enableGamepad, const float musicVolume, const float soundEffectsVolume) const;

		// get paths
		const std::wstring& getPathToConfigurationFiles() const;
		const std::wstring& getPrefsFile() const;
		const bool hasValidConfigurationFile() const;
		const bool fileLoggerIsActive() const;

		// get keymap files
		const std::wstring& getKeyboardFile() const;
		const std::wstring& getJoystickFile() const;
		const std::wstring& getGamepadFile() const;

		friend class core::DirectXApp;
	};
}