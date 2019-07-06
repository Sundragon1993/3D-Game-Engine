#include "Engine.h"


Engine *g_engine = NULL;

LRESULT CALLBACK WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_ACTIVATEAPP:
		g_engine->SetDeactiveFlag(!wparam);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProc(wnd, msg, wparam, lparam);
	}
}


Engine::Engine(EngineSetup *setup)
{
	
	m_loaded = false;

	
	m_setup = new EngineSetup;
	if (setup != NULL)
		memcpy(m_setup, setup, sizeof(EngineSetup));

	g_engine = this;

	// Prepare and register the window class.
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_CLASSDC;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_setup->instance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "WindowClass";
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&wcex);

	
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// Create the Direct3D interface.
	IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);

	
	g_deviceEnumeration = new DeviceEnumeration;
	if (g_deviceEnumeration->Enumerate(d3d) != IDOK)
	{
		SAFE_RELEASE(d3d);
		return;
	}

	// Create the window and retrieve a handle to it.
	m_window = CreateWindow("WindowClass", m_setup->name, g_deviceEnumeration->IsWindowed() ? WS_OVERLAPPED : WS_POPUP, 0, 0, 800, 600, NULL, NULL, m_setup->instance, NULL);

	// Prepare the device presentation parameters.
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	d3dpp.BackBufferWidth = g_deviceEnumeration->GetSelectedDisplayMode()->Width;
	d3dpp.BackBufferHeight = g_deviceEnumeration->GetSelectedDisplayMode()->Height;
	d3dpp.BackBufferFormat = g_deviceEnumeration->GetSelectedDisplayMode()->Format;
	d3dpp.BackBufferCount = m_setup->totalBackBuffers;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = m_window;
	d3dpp.Windowed = g_deviceEnumeration->IsWindowed();
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.FullScreen_RefreshRateInHz = g_deviceEnumeration->GetSelectedDisplayMode()->RefreshRate;
	if (g_deviceEnumeration->IsVSynced() == true)
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	else
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	
	SAFE_DELETE(g_deviceEnumeration);

	// Create the Direct3D device.
	if (FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_window, D3DCREATE_MIXED_VERTEXPROCESSING, &d3dpp, &m_device)))
		return;

	
	SAFE_RELEASE(d3d);

	
	m_device->SetRenderState(D3DRS_LIGHTING, false);

	// Set the texture filters to use anisotropic texture filtering.
	m_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	m_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	m_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	
	D3DXMATRIX projMatrix;
	D3DXMatrixPerspectiveFovLH(&projMatrix, D3DX_PI / 4, (float)d3dpp.BackBufferWidth / (float)d3dpp.BackBufferHeight, 0.1f / m_setup->scale, 1000.0f / m_setup->scale);
	m_device->SetTransform(D3DTS_PROJECTION, &projMatrix);

	// Store the display mode details.
	m_displayMode.Width = d3dpp.BackBufferWidth;
	m_displayMode.Height = d3dpp.BackBufferHeight;
	m_displayMode.RefreshRate = d3dpp.FullScreen_RefreshRateInHz;
	m_displayMode.Format = d3dpp.BackBufferFormat;

	// The swap chain always starts on the first back buffer.
	m_currentBackBuffer = 0;

	// Create the sprite interface.
	D3DXCreateSprite(m_device, &m_sprite);

	// Create the linked lists of states.
	m_states = new LinkedList< State >;
	m_currentState = NULL;

	// Create the resource managers.
	m_scriptManager = new ResourceManager< Script >;
	m_materialManager = new ResourceManager< Material >(m_setup->CreateMaterialResource);
	m_meshManager = new ResourceManager< Mesh >;

	// Create the input object.
	m_input = new Input(m_window);

	// Create the network object.
	m_network = new Network(m_setup->guid, m_setup->HandleNetworkMessage);

	// Create the sound system.
	m_soundSystem = new SoundSystem(m_setup->scale);

	// Create the scene manager.
	m_sceneManager = new SceneManager(m_setup->scale, m_setup->spawnerPath);

	srand(timeGetTime());

	
	if (m_setup->StateSetup != NULL)
		m_setup->StateSetup();

	
	m_loaded = true;
}

Engine::~Engine()
{
	// Ensure the engine is loaded.
	if (m_loaded == true)
	{
		// Destroy the states linked lists.
		if (m_currentState != NULL)
			m_currentState->Close();
		SAFE_DELETE(m_states);

		// Destroy everything.
		SAFE_DELETE(m_sceneManager);
		SAFE_DELETE(m_soundSystem);
		SAFE_DELETE(m_network);
		SAFE_DELETE(m_input);
		SAFE_DELETE(m_meshManager);
		SAFE_DELETE(m_materialManager);
		SAFE_DELETE(m_scriptManager);

		// Release the sprite interface.
		SAFE_RELEASE(m_sprite);

		// Release the device.
		SAFE_RELEASE(m_device);
	}

	// Uninitialise the COM.
	CoUninitialize();

	// Unregister the window class.
	UnregisterClass("WindowClass", m_setup->instance);

	// Destroy the engine setup structure.
	SAFE_DELETE(m_setup);
}

