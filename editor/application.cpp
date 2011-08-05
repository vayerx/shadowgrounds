// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "application.h"
#include "gui.h"
#include "dialog.h"
#include "dialog_utils.h"
#include "window.h"
#include "storm.h"
#include "icommand.h"
#include "command_list.h"
#include "camera.h"
#include "mouse.h"
#include "editor_state.h"
#include "resource/resource.h"
#include "export_options.h"
#include "export_dialog.h"
#include "file_wrapper.h"
#include "file_iterator.h"
#include "UniqueEditorObjectHandleManager.h"
#include "editor_object_state.h"

#include "common_dialog.h"
#include "../filesystem/input_file_stream.h"
#include "../filesystem/input_compressed_file_stream.h"
#include "../filesystem/output_file_stream.h"
#include "../filesystem/output_compressed_file_stream.h"
#include "../filesystem/memory_stream.h"
#include "../filesystem/ifile_package.h"
#include "../filesystem/standard_package.h"
#include "../filesystem/zip_package.h"
#include "../filesystem/file_package_manager.h"
#include "../util/hiddencommand.h"

#include <vector>
#include <windows.h>
#include <istorm3d.h>
#include <istorm3d_terrain_renderer.h>
#include <istorm3d_terrain_decalsystem.h>
#include <istorm3d_spotlight.h>
#include <istorm3d_videostreamer.h>
#include <stdio.h>
#include <fstream>
#include <boost/scoped_array.hpp>
#include <boost/lexical_cast.hpp>
#pragma comment(lib, "storm3dv2.lib")

#ifdef PROJECT_AOV
#define EDITOR_GZIP_SAVES 1
#else
#define EDITOR_COMPRESSED_SAVES 1
#endif


//extern std::vector<VC2> fakeMin;
//extern std::vector<VC2> fakeSize;

namespace frozenbyte {
namespace editor {

	std::string mission_id_global;

namespace {
	
	IStorm3D_Font *font = 0;

	struct NullDeleter
	{
		void operator () (void *)
		{
		}
	};

	void createFont(IStorm3D &storm)
	{
		/*
		IStorm3D_Texture *texture = storm.CreateNewTexture("Data/Fonts/font2.dds");
		font = 0; 

		if(texture)
		{
			std::string buff;
			buff += '\t';
			buff += '!';
			buff += '"';
			buff += "#$%@'()*+.-,/0123456789:;<=>?\nabcdefghijklmnopqrstuvwxyz[\\]\t_";

			std::vector<unsigned char> widths;
			for(unsigned int i = 0; i < buff.size(); ++i)
				widths.push_back(64);

			font = storm.CreateNewFont();
			font->AddTexture(texture);
			font->SetTextureRowsAndColums(8, 8);
			font->SetCharacters(buff.c_str(), &widths[0]);
			font->SetColor(COL(1.0f, 1.0f, 1.0f));	
		}
		*/

		font = storm.CreateNewFont();
		font->SetFont("Times New Roman", 0, 20, false, false);
		font->SetColor(COL(1.0f, 1.0f, 1.0f));	
	}

	void freeFont()
	{
		delete font;
		font = 0;
	}

	void setFileTitle(const Window &window, const std::string &file)
	{
		std::string title;
		if(!file.empty())
		{
			title = file;
			title += " - ";
		}

#ifdef PROJECT_AOV
		title += "AOV Editor";
#else
		title += "Frozenbyte Editor";
#endif
		SetWindowText(window.getWindowHandle(), title.c_str());
	}

	struct SharedData
	{
		EditorState &editorState;
		Storm &storm;
		std::string fileName;

		SharedData(EditorState &editorState_, Storm &storm_)
		:	editorState(editorState_),
			storm(storm_)
		{
		}

