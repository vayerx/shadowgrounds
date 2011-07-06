
#include "precompiled.h"

#include "GameProfiles.h"
#include "GameProfilesEnumeration.h"
#include "../editor/file_wrapper.h"
#include "../util/SimpleParser.h"
#include "../system/Logger.h"
#include "GameStats.h"

#include "../util/FBCopyFile.h"
#include "../filesystem/input_stream.h"
#include "../filesystem/file_package_manager.h"
#include <stdio.h>
#include <string>
#include <direct.h>
#include <fstream>
#include <vector>

namespace game
{
	class GameProfilesImpl
	{
		private:
			// unnecessary...
			// GameProfilesImpl();
			// ~GameProfilesImpl();

			GameProfilesImpl() :
				currentProfiles( 4 ),
				currentProfileDirectories( 4 )
			{
			}

			std::vector< std::string > currentProfiles;
			std::vector< std::string > currentProfileDirectories;
			// char *currentProfile;
			// char *currentProfileDirectory;

			const char* getCurrentProfile( int player = 0 ) const
			{
				if( player >= 0 && player < (int)currentProfiles.size() )
				{
					return currentProfiles[ player ].c_str();
				}

				return NULL;
			}

			const char* getCurrentProfileDirectory( int player = 0 ) const
			{
				if( player >= 0 && player < (int)currentProfileDirectories.size() )
				{
					return currentProfileDirectories[ player ].c_str();
				}

				return NULL;
			}

			void setCurrentProfile( const std::string& profile, int player = 0 )
			{
				
				/*if (currentProfile != NULL)
				{
					delete [] currentProfile;
					currentProfile = NULL;
				}
				if (profile != NULL)
				{
					currentProfile = new char[strlen(profile) + 1];
					strcpy(currentProfile, profile);
				}

				if (currentProfileDirectory != NULL)
				{
					delete [] currentProfileDirectory;
					currentProfileDirectory = NULL;
				}
				if (profile != NULL)
				{
					currentProfileDirectory = new char[64 + strlen(profile) + 1];
					strcpy(currentProfileDirectory, "Profiles/");
					strcat(currentProfileDirectory, profile);
				}*/
				if( player >= 0 && player < (int) currentProfiles.size() )
				{
					currentProfiles[ player ] = profile;
					currentProfileDirectories[ player ] = "Profiles/" + profile;
				}
			}


			void loadCurrentProfile()
			{
				util::SimpleParser sp;
				setCurrentProfile("Default");
#ifdef LEGACY_FILES
				if (sp.loadFile("Profiles/current_profile.txt"))
#else
				if (sp.loadFile("profiles/current_profile.txt"))
#endif
				{
					while(sp.next())
					{
						char *key = sp.getKey();
						if (key != NULL && strcmp(key, "current_profile") == 0)
						{
							if (sp.getValue() != NULL)
							{
								if (doesProfileExist(sp.getValue()))
								{
									setCurrentProfile(sp.getValue());
								} else {
									Logger::getInstance()->warning("GameProfilesImpl::loadCurrentProfile - Current profile data does not exist (falling back to Default).");
								}
							}
						} 
						else if (key != NULL && strcmp(key, "player2_profile") == 0)
						{
							if (sp.getValue() != NULL)
							{
								if (doesProfileExist(sp.getValue()))
								{
									setCurrentProfile(sp.getValue(), 1);
								} else {
//									Logger::getInstance()->warning("GameProfilesImpl::loadCurrentProfile - Current profile data does not exist (falling back to Default).");
									Logger::getInstance()->debug("GameProfilesImpl::loadCurrentProfile - Current profile data does not exist (falling back to Default).");
								}
							}
						}
						else if (key != NULL && strcmp(key, "player3_profile") == 0)
						{
							if (sp.getValue() != NULL)
							{
								if (doesProfileExist(sp.getValue()))
								{
									setCurrentProfile(sp.getValue(), 2);
								} else {
//									Logger::getInstance()->warning("GameProfilesImpl::loadCurrentProfile - Current profile data does not exist (falling back to Default).");
									Logger::getInstance()->debug("GameProfilesImpl::loadCurrentProfile - Current profile data does not exist (falling back to Default).");
								}
							}
						}
						else if (key != NULL && strcmp(key, "player4_profile") == 0)
						{
							if (sp.getValue() != NULL)
							{
								if (doesProfileExist(sp.getValue()))
								{
									setCurrentProfile(sp.getValue(), 3);
								} else {
//									Logger::getInstance()->warning("GameProfilesImpl::loadCurrentProfile - Current profile data does not exist (falling back to Default).");
									Logger::getInstance()->debug("GameProfilesImpl::loadCurrentProfile - Current profile data does not exist (falling back to Default).");
								}
							}
						}
					}
				}
			}


