/* Digger Remastered
   Copyright (c) Andrew Jenner 1998-2004 */

#define INITGUID
#include "def.h"
#include <stdio.h>
#include <stdlib.h>
#include "win_dig.h"
#include <windowsx.h>
#include "resource.h"
#include "scores.h"
#include <commctrl.h>
#include "win_vid.h"
#include "win_snd.h"
#include "win_cfg.h"
#include "main.h"
#include "ini.h"
#include "hardware.h"
#include "sprite.h"
#include "record.h"
#include "scores.h"

extern void finish(void);
extern int keycodes[17][5];
extern void redefkeyb(bool allf);
extern void parsecmd(int argc,char *argv[]);

extern bool joyflag;

/* these are used by the menu/dialogue boxes */
extern Sint4 nplayers,diggers,curplayer,startlev;
extern bool start;
extern void shownplayers(void);
extern bool gauntlet;
extern int gtime;
extern bool soundflag,musicflag;
extern bool pausef;
extern Uint5 ftime;
extern bool timeout;
extern Uint4 size;
extern bool soundlevdoneflag;
extern bool escape;
extern Sint4 frame;
extern bool started;

HINSTANCE hDirectDrawInstance=NULL;
HINSTANCE hDirectSoundInstance=NULL;
HRESULT (WINAPI *lpDirectSoundCreate)(LPGUID, LPDIRECTSOUND *, LPUNKNOWN);
HRESULT (WINAPI *lpDirectDrawCreate)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );
bool check_for_direct_x=TRUE;

// This will store a copy of the original level data so that the user can return to using the original set of level, after loading one of the DLF files
Sint3 orig_leveldat[8][MHEIGHT][MWIDTH];
Uint4 orig_bonusscore;

// This will store a copy of the 'current' level data so that the current level data is not lost when a DRF is played
Sint3 backup_leveldat[8][MHEIGHT][MWIDTH];
Uint4 backup_bonusscore;
char backup_levfname[FILENAME_BUFFER_SIZE];
bool backup_levfflag;

char drf_filename[FILENAME_BUFFER_SIZE]="";
int use_direct_input=0;

bool suspend_game=FALSE;
HWND hWnd;
SIZE window_size;
HINSTANCE g_hInstance;
bool reset_main_menu_screen=FALSE;
bool use_performance_counter;
_int64 performance_frequency;
int kb_buffer=0;
int command_buffer=0;

HWND cur_dialog_box=(HWND) NULL;
char* params[10];
int param_count;
bool main_function_started=FALSE;   /* has main() been called yet? */
bool shutting_down=FALSE;
HMENU hMenu;
char digger_dir[256];

LRESULT CALLBACK gauntlet_settings_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK sound_settings_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK help_about_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK levels_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* time routines */
void olddelay(Sint4 t) { };
Sint5 getkips(void) { return 0; };
void inittimer(void){ };
Uint5 gethrt(void)
{
  return (Uint5) ((DWORD) GetTickCount() * 1193l);
//  return (Uint5) ((DWORD) GetTickCount());
};

Sint5 getlrt(void)
{
  return gethrt();
};

Sint4 getkey(void)  // returns the Virtual Key Code of the last key pressed
{
  Sint4 temp_buffer;
  do
  {
    if (kb_buffer)
    {
      temp_buffer=kb_buffer;
      kb_buffer=0;
      return temp_buffer;
    }
    else
      do_windows_events();
  } while (TRUE);

};

Sint4 getcommand(void)   // similar to getkey() but the value returned has already been translated from the keycode to the function/command that the key is mapped to
{
  Sint4 temp_buffer;

  temp_buffer=command_buffer;
  command_buffer=0;
  return temp_buffer;
};
bool kbhit(void)
{
  return (kb_buffer!=0);
};
bool cmdhit(void)
{
  return (command_buffer);
}

void initkeyb(void) {};
void restorekeyb(void) {};


void clear_nplayerlines()
{
  outtext("          ",180,25,3);
  outtext("            ",170,39,3);
}

