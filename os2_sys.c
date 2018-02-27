#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define INCL_DOSFILEMGR   /* File Manager values */
#define INCL_DOSERRORS    /* DOS Error values    */
#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#define INCL_ERRORS
#define INCL_OS2MM        /* required for MCI and MMIO headers   */
#include <os2.h>
#define  _MEERROR_H_
#include <mmioos2.h>
#include <os2me.h>
#include <dive.h>
#include <fourcc.h>

#define _OS2_SYS_C_
#include "os2_sys.h"
#include "def.h"
#include "main.h"
#include "device.h"
#include "newsnd.h"
#include "ini.h"

extern void parsecmd(int argc,char *argv[]);

char		drf_filename[256] = "";
int		command_buffer = 0;
char		digger_dir[256];
bool		wave_device_available = TRUE;
bool		suspend_game = FALSE;
bool		start_full_screen = FALSE;
int		dx_sound_volume;
bool		g_bWindowed;

#define NUM_SOUND_BUFFERS	2

typedef struct _KEYMAPPING {
  LONG			lKeyCode;
  struct _KEYMAPPING	*pNext;
} KEYMAPPING, *PKEYMAPPING;

PKEYMAPPING aKeyMappings[19] = { NULL, NULL, NULL, NULL, NULL, NULL,
                                 NULL, NULL, NULL, NULL, NULL, NULL,
                                 NULL, NULL, NULL, NULL, NULL, NULL,
                                 NULL };

static HMQ			hmq;
static HAB			hab;
static HWND			hwndFrame		= NULLHANDLE;
static const CHAR		szClientClass[]		= "DIGGER";
static HDIVE			hDive			= NULLHANDLE;
static ULONG			ulDiveBufferNumber	= 0;
static BOOL			bDiveHaveBuffer		= FALSE;
static PBYTE			pbImgBuf		= NULL;
static const ULONG		cbImgBuf		= 640 * 400;
static BOOL			fImgBufChanged		= FALSE;
static ULONG			ulCurPalette		= 0;
static ULONG			ulCurIntensity		= 0;
static PBYTE			pbTitle			= NULL;
static BOOL			fCurFullScreen;
// ulKBBuffer: high word - chracter code, low word - scan code
static ULONG			ulKBBuffer		= 0;
static USHORT			usAudioDeviceId;
static MCI_MIXSETUP_PARMS	sMCIMixSetup;
static MCI_BUFFER_PARMS		sMCIBuffer;
static MCI_MIX_BUFFER		aMixBuffers[256];
static ULONG			ulMixBufCurTime = 0;
static ULONG			ulMixCurBuf = 0;
static PUCHAR			pucSoundBuffer = NULL;
static HMTX			hmtxMixBuf;

static BYTE	vga16_pal1_rgbq[] =	/* palette1, normal intensity */
  {   0,   0,   0, PC_RESERVED,  128,   0,   0, PC_RESERVED,
      0, 128,   0, PC_RESERVED,  128, 128,   0, PC_RESERVED,
      0,   0, 128, PC_RESERVED,  128,   0, 128, PC_RESERVED,
      0,  64, 128, PC_RESERVED,  128, 128, 128, PC_RESERVED,
     64,  64,  64, PC_RESERVED,  255,   0,   0, PC_RESERVED,
      0, 255,   0, PC_RESERVED,  255, 255,   0, PC_RESERVED,
      0,   0, 255, PC_RESERVED,  255,   0, 255, PC_RESERVED,
      0, 255, 255, PC_RESERVED,  255, 255, 255, PC_RESERVED };

static BYTE	vga16_pal1i_rgbq[] =	/* palette1, high intensity */
  {   0,   0,   0, PC_RESERVED,  255,   0,   0, PC_RESERVED,
      0, 255,   0, PC_RESERVED,  255, 255,   0, PC_RESERVED,
      0,   0, 255, PC_RESERVED,  255,   0, 255, PC_RESERVED,
      0, 128, 255, PC_RESERVED,  192, 192, 192, PC_RESERVED,
    128, 128, 128, PC_RESERVED,  255, 128, 128, PC_RESERVED,
    128, 255, 128, PC_RESERVED,  255, 255, 128, PC_RESERVED,
    128, 128, 255, PC_RESERVED,  255, 128, 255, PC_RESERVED,
    128, 255, 255, PC_RESERVED,  255, 255, 255, PC_RESERVED };

static BYTE	vga16_pal2_rgbq[] =	/* palette2, normal intensity */
  {   0,   0,   0, PC_RESERVED,    0, 128,   0, PC_RESERVED,
      0,   0, 128, PC_RESERVED,    0,  64, 128, PC_RESERVED,
    128,   0,   0, PC_RESERVED,  128, 128,   0, PC_RESERVED,
    128,   0, 128, PC_RESERVED,  128, 128, 128, PC_RESERVED,
     64,  64,  64, PC_RESERVED,    0, 255,   0, PC_RESERVED,
      0,   0, 255, PC_RESERVED,    0, 255, 255, PC_RESERVED,
    255,   0,   0, PC_RESERVED,  255, 255,   0, PC_RESERVED,
    255,   0, 255, PC_RESERVED,  255, 255, 255, PC_RESERVED };

static BYTE	vga16_pal2i_rgbq[] =	/* palette2, high intensity */
  {   0,   0,   0, PC_RESERVED,    0, 255,   0, PC_RESERVED,
      0,   0, 255, PC_RESERVED,    0, 128, 255, PC_RESERVED,
    255,   0,   0, PC_RESERVED,  255, 255,   0, PC_RESERVED,
    255,   0, 255, PC_RESERVED,  192, 192, 192, PC_RESERVED,
    128, 128, 128, PC_RESERVED,  128, 255, 128, PC_RESERVED,
    128, 128, 255, PC_RESERVED,  128, 255, 255, PC_RESERVED,
    255, 128, 128, PC_RESERVED,  255, 255, 128, PC_RESERVED,
    255, 128, 255, PC_RESERVED,  255, 255, 255, PC_RESERVED };

static BYTE	cga16_pal1_rgbq[] =	/* palette1, normal intensity */
  {   0,   0,   0, PC_RESERVED,    0, 168,   0, PC_RESERVED,
      0,   0, 168, PC_RESERVED,    0,  84, 168, PC_RESERVED,
      0,   0, 128, PC_RESERVED,  128,   0, 128, PC_RESERVED,
      0,  64, 128, PC_RESERVED,  128, 128, 128, PC_RESERVED,
     64,  64,  64, PC_RESERVED,  255,   0,   0, PC_RESERVED,
      0, 255,   0, PC_RESERVED,  255, 255,   0, PC_RESERVED,
      0,   0, 255, PC_RESERVED,  255,   0, 255, PC_RESERVED,
      0, 255, 255, PC_RESERVED,  255, 255, 255, PC_RESERVED };

