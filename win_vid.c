/* Digger Remastered
   Copyright (c) Andrew Jenner 1998-2004 */

#include "def.h"
#include "win_dig.h"
#include "hardware.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "win_vid.h"
#include "resource.h"
#include "sprite.h"

struct FULLSCREEN_VID_MODE_INFO* supported_video_modes=NULL;  // list of supported video modes (modes that are 640+ x 400+)
struct FULLSCREEN_VID_MODE_INFO preferred_video_mode;         // the vid mode to use when running fullscreen

#define NUM_SPRITES_TO_CACHE 150
int use_direct_draw=0;

HBITMAP title_bitmap[2]={(HBITMAP) NULL, (HBITMAP) NULL};

RGBQUAD vga16_pal1_rgbq[] = {{0,0,0,0},        /* palette1, normal intensity */
                      {128,0,0,0},
                      {0,128,0,0},
                      {128,128,0,0},
                      {0,0,128,0},
                      {128,0,128,0},
                      {0,64,128,0},
                      {128,128,128,0},
                      {64,64,64,0},
                      {255,0,0,0},
                      {0,255,0,0},
                      {255,255,0,0},
                      {0,0,255,0},
                      {255,0,255,0},
                      {0,255,255,0},
                      {255,255,255,0}};

RGBQUAD vga16_pal1i_rgbq[] = {{0,0,0,0},       /* palette1, high intensity */
                      {255,0,0,0},
                      {0,255,0,0},
                      {255,255,0,0},
                      {0,0,255,0},
                      {255,0,255,0},
                      {0,128,255,0},
                      {192,192,192,0},
                      {128,128,128,0},
                      {255,128,128,0},
                      {128,255,128,0},
                      {255,255,128,0},
                      {128,128,255,0},
                      {255,128,255,0},
                      {128,255,255,0},
                      {255,255,255,0}};

RGBQUAD vga16_pal2_rgbq[] = {{0,0,0,0},        /* palette2, normal intensity */
                      {0,128,0,0},
                      {0,0,128,0},
                      {0,64,128,0},
                      {128,0,0,0},
                      {128,128,0,0},
                      {128,0,128,0},
                      {128,128,128,0},
                      {64,64,64,0},
                      {0,255,0,0},
                      {0,0,255,0},
                      {0,255,255,0},
                      {255,0,0,0},
                      {255,255,0,0},
                      {255,0,255,0},
                      {255,255,255,0}};

RGBQUAD vga16_pal2i_rgbq[] = {{0,0,0,0},       /* palette2, high intensity */
                      {0,255,0,0},
                      {0,0,255,0},
                      {0,128,255,0},
                      {255,0,0,0},
                      {255,255,0,0},
                      {255,0,255,0},
                      {192,192,192,0},
                      {128,128,128,0},
                      {128,255,128,0},
                      {128,128,255,0},
                      {128,255,255,0},
                      {255,128,128,0},
                      {255,255,128,0},
                      {255,128,255,0},
                      {255,255,255,0}};

RGBQUAD cga16_pal1_rgbq[] = {{0,0,0,0},        /* palette1, normal intensity */
                      {0,168,0,0},
                      {0,0,168,0},
                      {0,84,168,0},
                      {0,0,128,0},
                      {128,0,128,0},
                      {0,64,128,0},
                      {128,128,128,0},
                      {64,64,64,0},
                      {255,0,0,0},
                      {0,255,0,0},
                      {255,255,0,0},
                      {0,0,255,0},
                      {255,0,255,0},
                      {0,255,255,0},
                      {255,255,255,0}};

RGBQUAD cga16_pal1i_rgbq[] = {{0,0,0,0},       /* palette1, high intensity */
                      {85,255,85,0},
                      {85,85,255,0},
                      {85,255,255,0},
                      {0,0,255,0},
                      {255,0,255,0},
                      {0,128,255,0},
                      {192,192,192,0},
                      {128,128,128,0},
                      {255,128,128,0},
                      {128,255,128,0},
                      {255,255,128,0},
                      {128,128,255,0},
                      {255,128,255,0},
                      {128,255,255,0},
                      {255,255,255,0}};

RGBQUAD cga16_pal2_rgbq[] = {{0,0,0,0},        /* palette2, normal intensity */
                      {0,128,0,0},
                      {128,0,128,0},
                      {160,160,160,0},
                      {160,160,160,0},
                      {128,128,0,0},
                      {128,0,128,0},
                      {128,128,128,0},
                      {64,64,64,0},
                      {0,255,0,0},
                      {0,0,255,0},
                      {0,255,255,0},
                      {255,0,0,0},
                      {255,255,0,0},
                      {255,0,255,0},
                      {255,255,255,0}};

RGBQUAD cga16_pal2i_rgbq[] = {{0,0,0,0},       /* palette2, high intensity */
                      {0,255,0,0},
                      {0,0,255,0},
                      {160,160,160,0},
                      {255,0,0,0},
                      {255,255,0,0},
                      {255,0,255,0},
                      {192,192,192,0},
                      {128,128,128,0},
                      {128,255,128,0},
                      {128,128,255,0},
                      {128,255,255,0},
                      {255,128,128,0},
                      {255,255,128,0},
                      {255,128,255,0},
                      {255,255,255,0}};


RGBQUAD *windowed_palette[4];  /* Used in Windowed mode. These palettes are applied to the 'back_bitmap' */

