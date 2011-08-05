
#ifndef ORVGUI_H
#define ORVGUI_H

#include <Storm3D_UI.h>

//
// OrvGUI quick hack conversion to Storm3D
// Don't use these functions directly, see the Ogui.h
//

//
// v2.0.0 - 19.4.2002 - jpkokkon
// 


// #define OG_HIDE_OWN_CURSORS

//#define OG_RAISE_WINDOW_ON_CLICK
#define OG_REACT_TOPMOST_WINDOW_ONLY

//#define OG_MOVE_WITH_BUTTON3
#define OG_MOVE_FROM_TOP_WITH_BUTTON1
#define OG_TOP_MOVE_HEIGHT 24

#define OG_CURSORS 8

#define OG_CTRL_MASK_MOUSE 1							// Controlled by every mouse.
																// It would be wisest to use this in menus as well.
#define OG_CTRL_MASK_KEYBOARD1 2
#define OG_CTRL_MASK_KEYBOARD2 4
#define OG_CTRL_MASK_JOYSTICK1 8
#define OG_CTRL_MASK_JOYSTICK2 16
#define OG_CTRL_MASK_JOYSTICK3 32
#define OG_CTRL_MASK_JOYSTICK4 64
#define OG_CTRL_MASK_DISABLE_JOYSTICK_WARP 128 // (ability to jump from button to another with joystick)
#define OG_CTRL_MASK_MOUSE0 (128 << 1)				// Controlled by mouse #0
#define OG_CTRL_MASK_MOUSE1 (128 << 2)				// Controlled by mouse #1
#define OG_CTRL_MASK_MOUSE2 (128 << 3)				// Controlled by mouse #2
#define OG_CTRL_MASK_MOUSE3 (128 << 4)				// Controlled by mouse #3
#define OG_CTRL_MASK_MOUSE4 (128 << 5)				// Controlled by mouse #4
#define OG_CTRL_MASK_MICE ( OG_CTRL_MASK_MOUSE | OG_CTRL_MASK_MOUSE0 | OG_CTRL_MASK_MOUSE1 | OG_CTRL_MASK_MOUSE2 | OG_CTRL_MASK_MOUSE3 | OG_CTRL_MASK_MOUSE4)


#define OG_WIN_SIMPLE 1

#define OG_BUT_PIC 1
#define OG_BUT_PIC_AND_TEXT 2

#define OG_WIN_UNMOVABLE 0
#define OG_WIN_MOVABLE 1

#define OG_WIN_NOTPOPUP 0
#define OG_WIN_POPUP 1
#define OG_WIN_POPUPNOCLOSE 2
#define OG_WIN_POPUPNOCLOSEONBUTTON 3

#define OG_CURSOR_1_MASK 1
#define OG_CURSOR_2_MASK 2
#define OG_CURSOR_3_MASK 4
#define OG_CURSOR_4_MASK 8
#define OG_CURSOR_5_MASK 16
#define OG_CURSOR_6_MASK 32
#define OG_CURSOR_7_MASK 64
#define OG_CURSOR_8_MASK 128
#define OG_CURSOR_ALL_MASK 0xff

#define OG_BUT_1_MASK 1
#define OG_BUT_2_MASK 2
#define OG_BUT_3_MASK 4
#define OG_BUT_4_MASK 8
#define OG_BUT_5_MASK 16
#define OG_BUT_6_MASK 32
#define OG_BUT_7_MASK 64
#define OG_BUT_OVER_MASK 128
#define OG_BUT_WHEEL_UP_MASK 256
#define OG_BUT_WHEEL_DOWN_MASK 512
#define OG_BUT_ALL_MASK 0x03FF

#define OG_H_ALIGN_LEFT 1
#define OG_H_ALIGN_CENTER 2
#define OG_H_ALIGN_RIGHT 3