static BYTE	cga16_pal1i_rgbq[] =	/* palette1, high intensity */
  {   0,   0,   0, PC_RESERVED,   85, 255,  85, PC_RESERVED,
     85,  85, 255, PC_RESERVED,   85, 255, 255, PC_RESERVED,
      0,   0, 255, PC_RESERVED,  255,   0, 255, PC_RESERVED,
      0, 128, 255, PC_RESERVED,  192, 192, 192, PC_RESERVED,
    128, 128, 128, PC_RESERVED,  255, 128, 128, PC_RESERVED,
    128, 255, 128, PC_RESERVED,  255, 255, 128, PC_RESERVED,
    128, 128, 255, PC_RESERVED,  255, 128, 255, PC_RESERVED,
    128, 255, 255, PC_RESERVED,  255, 255, 255, PC_RESERVED };

static BYTE	cga16_pal2_rgbq[] =	/* palette2, normal intensity */
  {   0,   0,   0, PC_RESERVED,    0, 128,   0, PC_RESERVED,
    128,   0, 128, PC_RESERVED,  160, 160, 160, PC_RESERVED,
    160, 160, 160, PC_RESERVED,  128, 128,   0, PC_RESERVED,
    128,   0, 128, PC_RESERVED,  128, 128, 128, PC_RESERVED,
     64,  64,  64, PC_RESERVED,    0, 255,   0, PC_RESERVED,
      0,   0, 255, PC_RESERVED,    0, 255, 255, PC_RESERVED,
    255,   0,   0, PC_RESERVED,  255, 255,   0, PC_RESERVED,
    255,   0, 255, PC_RESERVED,  255, 255, 255, PC_RESERVED };

static BYTE	cga16_pal2i_rgbq[] =	/* palette2, high intensity */
  {   0,   0,   0, PC_RESERVED,    0, 255,   0, PC_RESERVED,
      0,   0, 255, PC_RESERVED,  160, 160, 160, PC_RESERVED,
    255,   0,   0, PC_RESERVED,  255, 255,   0, PC_RESERVED,
    255,   0, 255, PC_RESERVED,  192, 192, 192, PC_RESERVED,
    128, 128, 128, PC_RESERVED,  128, 255, 128, PC_RESERVED,
    128, 128, 255, PC_RESERVED,  128, 255, 255, PC_RESERVED,
    255, 128, 128, PC_RESERVED,  255, 255, 128, PC_RESERVED,
    255, 128, 255, PC_RESERVED,  255, 255, 255, PC_RESERVED };

static PBYTE	pbPalette[4] = { &vga16_pal1_rgbq, &vga16_pal1i_rgbq,
                                 &vga16_pal2_rgbq, &vga16_pal2i_rgbq };

extern Uint3	*ascii2vga[];
extern Uint3	*ascii2cga[];
extern Uint3	*vgatable[];
extern Uint3	*cgatable[];
extern Uint5	ftime;
extern bool	soundflag,musicflag;

void initkeyb(void) {}
void restorekeyb(void) {}
void graphicsoff(void) {}
Sint5 getkips(void) { return 0; }
void gretrace(void) {}
void s0initint8(void) {}
void s0restoreint8(void) {}
void s0soundoff(void) {}
void s0setspkrt2(void) {}
void s0settimer0(Uint4 t0v) {}
void s0timer0(Uint4 t0v) {}
void s0settimer2(Uint4 t2v) {}
void s0timer2(Uint4 t2v) {}
void s0soundinitglob(void) {}
void s0soundkillglob(void) {}
void s1initint8(void) {}
void s1restoreint8(void) {}
void inittimer(void) {}
bool initsounddevice(void) { return TRUE; }
void killsounddevice(void) { }

#ifdef DEBUG_FILE

#include <time.h>
#include <stdarg.h>

static FILE		*fdDebug = NULL;

#define debug(s,...) debug_write(__FILE__"/"__func__"(): "##s"\n" ,##__VA_ARGS__)

void debugInit()
{
  fdDebug = fopen( DEBUG_FILE, "a" );
  if ( fdDebug == NULL )
    printf( "Cannot open debug file "DEBUG_FILE"\n" );
}

void debug_write(char *pcFormat, ...)
{
  va_list	arglist;
  time_t	t;
  char		acBuf[32];

  if ( fdDebug == NULL )
    return;

  t = time( NULL );
  strftime( &acBuf, sizeof(acBuf)-1, "%T", localtime( &t ) );
  fprintf( fdDebug, "[%s] ", &acBuf );
  va_start( arglist, pcFormat ); 
  vfprintf( fdDebug, pcFormat, arglist );
  va_end( arglist );
  fflush(NULL);
}

#define debugDone() fclose( fdDebug )

#else

#define debug(s,...)
#define debugInit()
#define debugDone()

#endif


bool get_open_save_filename(bool save, char* title, char* filter, char* defext, char filename[])
{
  return 0;
}

Uint5 gethrt(void)
{
  ULONG		ulTime;

  DosQuerySysInfo( QSV_MS_COUNT, QSV_MS_COUNT, &ulTime, sizeof(ULONG) );
  return ulTime * 1193;
}

Sint5 getlrt(void)
{
  return gethrt();
}


LONG APIENTRY AudioEvent(ULONG ulStatus, PMCI_MIX_BUFFER pBuffer, ULONG ulFlags)
{
  if ( (ulFlags & MIX_WRITE_COMPLETE) != 0 )
  {
    ULONG		ulIdx;
    MCI_STATUS_PARMS    sMCIStatus = { 0 };

    sMCIStatus.ulItem = MCI_STATUS_POSITION;
    mciSendCommand( usAudioDeviceId, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT,
                    &sMCIStatus, 0 );

    DosRequestMutexSem( hmtxMixBuf, SEM_INDEFINITE_WAIT );
    ulMixBufCurTime = sMCIStatus.ulReturn;

    firsts = sMCIBuffer.ulBufferSize - 1;
    while( firsts >= last )
      pucSoundBuffer[last++] = getsample();
    last = firsts = 0;

    memcpy( pBuffer->pBuffer, pucSoundBuffer, sMCIBuffer.ulBufferSize );
    DosReleaseMutexSem( hmtxMixBuf );

    sMCIMixSetup.pmixWrite( sMCIMixSetup.ulMixHandle, pBuffer, 1 );
  }

  return TRUE;
}

static VOID _dartPause(BOOL fPause)
{
  MCI_GENERIC_PARMS	sMCIGeneric = { 0 };

  if ( wave_device_available )
    mciSendCommand( usAudioDeviceId, fPause ? MCI_PAUSE : MCI_RESUME,
                    MCI_WAIT, &sMCIGeneric, 0 );
}

