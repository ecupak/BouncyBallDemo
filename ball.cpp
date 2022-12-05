#pragma once

#include "ball.h"

namespace Tmpl8
{
	// Would prefer to have within constructor.
	Sprite ball(new Surface("assets/squishyBall.png"), 15);


	// Default constructor to avoid error that none exists.
	Ball::Ball() {}

	
	// The important constructor.
	Ball::Ball(Surface* screen) :
		m_screen{ screen }
	{
	}


	void Ball::update(keyState& leftKey, keyState& rightKey)
	{
		/* Update position and draw sprite. */

		if (mode == Mode::AIR)
		{
			Ball::updateAir(leftKey, rightKey);
		}
		else if (mode == Mode::GROUND)
		{
			Ball::updateGround();
		}
		else if (mode == Mode::WALL)
		{
			Ball::updateWall(leftKey, rightKey);
		}

		Ball::internalDraw();
	}


	/*
		This section covers actions while in the air (or about to collide).
	*/


	void Ball::updateAir(keyState& leftKey, keyState& rightKey)
	{
		/* Hande movement and collision checking. */

		Ball::updateVerticalMovement();
		Ball::updateHorizontalMovement(leftKey, rightKey);

		Ball::updateCollisionDetectors();
		if (isTouchingGround)
		{
			Ball::handleGroundCollision();
		}
		else if (isTouchingSide)
		{
			Ball::handleWallCollision();
		}
	}


	void Ball::updateVerticalMovement()
	{
		/* Check if sprite image can be changed to normal. Stretched image is
			immediate result of squashed/GROUND state. Update vertical movement
			and speed.y, keeping within bounds. */

		Ball::updateFrameStretch2Normal();

		position.y += speed.y;
		speed.y += acceleration.y;
		speed.y = Clamp(speed.y, -maxSpeed.y, maxSpeed.y);
	}


	void Ball::updateHorizontalMovement(keyState& leftKey, keyState& rightKey)
	{
		/* Update horizontal position, unless direction locked from weak wall bounce. */

		position.x += speed.x;

		if (directionLockedFrameCount <= 0)
		{
			Ball::updateHorizontalMovementSpeed(leftKey, rightKey);
		}
		else
		{
			directionLockedFrameCount--;
			Ball::updateFrameDirectionLockRelease();
		}
	}


	void Ball::updateHorizontalMovementSpeed(keyState& leftKey, keyState& rightKey)
	{
		/* Get direction using bool-to-int conversion. Update speed.x as long as either
			left or right is pressed. Otherwise, lose all speed (landing precision &
			accuracy are important). */

		direction.x = -leftKey.isActive + rightKey.isActive;
		if (direction.x != 0)
		{
			speed.x += direction.x * acceleration.x;
			speed.x = Clamp(speed.x, -maxSpeed.x, maxSpeed.x);
		}
		else
		{
			speed.x = 0;
		}
	}


	void Ball::updateCollisionDetectors()
	{
		/* Check if ball is touching ground or wall. */

		isTouchingGround = (position.y + ball.GetHeight() >= ground);

		isTouchingSide = (position.x < leftWall ||
			position.x + ball.GetWidth() > rightWall);
	}


	void Ball::handleGroundCollision()
	{
		/* Cheat the y position so the ball can appear to be above the ground.
			Determine if ball will be squashed or come to a complete rest. */

		hiddenPos.y = position.y;
		position.y = ground - ball.GetHeight();

		// Set next mode.
		if (deadZone > fabsf(speed.y))
		{
			Ball::stopBouncing();
		}
		else
		{
			Ball::prepareForGroundMode();
		}
	}


	void Ball::stopBouncing()
	{
		/* Lose all vertical speed and set to normal image. REST mode is a dead end. */

		speed.y = 0.0f;
		ball.SetFrame(0);
		mode = Mode::REST;
	}