HPALETTE desktop_palette=(HPALETTE) NULL;                                     /* Used in Windowed mode, but is only if Windows is set to a color resolution which supports palettes.  {ie. 256 colors} ) */
bool palettized_desktop=FALSE;
LPDIRECTDRAWPALETTE    fullscreen_palette[4] = {NULL, NULL, NULL, NULL};      /* Used in Full Screen mode.  These palettes are applied to the DirectDraw primary surface. */
LPDIRECTDRAW            g_pDD         = NULL;   /* DirectDraw object */
LPDIRECTDRAWSURFACE     g_pDDSPrimary = NULL;   /* DirectDraw primary surface */
//LPDIRECTDRAWSURFACE     g_pDDSBack    = NULL;   /* DirectDraw back surface */
RECT                    g_rcWindow;             /* Saves the window size & pos.*/
RECT                    g_rcViewport;           /* Pos. & size to blt from */
RECT                    g_rcScreen;             /* Screen pos. for blt */
BOOL                    g_bActive     = FALSE;
BOOL                    g_bReady      = FALSE;  /* App is ready for updates */
BOOL                    g_bWindowed   = TRUE;   /* App is in windowed mode */
RECT                    rc_draw_area;
RECT                    rc_640x400;

bool use_async_screen_updates;
enum video_mode_enum video_mode= VIDEO_MODE_VGA_16;

int cur_intensity;
int cur_palette;

HDC back_dc;
HBITMAP back_bitmap;
char *back_bitmap_bits;
HBITMAP old_bitmap;
bool use_640x480_fullscreen;

Uint3 *sprite_data=NULL;
Uint3 *sprite_array[200];


HRESULT blit_to_window(void);
void vgaclear(void);
void destroy_palettes();
void init_palettes();

void init_graphics()
{
  HDC window_dc;

  window_dc = GetDC(hWnd);
  init_directdraw();
  create_back_buffer(window_dc);
  ReleaseDC(hWnd, window_dc);
  init_surfaces();
}

void graphicsoff(void) {};
void gretrace(void)
{
};



/********************************************************/
/* Functions for displaying the VGA data                */
/********************************************************/
void vgainit(void)
{
  video_mode=VIDEO_MODE_VGA_16;
  windowed_palette[0] = vga16_pal1_rgbq;
  windowed_palette[1] = vga16_pal1i_rgbq;
  windowed_palette[2] = vga16_pal2_rgbq;
  windowed_palette[3] = vga16_pal2i_rgbq;
  destroy_palettes();
  init_palettes();
}

void vgaclear(void)
{
  memset(back_bitmap_bits, 0, 256000l);
  blit_to_window();
};

void vgapal(Sint4 pal)
{
  cur_palette=pal;
  if (g_bWindowed || preferred_video_mode.bpp > 8)
  {
    SetDIBColorTable(back_dc,0,16, windowed_palette[cur_palette*2 + cur_intensity]);
    if (use_async_screen_updates)
      InvalidateRect(hWnd, NULL, FALSE);
    else
      blit_to_window();
  }
  else
  {
    IDirectDrawSurface_SetPalette(g_pDDSPrimary, fullscreen_palette[cur_palette*2 + cur_intensity]);
    SetDIBColorTable(back_dc,0,16, windowed_palette[cur_palette*2 + cur_intensity]);
  }
};

void vgainten(Sint4 inten)
{
  cur_intensity=inten;
  if (g_bWindowed || preferred_video_mode.bpp > 8)
  {
    SetDIBColorTable(back_dc,0,16, windowed_palette[cur_palette*2 + cur_intensity]);
    if (use_async_screen_updates)
      InvalidateRect(hWnd, NULL, FALSE);
    else
      blit_to_window();
  }
  else
  {
    IDirectDrawSurface_SetPalette(g_pDDSPrimary, fullscreen_palette[cur_palette*2 + cur_intensity]);
    SetDIBColorTable(back_dc,0,16, windowed_palette[cur_palette*2 + cur_intensity]);
  }
};

Sint4 vgagetpix(Sint4 x,Sint4 y)
{
  Uint4 xi,yi;
  Sint4 rval;

  rval=0;
  if (x>319||y>199)
  {
    return 0xff;
  }
  for (yi=0;yi<2;yi++)
    for (xi=0;xi<8;xi++)
      if (back_bitmap_bits[(Uint5) ((y*2l+yi)*640l + x*2l + xi)])
        rval |= 0x80 >> xi;

  rval &= 0xee;
  return rval;
};

void vgawrite(Sint4 x,Sint4 y,Sint4 ch,Sint4 c)
{
  Uint5 yi;
  Uint5 xi;
  int color;

  ch-=32;
  if (ch<0x5f)
  {
    if (ascii2vga[ch])
    {
      for (yi=0;yi<24;yi++)
      {
        for (xi=0;xi<24;xi++)
        {
          if (xi&0x1)
            color=(ascii2vga[ch][yi*12+(xi>>1)] & 0x0F);
          else
            color=(ascii2vga[ch][yi*12+(xi>>1)] >> 4);
          if (color==10)
            if (c==2)
              color=12;
            else
            {
              if (c==3)
                color=14;
            }
          else
            if (color==12)
              if (c==1)
                color=2;
              else
                if (c==2)
                  color=4;
                else
                  if (c==3)
                    color=6;

          back_bitmap_bits[(Uint5) ((y*2+yi)*640l + x*2l+xi)]=color;
        }
      }
    }
    else    /* draw a space (needed when reloading and displaying high scores when user switches to/from normal/gauntlet mode, etc ) */
      for (yi=0;yi<24;yi++)
        memset(&back_bitmap_bits[(y*2+yi)*640 + x*2] , 0, 24);
  }
  blit_rect_to_window(x*2, y*2, 24, 24);
}