void s1fillbuffer(void)
{
  MCI_STATUS_PARMS	sMCIStatus = { 0 };
  ULONG			ulRC;

  if ( !wave_device_available )
    return;

  sMCIStatus.ulItem = MCI_STATUS_POSITION;
  DosRequestMutexSem( hmtxMixBuf, SEM_INDEFINITE_WAIT );

  ulRC = mciSendCommand( usAudioDeviceId, MCI_STATUS,
                         MCI_STATUS_ITEM | MCI_WAIT, &sMCIStatus, 0 );
  firsts = ( sMCIStatus.ulReturn - ulMixBufCurTime ) *
           sMCIMixSetup.ulSamplesPerSec / 1000;
  firsts %= sMCIBuffer.ulBufferSize; // so dirty :(

  while( firsts >= last )
    pucSoundBuffer[last++] = getsample();

  DosReleaseMutexSem( hmtxMixBuf );
}

bool setsounddevice(int base,int irq,int dma,Uint4 samprate,Uint4 bufsize)
{
  MCI_AMP_OPEN_PARMS	sMCIAmpOpen;
  ULONG			ulRC, ulIdx;
  CHAR			acBuf[128];

  wave_device_available = FALSE;

  pucSoundBuffer = malloc( bufsize );
  if ( pucSoundBuffer == NULL )
  {
    debug( "Not enough memory" );
    return FALSE;
  }

  memset( pucSoundBuffer, ( MIN_SAMP + MAX_SAMP ) >> 1, bufsize );

  if ( DosCreateMutexSem( NULL, &hmtxMixBuf, 0, FALSE ) != NO_ERROR )
  {
    debug( "DosCreateMutexSem() failed" );
    return FALSE;
  }

  memset( &sMCIAmpOpen, 0, sizeof(MCI_AMP_OPEN_PARMS) );
  sMCIAmpOpen.usDeviceID = 0;
  sMCIAmpOpen.pszDeviceType = (PSZ)MCI_DEVTYPE_AUDIO_AMPMIX;
  ulRC = mciSendCommand( 0, MCI_OPEN,
                         MCI_WAIT | MCI_OPEN_TYPE_ID | MCI_OPEN_SHAREABLE,
                         &sMCIAmpOpen,  0 );
  if ( ulRC != MCIERR_SUCCESS )
  {
    mciGetErrorString( ulRC, &acBuf, sizeof(acBuf) );
    debug( "MCI_OPEN: %s", &acBuf );
    return FALSE;
  }

  usAudioDeviceId = sMCIAmpOpen.usDeviceID;

  memset( &sMCIMixSetup, 0, sizeof(MCI_MIXSETUP_PARMS) );
  sMCIMixSetup.ulBitsPerSample = 8;
  sMCIMixSetup.ulFormatTag     = MCI_WAVE_FORMAT_PCM;
  sMCIMixSetup.ulSamplesPerSec = samprate;
  sMCIMixSetup.ulChannels      = 1;
  sMCIMixSetup.ulFormatMode    = MCI_PLAY;
  sMCIMixSetup.ulDeviceType    = MCI_DEVTYPE_WAVEFORM_AUDIO;
  sMCIMixSetup.pmixEvent       = AudioEvent;

  ulRC = mciSendCommand( usAudioDeviceId, MCI_MIXSETUP,
                         MCI_WAIT | MCI_MIXSETUP_INIT,
                         &sMCIMixSetup, 0 );
  if ( ulRC != MCIERR_SUCCESS )
  {
    mciGetErrorString( ulRC, &acBuf, sizeof(acBuf) );
    debug( "MCI_MIXSETUP: %s", &acBuf );
    return FALSE;
  }

  sMCIBuffer.ulNumBuffers = NUM_SOUND_BUFFERS;
  sMCIBuffer.ulBufferSize = bufsize;
  sMCIBuffer.pBufList = &aMixBuffers;
  ulRC = mciSendCommand( usAudioDeviceId, MCI_BUFFER,
                         MCI_WAIT | MCI_ALLOCATE_MEMORY, &sMCIBuffer, 0 );
  if ( ulRC != MCIERR_SUCCESS )
  {
    mciGetErrorString( ulRC, &acBuf, sizeof(acBuf) );
    debug( "MCI_BUFFER: %s", &acBuf );
    return FALSE;
  }

  // Fill all device buffers with data.
  for( ulIdx = 0; ulIdx < sMCIBuffer.ulNumBuffers; ulIdx++)
  {
    aMixBuffers[ulIdx].ulFlags        = 0;
    aMixBuffers[ulIdx].ulBufferLength = sMCIBuffer.ulBufferSize; 

    memset( aMixBuffers[ulIdx].pBuffer, ( MIN_SAMP + MAX_SAMP ) >> 1,
            sMCIBuffer.ulBufferSize );
  }

  // Write buffers to kick off the amp mixer.
  ulRC = sMCIMixSetup.pmixWrite( sMCIMixSetup.ulMixHandle, &aMixBuffers,
                                 sMCIBuffer.ulNumBuffers );
  if ( ulRC != MCIERR_SUCCESS )
  {
    debug( "sMixSetupParms.pmixWrite(), rc = %u", ulRC );
    return FALSE;
  }  

  wave_device_available = TRUE;
  return TRUE;
}

void set_sound_volume(int new_volume)
{
  MCI_SET_PARMS		sMCISet = { 0 };

  if ( !wave_device_available )
    return;

  sMCISet.ulAudio = MCI_SET_AUDIO_ALL;
  sMCISet.ulLevel = min( 100, new_volume );
  mciSendCommand( usAudioDeviceId, MCI_SET,
                  MCI_WAIT | MCI_SET_AUDIO | MCI_SET_VOLUME, &sMCISet, 0 );
}

void pause_windows_sound_playback()
{
  _dartPause( TRUE );
}

void resume_windows_sound_playback()
{
  _dartPause( FALSE );
}



bool key_pressed(int function,bool check_current_state_only)
// checks to see if any of the keys mapped to a specific function have been
// preseed.
{
  PKEYMAPPING		pCur;
  LONG			lKeyState;

  pCur = aKeyMappings[function];
  while( pCur )
  {
    lKeyState = WinGetPhysKeyState( HWND_DESKTOP, pCur->lKeyCode );

    if ( check_current_state_only )
      lKeyState = lKeyState & 0x8000;

    if ( lKeyState != 0 )
      return TRUE;

    pCur = pCur->pNext;
  }

  return FALSE;
}

int translate_key_code_to_function(int code)
{
  ULONG			ulIdx;
  PKEYMAPPING		pCur;

  code &= 0xFF;
  for( ulIdx = 10; ulIdx < 19; ulIdx++ )
  {
    pCur = aKeyMappings[ulIdx];
    while( pCur )
    {
      if ( pCur->lKeyCode == code )
        return ulIdx + 1;

      pCur = pCur->pNext;
    }
  }

  return 0;
}

Sint4 getcommand(void)
// similar to getkey() but the value returned has already been translated from
// the keycode to the function/command that the key is mapped to
{
  Sint4		temp_buffer = command_buffer;

  command_buffer = 0;

  return temp_buffer;
}

