#include "precompiled.h"

#include <limits.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <boost/lexical_cast.hpp>

#include "CinematicScreen.h"
#include "../ogui/Ogui.h"
#include "../ogui/OguiFormattedText.h"
#include "../ui/CreditsMenu.h"

#include "../game/Game.h"
#include "../game/GameUI.h"
#include "../game/DHLocaleManager.h"
#include "../system/Timer.h"
#include "../sound/SoundMixer.h"
#include "../game/GameStats.h"
#include "../game/scripting/GameScripting.h"

#include "../game/SimpleOptions.h"
#include "../game/options/options_video.h"

#include "../system/Logger.h"

#include <IStorm3D.h>
#include <istorm3d_videostreamer.h>

#include "../util/StringUtil.h"

using namespace game;

namespace ui {
///////////////////////////////////////////////////////////////////////////////

class CinematicScreen::CinematicScreenImpl
{
public:

	struct ScrollEffect
	{
		ScrollEffect() :
			yPosition( 0 ),
			lastYPosition( 0 ),
			lastUpdate( 0 ),
			speed( 0 )
		{
		}

		float					yPosition;
		int						lastYPosition;
		int						lastUpdate;
		float					speed;
	};

	struct ImageFadeEffect
	{
		ImageFadeEffect() :
			fadeLength( 0 ),
			transparency( 1.0f ),
			initTime( 0 ),
			image( NULL )
		{
		}

		int			fadeLength;
		float		transparency; // 0 to 1
		int			initTime;
		IOguiImage*	image;
	};

	//=========================================================================
	
	int					initTime;
	int					timeToDie;
	Ogui*				ogui;
	Game* game;
	OguiWindow*			window;
	int				killMe;
	IOguiImage*			backgroundVideo;
	ScrollEffect*		scrollEffect;
	ImageFadeEffect*	imageFadeEffect;
	IStorm3D_VideoStreamer*	backgroundVideoStream;
	OguiButton*				backgroundImage;
	OguiButton* wideScreenBars[2];
	std::string cinematic_name;
  
	std::list< IOguiImage* >			images;
	std::list< IOguiFont* >				fonts;
	std::vector< OguiFormattedText* >	textAreas;

	std::map< int, std::pair< int, std::string > >		textsByTime;
	// map->first			= time to insert the text
	// map->second->first	= textArea number where the text is inserted
	// map->second->second	= text to be inserted in the textarea

	std::map< int, std::pair< int, int > >				effectsByTime;
	// map->first			= time to start the effect
	// map->second->first	= effect to be startted
	// map->second->second	= effect duration

	std::map< int, std::pair< std::string, int > >		pictureFadeByTime;
	// map->fisrt			= time to start the picture fade
	// map->second->first	= picture to be faded
	// map->second->second	= length of the fade

	struct QueuedText
	{
		QueuedText() : textArea(0), time(0), typingEffect(false) {}
		std::string text;
		int textArea;
		int time;
		bool typingEffect;
		int typingDelay;
	};

	std::vector< QueuedText > textQueue;
	unsigned int currentTextQueueEntry;

	struct TextQueueSorter
	{
		bool operator () (const QueuedText &a, const QueuedText &b) const
		{
			return a.time < b.time;
		}
	};


	struct TypingEffect
	{
		TypingEffect() : active(false), lastTyped(0) {}
		std::string text;
		std::string typingSound;
		bool active;
		int typingDelay;
		int typingStart;
		int lastTyped;
	};

	std::vector<TypingEffect> typingEffects;
  


  struct TextFader
  {
		TextFader() : time_start(INT_MAX), cliprect(false), button(NULL) {}
		int time_start;
		int time_end;
		int fade_duration;
		int start_x,start_y,start_w,start_h,start_alpha;
		int end_x,end_y,end_w,end_h,end_alpha;
		bool cliprect;
		int cliprect_x,cliprect_y,cliprect_w,cliprect_h;
		OguiButton *button;
		std::string image;
  };
  std::vector< TextFader > textFaders;



	struct SoundEffect
	{
		std::string file;
		int volume;
		int time;
		int handle;
		bool stream;
		bool speech;
	};
	std::vector< SoundEffect > soundEffects;
	unsigned int currentSoundEffect;

	struct SoundEffectSorter
	{
		bool operator () (const SoundEffect &a, const SoundEffect &b) const
		{
			return a.time < b.time;
		}
	};


	//=========================================================================

