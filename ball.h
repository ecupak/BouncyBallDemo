#pragma once

#include "keystate.h"
#include "template.h"
#include "surface.h"

namespace Tmpl8 {
	enum class Trigger // Action to trigger wall bounce.
	{
		LEFT,
		RIGHT,
	};

	enum class BounceStrength // Strength of the wall bounce.
	{
		WEAK,
		STRONG,
	};

	enum class Mode // State the ball is in.
	{
		AIR,
		GROUND,
		WALL,
		REST,
	};

	class Ball // Everyone's favorite.
	{
	public:		
		Ball();
		// Ball(Surface* screen); <-- can't do.
		void update(keyState& leftKey, keyState& rightKey);				
		void draw(Surface* screen);

	private:
		/* METHODS */

		// While in air.
		void updateAir(keyState& leftKey, keyState& rightKey);
		void updateVerticalMovement();
		void updateHorizontalMovement(keyState& leftKey, keyState& rightKey);
		void updateHorizontalMovementSpeed(keyState& leftKey, keyState& rightKey);
		void updateCollisionDetectors();
		void handleGroundCollision();
		void stopBouncing();
		void prepareForGroundMode();
		void setSquashFrameCount();
		void setStretchFrameCount();
		void handleWallCollision();
		void prepareForWallMode(float newPosX, Trigger trigger, int newFrame);

		// While on ground.
		void updateGround();
		void bounceOffGround();
		
		// While on wall.
		void updateWall(keyState& leftKey, keyState& rightKey);
		void bounceOffWall(BounceStrength bounceStrength);
		void setEjectionSpeed(bool isWeakBounce);
		void setDirectionLockFrameCount(bool isWeakBounce);
		void setEjectionSpeedY(bool isWeakBounce);
		void setEjectionSpeedX(bool isWeakBounce);
		
		// Sprite updates.
		void updateFrameStretch2Normal();
		void updateFrameDirectionLockRelease();
		void setFrameNormal2Squash();
		void setFrameSquash2Stretch();		
		void setFrameAfterWallBounce(bool isWeakBounce);


		/* ATTRIBUTES */

		// Screen reference.
		// Surface* m_screen;

		// State change tracker.
		Mode mode{ Mode::AIR };

		// Movement related.
		vec2 position{ 20.0f, 200.0f };
		vec2 hiddenPos{ 0.0f, 0.0f }; // Only y component is used; cheats the ball landing on the ground.
		vec2 speed{ 0.0f, 0.0f };
		vec2 maxSpeed{ 5.0f, 30.0f }; // x component is increased for strong wall bounce.
		float maxSpeedNormalX{ maxSpeed.x }; // Used to reset maxSpeed.x.
		vec2 acceleration{ 0.5f, 1.0f };
		vec2 direction{ 0.0f , 0.0f }; // Only x component is used.

		// Wall bounce related.
		Trigger wallBounceTrigger{}; // What direction to press to do strong bounce.
		BounceStrength wallBounceStrength{}; // Strength of wall bounce.

		// Intrinsic properties.
		float elasticity{ 1.0f }; // Anything less than 1 and ball loses speed with each bounce.
		float squashDampeningMagnitude{ 0.5f }; // Subtracts from squash value to get squash duration.
		float squashDampeningCoefficient{ 0.25f }; // Multiplies by squash value to get squash duration.
		float deadZone{ acceleration.y * 2.5f }; // When speed.y is less than this, ball stops bouncing.

		// Frame counts determine how long certain conditions last.
		int directionLockedFrameCount{ 0 };
		int squashFrameCount{ 0 };
		int stretchFrameCount{ 0 };
		int triggerFrameCount{ 0 };

		// Collision helpers.
		bool isTouchingSide{ false };
		bool isTouchingGround{ false };

		// Boundaries.
		int ground{ ScreenHeight };
		int leftWall{ 0 };
		int rightWall{ ScreenWidth };
	};




}