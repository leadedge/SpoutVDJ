//////////////////////////////////////////////////////////////////////////
//
// VirtualDJ
// Plugin SDK
// (c)Atomix Productions 2011-2018
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
#define VDJ_EXPORT		__declspec( dllexport )
#define VDJ_BITMAP		HBITMAP
#define VDJ_HINSTANCE	HINSTANCE
#define VDJ_WIN
#define VDJ_WINDOW		HWND
#define VDJ_API			__stdcall
#elif (defined(__APPLE__) || defined(MACOSX) || defined(__MACOSX__))
#include <CoreFoundation/CoreFoundation.h>
#define VDJ_EXPORT		__attribute__ ((visibility ("default")))
#define VDJ_BITMAP		char *
#define VDJ_HINSTANCE	CFBundleRef
#ifndef S_OK
#define S_OK            ((HRESULT)0x00000000L)
#endif
#ifndef S_FALSE
#define S_FALSE         ((HRESULT)0x00000001L)
#endif
#ifndef E_NOTIMPL
#define E_NOTIMPL       ((HRESULT)0x80004001L)
#endif
#define CLASS_E_CLASSNOTAVAILABLE -1
#define NO_ERROR		0
#include <MacTypes.h>
typedef SInt32 HRESULT;
typedef UInt32 ULONG;
typedef unsigned int DWORD;
#define VDJ_MAC
//VDJ_Window expected to be a NSWindow
#define VDJ_WINDOW void*
#define VDJ_API
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID {
unsigned long Data1;
unsigned short Data2;
unsigned short Data3;
unsigned char Data4[ 8 ];
} GUID;
#endif
#elif defined(__ANDROID__)
#define VDJ_BITMAP char *
#define VDJ_WINDOW void *
#define VDJ_HINSTANCE	void *
#define VDJ_API
typedef unsigned int ULONG;
#endif

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

struct IVdjVideoMouseCallbacks8
{
	virtual bool OnMouseMove(int x, int y, int buttons, int keyModifiers)=0;
	virtual bool OnMouseDown(int x, int y, int buttons, int keyModifiers)=0;
	virtual bool OnMouseUp(int x, int y, int buttons, int keyModifiers)=0;
	virtual void OnKey(const char *ch, int vkey, int modifiers, int flag, int scancode) {}
};

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

struct TVdjPluginInfo8_Extension1
{
	TVdjPluginInfo8 info;
	IVdjVideoMouseCallbacks8 *mouseCallbacks;
};

// flags used for plugin's parameters
#define VDJPARAM_BUTTON	0
#define VDJPARAM_SLIDER	1
#define VDJPARAM_SWITCH	2
#define VDJPARAM_STRING	3
#define VDJPARAM_CUSTOM	4
#define VDJPARAM_RADIO	5
#define VDJPARAM_COMMAND 6
#define VDJPARAM_COLORFX 7 //similar to slider, but with default position at 0.5 and only one per effect for single-knob full control
#define VDJPARAM_BEATS 8 //float specifying number of beats
#define VDJPARAM_BEATS_RELATIVE 9 //int, set to +1, -1 etc to move the number of beats higher or lower when OnParameter is called
#define VDJPARAM_POSITION 10 //used for video plugins to allow resizing/positioning by the user in the plugin's GUI
#define VDJPARAM_RELEASEFX 11
#define VDJPARAM_TRANSITIONFX 12

#define VDJFLAG_NODOCK 0x1
#define VDJFLAG_PROCESSAFTERSTOP 0x2 //when this flag is set, OnProcessSamples or OnDraw are called after plugin is stopped. Plugin should return E_FAIL as soon as possible to stop processing.
#define VDJFLAG_PROCESSFIRST 0x4	//this plugin should be processed first
#define VDJFLAG_PROCESSLAST  0x8	//this plugin should be processed last 
#define VDJFLAG_EXTENSION1	0x10	//will be set by VDJ when calling OnGetPluginInfo if the passed structure is of the extended type
#define VDJFLAG_SETPREVIEW 0x20		//preview for use on skin will exclude this plugin when this flag is set
#define VDJFLAG_POSITION_NOSLIP 0x40 //when set for position plugin, it will not use slip mode
#define VDJFLAG_ALWAYSPREFADER 0x80
#define VDJFLAG_ALWAYSPOSTFADER 0x100
#define VDJFLAG_EPHEMERAL 0x200 // don't save or load the params from ini

