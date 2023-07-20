// The typical way to include the PlayBuffer library in your projects  
#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

// Adding state-machine to make agent8 behave differently at different points in the game
enum Agent8State
{
	STATE_APPEAR = 0,
	STATE_HALT,
	STATE_PLAY,
	STATE_DEAD,
};

// GameState is a structure that holds variables that other functions can call from
struct GameState
{
	int score = 0;
	// adding the state-machine as a data member to GameState
	Agent8State agentState = STATE_APPEAR;
};

// Declaring the gamestate as existing
GameState gameState;

// A GameObject is a structure (or ‘struct’) provided by PlayManager for representing interactive 
// objects in a typical game.Every GameObject has a set of common properties and PlayManager
// has a range of useful functions for managing them.
// Introducing enumeration to represent the GameObject types in our game:
enum GameObjectType
{
	// PlayManager uses -1 to represent uninitiated game objects and the enumeration (enum) will auto
	// assign the next numerical value (0) to our first GameObjectType "TYPE_AGENT8".
	// All of our game objects will go here. It's to assign each one with a local numerical value.
	TYPE_NULL = -1,
	TYPE_AGENT8,
	TYPE_FAN,
	TYPE_TOOL,
	TYPE_COIN,
	TYPE_STAR,
	TYPE_LASER,
	TYPE_DESTROYED,
};

// Create a new function to handle the player controls.
// Declaring it at the start of our program bc we want to refer to them before we have ‘defined’ the code which determines how
// they actually work.We want to refer to this function in MainGameUpdate() (line 40ish) which
// appears before the function is defined later (line 50ish), so we need to make this declaration now.
// Declaring UpdateFan() and UpdateTools() here first for the same reasons.
// "Declaring" and "calling" functions
void HandlePlayerControls();
void UpdateFan();
void UpdateTools();
void UpdateCoinsAndStars();
void UpdateLasers();
void UpdateDestroyed();
void UpdateAgent8();

// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	// Resets the origin of each sprite so you hold them from their centre, they also rotate around their origins
	// and radial collisions would be detected relative to their centres too.
	Play::CentreAllSpriteOrigins();
	// Loads a large .png as the game's main bg
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	// Starts music and auto looks in the Data\\Audio directory, plays the first .mp3 file with the word 'music' in its name
	Play::StartAudioLoop("music");
	// Creates our Agent8 object using: object type, position you want it to be, its collision radius in px, 
	// and which sprite to use, it'll assign the first sprite it finds with "agent8" in the name.
	Play::CreateGameObject(TYPE_AGENT8, { 115, 0 }, 50, "agent8");
	// We want the fan to exist right at the start of the game so we're creating it here at the far right of the screen.
	// We assign it to the int variable id_fan so we can call it below
	int id_fan = Play::CreateGameObject(TYPE_FAN, { 1140,217 }, 0, "fan");
	// This time we want to start the fan moving and animating so we're using 
	// the object's id returned from Play::CreateGameObject() as a parameter to 
	// Play::GetGameObject() to get a reference to the GameObject
	// We're treating the function Play::GetGameObject() as a GameObject reference and 
	// accessing its data members from the function using .velocity and .animSpeed
	// This is because Play::GetGameObject() returns a GameObject reference and saves
	// us habing to create a new GameObject reference variable in this case.
	Play::GetGameObject(id_fan).velocity = { 0,3 };
	Play::GetGameObject(id_fan).animSpeed = 1.0f;
}