	CinematicScreenImpl( Ogui* ogui, Game *game, const std::string& cinematic_name, sfx::SoundMixer *mixer, IStorm3D* storm3d ) :
		initTime( 0 ),
		timeToDie( 0 ),
		ogui( ogui ),
		game(game),
		window( NULL ),
		killMe( false ),
		backgroundVideo( NULL ),
		scrollEffect( NULL ),
		imageFadeEffect( NULL ),
		backgroundVideoStream( NULL ),
		backgroundImage( NULL ),
		cinematic_name(cinematic_name),
		images(),
		fonts(),
		textAreas(),
		textsByTime(),
		effectsByTime(),
		pictureFadeByTime(),
		currentTextQueueEntry(0),
		currentSoundEffect(0)
	{
		wideScreenBars[0] = NULL;
		wideScreenBars[1] = NULL;

		// initTime = getTime();

		const std::string&	prefix = "cinematic_screen_" + cinematic_name;

		/*window = ogui->CreateSimpleWindow( 0, 0, 1024, 768, "Data/GUI/debug.tga" );

		*/
		timeToDie = getLocaleGuiInt( ( prefix + "_time_to_die" ).c_str(), -1 );
		window = ogui->CreateSimpleWindow(	getLocaleGuiInt( ( prefix + "_window_x" ).c_str(), 0 ), getLocaleGuiInt( ( prefix + "_window_y" ).c_str(), 0 ), 
											getLocaleGuiInt( ( prefix + "_window_w" ).c_str(), 0 ), getLocaleGuiInt( ( prefix + "_window_h" ).c_str(), 0 ),
											NULL );

		{
			IOguiImage* image = ogui->LoadOguiImage( getLocaleGuiString( ( prefix + "_window_img" ).c_str() ) );
			images.push_back( image );

			backgroundImage = ogui->CreateSimpleImageButton( window, getLocaleGuiInt( ( prefix + "_window_x" ).c_str(), 0 ), getLocaleGuiInt( ( prefix + "_window_y" ).c_str(), 0 ), getLocaleGuiInt( ( prefix + "_window_w" ).c_str(), 0 ), getLocaleGuiInt( ( prefix + "_window_h" ).c_str(), 0 ), NULL, NULL, NULL );
			backgroundImage->SetDisabled( true );
			// backgroundImage->SetDisabledImage( image );

			window->setBackgroundImage( image );
		}

		if( SimpleOptions::getBool( DH_OPT_B_VIDEO_ENABLED ) )
		{
			// sfx::SoundMixer *mixer = game->gameUI->getSoundMixer();
			IStorm3D_StreamBuilder *builder = 0;
			if(mixer)
				builder = mixer->getStreamBuilder();

			std::string video = getLocaleGuiString( ( prefix + "_window_video" ).c_str() );
			// backgroundVideo = ogui->LoadOguiVideo( video.c_str(), builder );

			backgroundVideoStream = storm3d->CreateVideoStreamer( video.c_str(), builder, false );
			float xm = 0.0f, ym = 0.0f;
			if (backgroundVideoStream) backgroundVideoStream->getTextureCoords(xm, ym);
			backgroundVideo = ogui->ConvertVideoToImage( backgroundVideoStream, builder );

			if( backgroundVideo )
			{
				window->setBackgroundImage( backgroundVideo );
			}


			const char *video_aspect_str = NULL;
			if(::DHLocaleManager::getInstance()->getString(::DHLocaleManager::BANK_GUI, (prefix + "_window_video_aspect").c_str(), &video_aspect_str))
			{
				float normal_aspect = (window->GetSizeX() * ogui->GetScaleX()) / (float)(window->GetSizeY() * ogui->GetScaleY());

				float video_aspect = (float)atof(video_aspect_str);

				float texcoord_multiplier = (video_aspect / normal_aspect);
				float texcoord_offset = (1.0f - texcoord_multiplier) * 0.5f;

				if(texcoord_multiplier > 1.0f)
				{
					window->setBackgroundRepeatFactor(xm, texcoord_multiplier * ym);
					window->setBackgroundScroll(0.0f, texcoord_offset);

					int w = window->GetSizeX();
					int h = (int)((1.0f - 1.0f / texcoord_multiplier) * window->GetSizeY() * 0.5f + 0.5f);
					const char *img = getLocaleGuiString( ( prefix + "_window_img" ).c_str() );
					wideScreenBars[0] = ogui->CreateSimpleImageButton(window, 0, 0, w, h, NULL, NULL, NULL, img, 0, NULL, false);
					wideScreenBars[0]->SetDisabled(true);

					int y = h + (int)(window->GetSizeY() / texcoord_multiplier + 0.5f);
					wideScreenBars[1] = ogui->CreateSimpleImageButton(window, 0, y, w, h, NULL, NULL, NULL, img, 0, NULL, false);
					wideScreenBars[1]->SetDisabled(true);

				}
			}
		}

		window->SetUnmovable();
		window->Raise();
		window->Show();


		// window->StartEffect( OGUI_WINDOW_EFFECT_TEXTTYPE, 1000 );

		// images
		{
			int num_of = getLocaleGuiInt( ( prefix + "_image_amount" ).c_str(), 0 );

			for( int i = 0; i < num_of; i++ )
			{
				// const std::string& pre_text = ( prefix + "_textarea_" + boost::lexical_cast< std::string >( i ) );
				const std::string& pre_effect = prefix + "_image_" + boost::lexical_cast< std::string >( i );
				int time = getLocaleGuiInt( ( pre_effect + "_time" ).c_str(), 0 );
				int length = getLocaleGuiInt( ( pre_effect + "_length" ).c_str(), 0 );
				std::string file = getLocaleGuiString( ( pre_effect + "_img" ).c_str() );
				
				{
					pictureFadeByTime.insert( std::pair< int, std::pair< std::string, int > >( time, std::pair< std::string, int >( file, length ) ) );
				}
			}			
		}

		// effects
		{
			int num_of = getLocaleGuiInt( ( prefix + "_effects_amount" ).c_str(), 0 );

			for( int i = 0; i < num_of; i++ )
			{
				// const std::string& pre_text = ( prefix + "_textarea_" + boost::lexical_cast< std::string >( i ) );
				const std::string& pre_effect = prefix + "_effect_" + boost::lexical_cast< std::string >( i );
				int time = getLocaleGuiInt( ( pre_effect + "_time" ).c_str(), 0 );
				int effect = getLocaleGuiInt( ( pre_effect + "_id" ).c_str(), 0 );
				int length = getLocaleGuiInt( ( pre_effect + "_duration" ).c_str(), 0 );
				
				if( time <= 0 )
				{
					window->StartEffect( effect, length );
				}
				else
				{
					effectsByTime.insert( std::pair< int, std::pair< int, int > >( time, std::pair< int, int >( effect, length ) ) );
				}
			}			
		}

		// text areas
		{
			int num_of = getLocaleGuiInt( ( prefix + "_textarea_amount" ).c_str(), 0 );

			textAreas.resize( num_of );
			typingEffects.resize( num_of );

			for( int i = 0; i < num_of; i++ )
			{
				const std::string& pre_text = ( prefix + "_textarea_" + boost::lexical_cast< std::string >( i ) );
				textAreas[ i ] = new OguiFormattedText( window, ogui, 
														getLocaleGuiInt( ( pre_text + "_x" ).c_str(), 0 ), getLocaleGuiInt( ( pre_text + "_y" ).c_str(), 0 ),
														getLocaleGuiInt( ( pre_text + "_w" ).c_str(), 0 ), getLocaleGuiInt( ( pre_text + "_h" ).c_str(), 0 ) );
				
			

				// fonts
				{
					IOguiFont* font = ogui->LoadFont( getLocaleGuiString( ( pre_text + "_font" ).c_str() ) );
					textAreas[i]->setFont( font );
					fonts.push_back( font );

					font = ogui->LoadFont( getLocaleGuiString( ( pre_text + "_font_b" ).c_str() ) );
					if( font )
					{
						textAreas[i]->registerFont( "b", font );
						fonts.push_back( font );
					}

					font = ogui->LoadFont( getLocaleGuiString( ( pre_text + "_font_i" ).c_str() ) );
					if( font )
					{
						textAreas[i]->registerFont( "i", font );
						fonts.push_back( font );
					}

					
					font = ogui->LoadFont( getLocaleGuiString( ( pre_text + "_font_u" ).c_str() ) );
					if( font )
					{
						textAreas[i]->registerFont( "u", font );
						fonts.push_back( font );
					}

					
					font = ogui->LoadFont( getLocaleGuiString( ( pre_text + "_font_h1" ).c_str() ) );
					if( font )
					{
						textAreas[i]->registerFont( "h1", font );
						fonts.push_back( font );
					}
				}

				// align
				{
					std::string align = getLocaleGuiString( ( pre_text + "_align" ).c_str() );
					if( align == "center" ) textAreas[ i ]->setTextHAlign( OguiButton::TEXT_H_ALIGN_CENTER );
					if( align == "left" )	textAreas[ i ]->setTextHAlign( OguiButton::TEXT_H_ALIGN_LEFT );
					if( align == "right" )	textAreas[ i ]->setTextHAlign( OguiButton::TEXT_H_ALIGN_RIGHT );
				}
				
				if(::DHLocaleManager::getInstance()->hasString(::DHLocaleManager::BANK_GUI, ( pre_text + "_typingsound" ).c_str()))
					typingEffects[i].typingSound = getLocaleGuiString( ( pre_text + "_typingsound" ).c_str() );
				
				int num_of_texts = getLocaleGuiInt( ( pre_text + "_text_amount" ).c_str(), 0 );
				for( int j = 0; j < num_of_texts; j++ )
				{
					// get time
					int time = getLocaleGuiInt( ( pre_text + "_text_" + boost::lexical_cast< std::string >( j )  + "_insert_time" ).c_str(), 0 );

					// get text
					std::string text = getLocaleGuiString( ( pre_text + "_text_" + boost::lexical_cast< std::string >( j ) ).c_str() ); 

					// translate $(locales)
#ifdef PROJECT_SURVIVOR
					text = convertLocaleSubtitleString(text.c_str());
#else
					text = convertLocaleGuiString(text.c_str());
#endif

					// credits hack
					if( ( pre_text + "_text_" + boost::lexical_cast< std::string >( j ) ) == "cinematic_screen_credits_textarea_0_text_0" )
					{
						text = G_UiCredits;
					}

#ifdef PROJECT_SURVIVOR
					// survivor credits hack
					if(cinematic_name == "SGS_credits")
					{
						std::string stats = getLocaleGuiString("credits_stats_text");

						int killsValue = 0;
						for(int c = 0; c < MAX_PLAYERS_PER_CLIENT; c++)
						{
							int temp = 0;
							util::Script::getGlobalArrayVariableValue( "player_total_kills", c, &temp );
							killsValue += temp;
							killsValue += game::GameStats::instances[c]->getTotalKills();
						}

						std::string kills = boost::lexical_cast<std::string>(killsValue);
						std::string secrets = boost::lexical_cast<std::string>(game->gameScripting->getGlobalIntVariableValue("secretpart_amount"));
						stats = util::StringReplace("($kills)", kills, stats);
						stats = util::StringReplace("($secrets)", secrets, stats);
						stats = "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" + stats;
						text = stats + text;
					}
#endif

					// sound file hack
					std::string sound_file_locale = pre_text + "_text_" + boost::lexical_cast< std::string >( j ) + "_audiofile";
					std::string speech_file_locale = pre_text + "_text_" + boost::lexical_cast< std::string >( j ) + "_speechfile";
					const char *sound_file = NULL;
					bool audiofile = ::DHLocaleManager::getInstance()->getString( ::DHLocaleManager::BANK_GUI, sound_file_locale.c_str(), &sound_file );
					bool audiofile_stream = ::DHLocaleManager::getInstance()->getString( ::DHLocaleManager::BANK_GUI, (sound_file_locale + "stream").c_str(), &sound_file );
					bool audiofile_speech = ::DHLocaleManager::getInstance()->getString( ::DHLocaleManager::BANK_GUI, speech_file_locale.c_str(), &sound_file );
					if(audiofile || audiofile_stream || audiofile_speech)
					{
						SoundEffect se;
						se.file = game::convertLocaleSpeechString(sound_file);
						se.handle = -1;
						se.volume = 100;
						se.time = time;
						se.stream = audiofile_stream;
						se.speech = audiofile_speech;
						soundEffects.push_back(se);
					}

					// set text
					if( time <= 0 )
					{
						textAreas[ i ]->setText( text.c_str() );
					}
					else
					{
						textsByTime.insert( std::pair< int, std::pair< int, std::string > >( time, std::pair< int, std::string >( i, text ) ) );
					}
					
				}

				// text queue
				std::string locale = pre_text + "_textqueue";
				if(::DHLocaleManager::getInstance()->hasString(::DHLocaleManager::BANK_GUI, locale.c_str() ))
				{
					std::string str = getLocaleGuiString( locale.c_str() );
					std::vector<std::string> entries = util::StringSplit( "\n", str );
					unsigned int t = textQueue.size();
					unsigned int entry_offset = textQueue.size();
					textQueue.resize(t + entries.size());
					int last_time = -1;
					for(; t < textQueue.size(); t++)
					{
						textQueue[t].textArea = i;

						std::string error_location = "locale " + locale + " line " + boost::lexical_cast<std::string>(t - entry_offset +1);

						std::vector<std::string> params;
						bool inside_quote = false;

						unsigned int start = 0;
						std::string entry = entries[t - entry_offset];
						for(unsigned int c = 0; c < entry.length(); c++)
						{
							// quote
							if(entry[c] == '"')
							{
								inside_quote = !inside_quote;
							}
							// separator
							else if(entry[c] == ',' && !inside_quote)
							{
								params.push_back(entry.substr(start, c - start));
								start = c + 1;
							}

							// end of line
							if(c + 1 == entry.length())
							{
								params.push_back(entry.substr(start, c + 1 - start));
								if(inside_quote)
								{
									Logger::getInstance()->error(("CinematicScreen - Missing terminating quote in " + error_location).c_str());
								}
								break;
							}
						}

						if(params.size() < 2)
						{
							Logger::getInstance()->error(("CinematicScreen - Expecting 2 or more parameters in " + error_location).c_str());
							continue;
						}

						// remove whitespace and quote chars
						for(unsigned int p = 0; p < params.size(); p++)
						{
							params[p] = util::StringRemoveWhitespace(params[p]);
							if(params[p][0] == '"' && params[p][ params[p].length() - 1 ] == '"')
							{
								params[p] = params[p].substr(1, params[p].length() - 2);
							}
						}

						// extra parameters
						for(unsigned int p = 2; p < params.size(); p++)
						{
							std::vector<std::string> define = util::StringSplit(":", params[p]);
							if(define.size() != 2)
							{
								Logger::getInstance()->error(("CinematicScreen - Error parsing parameter " + boost::lexical_cast<std::string>(p+1) + " in " + error_location).c_str());
								continue;
							}

							std::string key = util::StringRemoveWhitespace(define[0]);
							std::string value = util::StringRemoveWhitespace(define[1]);

							if(key == "typingdelay")
							{
								textQueue[t].typingDelay = atoi(value.c_str());
								textQueue[t].typingEffect = true;
							}
							else if(key == "audiofile" || key == "audiofilestream" || key == "speechfile")
							{
								SoundEffect se;

								// convert spaces back to '_'
								for(unsigned int i = 0; i < value.length(); i++)
								  if(value[i] == ' ') value[i] = '_';

								se.file = game::convertLocaleSpeechString(value.c_str());
								se.handle = -1;
								se.volume = 100;
								se.time = atoi(params[0].c_str());
								se.stream = (key == "audiofilestream");
								se.speech = (key == "speechfile");
								soundEffects.push_back(se);
							}
							else
							{
								Logger::getInstance()->error(("CinematicScreen - Unknown parameter " + boost::lexical_cast<std::string>(p+1) + " in " + error_location).c_str());
							}
						}

						textQueue[t].time = atoi(params[0].c_str());
						textQueue[t].text = convertLocaleSubtitleString(params[1].c_str());

						if(textQueue[t].time <= last_time)
						{
							Logger::getInstance()->error(("CinematicScreen - Expecting a time value larger than previous in " + error_location).c_str());
							continue;
						}

						last_time = textQueue[t].time;
					}
				}
			}
		}

		// text faders
		if(::DHLocaleManager::getInstance()->hasString(::DHLocaleManager::BANK_GUI, ( prefix + "_textfader_amount" ).c_str() ))
		{
			int num_of = getLocaleGuiInt( ( prefix + "_textfader_amount" ).c_str(), 0 );
			textFaders.resize(num_of);
			for(unsigned int i = 0; i < textFaders.size(); i++)
			{
				const std::string& pre_fader = ( prefix + "_textfader_" + boost::lexical_cast< std::string >( i ) );
				textFaders[i].time_start = getTime() + getLocaleGuiInt( (pre_fader + "_time_start").c_str() , 0 );
				textFaders[i].time_end = getTime() + getLocaleGuiInt( (pre_fader + "_time_end").c_str() , 0 );
				textFaders[i].fade_duration = getLocaleGuiInt( (pre_fader + "_fade_duration").c_str() , 0 );

				textFaders[i].start_x = getLocaleGuiInt( (pre_fader + "_start_x").c_str() , 0 );
				textFaders[i].start_y = getLocaleGuiInt( (pre_fader + "_start_y").c_str() , 0 );
				textFaders[i].start_w = getLocaleGuiInt( (pre_fader + "_start_w").c_str() , 0 );
				textFaders[i].start_h = getLocaleGuiInt( (pre_fader + "_start_h").c_str() , 0 );
				textFaders[i].start_alpha = 100 - getLocaleGuiInt( (pre_fader + "_start_alpha").c_str() , 0 );

				textFaders[i].end_x = getLocaleGuiInt( (pre_fader + "_end_x").c_str() , 0 );
				textFaders[i].end_y = getLocaleGuiInt( (pre_fader + "_end_y").c_str() , 0 );
				textFaders[i].end_w = getLocaleGuiInt( (pre_fader + "_end_w").c_str() , 0 );
				textFaders[i].end_h = getLocaleGuiInt( (pre_fader + "_end_h").c_str() , 0 );
				textFaders[i].end_alpha = 100 - getLocaleGuiInt( (pre_fader + "_end_alpha").c_str() , 0 );

				std::string img = getLocaleGuiString( (pre_fader + "_image").c_str());
				if(img == "background")
				{
					textFaders[i].cliprect = true;
					textFaders[i].cliprect_x = window->GetPositionX();
					textFaders[i].cliprect_y = window->GetPositionY();
					textFaders[i].cliprect_w = window->GetSizeX();
					textFaders[i].cliprect_h = window->GetSizeY();
					textFaders[i].image.clear();
				}
				else
				{
					textFaders[i].image = img;
				}

				textFaders[i].button = NULL;
			}
		}

		std::sort(textQueue.begin(), textQueue.end(), TextQueueSorter());

		// sound effects
		if(::DHLocaleManager::getInstance()->hasString(::DHLocaleManager::BANK_GUI, ( prefix + "_soundeffect_amount" ).c_str() ))
		{
			int num_of = getLocaleGuiInt( ( prefix + "_soundeffect_amount" ).c_str(), 0 );
			unsigned int offset = soundEffects.size();
			soundEffects.resize(offset + num_of);
			for(unsigned int i = offset; i < soundEffects.size(); i++)
			{
				const std::string& pre_fader = ( prefix + "_soundeffect_" + boost::lexical_cast< std::string >( i - offset ) );
				soundEffects[i].time = getLocaleGuiInt( (pre_fader + "_time").c_str() , 0 );
				soundEffects[i].volume = getLocaleGuiInt( (pre_fader + "_volume").c_str() , 100 );
				soundEffects[i].file = game::convertLocaleSpeechString( getLocaleGuiString( (pre_fader + "_file").c_str()) );
				soundEffects[i].handle = -1;
				soundEffects[i].speech = false;
				const char *stream = NULL;
				if(::DHLocaleManager::getInstance()->getString(::DHLocaleManager::BANK_GUI, (pre_fader + "_stream").c_str(), &stream)
					&& stream[0] == '1')
				{
					soundEffects[i].stream = true;
				}
				else
				{
					soundEffects[i].stream = false;
				}
			}
		}

		std::sort(soundEffects.begin(), soundEffects.end(), SoundEffectSorter());



		initTime = getTime();
	}

