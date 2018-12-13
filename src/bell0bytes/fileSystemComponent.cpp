// include the header
#include "fileSystemComponent.h"

// windows
#include <Shlobj.h>				// directory folders

// bell0bytes utilities
#include "expected.h"			// error handling
#include "serviceLocator.h"		// the service locator pattern

namespace fileSystem
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructors ///////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	FileSystemComponent::FileSystemComponent(const std::wstring& manufacturerName, const std::wstring& applicationName, const std::wstring& applicationVersion) : validUserConfigurationFile(false), manufacturerName(manufacturerName), applicationName(applicationName), applicationVersion(applicationVersion), activeFileLogger(false), userPrefFile(L"bell0prefs.lua")
	{
		// get path to My Documents folder
		if (!getPathToMyDocuments())
		{
			// show error message on a message box
			MessageBox(NULL, L"Unable to retrieve the path to the My Documents folder!", L"Critical Error!", MB_ICONEXCLAMATION | MB_OK);
			throw std::runtime_error("Unable to retrieve the path to the My Documents folder!");
		}

		// get path to application data folder
		if (!getPathToApplicationDataFolders())
		{
			// show error message on a message box
			MessageBox(NULL, L"Unable to retrieve the path to the application data folders!", L"Critical Error!", MB_ICONEXCLAMATION | MB_OK);
			throw std::runtime_error("Unable to retrieve the path to the aplication data folders!");
		}

		// create the logger
		try { createLoggingService(); }
		catch (std::runtime_error& e)
		{
			// show error message on a message box
			MessageBox(NULL, L"Unable to start the logging service!", L"Critical Error!", MB_ICONEXCLAMATION | MB_OK);
			throw e;
		}

		// check for valid configuration file
		if (!checkConfigurationFile())
			util::ServiceLocator::getFileLogger()->print<util::SeverityType::warning>("Non-existent or invalid configuration file. Starting with default settings.");
	}
	FileSystemComponent::~FileSystemComponent()
	{ }

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Configuration Files ///////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> FileSystemComponent::saveConfiguration(const unsigned int width, const unsigned int height, const unsigned int index, const bool fullscreen, const bool enableJoystick, const bool enableGamepad, const float musicVolume, const float soundEffectsVolume) const
	{
		// create directory (if it does not exist already)
		HRESULT hr;
		hr = SHCreateDirectory(NULL, pathToUserConfigurationFiles.c_str());
#ifndef NDEBUG
		if (FAILED(hr))
			return std::runtime_error("Critical error: unable to get path to 'My Documents' folder!");
#endif
		// append name of the log file to the path
		std::wstring pathToPrefFile = pathToUserConfigurationFiles + L"\\" + userPrefFile; // L"\\bell0prefs.lua";

		// the directory exists, check if the log file is accessible
		std::ifstream prefStream(pathToPrefFile.c_str());
		if (prefStream.good())
		{
			// the file exists and can be read
			try
			{
				util::Logger<util::FileLogPolicy> prefFileCreator(pathToPrefFile.c_str());
				std::stringstream printPref;
				printPref << "config =\r\n{ \r\n\tfullscreen = " << std::boolalpha << fullscreen << ",\r\n\tresolution = { width = " << width << ", height = " << height << ", index = " << index << " },\r\n\tjoystick = " << enableJoystick << ",\r\n\tgamepad = " << enableGamepad << ",\r\n" << "\tmusicVolume = " << musicVolume << ",\r\n " << "\tsoundEffectsVolume = " << soundEffectsVolume << "\r\n}";
				prefFileCreator.print<util::config>(printPref.str());
			}
			catch (std::exception& e)
			{
				return e;
			}
		}
		else
		{
			// the file does not exist --> create it
			try
			{
				util::Logger<util::FileLogPolicy> prefFileCreator(pathToPrefFile.c_str());
				std::stringstream printPref;
				printPref << "config =\r\n{ \r\n\tfullscreen = " << std::boolalpha << fullscreen << ",\r\n\tresolution = { width = " << width << ", height = " << height << ", index = " << index << " },\r\n\tjoystick = " << enableJoystick << ",\r\n\tgamepad = " << enableGamepad << ",\r\n" << "\tmusicVolume = " << musicVolume << ",\r\n " << "\tsoundEffectsVolume = " << soundEffectsVolume << "\r\n}";
				prefFileCreator.print<util::config>(printPref.str());
			}
			catch (std::exception& e)
			{
				return e;
			}
		}

		// return success
		return {};
	}
	const bool FileSystemComponent::checkConfigurationFile()
	{
		// create directory (if it does not exist already)
		HRESULT hr;
		hr = SHCreateDirectory(NULL, pathToUserConfigurationFiles.c_str());
#ifndef NDEBUG
		if (FAILED(hr))
			return false;
#endif
		// append name of the configuration file to the path
		std::wstring pathToPrefFile = pathToUserConfigurationFiles + L"\\" + userPrefFile; // L"\\bell0prefs.lua";

		  // the directory exists, check if the log file is accessible
		std::ifstream prefStream(pathToPrefFile.c_str());
		if (prefStream.good())
		{
			// the file exists and can be read
			if (prefStream.peek() == std::ifstream::traits_type::eof())
			{
				// the file is empty, create it
				try
				{
					util::Logger<util::FileLogPolicy> prefFileCreator(pathToPrefFile.c_str());
					std::stringstream printPref;
					printPref << "config =\r\n{ \r\n\tfullscreen = true,\r\n\tresolution = { width = 1920, height = 1080 },\r\n\tjoystick = false,\r\n\tgamepad = false\r\n\tmusicVolume = 1,\r\n\tsoundEffectsVolume = 1\r\n}";
					prefFileCreator.print<util::config>(printPref.str());
				}
				catch (std::runtime_error)
				{
					return false;
				}
			}
		}
		else
		{
			// the file does not exist --> create it
			try
			{
				util::Logger<util::FileLogPolicy> prefFileCreator(pathToPrefFile.c_str());
				std::stringstream printPref;
				printPref << "config =\r\n{ \r\n\tfullscreen = true,\r\n\tresolution = { width = 1920, height = 1080 },\r\n\tjoystick = false,\r\n\tgamepad = false\r\n\tmusicVolume = 1,\r\n\tsoundEffectsVolume = 1\r\n}";
				prefFileCreator.print<util::config>(printPref.str());
			}
			catch (std::runtime_error)
			{
				return false;
			}
		}

		validUserConfigurationFile = true;
		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Folder Paths //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	const bool FileSystemComponent::getPathToMyDocuments()
	{
		PWSTR docPath = NULL;

#ifndef NDEBUG
		HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, NULL, NULL, &docPath);
#else
		SHGetKnownFolderPath(FOLDERID_Documents, NULL, NULL, &docPath);
#endif

		// debug mode only: make sure the operation succeeded
#ifndef NDEBUG
		if (FAILED(hr))
			return false;
#endif
		// store the path
		pathToMyDocuments = docPath;

		// delete the wstring pointer to avoid memory leak
		::CoTaskMemFree(static_cast<void*>(docPath));

		// set up log and configuration paths

		// append custom folder to path

		// log folder
		std::wstringstream path;
		path << pathToMyDocuments << L"\\bell0bytes\\bell0tutorials\\Logs";
		pathToLogFiles = path.str();

		// settings folder
		path.str(std::wstring());
		path.clear();
		path << pathToMyDocuments << L"\\bell0bytes\\bell0tutorials\\Settings";
		pathToUserConfigurationFiles = path.str();

		// data folder
		path.str(std::wstring());
		path.clear();
		path << pathToMyDocuments << L"\\bell0bytes\\bell0tutorials\\Data";
		pathToDataFolder = path.str();

		// artwork folder
		path.str(std::wstring());
		path.clear();
		path << pathToDataFolder << L"\\Artwork";
		pathToArtworkFolder = path.str();

		// audio folder
		path.str(std::wstring());
		path.clear();
		path << pathToDataFolder << L"\\Audio";
		pathToAudioFolder = path.str();

		// return success
		return true;
	}
	const bool FileSystemComponent::getPathToApplicationDataFolders()
	{
		HRESULT hr;
		PWSTR appDataPath = NULL;

		// get and store path to local app data
#ifndef NDEBUG
		hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, NULL, NULL, &appDataPath);
