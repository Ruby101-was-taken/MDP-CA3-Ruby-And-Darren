
#include "RoboCatPCH.hpp"

const float WORLD_HEIGHT = 2160.f;
const float WORLD_WIDTH = 3840.f;

bool PlayerCar::OnFinalLap() {
	return mCurrentLap == mLapsToWin-1;
}

PlayerCar::PlayerCar() :
	GameObject(),
	// Car physics parameters - tuned for a small, responsive go-kart feel
	mMaxRotationSpeed(180.f),	// steering
	mMaxLinearSpeed(1200.f),		// speed
	mAcceleration(500.f),		// acceleration
	mReverseAccelScale(0.5f), // brake
	mLinearDrag(4.5f),
	mGrip(20.f),

	velocity_(Vector3::Zero),
	mCurrentSteer(0.f),
	mMinSteerScale(0.85f),		// reduce steering (percentage) at top speed
	mWallRestitution(0.1f),
	mCarRestitution(0.1f),
	mThrustDir(0.f),
	mPlayerId(0),
	mIsShooting(false),
	mHealth(10),
	// checkpoint / lap defaults
	mCurrentLap(0),
	mCurrentCheckpointIndex(0),
	mLapsToWin(2),
	total_checkpoints_(0),
	mRaceFinished(false),
	star_speed_increase_(30),
	stars_(0),
	max_stars_(20) {
	// Set scale based on original sprite size
	const float originalHalfHeight = 1010.f / 2.f;
	const float originalHalfWidth = 424.f / 2.f;
	const float desiredVisualRadius = 60.f;
	const float scale = desiredVisualRadius / originalHalfHeight;
	SetScale(scale);
	const float widthBasedRadius = originalHalfWidth * scale * 0.95f;
	SetCollisionRadius(widthBasedRadius);
}


