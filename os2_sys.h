#define INCL_WININPUT // Window Input functions (and key codes)
#include <os2.h>

#include "def.h"
#include "resource.h"

#define ID_APP_WINDOW		1

#define SAVE TRUE

bool IsCharAlphaNumeric(UINT c);
UINT getkey_ch(void);

void do_windows_events(void);
void windows_finish(void);
void set_sound_volume(int new_volume);
void pause_windows_sound_playback();
void resume_windows_sound_playback();
bool get_open_save_filename(bool save, char* title, char* filter, char* defext, char filename[]);
bool key_pressed(int function,bool check_current_state_only);
int translate_key_code_to_function(int code);
Sint4 getcommand(void);
void clear_mapped_keys(int idx);
void add_mapped_key(int idx, int key_code);
bool kbhit(void);

#ifndef _OS2_SYS_C_
extern char drf_filename[FILENAME_BUFFER_SIZE];
extern int command_buffer;
extern char digger_dir[];   // contains the startup directory
extern bool wave_device_available;
extern bool suspend_game;
extern LONG dx_sound_volume;
extern bool start_full_screen;
extern BOOL g_bWindowed;
extern bool use_640x480_fullscreen;
extern bool use_async_screen_updates;
#endif