		void save()
		{
			if(fileName.empty())
				return;

#ifdef EDITOR_GZIP_SAVES
			// if the file contains .gz already, don't be adding it...
			bool isGzipped = false;
			int gzipSplit = 0;
			for (int i = 0; i < (int)fileName.length(); i++)
			{
				if (strncmp(&(fileName.c_str()[i]), ".gz.", 4) == 0)
				{
					isGzipped = true;
					gzipSplit = i;
					break;
				}
			}
			std::string gunzippedFileName;
			if (isGzipped)
			{
				gunzippedFileName = fileName.substr(0, gzipSplit);
				gunzippedFileName += fileName.substr(gzipSplit + 3, fileName.length() - (gzipSplit + 3));

				fileName = gunzippedFileName;
			}
#endif

			{
#ifdef EDITOR_COMPRESSED_SAVES
				filesystem::OutputStream stream = filesystem::createOutputCompressedFileStream(fileName);
#else
				filesystem::OutputStream stream = filesystem::createOutputFileStream(fileName);
#endif
				stream << editorState;
			}

#ifdef EDITOR_GZIP_SAVES
			// gzip the file...
			std::string gzipcmd = "tools\\gzip_file.bat " + fileName;
			hiddencommand(gzipcmd.c_str());

			// TODO: check that the gzipped file really exists and has been updated before deleting anything!
			remove(fileName.c_str());
#endif

			UniqueEditorObjectHandleManager::uninit();
#ifdef LEGACY_FILES
		// (should be ifdef project_survivor, but survivor's editor does not have that defined)
#else
			if (UniqueEditorObjectHandleManager::wasError())
			{
				MessageBox(0, "Failed to save unique handle iterator value.\r\n\r\nThis may cause non-unique object handles in the future.\r\n(Changes made to maps in game may get read back to editor incorrectly.)", "Warning", MB_ICONEXCLAMATION|MB_OK);
			}
#endif

			UniqueEditorObjectHandleManager::init();
#ifdef LEGACY_FILES
		// (should be ifdef project_survivor, but survivor's editor does not have that defined)
#else
			if (UniqueEditorObjectHandleManager::wasError())
			{
				MessageBox(0, "Failed initialize unique handles.\r\n\r\nThis may cause non-unique object handles.\r\n(Changes made to maps in game may get read back to editor incorrectly.)", "Warning", MB_ICONEXCLAMATION|MB_OK);
			}
			else if (UniqueEditorObjectHandleManager::wasLocked())
			{
				MessageBox(0, "Editor unique handle files were locked.\r\n\r\nAnother editor instance already running or editor did not exit cleanly?\r\nThis may cause non-unique object handles.\r\n(Changes made to maps in game may get read back to editor incorrectly.)", "Warning", MB_ICONEXCLAMATION|MB_OK);
			}
			else if (UniqueEditorObjectHandleManager::wasFirstTimeInit())
			{
				MessageBox(0, "Editor unique handle files were supposedly first-time-initialized during re-init.\r\n\r\n(This should not happen?)", "Warning", MB_OK);
			}
#endif
		}

		void load()
		{
			if(fileName.empty())
				return;

#ifdef EDITOR_GZIP_SAVES
			// gunzip the file if it contains .gz
			bool isGzipped = false;
			int gzipSplit = 0;
			for (int i = 0; i < (int)fileName.length(); i++)
			{
				if (strncmp(&(fileName.c_str()[i]), ".gz.", 4) == 0)
				{
					isGzipped = true;
					gzipSplit = i;
					break;
				}
			}
			std::string gunzippedFileName;
			if (isGzipped)
			{
				gunzippedFileName = fileName.substr(0, gzipSplit);
				gunzippedFileName += fileName.substr(gzipSplit + 3, fileName.length() - (gzipSplit + 3));

				std::string gunzipcmd = "tools\\gunzip_file.bat " + gunzippedFileName;
				hiddencommand(gunzipcmd.c_str());

				fileName = gunzippedFileName;
			}
#endif

			freeFont();

			{

				bool isCompressed = false;
				unsigned int separator = fileName.find_last_of(".");
				if(separator != fileName.npos && separator < fileName.size())
				{
					const char *end = fileName.c_str() + separator;
					if(_stricmp(end, ".fbez") == 0)
						isCompressed = true;
				}

				if(isCompressed)
				{
					filesystem::InputStream stream = filesystem::createInputCompressedFileStream(fileName);
					stream >> editorState;
				}
				else
				{
					filesystem::FilePackageManager &manager = filesystem::FilePackageManager::getInstance();
					filesystem::InputStream stream = manager.getFile(fileName);
					stream >> editorState;
				}
			}
#ifdef EDITOR_GZIP_SAVES
			if (!gunzippedFileName.empty())
			{
				remove(gunzippedFileName.c_str());
			}
#endif
			editorState.update();
			createFont(*storm.storm);

		}
	};

	class UpdateCommand: public ICommand
	{
		SharedData &sharedData;

	public:
		UpdateCommand(Dialog &menu, SharedData &sharedData_)
		:	sharedData(sharedData_)
		{
			menu.getCommandList().addCommand(IDC_MENU_UPDATE, this);
		}

		void execute(int id)
		{
			freeFont();
			sharedData.editorState.update();
			createFont(*sharedData.storm.storm);
		}
	};

	class NewCommand: public ICommand
	{
		SharedData &sharedData;
		Gui &gui;

	public:
		NewCommand(Window &window, SharedData &sharedData_, Gui &gui_)
		:	sharedData(sharedData_),
			gui(gui_)
		{
			window.getCommandList().addCommand(ID_FILE__NEW, this);
		}

		void execute(int id)
		{
			freeFont();

			gui.reset();
			sharedData.editorState.reset();
			sharedData.fileName = "";

			createFont(*sharedData.storm.storm);
			setFileTitle(gui.getMainWindow(), "");
		}
	};

