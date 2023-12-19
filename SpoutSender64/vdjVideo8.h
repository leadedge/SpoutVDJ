//////////////////////////////////////////////////////////////////////////
//
// VirtualDJ
// Plugin SDK
// (c)Atomix Productions 2011-2021
//
//////////////////////////////////////////////////////////////////////////
//
// This file defines the video plugins (both Fx and Transition).
// In addition to all the elements supported from the base IVdjPlugin class,
// it defines additional video-specific functions and variables:
//
//////////////////////////////////////////////////////////////////////////


#ifndef VdjVideo8H
#define VdjVideo8H

#include "vdjPlugin8.h"

//////////////////////////////////////////////////////////////////////////
// data types

#ifndef TVertex
struct TVertex
{
	struct {float x,y,z;} position;
	DWORD color;
	float tu,tv;
};
#endif

enum EVdjVideoEngine
{
	VdjVideoEngineAny = 0,
	VdjVideoEngineDirectX9 = 1,
	VdjVideoEngineOpenGL = 2,
	VdjVideoEngineDirectX11 = 3,
	VdjVideoEngineOpenGLES2 = 4,
    VdjVideoEngineMetal = 5,
    VdjVideoEngineAnyPtr = 6,
};

#define VDJFLAG_VIDEO_MASTERONLY	0x10000
#define VDJFLAG_VIDEO_VISUALISATION 0x20000	// The effect generates visuals, rather than applying an effect on given images
#define VDJFLAG_VIDEO_OVERLAY		0x40000 // The effect is an overlay over the image already there. When this is set, you can't call DrawDeck in your plugin (it is already done before OnDraw())
#define VDJFLAG_VIDEO_HASRESIZE		0x80000 // The effect knows how to resize itself when ctrl is pressed
#define VDJFLAG_VIDEO_NOAUTOACTIVE  0x200000 // The video engine will not be automatically activated when activating this effect (for audio effects with a video option)
#define VDJFLAG_VIDEO_OUTPUTRESOLUTION 0x400000 // If the effect is applied on deck, the effect will be applied onto the video output resolution instead of the video source resolution
#define VDJFLAG_VIDEO_OUTPUTASPECTRATIO 0x800000  // If the effect is applied on deck, the effect will be applied in same aspect ratio as video output (and minimum resolution between video source and video output)
#define VDJFLAG_VIDEO_FORRECORDING 0x1000000 // If the effect is applied on the master, it will be rendered in the record resolution, and after videoskin

// For transitions, you need to define in OnGetPluginInfo the behavior of the auto-videocrossfader:
#define VDJFLAG_VIDEOTRANSITION_CONTINOUS	0x100000 // the crossfader moves continuously from one side to the other
#define VDJFLAG_VIDEOTRANSITION_MIDPOINTS	0x000000 // the crossfader stops at 25% or 75% when both videos are playing

//////////////////////////////////////////////////////////////////////////
// Internal structures
struct IVdjVideoCallbacks8
{
	virtual HRESULT DrawDeck()=0;

	//For DirectX 9 (windows 32-bit) device is IDirect3DDevice9*
	//For DirectX 11 (windows 64-bit) device is ID3D11Device*
	virtual HRESULT GetDevice(EVdjVideoEngine engine, void **device)=0;
	
	//For DirectX 9 (windows 32-bit) texture is IDirect3DTexture9*
	//For DirectX 11 (windows 64-bit) texture is ID3D11ShaderResourceView*
	//For OpenGL (macOS) texture is a regular opengl texture id, with type GLuint
	virtual HRESULT GetTexture(EVdjVideoEngine engine, void **texture, TVertex **vertices)=0;
};


//////////////////////////////////////////////////////////////////////////
// VideoTransition plugin class

class IVdjPluginVideoTransition8 : public IVdjPlugin8
{
public:
	// OnDraw is called whenever the video crossfader is different than 0.0 or 1.0.
	// this function should mix the two images from both decks.
	// call DrawDeck() to draw the image for each deck with the specified vertices.
	// call GetVertices() to retrive the default vertices, and modify it. (or pass NULL to DrawDeck() to use the default)
	virtual HRESULT VDJ_API OnDraw(float crossfader)=0;
	TVertex* (*GetVertices)(int deck);
	HRESULT (*DrawDeck)(int deck, TVertex* vertices);

	// for more complicated operations, you can ask direct access to the device and textures
	//For DirectX 9 (windows 32-bit) texture is IDirect3DTexture9*
	//For DirectX 11 (windows 64-bit) texture is ID3D11ShaderResourceView*
	HRESULT (*GetDevice)(EVdjVideoEngine engine, void **device);

	//For OpenGL (macOS) texture is a regular opengl texture id, with type GLuint
	//For DirectX 9 (windows 32-bit) device is IDirect3DDevice9*
	//For DirectX 11 (windows 64-bit) device is ID3D11Device*
	HRESULT (*GetTexture)(EVdjVideoEngine engine, int deck, void**texture);

	// When DirectX/OpenGL is initialized or closed, these functions will be called
	virtual HRESULT VDJ_API OnDeviceInit() {return S_OK;}
	virtual HRESULT VDJ_API OnDeviceClose() {return S_OK;}

	// size of the output
	int width, height;

	// Some useful variables
	int SampleRate;		 // samplerate of the audio engine
	int SongBpm;		 // number of samples between two consecutive beats for master song
	double SongPosBeats; // number of beats since the first beat in the master song
};

