//
// VDJSpoutReceiver64.cpp : Defines the exported functions for the DLL application.
//
//		09.02.15	corrected texture invert by adding invert flag to DrawSharedTexture
//		14.02.15	added Optimus enablement export
//					Changed to /MT compile
//					Inital release
//					Version 1.0
//		21.02.15	Fixed bug for fbo released too soon - only showed up with Optimus graphics
//					Verson 1.01
//		22.02.15	Debug SpoutPanel.exe to resolve freeze and crash due to "WideCharToMultiByte" with null command line
//					Set DX9 compatible because incompatible DX11 textures will not be received
//					Version 1.02
//		26.05.15	Recompile for revised SpoutPanel registry write of sender name
//					Version 1.03
//		07.07.15	Recompile on new drive Windows 7 32bit VS2010 - VDJ vers v8.0.2345
//		08.07.15	Create an invisible dummy button window for OpenGL due to SetPixelFormat problems noted with Mapio
//		01.08.15	Recompile for 2.004 release /MT
//					Version 1.04
//		15.10.15	Remove copy/paste error in draw
//					Changed to /MT compile
//					Recompile for 2.005
//					Version 1.05
//		19.10.15	Complete re-write - DirectX version
//		07.11.15	Allowed format change to allow receive of X8R8G8B8 textures from Milkdrop
//					VDJ <> Milkdrop is DX9 only, so must use DX9 for all apps
//		04.12.15	Cleanup
//		06.12.15	Version 1.06
//		07.12.15	Release offscreen surface before equating to resolved surface
//					Gave problems with Windows 7 32bit but not 64bit and only for AOV mode
//				    Cleanup of surface naming and unused variables
//					Version 1.07
//		08.12.15	info flag VDJFLAm_VIDEO_VISUALISATION only
//					Version 1.07b
//		11.12.15	Added release function as per examples - checked that it was previously called
//					Version 1.07c
//		14.12.15	Used UpdateSurface instead of  D3DXLoadSurfaceFromSurface
//					Removed dependency on D3dx9.lib
//		15.12.15	Rebuild for Spout 2.005 release
//					Version 1.08
//		18.02.16	Added an optional define "SPOUTEFFECT" to compile as an effect plugin
//					Should be copied to the "Plugins\VideoEffect" folder as "VDJSpoutEffect"
//					Version 1.09
//		23.06.16 - rebuild for Spout 2.005 release Version 1.10
//				   VS2012 /MT
//		23.01.17 - Rebuild for 2.006 VS2012 /MD - Version 1.11
//		08.02.17 - Reset closing flag on InitDevice - Version 1.12
//		23.12.18 - Rebuild Win32 for 2.007 VS2017 /MT - Version 1.13
//      ===================== end of 32 bit DX9 version ====================
//
//		29.12.18 - Revise for DX11 for VirtualDJ 64 bit
//		03.01.19 - Changed to revised registry functions in SpoutUtils
//		22.05.19 - Add local texture
//				 - Remove SPOUTEFFECT flag
//		02.12.19 - First working version
//				   Some interaction between deck and master.
//				   Fills the window OK if selected from Master but sometimes
//				   the deck visualisation has to be selected first.
//		11.01.19 - Rebuild 64bit for 2.007 VS2017 /MT - Version 2.00
//		02.06.20 - Rebuild without console
//		05.01.21 - Add VDJFLAm_VIDEO_OUTPUTRESOLUTION flag
//				   Add flush on VDJ device after shared texture copy
//				   Rebuild with latest 2.007 code from develop branch
//				   64bit for 2.007 VS2017 /MT - Version 2.01
//		06.01.21 - Add local shader to render texture to VDJ
//				   Only fetch VDJ device/context on device creation
//				   Create shader resource view with texture instead of every frame
//		06.01.21 - Credit : Scott Bye (https://github.com/SBDJUK)
//				   Pixel shader and new render method created to resolve
//				   VirtualDJ resolution change problems.
//				   64bit for 2.007 VS2017 /MT - Version 2.02
//				   GitHub release v2005
//		07.01.21 - SBDJUK : 
//				     Add SafeRelease() 
//				     Create sender shared texture pointer only on change
//				     Fix the resizing/full screen issue
//				   Clean up a bit and add some logs
//				   Change all "g_" to "m_"
//				   m_pVertexBuffer, m_pPixelShader, m_pImmediateContext, m_pVDJdevice
//				   Release shared texture pointer on sender re-size
//				   Remove context flush after shared texture copy to local texture
//				   64bit for 2.007 VS2017 /MT - Version 2.03
//				   GitHub release v2006
//		07.01.21 - Re-introduced info flag "VDJFLAG_VIDEO_VISUALISATION" due to typo
//				   Fixed ststr sender name compare
//				   64bit for 2.007 VS2017 /MT - Version 2.04
//		08.01.21 - Fix missing VDJFLAG_VIDEO_VISUALISATION typo - again
//				   Remove debugging console window
//				   Introduce changes by Nicotux (https://github.com/nicotux/VDJSpoutReceiver64_Overlay)
//				     Add Deck specific name & loopback prevention
//				   64bit for 2.007 VS2017 /MT - Version 2.05. GitHub binary updated.
//		08.01.21 - Fix master display on/off problem
//				   OnDeviceClose is called but not OnDeviceInit
//				   Create InitializeDraw function
//				   Re-organise
//				   Version 2.06. GitHub binary updated.
//		08.01.21 - Adapt received texture and shader reource view to sender format
//				   Allow for DX9 formats D3DFMT_A8R8G8B8 and D3DFMT_X8R8G8B8
//				   Version 2.07. GitHub binary updated.
//		08.01.21 - Release local texture for user selection
//				   Version 2.08. GitHub binary updated.
//		09.01.21 - Optional "#define OverlayPlugin" in header
//				   for build with overlay and position control change by Nicotux
//				   DLL renamed to VDJSpoutOverlay64.dll
//				   Detect changed sender format
//				   Version 2.09. GitHub source and binaries updated.
//		09.01.21 - Fixed variable name for Overlay plugin
//		10.01.21 - Overlay plugin possibly as a separate project
//				   https://github.com/nicotux/VDJSpoutReceiver64_Overlay
//				   Change back to original UpdateVertices code
//				   Add changed share handle check for received sender
//				   Change tested sender name based on deck of the sender plugin
//				   Removed cleanup code from Release() because OnDeviceClose() is called first
//				   Version 2.10. GitHub source and binaries updated. Release v2007.
//		11.01.21 - Include position control and build as overlay
//				   The plugin can be copied to both Visualisation and Video Effects
//				   Copy to GitHub beta branch for further testing
//				   TODO : documentation
//				   Version 2.11
//		12.01.21 - Change infos->PluginName to "SpoutReceiver"
//				   Change build copy name to SpoutReceiver
//		13.01.21 - Add " master" string check for sender names to avoid.
//		17.01.21 - Build for release v2008 - Version 2.12
//		19.12.23 - Use _access in place of shlwapi function PathFileExists
//				   Change from local Spout SDK files to use withing SDK "myApps" folder
//				   Rebuild with latest Spout files (SDKversion 2.007.013)
//				   Version 2.13
//
//		------------------------------------------------------------
//
//		Copyright (C) 2015-2024. Lynn Jarvis, Leading Edge. Pty. Ltd.
//
//		This program is free software: you can redistribute it and/or modify
//		it under the terms of the GNU Lesser General Public License as published by
//		the Free Software Foundation, either version 3 of the License, or
//		(at your option) any later version.
//
//		This program is distributed in the hope that it will be useful,
//		but WITHOUT ANY WARRANTY; without even the implied warranty of
//		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//		GNU Lesser General Public License for more details.
//
//		You will receive a copy of the GNU Lesser General Public License along 
//		with this program.  If not, see http://www.gnu.org/licenses/.
//		--------------------------------------------------------------
//
#include "stdafx.h"
#include "VDJSpoutReceiver64.h"
#include "PixelShader.h"

