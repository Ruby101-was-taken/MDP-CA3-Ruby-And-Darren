#include "RoboCatPCH.hpp"

const float WORLD_HEIGHT = 720.f;
const float WORLD_WIDTH = 1280.f;

RoboCat::RoboCat() :
	GameObject(),
	// tuned for small, quick car-like behavior
	mMaxRotationSpeed(300.f),	// stronger turning
	mMaxLinearSpeed(1200.f),		// top speed
	mAcceleration(1000.f),		// acceleration
	mReverseAccelScale(0.4f), // reverse is weaker than forward
	mLinearDrag(1.8f),			// drag when coasting
	mGrip(0.08f),				// low grip -> easy to oversteer
	mVelocity(Vector3::Zero),
	mWallRestitution(0.1f),
	mCatRestitution(0.1f),
	mThrustDir(0.f),
	mPlayerId(0),
	mIsShooting(false),
	mHealth(10)
{
	// Set scale based on original sprite size
	const float originalHalfHeight = 1010.f / 2.f;
	const float originalHalfWidth = 424.f / 2.f;
	const float desiredVisualRadius = 60.f;
	const float scale = desiredVisualRadius / originalHalfHeight;
	SetScale(scale);
	const float widthBasedRadius = originalHalfWidth * scale * 0.95f;
	SetCollisionRadius(widthBasedRadius);
}

void RoboCat::ProcessInput(float inDeltaTime, const InputState& inInputState)
{
	// Turning:
	// Keep rotation very responsive (small, quick car). We rotate the car sprite immediately,
	// but velocity direction is only slowly aligned to heading (grip), producing oversteer/drift.
	float desiredHorizontal = inInputState.GetDesiredHorizontalDelta();

	// Apply rotation. Rotation responsiveness is independent of throttle here to make it twitchy.
	float rotationDelta = desiredHorizontal * mMaxRotationSpeed * inDeltaTime;
	float newRotation = GetRotation() + rotationDelta;
	SetRotation(newRotation);

	// Movement/throttle input (-1 .. 1)
	float inputForwardDelta = inInputState.GetDesiredVerticalDelta();	
	if (inputForwardDelta >= 0.f)
		mThrustDir = inputForwardDelta;
	else
		mThrustDir = inputForwardDelta * mReverseAccelScale; // Reduce reverse power

	mIsShooting = inInputState.IsShooting();
}
// Darren Meidl - D00255479 - Apply acceleration, drag, and grip to velocity based on current throttle and heading
void RoboCat::AdjustVelocityByThrust(float inDeltaTime)
{
	Vector3 forwardVector = GetForwardVector(); // Vector forward
	// Apply acceleration/braking along forward vector
	if (mThrustDir != 0.f) {
		// Positive thrust accelerates forward; negative thrust brakes / reverses
		mVelocity += forwardVector * (mAcceleration * mThrustDir * inDeltaTime);
	}
	else {
		// No throttle: apply linear drag to gradually slow down
		// Use a stable multiplier rather than direct subtraction
		float dragFactor = 1.f / (1.f + mLinearDrag * inDeltaTime);
		mVelocity *= dragFactor;
	}

	// Clamp speed to max
	float speedSq = mVelocity.LengthSq2D();
	float maxSpeedSq = mMaxLinearSpeed * mMaxLinearSpeed;
	if (speedSq > maxSpeedSq)
	{
		float speed = sqrtf(speedSq);
		mVelocity = mVelocity * (mMaxLinearSpeed / speed);
	}

	// Grip: reduce lateral velocity relative to forward direction to simulate tire grip.
	// Lower mGrip => less lateral friction => easier to oversteer.
	// Separate velocity into forward and lateral components
	Vector3 vel = mVelocity;
	// project velocity onto forward
	float forwardSpeed = Dot2D(vel, forwardVector);
	Vector3 forwardVel = forwardVector * forwardSpeed;
	Vector3 lateralVel = vel - forwardVel;

	// Reduce lateral velocity by grip factor (1 - grip) per second
	// Keep forward velocity untouched except for drag above
	lateralVel *= (1.f - mGrip);

	mVelocity = forwardVel + lateralVel;
}

void RoboCat::SimulateMovement(float inDeltaTime)
{
	//simulate us...
	AdjustVelocityByThrust(inDeltaTime);
    
	SetLocation(GetLocation() + mVelocity * inDeltaTime);

	ProcessCollisions();
}

void RoboCat::Update()
{

}