	class OpenCommand: public ICommand
	{
		SharedData &sharedData;
		Gui &gui;

	public:
		OpenCommand(SharedData &sharedData_, Gui &gui_)
		:	sharedData(sharedData_),
			gui(gui_)
		{
		}

		void execute(int id)
		{
			std::string filterString;
#ifdef EDITOR_COMPRESSED_SAVES
			filterString += "fbez (compressed) files";
			filterString += char(0);
			filterString += "*.fbez";
			filterString += char(0);
#endif
			filterString += "fbe files";
			filterString += char(0);
			filterString += "*.fbe";
			filterString += char(0);
			filterString += char(0);

			std::string fileName = getOpenFileName(filterString, "Editor\\Save", false);
			if(fileName.empty())
				return;

			freeFont();

			gui.reset();
			sharedData.editorState.reset();

			{
				mission_id_global;

				int start = fileName.find_last_of("/\\");
				int end = fileName.size() - 4;
				if(start != fileName.npos && start + 1 < end)
					mission_id_global = fileName.substr(start + 1, end - start - 1);
			}

			sharedData.fileName = fileName;
			sharedData.load();
			createFont(*sharedData.storm.storm);

			// Set scene mode as default
			CheckRadioButton(gui.getMenuDialog().getWindowHandle(), IDC_MENU_TERRAIN, IDC_MENU_LIGHTS, IDC_MENU_SCENE);
			SendMessage(gui.getMenuDialog().getWindowHandle(), WM_COMMAND, IDC_MENU_SCENE, IDC_MENU_SCENE);

			// Disable light visualizations as default
			CheckDlgButton(gui.getBuildingsDialog().getWindowHandle(), IDC_LIGHT_VISUALIZATION, BST_UNCHECKED);
			SendMessage(gui.getMenuDialog().getWindowHandle(), WM_COMMAND, IDC_LIGHT_VISUALIZATION, IDC_LIGHT_VISUALIZATION);

			// Remove roofs
			CheckDlgButton(gui.getBuildingsDialog().getWindowHandle(), IDC_HIDE_ROOF, BST_CHECKED);
			SendMessage(gui.getBuildingsDialog().getWindowHandle(), WM_COMMAND, IDC_HIDE_ROOF, IDC_HIDE_ROOF);

			setFileTitle(gui.getMainWindow(), fileName);
			sharedData.editorState.updateLighting();
		}
	};

	class SaveCommand: public ICommand
	{
		SharedData &sharedData;
		Gui &gui;
		bool forceDialog;

	public:
		SaveCommand(SharedData &sharedData_, Gui &gui_, bool forceDialog_)
		:	sharedData(sharedData_),
			gui(gui_),
			forceDialog(forceDialog_)
		{
		}

		void execute(int id)
		{
			if(forceDialog || sharedData.fileName.empty())
			{
#ifdef EDITOR_COMPRESSED_SAVES
				std::string filterString = "fbez (compressed) files";
				filterString += char(0);
				filterString += "*.fbez";
				filterString += char(0);
#else
				std::string filterString = "fbe files";
				filterString += char(0);
				filterString += "*.fbe";
				filterString += char(0);
#endif
				filterString += "*.* files";
				filterString += char(0);
				filterString += "*.*";
				filterString += char(0);
				filterString += char(0);

				std::string fileName = getSaveFileName(filterString, "Editor\\Save", false);
				if(!fileName.empty())
					sharedData.fileName = fileName;
			}

			sharedData.save();
			setFileTitle(gui.getMainWindow(), sharedData.fileName);
		}
	};

	class ReloadCommand: public ICommand
	{
		Storm &storm;
		SharedData &sharedData;
		Gui &gui;

		HWND renderWindow;

	public:
		ReloadCommand(Storm &storm_, SharedData &sharedData_, Gui &gui_)
		:	storm(storm_),
			sharedData(sharedData_),
			gui(gui_)
		{
			renderWindow = 0;
			gui.getMenuDialog().getCommandList().addCommand(IDC_MENU_RELOAD, this);
		}

		void setWindow(HWND renderWindow_)
		{
			renderWindow = renderWindow_;
		}

