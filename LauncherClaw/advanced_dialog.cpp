#include "advanced_dialog.h"

#include "window.h"
#include "dialog_data.h"
#include "idlghandler.h"
#include "dlghandlerimpl.h"
#include "options_value_manager.h"
#include "launcher_window.h"

#include "../game/GameOptionManager.h"
#include "../game/GameOption.h"

#include "../util/hiddencommand.h"

#include <assert.h>
#include <string>
#include <vector>


#include "Resource/resource.h"

using namespace game;

namespace frozenbyte {
namespace launcher {
///////////////////////////////////////////////////////////////////////////////

class AdvancedDialogHandler : public DlgHandlerImpl
{
public:
	OptionsValueManager manager;
	LauncherWindow*		lwindow;

	void initDialog( HWND hwnd )
	{
		addComboItems( GetDlgItem( hwnd, IDC_COMBOFAKESHADOWSTEXTUREQUALITY ),	manager.getOptionNames( "Fake Shadows Texture Quality" ),	manager.getTheOneInUse( "Fake Shadows Texture Quality" ) );
		addComboItems( GetDlgItem( hwnd, IDC_COMBOLIGHTINGLEVEL ),				manager.getOptionNames( "Lighting Level" ),					manager.getTheOneInUse( "Lighting Level" ) );
		addComboItems( GetDlgItem( hwnd, IDC_COMBOLIGHTNINGTEXTUREQUALITY ),	manager.getOptionNames( "Lighting Texture Quality" ),		manager.getTheOneInUse( "Lighting Texture Quality" ) );
		addComboItems( GetDlgItem( hwnd, IDC_COMBOPARTICLEEFFECTSLEVEL ),		manager.getOptionNames( "Particle Effects Level" ),			manager.getTheOneInUse( "Particle Effects Level" ) );
		addComboItems( GetDlgItem( hwnd, IDC_COMBOSHADOWSLEVEL ),				manager.getOptionNames( "Shadows Level" ),					manager.getTheOneInUse( "Shadows Level" ) );
		addComboItems( GetDlgItem( hwnd, IDC_COMBOSHADOWSTEXTUREQUALITY ),		manager.getOptionNames( "Shadows Texture Quality" ),		manager.getTheOneInUse( "Shadows Texture Quality" ) );
		addComboItems( GetDlgItem( hwnd, IDC_COMBOTEXTUREDETAILLEVEL ),			manager.getOptionNames( "Texture Detail Level" ),			manager.getTheOneInUse( "Texture Detail Level" ) );
		addComboItems( GetDlgItem( hwnd, IDC_COMBOANISOTROPIC ),				manager.getOptionNames( "Anisotropic Filtering" ),			manager.getTheOneInUse( "Anisotropic Filtering" ) );
		addComboItems( GetDlgItem( hwnd, IDC_COMBOANTIALIAS ),					manager.getOptionNames( "Antialiasing" ),					manager.getTheOneInUse( "Antialiasing" ) );
		addComboItems( GetDlgItem( hwnd, IDC_COMBOMISC ),						manager.getOptionNames( "Misc" ),							manager.getTheOneInUse( "Misc" ) ); 

		setCheckBox( hwnd, IDC_CHECKFULLSCREEN,				manager.getTheOneInUse( "Fullscreen" ) );
		setCheckBox( hwnd, IDC_CHECKEXTRAGAMMAEFFECTS,		manager.getTheOneInUse( "Extra Gamma Effects" ) );
		setCheckBox( hwnd, IDC_CHECKHIGHQUALITYLIGHTMAP,	manager.getTheOneInUse( "High Quality Lightmap" ) );
		setCheckBox( hwnd, IDC_CHECKRENDERGLOW,				manager.getTheOneInUse( "Render Glow" ) );
		setCheckBox( hwnd, IDC_CHECKWEATHEREFFECTS,			manager.getTheOneInUse( "Weather Effects" ) );
		// setCheckBox( hwnd, IDC_CHECKVIDEOENABLED,			manager.getTheOneInUse( "VideoEnabled" ) );
		setCheckBox( hwnd, IDC_CHECKDISTORTIONEFFECTS,		manager.getTheOneInUse( "DistortionEffects" ) );
		setCheckBox( hwnd, IDC_CHECKDISAPEARINGCORPSES,		manager.getTheOneInUse( "CorpseDisapear" ) );
		setCheckBox( hwnd, IDC_CHECKHIGHQUALITYVIDEO,		manager.getTheOneInUse( "High Quality Video" ) );
		setCheckBox( hwnd, IDC_CHECKRESETRENDERERAFTERLOAD,	manager.getTheOneInUse( "ResetAfterLoad" ) );
		
		// buttons
		setDescriptionText( GetDlgItem( hwnd, IDOK ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CANCEEL ) );

		// combo boxes
		setDescriptionText( GetDlgItem( hwnd, IDC_CAPTIONFAKESHADOWS ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_STATICTEXTUREDETAIL ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_STATICLIGHTNINGLEVEL ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CAPTIONANISOTROPIC ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_STATICLIGHTININGTEXTURE ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_STATICANTIALIASING ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_STATICPARTICLEEFFECTAS ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_STATICSHADOWLEVELS ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_STATICSHADOWSTEXTUREQUALITY ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_STATICMISC ) ); 
		
		// check boxes
		setDescriptionText( GetDlgItem( hwnd, IDC_CHECKFULLSCREEN ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CHECKWEATHEREFFECTS ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CHECKHIGHQUALITYLIGHTMAP ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CHECKEXTRAGAMMAEFFECTS ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CHECKRENDERGLOW ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CHECKDISTORTIONEFFECTS ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CHECKDISAPEARINGCORPSES ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CHECKHIGHQUALITYVIDEO ) );
		setDescriptionText( GetDlgItem( hwnd, IDC_CHECKRESETRENDERERAFTERLOAD ) );
	}