void RoboCat::ProcessCollisions()
{
	//right now just bounce off the sides..
	ProcessCollisionsWithScreenWalls();

	float sourceRadius = GetCollisionRadius();
	Vector3 sourceLocation = GetLocation();

	//now let's iterate through the world and see what we hit...
	//note: since there's a small number of objects in our game, this is fine.
	//but in a real game, brute-force checking collisions against every other object is not efficient.
	//it would be preferable to use a quad tree or some other structure to minimize the
	//number of collisions that need to be tested.
	for (auto goIt = World::sInstance->GetGameObjects().begin(), end = World::sInstance->GetGameObjects().end(); goIt != end; ++goIt)
	{
		GameObject* target = goIt->get();
		if (target != this && !target->DoesWantToDie())
		{
			//simple collision test for spheres- are the radii summed less than the distance?
			Vector3 targetLocation = target->GetLocation();
			float targetRadius = target->GetCollisionRadius();

			Vector3 delta = targetLocation - sourceLocation;
			float distSq = delta.LengthSq2D();
			float collisionDist = (sourceRadius + targetRadius);
			if (distSq < (collisionDist * collisionDist))
			{
				//first, tell the other guy there was a collision with a cat, so it can do something...

				if (target->HandleCollisionWithCat(this))
				{
					//okay, you hit something!
					//so, project your location far enough that you're not colliding
					Vector3 dirToTarget = delta;
					dirToTarget.Normalize2D();
					Vector3 acceptableDeltaFromSourceToTarget = dirToTarget * collisionDist;
					//important note- we only move this cat. the other cat can take care of moving itself
					SetLocation(targetLocation - acceptableDeltaFromSourceToTarget);


					Vector3 relVel = mVelocity;

					//if other object is a cat, it might have velocity, so there might be relative velocity...
					RoboCat* targetCat = target->GetAsCat();
					if (targetCat)
					{
						relVel -= targetCat->mVelocity;
					}

					//got vel with dir between objects to figure out if they're moving towards each other
					//and if so, the magnitude of the impulse ( since they're both just balls )
					float relVelDotDir = Dot2D(relVel, dirToTarget);

					if (relVelDotDir > 0.f)
					{
						Vector3 impulse = relVelDotDir * dirToTarget;

						if (targetCat)
						{
							mVelocity -= impulse;
							mVelocity *= mCatRestitution;
						}
						else
						{
							mVelocity -= impulse * 2.f;
							mVelocity *= mWallRestitution;
						}

					}
				}
			}
		}
	}

}

void RoboCat::ProcessCollisionsWithScreenWalls()
{
	Vector3 location = GetLocation();
	float x = location.mX;
	float y = location.mY;

	float vx = mVelocity.mX;
	float vy = mVelocity.mY;

	float radius = GetCollisionRadius();

	//if the cat collides against a wall, the quick solution is to push it off
	if ((y + radius) >= WORLD_HEIGHT && vy > 0)
	{
		mVelocity.mY = -vy * mWallRestitution;
		location.mY = WORLD_HEIGHT - radius;
		SetLocation(location);
	}
	else if (y - radius <= 0 && vy < 0)
	{
		mVelocity.mY = -vy * mWallRestitution;
		location.mY = radius;
		SetLocation(location);
	}

	if ((x + radius) >= WORLD_WIDTH && vx > 0)
	{
		mVelocity.mX = -vx * mWallRestitution;
		location.mX = WORLD_WIDTH - radius;
		SetLocation(location);
	}
	else if (x - radius <= 0 && vx < 0)
	{
		mVelocity.mX = -vx * mWallRestitution;
		location.mX = radius;
		SetLocation(location);
	}
}

uint32_t RoboCat::Write(OutputMemoryBitStream& inOutputStream, uint32_t inDirtyState) const
{
	uint32_t writtenState = 0;

	if (inDirtyState & ECRS_PlayerId)
	{
		inOutputStream.Write((bool)true);
		inOutputStream.Write(GetPlayerId());

		writtenState |= ECRS_PlayerId;
	}
	else
	{
		inOutputStream.Write((bool)false);
	}


	if (inDirtyState & ECRS_Pose)
	{
		inOutputStream.Write((bool)true);

		Vector3 velocity = mVelocity;
		inOutputStream.Write(velocity.mX);
		inOutputStream.Write(velocity.mY);

		Vector3 location = GetLocation();
		inOutputStream.Write(location.mX);
		inOutputStream.Write(location.mY);

		inOutputStream.Write(GetRotation());

		writtenState |= ECRS_Pose;
	}
	else
	{
		inOutputStream.Write((bool)false);
	}

	//always write mThrustDir- it's just two bits
	if (mThrustDir != 0.f)
	{
		inOutputStream.Write(true);
		inOutputStream.Write(mThrustDir > 0.f);
	}
	else
	{
		inOutputStream.Write(false);
	}

	if (inDirtyState & ECRS_Color)
	{
		inOutputStream.Write((bool)true);
		inOutputStream.Write(GetColor());

		writtenState |= ECRS_Color;
	}
	else
	{
		inOutputStream.Write((bool)false);
	}

	if (inDirtyState & ECRS_Health)
	{
		inOutputStream.Write((bool)true);
		inOutputStream.Write(mHealth, 4);

		writtenState |= ECRS_Health;
	}
	else
	{
		inOutputStream.Write((bool)false);
	}

	return writtenState;


}