#define OG_V_ALIGN_TOP 1
#define OG_V_ALIGN_MIDDLE 2
#define OG_V_ALIGN_BOTTOM 3

#define OG_TEXT_H_ALIGN_LEFT_PAD 2
#define OG_TEXT_H_ALIGN_RIGHT_PAD 2
#define OG_TEXT_V_ALIGN_TOP_PAD 2
#define OG_TEXT_V_ALIGN_BOTTOM_PAD 4

// NOTE: these values must equal the ones defined in OguiWindow MOVE_BOUND enum.
#define OG_MOVE_BOUND_ALL_IN_SCREEN 0
#define OG_MOVE_BOUND_PART_IN_SCREEN 1
#define OG_MOVE_BOUND_NO_PART_IN_SCREEN 2

//#define OG_FONT_SIZEX 16
//#define OG_FONT_SIZEY 16
//#define OG_FONTS_PER_ROW 64

#define OG_MAX_HOTKEYS 3

#define OG_SCALE_MULTIPLIER 1024

typedef struct {
	unsigned char type;
	unsigned char react_cursor;
	unsigned char visible;
	unsigned char refresh_flag;
	unsigned char movable;
	unsigned char popup;
	unsigned char dragged;
	float alpha;
	//unsigned char cursor_state;
	//unsigned char fgcolor;
	//unsigned char bgcolor;
	//unsigned char brightcolor;
	//unsigned char darkcolor;
	IStorm3D_Material 		*bg_pic;
	//picture 		*get_pic;
	//picture 		*put_pic;
	short 		put_x;
	short 		put_y;
	short 		sizex;
	short 		sizey;
	void			*next_sister;
	void			*prev_sister;
	void			*first_child;
	unsigned char move_bound;
	float scroll_x;
	float scroll_y;
	float bg_repeat_factor_x;
	float bg_repeat_factor_y;
	bool wrap;
} orvgui_win;

typedef struct {
	unsigned char type;
	unsigned int react_but;
	unsigned char enabled;
	unsigned char pressed;
	unsigned char highlighted;
	//unsigned char fgcolor;
	//unsigned char bgcolor;
	//unsigned char brightcolor;
	//unsigned char darkcolor;
	unsigned char 	text_v_align;
	unsigned char 	text_h_align;
	unsigned char 	linebreaks;
	char			*text;
	int 			text_pixwidth;
	int 			text_pixheight;
	int 			fontwidth;
	int 			fontheight;
	IStorm3D_Font *font;
	COL font_color;
	IStorm3D_Font *font_down;
	COL font_down_color;
	IStorm3D_Font *font_disabled;
	COL font_disabled_color;
	IStorm3D_Font *font_highlighted;
	COL font_highlighted_color;
		
	IStorm3D_Material 		*get_pic;
	IStorm3D_Material 		*get_picdown;
	IStorm3D_Material 		*get_picdisabled;
	IStorm3D_Material 		*get_pichighlighted;
	float rotation;
	float alpha;
	//picture 		*put_pic;
	//short 		get_x;
	//short 		get_y;
	short 		put_x;
	short 		put_y;
	short 		sizex;
	short 		sizey;
	float 	clipleftx;
	float 	cliptopy;
	float 		cliprightx;
	float 	clipbottomy;
	bool	clip_to_window; // by Pete

	short hotKeys[OG_MAX_HOTKEYS];

	bool wrap;
	float repeat_x;
	float repeat_y;
	float scroll_x;
	float scroll_y;
	orvgui_win		*parent;
	void			*next_sister;
	void			*prev_sister;
	void			(*proc_press)(void);
	void			(*proc_click)(void);
	void			(*proc_out)(void);
	void			(*proc_over)(void);
	void			(*proc_leave)(void);
	void			(*proc_hold)(void);
	void			*arg_press;
	void			*arg_click;
	void			*arg_out;
	void			*arg_over;
	void			*arg_leave;
	void			*arg_hold;
	//void			(*proc_on)(void);

	typedef struct
	{
		int x,y;
		DWORD col;
	} vertex;
	vertex *vertices;
	int num_vertices;

} orvgui_but;