void refresh_menu_items()
{
  /* set checkmarks, etc. */
  HMENU cur_menu;

  cur_menu=GetMenu(hWnd);
  CheckMenuItem(cur_menu, ID_GAME_PLAYERS_ONE, MF_BYCOMMAND | (diggers>1||nplayers>1 ? MF_UNCHECKED : MF_CHECKED));
  CheckMenuItem(cur_menu, ID_GAME_PLAYERS_TWO, MF_BYCOMMAND | (diggers>1||nplayers!=2 ? MF_UNCHECKED : MF_CHECKED));
  CheckMenuItem(cur_menu, ID_GAME_PLAYERS_TWOSIMULTANEOUS, MF_BYCOMMAND | (diggers!=2 ? MF_UNCHECKED : MF_CHECKED));
  CheckMenuItem(cur_menu, ID_GAME_MODE_NORMAL, MF_BYCOMMAND | (gauntlet ? MF_UNCHECKED : MF_CHECKED));
  CheckMenuItem(cur_menu, ID_GAME_MODE_GAUNTLET, MF_BYCOMMAND | (!gauntlet ? MF_UNCHECKED : MF_CHECKED));
  CheckMenuItem(cur_menu, ID_SOUND_SOUNDS_PLAY, MF_BYCOMMAND | (!soundflag ? MF_UNCHECKED : MF_CHECKED));
  CheckMenuItem(cur_menu, ID_SOUND_MUSIC_PLAY, MF_BYCOMMAND | (!musicflag ? MF_UNCHECKED : MF_CHECKED));
  CheckMenuItem(cur_menu, ID_CONFIGURATION_GRAPHICS_ASYNCHRONOUS, MF_BYCOMMAND | (!use_async_screen_updates ? MF_UNCHECKED : MF_CHECKED));
  CheckMenuItem(cur_menu, ID_CONFIGURATION_SPEED_DEFAULT, MF_BYCOMMAND | (ftime!=80000l ? MF_UNCHECKED : MF_CHECKED));
  CheckMenuItem(cur_menu, ID_VIEW_FULLSCREEN, MF_BYCOMMAND | (g_bWindowed ? MF_UNCHECKED : MF_CHECKED));
  EnableMenuItem(cur_menu, ID_VIEW_FULLSCREEN, MF_BYCOMMAND | (use_direct_draw /*==6*/ ? MF_ENABLED : MF_GRAYED));
  CheckMenuItem(cur_menu, ID_VIEW_VGAGRAPHICS, MF_BYCOMMAND | (video_mode==VIDEO_MODE_VGA_16 ? MF_CHECKED : MF_UNCHECKED));
  CheckMenuItem(cur_menu, ID_VIEW_CGAGRAPHICS, MF_BYCOMMAND | (video_mode==VIDEO_MODE_CGA ? MF_CHECKED : MF_UNCHECKED));
  EnableMenuItem(cur_menu, ID_RECORDING_SAVE, MF_BYCOMMAND | (gotgame && drfvalid) ? MF_ENABLED : MF_GRAYED);
  EnableMenuItem(cur_menu, ID_RECORDING_INSTANTREPLAY, MF_BYCOMMAND | (gotgame && drfvalid) ? MF_ENABLED : MF_GRAYED);
}

void show_game_menu()
{
 /* Display the menu which should be displayed when a game is in progress */
  HMENU main_menu;
  HMENU cur_menu;

  main_menu = GetMenu(hWnd);
  EnableMenuItem(main_menu, ID_GAME_ABORT, MF_BYCOMMAND | MF_ENABLED);
  EnableMenuItem(main_menu, ID_GAME_PAUSE, MF_BYCOMMAND | MF_ENABLED);
  EnableMenuItem(main_menu, ID_GAME_START, MF_BYCOMMAND | MF_GRAYED);
  EnableMenuItem(main_menu, ID_VIEW_VGAGRAPHICS, MF_BYCOMMAND | MF_GRAYED);
  EnableMenuItem(main_menu, ID_VIEW_CGAGRAPHICS, MF_BYCOMMAND | MF_GRAYED);
  EnableMenuItem(main_menu, ID_CONFIGURATION_CONTROLS_KEYBOARD_REDEFINEKEYS, MF_BYCOMMAND | MF_GRAYED);
  EnableMenuItem(main_menu, ID_CONFIGURATION_CONTROLS_KEYBOARD_REDEFINEALLKEYS, MF_BYCOMMAND | MF_GRAYED);
  cur_menu = GetSubMenu(main_menu, 0);
  EnableMenuItem(cur_menu, 5, MF_BYPOSITION | MF_GRAYED); // MODE>
  EnableMenuItem(cur_menu, 6, MF_BYPOSITION | MF_GRAYED); // PLAYERS>
  EnableMenuItem(cur_menu, 7, MF_BYPOSITION | MF_GRAYED); // LEVELS...
  EnableMenuItem(cur_menu, 9, MF_BYPOSITION | MF_GRAYED); // Recording>
}

void show_main_menu()
{
  /* Display menu for title/high score screen */
  HMENU main_menu;
  HMENU cur_menu;

  main_menu=GetMenu(hWnd);
  EnableMenuItem(main_menu, ID_GAME_ABORT, MF_BYCOMMAND | MF_GRAYED);
  EnableMenuItem(main_menu, ID_GAME_PAUSE, MF_BYCOMMAND | MF_GRAYED);
  EnableMenuItem(main_menu, ID_GAME_START, MF_BYCOMMAND | MF_ENABLED);
  EnableMenuItem(main_menu, ID_VIEW_VGAGRAPHICS, MF_BYCOMMAND | MF_ENABLED);
  EnableMenuItem(main_menu, ID_VIEW_CGAGRAPHICS, MF_BYCOMMAND | MF_ENABLED);
  EnableMenuItem(main_menu, ID_RECORDING_PLAY, MF_BYCOMMAND | MF_ENABLED);
  EnableMenuItem(main_menu, ID_CONFIGURATION_CONTROLS_KEYBOARD_REDEFINEKEYS, MF_BYCOMMAND | MF_ENABLED);
  EnableMenuItem(main_menu, ID_CONFIGURATION_CONTROLS_KEYBOARD_REDEFINEALLKEYS, MF_BYCOMMAND | MF_ENABLED);
  cur_menu=GetSubMenu(main_menu,0);
  EnableMenuItem(cur_menu, 5, MF_BYPOSITION | MF_ENABLED); // MODE>
  EnableMenuItem(cur_menu, 6, MF_BYPOSITION | MF_ENABLED); // PLAYERS>
  EnableMenuItem(cur_menu, 7, MF_BYPOSITION | MF_ENABLED); // LEVEL...
  EnableMenuItem(cur_menu, 9, MF_BYPOSITION | MF_ENABLED); // Recordings>
  cur_menu=GetSubMenu(main_menu, 1);
}