// Called by PlayBuffer every frame (60 times a second!)
// Modifications to the program to allow sprites to be previewed.
bool MainGameUpdate(float elapsedTime)
{
	// Hide this old code below, for learning purposes

	//// Calls the timer variable from the gameState structure and adds elapsedTime to it.
	//gameState.timer += elapsedTime;
	//// This func clears the display buffer with whatever colour is provided.
	//// It's like clearing the previous frame for the next frame in an animation but only animating the things that move.
	//Play::ClearDrawingBuffer(Play::cOrange);
	//
	//// DrawDebugText takes: position you want text, char* what you want the text to say, what colour text.
	//// halfing display_width and height equals the centre of the display.
	//// GetSpriteName takes an int and returns the filename of a sprite based on its assigned number starting from 0
	//// gameState.spriteId is taking the spriteId variable from the gameState structure.
	//Play::DrawDebugText({ DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 },
	//					Play::GetSpriteName(gameState.spriteId),
	//					Play::cWhite);
	//// DrawSprite takes: sprite id number, position you want sprite, number of the animation frame
	//// Play::GetMousePos() will set the position where the mouse is
	//Play::DrawSprite(gameState.spriteId,
	//				Play::GetMousePos(),
	//				gameState.timer);
	//
	//// ++ operator adds one to the spriteId variable when the spacebar is pressed
	//if (Play::KeyPressed(VK_SPACE))
	//{
	//	gameState.spriteId++;
	//}

	// Play::DrawBackground() now replaces Play::ClearDrawingBuffer(), as both completely reset the display buffer
	Play::DrawBackground();
	// HandlePlayerControls(); replaced by UpdateAgent8()
	UpdateAgent8(); // replaces HandlePlayerControls()
	// Calling our new functions that were first 'declared' at the start of the code
	UpdateFan();
	UpdateTools();
	UpdateCoinsAndStars();
	UpdateLasers();
	UpdateDestroyed();
	// Adding instructions to the screen
	Play::DrawFontText("64px", "ARROW KEYS TO MOVE UP AND DOWN AND SPACE TO FIRE",
		{ DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 30 }, Play::CENTRE);
	// updates the score based on gameState.score
	Play::DrawFontText("132px", "SCORE: " + std::to_string(gameState.score),
		{ DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);
	// This func updates the visible window with the contents of the drawing buffer
	Play::PresentDrawingBuffer();
	return Play::KeyDown(VK_ESCAPE);
}

// Defining the function ‘definition’ for HandlePlayerControls(), which implements the desired behaviour in code
void HandlePlayerControls()
{
	// retrieving a reference to the player’s GameObject using Play::GetGameObjectByType()
	// always get access your objects through the PlayManager using functions like this one, and never try store a global reference to a GameObject, as it might get destroyed by
	// another part of the code in-between frames.
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	// now we have our reference we can access any of the GameObject’s data members directly to change its behaviour.
	// A simple example of this is below: If the up arrow key is held, it sets agent8's velocity in the y axis to -4 (up)
	if (Play::KeyDown(VK_UP))
	{
		obj_agent8.velocity = { 0,-4 };
		// SetSprite() changes the object's sprite by taking: GameObject, Sprite name, its animation speed
		Play::SetSprite(obj_agent8, "agent8_climb", 0.25f);
	}
	// if the down arrow key is held, it sets agent8's acceleration to 1 so the velocity increases over time to simulate
	// gravity
	else if (Play::KeyDown(VK_DOWN))
	{
		obj_agent8.acceleration = { 0, 1 };
		Play::SetSprite(obj_agent8, "agent8_fall", 0);
	}
	else
	{
		// if agent8's y-axis velocity is greater than 5 and up or down arrow keys aren't being pressed
		// it will switch to STATE_HALT and play the sprite animation for halt
		if (obj_agent8.velocity.y > 5)
		{
			gameState.agentState = STATE_HALT;
			Play::SetSprite(obj_agent8, "agent8_halt", 0.333f);
			obj_agent8.acceleration = { 0,0 };
		}
		// This condition controls what happens when neither key is being pressed
		// We slow the velocity by multiplying by a fraction (*=) and set the acceleration to 0
		else
		{
			Play::SetSprite(obj_agent8, "agent8_hang", 0.02f);
			obj_agent8.velocity *= 0.5f;
			obj_agent8.acceleration = { 0, 0 };
		}
	}
	// Shooting lasers if spacebar is pressed (once per key press vs continously)
	if (Play::KeyPressed(VK_SPACE))
	{
		// creating a vector2D called firePos to store a position from where the laser should shoot from
		// The end of agent8's gun is 155 px to the right and 75 px up from their origin
		Vector2D firePos = obj_agent8.pos + Vector2D(155, -75);
		// creating the obj_laser as TYPE_LASER, at firePos, 30 hitradius, sprite filename laser
		int id = Play::CreateGameObject(TYPE_LASER, firePos, 30, "laser");
		// using GetGameObject() to set its velocity
		Play::GetGameObject(id).velocity = { 32,0 };
		Play::PlayAudio("shoot");

	}
}

// Creating skeleton functions for our enemy updates (tools and fan)
void UpdateFan()
{
	GameObject& obj_fan = Play::GetGameObjectByType(TYPE_FAN);
	// We want fan object's behaviour to move up and down the screen, randomly spawning tool objects.
	// Using Play::RandomRoll() func, so if 50 is rolled on a 50 sided die, the condition happens.
	if (Play::RandomRoll(50) == 50)
	{
		// assigning int id as creating the tool obj at the fan obj position, collision radius 50, 
		// using whichever sprite that has "driver" in the name.
		int id = Play::CreateGameObject(TYPE_TOOL, obj_fan.pos, 50, "driver");
		// assigning obj_tool as a game object using GetGameObject which uses the variable: id
		GameObject& obj_tool = Play::GetGameObject(id);
		// making the obj_tool's velocity -8 on the x-axis (moves it to the left)
		// and a random number between -1 and 1 multiplied by 6 as its velocity on the y-axis.
		// (makes it bounce up and down randomly)
		obj_tool.velocity = Point2f(-8, Play::RandomRollRange(-1, 1) * 6);

		// every 1 in 2 rolls the object becomes a spanner instead of a screwdriver
		if (Play::RandomRoll(2) == 1)
		{
			// setting sprite: which obj to affect, what sprite to use, animation speed
			Play::SetSprite(obj_tool, "spanner", 0);
			// changing the obj_tool's data members
			obj_tool.radius = 100;
			// setting its x-axis velocity to -4
			obj_tool.velocity.x = -4;
			obj_tool.rotSpeed = 0.1f;
		}
		// On creation of gameobject, play spawning sound effect.
		Play::PlayAudio("tool");
	}
	// Spawning a coin from the fan's position once every 150 rolls
	if (Play::RandomRoll(150) == 1)
	{
		// Same as with spawning obj_tools, int id is assigned to create game obj
		int id = Play::CreateGameObject(TYPE_COIN, obj_fan.pos, 40, "coin");
		// Game object obj_coin is created and its data members set
		GameObject& obj_coin = Play::GetGameObject(id);
		obj_coin.velocity = { -3,0 };
		obj_coin.rotSpeed = 0.1f;

	}
	// We call Play::UpdateGameObject() next to process the object’s movement
	Play::UpdateGameObject(obj_fan);

	// Once its position has been updated, we can check to see 
	// if the fan is leaving the display area.
	if (Play::IsLeavingDisplayArea(obj_fan))
	{
		// resetting its position
		obj_fan.pos = obj_fan.oldPos;
		// reversing its y-axis velocity		
		obj_fan.velocity.y *= -1;
	}

	// We don't want to ever rotate our fan so we use Play::DrawObject() 
	// rather than Play::DrawObjectRotated()
	Play::DrawObject(obj_fan);
}

void UpdateTools()
{
	// We're getting a reference to Agent8 as Tools will need to detect
	// collisions with the player, but as there's more than one tool, we handle them
	// slightly differently
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	// (pg 24.) A std::vector is a kind of container 
	// which can store sequences of other data types.
	// We declare the vTools vector in UpdateTools() to 
	// create a local vector of integer values in which to store the ids of all the tool objects.
	// The function Play::CollectGameObjectIDsByType() finds all the ids of objects with 
	// the given type and returns them in a vector of integers, copying them over to our local vector.
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL);

	// Using a std::vector, we can create a for loop which goes through each item in the vector
	// in turn and assigns their unique id to the local id variable.
	// That id can then be used to retrieve a reference to the corresponding 
	// GameObject so that each object can be updated in turn.
	for (int id : vTools)
	{
		GameObject& obj_tool = Play::GetGameObject(id);

		// checks if two objects are within each others collision radii
		// the agentState is not in STATE_DEAD AND the two objects are colliding
		if (gameState.agentState != STATE_DEAD && Play::IsColliding(obj_tool, obj_agent8))
		{
			// Stops the music
			Play::StopAudioLoop("music");
			// Plays the die sound effect
			Play::PlayAudio("die");
			// Changes the agentState to STATE_DEAD
			gameState.agentState = STATE_DEAD;
			// Puts agent 8's object off screen (saves processing)
			// obj_agent8.pos = { -100,-100 };
		}
		// We call Play::UpdateGameObject() to process the object’s movement
		Play::UpdateGameObject(obj_tool);

		// Checking if our tool obj is leaving the display area off the vertical plane
		if (Play::IsLeavingDisplayArea(obj_tool, Play::VERTICAL))
		{
			// reset its position
			obj_tool.pos = obj_tool.oldPos;
			// reverse its y-axis velocity
			obj_tool.velocity.y *= -1;
		}
		// drawing our tool object
		Play::DrawObjectRotated(obj_tool);

		// if our tool object is not visible (!Play::IsVisible() means the reverse)
		if (!Play::IsVisible(obj_tool))
			// destroy the game object using the id variable we assigned earlier within this function
			Play::DestroyGameObject(id);
	}

}