void cgawrite(Sint4 x,Sint4 y,Sint4 ch,Sint4 c)
{
  Uint5 yi;
  Uint5 xi;
  int color;

  ch-=32;
  if (ch<0x5f)
  {
    if (ascii2cga[ch])
    {
      for (yi=0;yi<12;yi++)
      {
        for (xi=0;xi<12;xi++)
        {
          if (xi&0x1)
            color=(ascii2cga[ch][yi*6+(xi>>1)] & 0x0F);
          else
            color=(ascii2cga[ch][yi*6+(xi>>1)] >> 4);

          color=color&c;
          farmemset(back_bitmap_bits + ((y*2+yi*2)*640l + x*2+xi*2), color,2);
          farmemset(back_bitmap_bits + ((y*2+yi*2+1)*640l + x*2+xi*2), color,2);
        }
      }
    }
    else    /* draw a space (needed when reloading and displaying high scores when user switches to/from normal/gauntlet mode, etc ) */
      for (yi=0;yi<24;yi++)
        memset(&back_bitmap_bits[(y*2+yi)*640 + x*2] , 0, 24);
  }
  blit_rect_to_window(x*2, y*2, 24, 24);
}


void vgatitle(void)
{
  display_title_bitmap(0);
}

void cgatitle(void)
{
  display_title_bitmap(1);
}

void display_title_bitmap(int idx)
{
  HDC temp_dc;
  HBITMAP old_bitmap;
  GdiFlush();
  temp_dc = CreateCompatibleDC(back_dc);
  if (palettized_desktop)
  {
    SelectPalette(temp_dc, desktop_palette, FALSE);
    RealizePalette(temp_dc);
  }
  old_bitmap=SelectObject(temp_dc, title_bitmap[idx]);
  BitBlt(back_dc, 0, 0, 640, 400, temp_dc, 0, 0, SRCCOPY);
  GdiFlush();
  SelectObject(temp_dc, old_bitmap);
  DeleteDC(temp_dc);
  blit_to_window();

}