class IVdjPluginVideoTransitionMultiDeck8 : public IVdjPlugin8
{
public:
	// this version is used if you want a transition plugin that can handle more than 2 decks.
	// OnDrawMultiDeck is called for each frame (whatever the crossfader position).
	// videoDecks is an array of nbVideos integers, giving the decks number of each deck that has a video available.
	// the array is sorted by order of "importance" of the videos (videoLeft and videoRight first, then playing video, etc).
	// call DrawDeck() to draw the images for each deck with the specified vertices.
	// call GetVertices to initialize the vertices to their default value, and modify it. (or pass NULL to DrawDeck() to use the default)
	// NOTE: if you want to retrieve the value of the video crossfader, call GetInfo("video_crossfader");
	virtual HRESULT VDJ_API OnDrawMultiDeck(int nbVideoDecks, int *videoDecks)=0;
	TVertex* (*GetVertices)(int deck);
	HRESULT (*DrawDeck)(int deck, TVertex* vertices);
	
	// for more complicated operations, you can ask direct access to the device and textures
	//For DirectX 9 (windows 32-bit) device is IDirect3DDevice9*
	//For DirectX 11 (windows 64-bit) device is ID3D11Device*
	HRESULT (*GetDevice)(EVdjVideoEngine engine, void **device);

	//For OpenGL (macOS) texture is a regular opengl texture id, with type GLuint
	//For DirectX 9 (windows 32-bit) device is IDirect3DDevice9*
	//For DirectX 11 (windows 64-bit) device is ID3D11Device*
	HRESULT (*GetTexture)(EVdjVideoEngine engine, int deck, void**texture);

	// When DirectX/OpenGL is initialized or closed, these functions will be called
	virtual HRESULT VDJ_API OnDeviceInit() {return S_OK;}
	virtual HRESULT VDJ_API OnDeviceClose() {return S_OK;}

	// size of the output
	int width, height;

	// Some useful variables
	int SampleRate;		 // samplerate of the audio engine
	int SongBpm;		 // number of samples between two consecutive beats for master song
	double SongPosBeats; // number of beats since the first beat in the master song
};


//////////////////////////////////////////////////////////////////////////
// VideoFx plugin class

class IVdjPluginVideoFx8 : public IVdjPlugin8
{
public:
	// called on start and stop of the plugin
	virtual HRESULT VDJ_API OnStart() {return S_OK;}
	virtual HRESULT VDJ_API OnStop() {return S_OK;}

	// OnDraw() is called every frame while your plugin is activated.
	// You can get access to the DirectX/OpenGL device by calling GetDevice, and do any operation you want
	// In order to draw the original image, you can either just call DrawDeck() if you don't need to modify the image (for overlay plugins for examples),
	// or call GetTexture to get low-level access to the texture and its vertices.
	virtual HRESULT VDJ_API OnDraw()=0;
	
	//For DirectX 9 (windows 32-bit) device is IDirect3DDevice9*
	//For DirectX 11 (windows 64-bit) device is ID3D11Device*
    //For Metal on mac device is id<MTLRenderCommandEncoder>
	HRESULT GetDevice(EVdjVideoEngine engine, void **device) {return vcb->GetDevice(engine,device);}
	
	//For DirectX 9 (windows 32-bit) texture is IDirect3DTexture9*
	//For DirectX 11 (windows 64-bit) texture is ID3D11ShaderResourceView*
	//For OpenGL (macOS) texture is a regular opengl texture id, with type GLuint
	HRESULT GetTexture(EVdjVideoEngine engine, void **texture, TVertex **vertices) {return vcb->GetTexture(engine,texture,vertices);}
	
	HRESULT DrawDeck() {return vcb->DrawDeck();}

	// When DirectX/OpenGL is initialized or closed, these functions will be called
	virtual HRESULT VDJ_API OnDeviceInit() {return S_OK;}
	virtual HRESULT VDJ_API OnDeviceClose() {return S_OK;}

	// Optionally, you can implement OnAudioSamples to make your video effect operate based on sound input
	virtual HRESULT VDJ_API OnAudioSamples(float *buffer, int nb) { return E_NOTIMPL; };

	// Some useful variables
	int SampleRate;		 // samplerate of the audio engine
	int SongBpm;		 // number of samples between two consecutive beats for this song
	double SongPosBeats; // number of beats since the first beat in the song

	// size of the output
	int width, height;

	IVdjVideoCallbacks8 *vcb;
};

//////////////////////////////////////////////////////////////////////////
// GUID definitions

#ifndef VDJVIDEO8GUID_DEFINED
#define VDJVIDEO8GUID_DEFINED
static const GUID IID_IVdjPluginVideoFx8 = { 0xbf1876aa, 0x3cbd, 0x404a, { 0xbe, 0xab, 0x5f, 0x8b, 0x51, 0xe3, 0x90, 0xc0 } };
static const GUID IID_IVdjPluginVideoTransition8 = { 0x2f350983, 0xf88f, 0x429c, { 0x87, 0x75, 0x62, 0x87, 0x68, 0x7d, 0xe0, 0xd7 } };
static const GUID IID_IVdjPluginVideoTransitionMultiDeck8 = { 0x54d0e81c, 0x51a6, 0x49b0, { 0x82, 0x3f, 0x75, 0x91, 0x76, 0xf1, 0xcf, 0x06 } };
#else
extern static const GUID IID_IVdjPluginVideoFx8;
extern static const GUID IID_IVdjPluginVideoTransition8;
extern static const GUID IID_IVdjPluginVideoTransitionMultiDeck8;
#endif

//////////////////////////////////////////////////////////////////////////

#endif