void refresh_screen_info()
{
  /* called whenever the user switches the number of players or the game type */
  loadscores();
  showtable();
  clear_nplayerlines();
  shownplayers();
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  HRESULT hRet;
  BYTE clear_keyboard_state[256];

  switch(message)
  {
  case WM_KEYDOWN:
    //if (wParam!=VK_SHIFT && wParam!=VK_CONTROL)
    //{
      kb_buffer=wParam;
    //}
    if (wParam==VK_MENU && !g_bWindowed)
      SetMenu(hWnd,hMenu);
    return 0;

  case WM_SYSKEYDOWN:
    switch (wParam)
    {
/*
    case VK_RETURN:
      if (use_direct_draw)
        toggle_screen_mode();
      return 0;
*/
    case VK_MENU:
      if (!g_bWindowed)
        SetMenu(hWnd,hMenu);
    }
    break;

  case WM_SYSKEYUP:
    switch (wParam)
    {
    case VK_F10:
      kb_buffer=VK_F10;
      return 0;
    }
    break;

  /************************************/
  /* menu commands                    */
  /************************************/
  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case VK_F10:
      kb_buffer=VK_F10;
    case ID_GAME_EXIT:
      SendMessage(hWnd, WM_CLOSE, 0, 0L);
      break;
    case ID_VIEW_FULLSCREEN:
      toggle_screen_mode();
      break;
    case ID_GAME_PLAYERS_ONE:
      nplayers=1;
      diggers=1;
      refresh_screen_info();
      break;
    case ID_GAME_PLAYERS_TWO:
      nplayers=2;
      diggers=1;
      gauntlet=FALSE;
      refresh_screen_info();
      break;
    case ID_GAME_PLAYERS_TWOSIMULTANEOUS:
      nplayers=1;
      diggers=2;
      refresh_screen_info();
      break;
    case ID_GAME_MODE_NORMAL:
      gauntlet=FALSE;
      timeout=FALSE;  /* must do this in case the previous game was a guantlet game */
      refresh_screen_info();
      break;
    case ID_GAME_MODE_GAUNTLET:
      gauntlet=TRUE;
      nplayers=1;
      if (!gtime)
        gtime=120;
      refresh_screen_info();
      if (do_dialog_box(g_hInstance,
                    MAKEINTRESOURCE(IDD_DIALOG_GAUNTLET_SETTINGS),
                    hWnd, (DLGPROC) gauntlet_settings_dialog_proc))
        WriteINIInt(INI_GAME_SETTINGS, "GauntletTime", gtime, ININAME);
      break;
    case ID_GAME_LEVEL:
      if (do_dialog_box(g_hInstance,
                    MAKEINTRESOURCE(IDD_DIALOG_LEVELS),
                    hWnd, (DLGPROC) levels_dialog_proc))
      {
        startlev=startlev;
      }
      break;
    case ID_GAME_START:
      start=TRUE;
      break;
    case ID_GAME_ABORT:
      kb_buffer=0;
      escape=TRUE;
      break;
    case ID_GAME_PAUSE:
      if (!pausef)
        pausef=TRUE;
      else
        kb_buffer=1;  // any key will cause game to continue
      //kb_buffer=keycodes[16][0];
      break;
    case ID_SOUND_SOUNDS_PLAY:
      soundflag=!soundflag;
      soundlevdoneflag=FALSE;
      break;
    case ID_SOUND_MUSIC_PLAY:
      musicflag=!musicflag;
      break;
    case ID_CONFIGURATION_SPEED_DEFAULT:
      ftime=80000l;
      break;
    case ID_SPEED_FASTER:
      if (ftime>10000l)
        ftime-=10000l;
      break;
    case ID_SPEED_SLOWER:
      ftime+=10000l;
      break;
    case ID_SOUND_SETTINGS:
      if (do_dialog_box(g_hInstance,
                    MAKEINTRESOURCE(IDD_DIALOG_SOUND_SETTINGS),
                    hWnd, (DLGPROC) sound_settings_dialog_proc))
      {
        WriteINIBool(INI_SOUND_SETTINGS, "SoundOn", soundflag, ININAME);
        WriteINIInt(INI_SOUND_SETTINGS, "SoundVolume", get_sound_volume(), ININAME);
      }
      break;
    case ID_HELP_ABOUTDIGGER:
      do_dialog_box(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG_ABOUT), hWnd,
                    (DLGPROC) help_about_dialog_proc);
      break;
    case ID_CONFIGURATION_GRAPHICS_ASYNCHRONOUS:
      use_async_screen_updates=!use_async_screen_updates;
      break;
    case ID_CONFIGURATION_CONTROLS_KEYBOARD_REDEFINEKEYS:
        gclear();
        redefkeyb(FALSE);
        reset_main_menu_screen=TRUE;
        break;
    case ID_CONFIGURATION_CONTROLS_KEYBOARD_REDEFINEALLKEYS:
        gclear();
        redefkeyb(TRUE);
        reset_main_menu_screen=TRUE;
      break;
    case ID_CONFIGURATION_SHOWSETTINGSDIALOG:
      if (started)
        pausef=TRUE;
      create_config_window();
      break;
    case ID_VIEW_VGAGRAPHICS:
      change_graphics_routines(VIDEO_MODE_VGA_16);
      break;

    case ID_VIEW_CGAGRAPHICS:
      change_graphics_routines(VIDEO_MODE_CGA);
      break;
    case ID_RECORDING_PLAY:
      if (get_open_save_filename(OPEN, "Open Recorded Game","Digger Record Files (*.drf)\0*.DRF\0All Files (*.*)\0*.*\0","DRF", drf_filename))
      {
        // make a copy of the current level info, so that it can be restored after the DLF playback is finished
        memcpy(backup_leveldat,leveldat,8*MHEIGHT*MWIDTH);
        strcpy(backup_levfname,levfname);
        backup_bonusscore=bonusscore;
        backup_levfflag=levfflag;

        started=FALSE;
        openplay(drf_filename);
        started=FALSE;

        // restore level data after DLF playback is finished
        memcpy(leveldat,backup_leveldat,8*MHEIGHT*MWIDTH);
        strcpy(levfname,backup_levfname);
        bonusscore=backup_bonusscore;
        levfflag=backup_levfflag;

        reset_main_menu_screen=TRUE;
      }
      break;
    case ID_RECORDING_SAVE:
      if (gotgame)
        recsavedrf();
      break;
    case ID_RECORDING_INSTANTREPLAY:
      if (gotgame)
      {
        drf_filename[0]='\0';
        recsavedrf();
        if (drf_filename[0]!='\0')
        {
          started=FALSE;
          openplay(drf_filename);
          started=FALSE;
          reset_main_menu_screen=TRUE;
        }
      }
      break;
    }
    return 0;

  case WM_MENUSELECT:
    refresh_menu_items();   /* it isn't neccessary to do this for every WM_MENUSELECT...  Should only do this when the menu is first activated. */
    pause_windows_sound_playback();
    if (!g_bWindowed)
    {
      if (!(((UINT) HIWORD(wParam))==0xFFFF && lParam==(LPARAM) NULL ))
      {
        if (!GetCursor())
        {
          if (!g_bWindowed)
            IDirectDrawSurface_SetPalette(g_pDDSPrimary, NULL);  // restore default palette, so that the colors of menus and dialog boxes are correct.
          SetMenu(hWnd, GetMenu(hWnd));
          show_mouse_cursor();
        }
        break;
      }
      else
      {
        if (preferred_video_mode.height<=400)
          SetMenu(hWnd,NULL);
        hide_mouse_cursor();
        resume_windows_sound_playback();
        if (!g_bWindowed)
          gpal(cur_palette);
      /* repaint the window once the menu is deactivated */
      }
    }
    else
      break;

  case WM_PAINT:
    if (g_bReady)
    {
      if (g_bWindowed || preferred_video_mode.bpp > 8)
      {
        blit_to_window_dc(hWnd);
      }
      else
      {
        while (TRUE)
        {
          hRet = blit_to_window();
          if (hRet == DDERR_SURFACELOST)
          {
            hRet = restore_surface();
          }
          else
            if (hRet != DDERR_WASSTILLDRAWING)
            {
              if (hRet==DD_OK)
                if (cur_dialog_box!=NULL)
                {
                  InvalidateRect(cur_dialog_box, NULL, TRUE);
                  UpdateWindow(cur_dialog_box);
                }
              break;
            }
        }
      }
    }
    break;

  case WM_MOVE:
      if (g_bActive && g_bReady && g_bWindowed)
      {
        GetWindowRect(hWnd, &g_rcWindow);
        GetClientRect(hWnd, &g_rcViewport);
        GetClientRect(hWnd, &g_rcScreen);
        ClientToScreen(hWnd, (POINT*)&g_rcScreen.left);
        ClientToScreen(hWnd, (POINT*)&g_rcScreen.right);
      }
      break;

  case WM_SIZE:
    window_size.cx=LOWORD(lParam);
    window_size.cy=HIWORD(lParam);
    InvalidateRect(hWnd, NULL, FALSE);

    if (SIZE_MAXHIDE==wParam || SIZE_MINIMIZED==wParam)
        g_bActive = FALSE;
    else
        g_bActive = TRUE;
    break;

  case WM_SETCURSOR:
    /* Display the cursor in the window if windowed */
    if (g_bActive && g_bReady && !g_bWindowed)
    {
        SetCursor((HCURSOR) NULL);
        return TRUE;
    }
    break;

  case WM_CLOSE:
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_POWERBROADCAST:
    switch (wParam)
    {
    case PBT_APMQUERYSTANDBY:
    case PBT_APMQUERYSUSPEND:
      return BROADCAST_QUERY_DENY;
    }

  }
  return DefWindowProc(hWnd, message, wParam, lParam);

}