#else
		SHGetKnownFolderPath(FOLDERID_LocalAppData, NULL, NULL, &appDataPath);
#endif
		pathToLocalAppData = appDataPath;

		// get and store path to roaming app data
		appDataPath = NULL;

#ifndef NDEBUG
		hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, NULL, NULL, &appDataPath);
#else
		SHGetKnownFolderPath(FOLDERID_RoamingAppData, NULL, NULL, &appDataPath);
#endif
		pathToRoamingAppData = appDataPath;

		// get and store path to program data
		appDataPath = NULL;

#ifndef NDEBUG
		hr = SHGetKnownFolderPath(FOLDERID_ProgramData, NULL, NULL, &appDataPath);
#else
		SHGetKnownFolderPath(FOLDERID_ProgramData, NULL, NULL, &appDataPath);
#endif
		pathToProgramData = appDataPath;

		// delete the wstring pointer to avoid memory leak
		::CoTaskMemFree(static_cast<void*>(appDataPath));


		// create subdirectories

		// append custom folder to local data path
		std::wstringstream path;
		path << pathToLocalAppData << "\\" << manufacturerName << "\\" << applicationName << "\\" << applicationVersion << "\\";
		pathToLocalAppData = path.str();

		// Create the path (including all sub-directories) if it doesn't already exist
		if (FAILED(SHCreateDirectoryEx(NULL, pathToLocalAppData.c_str(), NULL)))
			return false;

		// append custom folder to roaming data path
		path.str(std::wstring());
		path.clear();
		path << pathToRoamingAppData << "\\" << manufacturerName << "\\" << applicationName << "\\" << applicationVersion << "\\";
		pathToRoamingAppData = path.str();

		// Create the path (including all sub-directories) if it doesn't already exist
		if (FAILED(SHCreateDirectoryEx(NULL, pathToRoamingAppData.c_str(), NULL)))
			return false;

		// append custom folder to application data path
		path.str(std::wstring());
		path.clear();
		path << pathToProgramData << "\\" << manufacturerName << "\\" << applicationName << "\\" << applicationVersion << "\\";
		pathToProgramData = path.str();

		// Create the path (including all sub-directories) if it doesn't already exist
		if (FAILED(SHCreateDirectoryEx(NULL, pathToProgramData.c_str(), NULL)))
			return false;

		// set file paths
		keyBindingsFileKeyboard = pathToLocalAppData + L"keyBindingsKeyboard.dat";
		keyBindingsFileJoystick = pathToLocalAppData + L"keyBindingsJoystick.dat";
		keyBindingsFileGamepad = pathToLocalAppData + L"keyBindingsGamepad.dat";

		// return success
		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Open Files ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	const std::wstring FileSystemComponent::openFile(const fileSystem::DataFolders& dataFolder, const std::wstring& filename) const
	{
		std::wstringstream file;

		if (dataFolder == fileSystem::DataFolders::Data)
		{
			// the file is in the main data folder
			file << pathToDataFolder << L"\\" << filename;
			return file.str();
		}

		if (dataFolder < fileSystem::DataFolders::EndFolders)
		{
			// the file is in a main folder
			file << pathToDataFolder << L"\\" << enumToString(dataFolder) << L"\\" << filename;
			return file.str();
		}

		if (dataFolder > fileSystem::DataFolders::EndFolders && dataFolder < fileSystem::DataFolders::EndArtworkSubFolders)
		{
			// the file is in an artwork subfolder
			file << pathToArtworkFolder << L"\\" << enumToString(dataFolder) << L"\\" << filename;
			return file.str();
		}

		if (dataFolder > fileSystem::DataFolders::EndArtworkSubFolders && dataFolder < fileSystem::DataFolders::EndAudioSubFolders)
		{
			// the file is in an audio subfolder
			file << pathToAudioFolder << L"\\" << enumToString(dataFolder) << L"\\" << filename;
			return file.str();
		}

		return L"Unable to locate file!";
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Logger ////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void FileSystemComponent::createLoggingService()
	{
		// create directory (if it does not exist already)
		HRESULT hr;
		hr = SHCreateDirectory(NULL, pathToLogFiles.c_str());

		// debug mode only: make sure the operator succeeded
#ifndef NDEBUG
		if (FAILED(hr))
			throw std::runtime_error("Unable to create directory!");
#endif

		// append name of the log file to the path
		std::wstring logFile = pathToLogFiles + L"\\bell0engine.log";

		// create file logger
		std::shared_ptr<util::Logger<util::FileLogPolicy> > engineLogger(new util::Logger<util::FileLogPolicy>(logFile));

		// set logger to active
		activeFileLogger = true;

		// set name of current thread
		engineLogger->setThreadName("mainThread");

		// register the logging service
		util::ServiceLocator::provideFileLoggingService(engineLogger);

#ifndef NDEBUG
		// print starting message
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The file logger was created successfully.");
#endif
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Getters ///////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	const std::wstring& FileSystemComponent::getPathToConfigurationFiles() const
	{
		return pathToUserConfigurationFiles;
	};
	const std::wstring& FileSystemComponent::getPrefsFile() const
	{
		return userPrefFile;
	};
	const bool FileSystemComponent::hasValidConfigurationFile() const
	{
		return validUserConfigurationFile;
	};
	const bool FileSystemComponent::fileLoggerIsActive() const
	{
		return activeFileLogger;
	};
	const std::wstring& FileSystemComponent::getKeyboardFile() const
	{
		return keyBindingsFileKeyboard;
	}
	const std::wstring& FileSystemComponent::getJoystickFile() const
	{
		return keyBindingsFileJoystick;
	}
	const std::wstring& FileSystemComponent::getGamepadFile() const
	{
		return keyBindingsFileGamepad;
	}
}