/* blit entire backbuffer to the window */
HRESULT blit_to_window()
{
  HRESULT hresult;
  DDSURFACEDESC ddsd;
  int x,y;
  RECT nodraw;
  POINT pt;

  ddsd.dwSize=sizeof(ddsd);
  hresult=DD_OK;

  if (!g_bWindowed && !g_bActive)
    return hresult;

  if (g_bWindowed || preferred_video_mode.bpp > 8)
  {
    InvalidateRect(hWnd, NULL, FALSE);
    UpdateWindow(hWnd);
  }
  else
  {
//    if (g_pDDSPrimary && !suspend_game)
//      return IDirectDrawSurface_Blt(g_pDDSPrimary,  &g_rcScreen, g_pDDSBack, &rc_draw_area, DDBLT_WAIT, NULL);
    if (cur_dialog_box)
      GetWindowRect(cur_dialog_box,&nodraw);
    IDirectDrawSurface_Lock(g_pDDSPrimary,NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
    for (x=0;x<640;x++)
      for (y=0;y<400;y++)
      {
        if (cur_dialog_box)
        {
          pt.x = x;
          pt.y = y;
          if (PtInRect(&nodraw,pt))
            continue;
        }
        ((char*) ddsd.lpSurface)[y*ddsd.lPitch+x]=back_bitmap_bits[y*640+x];
      }
    IDirectDrawSurface_Unlock(g_pDDSPrimary, ddsd.lpSurface);
  }
  return DD_OK;
}

/* This is ONLY called when the window receives a WM_PAINT message. */
/* Repaint all or a portion of the window.                          */
/* (used in both Windowed and Fullscreen mode)                      */
void blit_to_window_dc(HWND wnd)
{
  HDC hDC;
  PAINTSTRUCT paintStruct;

  hDC = BeginPaint(hWnd, &paintStruct);
  if (palettized_desktop)
  {
    SelectPalette (hDC, desktop_palette, TRUE);
    RealizePalette (hDC);
  }

  if (g_bWindowed)
  {
    BitBlt(hDC,0,0,640,400,back_dc, 0,0,SRCCOPY);
  }
  else
    BitBlt(hDC,g_rcScreen.left,g_rcScreen.top,640,400,back_dc, 0,0,SRCCOPY);
  EndPaint(hWnd, &paintStruct);
}

HRESULT restore_surface()
{
  HRESULT hRet;
  hRet=IDirectDrawSurface_Restore(g_pDDSPrimary);
  if (hRet != DD_OK)
  {
    fatal_error(hRet, "DirectDraw: restore surfaces (primary) failed");
  }
  return TRUE;
}

/* Blits the given rectangle to the Window (or marks the given rectangle */
/* as dirty if Async option is on).                                      */
/* In DirectX/Fullscreen mode, there must NOT be a Clipper attached to   */
/* the surface (otherwise BltFast will not work).                        */
void blit_rect_to_window(int x, int y, int w, int h)
{
  RECT rect;
  DDSURFACEDESC ddsd;
  int xi,yi;

  ddsd.dwSize=sizeof(ddsd);

//  if (!g_bWindowed && !g_bActive)
//    return;

  if (g_bWindowed || preferred_video_mode.bpp > 8)
  {
    rect.left=x;
    rect.right=x+w;
    rect.top=y;
    rect.bottom=y+h;
    InvalidateRect(hWnd, &rect, FALSE);
    if (!use_async_screen_updates)
      UpdateWindow(hWnd);
  }
  else
  {
    if (g_pDDSPrimary)
    {
      rect.left=x;
      rect.right=x+w;
      rect.top=y;
      rect.bottom=y+h;
//      IDirectDrawSurface_BltFast(g_pDDSPrimary, g_rcScreen.left+x, g_rcScreen.top+y, g_pDDSBack, &rect, DDBLTFAST_NOCOLORKEY /* | DDBLTFAST_WAIT */);
      IDirectDrawSurface_Lock(g_pDDSPrimary,&rect, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
      for (xi=0;xi<w;xi++)
        for (yi=0;yi<h;yi++)
          ((char*) ddsd.lpSurface)[yi*ddsd.lPitch+xi]=back_bitmap_bits[(yi+y)*640+xi+x];
      IDirectDrawSurface_Unlock(g_pDDSPrimary, ddsd.lpSurface);
    }
  }
}

/* This function creates a back buffer for drawing on.             */
/*  - on Win32 this is a DIBSection                                */
bool create_back_buffer(HDC window_dc)
{
  HANDLE hloc;
  BITMAPINFO *pbmi;

  back_dc = CreateCompatibleDC(window_dc);
  back_bitmap=0;

  hloc = LocalAlloc(LMEM_ZEROINIT | LMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 16));
  pbmi = (BITMAPINFO*) LocalLock(hloc);
  pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  pbmi->bmiHeader.biWidth = 640;
  pbmi->bmiHeader.biHeight = -400;
  pbmi->bmiHeader.biPlanes = 1;
  pbmi->bmiHeader.biBitCount = 8;
  pbmi->bmiHeader.biClrUsed = 16;
  pbmi->bmiHeader.biClrImportant = 16;
  pbmi->bmiHeader.biCompression = BI_RGB;
  memset(pbmi->bmiColors,0,sizeof(RGBQUAD)*16);
  back_bitmap = CreateDIBSection(window_dc, (BITMAPINFO*) pbmi, DIB_RGB_COLORS, (VOID **) &back_bitmap_bits, (HANDLE) NULL, 0);
  LocalFree(hloc);
  if (!back_bitmap)
    return FALSE;
  old_bitmap = SelectObject(back_dc, back_bitmap);

  return TRUE;
}

void destroy_back_buffer()
{
  release_directdraw_objects();
  SelectObject(back_dc, old_bitmap);
  DeleteObject(back_bitmap);
  DeleteDC(back_dc);
  DeleteObject(desktop_palette);
}

HRESULT release_directdraw_objects()
{
  if (use_direct_draw)
  {
    if (g_pDD != NULL)
    {
      IDirectDraw_RestoreDisplayMode(g_pDD);
      IDirectDraw_SetCooperativeLevel(g_pDD, hWnd, DDSCL_NORMAL);
//      if (g_pDDSBack != NULL)
//      {
//          IDirectDraw_Release(g_pDDSBack);
//          g_pDDSBack = NULL;
//      }
      if (g_pDDSPrimary != NULL)
      {
          IDirectDraw_Release(g_pDDSPrimary);
          g_pDDSPrimary = NULL;
      }
    }
  }
  return DD_OK;
}

HRESULT fatal_error(HRESULT hRet, LPCTSTR szError)
{
  windows_finish();
  MessageBox(hWnd, szError, "Digger", MB_OK);
  DestroyWindow(hWnd);
  exit(1);
  return hRet;
}


HRESULT init_surfaces()
{
  HRESULT          hRet;
  DDSURFACEDESC   ddsd;
  DDBLTFX ddbltfx;
  int i, palnum;
  HDC hDC;
  LOGPALETTE*      logpalette;
  RGBQUAD *wp[] = { vga16_pal1_rgbq,vga16_pal1i_rgbq,vga16_pal2_rgbq,vga16_pal2i_rgbq };

  if (!use_direct_draw)
    g_bWindowed=TRUE;

  if (g_bWindowed)
  {
    /* Are we running in 256 colors (or less)?  */

    hDC=GetDC(hWnd);
    palettized_desktop=GetDeviceCaps(hDC, RASTERCAPS)&RC_PALETTE;
    if (palettized_desktop)
    {
      logpalette=(LOGPALETTE*) malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY)*64);
      if (!logpalette)
        fatal_error(0,"init_surfaces: could not allocate memory for 'logpalette'");
      logpalette->palNumEntries=64;
      logpalette->palVersion=0x300;

      for (palnum=0;palnum<4;palnum++)
        for (i=0;i<16;i++)
        {
          logpalette->palPalEntry[palnum*16+i].peRed=wp[palnum][i].rgbRed;
          logpalette->palPalEntry[palnum*16+i].peBlue=wp[palnum][i].rgbBlue;
          logpalette->palPalEntry[palnum*16+i].peGreen=wp[palnum][i].rgbGreen;
          logpalette->palPalEntry[palnum*16+i].peFlags=(BYTE) NULL; //PC_NOCOLLAPSE;
        }

      desktop_palette=CreatePalette(logpalette);
      free (logpalette);
      SelectPalette(hDC,desktop_palette,TRUE);
      i=RealizePalette(hDC);
    }
    ReleaseDC(hWnd, hDC);
    if (use_direct_draw)
    {
      IDirectDraw_RestoreDisplayMode(g_pDD);
      hRet = IDirectDraw_SetCooperativeLevel(g_pDD, hWnd, DDSCL_NORMAL);
      if (hRet != DD_OK)
        return fatal_error(hRet, "DirectDraw: SetCooperativeLevel FAILED");
    }
    GetClientRect(hWnd, &g_rcViewport);
    GetClientRect(hWnd, &g_rcScreen);
    ClientToScreen(hWnd, (POINT*)&g_rcScreen.left);
    ClientToScreen(hWnd, (POINT*)&g_rcScreen.right);
  }
  else
  {
    SetWindowLong(hWnd, GWL_STYLE, DIGGER_WS_FULLSCREEN);
    if (preferred_video_mode.height<=400)
      SetMenu(hWnd,NULL);
    hRet = IDirectDraw_SetCooperativeLevel(g_pDD, hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
    if (hRet != DD_OK)
      return fatal_error(hRet, "DirectDraw: SetCooperativeLevel (fullscreen) FAILED");

    SetRect(&g_rcViewport, 0, 0, 640, 400 );
    memcpy(&g_rcScreen, &g_rcViewport, sizeof(RECT) );

    if (!use_640x480_fullscreen)
      hRet = IDirectDraw_SetDisplayMode(g_pDD, preferred_video_mode.width, preferred_video_mode.height, preferred_video_mode.bpp);
    if (hRet != DD_OK || use_640x480_fullscreen)
    {
      hRet = IDirectDraw_SetDisplayMode(g_pDD, 640, 480, 8);
      if (hRet != DD_OK)
      {
        return fatal_error(hRet, "DirectDraw: SetDisplayMode FAILED");
      }
      SetRect(&g_rcViewport, 0, 40, 640, 440 );
      memcpy(&g_rcScreen, &g_rcViewport, sizeof(RECT) );
    }

    /* Create the primary surface */
    ZeroMemory(&ddsd,sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    hRet = IDirectDraw_CreateSurface(g_pDD, &ddsd, &g_pDDSPrimary, NULL);
    if (hRet != DD_OK)
      return fatal_error(hRet, "DirectDraw: CreateSurface (primary, fullscreen) FAILED");

    vgainten(cur_intensity);

    /* clear the screen */
    ZeroMemory(&ddbltfx,sizeof(DDBLTFX));
    ddbltfx.dwFillColor=0;
    ddbltfx.dwSize=sizeof(DDBLTFX);
    IDirectDrawSurface_Blt(g_pDDSPrimary, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT , &ddbltfx );

  }
  rc_640x400.left=0;
  rc_640x400.top=0;
  rc_640x400.right=640;
  rc_640x400.bottom=400;
  rc_draw_area.left=0;
  rc_draw_area.top=0;
  rc_draw_area.right=640;
  rc_draw_area.bottom=400;
  return DD_OK;
}

void destroy_palettes()
{
  fullscreen_palette[0]=NULL;
}

void init_palettes()
{
  HRESULT          hRet;
  PALETTEENTRY     palentry[256];
  int i, palnum;
  HDC hDC;
  LOGPALETTE*      logpalette;

  if (!use_direct_draw)
    g_bWindowed=TRUE;

  if (g_bWindowed)
  {
    /* Are we running in 256 colors (or less)?  */

    hDC=GetDC(hWnd);
    palettized_desktop=GetDeviceCaps(hDC, RASTERCAPS)&RC_PALETTE;
    if (palettized_desktop)
    {
      logpalette=(LOGPALETTE*) malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY)*64);
      if (!logpalette)
        fatal_error(0,"init_surfaces: could not allocate memory for 'logpalette'");
      logpalette->palNumEntries=64;
      logpalette->palVersion=0x300;

      for (palnum=0;palnum<4;palnum++)
        for (i=0;i<16;i++)
        {
          logpalette->palPalEntry[palnum*16+i].peRed=windowed_palette[palnum][i].rgbRed;
          logpalette->palPalEntry[palnum*16+i].peBlue=windowed_palette[palnum][i].rgbBlue;
          logpalette->palPalEntry[palnum*16+i].peGreen=windowed_palette[palnum][i].rgbGreen;
          logpalette->palPalEntry[palnum*16+i].peFlags=(BYTE) NULL; //PC_NOCOLLAPSE;
        }

      desktop_palette=CreatePalette(logpalette);
      free (logpalette);
      SelectPalette(hDC,desktop_palette,TRUE);
      i=RealizePalette(hDC);
    }
    ReleaseDC(hWnd, hDC);

/*
    if (use_direct_draw)
    {
      hRet = IDirectDraw_SetCooperativeLevel(g_pDD, hWnd, DDSCL_NORMAL);
      if (hRet != DD_OK)
        fatal_error(hRet, "DirectDraw: SetCooperativeLevel FAILED");
    }
*/
    GetClientRect(hWnd, &g_rcViewport);
    GetClientRect(hWnd, &g_rcScreen);
    ClientToScreen(hWnd, (POINT*)&g_rcScreen.left);
    ClientToScreen(hWnd, (POINT*)&g_rcScreen.right);
  }
  else
  {
    if (fullscreen_palette[0]==NULL)
    {
      for (palnum=0;palnum<4;palnum++)
      {
        for (i=0;i<256;i++)
        {
          if (i<16)
          {
            palentry[i].peRed=windowed_palette[palnum][i].rgbRed;
            palentry[i].peBlue=windowed_palette[palnum][i].rgbBlue;
            palentry[i].peGreen=windowed_palette[palnum][i].rgbGreen;
            palentry[i].peFlags=(BYTE) NULL;
          }
          else
          {
            palentry[i].peRed=0;
            palentry[i].peBlue=0;
            palentry[i].peGreen=0;
            palentry[i].peFlags=(BYTE) NULL;
          }
        }
        IDirectDraw_CreatePalette(g_pDD, DDPCAPS_8BIT, palentry, &fullscreen_palette[palnum], NULL);
      }
    }

    vgainten(cur_intensity);
  }
}