// Creating a function for UpdateCoinsAndStars() to program coins and stars object behavior
void UpdateCoinsAndStars()
{
	// Setting obj_agent8 game object in this function by using GetGameObjectByType()
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	// Again assigning vCoins with the IDs of all the game objects that match the type: TYPE_COIN
	// *click moment* in C++ you put the data type in front of the variable 
	// so std::vector<int> is just the data type you want vCoins to be. It's a sort of 2d array in python!
	// GameObject& in front of obj_agent8 is acting the same way!
	// So this is just making an array with all the TYPE_COIN's object IDs
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(TYPE_COIN);

	// this is saying each created game object in vCoins... 
	// is separately refered to as id_coin so each object can be dealt with in turn
	// rather than only being able to bulk edit all obj_coins
	for (int id_coin : vCoins)
	{
		// so one id_coin is assigned the local obj_coin properties, from its own ID
		GameObject& obj_coin = Play::GetGameObject(id_coin);
		// assigning hasCollided as false
		bool hasCollided = false;

		// if coin and agent8 collide...
		if (Play::IsColliding(obj_coin, obj_agent8))
		{
			// for loop that starts at 0.25 radians and adds 0.5 rads until it's completed a full circle (2 radians)
			// This produces 4 stars in diagonally opposite corners from the coin.
			for (float rad{ 0.25f }; rad < 2.0f; rad += 0.5f)
			{
				// assign id with created star game object info
				int id = Play::CreateGameObject(TYPE_STAR, obj_agent8.pos, 0, "star");
				// assign obj_star with id info
				GameObject& obj_star = Play::GetGameObject(id);
				// set star's rotation speed and acceleration
				obj_star.rotSpeed = 0.1f;
				obj_star.acceleration = { 0.0f,0.5f };
				// Sets the velocity of the object based on a target rotation angle
				Play::SetGameObjectDirection(obj_star, 16, rad * PLAY_PI);
			}
			// Changes hasCollided variable to true, adds 500 points to the score, plays collect sound effect.
			hasCollided = true;
			gameState.score += 500;
			Play::PlayAudio("collect");
		}

		// updates our game object and draws it (rotated)
		Play::UpdateGameObject(obj_coin);
		Play::DrawObjectRotated(obj_coin);

		// if obj_coin is NOT visible OR (||) if hasCollided is true...
		if (!Play::IsVisible(obj_coin) || hasCollided)
			// destroys that particular coin object rather than every coin object
			Play::DestroyGameObject(id_coin);

		/*(pg 29.) Instead of getting rid of coins as soon as we detect a collision with the player, we are using a
		hasCollided variable to record that this has happened, and then destroying it later
		when we test to see if it is no longer visible. We’ve done this because calling DestroyGameObject()
		at the wrong time can create unintended problems. Imagine we destroyed the object inside the if
		statement that detects the collision. That would mean that obj_coin would nolonger be a valid object
		reference when it is accessed again later and the game would crash! So – in general – if you are going to
		destroy an object then you need to do it at the last point in the code where you are working on that object.
		So long as it is destroyed last then nothing should accidently use it again afterwards.*/
	}

	// creating a vector called vStars and putting all the TYPE_STAR object ids in it
	std::vector<int> vStars = Play::CollectGameObjectIDsByType(TYPE_STAR);

	// for each id_star in vStars...
	for (int id_star : vStars)
	{
		// creat a gameobject called obj_star
		GameObject& obj_star = Play::GetGameObject(id_star);

		// update and draw all obj_stars
		Play::UpdateGameObject(obj_star);
		Play::DrawObjectRotated(obj_star);

		// if the star isn't visible anymore, destroy that particular star
		if (!Play::IsVisible(obj_star))
		{
			Play::DestroyGameObject(id_star);
		}
	}


}