/* This creates the main window, then calls another function to create  */
/* the back buffer that is to be used, and set up other graphics stuff. */
bool create_window(HINSTANCE hInstance, int nShowCmd)
{
  WNDCLASS wndClass;
  int width;
  int height;

  width = 640 + GetSystemMetrics(SM_CXBORDER) * 2;
  width+= GetSystemMetrics(SM_CXEDGE)*2;

  height = 400 + GetSystemMetrics(SM_CYBORDER) * 2 + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU);
  height += GetSystemMetrics(SM_CYEDGE)*2;

  wndClass.style = CS_HREDRAW | CS_VREDRAW;
  wndClass.lpfnWndProc = WndProc;
  wndClass.cbClsExtra = 0;
  wndClass.cbWndExtra = 0;
  wndClass.hInstance = hInstance;
  wndClass.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_ICON1));
  wndClass.hCursor = LoadCursor ((HINSTANCE) NULL, IDC_ARROW);
  wndClass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
  wndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
  wndClass.lpszClassName =  "Digger";
  RegisterClass(&wndClass);
  hWnd = CreateWindowEx(
    WS_EX_WINDOWEDGE,
    "Digger",
    "Digger",
    DIGGER_WS_WINDOWED,
    (GetSystemMetrics(SM_CXFULLSCREEN) - 640) / 2,
    (GetSystemMetrics(SM_CYFULLSCREEN) - 400) / 2,
    width,
    height,
    (HWND) NULL,
    (HMENU) NULL,
    (HINSTANCE) hInstance,
    (void FAR*) NULL );

  window_size.cx=0;
  window_size.cy=0;
  ShowWindow(hWnd, nShowCmd);
  GetWindowRect(hWnd, &g_rcWindow);
  GetClientRect(hWnd, &g_rcViewport);
  GetClientRect(hWnd, &g_rcScreen);
  ClientToScreen(hWnd, (POINT*)&g_rcScreen.left);
  ClientToScreen(hWnd, (POINT*)&g_rcScreen.right);
  UpdateWindow(hWnd);

  return TRUE; /* TO DO - add error checking */
}