/* toggle between windowed and fullscreen mode */

HRESULT ChangeCoopLevel()
{
    HRESULT hRet;

    if (FAILED(hRet = release_directdraw_objects()))
      return fatal_error(hRet, "DirectDraw: release_directdraw_objects FAILED");

    if (g_bWindowed)
    {
      SetWindowLong(hWnd, GWL_STYLE, DIGGER_WS_WINDOWED);
      SetWindowPos(hWnd, HWND_NOTOPMOST, g_rcWindow.left, g_rcWindow.top,
                  (g_rcWindow.right - g_rcWindow.left),
                  (g_rcWindow.bottom - g_rcWindow.top), SWP_SHOWWINDOW );
      InvalidateRect(NULL, NULL, TRUE);   // all windows should be repainted
    }
    hRet = init_surfaces();
    init_palettes();
    return hRet;
}

void init_directdraw()
{
  HRESULT hRet;
  DDSURFACEDESC ddsd;

  GetWindowRect(hWnd, &g_rcWindow); // what is this doing here???

  if (!lpDirectDrawCreate)
  {
    use_direct_draw=0;
    return;
  }

  hRet = DirectDrawCreate( NULL, &g_pDD, NULL);
  if (hRet==DD_OK)
  {
    use_direct_draw=1;
  }
  memset(&ddsd,0,sizeof(ddsd));
  ddsd.dwSize=sizeof(ddsd);
  ddsd.dwWidth=640;
  ddsd.dwFlags=DDSD_WIDTH;
  hRet=IDirectDraw_EnumDisplayModes(g_pDD, (DWORD) NULL, /*NULL*/ &ddsd, NULL, &EnumModesCallback);
}