		void execute(int id)
		{
			freeFont();

			boost::shared_ptr<filesystem::MemoryStreamBuffer> streamBuffer(new filesystem::MemoryStreamBuffer());
			filesystem::OutputStream outputStream;
			outputStream.setBuffer(streamBuffer);

			filesystem::InputStream inputStream;
			inputStream.setBuffer(streamBuffer);

			outputStream << sharedData.editorState;
			sharedData.editorState.reset();

			storm.recreate(renderWindow);
			inputStream >> sharedData.editorState;

			sharedData.editorState.update();
			createFont(*sharedData.storm.storm);

			// Remove roofs
			CheckDlgButton(gui.getBuildingsDialog().getWindowHandle(), IDC_HIDE_ROOF, BST_CHECKED);
			SendMessage(gui.getBuildingsDialog().getWindowHandle(), WM_COMMAND, IDC_HIDE_ROOF, IDC_HIDE_ROOF);

			sharedData.editorState.updateLighting();
			sharedData.editorState.updateGrid(true);
			sharedData.editorState.updateFolds(true);
			sharedData.editorState.updateTriggersVisibility(true);
			sharedData.editorState.updateUnitsVisibility(true);

			sharedData.editorState.updateHelpersVisibility(true);
			sharedData.editorState.updateStaticVisibility(true);
			sharedData.editorState.updateDynamicVisibility(true);
			sharedData.editorState.updateIncompleteVisibility(true);
			sharedData.editorState.updateNoCollisionVisibility(true);
		}
	};

	class ResetCommand: public ICommand
	{
		Storm &storm;
		HWND renderWindow;

	public:
		ResetCommand(Storm &storm_, Gui &gui)
		:	storm(storm_)
		{
			gui.getMenuDialog().getCommandList().addCommand(IDC_MENU_RESET, this);
		}

		void execute(int id)
		{
			if(storm.storm)
				storm.storm->forceReset();
		}
	};

	class QuitCommand: public ICommand
	{
		Window &window;
	
	public:
		QuitCommand(Window &window_)
		:	window(window_)
		{
			window.getCommandList().addCommand(ID_FILE__QUIT, this);
		}

		void execute(int id)
		{
			PostQuitMessage(0);
		}
	};

	class ExportCommand: public ICommand
	{
		SharedData &sharedData;
		Gui &gui;

	public:
		ExportCommand(SharedData &sharedData_, Gui &gui_)
		:	sharedData(sharedData_),
			gui(gui_)
		{
		}

		void execute(int id)
		{
			sharedData.editorState.updateLighting();

			ExportDialog dialog;
			ExportOptions options = dialog.show();

			if(!options.fileName.empty())
			{
				sharedData.editorState.update();
				sharedData.editorState.exportData(options);

				// Remove roofs
				CheckDlgButton(gui.getBuildingsDialog().getWindowHandle(), IDC_HIDE_ROOF, BST_CHECKED);
				SendMessage(gui.getBuildingsDialog().getWindowHandle(), WM_COMMAND, IDC_HIDE_ROOF, IDC_HIDE_ROOF);
			}
		}
	};

	class MultiExportCommand: public ICommand
	{
		SharedData &sharedData;
		std::string log;
		std::vector< std::pair< std::string, std::string > > files;
		Dialog *dialog;
		Gui &gui;

		inline void StringRemoveWhitespace( std::string& result )
		{
			size_t position = result.find_first_not_of(" \t\n");
			result.erase( 0,  position );

			position = result.find_last_not_of(" \t\n");
			result.erase( position+1 );
		}

		void doExport(const std::string fileName, const std::string &fileName_to, int i, int max)
		{
			log += "  Exported " + fileName + " to " + fileName_to + "\r\n";

			setFileTitle(gui.getMainWindow(), "Multi-export (" + boost::lexical_cast<std::string>(i)
				+ "/" + boost::lexical_cast<std::string>(max)
				+ "): " + fileName);
			sharedData.editorState.updateLighting();

			//////////////////////////////
			// load

			freeFont();
			gui.reset();
			sharedData.editorState.reset();

			{
				mission_id_global;

				int start = fileName.find_last_of("/\\");
				int end = fileName.size() - 4;
				if(start != fileName.npos && start + 1 < end)
					mission_id_global = fileName.substr(start + 1, end - start - 1);
			}

			sharedData.fileName = fileName;
			sharedData.load();
			createFont(*sharedData.storm.storm);

			//////////////////////////////
			// recalculate lightmap
			int quality = getComboIndex(*dialog, IDC_MULTIEXPORT_LIGHTMAP) - 1;
			if(quality != -2)
			{
				if(quality == -1)
					quality = 4;
				int area = 0;

				sharedData.editorState.roofCollision(true);
				EditorObjectState editorObjects;
				sharedData.editorState.getEditorObjectStates(editorObjects);

				TerrainLightMap &map = sharedData.editorState.getLightMap();
				std::vector<TerrainLightMap::PointLight> lights;
				sharedData.editorState.getLights(lights);

				map.create(lights, area, quality, sharedData.editorState.getSunDirection());
				sharedData.editorState.updateLighting();

				sharedData.editorState.setEditorObjectStates(editorObjects);
				sharedData.editorState.roofCollision(false);
				sharedData.editorState.visualizeCompletion();

				sharedData.save();
			}

			//////////////////////////////
			// export

			ExportOptions options;
			options.fileName = fileName_to;
			options.onlyScripts = false;

			options.id = makeMissionIdFromFileName(options.fileName);

			if(!options.fileName.empty())
			{
				sharedData.editorState.update();
				sharedData.editorState.exportData(options);
			}
		}