			void saveCurrentProfile()
			{
#ifdef LEGACY_FILES
				FILE *f = fopen("Profiles/current_profile.txt", "wb");
#else
				FILE *f = fopen("profiles/current_profile.txt", "wb");
#endif
				if (f != NULL)
				{/*
					char *buf = new char[64 + strlen(getCurrentProfile()) + 1];
					strcpy(buf, "current_profile=");
					strcat(buf, getCurrentProfile());
					strcat(buf, "\r\n\r\n");
					fwrite(buf, strlen(buf), 1, f);
					delete [] buf;*/

					{
						std::string buf = std::string( "current_profile=" ) + getCurrentProfile( 0 ) + "\r\n\r\n";
						fwrite( buf.c_str(), buf.size(), 1, f);
					}

					{
						std::string buf = std::string( "player2_profile=" ) + getCurrentProfile( 1 ) + "\r\n\r\n";
						fwrite( buf.c_str(), buf.size(), 1, f);
					}

					{
						std::string buf = std::string( "player3_profile=" ) + getCurrentProfile( 2 ) + "\r\n\r\n";
						fwrite( buf.c_str(), buf.size(), 1, f);
					}

					{
						std::string buf = std::string( "player4_profile=" ) + getCurrentProfile( 3 ) + "\r\n\r\n";
						fwrite( buf.c_str(), buf.size(), 1, f);
					}

					fclose(f);
				}
			}

			void newProfile( const std::string& profile )
			{
				_mkdir( ( "Profiles/"+profile ).c_str() );
				_mkdir( ( "Profiles/"+profile + "/Save" ).c_str() );
				_mkdir( ( "Profiles/"+profile + "/Config" ).c_str() );
				
				// should copy the default keybinds to the 
				// Profiles/"profile"/Config/
#ifdef LEGACY_FILES
				copyFile( "Data/Misc/keybinds.txt", "Profiles/"+profile+"/Config/keybinds.txt" );
#else
				copyFile( "data/misc/default_keybinds.txt", "profiles/"+profile+"/config/keybinds.txt" );
#endif

			}
			
			void deleteProfile( const std::string& profile )
			{
				if( doesProfileExist( profile.c_str() ) )
				{
#ifdef LEGACY_FILES
					removeDirectory( "Profiles/" + profile + "/Save" );
					removeDirectory( "Profiles/" + profile + "/Config" );
					removeDirectory( "Profiles/" + profile );

					//HAXHAXHAX
					Sleep(100);
					removeDirectory( "Profiles/" + profile + "/Save" );
					removeDirectory( "Profiles/" + profile + "/Config" );
					removeDirectory( "Profiles/" + profile );
#else
					removeDirectory( "profiles/" + profile + "/save" );
					removeDirectory( "profiles/" + profile + "/config" );
					removeDirectory( "profiles/" + profile );

					//HAXHAXHAX
					Sleep(100);
					removeDirectory( "profiles/" + profile + "/save" );
					removeDirectory( "profiles/" + profile + "/config" );
					removeDirectory( "profiles/" + profile );
#endif
				}
			}

			bool doesProfileExist(const char *temp) const
			{
				if( temp == NULL ) 
					return false;
				std::string profile( temp );

				frozenbyte::editor::FileWrapper fwbase(
#ifdef LEGACY_FILES
				std::string("Profiles"), std::string("*"), true);
#else
				std::string("profiles"), std::string("*"), true);
#endif
				std::vector<std::string> tmplist = fwbase.getAllDirs();
				for (int j = 0; j < (int)tmplist.size(); j++)
				{
					if( tmplist[j] == profile ) 
						return true;
				}

				return false;
			}

			bool isProfileValid( const char* temp ) const
			{
				if( temp == NULL ) return false;

				std::string profile( temp );
				
				// if( doesProfileExist( temp ) ) 
				//	return false;

				if( profile.empty() )
					return false;

				if( profile.find_first_of( "/\\:*|?<>" ) != profile.npos ) 
					return false;

				if( profile[0] == '.' || profile[ profile.size() - 1 ] == '.' )
					return false;
				
				return true;
			}

