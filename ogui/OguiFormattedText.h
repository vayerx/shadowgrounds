#ifndef INC_OGUIFORMATTEDTEXT_H
#define INC_OGUIFORMATTEDTEXT_H

#include <map>
#include <list>
#include <string>
#include <stack>

#include "OguiButton.h"

class IOguiFont;
class OguiWindow;
class Ogui;
class IOguiFormattedCommand;

class OguiFormattedText
{
public: 
	struct ParseData
	{
		ParseData( int x, int y, int w, int h ) : 
			next_tag( 0 ),
			next_linebrk( 0 ),
			next_hardbreak( 0 ),
			cur_pos( 0 ),
			currentFont( NULL ),
			mx( 0 ),
			my( 0 ),
			cur_w( 0 ),
			add_x( 0 ),
			x( x ), 
			y( y ), 
			w( w ), 
			h( h ),
			after_y( h ),
			after_y_x_value( 0 ),
			after_y_w_value( 0 ) { }

		std::string::size_type next_tag;
		std::string::size_type next_linebrk;
		std::string::size_type next_hardbreak;
		int cur_pos;
		IOguiFont* currentFont;

		int mx;
		int my;
		int cur_w;
		int add_x;

		int x;
		int y;
		int w;
		int h;

		int after_y;
		int after_y_x_value;
		int after_y_w_value;
	};

	OguiFormattedText( OguiWindow* win, Ogui* ogui, int x, int y, int w, int h, int id = 0 );
	~OguiFormattedText();

	//.........................................................................

	// Registers a font to a certain tag. For example, 
	// registerFont( "b", arial_bold );
	// would register the text between <b></b> tags to be used as arial_bold
	void registerFont( const std::string& tag_name, IOguiFont*font );

	// Deletes all registered fonts. Not just remove, but actually delete!
	void deleteRegisteredFonts();

	// Registers a OguiFormattedCommand to a certain tag.
	void registerCommand( const std::string& command_name, IOguiFormattedCommand* command );

	// This font will be used there where there is text with no tags
	void setFont( IOguiFont* font );
	IOguiFont* getFont();

	// Sets text to be parsed as a nice formated text
	void setText( const std::string& text );

	// For the scrolling and scrollbars
	void setClip( int x, int y, int w, int h, bool half_clip = true );
	
	// Where do we begin to draw the text
	void setY( int y );

	// Moves all the hole thing by given pixels
	void moveBy( int x, int y , bool clear_top = false, bool clear_bottom = false );

	// Sets new position
	void move(int x, int y);
	
	// These tell us up to where did the clipping happen. 
	// Returns clip sizes if half_clip is true
	int getClipPositionTop() const;
	int getClipPositionBottom() const;

	int getId() const;

	Ogui* getOgui() const;
	
	// applies the transparency (0-100) to all the children
	void setTransparency( int transparency );

	// sets horizontal alignment for the button caption text
	void setTextHAlign( OguiButton::TEXT_H_ALIGN hAlign );

	// sets vertical alignment for the button caption text
	void setTextVAlign( OguiButton::TEXT_V_ALIGN vAlign );

	// sets lineheight multiplier, default: 1.0
	void setLineHeight( float times );

	// creates a new OguiButton and pushes it to the std::list buttons
	void createTextButton( int x, int y, int w, int h, const std::string& text, IOguiFont* font = NULL );

	// creates a new ogui button from the given image
	void createImageButton( int button_x, int button_y, int button_w, int button_h, const std::string& image );

	// creates a new ogui button from the given IOguiImage* pointer. The 
	// instance of the image is stored and freed when the button is freed
	void createImageButton( int button_x, int button_y, int button_w, int button_h, IOguiImage* image );

	int getX() const;
	int getY() const;
	int getW() const;
	int getH() const;

	// valid after setting text
	inline int getLinePositionY( unsigned int line ) { if(line >= 0 && line < linePositionYs.size()) return linePositionYs[line]; return 0; }

private:

	//.........................................................................

	// parses text ( the member variable ) to a collection of OguiButtons
	void					parseTextToButtons();

	void					parseFormattedCommand( const std::string& command, ParseData* data );

	// returns the line width in pixels. Parses the text ( the member 
	// variable ) startting from curpos
	int						getLineWidth( int curpos, const std::stack< std::string >& tagstack, ParseData* data );
	
	// finds a place where the text goes over the width with the given cursor. 
	// Returns the string position in pair.first and the length of it in pixels 
	// in pair.second. The Length in pixels isn't that useful anymore.
	std::pair< int, int >	findLineWidthBreak( std::string::size_type pos, int width, IOguiFont* cur_font );

	// releases all the allocated buttons and reserved images
	void					releaseAllButtons();

	//.........................................................................

	// replaces all occurances of _what_ with _with_ in the _in_here_ parameter
	// std::string stringReplace( const std::string& what, const std::string& with, const std::string& in_here, int limit = -1 );

	//.........................................................................

	
	std::list< OguiButton* >						buttons;
	std::map< std::string, IOguiFormattedCommand* >	commands;

	std::vector< int > linePositionYs;
	
	// used to keep track of the tag names and their corresponding fonts
	std::map< std::string, IOguiFont* >	fonts;
	std::list< IOguiImage* >			images;
	
	// keeps track of the tags we encounter in the parseable text
	std::stack< std::string > tagstack;

	IOguiFont*					currentFont;

	// The text aligments
	OguiButton::TEXT_H_ALIGN hAlign;

	// Unsupported...
	OguiButton::TEXT_V_ALIGN vAlign;


	OguiWindow*		window;
	Ogui*			ogui;
	
	// the current font used if no other font by tags are given
	IOguiFont*		font;
	
	// the text we parse
	std::string		text;

	float					lineHeight;

	struct Rect;

	Rect*	position;
	Rect*	clip;
	bool	halfClip;
	int		clipPositionTop;
	int		clipPositionBottom;

	int		id;

	IOguiFormattedCommand*	releaseMe;

	//.........................................................................

};


#endif
