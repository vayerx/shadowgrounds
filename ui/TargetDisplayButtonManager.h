#ifndef INC_TARGETDISPLAYBUTTONMANAGER_H
#define INC_TARGETDISPLAYBUTTONMANAGER_H

#include <map>
#include <vector>
#include <stack>
#include <string>

class Ogui;
class OguiWindow;
class OguiButton;
class IOguiFont;
class OguiSlider;
class IOguiImage;

namespace ui {

class TargetDisplayButtonManager
{
public:
	///////////////////////////////////////////////////////////////////////////

	struct ButtonCreator
	{
	public:
		std::vector< std::string > buttonImages;
		int width;
		int height;
		float beginAnimPos;
		IOguiFont*					font;
		int textOffsetX;
		int textOffsetY;
		bool textCenter;

		bool hasSlider;
		std::string sliderBackground;
		std::string sliderForeground;
		std::string sliderBackgroundLow;
		std::string sliderForegroundLow;
		std::string sliderBackgroundHigh;
		std::string sliderForegroundHigh;
		int sliderLowLimit;
		int sliderHighLimit;
		int sliderWidth;
		int sliderHeight;

		int rect_style;
	};

	///////////////////////////////////////////////////////////////////////////

	struct ButtonData
	{
		ButtonData() 
		{ 
			style = -1; 
			font = NULL; 
			hasText = false; 
			width = 0;
			height = 0;
			beginAnimPos = 0;
			offsetX = 0;
			offsetY = 0;
			textCenter = true;
			textWidth = 0;
			healthSlider = NULL;
			healthSliderImages[0] = 0;
			healthSliderImages[1] = 0;
			healthSliderImages[2] = 0;
			healthSliderImages[3] = 0;
			healthSliderImages[4] = 0;
			healthSliderImages[5] = 0;
			sliderWidth = 0;
			sliderHeight = 0;
			sliderLowLimit = 0.3f;
			sliderHighLimit = 0.7f;
			sliderCurrentImages = 0;
		}

		ButtonData( const ButtonData& b ) : 
			buttons( b.buttons ), 
			style( b.style ), 
			font( b.font ),
			hasText( b.hasText ),
			width( b.width ),
			height( b.height ),
			beginAnimPos( b.beginAnimPos ),
			offsetX( b.offsetX ),
			offsetY( b.offsetY ),
			textCenter( b.textCenter ),
			textWidth( b.textWidth ),
			healthSlider( b.healthSlider ),
			sliderWidth( b.sliderWidth ),
			sliderHeight( b.sliderHeight ),
			sliderLowLimit( b.sliderLowLimit ),
			sliderHighLimit( b.sliderHighLimit ),
			sliderCurrentImages( b.sliderCurrentImages )
		{
			healthSliderImages[0] = b.healthSliderImages[0];
			healthSliderImages[1] = b.healthSliderImages[1];
			healthSliderImages[2] = b.healthSliderImages[2];
			healthSliderImages[3] = b.healthSliderImages[3];
			healthSliderImages[4] = b.healthSliderImages[4];
			healthSliderImages[5] = b.healthSliderImages[5];
		}

		~ButtonData() { }

		void operator=( const ButtonData& b ) 
		{
			buttons = b.buttons;
			style = b.style;
			font = b.font;
			hasText = b.hasText;
			width = b.width;
			height = b.height;
			beginAnimPos = b.beginAnimPos;
			offsetX = b.offsetX;
			offsetY = b.offsetY;
			textCenter = b.textCenter;
			textWidth = b.textWidth;
			healthSlider = b.healthSlider;
			healthSliderImages[0] = b.healthSliderImages[0];
			healthSliderImages[1] = b.healthSliderImages[1];
			healthSliderImages[2] = b.healthSliderImages[2];
			healthSliderImages[3] = b.healthSliderImages[3];
			healthSliderImages[4] = b.healthSliderImages[4];
			healthSliderImages[5] = b.healthSliderImages[5];
			sliderWidth = b.sliderWidth;
			sliderHeight = b.sliderHeight;
			sliderLowLimit = b.sliderLowLimit;
			sliderHighLimit = b.sliderHighLimit;
			sliderCurrentImages = b.sliderCurrentImages;
		}

		OguiButton*& operator[] (int i) 
		{
			return buttons[ i ];
		}

		std::vector< OguiButton* >	buttons;
		int							style;
		IOguiFont*					font;
		bool						hasText;
		int							width;
		int							height;
		float						beginAnimPos;
		std::string					theText;
		int							offsetX;
		int							offsetY;
		bool						textCenter;
		int							textWidth;
		// bool						hasHealthSlider;
		OguiSlider*					healthSlider;
		IOguiImage*					healthSliderImages[6];
		int							sliderWidth;
		int							sliderHeight;
		float sliderLowLimit;
		float sliderHighLimit;
		int sliderCurrentImages;
	};
	
	///////////////////////////////////////////////////////////////////////////


	typedef ButtonData button;

	TargetDisplayButtonManager( Ogui* ogui, OguiWindow* window );
	~TargetDisplayButtonManager();

	void	registerButtonStyle( int style, const ButtonCreator& butt );

	button	createButton( int style );
	void	releaseButton( button b );

	void    setText( button& b, const std::string& text );

	bool	isRegistered( int style ) const;


	const ButtonCreator &getButtonStyle( int style ) const;

private:

	// This just creates a new button from the buttnOriginals
	// doesn't use the buttonStack in anyway
	button  newButton( int style );
	
	Ogui*		ogui;
	OguiWindow*	window;

	std::map< int, std::stack< button > >	buttonStack;
	std::map< int, ButtonCreator >			buttonOriginals;		

};

} // end of namespace ui

#endif