void clear_mapped_keys(int idx)
{
  PKEYMAPPING		pCur, pPrev;

  /* remove existing keys mapped to this function */
  pCur = aKeyMappings[idx];
  aKeyMappings[idx] = NULL;
  while( pCur != NULL )
  {
    pPrev = pCur;
    pCur = pCur->pNext;
    free( pPrev );
  }
}

void add_mapped_key(int idx, int key_code)
{
  PKEYMAPPING		pCur;

  if ( key_code == 0 )  //if not a valid Virtual Key Code then return
    return;

  if ( aKeyMappings[idx] != NULL )
  {
    pCur = aKeyMappings[idx];
    while( pCur->pNext != NULL )
      pCur = pCur->pNext;

    pCur->pNext = malloc( sizeof(KEYMAPPING) );
    pCur = pCur->pNext;
  }
  else
    pCur = aKeyMappings[idx] = malloc( sizeof(KEYMAPPING) );

  pCur->lKeyCode = key_code;
  pCur->pNext = NULL;
}

Sint4 getkey(void)
// returns the Scan Key Code of the last key pressed
{
  ULONG		ulTempBuffer; // low word - scan code

  do
  {
    if ( ulKBBuffer != 0 )
    {
      ulTempBuffer = ulKBBuffer;
      ulKBBuffer = 0;
      return ulTempBuffer & 0xFFFF;
    }
    do_windows_events();
  }
  while( TRUE );
}

UINT getkey_ch(void)
// returns the Caracter Code of the last key pressed
{
  ULONG		ulTempBuffer; // high word - chracter code

  do
  {
    if ( ulKBBuffer != 0 )
    {
      ulTempBuffer = ulKBBuffer;
      ulKBBuffer = 0;
      return (ulTempBuffer >> 16) & 0xFFFF;
    }
    do_windows_events();
  }
  while( TRUE );
}

bool kbhit(void)
{
  return ( ulKBBuffer != 0 );
}


static VOID _titleBitmapLoad(LONG lId)
{
  HBITMAP		hbmTitle, hbmOld;
  LONG			lScans;
  BITMAPINFOHEADER2	sInfHdr;
  PBYTE			pbBuffer;	// bit-map data buffer
  ULONG			cbBuffer;
  PBITMAPINFO2		pInfo;		// info structure        
  ULONG			cbInfo;
  SIZEL			sSize = { 0 };
  HDC			hdcBitmap = DevOpenDC( hab, OD_MEMORY, "*", 0, NULL, NULLHANDLE );
  HPS			hpsBitmap = GpiCreatePS( hab, hdcBitmap, &sSize,
                            PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC );
  PBYTE			pbDst, pbSrc;

  hbmTitle = GpiLoadBitmap( hpsBitmap, NULLHANDLE, lId, 0, 0 );
  if ( hbmTitle == GPI_ERROR )
  {
    debug( "Cannot load bitmap (Id: %u)", IDB_TITLEBITMAP );
    return;
  }

  hbmOld = GpiSetBitmap( hpsBitmap, hbmTitle );
  sInfHdr.cbFix = sizeof(BITMAPINFOHEADER2);
  if ( !GpiQueryBitmapInfoHeader( hbmTitle, &sInfHdr ) )
  {
    debug( "GpiQueryBitmapInfoHeader() failed" );
    return;
  }

  if ( sInfHdr.cx != 640 || sInfHdr.cy != 400 )
  {
    debug( "Bitmap #%u width: %u, height: %u. Must be 640x400.",
           sInfHdr.cx, sInfHdr.cy );
    return;
  }

  if ( sInfHdr.cBitCount != 4 )
  {
    debug( "Bitmap #%u bits per pixel: %u. Must be 4.", sInfHdr.cBitCount );
    return;
  }

//  debug( "[Title bitmap %u] width: %u, height: %u, planes: %u, bits per pel: %u",
//         lId, sInfHdr.cx, sInfHdr.cy, sInfHdr.cPlanes, sInfHdr.cBitCount );

  cbBuffer = (((sInfHdr.cBitCount * sInfHdr.cx) + 31) / 32) * 4
                       * sInfHdr.cy * sInfHdr.cPlanes;
  pbBuffer = malloc( cbBuffer );

  cbInfo = sizeof(BITMAPINFO2) + ( sizeof(RGB) * 256 );
  pInfo = malloc( cbInfo );
  memcpy( pInfo, &sInfHdr, sizeof(BITMAPINFOHEADER2) );

  lScans = GpiQueryBitmapBits( hpsBitmap, 0, sInfHdr.cy, pbBuffer, pInfo );
  if ( lScans == GPI_ALTERROR )
  {
    debug( "GpiQueryBitmapBits() failed" );
    return;
  }

  free( pInfo );
  GpiSetBitmap( hpsBitmap, hbmOld );
  if ( !GpiDeleteBitmap( hbmTitle ) )
    debug( "GpiDeleteBitmap() failed" );
  if ( !GpiDestroyPS( hpsBitmap ) )
    debug( "GpiDestroyPS() failed" );
  if ( DevCloseDC( hdcBitmap ) == DEV_ERROR )
    debug( "DevCloseDC() failed" );


  if ( pbTitle != NULL )
    free( pbTitle );

  pbTitle = malloc( cbImgBuf );

  pbSrc = pbBuffer;
  pbDst = &pbTitle[399 * 640]; // Last line of source bitmap
  do
  {
    for( lScans = 0; lScans < 640/2; lScans++ )
    {
      pbDst[0] = *pbSrc >> 4;
      pbDst[1] = *pbSrc & 0x0F;
      pbDst += 2;
      pbSrc++;
    }
    pbDst -= (640 * 2); // Move back to the begin of next line to fill
  }
  while( pbDst > pbTitle );
  free( pbBuffer );
}

void vgainit(void)
{
  pbPalette[0] = &vga16_pal1_rgbq;
  pbPalette[1] = &vga16_pal1i_rgbq;
  pbPalette[2] = &vga16_pal2_rgbq;
  pbPalette[3] = &vga16_pal2i_rgbq;

  _titleBitmapLoad( IDB_TITLEBITMAP );
}

void vgaclear(void)
{
  memset( pbImgBuf, 0, cbImgBuf );
  fImgBufChanged = TRUE;
}

void vgapal(Sint4 pal)
{
  ulCurPalette = pal;
  DiveSetSourcePalette( hDive, 0, 16,
                        pbPalette[ulCurPalette * 2 + ulCurIntensity] );
  fImgBufChanged = TRUE;
}

void vgainten(Sint4 inten)
{
  ulCurIntensity = inten;
  DiveSetSourcePalette( hDive, 0, 16,
                        pbPalette[ulCurPalette * 2 + ulCurIntensity] );
  fImgBufChanged = TRUE;
}