void attach_clipper()
{
  LPDIRECTDRAWCLIPPER pClipper;
  HRESULT hRet;

  /* add a clipper to the primary surface */
  hRet = IDirectDraw_CreateClipper(g_pDD, 0, &pClipper, NULL);
  if (hRet != DD_OK)
    fatal_error(hRet, "attach_clipper(): CreateClipper FAILED");
  hRet = IDirectDrawClipper_SetHWnd(pClipper, 0, hWnd);
  if (hRet != DD_OK)
    fatal_error(hRet, "attach_clipper(): SetHWnd FAILED");
  hRet = IDirectDrawSurface_SetClipper(g_pDDSPrimary, pClipper);
  if (hRet != DD_OK)
    fatal_error(hRet, "attach_clipper(): SetClipper FAILED");
  IDirectDrawClipper_Release(pClipper);
  pClipper = NULL;
}

void release_clipper()
{
  HRESULT hRet;
  hRet = IDirectDrawSurface_SetClipper(g_pDDSPrimary, (LPDIRECTDRAWCLIPPER) NULL);
  if (hRet != DD_OK)
    fatal_error(hRet, "release_clipper(): SetClipper FAILED");
}

void vgaputim2(Sint4 x,Sint4 y,Sint4 ch,Sint4 w,Sint4 h)
{
/* TO DO: convert the vgagraphics to a more appropriate format,
          rewrite this routine(load the sprites onto the DirectDraw
          Surface beforehand). */

  Uint5 scrn_width;
  Uint5 scrn_height;
  int y_loop_end;
  int y_loop_count;
  int x_loop_end;
  int x_loop_count;
  unsigned char* cur_src_mask_ptr;
  unsigned char* cur_src_ptr;
  unsigned char* cur_dest_ptr;
  unsigned char* scrn_max_ptr;
  Uint5 i;
  Uint5 plane;
  int color;
  Uint5 dest_next_row_offset;
  Uint5 src_plane_offset;
  RECT rect;
  DDSURFACEDESC ddsd;
  DDBLTFX ddbltfx;
  HRESULT hRet;

  scrn_width=640;
  scrn_height=400;
  scrn_max_ptr = back_bitmap_bits + (Uint5) scrn_width*scrn_height;

  cur_src_mask_ptr=vgatable[ch*2+1];
  cur_src_ptr=vgatable[ch*2];
  cur_dest_ptr=&(back_bitmap_bits[(Uint5) (y*2l * scrn_width + x*2l)]);
  dest_next_row_offset = scrn_width - w*8l;

  src_plane_offset = w*h*2;

  y_loop_end=h*2;
  x_loop_end=w*8;

  if (ch>111) return;

  for (y_loop_count=0;y_loop_count<y_loop_end;y_loop_count++)
  {
    for (x_loop_count=0;x_loop_count<x_loop_end;x_loop_count++)
    {
      color=*(sprite_array+ch*1024/2+y_loop_count*32+x_loop_count);
      if (cur_dest_ptr < scrn_max_ptr)
        *cur_dest_ptr = color;
      cur_dest_ptr++;
    }
    cur_dest_ptr+=dest_next_row_offset;
  }

  if (y*2 + h*2 < 400)
    blit_rect_to_window(x*2, y*2, w*8, h*2);
  else
    blit_rect_to_window(x*2, y*2, w*8, h*2 - (y*2 + h*2) + 400);
}