void windows_finish()
{
  int i;

  shutting_down=TRUE;

  /* save the current settings */
  WriteINIBool(INI_SOUND_SETTINGS, "SoundOn", soundflag, ININAME);
  WriteINIBool(INI_SOUND_SETTINGS, "MusicOn", musicflag, ININAME);
  WriteINIBool(INI_GRAPHICS_SETTINGS, "FullScreen", !g_bWindowed, ININAME);
  WriteINIBool(INI_GRAPHICS_SETTINGS, "Async", use_async_screen_updates, ININAME);
  if (diggers>1)
    WriteINIString(INI_GAME_SETTINGS, "Players", "2S", ININAME);
  else
    WriteINIInt(INI_GAME_SETTINGS, "Players", nplayers, ININAME);
  WriteINIBool(INI_GAME_SETTINGS, "GauntletMode", gauntlet, ININAME);
  WriteINIInt(INI_GAME_SETTINGS, "Speed", ftime, ININAME);
  WriteINIBool(INI_GRAPHICS_SETTINGS, "CGA", (video_mode==VIDEO_MODE_CGA), ININAME);
  WriteINIInt(INI_GAME_SETTINGS,"StartLevel", startlev, ININAME);
  WriteINIString(INI_GAME_SETTINGS,"LevelFile", levfname, ININAME);

  release_sound_card(); /* see if this solves Brandon's NT problem */

  /* destroy variables */
  destroy_back_buffer();
  destroy_sound_buffers();
  for (i=0;i<param_count;i++)
    free(params[i]);
  release_direct_x();
  if (title_bitmap[0])
    DeleteObject(title_bitmap[0]);
  if (title_bitmap[1])
    DeleteObject(title_bitmap[1]);
}

int WINAPI WinMain(HINSTANCE  hInstance, HINSTANCE hPrevInstance, LPSTR  lpCmdLine, int  nShowCmd)
{
  g_hInstance=hInstance;

  memcpy(orig_leveldat,leveldat,8*MHEIGHT*MWIDTH);  // Make a backup copy of the original level data so
  orig_bonusscore=bonusscore;

  GetCurrentDirectory(255,digger_dir);
  if (strlen(digger_dir)>0)
  {
    if (digger_dir[strlen(digger_dir)-1]!="\\")
      strcat(digger_dir,"\\");
  }

  check_for_direct_x = !GetINIBool("TROUBLESHOOTING","NoDirectX", FALSE, ININAME);
  init_direct_x();

  //LoadSpriteFile("vga.spr");
  InitCommonControls();
  wave_device_available=FALSE;
  hMenu=LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAINMENU));
  create_window(hInstance, nShowCmd);
  init_graphics();
  load_title_bitmaps();
  main_function_started=TRUE;
  maininit();
  g_bReady = TRUE;
  g_bActive = TRUE;
  mainprog();
  return 0;
}

void toggle_screen_mode()
{
  //if (g_bActive && g_bReady)
  //{
    //g_bReady = FALSE;
    g_bWindowed = !g_bWindowed;

    pause_windows_sound_playback();
    ChangeCoopLevel();
    resume_windows_sound_playback();
    g_bReady = TRUE;
    if (g_bWindowed)
      SetMenu(hWnd,hMenu);
    //if (g_bWindowed)
    //  GetWindowRect(hWnd, &g_rcWindow);
  //}
  //InvalidateRect(hWnd,NULL,TRUE);
  //UpdateWindow(hWnd);
}