void vgaputi(Sint4 x,Sint4 y,Uint3 *p,Sint4 w,Sint4 h)
{
  ULONG		ulIdx;

  for( ulIdx = 0; ulIdx < h * 2; ulIdx++ )
    if ( ulIdx + y * 2 < 400 )
      memcpy( &pbImgBuf[ (ULONG)(( y * 2 + ulIdx ) * 640L + x * 2 ) ],
              &p[ (ULONG)( ulIdx * w * 8L ) ], w * 8L );

  fImgBufChanged = TRUE;
}

void vgageti(Sint4 x,Sint4 y,Uint3 *p,Sint4 w,Sint4 h)
{
  ULONG		ulIdx;

  for( ulIdx = 0; ulIdx < h * 2; ulIdx++ )
    if ( ulIdx + y * 2 < 400 )
      memcpy( &p[ ulIdx * w * 8L ],
              &pbImgBuf[ (ULONG)(( y * 2L + ulIdx ) * 640L + x * 2L ) ], w * 8 );
}

void vgaputim(Sint4 x,Sint4 y,Sint4 ch,Sint4 w,Sint4 h)
{
  ULONG		ulX, ulY, ulIdx, ulPlane, ulColor;
  PBYTE		pbImgBufMax = &pbImgBuf[cbImgBuf];
  PUCHAR	pucSrcMask = vgatable[(ch * 2) + 1];
  PUCHAR	pucSrc = vgatable[ch * 2];
  PUCHAR	pucDest = &pbImgBuf[ (ULONG)( y * 2L * 640L + x * 2L ) ];
  ULONG		ulNextRowOffs = 640 - ( w * 8L );
  ULONG		ulSrcPlaneOffs = w * h * 2;

  for( ulY = 0; ulY < (h * 2); ulY++ )
  {
    for( ulX = 0; ulX < w; ulX++ )
    {
      for( ulIdx = 0; ulIdx < 8; ulIdx++ )
      {
        if ( ( *pucSrcMask & (0x80 >> ulIdx) ) == 0 )
        {
          ulColor = 0;

          for( ulPlane = 0; ulPlane < 4; ulPlane++ )
            ulColor |= ( ( pucSrc[ulPlane * ulSrcPlaneOffs] << ulIdx ) & 0x80 ) >> ( 4 + ulPlane );

          if ( pucDest < pbImgBufMax )
            *pucDest = ulColor;
        }
        pucDest++;
      }
      pucSrc++;
      pucSrcMask++;
    }
    pucDest += ulNextRowOffs;
  }

  fImgBufChanged = TRUE;
}

Sint4 vgagetpix(Sint4 x,Sint4 y)
{
  ULONG		ulX, ulY;
  Sint4		ret = 0;

  if ( x > 319 || y > 199 )
    return 0xFF;

  for( ulY = 0; ulY < 2; ulY++ )
    for( ulX = 0; ulX < 8; ulX++ )
      if ( pbImgBuf[ (ULONG)( (y * 2L + ulY) * 640L + x * 2L + ulX ) ] != 0 )
        ret |= 0x80 >> ulX;

  return ( ret & 0xEE );
}

void vgatitle(void)
{
  memcpy( pbImgBuf, pbTitle, cbImgBuf );
}

void vgawrite(Sint4 x,Sint4 y,Sint4 ch,Sint4 c)
{
  ULONG		ulY, ulX, ulColor;

  ch -= 32;

  if ( ch < 0x5f )
  {
    if ( ascii2vga[ch] )
    {
      for( ulY = 0; ulY < 24; ulY++ )
      {
        for( ulX = 0; ulX < 24; ulX++ )
        {
          ulColor = ascii2vga[ch][ulY * 12 + ( ulX >> 1 )];
          if ( (ulX & 0x1) != 0 )
            ulColor &= 0x0F;
          else
            ulColor >>= 4;

          if ( ulColor == 10 )
          {
            if ( c == 2 )
              ulColor = 12;
            else if ( c == 3 )
              ulColor = 14;
          }
          else if ( ulColor == 12 )
            if ( c == 1 )
              ulColor = 2;
            else if ( c == 2 )
              ulColor = 4;
            else if ( c == 3 )
              ulColor = 6;

          pbImgBuf[ ( y * 2 + ulY ) * 640L + x * 2L + ulX ] = ulColor;
        }
      }
    }
    else    /* draw a space (needed when reloading and displaying high scores when user switches to/from normal/gauntlet mode, etc ) */
      for( ulY = 0; ulY < 24; ulY++ )
        memset( &pbImgBuf[ ( y * 2 + ulY ) * 640 + x * 2 ], 0, 24 );

    fImgBufChanged = TRUE;
  }
}


