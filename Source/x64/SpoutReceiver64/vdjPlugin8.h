//////////////////////////////////////////////////////////////////////////
//
// VirtualDJ
// Plugin SDK
// (c)Atomix Productions 2011-2019
// extended with extra "official" initially unexposed features 2021
// known addons status as of 2020/10/10
//
//////////////////////////////////////////////////////////////////////////
//
// This file defines the basic functions that are used in all plugins.
// It defines the functions and variables needed to:
// - load and unload a plugin
// - give the infos about the plugin (name, picture, etc)
// - get the parameters automatically saved and restored between loads
// - interact with VirtualDJ (ask queries or send commands)
// - implement a custom interface
//
// Other functions specific to particular types of plugin can be found
// in their respective header file
//
//////////////////////////////////////////////////////////////////////////


#ifndef VdjPlugin8H
#define VdjPlugin8H

//////////////////////////////////////////////////////////////////////////
// Platform specific defines for compatibility Mac/Windows
#ifdef VDJ_NOEXPORT
#elif (defined(WIN32) || defined(_WIN32) || defined(__WIN32_))
#define WIN32_LEAN_AND_MEAN //For preventing linking errors with DllGetClassObject
#include <windows.h>
#define VDJ_WIN
#if (defined(_M_X64) || defined(__amd64__))
#define VDJ_IS_WIN64 // if you need special code for the 64bit version of VirtualDJ, use this
#endif
#define VDJ_EXPORT	__declspec( dllexport )
#define VDJ_API	__stdcall
#define VDJ_BITMAP	HBITMAP
#define VDJ_HINSTANCE	HINSTANCE
#define VDJ_WINDOW	HWND
#elif (defined(__APPLE__) || defined(MACOSX) || defined(__MACOSX__))
#include <CoreFoundation/CoreFoundation.h>
#include <MacTypes.h>
#define VDJ_MAC
#define VDJ_EXPORT	__attribute__ ((visibility ("default")))
#define VDJ_API
#define VDJ_BITMAP	char *
#define VDJ_HINSTANCE	CFBundleRef
#define VDJ_WINDOW void* // VDJ_Window expected to be a NSWindow
typedef SInt32 HRESULT;
typedef UInt32 ULONG;
typedef unsigned int DWORD;
#define S_OK ((HRESULT)0x00000000L)
#define S_FALSE ((HRESULT)0x00000001L)
#define E_NOTIMPL ((HRESULT)0x80004001L)
#define E_FAIL ((HRESULT)0x80004005L)
#define CLASS_E_CLASSNOTAVAILABLE -1
#define NO_ERROR	0
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID {
unsigned long Data1;
unsigned short Data2;
unsigned short Data3;
unsigned char Data4[ 8 ];
} GUID;
#endif
#endif

//////////////////////////////////////////////////////////////////////////
// Standard structures and defines

// structure used in plugin identification
struct TVdjPluginInfo8
{
const char *PluginName;
const char *Author;
const char *Description;
const char *Version;
VDJ_BITMAP Bitmap;
DWORD Flags;
};

// flags used for struct TVdjPluginInfo8
#define VDJFLAG_NODOCK 0x1
#define VDJFLAG_PROCESSAFTERSTOP 0x2
#define VDJFLAG_PROCESSFIRST 0x4
#define VDJFLAG_PROCESSLAST 0x8

// flags used for plugin's parameters
#define VDJPARAM_BUTTON	0
#define VDJPARAM_SLIDER	1
#define VDJPARAM_SWITCH	2
#define VDJPARAM_STRING	3
#define VDJPARAM_CUSTOM	4
#define VDJPARAM_RADIO	5
#define VDJPARAM_COMMAND 6
// Similar to slider, but with default position at 0.5 and only one per effect for single-knob full control
#define VDJPARAM_COLORFX 7
// float specifying number of beats
#define VDJPARAM_BEATS 8
// int, set to +1, -1 etc to move the number of beats higher or lower when OnParameter is called
#define VDJPARAM_BEATS_RELATIVE 9
// Not already exposed in the SDK (official add)
// Position array of 4 float, (x0,y0), (x1,y1)
#define VDJPARAM_POSITION 10
// Scratch String / Shader string ???
#define VDJPARAM_SSTRING 11
// Mixfx same as a colorfx (but needed to appear in correct selection menu)
#define VDJPARAM_TRANSITIONFX 12
// Testing adds
#define VDJPARAM_TEST_13 13
#define VDJPARAM_TEST_14 14
#define VDJPARAM_TEST_15 15
#define VDJPARAM_TEST_16 16

