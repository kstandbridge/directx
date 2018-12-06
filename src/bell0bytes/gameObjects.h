#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		29/10/2017 - Dortmund, Germany
*
* Desc:		Game Objects
*
* History:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// DEFINITIONS //////////////////////////////////////////////////////////////////////////
class DirectXGame;

namespace world
{
	class Player
	{
	private:
		float x, y;
		int weaponPower;

	public:
		void fireWeapon() { weaponPower = 0; };
		void chargeWeapon() { weaponPower += 1; }
		void moveLeft(float dt) { x -= dt; };
		void moveRight(float dt) { x += dt; };

		Player() { x = 0; y = 0; weaponPower = 0; };
		~Player() {};

		friend class DirectXGame;
	};
}