	//=========================================================================
	
	~CinematicScreenImpl()
	{
		// assert( false );
		delete backgroundVideo;
		delete backgroundVideoStream;

		delete imageFadeEffect;
		delete backgroundImage;

		while( images.empty() == false )
		{
			delete images.front();
			images.pop_front();
		}

		for( int i = 0; i < (int)textAreas.size(); i++ )
		{
			delete textAreas[ i ];
			textAreas[ i ] = NULL;
		}

		while( fonts.empty() == false )
		{
			delete fonts.front();
			fonts.pop_front();
		}

		for(unsigned int i = 0; i < currentSoundEffect; i++)
		{
			if(game->gameUI)
			{
				if(soundEffects[i].handle != -1)
					game->gameUI->stopSound(soundEffects[i].handle);
				else if(soundEffects[i].stream)
					game->gameUI->stopAllStreamedSounds();
			}
		}

		delete wideScreenBars[0];
		delete wideScreenBars[1];

		delete window;
	}

	//=========================================================================

	void update()
	{
		// window->Raise();
		int time = getTime();

		if( textsByTime.empty() == false && textsByTime.begin()->first + initTime < time )
		{
			textAreas[ textsByTime.begin()->second.first ]->setText( textsByTime.begin()->second.second );

			textsByTime.erase( textsByTime.begin() );
		}

		// text queue
		while(currentTextQueueEntry < textQueue.size())
		{
			QueuedText &qt = textQueue[currentTextQueueEntry];
			if(time - initTime >= qt.time)
			{
				if(qt.typingEffect)
				{
					typingEffects[qt.textArea].text = qt.text;
					typingEffects[qt.textArea].active = true;
					typingEffects[qt.textArea].typingDelay = qt.typingDelay;
					typingEffects[qt.textArea].lastTyped = 0;
					typingEffects[qt.textArea].typingStart = time;
				}
				else
				{
					textAreas[qt.textArea]->setText( qt.text );

					// reset typing effect
					typingEffects[qt.textArea].active = false;
				}
				currentTextQueueEntry++;
			}
			else
			{
				break;
			}
		}

		// typing effects
		for(unsigned int i = 0; i < typingEffects.size(); i++)
		{
			TypingEffect &te = typingEffects[i];
			if(!te.active)
			{
				continue;
			}

			int numChars = (time - te.typingStart) / te.typingDelay;
			if(te.lastTyped >= numChars)
			{
				continue;
			}
			te.lastTyped = numChars;

			// numChars is for visible characters, eg. text outside tags.
			// so let's find out how many chars in reality we need to show
			unsigned int chars = 0;
			bool no_update = false;
			bool inside_tag = false;
			for(chars = 0; chars < te.text.length(); chars++)
			{
				char c = te.text[chars];
				if(c == '<')
				{
					inside_tag = true;
					std::string str = te.text.substr(chars);
					int delay = 0;
					if(sscanf(str.c_str(), "<wait:%i>", &delay) == 1)
					{
						int maxChars = (time - te.typingStart) / te.typingDelay;
						int wait_start = (maxChars - numChars) * te.typingDelay + te.typingStart;
						int wait_end = wait_start + delay;
						if(time < wait_end)
						{
							no_update = true;
							break;
						}
						else
						{
							numChars -= delay / te.typingDelay;
						}
					}
				}
				else if(inside_tag && c == '>')
				{
					inside_tag = false;
				}
				// this is text outside tag
				else if(!inside_tag)
				{
					numChars--;
					if(numChars <= 0)
					{
						break;
					}
				}
			}

			if(no_update)
			{
				continue;
			}
			std::string cut_text = te.text.substr(0, chars+1);
			textAreas[i]->setText(cut_text);
			if(chars == te.text.length())
			{
				te.active = false;
			}

			if(!te.typingSound.empty())
			{
				game->gameUI->playGUISound(te.typingSound.c_str());
			}
		}

		while(currentSoundEffect < soundEffects.size())
		{
			SoundEffect &se = soundEffects[currentSoundEffect];
			if(time - initTime >= se.time)
			{
				VC3 pos = game->gameUI->getListenerPosition();
				if(se.stream) game->gameUI->playStreamedSound(se.file.c_str());
				else if(se.speech) se.handle = game->gameUI->playSpeech(se.file.c_str(), pos.x, pos.y, pos.z, false, 100, false);
				else se.handle = game->gameUI->playGUISound(se.file.c_str(), se.volume);
				currentSoundEffect++;
			}
			else
			{
				break;
			}
		}

		if( effectsByTime.empty() == false && effectsByTime.begin()->first + initTime < time )
		{
			if( effectsByTime.begin()->second.first != -1 )
			{
				window->StartEffect( effectsByTime.begin()->second.first, effectsByTime.begin()->second.second );
			}
			else
			{
				StartScrollEffect( effectsByTime.begin()->second.second );
			}
			

			effectsByTime.erase( effectsByTime.begin() );
		}

		if( pictureFadeByTime.empty() == false && pictureFadeByTime.begin()->first + initTime < time )
		{
			if( imageFadeEffect ) 
				delete imageFadeEffect;

			imageFadeEffect = new ImageFadeEffect;
			imageFadeEffect->initTime = time;
			imageFadeEffect->transparency = 1.0f;
			imageFadeEffect->fadeLength = pictureFadeByTime.begin()->second.second;
			

			IOguiImage* image = ogui->LoadOguiImage( pictureFadeByTime.begin()->second.first.c_str() );
			images.push_back( image );
		
			backgroundImage->SetTransparency( 100 );
			backgroundImage->SetDisabledImage( image );
			imageFadeEffect->image = image;

			pictureFadeByTime.erase( pictureFadeByTime.begin() );
		}

		if( (timeToDie == -1 && backgroundVideoStream && !backgroundVideoStream->isPlaying()) || (timeToDie > 0 && timeToDie + initTime < time) )
		{
			close();
		}

		if( imageFadeEffect )
		{
			float transparency = 1.0f - ( (float)( time - imageFadeEffect->initTime )  / imageFadeEffect->fadeLength );
			imageFadeEffect->transparency = transparency;
			if( transparency <= 0 )
			{
				window->setBackgroundImage( imageFadeEffect->image );
				backgroundImage->SetDisabledImage( NULL );
				delete imageFadeEffect;
				imageFadeEffect = NULL;

			}
			else
			{
				backgroundImage->SetTransparency( (int)( transparency * 100.0f ) );
			}
		}

		if( scrollEffect )
		{
				
			scrollEffect->yPosition -= ( getTime() - scrollEffect->lastUpdate ) * scrollEffect->speed;
			scrollEffect->lastUpdate = getTime();
	
			int moveBy = (int)scrollEffect->yPosition - (int)scrollEffect->lastYPosition;
			scrollEffect->lastYPosition = (int)scrollEffect->yPosition;

			// theText->moveBy( 0, (int)scrollEffect->yPosition - (int)scrollEffect->lastYPosition );
			
			unsigned int i;

			for( i = 0; i < textAreas.size(); i++ )
			{
				textAreas[ i ]->moveBy( 0, moveBy );
			}
		}


		for(unsigned int i = 0; i < textFaders.size(); i++)
		{
			TextFader &tf = textFaders[i];
			if(time < tf.time_start)
			{
				continue;
			}
			if(time > tf.time_end)
			{
				delete tf.button;
				tf.button = NULL;
				tf.time_start = INT_MAX;
				continue;
			}

			float lerp_factor = (time - tf.time_start) / (float)tf.fade_duration;
			if(lerp_factor > 1.0f)
			{
				lerp_factor = 1.0f;
			}

			int x,y,w,h,alpha;
			x = (int)(tf.end_x * lerp_factor + tf.start_x * (1.0f - lerp_factor));
			y = (int)(tf.end_y * lerp_factor + tf.start_y * (1.0f - lerp_factor));
			w = (int)(tf.end_w * lerp_factor + tf.start_w * (1.0f - lerp_factor));
			h = (int)(tf.end_h * lerp_factor + tf.start_h * (1.0f - lerp_factor));
			alpha = (int)(tf.end_alpha * lerp_factor + tf.start_alpha * (1.0f - lerp_factor));

			float clip_x, clip_y, clip_w, clip_h;
			clip_x = 0.0f;
			clip_y = 0.0f;
			clip_w = 100.0f;
			clip_h = 100.0f;

			if(tf.cliprect)
			{
				clip_x = 100.0f * (x - tf.cliprect_x) / (float)tf.cliprect_w;
				clip_y = 100.0f * (y - tf.cliprect_y) / (float)tf.cliprect_h;
				clip_w = 100.0f * (x + w - tf.cliprect_x) / (float)tf.cliprect_w;
				clip_h = 100.0f * (y + h - tf.cliprect_y) / (float)tf.cliprect_h;
				x = tf.cliprect_x;
				y = tf.cliprect_y;
				w = tf.cliprect_w;
				h = tf.cliprect_h;
			}

			if(tf.button == NULL)
			{
				const char *img = NULL;
				if(!tf.image.empty())
				{
					img = tf.image.c_str();
				}
				tf.button = ogui->CreateSimpleImageButton(window, x, y, w, h, NULL, NULL, NULL, img, 0, NULL, false);
				tf.button->SetDisabled(true);
				if(tf.image.empty())
				{
					tf.button->SetDisabledImage(window->getBackgroundImage());
				}
			}

			tf.button->Move(x, y);
			tf.button->Resize(w, h);
			tf.button->SetClip(clip_x, clip_y, clip_w, clip_h);
			tf.button->SetTransparency(alpha);
		}

	}

