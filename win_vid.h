/* Digger Remastered
   Copyright (c) Andrew Jenner 1998-2004 */

#define DIRECTDRAW_VERSION 0x0300
#include <ddraw.h>

HRESULT restore_surface();
void init_directdraw();
HRESULT ChangeCoopLevel();
HRESULT release_directdraw_objects();
void toggle_screen_mode();
void attach_clipper();
void release_clipper();
extern LPDIRECTDRAWSURFACE g_pDDSPrimary;
extern int use_direct_draw;

void blit_rect_to_window(int x, int y, int w, int h);
HRESULT blit_to_window();
HRESULT init_surfaces();
HRESULT fatal_error(HRESULT hRet, LPCTSTR szError);
bool create_back_buffer(HDC window_dc);
void destroy_back_buffer();
void blit_to_window_dc(HWND wnd);
void init_graphics();
void display_title_bitmap(int i);
void load_title_bitmaps();

extern BOOL g_bActive;
extern BOOL g_bReady;
extern BOOL g_bWindowed;
extern RECT g_rcWindow;
extern RECT g_rcViewport;
extern RECT g_rcScreen;
HRESULT init_surfaces();
extern HBITMAP title_bitmap[2];
extern bool use_async_screen_updates;
extern int cur_intensity;
extern int cur_palette;
extern bool palettized_desktop;
extern HDC back_dc;
extern HBITMAP back_bitmap;
extern char *back_bitmap_bits;
extern HBITMAP old_bitmap;
extern bool use_640x480_fullscreen;

extern Uint3 *ascii2vga[];
extern Uint3 *ascii2cga[];
extern Uint3 *vgatable[];
extern Uint3 *cgatable[];
extern void outtext(char *p,Sint4 x,Sint4 y,Sint4 c);
extern bool synchvid;

enum video_mode_enum
{
  VIDEO_MODE_CGA=0,
  VIDEO_MODE_VGA_16=1,
  VIDEO_MODE_VGA_256=2
} video_mode;

extern HINSTANCE hDirectDrawInstance;
extern HRESULT (WINAPI *lpDirectDrawCreate)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );

void change_graphics_routines(Uint3 newmode);

struct FULLSCREEN_VID_MODE_INFO
{
  int width;
  int height;
  int bpp;
  struct FULLSCREEN_VID_MODE_INFO* next_mode;
};

HRESULT WINAPI EnumModesCallback(LPDDSURFACEDESC lpDDSurfaceDesc,LPVOID lpContext);
extern struct FULLSCREEN_VID_MODE_INFO* supported_video_modes;
void set_preferred_vidmode(int w, int h, int bpp);
extern struct FULLSCREEN_VID_MODE_INFO preferred_video_mode;

BOOL LoadSpriteFile(LPSTR pszFileName);