	void Ball::prepareForGroundMode()
	{
		/* Remove any directional lock since player can't move ball while it is
			squashed anyway. Calculate the squash and stretch frame counts. Slower
			speeds result in less frames of each. GROUND mode squashes the ball. */

		directionLockedFrameCount = 0;

		Ball::setSquashFrameCount();
		Ball::setStretchFrameCount();

		Ball::setFrameNormal2Squash();

		// Set next mode.
		mode = Mode::GROUND;
	}


	void Ball::setSquashFrameCount()
	{
		/* Squash delay / animation is a function of impact speed. All in an effort
			to reduce / remove squashing when at low speeds. Around 4 speed there
			should be no squashing at all, but kept it generalized just in case. */

		// Easier to step through.
		float squashValue = fabsf(speed.y);
		squashValue -= squashDampeningMagnitude;
		squashValue *= squashDampeningCoefficient;
		squashValue = ceil(squashValue) - 1;

		squashFrameCount = static_cast<int>(squashValue);
	}


	void Ball::setStretchFrameCount()
	{
		/* Stretch animation is a function of squash delay. Must be calculated
			before the squash frames begin counting down. */

		stretchFrameCount = (squashFrameCount - 1) * 2;
	}


	void Ball::handleWallCollision()
	{
		/* Based on wall, prepare for WALL mode. Frame int is based on 3 images per
			orientation and style of ball. */

		if (position.x < leftWall)
		{
			Ball::prepareForWallMode(leftWall, Trigger::RIGHT, 1 + (3 * 4));
		}
		else
		{
			Ball::prepareForWallMode(rightWall - ball.GetWidth(),
				Trigger::LEFT, 1 + (3 * 2));
		}
	}


	void Ball::prepareForWallMode(float newPosX, Trigger trigger, int newFrame)
	{
		/* Reposition ball just off of wall and lose all speed. Register trigger and
			set trigger duration. Update the sprite. WALL mode sticks ball on wall. */

		position.x = newPosX;
		speed = vec2(0.0f, 0.0f);

		wallBounceTrigger = trigger;
		triggerFrameCount = 20;

		ball.SetFrame(newFrame);

		// Set next mode.
		mode = Mode::WALL;
	}


	/*
		This section covers when the ball hits the ground and bounces back up.
	*/


	void Ball::updateGround()
	{
		/* "Freeze" ball in squash mode for squash frame count. Then bounce back up
			with equal speed to landing speed. Reset maxSpeed.x to normal speed (is
			increased after a strong wall bounce). AIR mode handles movement and
			collisions. */

		if (--squashFrameCount <= 0)
		{
			Ball::bounceOffGround();
			Ball::setFrameSquash2Stretch();

			maxSpeed.x = maxSpeedNormalX;

			// Set next mode.
			mode = Mode::AIR;
		}
	}


	void Ball::bounceOffGround()
	{
		/* Restore true position and send ball upwards. If speed.y is not at max,
			undo the speed incease from previous acceleration (applied after movement,
			but not yet ~used~). Otherwise, at 100% elasticity, the ball will bounce
			higher than it started. */

		position.y = hiddenPos.y;

		// Undo unused speed.
		if (fabsf(speed.y) < maxSpeed.y)
		{
			speed.y -= acceleration.y;
		}

		// Reverse speed (bounce!).
		speed.y *= -elasticity;

		// Go up.
		position.y += speed.y;
		speed.y += acceleration.y;
	}


	/*
		This section covers when the ball hits / clings to a wall and bounces off.
	*/


	void Ball::updateWall(keyState& leftKey, keyState& rightKey)
	{
		/* If trigger window closes with no action, wall bounces weakly off wall.
			Otherwise it has more powerful bounce. */

		if (--triggerFrameCount <= 0)
		{
			wallBounceStrength = BounceStrength::WEAK;
			Ball::bounceOffWall(BounceStrength::WEAK);
		}
		else
		{
			if ((wallBounceTrigger == Trigger::LEFT && leftKey.isActive) ||
				(wallBounceTrigger == Trigger::RIGHT && rightKey.isActive))
			{
				wallBounceStrength = BounceStrength::STRONG;
				Ball::bounceOffWall(BounceStrength::STRONG);
			}
		}
	}