void cgainit(void)
{
  pbPalette[0] = &cga16_pal1_rgbq;
  pbPalette[1] = &cga16_pal1i_rgbq;
  pbPalette[2] = &cga16_pal2_rgbq;
  pbPalette[3] = &cga16_pal2i_rgbq;

  _titleBitmapLoad( IDB_CGATITLEBITMAP );
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

void cgaputi(Sint4 x,Sint4 y,Uint3 *p,Sint4 w,Sint4 h)
{
  vgaputi(x,y,p,w,h);
}

void cgageti(Sint4 x,Sint4 y,Uint3 *p,Sint4 w,Sint4 h)
{
  vgageti(x,y,p,w,h);
}

void cgaputim(Sint4 x,Sint4 y,Sint4 ch,Sint4 w,Sint4 h)
{
  ULONG		ulX, ulY, ulIdx, ulColor;
  PBYTE		pbImgBufMax = &pbImgBuf[cbImgBuf];
  PUCHAR	pucSrcMask = cgatable[(ch * 2) + 1];
  PUCHAR	pucSrc = cgatable[ch * 2];
  PUCHAR	pucDest = &pbImgBuf[ (ULONG)( y * 2L * 640L + x * 2L ) ];

  for( ulY = 0; ulY < h; ulY++ )
  {
    for( ulX = 0; ulX < w; ulX++ )
    {
      for( ulIdx = 0; ulIdx < 4; ulIdx++ )
      {
        if ( ( *pucSrcMask & (0xC0 >> (ulIdx*2)) ) == 0 )
        {
          ulColor = ( *pucSrc >> ( 6 - ( ulIdx * 2 ) ) ) & 0x03;
          if ( pucDest < pbImgBufMax )
            pucDest[0] = pucDest[1] = pucDest[640] = pucDest[641] = ulColor;
        }
        pucDest += 2;
      }
      pucSrc++;
      pucSrcMask++;
    }
    pucDest += (640 * 2) - (w * 8);
  }

  fImgBufChanged = TRUE;
}

Sint4 cgagetpix(Sint4 x,Sint4 y)
{
  return vgagetpix(x,y);
}

void cgatitle(void)
{
  vgatitle();
}

void cgawrite(Sint4 x,Sint4 y,Sint4 ch,Sint4 c)
{
  ULONG		ulY, ulX, ulColor, ulOffs;

  ch -= 32;

  if ( ch < 0x5f )
  {
    if ( ascii2cga[ch] )
    {
      for( ulY = 0; ulY < 12; ulY++ )
      {
        for( ulX = 0; ulX < 12; ulX++ )
        {
          ulColor = ascii2cga[ch][ulY * 6 + ( ulX >> 1 )];
          if ( (ulX & 0x1) != 0 )
            ulColor &= 0x0F;
          else
            ulColor >>= 4;

          ulColor &= c;
          ulOffs = ( y * 2 + ulY * 2 ) * 640L + x * 2L + ulX * 2;
          pbImgBuf[ulOffs] = pbImgBuf[ulOffs + 1] = ulColor;
          ulOffs = ( y * 2 + ulY * 2 + 1 ) * 640L + x * 2L + ulX * 2;
          pbImgBuf[ulOffs] = pbImgBuf[ulOffs + 1] = ulColor;
        }
      }
    }
    else    /* draw a space (needed when reloading and displaying high scores when user switches to/from normal/gauntlet mode, etc ) */
      for( ulY = 0; ulY < 24; ulY++ )
        memset( &pbImgBuf[ ( y * 2 + ulY ) * 640 + x * 2 ], 0, 24 );

    fImgBufChanged = TRUE;
  }
}



void windows_finish(void)
{
  ULONG			ulRC;
  CHAR			acBuf[128];
  MCI_GENERIC_PARMS	sMCIGeneric = { 0 };

  /* save the current settings */
  if ( diggers > 1 )
    WriteINIString( INI_GAME_SETTINGS, "Players", "2S", ININAME );
  else
    WriteINIInt( INI_GAME_SETTINGS, "Players", nplayers, ININAME );
  WriteINIBool( INI_GAME_SETTINGS, "GauntletMode", gauntlet, ININAME );
  WriteINIInt(INI_GAME_SETTINGS, "GauntletTime", gtime, ININAME);
  WriteINIInt( INI_GAME_SETTINGS, "Speed", ftime, ININAME );
  WriteINIInt( INI_GAME_SETTINGS,"StartLevel", startlev, ININAME );

  WriteINIBool( INI_SOUND_SETTINGS, "SoundOn", soundflag, ININAME );
  WriteINIBool( INI_SOUND_SETTINGS, "MusicOn", musicflag, ININAME );
  WriteINIInt( INI_SOUND_SETTINGS, "SoundVolume", dx_sound_volume, ININAME );

  WriteINIBool( INI_GRAPHICS_SETTINGS, "FullScreen", !g_bWindowed, ININAME );
  WriteINIBool( INI_GRAPHICS_SETTINGS, "CGA", (pbPalette[0]==&cga16_pal1_rgbq),
                ININAME );

  if ( wave_device_available )
  {
    ulRC = mciSendCommand( usAudioDeviceId, MCI_STOP, MCI_WAIT,
                           &sMCIGeneric, 0 );
    if ( ulRC != MCIERR_SUCCESS )
    {
      mciGetErrorString( ulRC, &acBuf, sizeof(acBuf) );
      debug( "MCI_STOP: %s", &acBuf );
    }

    ulRC = mciSendCommand( usAudioDeviceId, MCI_BUFFER,
                           MCI_WAIT | MCI_DEALLOCATE_MEMORY, &sMCIBuffer, 0 );
    if ( ulRC != MCIERR_SUCCESS )
    {
      mciGetErrorString( ulRC, &acBuf, sizeof(acBuf) );
      debug( "MCI_BUFFER: %s", &acBuf );
    }

    ulRC = mciSendCommand( usAudioDeviceId, MCI_MIXSETUP,
                           MCI_WAIT | MCI_MIXSETUP_DEINIT, &sMCIMixSetup, 0 );
    if ( ulRC != MCIERR_SUCCESS )
    {
      mciGetErrorString( ulRC, &acBuf, sizeof(acBuf) );
      debug( "MCI_MIXSETUP: %s", &acBuf );
    }

    ulRC = mciSendCommand( usAudioDeviceId, MCI_CLOSE, MCI_WAIT,
                           &sMCIGeneric, 0 );
    if ( ulRC != MCIERR_SUCCESS )
    {
      mciGetErrorString( ulRC, &acBuf, sizeof(acBuf) );
      debug( "MCI_CLOSE: %s", &acBuf );
    }

    DosCloseMutexSem( hmtxMixBuf );
    debug( "DART closed" );
  }

  if ( pucSoundBuffer != NULL )
    free( pucSoundBuffer );

//  WinSetVisibleRegionNotify( hwndClient, FALSE );

  if ( bDiveHaveBuffer )
  {
    DiveFreeImageBuffer( hDive, ulDiveBufferNumber );
    ulDiveBufferNumber = 0;
    bDiveHaveBuffer = FALSE;
  };

  if ( hDive != NULLHANDLE )
    DiveClose( hDive );

  DosFreeMem( (PPVOID)&pbImgBuf );

  WinDestroyWindow( hwndFrame );
  WinDestroyMsgQueue( hmq );
  WinTerminate( hab );

  if ( pbTitle != NULL )
    free( pbTitle );

  debugDone();
}

void do_windows_events(void)
{
  QMSG		qmsg;

  if ( WinPeekMsg( hab, &qmsg, 0, 0, 0, PM_REMOVE ) )
  {
    if ( qmsg.msg == WM_QUIT )
    {
      windows_finish();
      exit( 0 );
    }
    WinDispatchMsg( hab, &qmsg );
  }
  else
  {
    if ( fImgBufChanged )
    {
      DiveBlitImage( hDive, ulDiveBufferNumber, DIVE_BUFFER_SCREEN );
      fImgBufChanged = FALSE;
    }
    DosSleep( 1 );
  }
}


static VOID _createWindow(BOOL fWindowed)
{
  HWND		hwndClient;
  ULONG		flFrameFlags;

  if ( hwndFrame != NULLHANDLE )
    WinDestroyWindow( hwndFrame );

  if ( fWindowed )
    flFrameFlags = FCF_TITLEBAR | FCF_DLGBORDER | FCF_MINBUTTON | FCF_ICON |
                   FCF_MAXBUTTON | FCF_TASKLIST | FCF_SYSMENU | FCF_SIZEBORDER;
  else
    flFrameFlags = FCF_TASKLIST;

  hwndFrame = WinCreateStdWindow( HWND_DESKTOP, 0, &flFrameFlags,
                szClientClass, "Digger", 0, 0, ID_APP_WINDOW, &hwndClient );
  WinSetVisibleRegionNotify( hwndClient, TRUE );
  WinPostMsg( hwndFrame, WM_VRNENABLED, 0L, 0L );

  if ( fWindowed )
  {
    LONG		lBorderWidth, lBorderHeight, lTitlebarHeight;
    LONG		lcxWindowPos, lcyWindowPos, lTotalWidth, lTotalHeight;
  //  LONG		lMenuHeight     = (LONG)WinQuerySysValue( HWND_DESKTOP, SV_CYMENU );

    lBorderWidth    = (LONG)WinQuerySysValue( HWND_DESKTOP, SV_CXDLGFRAME );
    lBorderHeight   = (LONG)WinQuerySysValue( HWND_DESKTOP, SV_CYDLGFRAME );
    lTitlebarHeight = (LONG)WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR );

    lTotalWidth  = 640 + lBorderWidth * 2;
    lTotalHeight = 482 + (lBorderHeight * 2) + lTitlebarHeight/* + lMenuHeight*/;
    lcxWindowPos   = ( (LONG)WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN )
                         - lTotalWidth ) / 2;
    lcyWindowPos   = ( (LONG)WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN )
                         - lTotalHeight ) / 2;

    WinSetWindowPos( hwndFrame, HWND_TOP, lcxWindowPos, lcyWindowPos,
                     lTotalWidth, lTotalHeight,
                     SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ACTIVATE | SWP_ZORDER );
  }
  else
    WinSetWindowPos( hwndFrame, HWND_TOP, 0, 0, 0, 0,
                     SWP_ACTIVATE | SWP_SHOW | SWP_MAXIMIZE );

  g_bWindowed = fWindowed;
}