extern void *og_arg;

extern IStorm3D_Scene *og_renderer;
extern IStorm3D *og_storm3d;

extern int got_orvgui;

extern int og_errors;

extern int og_terminate;

extern int og_disable_kj_move;

extern bool og_menu_index_mode;

extern bool og_hotkeys_enabled;

extern short og_cursor_num;
extern short og_cursor_x;
extern short og_cursor_y;
extern short og_cursor_scrx;
extern short og_cursor_scry;
extern unsigned int og_cursor_but;
extern unsigned int og_cursor_obut;
extern orvgui_but *og_cursor_on;

// these are for a real quick hack, should not be used
extern int cursor_x[OG_CURSORS];
extern int cursor_y[OG_CURSORS];

extern char og_readchar;
extern int og_readkey;

extern const char *og_errorfile;

extern int og_escape;

extern bool og_visualize_windows;


void init_orvgui(void);
void uninit_orvgui(void);

void og_setRendererScene(IStorm3D_Scene *scene);
void og_setStorm3D(IStorm3D *s3d);

void og_set_scale(int scale_x, int scale_y);
int og_get_scale_x();
int og_get_scale_y();
void og_set_mouse_sensitivity(float sensitivity_x, float sensitivity_y);

void og_set_skip_cursor_movement();
void og_update_cursor_positions();
int og_get_cursor_position_x(int cursornum, bool exact = false);
int og_get_cursor_position_y(int cursornum, bool exact = false);
void og_set_cursor_position_x(int cursornum, int screenX);
void og_set_cursor_position_y(int cursornum, int screenY);
void og_set_cursor_position_offset_x(int cursornum, int screenX);
void og_set_cursor_position_offset_y(int cursornum, int screenY);

void og_apply_system_cursor_pos(int cursornum);

void og_warp_cursor_to(int cursor, orvgui_but *to_button);

void og_empty_proc(void);
void og_proc_hide_parent(void);
void og_proc_delete_parent(void);
void og_proc_delete_self(void);

void og_run_gui(void);
void og_set_only_active(orvgui_win *win);
void og_add_keyreader(void (*handler_proc)(void), int id, int cursornum, void *arg);
void og_remove_keyreader(int id);
void og_set_cursor_pic(int curnum, IStorm3D_Material *pic);
void og_set_cursor_state(int curnum, int state);
int og_get_cursor_state(int curnum);
void og_set_cursor_control(int curnum, unsigned int ctrlmask);
unsigned int og_get_cursor_control(int curnum);
void og_set_cursor_control_keyboard(int curnum, int left, int right, int up, int down, int fire, int fire2);
void og_set_cursor_offset(int curnum, int offx, int offy);

void og_refresh_screen(void);
void og_draw_screen(void);

orvgui_but *og_create_button(orvgui_win *parent, unsigned char type,
	short x, short y, short sizex, short sizey, bool clip_to_window = true );
void og_delete_button(orvgui_but *but);
void og_refresh_button(orvgui_but *but);

void og_move_button(orvgui_but *but, short x, short y);
void og_move_button_by( orvgui_but *but, short bx, short by );		// added by Pete
void og_resize_button(orvgui_but *but, short sizex, short sizey);
void og_rotate_button(orvgui_but *but, float angle);
void og_set_transparency_button(orvgui_but *but, int transparency);
void og_enable_button(orvgui_but *but);
void og_disable_button(orvgui_but *but);
void og_press_button(orvgui_but *but);
void og_unpress_button(orvgui_but *but);
void og_highlight_button(orvgui_but *but);
void og_unhighlight_button(orvgui_but *but);
void og_set_react_button(orvgui_but *but, unsigned int mask);
void og_set_pic_button(orvgui_but *but, IStorm3D_Material *pic, 
	IStorm3D_Material *picdown, IStorm3D_Material *picdisabled, IStorm3D_Material *pichighlighted);