void vgaputim(Sint4 x,Sint4 y,Sint4 ch,Sint4 w,Sint4 h)
{
/* TO DO: convert the vgagraphics to a more appropriate format,
          rewrite this routine(load the sprites onto the DirectDraw
          Surface beforehand). */

  Uint5 scrn_width;
  Uint5 scrn_height;
  int y_loop_end;
  int y_loop_count;
  int x_loop_end;
  int x_loop_count;
  unsigned char* cur_src_mask_ptr;
  unsigned char* cur_src_ptr;
  unsigned char* cur_dest_ptr;
  unsigned char* scrn_max_ptr;
  Uint5 i;
  Uint5 plane;
  int color;
  Uint5 dest_next_row_offset;
  Uint5 src_plane_offset;
  RECT rect;
  DDSURFACEDESC ddsd;
  DDBLTFX ddbltfx;
  HRESULT hRet;

  scrn_width=640;
  scrn_height=400;
  scrn_max_ptr = back_bitmap_bits + (Uint5) scrn_width*scrn_height;

  cur_src_mask_ptr=vgatable[ch*2+1];
  cur_src_ptr=vgatable[ch*2];
  cur_dest_ptr=&(back_bitmap_bits[(Uint5) (y*2l * scrn_width + x*2l)]);
  dest_next_row_offset = scrn_width - w*8l;

  src_plane_offset = w*h*2;

  y_loop_end=h*2;
  x_loop_end=w;

  for (y_loop_count=0;y_loop_count<y_loop_end;y_loop_count++)
  {
    for (x_loop_count=0;x_loop_count<x_loop_end;x_loop_count++)
    {
      for (i=0;i<8;i++)
      {
        if (!((*cur_src_mask_ptr)&(0x80>>i)))
        {
          color=0;
          for (plane=0;plane<4;plane++)
          {
            color|=((((*(cur_src_ptr + (Uint5) (plane*src_plane_offset))) << i) & 0x80 ) >> (4 + plane));
          }
          if (cur_dest_ptr < scrn_max_ptr)
            *cur_dest_ptr = color;

        }
        cur_dest_ptr++;
      }
      cur_src_ptr++;
      cur_src_mask_ptr++;
    }
    cur_dest_ptr+=dest_next_row_offset;
  }

  if (y*2 + h*2 < 400)
    blit_rect_to_window(x*2, y*2, w*8, h*2);
  else
    blit_rect_to_window(x*2, y*2, w*8, h*2 - (y*2 + h*2) + 400);
}

void vgaputi(Sint4 x,Sint4 y,Uint3 *p,Sint4 w,Sint4 h)
{
  Uint5 i;
  for (i=0;i<h*2;i++)
    if (i+y*2 < 400)
      farmemcpy(back_bitmap_bits+(Uint5) ((y*2+i)*640l + x*2), (char far*) (p + (Uint5) (i*w*8l)) , w*8l);
  if (y*2 + h*2 < 400)
    blit_rect_to_window(x*2, y*2, w*8, h*2);
  else
    blit_rect_to_window(x*2, y*2, w*8, h*2 - (y*2 + h*2) + 400);
}

void vgageti(Sint4 x,Sint4 y,Uint3 *p,Sint4 w,Sint4 h)
{
  Uint5 i;
  for (i=0;i<h*2;i++)
    if (i+y*2 < 400)
      farmemcpy( (char far*) (p + (Uint5) i*w*8l), back_bitmap_bits+ (Uint5) ((y*2l+i)*640l + x*2l) , w*8);
}

void cgaputim(Sint4 x,Sint4 y,Sint4 ch,Sint4 w,Sint4 h)
{
/* TO DO: convert the cgagraphics to a more appropriate format,
          rewrite this routine(load the sprites onto the DirectDraw
          Surface beforehand). */

  Uint5 scrn_width;
  Uint5 scrn_height;
  int y_loop_end;
  int y_loop_count;
  int x_loop_end;
  int x_loop_count;
  unsigned char* cur_src_mask_ptr;
  unsigned char* cur_src_ptr;
  unsigned char* cur_dest_ptr;
  unsigned char* scrn_max_ptr;
  Uint5 i;
  int color;
  Uint5 dest_next_row_offset;
  RECT rect;
  DDSURFACEDESC ddsd;
  DDBLTFX ddbltfx;
  HRESULT hRet;

  scrn_width=640;
  scrn_height=400;
  scrn_max_ptr = back_bitmap_bits + (Uint5) scrn_width*scrn_height;

  cur_src_mask_ptr=cgatable[ch*2+1];
  cur_src_ptr=cgatable[ch*2];
  cur_dest_ptr=&(back_bitmap_bits[(Uint5) (y*2 * scrn_width + x*2)]);
  dest_next_row_offset = scrn_width; // - w*4l;

  y_loop_end=h;
  x_loop_end=w;

  for (y_loop_count=0;y_loop_count<y_loop_end;y_loop_count++)
  {
    for (x_loop_count=0;x_loop_count<x_loop_end;x_loop_count++)
    {
      for (i=0;i<4;i++)
      {
        if (!( (*cur_src_mask_ptr)&(0xC0>>(i*2)) ))
        {
          color=((*cur_src_ptr)>>(6-(i*2)))&0x03;
          if (cur_dest_ptr < scrn_max_ptr)
            *cur_dest_ptr = *(cur_dest_ptr+1) = *(cur_dest_ptr+dest_next_row_offset) = *(cur_dest_ptr+dest_next_row_offset+1) = color;
        }
        cur_dest_ptr+=2;
      }
      cur_src_ptr++;
      cur_src_mask_ptr++;
    }
    cur_dest_ptr+=(dest_next_row_offset*2-w*8);
  }
  if (y*2 + h*2 < 400)
    blit_rect_to_window(x*2, y*2, w*8, h*2);
  else
    blit_rect_to_window(x*2, y*2, w*8, h*2 - (y*2 + h*2) + 400);
}