	public:
		MultiExportCommand(SharedData &sharedData_, Gui &gui_)
		:	sharedData(sharedData_),
			gui(gui_),
			dialog(NULL)
		{
		}

		~MultiExportCommand()
		{
			delete dialog;
		}

		void execute(int id)
		{
			switch(id)
			{
			case ID_FILE_MULTIEXPORT:
				menuCommand();
				break;
			case WM_INITDIALOG:
				initDialog();
				break;
			case IDC_MULTIEXPORT_OK:
				multiExport();
				break;
			}
		}

		void menuCommand()
		{
			std::string filterString = "text files";
			filterString += char(0);
			filterString += "*.txt";
			filterString += char(0);
			filterString += char(0);

			std::string fileName = getOpenFileName(filterString, "Editor\\Dev\\Exports", false);
			if(fileName.empty())
				return;

			FILE *file = fopen(fileName.c_str(), "rt");
			if(file == NULL)
			{
				MessageBox(0, ("Cannot open file " + fileName).c_str(), "Multi-export error!", MB_ICONEXCLAMATION|MB_OK);
				return;
			}
			log = "Multi-export queue:\r\n";

			// parse
			//
			files.clear();
			unsigned int line_max_length = 1024;
			char line[1024];
			unsigned int line_number = 0;
			while(!feof(file))
			{
				if(fgets(line, line_max_length, file) == NULL) break;
				unsigned int line_length = strlen(line);
				line_number++;

				// find '>'
				unsigned int i;
				unsigned int textStart = line_max_length - 2; // must be safe to check 1 char forward
				bool line_is_empty = true;
				for(i = 0; i < line_length; i++)
				{
					// has non-whitespace char
					if(line[i] != ' '
						&& line[i] != '\t'
						&& line[i] != '\n')
					{
						line_is_empty = false;
						if(i < textStart)
							textStart = i;
					}

					if(line[i] == '>')
					{
						line[i] = 0;
						break;
					}
				}

				// comment or empty
				if(line_is_empty ||
					 line[textStart] == ';' ||
					 (line[textStart] == '/' && line[textStart + 1] == '/'))
				{
					continue;
				}

				log += "  ";

				// did not find '>'
				if(i >= line_length)
				{
					log += "Parsing line " + boost::lexical_cast<std::string>(line_number) + " failed!\r\n";
					continue;
				}

				std::string filename_from(line);
				std::string filename_to(line + i + 1);
				StringRemoveWhitespace(filename_from);
				StringRemoveWhitespace(filename_to);

				std::string missionId = makeMissionIdFromFileName(filename_to);
				std::string missionFile = filename_to + "/" + missionId + ".dhm";

				if(!fileExists(missionFile))
				{
					log += "Mission file " + missionFile + " not found!\r\n";
					continue;
				}

				// look for pattern
				{
					if(filename_from.find_first_of('*') != std::string::npos ||
						 filename_from.find_first_of('?') != std::string::npos)
					{
						size_t folder_position = filename_from.find_last_of('/');
						std::string folder = filename_from.substr(0, folder_position + 1);

						::frozenbyte::editor::FileIterator iterator(filename_from, false);
						std::string top_file = "";
						std::string fileName = iterator.getFileName();
						while(!fileName.empty())
						{							
							if(top_file.empty() || top_file < fileName)
							{
								top_file = fileName;
							}

							fileName = iterator.getFileName();
							iterator.next();
						}

						if(top_file.empty())
						{
							log += "No files found with pattern " + filename_from + "\r\n";
							continue;
						}

						filename_from = folder + top_file;
					}
				}

				if(!fileExists(filename_from))
				{
					log += "Save file " + filename_from + " not found!\r\n";
					continue;
				}

				log += "Export " + filename_from + " to " + filename_to + "\r\n";
				files.push_back( std::pair<std::string, std::string>( filename_from, filename_to ) );
			}

			fclose(file);

			dialog = new Dialog(IDD_MULTIEXPORT);
			dialog->getCommandList(WM_INITDIALOG).addCommand(WM_INITDIALOG, this);
			dialog->getCommandList().addCommand(IDC_MULTIEXPORT_OK, this);
			dialog->show();
		}

