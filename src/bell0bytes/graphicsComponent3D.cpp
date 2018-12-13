// include the header
#include "graphicsComponent3D.h"

// bell0bytes core
#include "app.h"

// bell0bytes graphics
#include "d3d.h"

namespace graphics
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructors ///////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	GraphicsComponent3D::GraphicsComponent3D(core::DirectXApp& dxApp, const core::Window& appWindow): d3d(NULL)
	{
		// initialize Direct3D
		try { d3d = new Direct3D(dxApp, appWindow); }
		catch (std::runtime_error& e)
		{ throw e; }
	}
	GraphicsComponent3D::~GraphicsComponent3D()
	{
		if (d3d)
			delete d3d;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Scene Presentation ///////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void GraphicsComponent3D::clearBuffers()
	{
		d3d->clearBuffers();
	}
	void GraphicsComponent3D::clearBuffers(float colour[4])
	{
		d3d->clearBuffers(colour);
	}
	util::Expected<int> GraphicsComponent3D::present()
	{
		return d3d->present();
	}
	unsigned int GraphicsComponent3D::getNumberOfSupportedModes() const
	{
		return d3d->getNumberOfSupportedModes();
	}
	unsigned int GraphicsComponent3D::getCurrentModeIndex() const
	{
		return d3d->getCurrentModeIndex();
	}
	const DXGI_MODE_DESC* const GraphicsComponent3D::getSupportedModes() const
	{ return d3d->getSupportedModes(); };

}