/* This function is called from various places in Digger to allow Windows */
/* to perform its tasks.                                                  */
void do_windows_events()
{
  MSG msg;
  //if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD ))
  //if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE | PM_NOYIELD ))
  //do
  //{
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE /*| PM_NOYIELD */))
    {
      if (g_hDlg)
        if (IsDialogMessage(g_hDlg,&msg))
          return;
      if (msg.message == WM_QUIT)
      {
        if (main_function_started)
          finish();
        else
          windows_finish();
        exit(0);
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  //}
  //while (suspend_game);
}

int do_dialog_box(HANDLE hInstance, LPCTSTR lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc)
{
  int result;

  pause_windows_sound_playback();
  if (!g_bWindowed)
  {
    attach_clipper();
    IDirectDrawSurface_SetPalette(g_pDDSPrimary, NULL);
  }
  SetMenu(hWnd, GetMenu(hWnd));
  show_mouse_cursor();
  result=DialogBox(hInstance, lpTemplate, hWndParent, lpDialogFunc);
  cur_dialog_box=(HWND) NULL;
  hide_mouse_cursor();
  if (!g_bWindowed)
  {
    release_clipper();
    gpal(cur_palette);
  }
  resume_windows_sound_playback();
  InvalidateRect(NULL, NULL, FALSE);
  //UpdateWindow(hWnd);
  return result;
}

void show_mouse_cursor()
{
  POINT cursor_pos;

  SetCursor(LoadCursor((HINSTANCE) NULL, (LPCSTR) IDC_ARROW));
  GetCursorPos(&cursor_pos);
  SetCursorPos(cursor_pos.x-1, cursor_pos.y-1);
  SetCursorPos(cursor_pos.x, cursor_pos.y);
}

void hide_mouse_cursor()
{
  SetCursor((HCURSOR) NULL);
}

LRESULT CALLBACK gauntlet_settings_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  BOOL success;
  UINT rval;

  switch (uMsg)
  {
  case WM_INITDIALOG:
    cur_dialog_box=hDlg;
    SetDlgItemInt(hDlg, IDC_EDIT_TIME, gtime, FALSE);
    return TRUE;

  case WM_SYSCOMMAND:
    switch (wParam)
    {
    case SC_CLOSE:
      EndDialog(hDlg, FALSE);
      return TRUE;
    }
    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDCANCEL:
      EndDialog(hDlg, FALSE);
      return TRUE;
    case IDOK:
      rval=GetDlgItemInt(hDlg, IDC_EDIT_TIME, &success, FALSE);
      if (success)
        gtime=rval;
      if (gtime>3599)
        gtime=3599;
      if (gtime==0)
        gtime=120;
      EndDialog(hDlg, TRUE);
      return TRUE;
    }
    break;
  }
  return FALSE;
}


LRESULT CALLBACK sound_settings_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  LONG cur_volume;
  LONG new_val;
  BOOL success;
  HWND control;

  switch (uMsg)
  {
  case WM_INITDIALOG:
    cur_dialog_box=hDlg;
    cur_volume=get_sound_volume();
  control = GetDlgItem(hDlg, IDC_SCROLLBAR_SOUND_VOLUME);
    SetScrollRange(control, SB_CTL, 0, 100, TRUE);
    SetScrollPos(control, SB_CTL, (100 - cur_volume), TRUE);
    EnableScrollBar(control, SB_CTL, ESB_ENABLE_BOTH);
    CheckDlgButton(hDlg, IDC_CHECK_PLAY_SOUNDS, soundflag ? 1 : 0);
    CheckDlgButton(hDlg, IDC_CHECK_PLAY_MUSIC, musicflag ? 1 : 0);
    SetDlgItemInt(hDlg, IDC_EDIT_BUFFER_SIZE, (int) (size / 2), FALSE);
    SetDlgItemInt(hDlg, IDC_EDIT_SAMPLE_RATE, get_sound_freq(), FALSE);
    return TRUE;

  case WM_SYSCOMMAND:
    switch (wParam)
    {
    case SC_CLOSE:
      EndDialog(hDlg, FALSE);
      return TRUE;
    }
    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDCANCEL:
      EndDialog(hDlg, FALSE);
      return TRUE;
    case IDOK:
      new_val = GetScrollPos(GetDlgItem(hDlg, IDC_SCROLLBAR_SOUND_VOLUME), SB_CTL);
      set_sound_volume(100-new_val);

      soundflag = IsDlgButtonChecked(hDlg, IDC_CHECK_PLAY_SOUNDS);
      musicflag = IsDlgButtonChecked(hDlg, IDC_CHECK_PLAY_MUSIC);

      soundlevdoneflag=FALSE;   /* prevent game from locking up if you toggle sound (using menu/dialog box) while the 'level-done' tune is playing */

      new_val=GetDlgItemInt(hDlg, IDC_EDIT_BUFFER_SIZE, &success, FALSE);
      if (new_val>0)
        WriteINIInt(INI_SOUND_SETTINGS, "BufferSize", new_val, ININAME);
      new_val=GetDlgItemInt(hDlg, IDC_EDIT_SAMPLE_RATE, &success, FALSE);
      if (new_val>0)
        WriteINIInt(INI_SOUND_SETTINGS, "Rate", new_val, ININAME);
      EndDialog(hDlg, TRUE);
      return TRUE;
    }
    break;

  case WM_VSCROLL:
    control = (HWND) lParam;
    switch ((int) LOWORD(wParam))
    {
    case SB_THUMBPOSITION:
      SetScrollPos(control, SB_CTL, (short int) HIWORD(wParam), TRUE);
      break;
    case SB_LINEUP:
      new_val=GetScrollPos(control, SB_CTL) - 1;
      SetScrollPos(control, SB_CTL,new_val, TRUE);
      break;
    case SB_LINEDOWN:
      new_val=GetScrollPos(control, SB_CTL) + 1;
      SetScrollPos(control, SB_CTL,new_val, TRUE);
      break;
    case SB_PAGEUP:
      new_val=GetScrollPos(control, SB_CTL) - 10;
      SetScrollPos(control, SB_CTL,new_val, TRUE);
      break;
    case SB_PAGEDOWN:
      new_val=GetScrollPos(control, SB_CTL) + 10;
      SetScrollPos(control, SB_CTL,new_val, TRUE);
      break;
    case SB_TOP:
      SetScrollPos(control, SB_CTL,0, TRUE);
      break;
    case SB_BOTTOM:
      SetScrollPos(control, SB_CTL,100, TRUE);
      break;
    }
    return TRUE;
  }
  return FALSE;
}