			void removeDirectory( const std::string& directory )
			{
				// Removes a directory and its files 
				// ( currently does not delete its subfolders )
				{
					frozenbyte::editor::FileWrapper fwbase( directory , "*.*" );
					std::vector<std::string> tmplist = fwbase.getAllDirs();
					int i;
					/*
					for ( i = 0; i < (int)tmplist.size(); i++)
					{
						removeDirectory( directory + "/" + tmplist[ i ] );
					}*/

					tmplist = fwbase.getAllFiles();
					for( i = 0; i < (int)tmplist.size(); i++ )
					{
						//if( _unlink( ( tmplist[i] ).c_str() ) != 0 )
						if(DeleteFile(tmplist[i].c_str()) == FALSE)
						{
							Logger::getInstance()->error( ( "GameProfilesImpl::removeDirectory() Could not remove file: " + tmplist[i] ).c_str()  );
						}
					}
				}

				//if( _rmdir( directory.c_str() ) != 0 ) 
				if(RemoveDirectory(directory.c_str()) == FALSE)
				{
					Logger::getInstance()->error( "GameProfilesImpl::removeDirectory() Could not remove directory" );
				};
			}

			void copyFile( const std::string& from, const std::string& to )
			{
				util::FBCopyFile::copyFile(from, to);
			}

		friend class GameProfiles;
	};


	GameProfiles::GameProfiles()
	{
		this->impl = new GameProfilesImpl();
		// impl->currentProfile = NULL;
		// impl->currentProfileDirectory = NULL;
		impl->loadCurrentProfile();
	}


	GameProfiles::~GameProfiles()
	{
		impl->saveCurrentProfile();
		impl->setCurrentProfile( "" );
		delete this->impl;
	}


	const char *GameProfiles::getCurrentProfile( int player ) const
	{
		return impl->getCurrentProfile( player );
	}


	const char *GameProfiles::getProfileDirectory( int player ) const
	{
		return impl->getCurrentProfileDirectory( player );
	}


	bool GameProfiles::isProfileNameValid( const char* profile ) const
	{
		return impl->isProfileValid( profile );
	}

	bool GameProfiles::doesProfileExist( int player ) const
	{
		return impl->doesProfileExist( getCurrentProfile( player ) );
	}

	void GameProfiles::createNewProfile(const char *profile)
	{
		if (!isProfileNameValid(profile))
		{
			Logger::getInstance()->error("GameProfiles::createNewProfile - Attempt to create an invalid profile.");
			return;
		}

		impl->newProfile( profile );
		
	}


	void GameProfiles::deleteProfile(const char *profile)
	{
		if (!isProfileNameValid(profile))
		{
			Logger::getInstance()->error("GameProfiles::deleteProfile - Attempt to delete an invalid profile.");
			return;
		}

		impl->deleteProfile( profile );
	}


	void GameProfiles::setCurrentProfile(const char *profile, int player, bool safetyChecks)
	{
		if( player != 0 && std::string( profile ).empty() )
		{
			impl->setCurrentProfile( "", player );
		}

		if (safetyChecks && !isProfileNameValid(profile))
		{
			if (player == 0)
			{
				Logger::getInstance()->error("GameProfiles::setCurrentProfile - Attempt to set an invalid profile.");
			} else {
				Logger::getInstance()->debug("GameProfiles::setCurrentProfile - Attempt to set an invalid profile.");
			}
			return;
		}

		if (safetyChecks && !impl->doesProfileExist(profile))
		{
			Logger::getInstance()->error("GameProfiles::setCurrentProfile - Attempt to select a non-existing profile.");
			return;
		}

		impl->setCurrentProfile(profile, player);
	}


	std::string GameProfiles::convertFilenameToProfileDirectory(const std::string &filename)
	{
		std::string ret = std::string( impl->getCurrentProfileDirectory() );
		ret += "/";
		ret += filename;

		return ret;
	}


	GameProfilesEnumeration *GameProfiles::getProfileList()
	{
		GameProfilesEnumeration *ret = new GameProfilesEnumeration();
		ret->currentPos = 0;

		frozenbyte::editor::FileWrapper fwbase(
			std::string("Profiles"), std::string("*"), true);
		std::vector<std::string> tmplist = fwbase.getAllDirs();
		for (int j = 0; j < (int)tmplist.size(); j++)
		{
			std::string realdirname = tmplist[j];
			// TODO: process dir name somehow? (all caps, or something?)
			ret->profileList.push_back(realdirname);

			// TODO: handle too many profiles correctly? (or maybe just leave that to gui)
		}

		return ret;
	}

}