void PlayerCar::ProcessInput(float inDeltaTime, const InputState& inInputState) {
	// Turning:
	// Keep rotation very responsive (small, quick car). We rotate the car sprite immediately,
	// but velocity direction is only slowly aligned to heading (grip), producing oversteer/drift.
	float desiredHorizontal = inInputState.GetDesiredHorizontalDelta();

	// Compute speed-based steering scale (1.0 at rest, mMinSteerScale at max speed)
	float speed = velocity_.Length2D();
	float speedRatio = 0.f;
	if (mMaxLinearSpeed > 0.f)
	{
		speedRatio = speed / mMaxLinearSpeed;
		if (speedRatio > 1.f) speedRatio = 1.f;
	}
	float steerScale = 1.f - speedRatio * (1.f - mMinSteerScale); // linear lerp(1, mMinSteerScale, speedRatio)

	//mCurrentSteer = desiredHorizontal; // Immediate steering (no gradual increase) for responsiveness
	float steerLerpSpeed = 8.f;
	mCurrentSteer += (desiredHorizontal - mCurrentSteer) * steerLerpSpeed * inDeltaTime;

	// Apply rotation scaled by speed-based steering limit
	float effectiveSteer = mCurrentSteer * steerScale;
	float rotationDelta = effectiveSteer * mMaxRotationSpeed * inDeltaTime;
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
void PlayerCar::AdjustVelocityByThrust(float inDeltaTime) {
	Vector3 forwardVector = GetForwardVector(); // Vector forward
	// Apply acceleration/braking along forward vector
	if (mThrustDir != 0.f) {
		// Positive thrust accelerates forward; negative thrust brakes / reverses
		velocity_ += forwardVector * (mAcceleration * mThrustDir * inDeltaTime);
	}
	else {
		// No throttle: apply linear drag to gradually slow down
		// Use a stable multiplier rather than direct subtraction
		float dragFactor = 1.f / (1.f + mLinearDrag * inDeltaTime);
		velocity_ *= dragFactor;
	}

	// Clamp speed to max
	float speedSq = velocity_.LengthSq2D();
	float maxSpeedSq = mMaxLinearSpeed * mMaxLinearSpeed;
	if (speedSq > maxSpeedSq)
	{
		float speed = sqrtf(speedSq);
		velocity_ = velocity_ * (mMaxLinearSpeed / speed);
	}

	// Grip: reduce lateral velocity relative to forward direction to simulate tire grip.
	// Lower mGrip => less lateral friction => easier to oversteer.
	// Separate velocity into forward and lateral components
	Vector3 vel = velocity_;
	// project velocity onto forward
	float forwardSpeed = Dot2D(vel, forwardVector);
	Vector3 forwardVel = forwardVector * forwardSpeed;
	Vector3 lateralVel = vel - forwardVel;

	// Reduce lateral velocity by grip factor (1 - grip) per second
	// Keep forward velocity untouched except for drag above
	//lateralVel *= (1.f - mGrip);
	float gripFactor = std::max(0.f, 1.f - (mGrip * inDeltaTime));
	lateralVel *= gripFactor;

	velocity_ = forwardVel + lateralVel;
}

void PlayerCar::SimulateMovement(float inDeltaTime) {
	//simulate us...
	AdjustVelocityByThrust(inDeltaTime);

	MoveCar(inDeltaTime);

	ProcessCollisions();
}

// Ruby White - D00255322
void PlayerCar::MoveCar(float inDeltaTime) {
	//move on X axis
	Vector3 location = GetLocation();
	float new_x = location.mX + velocity_.mX * inDeltaTime;

	SetLocation(Vector3(new_x, location.mY, 0));

	ProcessCollisionsWithLevel(velocity_.mX, 0);

	//move on Y axis
	location = GetLocation();
	float new_y = location.mY + velocity_.mY * inDeltaTime;

	SetLocation(Vector3(location.mX, new_y, 0));

	ProcessCollisionsWithLevel(0, velocity_.mY);
}


void PlayerCar::Update() {

}

void PlayerCar::ProcessCollisions() {
	//right now just bounce off the sides..

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
			float targetRadius = target->GetCollisionRadius() * target->GetScale();

			Vector3 delta = targetLocation - sourceLocation;
			float distSq = delta.LengthSq2D();
			float collisionDist = (sourceRadius + targetRadius);
			if (distSq < (collisionDist * collisionDist))
			{
				//first, tell the other guy there was a collision with a cat, so it can do something...

				if (target->HandleCollisionWithCar(this))
				{
					//okay, you hit something!
					//so, project your location far enough that you're not colliding
					Vector3 dirToTarget = delta;
					dirToTarget.Normalize2D();
					Vector3 acceptableDeltaFromSourceToTarget = dirToTarget * collisionDist;
					//important note- we only move this cat. the other cat can take care of moving itself
					SetLocation(targetLocation - acceptableDeltaFromSourceToTarget);


					Vector3 relVel = velocity_;

					//if other object is a cat, it might have velocity, so there might be relative velocity...
					PlayerCar* target_car = target->GetAsCar();
					if (target_car)
					{
						relVel -= target_car->velocity_;

						OnCollideWithOtherCar(target_car);
					}

					//got vel with dir between objects to figure out if they're moving towards each other
					//and if so, the magnitude of the impulse ( since they're both just balls )
					float relVelDotDir = Dot2D(relVel, dirToTarget);

					if (relVelDotDir > 0.f)
					{
						Vector3 impulse = relVelDotDir * dirToTarget;

						if (target_car)
						{
							velocity_ -= impulse;
							velocity_ *= mCarRestitution;
						}
						else
						{
							velocity_ -= impulse * 2.f;
							velocity_ *= mWallRestitution;
						}

					}
				}
			}
		}
	}

}

// Ruby White - D00255322
void PlayerCar::OnCollideWithOtherCar(PlayerCar* target_car) {
}