// Defining the UpdateLasers() function
void UpdateLasers()
{
	// Creating a vector for each object that interacts with lasers
	// using CollectGameOnjectIDsByType() as it returns the std::vector<int> data type from the TYPE id
	std::vector<int> vLasers = Play::CollectGameObjectIDsByType(TYPE_LASER);
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL);
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(TYPE_COIN);

	// coding the laser interactions
	for (int id_laser : vLasers)
	{
		// creates obj_laser from id_laser using Play::GetGameObject()
		GameObject& obj_laser = Play::GetGameObject(id_laser);
		// sets its hasCollided variable to false
		bool hasCollided = false;

		for (int id_tool : vTools)
		{
			GameObject& obj_tool = Play::GetGameObject(id_tool);
			if (Play::IsColliding(obj_laser, obj_tool))
			{
				// changes the hasCollided variable to true
				hasCollided = true;
				// changes the obj type to TYPE_DESTROYED
				obj_tool.type = TYPE_DESTROYED;
				gameState.score += 100;
			}
		}

		for (int id_coin : vCoins)
		{
			GameObject& obj_coin = Play::GetGameObject(id_coin);
			if (Play::IsColliding(obj_laser, obj_coin))
			{
				hasCollided = true;
				obj_coin.type = TYPE_DESTROYED;
				Play::PlayAudio("error");
				gameState.score -= 300;
			}

		}

		// if the score is less than zero, make the score zero
		if (gameState.score < 0)
		{
			gameState.score = 0;
		}

		Play::UpdateGameObject(obj_laser);
		Play::DrawObject(obj_laser);

		// Destroy that particular id_laser object if it's not visible or hasCollided
		if (!Play::IsVisible(obj_laser) || hasCollided) // I put a semi-colon here and it caused the laser to appear and disapear immediately
		{
			Play::DestroyGameObject(id_laser);
		}
	}
}