template <class T> void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

struct D3DXCOLOR
{
public:
	D3DXCOLOR() = default;
	D3DXCOLOR(FLOAT r, FLOAT g, FLOAT b, FLOAT a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	operator FLOAT* ()
	{
		return &r;
	}

	FLOAT r, g, b, a;
};

struct TLVERTEX
{
	FLOAT x, y, z;
	D3DXCOLOR colour;
	FLOAT u, v;
};

VDJ_EXPORT HRESULT __stdcall DllGetClassObject(const GUID &rclsid, const GUID &riid, void** ppObject)
{ 
	// VDJ 8 only
	if(memcmp(&rclsid, &CLSID_VdjPlugin8, sizeof(GUID)) != 0) return CLASS_E_CLASSNOTAVAILABLE; 
    if(memcmp(&riid, &IID_IVdjPluginVideoFx8, sizeof(GUID)) != 0) return CLASS_E_CLASSNOTAVAILABLE; 

	*ppObject = new SpoutReceiverPlugin(); 

    return NO_ERROR; 
}

SpoutReceiverPlugin::SpoutReceiverPlugin()
{
	// OpenSpoutConsole(); // For debugging
	// printf("SpoutReceiver - 2.13\n");

	m_pVDJdevice = nullptr;
	m_pImmediateContext = nullptr;
	m_pSharedTexture = nullptr;
	m_pPixelShader = nullptr;
	m_pVertexBuffer = nullptr;
	m_dxShareHandle = NULL;
	m_dwFormat = 0;
	m_pTexture = nullptr;
	pSRView = nullptr;
	m_SenderName[0] = 0;
	m_SenderWidth = 0;
	m_SenderHeight = 0;
	bSpoutInitialized = false;
	bSpoutPanelOpened = false;
	bSpoutPanelActive = false;
	m_ShExecInfo = { 0 };
	
	bSpoutOut = false; // toggle for plugin start and stop
	bIsClosing = false; // plugin is not closing
	SelectButton = 0;

	m_noReceiveName[0] = 0; // Sender plugin on the same deck to avoid
	m_ReceiverName[0] = 0; // The name of this effect

	oldWidth = 0;
	oldHeight = 0;
	stride = sizeof(TLVERTEX);
	offset = 0;

}

SpoutReceiverPlugin::~SpoutReceiverPlugin()
{
	
}

HRESULT __stdcall SpoutReceiverPlugin::OnLoad()
{
	// The deck the plugin was loaded
	// is it a standard one ? the master one ? yes master, no, standard, else it is special one or fail
	double query = nan(""); // NAN;
	int deck = (S_OK == GetInfo("get_plugindeck", &query)) ? (int)query : MININT;
	
	// Create a sender name not to connect to
	// i.e. a sender plugin on the same deck
	sprintf_s(m_noReceiveName, 256, "VDJSpoutSender64 %s%s", (deck == 0) ? "" : "Deck ", (deck <= 0 ? (deck >= -3 ? std::array <std::string, 4> { "master", "sampler", "mic", "aux" }.at(-(int)deck) : std::to_string((int)deck)) : std::to_string((int)deck)).c_str()); // add deck n

	// Sender selection button
	DeclareParameterButton(&SelectButton, 1, "Sender", "Sender");

	// Position control button
	DeclareParameterPosition(m_position, 2, "Position", "pos");

	// To prevent double selection problem for plugin activation
	// Sender name appears in the control GUI
	DeclareParameterString(m_SenderName, 3, "Sender", "Sender", SpoutMaxSenderNameLen);

	// The dll name of this effect including where it is loaded
	sprintf_s(m_ReceiverName, 256, "%s", (GetDLLName() + " " + deck_name(deck)).c_str());

	// Enable logging to show Spout error logs
	// Log file saved in AppData>Roaming>Spout
	if (GetDLLDir().find("\\Visualisations") != std::string::npos)
		strcat_s(m_ReceiverName, 256, " Visualisation");
	else
		strcat_s(m_ReceiverName, 256, " Effect");

	EnableSpoutLogFile(m_ReceiverName);
	SetSpoutLogLevel(SPOUT_LOG_WARNING); // show only warnings and errors

    return NO_ERROR;
}

HRESULT __stdcall SpoutReceiverPlugin::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
	infos->Author = "Lynn Jarvis";
	infos->PluginName = (char *)"SpoutReceiver";
	infos->Flags = VDJFLAG_VIDEO_OVERLAY | VDJFLAG_VIDEO_OUTPUTRESOLUTION;
	infos->Description = (char *)"Receives frames from a Spout Sender\nSpout : https://Spout.zeal.co/";
	infos->Version = (char *)"v2.13";
    infos->Bitmap = NULL;
    return NO_ERROR;
}