	//=========================================================================

	int getTime() const
	{
		
		// this is a bad idea because the timer stops when video stops...
		/*if( backgroundVideoStream )
		{
			int time = backgroundVideoStream->getTime();
			
			// Logger::getInstance()->error( boost::lexical_cast< std::string >( time ).c_str() );
			// assert( time > 0 );
			return time;
		}*/
		
		return Timer::getTime();
	}

	//=========================================================================

	void StartScrollEffect( int duration )
	{
		scrollEffect = new ScrollEffect;
		int bottom = 0;
		unsigned int i;

		for( i = 0; i < textAreas.size(); i++ )
		{
			bottom = std::max( bottom, textAreas[ i ]->getY() + textAreas[ i ]->getH() );
		}

		scrollEffect->speed = (float)bottom / (float)duration;
		scrollEffect->lastUpdate = getTime();

	}

	//=========================================================================

	bool shouldBeDeleted()
	{
		return killMe != 0 && game->gameTimer >= killMe;
	}

	//=========================================================================

	void close()
	{
		// kill after 1 tick has passed
		if(killMe == 0)
		{
#ifdef PROJECT_SURVIVOR
			if(cinematic_name == "SGS_intro")
			{
				game->loadGame("new");
			}
#endif
			killMe = game->gameTimer + 1;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////

CinematicScreen::CinematicScreen( Ogui* ogui, game::Game *game, const std::string& name, sfx::SoundMixer *mixer, IStorm3D* storm3d )
{
	impl = new CinematicScreenImpl( ogui, game, name, mixer, storm3d );
}

//=============================================================================

CinematicScreen::~CinematicScreen()
{
	delete impl;
}

///////////////////////////////////////////////////////////////////////////////

void CinematicScreen::update()
{
	impl->update();
}

//=============================================================================

bool CinematicScreen::isOpen() const
{
	return impl->killMe == 0;
}

//=============================================================================

bool CinematicScreen::shouldBeDeleted() const
{
	return impl->shouldBeDeleted();
}

//=============================================================================

void CinematicScreen::close()
{
	impl->close();
}

//=============================================================================

bool CinematicScreen::hasVideo() const
{
	return ( impl->backgroundVideo != NULL );
}

//=============================================================================

void CinematicScreen::raise()
{
	impl->window->Raise();
}
///////////////////////////////////////////////////////////////////////////////
} // end of namespace ui