static VOID _calcFrameRect(PRECTL prctlDst, ULONG ulWidth, ULONG ulHeight)
{
#define		ASPECT_RATIO_W		640
#define		ASPECT_RATIO_H		480
  ULONG		ulDstW, ulDstH;

  if ( ulWidth * ASPECT_RATIO_H > ulHeight * ASPECT_RATIO_W )
  {
    ulDstH = ulHeight;
    ulDstW = ( ulHeight * ASPECT_RATIO_W ) / ASPECT_RATIO_H;
  }
  else
  {
    ulDstW = ulWidth;
    ulDstH = ( ulWidth * ASPECT_RATIO_H ) / ASPECT_RATIO_W;
  }

  prctlDst->xLeft = ( ulWidth - ulDstW ) / 2;
  prctlDst->yBottom = ( ulHeight - ulDstH ) / 2;
  prctlDst->xRight = prctlDst->xLeft + ulDstW;
  prctlDst->yTop = prctlDst->yBottom + ulDstH;
}

MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  HPS                hps;
  POINTL             pointl;         // Point to offset from Desktop
  SWP                swp;            // Window position
  HRGN               hrgn;           // Region handle
  RECTL              rcls[50];       // Rectangle coordinates
  RGNRECT            rgnCtl;         // Processing control structure
  SETUP_BLITTER      SetupBlitter;   // structure for DiveSetupBlitter
  SIZEL              sizl;
  RECTL              sRectl;
  HPS                hpsClient;
  HRGN               hrgnPaint, hrgnFrame, hrgnFill;
  LONG               lRC;
  static RECTL       sFrameRect;

  switch (msg)
  {
    case WM_CREATE:
      sizl.cx = sizl.cy = 0;
      hpsClient = GpiCreatePS( hab, WinOpenWindowDC( hwnd ), &sizl,
                          PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC );
      if ( !WinSetWindowULong( hwnd, 0, hpsClient ) )
        debug( "WinSetWindowULong() fail" );

      memset( &sFrameRect, 0, sizeof(RECTL) );
//      DiveSetDestinationPalette( hDive, 0, 16, 0 );
//      DiveSetSourcePalette( hDive, 0, 16, &vga16_pal1_rgbq );
      return 0;

    case WM_DESTROY:
      hpsClient = WinQueryWindowULong( hwnd, 0 );
      if ( hpsClient != NULLHANDLE )
        GpiDestroyPS( hpsClient );
      return 0;

    case WM_PAINT:
      hps = WinBeginPaint( hwnd, NULLHANDLE, &sRectl );

      hrgnPaint = GpiCreateRegion( hps, 1, &sRectl );
      hrgnFrame = GpiCreateRegion( hps, 1, &sFrameRect );
      hrgnFill = GpiCreateRegion( hps, 0, NULL );
      GpiCombineRegion( hps, hrgnFill, hrgnPaint, hrgnFrame, CRGN_DIFF );

      GpiSetColor( hps, CLR_BLACK );
      GpiSetPattern( hps, PATSYM_SOLID );
      GpiPaintRegion( hps, hrgnFill );

      GpiDestroyRegion( hps, hrgnFill );
      GpiDestroyRegion( hps, hrgnFrame );
      GpiDestroyRegion( hps, hrgnPaint );

      // Blit DIVE buffer to screen.
      DiveBlitImage( hDive, ulDiveBufferNumber, DIVE_BUFFER_SCREEN );
      fImgBufChanged = FALSE;

      WinEndPaint( hps );
      return 0;

    case WM_REALIZEPALETTE:
      // This tells DIVE that the physical palette may have changed.
      DiveSetDestinationPalette( hDive, 0, 256, 0 );
      break;

    case WM_VRNDISABLED:
      DiveSetupBlitter( hDive, 0 );
      break;

    case WM_MOVE:
      WinPostMsg( hwnd, WM_VRNENABLED, 0L, 0L);
      WinPostMsg( hwnd, WM_PAINT, 0L, 0L);
      break;

    case WM_VRNENABLED:
      hpsClient = WinQueryWindowULong( hwnd, 0 );
      if ( !hpsClient )
         break;

      hrgn = GpiCreateRegion( hpsClient, 0L, NULL );
      if ( hrgn )
      {
        WinQueryVisibleRegion( hwnd, hrgn );
        rgnCtl.ircStart     = 0;
        rgnCtl.crc          = 50;
        rgnCtl.ulDirection  = 1;

        // Get the all rectangles
        if ( GpiQueryRegionRects( hpsClient, hrgn, NULL, &rgnCtl, rcls ) )
        {
          // Now find the window position and size, relative to parent.
          WinQueryWindowPos( hwnd, &swp );
          // Convert the point to offset from desktop lower left.
//          pointl.x = swp.x;
//          pointl.y = swp.y;
//          WinMapWindowPoints( hwndFrame, HWND_DESKTOP, &pointl, 1 );

          pointl.x = 0;
          pointl.y = 0;
          WinMapWindowPoints( hwnd, HWND_DESKTOP, &pointl, 1 );

          // Tell DIVE about the new settings.
          SetupBlitter.ulStructLen = sizeof(SETUP_BLITTER);
          SetupBlitter.fccSrcColorFormat = FOURCC_LUT8;
          SetupBlitter.ulSrcWidth   = 640;
          SetupBlitter.ulSrcHeight  = 400;
          SetupBlitter.ulSrcPosX    = 0;
          SetupBlitter.ulSrcPosY    = 0;
          SetupBlitter.fInvert      = FALSE;
          SetupBlitter.ulDitherType = 1;
          SetupBlitter.fccDstColorFormat = FOURCC_SCRN;

          _calcFrameRect( &sFrameRect, swp.cx, swp.cy );
          SetupBlitter.ulDstWidth        = sFrameRect.xRight - sFrameRect.xLeft;
          SetupBlitter.ulDstHeight       = sFrameRect.yTop - sFrameRect.yBottom;
          SetupBlitter.lDstPosX          = sFrameRect.xLeft;
          SetupBlitter.lDstPosY          = sFrameRect.yBottom;
          SetupBlitter.lScreenPosX       = pointl.x;
          SetupBlitter.lScreenPosY       = pointl.y;
          SetupBlitter.ulNumDstRects     = rgnCtl.crcReturned;
          SetupBlitter.pVisDstRects      = rcls;
          DiveSetupBlitter( hDive, &SetupBlitter );
        }
        else
          DiveSetupBlitter( hDive, 0 );

        GpiDestroyRegion( hpsClient, hrgn );
      }
      break;

    case WM_CHAR:
      if ( ( (USHORT)SHORT1FROMMP( mp1 ) & KC_KEYUP ) != 0 )
        break;

      if ( ((USHORT)SHORT1FROMMP( mp1 ) & KC_VIRTUALKEY) != 0 &&
           ( (USHORT)SHORT2FROMMP( mp2 ) == VK_F3 ) )
      {
        _createWindow( !g_bWindowed );
        return 0;
      }

      if ( ((USHORT)SHORT1FROMMP( mp1 ) & KC_SCANCODE) != 0 )
      {
        USHORT		usScanCode = (UCHAR)CHAR4FROMMP( mp1 );

        if ( ((USHORT)SHORT1FROMMP( mp1 ) & KC_ALT) != 0 )
        {
          switch( usScanCode )
          {
            case 0x4E: // gray '+'
              if ( dx_sound_volume < 100 )
              {
                dx_sound_volume += min( 100 - dx_sound_volume, 10 );
                set_sound_volume( dx_sound_volume );
              }
              return 0;

            case 0x4A: // gray '-'
              if ( dx_sound_volume > 0 )
              {
                dx_sound_volume -= min( dx_sound_volume, 10 );
                set_sound_volume( dx_sound_volume );
              }
              return 0;
          }
        }

        ulKBBuffer = usScanCode;
        if ( ((USHORT)SHORT1FROMMP( mp1 ) & KC_CHAR) != 0 )
          ulKBBuffer |= (USHORT)SHORT1FROMMP( mp2 ) << 16;
      }
/*
      fKeyUp = ( (USHORT)SHORT1FROMMP( mp1 ) & KC_KEYUP ) == 0;
      usKeyFlags = (USHORT)SHORT1FROMMP( mp1 ); // KC_SCANCODE, KC_CHAR, KC_VIRTUALKEY
      ucKeyScanCode = (UCHAR)CHAR4FROMMP( mp1 );
      usKeyChar = (USHORT)SHORT1FROMMP( mp2 );
      usKeyVirtualKey = (USHORT)SHORT2FROMMP( mp2 );
*/
      break;

    case WM_SETFOCUS:
      suspend_game = (USHORT)SHORT1FROMMP( mp2 ) == 0;
      _dartPause( suspend_game );
      break;
  }

  return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