HRESULT __stdcall SpoutReceiverPlugin::OnStart()
{
	// Initialize again in case OnDeviceClose was called
	// Noted for toggle of main window
	if (!m_pImmediateContext) {
		HRESULT hr = InitializeDraw();
		if (hr != NO_ERROR) {
			bSpoutOut = false; // we can't draw
			return hr;
		}
	}

	bIsClosing = false; // is not closing
	bSpoutOut = true;
	return NO_ERROR;

}

HRESULT __stdcall SpoutReceiverPlugin::OnStop()
{
	bSpoutOut = false;
	return NO_ERROR;
}

// When DirectX is initialized or closed, these functions will be called
HRESULT __stdcall  SpoutReceiverPlugin::OnDeviceInit() 
{
	bIsClosing = false; // is not closing
	// Set everything up for draw
	return InitializeDraw();
}

HRESULT __stdcall SpoutReceiverPlugin::OnDeviceClose() 
{
	bIsClosing = true; // It is closing to don't do anything in draw

	// Release everything
	SafeRelease(&m_pTexture);
	SafeRelease(&m_pSharedTexture);
	m_SenderName[0] = 0;
	m_SenderWidth = 0;
	m_SenderHeight = 0;
	m_dxShareHandle = NULL;
	frame.CloseAccessMutex();
	frame.CleanupFrameCount();
	bSpoutInitialized = false;

	SafeRelease(&m_pVertexBuffer);
	SafeRelease(&m_pPixelShader);
	SafeRelease(&pSRView);
	SafeRelease(&m_pImmediateContext);
	m_pVDJdevice = nullptr;

	return S_OK;
}