void cgageti(Sint4 x,Sint4 y,Uint3 *p,Sint4 w,Sint4 h)
{
  vgageti(x,y,p,w,h);
}

void cgaputi(Sint4 x,Sint4 y,Uint3 *p,Sint4 w,Sint4 h)
{
  vgaputi(x,y,p,w,h);
}

Sint4 cgagetpix(Sint4 x,Sint4 y)
{
  Uint4 xi,yi;
  Sint4 rval;

  rval=0;
  if (x>319||y>199)
  {
    return 0xff;
  }
  for (yi=0;yi<2;yi++)
    for (xi=0;xi<8;xi++)
      if (back_bitmap_bits[(Uint5) ((y*2l+yi)*640l + x*2l + xi)])
        rval |= 0x80 >> xi;

  rval &= 0xee;
  return rval;
}

/*******************************************************/
/* Functions for displaying the CGA data               */
/*******************************************************/
void cgainit(void)
{
  video_mode=VIDEO_MODE_CGA;
  windowed_palette[0] = cga16_pal1_rgbq;
  windowed_palette[1] = cga16_pal1i_rgbq;
  windowed_palette[2] = cga16_pal2_rgbq;
  windowed_palette[3] = cga16_pal2i_rgbq;
  destroy_palettes();
  init_palettes();
}

void cgaclear(void)
{
  vgaclear();
}

void cgapal(Sint4 pal)
{
  vgapal(pal);
}

void cgainten(Sint4 inten)
{
  vgainten(inten);
}

void load_title_bitmaps()
{
  title_bitmap[0]=LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_TITLEBITMAP));
  title_bitmap[1]=LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_CGATITLEBITMAP));
}

void change_graphics_routines(Uint3 newmode)
{
  g_bReady=FALSE;
  ginit=cgainit;
  switch (newmode)
  {
  case VIDEO_MODE_CGA:
    gpal=cgapal;
    ginten=cgainten;
    gclear=cgaclear;
    ggetpix=cgagetpix;
    gputi=cgaputi;
    ggeti=cgageti;
    gputim=cgaputim;
    gwrite=cgawrite;
    gtitle=cgatitle;
    break;
  case VIDEO_MODE_VGA_16:
    ginit=vgainit;
    gpal=vgapal;
    ginten=vgainten;
    gclear=vgaclear;
    ggetpix=vgagetpix;
    gputi=vgaputi;
    ggeti=vgageti;
    gputim=vgaputim;
    gwrite=vgawrite;
    gtitle=vgatitle;
  }

  video_mode=newmode;
  g_bReady=TRUE;
  gclear();
  ginit();
  gpal(0);
  reset_main_menu_screen=TRUE;
}

HRESULT WINAPI EnumModesCallback(LPDDSURFACEDESC lpddsd,LPVOID lpContext)
{
  struct FULLSCREEN_VID_MODE_INFO* vmode;

  if (lpddsd->dwHeight>=400)
  {
    if (supported_video_modes==NULL)
    {
      vmode=supported_video_modes=malloc(sizeof(struct FULLSCREEN_VID_MODE_INFO));
    }
    else
    {
      vmode=supported_video_modes;
      while(vmode->next_mode!=NULL)
      {
        vmode=vmode->next_mode;
      }
      vmode->next_mode=malloc(sizeof(struct FULLSCREEN_VID_MODE_INFO));
      vmode=vmode->next_mode;
    }
    vmode->width=lpddsd->dwWidth;
    vmode->height=lpddsd->dwHeight;
    vmode->bpp=lpddsd->ddpfPixelFormat.dwRGBBitCount;
    vmode->next_mode=NULL;
  }
  return DDENUMRET_OK;
}

void set_preferred_vidmode(int w, int h, int bpp)
{
  preferred_video_mode.width=w;
  preferred_video_mode.height=h;
  preferred_video_mode.bpp=bpp;
  preferred_video_mode.next_mode=NULL;

  // TODO: if we are in fullscreen mode, then switch to new mode
}

BOOL LoadSpriteFile(LPSTR pszFileName)
{
   HANDLE hFile;
   BOOL bSuccess = FALSE;
   int i;


   hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
      OPEN_EXISTING, NULL, NULL);
   if(hFile != INVALID_HANDLE_VALUE)
   {
      DWORD dwFileSize;
      dwFileSize = GetFileSize(hFile, NULL);
      if(dwFileSize != 0xFFFFFFFF)
      {
         sprite_data=GlobalAlloc(GPTR, dwFileSize);
         if(sprite_data != NULL)
         {
            DWORD dwRead;
            ReadFile(hFile, sprite_data, dwFileSize, &dwRead, NULL);
         }
      }
      CloseHandle(hFile);
   }
   /*
   for (i=0;i<200;i++)
   {
     if
     sprite_array
   }
   */
   return bSuccess;
}
