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
//		08.12.15	info flag VDJFLAG_VIDEO_VISUALISATION only
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
//		05.01.21 - Add VDJFLAG_VIDEO_OUTPUTRESOLUTION flag
//				   Add flush on VDJ device after shared texture copy
//				   Rebuild with latest 2.007 code from develop branch
//				   64bit for 2.007 VS2017 /MT - Version 2.01
//		06.01.21 - Add local shader to render texture to VDJ
//				   Only fetch VDJ device/context on device creation
//				   Create shader resource view with texture instead of every frame
//
//		------------------------------------------------------------
//
//		Copyright (C) 2015-2021. Lynn Jarvis, Leading Edge. Pty. Ltd.
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
	// Enable logging to show Spout logs
	// Log file saved in AppData>Roaming>Spout
	EnableSpoutLogFile("VDJSpoutReceiver64.log");
	SetSpoutLogLevel(SPOUT_LOG_WARNING); // show only warnings and errors
	// OpenSpoutConsole(); // For debugging

	pDevice = nullptr;
	pImmediateContext = nullptr;
	g_pSharedTexture = nullptr;
	pPixelShader = nullptr;
	pVertexBuffer = nullptr;
	g_dxShareHandle = NULL;
	g_dwFormat = 0;
	g_pTexture = nullptr;
	pSRView = nullptr;
	g_SenderName[0] = 0;
	g_SenderWidth = 0;
	g_SenderHeight = 0;
	bSpoutInitialized = false;
	bSpoutPanelOpened = false;
	bSpoutPanelActive = false;

	bSpoutOut = false; // toggle for plugin start and stop
	bIsClosing = false; // plugin is not closing
	SelectButton = 0;

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
	DeclareParameterButton(&SelectButton, 1, "Sender", "Sender");
    return NO_ERROR;
}

HRESULT __stdcall SpoutReceiverPlugin::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
	infos->Author = "Lynn Jarvis";

	infos->PluginName = (char *)"VDJSpoutReceiver64";
	infos->Description = (char *)"Receives frames from a Spout Sender\nas a visualisation plugin\nSpout : http://Spout.zeal.co/";
	infos->Version = (char *)"v2.01";
    infos->Bitmap = NULL;
	infos->Flags = VDJFLAG_VIDEO_OUTPUTRESOLUTION | VDJFLAG_VIDEO_OUTPUTRESOLUTION;

    return NO_ERROR;
}


HRESULT __stdcall SpoutReceiverPlugin::OnStart()
{
	bSpoutOut = true;
	return NO_ERROR;
}

HRESULT __stdcall SpoutReceiverPlugin::OnStop()
{
	bSpoutOut = false;
	return NO_ERROR;
}