ULONG __stdcall SpoutReceiverPlugin::Release()
{
	bSpoutOut = false;

	delete this; 
	return S_OK;
}


HRESULT __stdcall SpoutReceiverPlugin::OnParameter(int ParamID) 
{
	// Activate SpoutPanel to select a sender
	if (ParamID == 1)
		OpenSpoutPanel();

	return S_OK;
}


// Receive a sender texture and transfer to VirtualDJ
HRESULT __stdcall SpoutReceiverPlugin::OnDraw()
{
	if (bIsClosing)
		return S_FALSE;

	if (bSpoutOut) {

		UpdateVertices();

		if (ReceiveSpoutTexture() && bSpoutInitialized && m_pTexture && m_pImmediateContext && pSRView) {
			// A local texture, m_pTexture, has been updated
			// Activate local shader
			m_pImmediateContext->PSSetShader(m_pPixelShader, nullptr, 0);
			// Bind our texture shader resource view
			m_pImmediateContext->PSSetShaderResources(0, 1, &pSRView);
			// Draw the texture
			stride = sizeof(TLVERTEX);
			offset = 0;
			m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
			m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_pImmediateContext->Draw(6, 0);
		}
	}
	// return S_OK if you actually draw the texture on the device
	return S_OK;
}


HRESULT SpoutReceiverPlugin::InitializeDraw()
{
	SafeRelease(&m_pVertexBuffer);
	SafeRelease(&m_pPixelShader);
	SafeRelease(&pSRView);
	SafeRelease(&m_pImmediateContext);
	m_pVDJdevice = nullptr;

	HRESULT hr = GetDevice(VdjVideoEngineDirectX11, (void**)&m_pVDJdevice);
	if (hr != S_OK) {
		SpoutLogWarning("SpoutReceiverPlugin::InitializeDraw() - VirtualDJ GetDevice failed");
		return E_FAIL;
	}

	m_pVDJdevice->GetImmediateContext(&m_pImmediateContext);
	if (!m_pImmediateContext) {
		SpoutLogWarning("SpoutReceiverPlugin::InitializeDraw() - VirtualDJ GetImmediateContext failed");
		return E_FAIL;
	}

	D3D11_BUFFER_DESC bd = { 0 };
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(TLVERTEX) * 6;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_pVDJdevice->CreateBuffer(&bd, NULL, &m_pVertexBuffer);

	UpdateVertices();

	m_pVDJdevice->CreatePixelShader(PixelShaderCode, sizeof(PixelShaderCode), nullptr, &m_pPixelShader);

	return NO_ERROR;
}


