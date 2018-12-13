// the header file
#include "depesche.h"

namespace core
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	Depesche::Depesche() : sender(nullptr), destination(nullptr), type((DepescheTypes)0), message(nullptr)
	{

	}

	Depesche::Depesche(DepescheSender& sender, DepescheDestination& destination, const DepescheTypes type, void* const msg) : sender(&sender), destination(&destination), type(type), message(msg)
	{

	}

	Depesche::~Depesche()
	{ }
}