// structure used for custom interfaces
#define VDJINTERFACE_DEFAULT	0
#define VDJINTERFACE_SKIN		1
#define VDJINTERFACE_DIALOG		2
struct TVdjPluginInterface8
{
	DWORD Type;
	// xml and image buffers if Type==VDJINTERFACE_SKIN
	const char *Xml;
	void *ImageBuffer;
	int ImageSize;
	// Type==VDJINTERFACE_DIALOG. HWND returned by CreateDialog or CreateWindow on Windows, or NSWindow pointer on mac
	VDJ_WINDOW hWnd;
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
	HRESULT DeclareParameterButton(int *parameter, int id, const char *name, const char *shortName) {return cb->DeclareParameter(parameter,VDJPARAM_BUTTON,id,name,shortName,0);}
	HRESULT DeclareParameterSlider(float *parameter, int id, const char *name, const char *shortName, float defaultvalue) {return cb->DeclareParameter(parameter,VDJPARAM_SLIDER,id,name,shortName,defaultvalue);}
	HRESULT DeclareParameterSwitch(int *parameter, int id, const char *name, const char *shortName, bool defaultvalue) {return cb->DeclareParameter(parameter,VDJPARAM_SWITCH,id,name,shortName,(float)defaultvalue);}
	HRESULT DeclareParameterString(char *parameter, int id, const char *name, const char *shortName, int parameterSize) { return cb->DeclareParameter(parameter, VDJPARAM_STRING, id, name, shortName, (float)parameterSize); }
	HRESULT DeclareParameterCustom(void *parameter, int id, const char *name, const char *shortName, int parameterSize) { return cb->DeclareParameter(parameter, VDJPARAM_CUSTOM, id, name, shortName, (float)parameterSize); }
	HRESULT DeclareParameterRadio(int *parameter, int id, const char *name, const char *shortName, float defaultvalue) { return cb->DeclareParameter(parameter, VDJPARAM_RADIO, id, name, shortName, (float)defaultvalue); }
	HRESULT DeclareParameterCommand(char *parameter, int id, const char *name, const char *shortName, int parameterSize) { return cb->DeclareParameter(parameter, VDJPARAM_COMMAND, id, name, shortName, (float)parameterSize); }
	HRESULT DeclareParameterColorFX(float *parameter, int id, const char *name, const char *shortName) { return cb->DeclareParameter(parameter, VDJPARAM_COLORFX, id, name, shortName, 0.5f); }
	HRESULT DeclareParameterBeats(float *parameter, int id, const char *name, const char *shortName) { return cb->DeclareParameter(parameter, VDJPARAM_BEATS, id, name, shortName, 0.5f); }
	HRESULT DeclareParameterBeatsRelative(int *parameter, int id, const char *name, const char *shortName) { return cb->DeclareParameter(parameter, VDJPARAM_BEATS_RELATIVE, id, name, shortName, 0.5f); }
	HRESULT DeclareParameterPosition(float parameter[4], int id, const char *name, const char *shortName) { return cb->DeclareParameter(parameter, VDJPARAM_POSITION, id, name, shortName, 4*sizeof(float)); }
	HRESULT DeclareParameterReleaseFX(float *parameter, int id, const char *name, const char *shortName) { return cb->DeclareParameter(parameter, VDJPARAM_RELEASEFX, id, name, shortName, 0.f); }
	HRESULT DeclareParameterTransitionFX(float *parameter, int id) { return cb->DeclareParameter(parameter, VDJPARAM_TRANSITIONFX, id, "Transition FX", "Trans", 0.f); }

	// OnParameter will be called each time a parameter is changed from within VirtualDJ
	virtual HRESULT VDJ_API OnParameter(int id) {return S_OK;}
	// OnGetParameterString will be called each time the string label of a parameter is requested by VirtualDJ
	virtual HRESULT VDJ_API OnGetParameterString(int id, char *outParam, int outParamSize) {return E_NOTIMPL;}

	// Custom user-interface
	// Fill the HWND or xml/bitmap info in the passed pluginInterface structure, to define your own plugin window
	virtual HRESULT VDJ_API OnGetUserInterface(TVdjPluginInterface8 *pluginInterface) {return E_NOTIMPL;}
	VDJ_HINSTANCE hInstance;

	// send a VDJScript command to VirtualDJ
	HRESULT SendCommand(const char *command) {return cb->SendCommand(command);}
	// get info from VirtualDJ (as a value, or as a utf-8 string)
	HRESULT GetInfo(const char *command, double *result) {return cb->GetInfo(command,result);}
	HRESULT GetStringInfo(const char *command, char *result, int size) {return cb->GetStringInfo(command,result,size);}

	IVdjCallbacks8 *cb;
};

class IVdjPluginStartStop8 : public IVdjPlugin8
{
public:
	// called when the plugin is started or stopped
	virtual HRESULT VDJ_API OnStart() { return 0; }
	virtual HRESULT VDJ_API OnStop() { return 0; }
};

//////////////////////////////////////////////////////////////////////////
// GUID definitions

#ifndef VDJCLASS8GUID_DEFINED
#define VDJCLASS8GUID_DEFINED
static const GUID CLSID_VdjPlugin8 = { 0xED8A8D87, 0xF4F9, 0x4DCD, { 0xBD, 0x24, 0x29, 0x14, 0x12, 0xE9, 0x3B, 0x60 } };
static const GUID IID_IVdjPluginBasic8 = { 0xa1d90ea1, 0x4d0d, 0x42dd, { 0xa4, 0xd0, 0xb8, 0xf3, 0x37, 0xb3, 0x21, 0xf1 } };
static const GUID IID_IVdjPluginStartStop8 = { 0xa1d91ea1, 0x4e0d, 0x32dd, { 0x14, 0xd0, 0xc8, 0xf3, 0x47, 0xb6, 0x41, 0xd1 }};
#else
extern static const GUID CLSID_VdjPlugin8;
extern static const GUID IID_IVdjPluginBasic8;
extern static const GUID IID_IVdjPluginStartStop8;
#endif

//////////////////////////////////////////////////////////////////////////
// DLL export function

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