		void initDialog()
		{
			SetWindowText( GetDlgItem(dialog->getWindowHandle(), IDC_MULTIEXPORT_LOG), log.c_str());
			SendDlgItemMessage(dialog->getWindowHandle(), IDC_MULTIEXPORT_LIGHTMAP, CB_RESETCONTENT, 0, 0);
			addComboString(*dialog, IDC_MULTIEXPORT_LIGHTMAP, "Ultra quality");
			addComboString(*dialog, IDC_MULTIEXPORT_LIGHTMAP, "High quality");
			addComboString(*dialog, IDC_MULTIEXPORT_LIGHTMAP, "Low quality");
			addComboString(*dialog, IDC_MULTIEXPORT_LIGHTMAP, "Very low quality");
			addComboString(*dialog, IDC_MULTIEXPORT_LIGHTMAP, "No shadows");
			//setComboIndex(dialog, IDC_LIGHTMAP_QUALITY, 1);
		}

		void multiExport()
		{
			dialog->hide();

			log = "Multi-export done:\r\n";
			for(unsigned int i = 0; i < files.size(); i++)
			{
				doExport(files[i].first, files[i].second, i+1, files.size());
			}

			freeFont();
			gui.reset();
			sharedData.editorState.reset();
			sharedData.fileName = "";
			createFont(*sharedData.storm.storm);
			setFileTitle(gui.getMainWindow(), "");

			MessageBox(0, log.c_str(), "Multi-export", MB_ICONINFORMATION|MB_OK);
		}
	};
} // end of unnamed namespace

extern bool editorMode;
struct ApplicationData
{
	Gui gui;
	Storm storm;
	Camera camera;

	EditorState editorState;
	SharedData sharedData;

	bool mustQuit;

	UpdateCommand updateCommand; 
	NewCommand newCommand;
	OpenCommand openCommand;
	SaveCommand saveAsCommand;
	SaveCommand saveCommand;
	QuitCommand quitCommand;
	ReloadCommand reloadCommand;
	ResetCommand resetCommand;

	ExportCommand exportCommand;
	MultiExportCommand multiExportCommand;

	ApplicationData()
	:	storm(gui.getRenderDialog().getWindowHandle()),
		camera(storm),
		editorState(gui, storm, camera),
		sharedData(editorState, storm),

		updateCommand(gui.getMenuDialog(), sharedData),
		newCommand(gui.getMainWindow(), sharedData, gui),
		openCommand(sharedData, gui),
		saveAsCommand(sharedData, gui, true),
		saveCommand(sharedData, gui, false),
		quitCommand(gui.getMainWindow()),
		reloadCommand(storm, sharedData, gui),
		resetCommand(storm, gui),

		multiExportCommand(sharedData, gui),
		exportCommand(sharedData, gui)
	{
		mustQuit = false;

		gui.getMainWindow().getCommandList().addCommand(ID_FILE__OPEN, &openCommand);
		gui.getMainWindow().getCommandList().addCommand(ID_FILE__SAVEAS, &saveAsCommand);
		gui.getMainWindow().getCommandList().addCommand(ID_FILE_EXPORTTOGAME, &exportCommand);
		gui.getMainWindow().getCommandList().addCommand(ID_FILE_MULTIEXPORT, &multiExportCommand);
		gui.getMenuDialog().getCommandList().addCommand(IDC_SAVE, &saveCommand);

		reloadCommand.setWindow(gui.getRenderDialog().getWindowHandle());
		gui.getMouse().setStorm(storm);

		/*
		filesystem::FilePackageManager &manager = filesystem::FilePackageManager::getInstance();
		boost::shared_ptr<filesystem::StandardPackage> standardPackage(new filesystem::StandardPackage());
		manager.addPackage(standardPackage, -1);
		boost::shared_ptr<filesystem::IFilePackage> standardPackage( new filesystem::StandardPackage() );
		boost::shared_ptr<filesystem::IFilePackage> zipPackage1( new filesystem::ZipPackage( "data1.fbz" ) );
		boost::shared_ptr<filesystem::IFilePackage> zipPackage2( new filesystem::ZipPackage( "data2.fbz" ) );
		boost::shared_ptr<filesystem::IFilePackage> zipPackage3( new filesystem::ZipPackage( "data3.fbz" ) );
		boost::shared_ptr<filesystem::IFilePackage> zipPackage4( new filesystem::ZipPackage( "data4.fbz" ) );

		manager.addPackage( standardPackage, 999 );
		manager.addPackage( zipPackage1, 1 );
		manager.addPackage( zipPackage2, 2 );
		manager.addPackage( zipPackage3, 3 );
		manager.addPackage( zipPackage4, 4 );
		*/

		camera.setToSky();
	}

	~ApplicationData()
	{
	}