// Ruby White - D00255322
void PlayerCar::ProcessCollisionsWithLevel(float vx, float vy) {
	Vector3 location = GetLocation();
	float x = location.mX;
	float y = location.mY;

	float radius = GetCollisionRadius();

	sf::FloatRect rect({ x - radius, y - radius }, { radius * 2.f , radius * 2.f });


	float recoil_strength = 0.1;

	////if the car collides against a wall, the quick solution is to push it off
	if (LevelManager::sInstance->IsCollidingWithWalls(rect) && vy > 0) {
		velocity_.mY = 0;
		MoveOutOfWall(Vector3(0, -1, 0));
	}
	else if (LevelManager::sInstance->IsCollidingWithWalls(rect) && vy < 0) {
		velocity_.mY = 0;
		MoveOutOfWall(Vector3(0, 1, 0));
	}

	if (LevelManager::sInstance->IsCollidingWithWalls(rect) && vx > 0) {
		velocity_.mX = 0;
		MoveOutOfWall(Vector3(-1, 0, 0));
	}
	else if (LevelManager::sInstance->IsCollidingWithWalls(rect) && vx < 0) {
		velocity_.mX = 0;
		MoveOutOfWall(Vector3(1, 0, 0));
	}

	if (LevelManager::sInstance->IsCollidingWithOffRoad(rect)) {
		velocity_ *= 0.9;
	}
}

// Ruby White - D00255322
void PlayerCar::MoveOutOfWall(Vector3 direction) {
	sf::FloatRect rect;
	do {
		Vector3 location = GetLocation();
		float x = location.mX;
		float y = location.mY;
		float radius = GetCollisionRadius();

		rect = sf::FloatRect({ x - radius, y - radius }, { radius * 2.f , radius * 2.f });

		SetLocation(location + direction);
	} while (LevelManager::sInstance->IsCollidingWithWalls(rect));
}

uint32_t PlayerCar::Write(OutputMemoryBitStream& inOutputStream, uint32_t inDirtyState) const {
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

		Vector3 velocity = velocity_;
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

	if (inDirtyState & ECRS_Speed) {
		inOutputStream.Write((bool)true);
		inOutputStream.Write(mMaxLinearSpeed);

		writtenState |= ECRS_Speed;
	}
	else {
		inOutputStream.Write((bool)false);
	}

	if (inDirtyState & ECRS_Checkpoints) {
		inOutputStream.Write((bool)true);
		inOutputStream.Write(total_checkpoints_);

		writtenState |= ECRS_Checkpoints;
	}
	else {
		inOutputStream.Write((bool)false);
	}

	return writtenState;


}
// Ruby White - D00255322 Darren Meidl - D00255479 - Handle checkpoint collision
void PlayerCar::OnCheckpointPassed(Checkpoint* inCheckpoint) {
	if (!inCheckpoint || mRaceFinished)
		return;

	int cpIndex = inCheckpoint->GetIndex();
	int expectedIndex = mCurrentCheckpointIndex + 1; // expected next index

	if (cpIndex != expectedIndex) {
		// Not the checkpoint we expect next (ignore)
		/*Logging::Log("PlayerCar", "Not the checkpoint we expect next (" + std::to_string(expectedIndex) + "/" + std::to_string(total_checkpoints_) + ")");*/
		return;
	}
	else {
		if (mCurrentCheckpointIndex == -1) {
			Logging::Log("PlayerCar", "Completed a lap");
			OnCompleteLap();
		}
		mCurrentCheckpointIndex++;
		if (mCurrentCheckpointIndex == total_checkpoints_ - 1) { //we have passed the last checkpoint
			mCurrentCheckpointIndex = -1;
		}
	}
}
// Darren Meidl - D00255479 - Reset lap and checkpoint progress
void PlayerCar::ResetRaceProgress() {
	mCurrentLap = 0;
	mCurrentCheckpointIndex = -1;
	mRaceFinished = false;
}

// Ruby White - D00255322
void PlayerCar::SetTotalCheckpoints(int in_total) {
	total_checkpoints_ = in_total;
	Logging::Log("PlayerCar", "Set checkpoints: " + std::to_string(total_checkpoints_));
}


// Ruby White - D00255322
void PlayerCar::IncreaseTopSpeed() {
	stars_++;
	if (stars_ <= max_stars_) {
		mMaxLinearSpeed += star_speed_increase_;
	}
}

void PlayerCar::OnCompleteLap() {
	mCurrentLap++;
	if (mCurrentLap > mLapsToWin - 1) {
		mRaceFinished = true;
		OnCompleteRace();
	}
}
void PlayerCar::OnCompleteRace() {
	Logging::Log("PlayerCar", "OnCompleteRace");
}