LRESULT CALLBACK help_about_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  char info_string[200];
  switch (uMsg)
  {
  case WM_INITDIALOG:
    cur_dialog_box=hDlg;
    strcpy(info_string, DIGGER_VERSION);
    strcat(info_string, "\n");
    strcat(info_string, "Win32 version (DirectX optional)");
    SetDlgItemText(hDlg, IDC_STATIC_VERSION, info_string);
    if (g_bWindowed)
    {
      strcpy(info_string, "Running in Windowed Mode\n");
      strcat(info_string,palettized_desktop ? "(256 Colors/Using Palette)\n" : "(HighColor/TrueColor)\n");
    }
    else
    {
      strcpy(info_string, "Running in Full Screen Mode\n");
    }
    strcat(info_string, hDirectDrawInstance ? "DirectDraw detected\n" : "DirectDraw NOT detected\n");
    if (!use_direct_draw)
      strcat(info_string, "Not using DirectDraw");
    else if (use_direct_draw)
      strcat(info_string, "Using DirectDraw");
    SetDlgItemText(hDlg, IDC_GRAPHICS_INFO, info_string);
    strcpy(info_string, wave_device_available ? "Available\n" : "Unavailable\n");
    strcat(info_string, hDirectSoundInstance ? "DirectSound detected" : "DirectSound NOT detected");
    SetDlgItemText(hDlg, IDC_SOUND_INFO, info_string);
    return TRUE;

  case WM_SYSCOMMAND:
    switch (wParam)
    {
    case SC_CLOSE:
      EndDialog(hDlg, FALSE);
      return TRUE;
    }
    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDOK:
      EndDialog(hDlg, TRUE);
      return TRUE;
    }
    break;
  }
  return FALSE;
}

void init_direct_x()
{
  if (!check_for_direct_x)
    return;

  hDirectDrawInstance = LoadLibrary("ddraw.dll");
  hDirectSoundInstance = LoadLibrary("dsound.dll");
  if (hDirectDrawInstance)
  {
    lpDirectDrawCreate = (long (__stdcall *)(struct _GUID *,struct IDirectDraw ** ,struct IUnknown *)) GetProcAddress(hDirectDrawInstance,"DirectDrawCreate");
    if (lpDirectDrawCreate)
      use_direct_draw=0;
  }
  else
    use_direct_draw=0;

  if (hDirectSoundInstance)
  {
    lpDirectSoundCreate = (long (__stdcall *)(struct _GUID *,struct IDirectSound ** ,struct IUnknown *)) GetProcAddress(hDirectSoundInstance,"DirectSoundCreate");
  }
}

void release_direct_x()
{
  if (hDirectDrawInstance)
    FreeLibrary(hDirectDrawInstance);
  if (hDirectSoundInstance)
    FreeLibrary(hDirectSoundInstance);
}

HRESULT WINAPI DirectDrawCreate( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter )
{
  return (lpDirectDrawCreate)(lpGUID, lplpDD, pUnkOuter);
}

HRESULT WINAPI DirectSoundCreate(LPGUID lpGUID, LPDIRECTSOUND * lplpDS, LPUNKNOWN pUnkOuter)
{
  return (lpDirectSoundCreate)(lpGUID, lplpDS, pUnkOuter);
}

LRESULT CALLBACK levels_dialog_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  BOOL success;
  UINT rval;
  HWND control;
  int i;
  char str[4];
  char dlf_filename[FILENAME_BUFFER_SIZE];

  switch (uMsg)
  {
  case WM_INITDIALOG:
    cur_dialog_box=hDlg;
    SetDlgItemInt(hDlg, IDC_START_LEVEL, startlev, FALSE);
    control = GetDlgItem(hDlg, IDC_START_LEVEL);
    for (i=1;i<16;i++)
    {
      ComboBox_AddString(control,itoa(i,str,10));
    }
    CheckRadioButton(hDlg,IDC_RADIO_USE_BUILT_IN, IDC_RADIO_USE_EXTERNAL, levfflag ? IDC_RADIO_USE_EXTERNAL : IDC_RADIO_USE_BUILT_IN);
    control = GetDlgItem(hDlg, IDC_EDIT_FILENAME);
    Edit_Enable(control, levfflag);
    Edit_SetText(control, levfname);
    return TRUE;

  case WM_SYSCOMMAND:
    switch (wParam)
    {
    case SC_CLOSE:
      EndDialog(hDlg, FALSE);
      return TRUE;
    }
    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDCANCEL:
      EndDialog(hDlg, FALSE);
      return TRUE;
    case IDOK:
      rval=GetDlgItemInt(hDlg, IDC_START_LEVEL, &success, FALSE);
      if (rval>0)
        startlev=rval;
      if (IsDlgButtonChecked(hDlg,IDC_RADIO_USE_EXTERNAL))
      {
        control = GetDlgItem(hDlg, IDC_EDIT_FILENAME);
        Edit_GetText(control,dlf_filename,FILENAME_BUFFER_SIZE);
        load_level_file(dlf_filename);
      }
      else
      {
        restore_original_level_data();
      }
      refresh_screen_info();
      EndDialog(hDlg, TRUE);
      return TRUE;
    case IDC_BROWSE_FILENAME:
      CheckRadioButton(hDlg,IDC_RADIO_USE_BUILT_IN, IDC_RADIO_USE_EXTERNAL, IDC_RADIO_USE_EXTERNAL);
      control = GetDlgItem(hDlg, IDC_EDIT_FILENAME);
      Edit_GetText(control,dlf_filename,FILENAME_BUFFER_SIZE);
      if (get_open_save_filename(OPEN,"Load Extra Levels","Digger Level Files (*.dlf)\0*.DLF\0All Files (*.*)\0*.*\0","DLF", dlf_filename))
        Edit_SetText(control,dlf_filename);
      control = GetDlgItem(hDlg, IDC_EDIT_FILENAME);
      Edit_Enable(control, TRUE);
      SetActiveWindow(hDlg);
      break;
    case IDC_RADIO_USE_EXTERNAL:
      control = GetDlgItem(hDlg, IDC_EDIT_FILENAME);
      Edit_Enable(control, TRUE);
      break;
    case IDC_RADIO_USE_BUILT_IN:
      control = GetDlgItem(hDlg, IDC_EDIT_FILENAME);
      Edit_Enable(control, FALSE);
      break;
    }
    break;
  }
  return FALSE;
}