bool IsCharAlphaNumeric(UINT c)
{
  return isalnum( c );
}



static BOOL _initializeDive()
{
  // Get the screen capabilities, and if the system supports only 16 colors
  //  the program should be terminated.
  DIVE_CAPS	DiveCaps		= {0};
  FOURCC	fccFormats[100]		= {0};

  DiveCaps.pFormatData    = fccFormats;
  DiveCaps.ulFormatLength = 100;
  DiveCaps.ulStructLen    = sizeof(DIVE_CAPS);

  if ( DiveQueryCaps( &DiveCaps, DIVE_BUFFER_SCREEN ) )
  {
    WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
      "Error: DIVE routines cannot function in this system environment.",
      "   This program is unable to run.", 0, MB_OK | MB_INFORMATION );
    return FALSE;
  }

  if ( DiveCaps.ulDepth < 8 )
  {
    WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
      "Error: Not enough screen colors to run DIVE.  Must be at least 256 colors.",
      "   This program is unable to run.", 0, MB_OK | MB_INFORMATION );
    return FALSE;
  }

  // Get an instance of DIVE APIs.
  if ( DiveOpen( &hDive, FALSE, 0 ) )
  {
    WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
      "Error: Unable to open an instance of DIVE.",
      "   This program is unable to run.", 0, MB_OK | MB_INFORMATION );
    return FALSE;
  }

  DosAllocMem( (PPVOID)&pbImgBuf, cbImgBuf,
               PAG_COMMIT | PAG_EXECUTE | PAG_READ | PAG_WRITE );

  if ( DiveAllocImageBuffer( hDive, &ulDiveBufferNumber, FOURCC_LUT8,
                             640, 400, 0, pbImgBuf ) != NO_ERROR )
  {
    WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
      "DiveAllocImageBuffer() fail",
      "   This program is unable to run.", 0, MB_OK | MB_INFORMATION );
    return FALSE;
  }

  bDiveHaveBuffer = TRUE;
  DiveSetDestinationPalette( hDive, 0, 16, 0 );

  return TRUE;
}


int main(int argc, char *argv[])
{
  ULONG		ulRC;
  ULONG		cBytes;
  ULONG		ulDisk;

  debugInit();

  // Get the full path of the current director

  ulRC = DosQueryCurrentDisk( &ulDisk, &cBytes );
  if ( ulRC != NO_ERROR )
  {
    debug( "DosQueryCurrentDisk(), rc = %u\n", ulRC );
    return 1;
  }

  digger_dir[0] = ulDisk - 1 + 'A';
  digger_dir[1] = ':';
  digger_dir[2] = '\\';

  cBytes = sizeof(digger_dir) - 5;
  ulRC = DosQueryCurrentDir( 0, &digger_dir[3], &cBytes );
  if ( ulRC != NO_ERROR )
  {
    debug( "DosQueryCurrentDir(), rc = %u\n", ulRC );
    return 1;
  }
  strcat( &digger_dir, "\\" );

  // Create PM window

  hab = WinInitialize( 0 );
  hmq = WinCreateMsgQueue( hab, 0 );

  WinRegisterClass( hab, szClientClass, ClientWndProc,
                    CS_SIZEREDRAW | CS_MOVENOTIFY, sizeof(ULONG) );

  if ( !_initializeDive() )
    return 1;

  maininit();
  parsecmd( argc, argv );
  _createWindow( g_bWindowed );
  mainprog();

  return 0;
}