bool SpoutReceiverPlugin::UpdateVertices()
{

	D3D11_MAPPED_SUBRESOURCE ms;
	HRESULT hr = m_pImmediateContext->Map(m_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	if (hr != S_OK)
		return false;

#define x0 m_position[0]
#define y0 m_position[1]
#define x1 m_position[2]
#define y1 m_position[3]
#define	pos_x ((x0)*width)
#define pos_y ((y0)*height)

#define	pos_width (((x1)-(x0))*width)
#define pos_height (((y1)-(y0))*height)

	TLVERTEX* vertices = (TLVERTEX*)ms.pData;
	const D3DXCOLOR color = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
	vertices[0].colour = color;
	vertices[0].x = (FLOAT)pos_width + pos_x * 2;
	vertices[0].y = pos_y;// 0;
	vertices[0].z = 0.0f;

	vertices[1].colour = color;
	vertices[1].x = (FLOAT)pos_width + pos_x * 2;
	vertices[1].y = (FLOAT)pos_height + pos_y * 2;
	vertices[1].z = 0.0f;

	vertices[2].colour = color;
	vertices[2].x = pos_x;// 0;
	vertices[2].y = (FLOAT)pos_height + pos_y * 2;
	vertices[2].z = 0.0f;

	vertices[3].colour = color;
	vertices[3].x = 1 + pos_x;// 1;
	vertices[3].y = (FLOAT)pos_height + pos_y * 2;
	vertices[3].z = 0.0f;

	vertices[4].colour = color;
	vertices[4].x = pos_x;// 0;
	vertices[4].y = pos_y;//0;
	vertices[4].z = 0.0f;

	vertices[5].colour = color;
	vertices[5].x = (FLOAT)pos_width + pos_x * 2;
	vertices[5].y = pos_y;// 0;
	vertices[5].z = 0.0f;

	vertices[0].u = 1; vertices[0].v = 0;
	vertices[1].u = 1; vertices[1].v = 1;
	vertices[2].u = 0; vertices[2].v = 1;
	vertices[3].u = 0; vertices[3].v = 1;
	vertices[4].u = 0; vertices[4].v = 0;
	vertices[5].u = 1; vertices[5].v = 0;

	m_pImmediateContext->Unmap(m_pVertexBuffer, NULL);

	oldWidth = width;
	oldHeight = height;

	return true;
}

bool SpoutReceiverPlugin::ReceiveSpoutTexture()
{
	// Set the initial width, height, format and share handle to globals.
	// They are returned from the sender and can be tested for change.
	unsigned int senderwidth = m_SenderWidth;
	unsigned int senderheight = m_SenderHeight;
	DWORD senderFormat = m_dwFormat;
	HANDLE dxShareHandle = m_dxShareHandle;

	// Check to see if SpoutPanel has been opened
	// If it has, the sender name will be different
	if (CheckSpoutPanel(m_SenderName)) {
		// The user has selected a different sender
		// and the new name has been returned
		if (bSpoutInitialized) {
			// Close the named access mutex and frame counting
			frame.CloseAccessMutex();
			frame.CleanupFrameCount();
		}
		// The texture could be a different format
		// Release the local one to re-create
		SafeRelease(&m_pTexture);
		SafeRelease(&m_pSharedTexture); // The shared texture is different
		m_dxShareHandle = NULL; // And the share handle
		bSpoutInitialized = false;
		// A new sender has been selected, so continue
	}

	// Find if the sender exists and return width, height, sharehandle and format.
	if (spoutsender.FindSender(m_SenderName, senderwidth, senderheight, dxShareHandle, senderFormat)) {

		// It's possible that the sharehandle could be null
		// for a 2.006 sender using shared memory instead of a shared texture
		if (!dxShareHandle)
			return false;

		// Don't receive from VDJ itself or master
		if (strncmp(m_SenderName, "VDJSpoutSender64", 16) == 0) {
			char* n = m_SenderName + 16;
			if (*n == 0 ||
				strcmp(m_SenderName, m_noReceiveName) == 0 ||
				strcmp(m_SenderName, (std::string(m_noReceiveName) + "_Release").c_str()) == 0 ||
				strncmp(n, " deck Master", 12) == 0 ||
				strncmp(n, " master", 7) == 0
				) {
				m_SenderName[0] = 0;
				return false;
			}
		}

		// Check here for sender size or format changes to re-create the local texture
		// or create one if it does not exist yet
		if (!m_pTexture 
			|| m_dxShareHandle != dxShareHandle
			|| m_dwFormat != senderFormat 
			|| m_SenderWidth != senderwidth 
			|| m_SenderHeight != senderheight) {

			// Save the sender's width, height, format and share handle
			m_SenderWidth = senderwidth;
			m_SenderHeight = senderheight;
			m_dwFormat = senderFormat;
			m_dxShareHandle = dxShareHandle;

			// Release any existing shared texture pointer
			// so it can be created again from the new share handle
			SafeRelease(&m_pSharedTexture);

			// Existing local texture must be released and re-created to the new size
			if (m_pVDJdevice) {

				SafeRelease(&m_pTexture);

				// Formats compatible with the NVIDIA GL/DX interop
				//
				// DX9 
				// 21 - D3DFMT_A8R8G8B8
				// 22 - D3DFMT_X8R8G8B8
				//
				// DX11
				// 28 - DXGI_FORMAT_R8G8B8A8_UNORM
				// 87 - DXGI_FORMAT_B8G8R8A8_UNORM
				// 88 - DXGI_FORMAT_B8G8R8AX_UNORM (untested)
				// 02 - DXGI_FORMAT_R32G32B32A32_FLOAT
				// 10 - DXGI_FORMAT_R16G16B16A16_FLOAT
				// 13 - DXGI_FORMAT_R16G16B16A16_SNORM
				// 24 - DXGI_FORMAT_R10G10B10A2_UNORM

				// DX9 formats are not supported for creation of a DX11 shader resource view
				// so use the default DXGI_FORMAT_B8G8R8A8_UNORM.
				// Otherwise DX11 formats will be detected correctly.
				if (m_dwFormat == (DWORD)D3DFMT_A8R8G8B8 || m_dwFormat == (DWORD)D3DFMT_X8R8G8B8)
					CreateDX11Texture(m_pVDJdevice, m_SenderWidth, m_SenderHeight, DXGI_FORMAT_B8G8R8A8_UNORM, &m_pTexture);
				else
					CreateDX11Texture(m_pVDJdevice, m_SenderWidth, m_SenderHeight, (DXGI_FORMAT)m_dwFormat, &m_pTexture);
			}
		}

		// Set up the receiver if not initialized yet
		if (!bSpoutInitialized) {
			// The local texture will have been created on size change
			// Create a named sender mutex for access to the shared texture
			frame.CreateAccessMutex(m_SenderName);
			// Enable frame counting to get the sender frame number an fps
			frame.EnableFrameCount(m_SenderName);
			bSpoutInitialized = true;
		}

		// If no sender shared texture pointer, create one from the share handle
		if (!m_pSharedTexture) {
			// Open the shared texture
			spoutdx.OpenDX11shareHandle(m_pVDJdevice, &m_pSharedTexture, m_dxShareHandle);
		}

		// Access the sender shared texture
		// When it gets access and the frame is new
		// the new shared texture pointer is retrieved.
		// If not, use m_pTexture from the previous frame
		if (frame.CheckAccess()) {
			// Check if the sender has produced a new frame
			if (frame.GetNewFrame() && m_pVDJdevice) {
				// m_dxShareHandle was retrieved from the sender
				// and the shared texture pointer (m_pSharedTexture) has been retrieved via the sharehandle
				// Now copy the shared texture to the local texture which will be the same size
				if (m_pTexture && m_pSharedTexture && m_pImmediateContext) {
					m_pImmediateContext->CopyResource(m_pTexture, m_pSharedTexture);
					// No need to flush because the shared texture is not modified on this device
				}
			}
			// Allow access to the shared texture
			frame.AllowAccess();
		}

		return true;

	} // sender exists
	
	// No sender found
	if (bSpoutInitialized) {
		// The connected sender closed
		// Zero the name to get the active sender if it is running
		m_SenderName[0] = 0;
		// No need to reset the size or re-create the local texture
		SafeRelease(&m_pSharedTexture); // The shared texture no longer exists
		m_dxShareHandle = NULL; // Or the share handle
		// Close the named access mutex and frame counting
		frame.CloseAccessMutex();
		frame.CleanupFrameCount();
		// Initialize them again when a sender is found
		bSpoutInitialized = false;
	}

	return false;

}

//
// The following functions are adapted from Spout SDK equivalents
//

// Create a texture that is not shared
// and will be used to create a shader resource view
bool SpoutReceiverPlugin::CreateDX11Texture(ID3D11Device* pd3dDevice,
						unsigned int width,	unsigned int height, 
						DXGI_FORMAT format, ID3D11Texture2D** ppTexture)
{
	ID3D11Texture2D* pTexture = nullptr;
	DXGI_FORMAT texformat = DXGI_FORMAT_B8G8R8A8_UNORM;
	if (format != 0)
		texformat = format;

	if (pd3dDevice == NULL) {
		SpoutLogFatal("SpoutReceiverPlugin::CreateDX11Texture NULL device");
		return false;
	}

	SpoutLogNotice("SpoutReceiverPlugin::CreateDX11Texture");
	SpoutLogNotice("    pd3dDevice = 0x%.7X, width = %d, height = %d, format = %d", PtrToUint(pd3dDevice), width, height, format);

	// Create a new DX11 texture
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	// Bind flag for creating a shader resource view
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = 0; // This texture will not be shared
	desc.Format = texformat; // Default DXGI_FORMAT_B8G8R8A8_UNORM
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.SampleDesc.Quality = 0;
	desc.SampleDesc.Count = 1;
	desc.MipLevels = 1;
	desc.ArraySize = 1;

	HRESULT res = pd3dDevice->CreateTexture2D(&desc, NULL, &pTexture);

	if (res != S_OK) {
		char tmp[256];
		sprintf_s(tmp, 256, "SpoutReceiverPlugin::CreateDX11Texture ERROR - [0x%.7X] : ", (UINT)res);
		switch (res) {
		case D3DERR_INVALIDCALL:
			strcat_s(tmp, 256, "D3DERR_INVALIDCALL");
			break;
		case E_INVALIDARG:
			strcat_s(tmp, 256, "E_INVALIDARG");
			break;
		case E_OUTOFMEMORY:
			strcat_s(tmp, 256, "E_OUTOFMEMORY");
			break;
		default:
			strcat_s(tmp, 256, "Unlisted error");
			break;
		}
		SpoutLogFatal("%s", tmp);
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srdesc;
	ZeroMemory(&srdesc, sizeof(srdesc));
	srdesc.Format = texformat; // Matching format is important
	srdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srdesc.Texture2D.MostDetailedMip = 0;
	srdesc.Texture2D.MipLevels = 1;
	SafeRelease(&pSRView);

	res = m_pVDJdevice->CreateShaderResourceView(pTexture, &srdesc, &pSRView);
	if (res != S_OK) {
		SpoutLogWarning("SpoutReceiverPlugin::CreateDX11Texture - could not create shader resouce view");
	}

	// Return the DX11 texture pointer
	*ppTexture = pTexture;

	SpoutLogNotice("    pTexture = 0x%.7X",  PtrToUint(pTexture));

	return true;

}


//
// Check whether SpoutPanel opened and return the new sender name
//
bool SpoutReceiverPlugin::CheckSpoutPanel(char *sendername, int maxchars)
{
	// If SpoutPanel has been activated, test if the user has clicked OK
	if (bSpoutPanelOpened) { // User has activated spout panel

		SharedTextureInfo TextureInfo;
		HANDLE hMutex = NULL;
		DWORD dwExitCode;
		char newname[256];
		bool bRet = false;

		// Must find the mutex to signify that SpoutPanel has opened
		// and then wait for the mutex to close
		hMutex = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");

		// Has it been activated 
		if (!bSpoutPanelActive) {
			// If the mutex has been found, set the active flag true and quit
			// otherwise on the next round it will test for the mutex closed
			if (hMutex) bSpoutPanelActive = true;
		}
		else if (!hMutex) { // It has now closed
			bSpoutPanelOpened = false; // Don't do this part again
			bSpoutPanelActive = false;
			// call GetExitCodeProcess() with the hProcess member of SHELLEXECUTEINFO
			// to get the exit code from SpoutPanel
			if (m_ShExecInfo.hProcess) {
				GetExitCodeProcess(m_ShExecInfo.hProcess, &dwExitCode);
				// Only act if exit code = 0 (OK)
				if (dwExitCode == 0) {
					// SpoutPanel has been activated and OK clicked
					// Test the active sender which should have been set by SpoutPanel
					newname[0] = 0;
					if (!spoutsender.GetActiveSender(newname)) {
						// Otherwise the sender might not be registered.
						// SpoutPanel always writes the selected sender name to the registry.
						if (ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutPanel", "Sendername", newname)) {
							// Register the sender if it exists
							if (newname[0] != 0) {
								if (spoutsender.getSharedInfo(newname, &TextureInfo)) {
									// Register in the list of senders and make it the active sender
									spoutsender.RegisterSenderName(newname);
									spoutsender.SetActiveSender(newname);
								}
							}
						}
					}
					// Now do we have a valid sender name ?
					if (newname[0] != 0) {
						// Pass back the new name
						strcpy_s(sendername, maxchars, newname);
						bRet = true;
					} // endif valid sender name
				} // endif SpoutPanel OK
			} // got the exit code
		} // endif no mutex so SpoutPanel has closed
		// If we opened the mutex, close it now or it is never released
		if (hMutex) CloseHandle(hMutex);
		return bRet;
	} // SpoutPanel has not been opened

	return false;

}

//
// Pop up SpoutPanel to allow the user to select a sender
// activated by RH click in this application
//
bool SpoutReceiverPlugin::OpenSpoutPanel()
{
	HANDLE hMutex1 = NULL;
	HMODULE module = NULL;
	char path[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH];

	// The selected sender is then the "Active" sender and this receiver switches to it.
	// If Spout is not installed, SpoutPanel.exe has to be in the same folder
	// as this executable. This rather complicated process avoids having to use a dialog
	// which causes problems with host GUI messaging.

	// First find if there has been a Spout installation >= 2.002 with an install path for SpoutPanel.exe
	if (!ReadPathFromRegistry(HKEY_CURRENT_USER, "Software\\Leading Edge\\SpoutPanel", "InstallPath", path)) {
		// Path not registered so find the path of the host program
		// where SpoutPanel should have been copied
		module = GetModuleHandle(NULL);
		GetModuleFileNameA(module, path, MAX_PATH);
		_splitpath_s(path, drive, MAX_PATH, dir, MAX_PATH, fname, MAX_PATH, NULL, 0);
		_makepath_s(path, MAX_PATH, drive, dir, "SpoutPanel", ".exe");
		// Does SpoutPanel.exe exist in this path ?
		if (_access(path, 0) != -1) {
			// Try the current working directory
			if (_getcwd(path, MAX_PATH)) {
				strcat_s(path, MAX_PATH, "\\SpoutPanel.exe");
				// Does SpoutPanel exist here?
				if (_access(path, 0) != -1) {
					SpoutLogWarning("SpoutReceiverPlugin::OpenSpoutPanel - SpoutPanel path not found");
					return false;
				}
			}
		}
	}

	// Check whether the panel is already running
	// Try to open the application mutex.
	hMutex1 = OpenMutexA(MUTEX_ALL_ACCESS, 0, "SpoutPanel");
	if (!hMutex1) {
		// No mutex, so not running, so can open it

		// First release any orphaned senders if the name exists
		// in the sender list but the shared memory info does not
		// So that the sender list is clean
		spoutsender.CleanSenders();

		// Use ShellExecuteEx so we can test its return value later
		ZeroMemory(&m_ShExecInfo, sizeof(m_ShExecInfo));
		m_ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		m_ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		m_ShExecInfo.hwnd = NULL;
		m_ShExecInfo.lpVerb = NULL;
		m_ShExecInfo.lpFile = (LPCSTR)path;
		m_ShExecInfo.lpDirectory = NULL;
		m_ShExecInfo.nShow = SW_SHOW;
		m_ShExecInfo.hInstApp = NULL;
		ShellExecuteExA(&m_ShExecInfo);
		Sleep(125); // allow time for SpoutPanel to open nominally 0.125s
		//
		// The flag "bSpoutPanelOpened" is set here to indicate that the user
		// has opened the panel to select a sender. This flag is local to 
		// this process so will not affect any other receiver instance
		// Then when the selection panel closes, sender name is tested
		//
		bSpoutPanelOpened = true;
	}
	else {
		// The mutex exists, so another instance is already running.
		// Find the SpoutPanel window and bring it to the top.
		// SpoutPanel is opened as topmost anyway but pop it to
		// the front in case anything else has stolen topmost.
		HWND hWnd = FindWindowA(NULL, (LPCSTR)"SpoutPanel");
		if (hWnd && IsWindow(hWnd)) {
			SetForegroundWindow(hWnd);
			// prevent other windows from hiding the dialog
			// and open the window wherever the user clicked
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
		}
		else if (path[0]) {
			// If the window was not found but the mutex exists
			// and SpoutPanel is installed, it has crashed.
			// Terminate the process and the mutex or the mutex will remain
			// and SpoutPanel will not be started again.
			PROCESSENTRY32 pEntry;
			pEntry.dwSize = sizeof(pEntry);
			bool done = false;
			// Take a snapshot of all processes and threads in the system
			HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
			if (hProcessSnap == INVALID_HANDLE_VALUE) {
				SpoutLogError("SpoutReceiverPlugin::OpenSpoutPanel - CreateToolhelp32Snapshot error");
			}
			else {
				// Retrieve information about the first process
				BOOL hRes = Process32First(hProcessSnap, &pEntry);
				if (!hRes) {
					SpoutLogError("SpoutReceiverPlugin::OpenSpoutPanel - Process32First error");
					CloseHandle(hProcessSnap);
				}
				else {
					// Look through all processes
					while (hRes && !done) {
						int value = _tcsicmp(pEntry.szExeFile, _T("SpoutPanel.exe"));
						if (value == 0) {
							HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, (DWORD)pEntry.th32ProcessID);
							if (hProcess != NULL) {
								// Terminate SpoutPanel and it's mutex
								TerminateProcess(hProcess, 9);
								CloseHandle(hProcess);
								done = true;
							}
						}
						if (!done)
							hRes = Process32Next(hProcessSnap, &pEntry); // Get the next process
						else
							hRes = NULL; // found SpoutPanel
					}
					CloseHandle(hProcessSnap);
				}
			}
			// Now SpoutPanel will start the next time the user activates it
		} // endif SpoutPanel crashed
	} // endif SpoutPanel already open

	// If we opened the mutex, close it now or it is never released
	if (hMutex1) CloseHandle(hMutex1);

	return true;

} // end OpenSpoutPanel


//
// For overlay effect 
// (https://github.com/nicotux)
//

// construct the Full qualified deck name [[deck <deck>|Master|Mic|Sampler][_[Audio|Video]]|unknown]
std::string SpoutReceiverPlugin::deck_name(int deck)
{
	double query = NAN;
	return ((deck > 0) ? "deck " + std::to_string((int)deck)
		: ((deck >= -3) ? std::array <std::string, 4> { "Master", "Sampler", "Mic", "Aux" }.at(-(int)deck)
			: "")
		)
		+ (GetInfo("is_releasefx ? true : false", &query) != S_OK ? "" : (query ? "_Release" : ""))
		+ (GetInfo("is_audioonlyvisualisation ? true : false", &query) != S_OK ? "" : (query ? "_Source" : ""))
		+ (GetInfo("is_colorfx ? true : false", &query) != S_OK ? "" : (query ? "_Colorfx" : ""))
		;
}

// Get the filename of the dll
// Note: hInstance is not valid for the plugin in the constructor
std::string SpoutReceiverPlugin::GetDLLName()
{
	char		path[MAX_PATH * sizeof(char)];
	std::string dllName;
	size_t		pos;

	GetModuleFileNameA(hInstance, path, MAX_PATH * sizeof(char));
	dllName = path;
	pos = dllName.rfind('\\') + 1;
	return dllName.substr(pos, dllName.rfind('.') - pos);

}

// Get the folder the dll stays in
std::string SpoutReceiverPlugin::GetDLLDir()
{
	char path[MAX_PATH * sizeof(char)];
	std::string dllDir;

	// Get the DLL full path
	GetModuleFileNameA(hInstance, path, MAX_PATH * sizeof(char));
	dllDir = path;
	return dllDir.substr(0, dllDir.rfind('\\'));
}