UINT FAR PASCAL CommonDialogBoxHook(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
/* TODO: This function *should* center the File Open/Save dialog box on the screen*/
/*       Currently, the dialog box always appears at the top-left corner when in full-screen mode.  */
/*       Without this Hook function though, the File Open/Save dialog box often appears off-screen. */

/*
  RECT        rc;
  POINT       pt,pt2;

  if (WM_NOTIFY==iMsg)
  {
    if ( ((LPOFNOTIFY) lParam)->hdr.code == CDN_INITDONE )
    {
      SetWindowPos(hDlg, NULL, 100, 100, 0, 0,
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
      return FALSE;
    }
  }
*/
  return FALSE;
}

/* use Windows Common Dialog Box to get a filename for save/open operations */
bool get_open_save_filename(bool save, char* title, char* filter, char* defext, char* filename)
{
  OPENFILENAME ofn;
  char fn[FILENAME_BUFFER_SIZE];
  char dir[FILENAME_BUFFER_SIZE];
  bool result;

  if ( (!g_bWindowed) && (!cur_dialog_box) )
  {
    attach_clipper();
    IDirectDrawSurface_SetPalette(g_pDDSPrimary, NULL);
  }
  SetMenu(hWnd, GetMenu(hWnd));
  show_mouse_cursor();
  strcpy (fn, filename);
  GetCurrentDirectory(FILENAME_BUFFER_SIZE-1,dir);
  ofn.lStructSize       = sizeof (OPENFILENAME);
  ofn.hwndOwner         = hWnd;
  ofn.hInstance         = (HANDLE) g_hInstance;
  ofn.lpstrFilter       = filter;
  ofn.lpstrCustomFilter = (LPTSTR)NULL;
  ofn.nMaxCustFilter    = 0L;
  ofn.nFilterIndex      = 1L;
  ofn.lpstrFile         = fn;
  ofn.nMaxFile          = FILENAME_BUFFER_SIZE-1;
  ofn.lpstrFileTitle    = NULL;
  ofn.nMaxFileTitle     = 0;
  ofn.lpstrInitialDir   = dir;
  ofn.lpstrTitle        = title;
  ofn.nFileOffset       = 0;
  ofn.nFileExtension    = 0;
  ofn.lpstrDefExt       = "drf";
  ofn.lCustData         = 0;
  ofn.lpfnHook=CommonDialogBoxHook;
  if (save)
  {
    ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
    if (!g_bWindowed)
      ofn.Flags|=OFN_ENABLEHOOK;
    result=GetSaveFileName(&ofn);
  }
  else
  {
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
    if (!g_bWindowed)
      ofn.Flags|=OFN_ENABLEHOOK;
    result=GetOpenFileName(&ofn);
  }
  if ( (!g_bWindowed) && (!cur_dialog_box) )
  {
    release_clipper();
    gpal(cur_palette);
  }
  resume_windows_sound_playback();
  InvalidateRect(hWnd, NULL, FALSE);
  UpdateWindow(hWnd);
  if (result)
    strcpy(filename,fn);
  return result;
}

void restore_original_level_data()
{
  memcpy(leveldat,orig_leveldat,8*MHEIGHT*MWIDTH);
  bonusscore=orig_bonusscore;
  strcpy(levfname,"");
  levfflag=FALSE;
}

/* load a DLF file */
void load_level_file(char* fn)
{
  FILE* levf;
  levf=fopen(fn,"rb");
  if (levf==NULL) {
    strcat(fn,".DLF");
    levf=fopen(fn,"rb");
  }
  if (levf==NULL)
  {
    levfflag=FALSE;
    restore_original_level_data();
    strcpy(levfname,"");
  }
  else
  {
    fread(&bonusscore,2,1,levf);
    fread(leveldat,1200,1,levf);
    fclose(levf);
    strcpy(levfname,fn);
    levfflag=TRUE;
  }
}

void init_joystick()
{
    /* use the standard Win32 joystick functions */
    JOYINFO ji;
    if (joyGetNumDevs())
      if (joyGetPos(0,&ji)==JOYERR_NOERROR)
        joyflag=TRUE;
}


