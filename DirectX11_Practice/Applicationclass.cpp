////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_MultiTextureShader = 0;
	m_Model = 0;
}


ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}


ApplicationClass::~ApplicationClass()
{
}

// The model initialization now takes
// in the filename of the model file it is loading.
// In this tutorial we will use the cube.txt file
// so this model loads in a 3D cube object for rendering.
bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	char modelFilename[128], textureFilename1[128], textureFilename2[128];
	bool result;


	// Create and initialize the Direct3D object.
	m_Direct3D = new D3DClass;

	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	// Create and initialize the camera object.
	m_Camera = new CameraClass;

	m_Camera->SetPosition(0.0f, 0.0f, -5.0f);
	m_Camera->Render();

	// Create and initialize the multitexture shader object.
	m_MultiTextureShader = new MultiTextureShaderClass;

	result = m_MultiTextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the multitexture shader object.", L"Error", MB_OK);
		return false;
	}

	// Set the file name of the model.
	strcpy_s(modelFilename, "../Resource/square.txt");

	// Set the file name of the textures.
	strcpy_s(textureFilename1, "../Resource/stone01.tga");
	strcpy_s(textureFilename2, "../Resource/dirt01.tga");

	// Create and initialize the model object.
	m_Model = new ModelClass;

	result = m_Model->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), modelFilename, textureFilename1, textureFilename2);
	if (!result)
	{
		return false;
	}

	return true;
}


void ApplicationClass::Shutdown()
{
	// Release the model object.
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}

	// Release the multitexture shader object.
	if (m_MultiTextureShader)
	{
		m_MultiTextureShader->Shutdown();
		delete m_MultiTextureShader;
		m_MultiTextureShader = 0;
	}

	// Release the camera object.
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Release the Direct3D object.
	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}

	return;
}


bool ApplicationClass::Frame(InputClass* Input)
{
	int mouseX, mouseY;
	bool result, mouseDown;
	//We now check for the escape key press in this function instead of the SystemClass.

	// Check if the user pressed escape and wants to exit the application.
	if (Input->IsEscapePressed())
	{
		return false;
	}
	//Each frame we will now get the mouse location and button status from the Input object and then update the mouse strings.

	// Get the location of the mouse from the input object,
	Input->GetMouseLocation(mouseX, mouseY);

	// Check if the mouse has been pressed.
	mouseDown = Input->IsMousePressed();

	// Update the mouse strings each frame.
	result = UpdateMouseStrings(mouseX, mouseY, mouseDown);
	if (!result)
	{
		return false;
	}

	// Render the graphics scene.
	result = Render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool ApplicationClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	// Clear the buffers to begin the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// Render the model using the multitexture shader.
	m_Model->Render(m_Direct3D->GetDeviceContext());

	result = m_MultiTextureShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_Model->GetTexture(0), m_Model->GetTexture(1));

	if(!result)
	{
		return false;
	}

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}

//The new UpdateMouseStrings function will update the three mouse strings each frame.

bool ApplicationClass::UpdateMouseStrings(int mouseX, int mouseY, bool mouseDown)
{
	//char tempString[16], finalString[32];
	//bool result;


	//// Convert the mouse X integer to string format.
	//sprintf_s(tempString, "%d", mouseX);

	//// Setup the mouse X string.
	//strcpy_s(finalString, "Mouse X: ");
	//strcat_s(finalString, tempString);

	//// Update the sentence vertex buffer with the new string information.
	//result = m_MouseStrings[0].UpdateText(m_Direct3D->GetDeviceContext(), m_Font, finalString, 10, 10, 1.0f, 1.0f, 1.0f);
	//if (!result)
	//{
	//	return false;
	//}

	//// Convert the mouse Y integer to string format.
	//sprintf_s(tempString, "%d", mouseY);

	//// Setup the mouse Y string.
	//strcpy_s(finalString, "Mouse Y: ");
	//strcat_s(finalString, tempString);

	//// Update the sentence vertex buffer with the new string information.
	//result = m_MouseStrings[1].UpdateText(m_Direct3D->GetDeviceContext(), m_Font, finalString, 10, 35, 1.0f, 1.0f, 1.0f);
	//if (!result)
	//{
	//	return false;
	//}

	//// Setup the mouse button string.
	//if (mouseDown)
	//{
	//	strcpy_s(finalString, "Mouse Button: Yes");
	//}
	//else
	//{
	//	strcpy_s(finalString, "Mouse Button: No");
	//}

	//// Update the sentence vertex buffer with the new string information.
	//result = m_MouseStrings[2].UpdateText(m_Direct3D->GetDeviceContext(), m_Font, finalString, 10, 60, 1.0f, 1.0f, 1.0f);
	//if (!result)
	//{
	//	return false;
	//}

	return true;
}