bool SpoutReceiverPlugin::UpdateVertices()
{
	D3D11_MAPPED_SUBRESOURCE ms;
	HRESULT hr = pImmediateContext->Map(pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	if (hr != S_OK)
		return false;

	TLVERTEX* vertices = (TLVERTEX*)ms.pData;

	const D3DXCOLOR color = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
	vertices[0].colour = color;
	vertices[0].x = (FLOAT)width;
	vertices[0].y = 0;
	vertices[0].z = 0.0f;

	vertices[1].colour = color;
	vertices[1].x = (FLOAT)width;
	vertices[1].y = (FLOAT)height;
	vertices[1].z = 0.0f;

	vertices[2].colour = color;
	vertices[2].x = 0;
	vertices[2].y = (FLOAT)height;
	vertices[2].z = 0.0f;

	vertices[3].colour = color;
	vertices[3].x = 1;
	vertices[3].y = (FLOAT)height;
	vertices[3].z = 0.0f;

	vertices[4].colour = color;
	vertices[4].x = 0;
	vertices[4].y = 0;
	vertices[4].z = 0.0f;

	vertices[5].colour = color;
	vertices[5].x = (FLOAT)width;
	vertices[5].y = 0;
	vertices[5].z = 0.0f;

	vertices[0].u = 1.0f; vertices[0].v = 0.0f;
	vertices[1].u = 1.0f; vertices[1].v = 1.0f;
	vertices[2].u = 0.0f; vertices[2].v = 1.0f;
	vertices[3].u = 0.0f; vertices[3].v = 1.0f;
	vertices[4].u = 0.0f; vertices[4].v = 0.0f;
	vertices[5].u = 1.0f; vertices[5].v = 0.0f;

	pImmediateContext->Unmap(pVertexBuffer, NULL);

	return true;
}

// When DirectX is initialized or closed, these functions will be called
HRESULT __stdcall  SpoutReceiverPlugin::OnDeviceInit() 
{
	bIsClosing = false; // is not closing

	HRESULT hr = GetDevice(VdjVideoEngineDirectX11, (void**)&pDevice);
	if (hr != S_OK)
		return E_FAIL;

	pDevice->GetImmediateContext(&pImmediateContext);
	if (!pImmediateContext)
		return E_FAIL;

	D3D11_BUFFER_DESC bd = { 0 };
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(TLVERTEX) * 6;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pDevice->CreateBuffer(&bd, NULL, &pVertexBuffer);

	oldWidth = width;
	oldHeight = height;

	UpdateVertices();

	pDevice->CreatePixelShader(PixelShaderCode, sizeof(PixelShaderCode) , nullptr, &pPixelShader);

	return S_OK;
}

HRESULT __stdcall SpoutReceiverPlugin::OnDeviceClose() 
{
	// Leave everything as-is to allow on/off for the master video
	// Re-init does not seem to affect it

	bIsClosing = true; // It is closing to don't do anything in draw

	if (pVertexBuffer)
	{
		pVertexBuffer->Release();
		pVertexBuffer = nullptr;
	}
	if (pPixelShader)
	{
		pPixelShader->Release();
		pPixelShader = nullptr;
	}
	if (pSRView)
	{
		pSRView->Release();
		pSRView = nullptr;
	}
	if (g_pTexture)
	{
		g_pTexture->Release();
		g_pTexture = nullptr;
	}
	if (pImmediateContext)
	{
		pImmediateContext->Release();
		pImmediateContext = nullptr;
	}
	pDevice = nullptr;

	return S_OK;
}

ULONG __stdcall SpoutReceiverPlugin::Release()
{
	g_pSharedTexture = nullptr;
	g_dxShareHandle = NULL;
	g_SenderName[0] = 0;
	g_SenderWidth = 0;
	g_SenderHeight = 0;
	bSpoutInitialized = false;
	bSpoutOut = false;

	delete this; 
	return S_OK;
}


HRESULT __stdcall SpoutReceiverPlugin::OnParameter(int ParamID) 
{
	// Activate SpoutPanel to select a sender
	OpenSpoutPanel();
	return S_OK;
}


// Receive a sender texture and transfer to VirtualDJ
HRESULT __stdcall SpoutReceiverPlugin::OnDraw()
{
	if (bIsClosing)
		return S_FALSE;

	if (bSpoutOut) {

		if (ReceiveSpoutTexture() && bSpoutInitialized && g_pTexture && pImmediateContext && pSRView) {
			// A local texture, g_pTexture, has been updated
			if (oldWidth != width || oldHeight != height)
				UpdateVertices();
			// Activate local shader
			pImmediateContext->PSSetShader(pPixelShader, nullptr, 0);
			// Bind our texture shader resource view
			pImmediateContext->PSSetShaderResources(0, 1, &pSRView);
			// Draw the texture
			stride = sizeof(TLVERTEX);
			offset = 0;
			pImmediateContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
			pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			pImmediateContext->Draw(6, 0);
		}
	}
	// return S_OK if you actually draw the texture on the device
	return S_OK;
}


bool SpoutReceiverPlugin::ReceiveSpoutTexture()
{
	// Set the initial width and height to globals
	// width and height are returned from the sender
	unsigned int senderwidth = g_SenderWidth;
	unsigned int senderheight = g_SenderHeight;

	// printf("ReceiveSpoutTexture %dx%d\n", senderwidth, senderheight);

	// Check to see if SpoutPanel has been opened
	// If it has, the sender name will be different
	if (CheckSpoutPanel(g_SenderName)) {
		// The user has selected a different sender
		// and the new name has been returned
		if (bSpoutInitialized) {
			// Close the named access mutex and frame counting
			frame.CloseAccessMutex();
			frame.CleanupFrameCount();
		}
		g_pSharedTexture = nullptr; // The shared texture is different
		g_dxShareHandle = NULL; // And the share handle
		// The local texture is only resized if the sender size has changed
		bSpoutInitialized = false;
		// A sender has been selected, so continue
	}

	// Find if the sender exists and return width, height, sharehandle and format.
	if (spoutsender.FindSender(g_SenderName, senderwidth, senderheight, g_dxShareHandle, g_dwFormat)) {
		
		// Don't receive from VDJ itself
		if (strcmp(g_SenderName, "VDJSpoutSender64") == 0) {
			g_SenderName[0] = 0;
			return false;
		}

		// Check here for sender size changes to resize the local texture
		if (g_SenderWidth != senderwidth || g_SenderHeight != senderheight) {

			// Save the sender's width and height to use as necessary
			g_SenderWidth = senderwidth;
			g_SenderHeight = senderheight;
			if (pDevice) {

				// Existing texture must be released
				if(g_pTexture) g_pTexture->Release();
				g_pTexture = nullptr;

				CreateDX11Texture(pDevice, g_SenderWidth, g_SenderHeight, DXGI_FORMAT_B8G8R8A8_UNORM, &g_pTexture);

			}
			else {
				printf("GetDevice failed\n");
				return false;
			}
		}

		// Set up the receiver if not initialized yet
		if (!bSpoutInitialized) {
			// printf("set up receiver %s, %dx%d\n", g_SenderName, g_SenderWidth, g_SenderHeight);
			// The local texture will have been created on size change
			// Create a named sender mutex for access to the shared texture
			frame.CreateAccessMutex(g_SenderName);
			// Enable frame counting to get the sender frame number and fps
			frame.EnableFrameCount(g_SenderName);
			bSpoutInitialized = true;
		}

		// Access the sender shared texture
		// When it gets access and the frame is new
		// the new shared texture pointer is retrieved.
		// If not, use g_pTexture from the previous frame
		if (frame.CheckAccess()) {
			// Check if the sender has produced a new frame
			if (frame.GetNewFrame() && pDevice) {
				// Get the VirtualDJ DX11 device
				// g_dxShareHandle was retrieved from the sender
				// The shared texture pointer can be retrieved via the sharehandle
				if (spoutdx.OpenDX11shareHandle(pDevice, &g_pSharedTexture, g_dxShareHandle)) {
					// Now copy the shared texture to the local texture which will be the same size
					if (g_pTexture && g_pSharedTexture && pImmediateContext) {
							pImmediateContext->CopyResource(g_pTexture, g_pSharedTexture);
							// The shared texture has been updated on this device
							// so flush must be called on this device
							pImmediateContext->Flush();
					}
				}
			}
			// Allow access to the shared texture
			frame.AllowAccess();
		}

		return true;

	} // sender exists
	else {
		if (bSpoutInitialized) {
			// The connected sender closed
			// Zero the name to get the active sender if it is running
			g_SenderName[0] = 0;
			// No need to reset the size or re-create the local texture
			g_pSharedTexture = nullptr; // The shared texture no longer exists
			g_dxShareHandle = NULL; // Or the share handle
			// Close the named access mutex and frame counting
			frame.CloseAccessMutex();
			frame.CleanupFrameCount();
			// Initialize them again when a sender is found
			bSpoutInitialized = false;
		}
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
	SpoutLogNotice("    pDevice = 0x%Ix, width = %d, height = %d, format = %d", (intptr_t)pd3dDevice, width, height, format);

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
		sprintf_s(tmp, 256, "SpoutReceiverPlugin::CreateDX11Texture ERROR - [0x%x] : ", res);
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
	srdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // Matching format is important
	srdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srdesc.Texture2D.MostDetailedMip = 0;
	srdesc.Texture2D.MipLevels = 1;
	if (pSRView)
	{
		pSRView->Release();
		pSRView = nullptr;
	}

	res = pDevice->CreateShaderResourceView(pTexture, &srdesc, &pSRView);
	if (res != S_OK)
	{
	}
	// Return the DX11 texture pointer
	*ppTexture = pTexture;

	SpoutLogNotice("    pTexture = 0x%Ix", pTexture);

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
			if (g_ShExecInfo.hProcess) {
				GetExitCodeProcess(g_ShExecInfo.hProcess, &dwExitCode);
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
		if (!PathFileExistsA(path)) {
			// Try the current working directory
			if (_getcwd(path, MAX_PATH)) {
				strcat_s(path, MAX_PATH, "\\SpoutPanel.exe");
				// Does SpoutPanel exist here?
				if (!PathFileExistsA(path)) {
					SpoutLogWarning("Tutorial07::OpenSpoutPanel - SpoutPanel path not found");
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
		// Use ShellExecuteEx so we can test its return value later
		ZeroMemory(&g_ShExecInfo, sizeof(g_ShExecInfo));
		g_ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		g_ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		g_ShExecInfo.hwnd = NULL;
		g_ShExecInfo.lpVerb = NULL;
		g_ShExecInfo.lpFile = (LPCSTR)path;
		g_ShExecInfo.lpDirectory = NULL;
		g_ShExecInfo.nShow = SW_SHOW;
		g_ShExecInfo.hInstApp = NULL;
		ShellExecuteExA(&g_ShExecInfo);
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
				SpoutLogError("Tutorial07::OpenSpoutPanel - CreateToolhelp32Snapshot error");
			}
			else {
				// Retrieve information about the first process
				BOOL hRes = Process32First(hProcessSnap, &pEntry);
				if (!hRes) {
					SpoutLogError("Tutorial07::OpenSpoutPanel - Process32First error");
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