	bool ignoreMessage(MSG message)
	{
		bool focused_on_text = false;
		HWND focusWindow = GetFocus();
		if(IsWindow(focusWindow))
		{
			char winname[32] = {0};
			GetClassName(focusWindow, winname, 32);
			if(strcmp(winname, "Edit") == 0)
				focused_on_text = true;
		}
		// Ignore keys that are used as controls
		if(!focused_on_text && (message.message == WM_KEYDOWN) || (message.message == WM_KEYUP))
		{
			switch(message.wParam)
			{
				case 'W':
				case 'A':
				case 'S':
				case 'D':
				case 'Q':
				case 'E':
				case 'R':
				case 'H':
				case 'C':
				case 'X':
				case 'V':
				case 'N':
				case 'Z':
				case 'T':
				case 'G':
				case 'P':
				case VK_ADD:
				case VK_SUBTRACT:
				case VK_PRIOR:
				case VK_NEXT:
				case VK_LEFT:
				case VK_RIGHT:
				case VK_UP:
				case VK_DOWN:
				case VK_MENU:
					return true;
			}
		}

		return false;
	}

	void handleMessage()
	{
		MSG windowsMessage = { 0 };
		if(GetMessage(&windowsMessage, 0, 0, 0) <= 0)
		{
			mustQuit = true;
			return;
		}

		if(windowsMessage.message == WM_LBUTTONDOWN || windowsMessage.message == WM_ACTIVATE || windowsMessage.message == WM_NCLBUTTONDOWN)
			editorState.endCompletionVisualization();

		if(windowsMessage.message == WM_MOUSELAST + 1)
			windowsMessage.hwnd = gui.getMainWindow().getWindowHandle();
		if(ignoreMessage(windowsMessage))
			return;
		if(gui.handleDialogs(windowsMessage))
			return;

		TranslateMessage(&windowsMessage);
		DispatchMessage(&windowsMessage);
	}

	void windowsMessages()
	{
		if(!gui.getMainWindow().isActive())
			handleMessage();

		MSG windowsMessage = { 0 };
		while(PeekMessage(&windowsMessage, 0, 0, 0, PM_NOREMOVE)) 
			handleMessage();

//if(!mustQuit)
//	handleMessage();


//		Sleep(0);
	}