//-----------------------------------------------------------------------------
// Enters the engine into the main processing loop.
//-----------------------------------------------------------------------------
void Engine::Run()
{
	
	if (m_loaded == true)
	{
		// Show the window.
		ShowWindow(m_window, SW_NORMAL);

		
		ViewerSetup viewer;

		
		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else if (!m_deactive)
			{
				
				unsigned long currentTime = timeGetTime();
				static unsigned long lastTime = currentTime;
				float elapsed = (currentTime - lastTime) / 1000.0f;
				lastTime = currentTime;

				m_network->Update();

				
				m_input->Update();

				
				if (m_input->GetKeyPress(DIK_F1))
					PostQuitMessage(0);

				
				if (m_currentState != NULL)
					m_currentState->RequestViewer(&viewer);

				// Ensure the viewer is valid.
				if (viewer.viewer != NULL)
				{
					
					m_sceneManager->Update(elapsed, viewer.viewer->GetViewMatrix());

					// Set the view transformation.
					m_device->SetTransform(D3DTS_VIEW, viewer.viewer->GetViewMatrix());

					// Update the 3D sound listener.
					m_soundSystem->UpdateListener(viewer.viewer->GetForwardVector(), viewer.viewer->GetTranslation(), viewer.viewer->GetVelocity());
				}

				
				m_stateChanged = false;
				if (m_currentState != NULL)
					m_currentState->Update(elapsed);
				if (m_stateChanged == true)
					continue;

				// Begin the scene.
				m_device->Clear(0, NULL, viewer.viewClearFlags, 0, 1.0f, 0);
				if (SUCCEEDED(m_device->BeginScene()))
				{
					// Render the scene, if there is a valid viewer.
					if (viewer.viewer != NULL)
						m_sceneManager->Render(elapsed, viewer.viewer->GetTranslation());

					// Render the current state, if there is one.
					if (m_currentState != NULL)
						m_currentState->Render();

					// End the scene and present it.
					m_device->EndScene();
					m_device->Present(NULL, NULL, NULL, NULL);

					// Keep track of the index of the current back buffer.
					if (++m_currentBackBuffer == m_setup->totalBackBuffers + 1)
						m_currentBackBuffer = 0;
				}
			}
		}
	}

	// Destroy the engine.
	SAFE_DELETE(g_engine);
}

HWND Engine::GetWindow()
{
	return m_window;
}

void Engine::SetDeactiveFlag(bool deactive)
{
	m_deactive = deactive;
}

float Engine::GetScale()
{
	return m_setup->scale;
}

IDirect3DDevice9 *Engine::GetDevice()
{
	return m_device;
}

D3DDISPLAYMODE *Engine::GetDisplayMode()
{
	return &m_displayMode;
}

ID3DXSprite *Engine::GetSprite()
{
	return m_sprite;
}

void Engine::AddState(State *state, bool change)
{
	m_states->Add(state);

	if (change == false)
		return;

	if (m_currentState != NULL)
		m_currentState->Close();

	m_currentState = m_states->GetLast();
	m_currentState->Load();
}

void Engine::RemoveState(State *state)
{
	m_states->Remove(&state);
}

void Engine::ChangeState(unsigned long id)
{
	m_states->Iterate(true);
	while (m_states->Iterate() != NULL)
	{
		if (m_states->GetCurrent()->GetID() == id)
		{
			// Close the old state.
			if (m_currentState != NULL)
				m_currentState->Close();

			// Let the sound system perform garbage collection.
			m_soundSystem->GarbageCollection();

			// Set the new current state and load it.
			m_currentState = m_states->GetCurrent();
			m_currentState->Load();

			// Swap the back buffers until the first one is in the front.
			while (m_currentBackBuffer != 0)
			{
				m_device->Present(NULL, NULL, NULL, NULL);

				if (++m_currentBackBuffer == m_setup->totalBackBuffers + 1)
					m_currentBackBuffer = 0;
			}

			// Indicate that the state has changed.
			m_stateChanged = true;

			break;
		}
	}
}

State *Engine::GetCurrentState()
{
	return m_currentState;
}

ResourceManager< Script > *Engine::GetScriptManager()
{
	return m_scriptManager;
}

ResourceManager< Material > *Engine::GetMaterialManager()
{
	return m_materialManager;
}

ResourceManager< Mesh > *Engine::GetMeshManager()
{
	return m_meshManager;
}

Input *Engine::GetInput()
{
	return m_input;
}

Network *Engine::GetNetwork()
{
	return m_network;
}


SoundSystem *Engine::GetSoundSystem()
{
	return m_soundSystem;
}

SceneManager *Engine::GetSceneManager()
{
	return m_sceneManager;
}