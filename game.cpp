#include "game.h"
#include "surface.h"
#include <cstdio> //printf

#define not !

namespace Tmpl8
{	
	// -----------------------------------------------------------
	// Initialize the application
	// -----------------------------------------------------------
	void Game::Init()
	{
		// this->ball = new Ball(screen); <-- want to do this, but get message that Game::Game(void) references a deleted function.
	}

	// -----------------------------------------------------------
	// Close down application
	// -----------------------------------------------------------
	void Game::Shutdown()
	{
	}

	// -----------------------------------------------------------
	// Main application input detection.
	// -----------------------------------------------------------
	
	/* Register key press only if the key was not already being pressed
		(must register key release first). */

	void Game::KeyDown(int key)
	{
		switch (key)
		{
		case 80: // LEFT.
			if (not leftKey.isPressed)
			{
				leftKey.isActive = true;
				leftKey.isPressed = true;
				rightKey.isActive = false;
			}
			break;
		case 79: // RIGHT.
			if (not rightKey.isPressed)
			{
				rightKey.isActive = true;
				rightKey.isPressed = true;
				leftKey.isActive = false;
			}
			break;
		}
	}

	void Game::KeyUp(int key)
	{
		switch (key)
		{
		case 80: // LEFT.
			leftKey.isActive = false;
			leftKey.isPressed = false;
			break;
		case 79: // RIGHT.
			rightKey.isActive = false;
			rightKey.isPressed = false;
			break;
		}
	}

	// -----------------------------------------------------------
	// Main application tick function
	// -----------------------------------------------------------

	void Game::Tick(float deltaTime)
	{
		// Clear screen.
		screen->Clear(0);

		// Update and draw ball position.
		ball.update(leftKey, rightKey);
		ball.draw(screen); // <-- want to put in class Ball.

		screen->Print("Left/Right arrow keys to move.", 20, 20, 0xffffff);
		screen->Print("When stuck to a wall, quickly press the other direction to do a strong wall bounce.", 20, 40, 0xffffff);
		screen->Print("Otherwise you just fall off.", 20, 60, 0xffffff);
	}
};