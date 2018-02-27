/* Digger Remastered
   Copyright (c) Andrew Jenner 1998-2004 */

#ifdef TRUE
#undef TRUE
#endif

#ifdef _MSVC
#ifdef LIBC               /* linking with static library LIBC.LIB */
#define strupr _strupr
#define strnicmp _strnicmp
#define stricmp _stricmp
#define itoa _itoa
#endif
#endif

#include <windows.h>
#define DIGGER_WS_WINDOWED WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER
#define DIGGER_WS_FULLSCREEN WS_EX_TOPMOST
#define SAVE TRUE
#define OPEN FALSE

extern HWND hWnd;
extern SIZE window_size;
extern HINSTANCE g_hInstance;
extern bool shutting_down;
extern bool suspend_game;

void do_windows_events(void);
void windows_finish(void);
void refresh_menu_items(void);
void show_game_menu(void);
void show_main_menu(void);
void remove_menu(void);

void pause_windows_sound_playback(void);
void resume_windows_sound_playback(void);
void init_joystick();

void show_mouse_cursor();
void hide_mouse_cursor();

int do_dialog_box(HANDLE hInstance, LPCTSTR lpTemplate, HWND hWndParent,
                  DLGPROC lpDialogFunc);
bool get_open_save_filename(bool save, char* title, char* filter, char* defext, char filename[]);
HRESULT fatal_error(HRESULT hRet, LPCTSTR szError);
void load_level_file(char* fn);
void restore_original_level_data();

extern bool use_performance_counter;
extern _int64 performance_frequency;
extern void windows_init_sound();

void init_direct_x();
void release_direct_x();
extern bool check_for_direct_x;

extern HINSTANCE g_hInstance;
extern BOOL g_bActive;
extern bool reset_main_menu_screen;
extern char drf_filename[FILENAME_BUFFER_SIZE];

enum CFG_AUDIO_SETTING_OUTPUT_DEVICE_ENUM
{
  disabled,
  direct_sound,
  wave_out
};

extern HWND cur_dialog_box;
BOOL key_pressed(int function,BOOL check_current_state_only);
int translate_key_code_to_function(int code);
extern int command_buffer;
Sint4 getcommand(void);
BOOL cmdhit(void);
void add_mapped_key(int idx, int key_code);
extern char digger_dir[];   // contains the startup directory