	void Ball::bounceOffWall(BounceStrength bounceStrength)
	{
		/* Set speed and if any direction lock, as well as next sprite. AIR mode
			handles movement and collision. */			

		bool isWeakBounce = (bounceStrength == BounceStrength::WEAK);

		Ball::setEjectionSpeed(isWeakBounce);
		Ball::setDirectionLockFrameCount(isWeakBounce);
		Ball::setFrameAfterWallBounce(isWeakBounce);

		// Set sprite and mode.
		
		mode = Mode::AIR;
	}
	

	void Ball::setEjectionSpeed(bool isWeakBounce)
	{
		/* Update speeds from wall bounce. */

		Ball::setEjectionSpeedY(isWeakBounce);
		Ball::setEjectionSpeedX(isWeakBounce);
	}


	void Ball::setEjectionSpeedY(bool isWeakBounce)
	{
		/* Wall bounce always has "up" vertical speed. */

		speed.y = -maxSpeed.y * (isWeakBounce ? 0.3f : 0.7f);
	}


	void Ball::setEjectionSpeedX(bool isWeakBounce)
	{
		/* Horizontal speed needs direction away from wall. Temporarily increase
			max horizontal speed from a strong wall bounce. Will be reset on squash. */

		maxSpeed.x = (isWeakBounce ? maxSpeedNormalX : maxSpeedNormalX * 1.5f);

		speed.x = maxSpeed.x * (wallBounceTrigger == Trigger::LEFT ? -1 : 1);
	}

	
	void Ball::setDirectionLockFrameCount(bool isWeakBounce)
	{
		/* Weak bounce locks direction for a while. Prevents player from clinging
			against same wall and climbing. */

		if (isWeakBounce)
		{
			directionLockedFrameCount = 10;
		}
	}


	/*
		This section draws the sprite.
	*/


	void Ball::draw(Surface* screen)
	{
		/* Would prefer to get the screen* in constructor and store in class. */
		
		ball.Draw(screen, position.x, position.y);
	}
	

	void Ball::internalDraw()
	{
		ball.Draw(m_screen, position.x, position.y);
	}


	/*
		This section deals with changing the sprite image / frame.	
	*/


	void Ball::updateFrameStretch2Normal()
	{
		/* Countdown until switching from stretched image to normal image. Must
			also not be direction locked from a weak wall bounce. */

		if (stretchFrameCount <= 0 && directionLockedFrameCount <= 0)
		{
			ball.SetFrame(0);
		}
		else
		{
			stretchFrameCount--;
		}
	}


	void Ball::updateFrameDirectionLockRelease()
	{
		/* Switch to normal image once direction lock has ended. Used to signal to
			player when control is restored. Must also not be in stretch mode from 
			a ground bounce (or else stretch image is ended too early). */

		if (directionLockedFrameCount <= 0 && stretchFrameCount <= 0)
		{
			ball.SetFrame(0);
		}
	}


	void Ball::setFrameNormal2Squash()
	{
		/* If speed was fast enough to generate a frame of squash, change image. */
		
		if (squashFrameCount > 0)
		{
			ball.SetFrame(1);
		}
	}


	void Ball::setFrameSquash2Stretch()
	{
		// Set mode to stretch.
		if (stretchFrameCount > 0)
		{
			ball.SetFrame(2);
		}
	}


	void Ball::setFrameAfterWallBounce(bool isWeakBounce)
	{
		/* Weak bounce sprite is unfilled until control is given back to player. */

		ball.SetFrame(isWeakBounce ? 3 : 0);
	}
}