	void tick()
	{
		gui.getMouse().update();

		bool focused_on_text = false;
		HWND focusWindow = GetFocus();
		if(IsWindow(focusWindow))
		{
			char winname[32] = {0};
			GetClassName(focusWindow, winname, 32);
			if(strcmp(winname, "Edit") == 0)
				focused_on_text = true;
		}
		camera.update(gui.getMouse(), !focused_on_text);
	}
};

Application::Application()
{
	editorMode = true;
/*
	filesystem::FilePackageManager &manager = filesystem::FilePackageManager::getInstance();
	boost::shared_ptr<filesystem::IFilePackage> standardPackage( new filesystem::StandardPackage() );
	boost::shared_ptr<filesystem::IFilePackage> zipPackage1( new filesystem::ZipPackage( "data1.fbz" ) );
	boost::shared_ptr<filesystem::IFilePackage> zipPackage2( new filesystem::ZipPackage( "data2.fbz" ) );
	boost::shared_ptr<filesystem::IFilePackage> zipPackage3( new filesystem::ZipPackage( "data3.fbz" ) );
	boost::shared_ptr<filesystem::IFilePackage> zipPackage4( new filesystem::ZipPackage( "data4.fbz" ) );

	manager.addPackage( standardPackage, 999 );
	manager.addPackage( zipPackage1, 1 );
	manager.addPackage( zipPackage2, 2 );
	manager.addPackage( zipPackage3, 3 );
	manager.addPackage( zipPackage4, 4 );
*/

	boost::scoped_ptr<ApplicationData> tempData(new ApplicationData());
	data.swap(tempData);

	UniqueEditorObjectHandleManager::init();
#ifdef LEGACY_FILES
		// (should be ifdef project_survivor, but survivor's editor does not have that defined)
#else
	if (UniqueEditorObjectHandleManager::wasError())
	{
		MessageBox(0, "Failed initialize unique handles.\r\n\r\nThis may cause non-unique object handles.\r\n(Changes made to maps in game may get read back to editor incorrectly.)", "Warning", MB_ICONEXCLAMATION|MB_OK);
	}
	else if (UniqueEditorObjectHandleManager::wasLocked())
	{
		MessageBox(0, "Editor unique handle files were locked.\r\n\r\nAnother editor instance already running or editor did not exit cleanly?\r\nThis may cause non-unique object handles.\r\n(Changes made to maps in game may get read back to editor incorrectly.)", "Warning", MB_ICONEXCLAMATION|MB_OK);
	}
	else if (UniqueEditorObjectHandleManager::wasFirstTimeInit())
	{
		MessageBox(0, "Editor unique handle files were successfully initialized.\r\n\r\n(This should happen during first use of editor.)", "Information", MB_OK);
	}
#endif
}

Application::~Application()
{
	UniqueEditorObjectHandleManager::uninit();
#ifdef LEGACY_FILES
		// (should be ifdef project_survivor, but survivor's editor does not have that defined)
#else
	if (UniqueEditorObjectHandleManager::wasError())
	{
		MessageBox(0, "Failed to save unique handle iterator value.\r\n\r\nThis may cause non-unique object handles in the future.\r\n(Changes made to maps in game may get read back to editor incorrectly.)", "Warning", MB_ICONEXCLAMATION|MB_OK);
	}
#endif
}

void Application::run()
{
	if(!data->storm.storm)
		return;

	timeBeginPeriod(1);
	data->windowsMessages();
	//data->storm.scene->RenderVideo("Data\\Videos\\frozenbyte_logo.mpg", 0);
	int polygonCounter = 0;
	int frameCount = 0;
	int timeValue = 0;

	char buffer[128] =  { 0 };
	createFont(*data->storm.storm);

/*
IStorm3D_Texture *tex = data->storm.storm->CreateNewTexture("missing.dds");
IStorm3D_Material *m = data->storm.storm->CreateNewMaterial("..");
m->SetAlphaType(IStorm3D_Material::ATYPE_USE_TRANSPARENCY);
m->SetTransparency(0.7f);
m->SetBaseTexture(tex);
Storm3D_SurfaceInfo info = data->storm.storm->GetScreenSize();
*/
/*
	IStorm3D_Texture *t = data->storm.storm->CreateNewTexture("missing.dds", TEXLOADFLAGS_NOCOMPRESS);
	Storm3D_SurfaceInfo info = t->GetSurfaceInfo();
	DWORD *buf = new DWORD[info.width * info.height];
	t->CopyTextureTo32BitSysMembuffer(buf);
	delete[] buf;
*/

//IStorm3D_VideoStreamer *streamer = data->storm.storm->CreateVideoStreamer("Data/Videos/logo.wmv", 0, 0);
//IStorm3D_VideoStreamer *streamer = data->storm.storm->CreateVideoStreamer("Data/Videos/Mainmenu.wmv", 0, true);
//IStorm3D_VideoStreamer *streamer = data->storm.storm->CreateVideoStreamer("Data/Videos/RepairHall_menu_divx.avi", 1.f, true);
//IStorm3D_VideoStreamer *streamer2 = 0;//data->storm.storm->CreateVideoStreamer("Data/Videos/SHADOWGROUNDS_menulogo.wmv", 1.f, true);
/*
IStorm3D_Texture *tex = data->storm.storm->CreateNewTexture(512, 512, IStorm3D_Texture::TEXTYPE_BASIC_RENDER);
IStorm3D_Material *m = data->storm.storm->CreateNewMaterial("..");
m->SetAlphaType(IStorm3D_Material::ATYPE_USE_TRANSPARENCY);
m->SetTransparency(0.7f);
m->SetBaseTexture(tex);
Storm3D_SurfaceInfo info = data->storm.storm->GetScreenSize();
*/

	while(!data->mustQuit)
	{
		data->tick();
		data->windowsMessages();

		if(!data->gui.getMainWindow().isActive())
			continue;

		data->editorState.tick();

		int newTime = timeGetTime();
		++frameCount;

		char *stormInfo = data->storm.storm->GetPrintableStatusInfo();
		//if(GetKeyState('P') & 0x80)
		//	OutputDebugString(stormInfo);

		if(newTime - timeValue > 1000)
		{
			int fps = frameCount / ((newTime - timeValue) / 1000);

			timeValue = newTime;
			frameCount = 0;

			sprintf(buffer, "fps %d, polygons: %d", +fps, polygonCounter);
		}

		if(font)
			data->storm.scene->Render2D_Text(font, VC2(10,10), VC2(10, 18), buffer);
/*
for(unsigned int i = 0; i < fakeMin.size(); ++i)
{
VC2 min = fakeMin[i];
VC2 size = fakeSize[i];
float x = info.width * min.x;
float y = info.height * min.y;
float width = info.width * (size.x);
float height = info.height * (size.y);
data->storm.scene->Render2D_Picture(m, VC2(x,y), VC2(width,height));
}
if(streamer)
{
streamer->update();
data->storm.scene->Render2D_Picture(streamer->getMaterial(), VC2(0,200), VC2(800,600));
}
if(streamer2)
{
streamer2->update();
data->storm.scene->Render2D_Picture(streamer2->getMaterial(), VC2(0,50), VC2(200,150));
}
*/

		polygonCounter = data->storm.scene->RenderScene();
		data->storm.storm->DeletePrintableStatusInfo(stormInfo);
	}

//delete streamer;
//delete streamer2;

}

} // end of namespace editor
} // end of namespace frozenbyte
