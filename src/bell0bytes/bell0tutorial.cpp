/****************************************************************************************
* Author:	Gilles Bellot
* Date:		29/06/2017 - Dortmund - Germany
*
* Desc:		Basic Windows Programming Tutorial
*
* History:	- 29/06/2017 - Hello World!
*			- 06/07/2017 - Expected!
*			- 07/07/2017 - A Thread-Safe Logger
*			- 08/07/2017 - Of the Moon and the Sun
*			- 08/07/2017 - Handling Important Events
*			- 09/07/2017 - Keeping Track of Time
*			- 10/07/2017 - The Game Loop
*			- 13/07/2017 - Keyboard and Mouse
*			- 13/07/2017 - Of Icons and Cursors
*			- 14/07/2017 - A First Framework
*			- 15/07/2017 - First Contact
*			- 25/07/2017 - The Swap Chain
*			- 31/07/2017 - Render Targets
*			- 05/08/2017 - Printing Text with DirectWrite
*			- 27/08/2017 - Of Shaders and Triangles
*			- 01/09/2017 - Among Colourful Stars
*			- 29/10/2017 - Going Fullscreen
*			- 20/05/2018 - Shader Effects
*			- 21/05/2018 - Introduction to Drawing with Direct2D
*			- 21/05/2018 - Fun with Brushes
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// windows includes
#include <windows.h>

// exceptions
#include <exception>
#include <stdexcept>

// bell0bytes core
#include "app.h"

// bell0ybtes util
#include "expected.h"							// error handling with "expected"
#include "serviceLocator.h"						// enables global access to services

// bell0bytes graphics
#include "graphicsHelper.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////

// CLASSES //////////////////////////////////////////////////////////////////////////////

// the core game class, derived from DirectXApp
class DirectXGame : core::DirectXApp
{
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

public:
	// constructor and destructor
	DirectXGame(HINSTANCE hInstance);
	~DirectXGame();

	// override virtual functions
	util::Expected<void> init() override;								// game initialization
	void shutdown(util::Expected<void>* expected = NULL) override;		// cleans up and shuts the game down (handles errors)
	util::Expected<int> update(double dt);								// update the game world
	util::Expected<int> render(double farSeer);							// render the scene

	// create graphics
	util::Expected<void> initGraphics();								// initializes graphics

	// run the game
	util::Expected<int> run() override;
};

// FUNCTIONS ////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// WinMain //////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	// create and initialize the game
	DirectXGame game(hInstance);
	util::Expected<void> gameInitialization = game.init();

	// if the initialization was successful, run the game, else, try to clean up and exit the application
	if (gameInitialization.wasSuccessful())
	{
		// initialization was successful -> run the game
		util::Expected<int> returnValue = game.run();

		// clean up after the game has ended
		util::Expected<void> convertedReturnValue(returnValue);
		game.shutdown(&convertedReturnValue);

		// gracefully return
		if (returnValue.isValid())
			return returnValue.get();
		else
			return -1;
	}
	else
	{
		// a critical error occured during initialization, try to clean up and to print information about the error
		game.shutdown(&gameInitialization);
		
		// humbly return with an error
		return -1;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Game Initialization //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// constructor and destructor
DirectXGame::DirectXGame(HINSTANCE hInstance) : DirectXApp(hInstance)
{ }
DirectXGame::~DirectXGame()
{ }

// initialize the game
util::Expected<void> DirectXGame::init()
{
	// initialize the core DirectX application
	util::Expected<void> applicationInitialization = DirectXApp::init();
	if (!applicationInitialization.wasSuccessful())
		return applicationInitialization;

	// initialize game graphics
	applicationInitialization = initGraphics();
	if(!applicationInitialization.wasSuccessful())
		return applicationInitialization;

	// log and return success
	util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("Game initialization was successful.");
	return {};
}

// initialize graphics
util::Expected<void> DirectXGame::initGraphics()
{
	// create a triangle
	graphics::VERTEX triangleVertices[] = { { 0.0f, 0.1f, 0.3f, 1.0f, 1.0f, 0.0f },
											{ 0.11f, -0.1f, 0.3f, 1.0f, 0.0f, 0.0f },
											{ -0.11f, -0.1f, 0.3f, 0.0f, 1.0f, 0.0f } };

	// set up buffer description
	D3D11_BUFFER_DESC bd;
	bd.ByteWidth = sizeof(graphics::VERTEX) * ARRAYSIZE(triangleVertices);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	// define subresource data
	D3D11_SUBRESOURCE_DATA srd = { triangleVertices, 0,0 };

	// create the vertex buffer
	if (FAILED(d3d->dev->CreateBuffer(&bd, &srd, &vertexBuffer)))
		return "Critical Error: Unable to create vertex buffer!";

	// return success
	return {};
}

// run the game
util::Expected<int> DirectXGame::run()
{
	// run the core DirectX application
	return DirectXApp::run();
}

// shutdown
void DirectXGame::shutdown(util::Expected<void>* expected)
{
	// check for error message
	if (expected != NULL && !expected->isValid())
	{
		// the game was shutdown by an error
		// try to clean up and log the error message
		try
		{
			// do clean up

			// throw error
			expected->get();
		}
		catch (std::runtime_error& e)
		{
			// create and print error message string (if the logger is available)
			if (DirectXApp::fileLoggerIsActive())
			{
				std::stringstream errorMessage;
				errorMessage << "The game is shutting down with a critical error: " << e.what();
				util::ServiceLocator::getFileLogger()->print<util::SeverityType::error>(std::stringstream(errorMessage.str()));
			}
			return;
		}
	}

	// no error: clean up and shut down normally
	util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The game was shut down successfully.");
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Update ////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
util::Expected<int> DirectXGame::update(double /*deltaTime*/)
{	
	// update the game world

	// return success
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Render ////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
util::Expected<int> DirectXGame::render(double /*farSeer*/)
{
	// clear the back buffer and the depth/stencil buffer
	d3d->clearBuffers();
	
	// print FPS information
	if (!d2d->printFPS().wasSuccessful())
		return std::runtime_error("Failed to print FPS information!");

	// update the constant buffers

	// set the vertex buffer
	unsigned int stride = sizeof(graphics::VERTEX);
	unsigned int offset = 0;
	d3d->devCon->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

	// set primitive topology
	d3d->devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// draw 3 vertices, starting from vertex 0
	d3d->devCon->Draw(3, 0);

	// present the scene
	if (!d3d->present().wasSuccessful())
		return std::runtime_error("Failed to present the scene!");

	// return success
	return 0;
}