// Creating the UpdateDestroyed() function to slowly fade away destroyed objects and not have their collisions anymore
void UpdateDestroyed()
{
	std::vector<int> vDead = Play::CollectGameObjectIDsByType(TYPE_DESTROYED);

	for (int id_dead : vDead)
	{
		GameObject& obj_dead = Play::GetGameObject(id_dead);
		obj_dead.animSpeed = 0.2f;
		Play::UpdateGameObject(obj_dead);

		// % modulus gives us the remainder when the first value is divided by the second
		// so the sprite will only get drawn (made transparent) on odd numbered frames
		// as 0 is treated as false and any value greater than 0 as true.
		if (obj_dead.frame % 2)
			// making the object more and more transparent per frame
			Play::DrawObjectRotated(obj_dead, (10 - obj_dead.frame) / 10.0f);

		// if the dead object isn't visible OR if the dead object has been around for more than 10 frames
		// destroy the particular object
		if (!Play::IsVisible(obj_dead) || obj_dead.frame >= 10)
			Play::DestroyGameObject(id_dead);
	}
}

void UpdateAgent8()
{
	// creating obj_agent8 from object type
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);

	// switch is a statement to clearly separate out the states and their different behaviours
	// tidier than set of if statements, but remember to finish each case with a break statement
	switch (gameState.agentState)
	{
	case STATE_APPEAR:
		obj_agent8.velocity = { 0,12 };
		obj_agent8.acceleration = { 0,0.5f };
		Play::SetSprite(obj_agent8, "agent8_fall", 0);
		obj_agent8.rotation = 0;
		if (obj_agent8.pos.y >= DISPLAY_HEIGHT / 3)
			gameState.agentState = STATE_PLAY;
		break;

	case STATE_HALT:
		obj_agent8.velocity *= 0.9f;
		if (Play::IsAnimationComplete(obj_agent8))
			gameState.agentState = STATE_PLAY;
		break;

	case STATE_PLAY:
		HandlePlayerControls();
		break;

	case STATE_DEAD:
		obj_agent8.acceleration = { -0.3f,0.5f };
		obj_agent8.rotation += 0.25f;
		if (Play::KeyPressed(VK_SPACE) == true)
		{
			gameState.agentState = STATE_APPEAR;
			obj_agent8.pos = { 115,0 };
			obj_agent8.velocity = { 0,0 };
			obj_agent8.frame = 0;
			Play::StartAudioLoop("music");
			gameState.score = 0;

			for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_TOOL))
				Play::GetGameObject(id_obj).type = TYPE_DESTROYED;
		}
		break;
	} // End of switch on Agent8State

	// After setting up the GameObject properties, we call Play::UpdateGameObject()
	// to get PlayManager to apply their effects to the object
	Play::UpdateGameObject(obj_agent8);

	// This checks if the object is leaving the display area and moves it back to its previous
	// position if it is leaving
	// .pos and .oldpos are GameObject properties
	if (Play::IsLeavingDisplayArea(obj_agent8) && gameState.agentState != STATE_DEAD)
	{
		obj_agent8.pos = obj_agent8.oldPos;
	}

	// Draws a vertical white line from the top of the display above the player’s position to the 
	// centre of the player’s sprite
	Play::DrawLine({ obj_agent8.pos.x, 0 }, obj_agent8.pos, Play::cWhite);

	// Using the Play::DrawObjectRotated() function rather than the faster Play::DrawObject() function 
	// as we want to rotate the player’s sprite later on
	Play::DrawObjectRotated(obj_agent8);
}

// The MainGameExit() function for PlayManager
int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}