// .ini file extention (Adion)
// append to dllname to save/retreive parameters
// master audio/video and decks and mic and aux and sampler all share only one ini ... (bug)
#define Deck_ini ""
// mixer color fx
#define ColorFX_ini "_1"
// audio visualisation (source)
#define Audio_Only_Visualisation_ini = "_2"
// master audio/video and decks and mic and aux and sampler Release chanels all share only one ini ... (bug)
#define ReleaseFX_ini "_3"
// mixer 1..4 color fx
#define ColorFX1_ini "_4"
#define ColorFX2_ini "_5"
#define ColorFX3_ini "_6"
#define ColorFX4_ini "_7"
// master Mixer fx
#define MixFX_ini "_8"



struct TVdjPluginInterface8
{
DWORD Type;
// xml and image buffers if Type==VDJINTERFACE_SKIN
const char *Xml;
void *ImageBuffer;
int ImageSize;
// Type==VDJINTERFACE_DIALOG. HWND returned by CreateDialog or CreateWindow on Windows 32bit, or NSWindow pointer on mac
VDJ_WINDOW hWnd;
};

// structure used for custom interfaces
#define VDJINTERFACE_DEFAULT	0
#define VDJINTERFACE_SKIN	1
#define VDJINTERFACE_DIALOG	2

//////////////////////////////////////////////////////////////////////////
// Internal structures
struct IVdjCallbacks8
{
virtual HRESULT SendCommand(const char *command)=0;
virtual HRESULT GetInfo(const char *command,double *result)=0;
virtual HRESULT GetStringInfo(const char *command,void *result,int size)=0;
virtual HRESULT DeclareParameter(void *parameter,int type,int id,const char *name,const char *shortName,float defaultvalue)=0;
virtual HRESULT GetSongBuffer(int pos, int nb, short **buffer)=0;
};

//////////////////////////////////////////////////////////////////////////
// Base class

struct IVdjCallbacks8;