void og_set_text_button(orvgui_but *but, IStorm3D_Font *fnt, const COL &fnt_color, unsigned char h_align, unsigned char v_align, const char *text, int pixwidth, int pixheight, int fontwidth, int fontheight);
void og_set_text_button(orvgui_but *but, const char *text, int pixwidth, int pixheight, int fontwidth, int fontheight);
void og_set_linebreaks_button(orvgui_but *but);
void og_set_nolinebreaks_button(orvgui_but *but);
void og_set_font_button(orvgui_but *but, IStorm3D_Font *fnt, const COL &fnt_color);
void og_set_fonts_button(orvgui_but *but, IStorm3D_Font *fnt, const COL &fnt_color, IStorm3D_Font *down, const COL &down_color, IStorm3D_Font *disa, const COL &disa_color, IStorm3D_Font *high, const COL &high_color );
void og_set_h_align_button(orvgui_but *but, unsigned char h_align);
void og_set_v_align_button(orvgui_but *but, unsigned char v_align);
void og_set_press_button(orvgui_but *but, void (*handler_proc)(void), void *handler_arg);
//void og_set_release_button(orvgui_but *but, void (*handler_proc)(void));
void og_set_click_button(orvgui_but *but, void (*handler_proc)(void), void *handler_arg);
void og_set_out_button(orvgui_but *but, void (*handler_proc)(void), void *handler_arg);
void og_set_over_button(orvgui_but *but, void (*handler_proc)(void), void *handler_arg);
void og_set_leave_button(orvgui_but *but, void (*handler_proc)(void), void *handler_arg);
void og_set_hold_button(orvgui_but *but, void (*handler_proc)(void), void *handler_arg);
//void og_set_on_button(orvgui_but *but, void (*handler_proc)(void));

orvgui_win *og_create_window(unsigned char type, short x, short y,
	short sizex, short sizey, IStorm3D_Material *bg_pic);
void og_delete_window(orvgui_win *win);
void og_refresh_window(orvgui_win *win);
void og_refresh_overlapping(orvgui_win *win);
void og_draw_window(orvgui_win *win);
void og_raise_window(orvgui_win *win);
void og_lower_window(orvgui_win *win);
void og_set_background_window(orvgui_win *win, IStorm3D_Material *bg_pic);
void og_set_transparency_window(orvgui_win *win, int transparency);

void og_move_window(orvgui_win *win, short x, short y);
void og_force_move_window(orvgui_win *win, short x, short y);
void og_resize_window(orvgui_win *win, short sizex, short sizey);
void og_hide_window(orvgui_win *win);
void og_show_window(orvgui_win *win);
void og_set_react_window(orvgui_win *win, unsigned char mask);
void og_set_bg_window(orvgui_win *win, IStorm3D_Material *bg);
void og_set_movable_window(orvgui_win *win, unsigned char movable);
void og_set_popup_window(orvgui_win *win, unsigned char popup);

// void og_write_text(orvgui_win *win, short x, short y, char *text);
void og_write_text_transp(orvgui_win *win, IStorm3D_Font *font, short x, short y, char *text, short fontwidth, short fontheight, float alpha = 1.0f, const COL &color = COL(1.f, 1.f, 1.f));
// void og_box(orvgui_win *win, short x, short y, short x2, short y2, COL col);
// void og_fbox(orvgui_win *win, short x, short y, short x2, short y2, COL col);
// void og_line(orvgui_win *win, short x, short y, short x2, short y2, COL col);

//color og_get_fg_color(void);
//color og_get_bg_color(void);
//color og_get_cursor_color(void);
//color og_get_selection_color(void);

int og_get_scr_size_x(void);
int og_get_scr_size_y(void);

void og_show_error(const char *msg);

#endif