	void applyOptions( HWND hwnd )
	{
		manager.applyOptions( "Fake Shadows Texture Quality",	getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBOFAKESHADOWSTEXTUREQUALITY ) ) );
		manager.applyOptions( "Lighting Level",					getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBOLIGHTINGLEVEL ) ) );
		manager.applyOptions( "Lighting Texture Quality",		getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBOLIGHTNINGTEXTUREQUALITY ) ) );
		manager.applyOptions( "Particle Effects Level",			getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBOPARTICLEEFFECTSLEVEL ) ) );
		manager.applyOptions( "Shadows Level",					getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBOSHADOWSLEVEL ) ) );
		manager.applyOptions( "Shadows Texture Quality",		getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBOSHADOWSTEXTUREQUALITY ) ) );
		manager.applyOptions( "Texture Detail Level",			getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBOTEXTUREDETAILLEVEL ) ) );
		manager.applyOptions( "Anisotropic Filtering",			getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBOANISOTROPIC ) ) );
		manager.applyOptions( "Antialiasing",					getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBOANTIALIAS ) ) );
		manager.applyOptions( "Misc",							getComboBoxSelection( GetDlgItem( hwnd, IDC_COMBOMISC ) ) );

		manager.applyOptions( "Fullscreen",				getCheckBoxValue( hwnd, IDC_CHECKFULLSCREEN ) );
		manager.applyOptions( "Extra Gamma Effects",	getCheckBoxValue( hwnd, IDC_CHECKEXTRAGAMMAEFFECTS )  );
		manager.applyOptions( "High Quality Lightmap",	getCheckBoxValue( hwnd, IDC_CHECKHIGHQUALITYLIGHTMAP )  );
		manager.applyOptions( "Render Glow",			getCheckBoxValue( hwnd, IDC_CHECKRENDERGLOW )  );
		manager.applyOptions( "Weather Effects",		getCheckBoxValue( hwnd, IDC_CHECKWEATHEREFFECTS )  );
		// manager.applyOptions( "VideoEnabled",			getCheckBoxValue( hwnd, IDC_CHECKVIDEOENABLED )  );
		manager.applyOptions( "CorpseDisapear",			getCheckBoxValue( hwnd, IDC_CHECKDISAPEARINGCORPSES )  );
		manager.applyOptions( "DistortionEffects",		getCheckBoxValue( hwnd, IDC_CHECKDISTORTIONEFFECTS )  );
		manager.applyOptions( "High Quality Video",		getCheckBoxValue( hwnd, IDC_CHECKHIGHQUALITYVIDEO ) );
		manager.applyOptions( "ResetAfterLoad",			getCheckBoxValue( hwnd, IDC_CHECKRESETRENDERERAFTERLOAD ) );

		// CorpseDisapear
		// getCheckBoxValue( hwnd, IDC_CHECKFULLSCREEN )

	}

	BOOL handleMessages( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		switch( msg )
		{
		case WM_INITDIALOG:
			// initDialog( hwnd );			
			break; 

		case WM_COMMAND:
		{
			int command = LOWORD(wParam);
						
			switch( command )
			{
			case IDOK:
				applyOptions( hwnd );
				// GameOptionManager::getInstance()->save();
				// PostQuitMessage( 0 );
				// ShellExecute( 0, 0, "disposable.exe", 0, 0, SW_NORMAL );
				
				// manager.save();
				lwindow->closeAdvanced();
				break;

			case IDC_CANCEEL:
				lwindow->closeAdvanced();
				break;
			}

			break;
		}

		case WM_SHOWWINDOW:
			manager.load();
			initDialog( hwnd );	
			break;

		default:;
			// assert( false );
		};

		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////

AdvancedDialog::AdvancedDialog( const Window& parent_window, LauncherWindow* lwindow )
{
	handler = new AdvancedDialogHandler;
	handler->lwindow = lwindow;

	dialog = new DialogData( IDD_ADVANCED, parent_window, DialogData::ATTACH_ALL, handler );
}

//=============================================================================

AdvancedDialog::~AdvancedDialog()
{
	delete handler;
	delete dialog;
}

///////////////////////////////////////////////////////////////////////////////

void AdvancedDialog::show()
{
	dialog->show();
}

//=============================================================================

void AdvancedDialog::hide()
{
	dialog->hide();
}

///////////////////////////////////////////////////////////////////////////////
} // end of namespace launcher
} // end of namespace frozenbyte