class IVdjPlugin8
{
public:
// Initialization
virtual HRESULT VDJ_API OnLoad() {return S_OK;}
virtual HRESULT VDJ_API OnGetPluginInfo(TVdjPluginInfo8 *info) {return E_NOTIMPL;}
virtual ULONG VDJ_API Release() {delete this; return S_OK;}
virtual ~IVdjPlugin8() {}

// call DeclareParameter*() for all your variables during OnLoad()
// if type=VDJPARAM_CUSTOM or VDJPARAM_STRING, parameterSize must be set to sizeof(*parameter)
HRESULT DeclareParameterButton(int *parameter, int id, const char *name, const char *shortName) {return cb->DeclareParameter(parameter,VDJPARAM_BUTTON,id,name,shortName,0.0f);}
HRESULT DeclareParameterSlider(float *parameter, int id, const char *name, const char *shortName, float defaultvalue) {return cb->DeclareParameter(parameter,VDJPARAM_SLIDER,id,name,shortName,defaultvalue);}
HRESULT DeclareParameterSwitch(int *parameter, int id, const char *name, const char *shortName, bool defaultvalue) {return cb->DeclareParameter(parameter,VDJPARAM_SWITCH,id,name,shortName,(float) defaultvalue);}
HRESULT DeclareParameterString(char *parameter, int id, const char *name, const char *shortName, int parameterSize) {return cb->DeclareParameter(parameter,VDJPARAM_STRING,id,name,shortName,(float) parameterSize);}
HRESULT DeclareParameterCustom(void *parameter, int id, const char *name, const char *shortName, int parameterSize) {return cb->DeclareParameter(parameter,VDJPARAM_CUSTOM,id,name,shortName,(float) parameterSize);}
HRESULT DeclareParameterRadio(int *parameter, int id, const char *name, const char *shortName, float defaultvalue) {return cb->DeclareParameter(parameter,VDJPARAM_RADIO,id,name,shortName,(float) defaultvalue);}
HRESULT DeclareParameterCommand(char *parameter, int id, const char *name, const char *shortName, int parameterSize) {return cb->DeclareParameter(parameter,VDJPARAM_COMMAND,id,name,shortName,(float) parameterSize);}
HRESULT DeclareParameterColorFX(float *parameter, int id, const char *name, const char *shortName) { return cb->DeclareParameter(parameter, VDJPARAM_COLORFX, id, name, shortName, 0.5f); }
HRESULT DeclareParameterBeats(float *parameter, int id, const char *name, const char *shortName) { return cb->DeclareParameter(parameter, VDJPARAM_BEATS, id, name, shortName, 0.5f); }
HRESULT DeclareParameterBeatsRelative(int *parameter, int id, const char *name, const char *shortName) { return cb->DeclareParameter(parameter, VDJPARAM_BEATS_RELATIVE, id, name, shortName, 0.5f); }
// not official
// array of 4 floats coords x0 y0 x1 y1 top left bottom right corners
HRESULT DeclareParameterPosition(float *parameter, int id, const char *name, const char *shortName) { return cb->DeclareParameter(parameter, VDJPARAM_POSITION, id, name, shortName, sizeof(float) * 4.0f); }
// testing
//HRESULT DeclareParameterSString(char *parameter, int id, const char *name, const char *shortName, int allocated) { return cb->DeclareParameter(parameter, VDJPARAM_SSTRING, id, name, shortName, (float) allocated); }
HRESULT DeclareParametertest11(float *parameter, int id, const char* name, const char* shortName, bool defaultvalue) { return cb->DeclareParameter(parameter, VDJPARAM_SSTRING, id, name, shortName, (float)defaultvalue); }
// official (Adion)
HRESULT DeclareParameterTransitionFX(float *parameter, int id, const char *name, const char *shortName) { return cb->DeclareParameter(parameter, VDJPARAM_TRANSITIONFX, id, name, shortName, 0.5f); }
// not official testing
HRESULT DeclareParametertest13(char *parameter, int id, const char *name, const char *shortName, int parameterSize) { return cb->DeclareParameter(parameter, VDJPARAM_TEST_13, id, name, shortName, (float)parameterSize); }
HRESULT DeclareParametertest14(char *parameter, int id, const char *name, const char *shortName, int parameterSize) { return cb->DeclareParameter(parameter, VDJPARAM_TEST_14, id, name, shortName, (float)parameterSize); }
HRESULT DeclareParametertest15(char *parameter, int id, const char *name, const char *shortName, int parameterSize) { return cb->DeclareParameter(parameter, VDJPARAM_TEST_15, id, name, shortName, (float)parameterSize); }
HRESULT DeclareParametertest16(char *parameter, int id, const char *name, const char *shortName, int parameterSize) { return cb->DeclareParameter(parameter, VDJPARAM_TEST_16, id, name, shortName, (float)parameterSize); }

// OnParameter will be called each time a parameter is changed from within VirtualDJ
virtual HRESULT VDJ_API OnParameter(int id) {return S_OK;}
// OnGetParameterString will be called each time the string label of a parameter is requested by VirtualDJ
virtual HRESULT VDJ_API OnGetParameterString(int id, char *outParam, int outParamSize) {return E_NOTIMPL;} ;

// Custom user-interface
// Fill the VDJ_WINDOW or xml/bitmap info in the passed pluginInterface structure, to define your own plugin window
virtual HRESULT VDJ_API OnGetUserInterface(TVdjPluginInterface8 *pluginInterface) {return E_NOTIMPL;}

// send a VDJScript command to VirtualDJ
HRESULT SendCommand(const char *command) {return cb->SendCommand(command);}
// get info from VirtualDJ (as a value, or as a utf-8 string). You can also use command = "setting 'xxx'" for VirtualDJ options
HRESULT GetInfo(const char *command, double *result) {return cb->GetInfo(command,result);}
HRESULT GetStringInfo(const char *command, char *result, int size) {return cb->GetStringInfo(command,result,size);}

VDJ_HINSTANCE hInstance = 0;

IVdjCallbacks8 *cb = nullptr;
};

//////////////////////////////////////////////////////////////////////////
// GUID definitions

#ifndef VDJCLASS8GUID_DEFINED
#define VDJCLASS8GUID_DEFINED
static const GUID CLSID_VdjPlugin8 = { 0xED8A8D87, 0xF4F9, 0x4DCD, { 0xBD, 0x24, 0x29, 0x14, 0x12, 0xE9, 0x3B, 0x60 } };
static const GUID IID_IVdjPluginBasic8 = { 0xa1d90ea1, 0x4d0d, 0x42dd, { 0xa4, 0xd0, 0xb8, 0xf3, 0x37, 0xb3, 0x21, 0xf1 } };
#else
extern static const GUID CLSID_VdjPlugin8;
extern static const GUID IID_IVdjPluginBasic8;
#endif

//////////////////////////////////////////////////////////////////////////
// DLL export function (Entry point of the plugin)

#ifndef NODLLEXPORT
#ifdef __cplusplus
extern "C" {
#endif
VDJ_EXPORT HRESULT VDJ_API DllGetClassObject(const GUID &rclsid,const GUID &riid,void** ppObject);
#ifdef __cplusplus
}
#endif
#endif

//////////////////////////////////////////////////////////////////////////

#endif
