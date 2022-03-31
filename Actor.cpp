#include "Actor.h"
#include "StudentWorld.h"

using namespace std;

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// ACTOR
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Actor::Actor(StudentWorld* wrld, int IMID, int xpos_start, int ypos_start, Direction dir, float size, unsigned int depth, bool penetrates)
	:GraphObject(IMID, xpos_start, ypos_start, dir, size, depth)
{
	setVisible(true);
	m_wrld = wrld;
	m_alive = true;
	cannotPenetrate = penetrates;
}

Actor::~Actor()
{ 

}

bool Actor::isAlive()
{
	return m_alive;
}

void Actor::setDead() 
{
	m_alive = false;
}

bool Actor::cannotPenetrateThisObj()
{
	return cannotPenetrate;
}

StudentWorld* Actor::myW()
{
	return m_wrld;
}

void Actor::takeGold() 
{

}

bool Actor::getIsGoingAwayFromOilField() 
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// EARTH
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Earth::Earth(StudentWorld* wrld, int xpos, int ypos)
	: Actor(wrld, TID_EARTH, xpos, ypos, right, .25, 3)
{ 

}

Earth::~Earth()
{ 

}

void Earth::doSomething() // dirt doesn't do anything
{
	return;
}

void Earth::annoy(int hit) // dirt can't be annoyed
{
	return;
}

int Earth::getIMID()
{
	return TID_EARTH;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// TUNNELMAN
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Tunnelman::Tunnelman(StudentWorld* wrld)
	:Actor(wrld, TID_PLAYER, 30, 60, right, 1.0, 0)
{
	m_hlth = 10;
	m_squirts = 5;
	m_sonar = 1;
	m_gold = 0;
}

Tunnelman::~Tunnelman()
{

}

void Tunnelman::doSomething()
{
	// do nothing if not alive
	if (isAlive() == false)
		return;

	// remove Earth
	for (int x = getX(); x < getX() + 4; x++)
		for (int y = getY(); y < getY() + 4; y++)
			if (myW()->isEarthHere(x, y))
			{
				myW()->removeEarth(x, y);
				myW()->playSound(SOUND_DIG);
				myW()->updatePatch(getX(), getY());
			}

	int k;
	if (myW()->getKey(k))
	{
		switch (k)
		{
		case KEY_PRESS_RIGHT:
			if (getDirection() != right)
				setDirection(right);
			else if (!myW()->boulderPresent(getX() + 1, getY()))
			{
				if (getX() <= 59)
					moveTo(getX() + 1, getY());
			}
			break;
		case KEY_PRESS_LEFT:
			if (getDirection() != left)
				setDirection(left);
			else if (!myW()->boulderPresent(getX() - 1, getY()))
			{
				if (getX() >= 1)
					moveTo(getX() - 1, getY());
			}
			break;
		case KEY_PRESS_UP:
			if (getDirection() != up)
				setDirection(up);
			else if (!myW()->boulderPresent(getX(), getY() + 1))
			{
				if (getY() <= 59)
					moveTo(getX(), getY() + 1);
			}
			break;
		case KEY_PRESS_DOWN:
			if (getDirection() != down)
				setDirection(down);
			else if (!myW()->boulderPresent(getX(), getY() - 1))
			{
				if (getY() >= 1)
					moveTo(getX(), getY() - 1);
			}
			break;
		case KEY_PRESS_SPACE:
			if (getSquirts() > 0)
			{
				fireSquirt();
				myW()->initSquirt(getX(), getY(), getDirection());
			}
			break;
		case KEY_PRESS_TAB:
			if (getGold() > 0)
			{
				giveGold();
				myW()->leftGoldNugget(getX(), getY());
			}
			break;
		case 'z':
			if (getSonarKits() > 0)
			{
				useSonarKit();
				myW()->revealNearbyObjects(getX(), getY());
				myW()->playSound(SOUND_SONAR);
			}
			break;
		case 'Z':
			if (getSonarKits() > 0)
			{
				useSonarKit();
				myW()->revealNearbyObjects(getX(), getY());
				myW()->playSound(SOUND_SONAR);
			}
			break;
		case KEY_PRESS_ESCAPE:
			setDead();
			break;
		}
	}
}

void Tunnelman::annoy(int hit)
{
	m_hlth = m_hlth - hit;
	
	if (m_hlth <= 0)
		setDead();
	return;
}

int Tunnelman::getIMID()
{
	return TID_PLAYER;
}

void Tunnelman::plusGold()
{
	m_gold++;
}

void Tunnelman::giveGold()
{
	m_gold--;
}

void Tunnelman::plusSonarKit()
{
	m_sonar++;
}

void Tunnelman::useSonarKit()
{
	m_sonar--;
}

void Tunnelman::plusWaterPool()
{
	m_squirts += 5;
}

void Tunnelman::fireSquirt()
{
	m_squirts--;
}

int Tunnelman::getSquirts()
{
	return m_squirts;
}

int Tunnelman::getSonarKits()
{
	return m_sonar;
}

int Tunnelman::getGold()
{
	return m_gold;
}

int Tunnelman::getHP()
{
	return m_hlth;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// BOULDER
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Boulder::Boulder(StudentWorld* wrld, int xpos, int ypos)
	: Actor(wrld, TID_BOULDER, xpos, ypos, down, 1, 1, true)
{
	for (int x = xpos; x < xpos + 4; x++)
		for (int y = ypos; y < ypos + 4; y++)
			if (myW()->isEarthHere(x, y))
				myW()->removeEarth(x, y);

	m_stable = 0; // S, W, F states
	m_ticks = 0;
}

Boulder::~Boulder()
{ 

}

void Boulder::doSomething()
{
	if (isAlive() == false)
		return;

	if (m_stable == 0) // stable
	{
		bool earthUnder = false;

		for (int i = 0; i < 4; i++)
			if (myW()->isEarthHere(getX() + i, getY() - 1))
				earthUnder = true;

		if (!earthUnder)
			m_stable = 1;
	}
	else if (m_stable == 1) // wait
	{
		m_ticks++;
		if (m_ticks >= 30)
		{
			m_stable = 2;
			for (int i = 0; i < 4; i++)
				myW()->updatePatch(getX(), getY() - i);
			myW()->playSound(SOUND_FALLING_ROCK); // ONLY PLAY ONCE!!
		}
	}
	else if (m_stable == 2) // fall
	{
		myW()->annoyTunnelman(getX(), getY(), 100, 3.0);
		myW()->annoyProtesters(getX(), getY(), 100, 3.0);

		if (myW()->isEmptyPatch(getX(), getY() - 1)) 
			moveTo(getX(), getY() - 1);
		else
			setDead();
	}
}

void Boulder::annoy(int hit)
{
	return;
}

int Boulder::getIMID()
{
	return TID_BOULDER;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// OIL
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Oil::Oil(StudentWorld* wrld, int xpos, int ypos)
	:Actor(wrld, TID_BARREL, xpos, ypos, right, 1.0, 2)
{
	setVisible(false); 
}

Oil::~Oil()
{ 

}

void Oil::Oil::doSomething()
{
	if (!isAlive())
		return;

	double rad = myW()->radius(getX(), getY(), myW()->pointerToTunnelman()->getX(), myW()->pointerToTunnelman()->getY());
	
	if (rad <= 3.)
	{
		myW()->playSound(SOUND_FOUND_OIL);
		myW()->increaseScore(1000);
		myW()->minusOil();
		setDead();
	}

	if (rad <= 4.) 
		setVisible(true);
}

void Oil::annoy(int hit)
{ 
	return;
}

int Oil::getIMID()
{
	return TID_BARREL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// GOLD NUGGET
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Gold::Gold(StudentWorld* wrld, int xpos, int ypos, bool initial_state)
	:Actor(wrld, TID_GOLD, xpos, ypos, right, 1.0, 2)
{
	m_picked_up = initial_state;
	
	if (initial_state)
		setVisible(false);
	else
		m_time = 100;
}

Gold::~Gold()
{

}

void Gold::doSomething()
{
	if (isAlive() == false)
		return;

	if (m_picked_up)
	{
		double rad = myW()->radius(getX(), getY(), myW()->pointerToTunnelman()->getX(), myW()->pointerToTunnelman()->getY());
		if (rad <= 3.) 
		{
			myW()->increaseScore(10);
			myW()->playSound(SOUND_GOT_GOODIE);
			myW()->pointerToTunnelman()->plusGold();
			setDead();
		}
		if (rad <= 4.) // discoverable
			setVisible(true);
	}
	else 
	{
		m_time--;
		
		if (myW()->tookGoldNugget(getX(), getY()))
			setDead(); 

		if (m_time <= 0)
			setDead();
	}
}

bool Gold::canProtestTakeGold() 
{
	return !m_picked_up;
}

void Gold::annoy(int hit) 
{ 
	return;
}

int Gold::getIMID()
{
	return TID_GOLD;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// POWER UPS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PowerUp::PowerUp(StudentWorld* wrld, int IMID, int xpos, int ypos)
	:Actor(wrld, IMID, xpos, ypos, right, 1.0, 2)
{

}

PowerUp::~PowerUp()
{

}

void PowerUp::annoy(int hit)
{ 
	return;
}

void PowerUp::setTickCount(int ticker)
{
	m_ticks = ticker;
}

void PowerUp::decrementTicks()
{
	m_ticks--;
}

int PowerUp::getTickCount()
{
	return m_ticks;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// SONAR KITS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sonar::Sonar(StudentWorld* wrld)
	:PowerUp(wrld, TID_SONAR, 0, 60)
{

}

Sonar::~Sonar()
{ 

}

void Sonar::doSomething()
{
	if (isAlive() == false)
		return;

	double rad = myW()->radius(getX(), getY(), myW()->pointerToTunnelman()->getX(), myW()->pointerToTunnelman()->getY());
	
	if (rad <= 3.)
	{
		myW()->playSound(SOUND_GOT_GOODIE);
		myW()->increaseScore(75);
		myW()->pointerToTunnelman()->plusSonarKit();
		setDead();
	}

	decrementTicks();

	if (getTickCount() <= 0) 
		setDead();
}

int Sonar::getIMID()
{
	return TID_SONAR;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// WATERPOOL
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

WaterPool::WaterPool(StudentWorld* wrld, int xpos, int ypos)
	:PowerUp(wrld, TID_WATER_POOL, xpos, ypos)
{ 

}

WaterPool::~WaterPool()
{

}

void WaterPool::doSomething()
{
	if (isAlive() == false)
		return;

	double rad = myW()->radius(getX(), getY(), myW()->pointerToTunnelman()->getX(), myW()->pointerToTunnelman()->getY());
	
	if (rad <= 3.) 
	{
		myW()->playSound(SOUND_GOT_GOODIE);
		myW()->increaseScore(100);
		myW()->pointerToTunnelman()->plusWaterPool();
		setDead();
	}

	decrementTicks();
	if (getTickCount() == 0)
		setDead();

}

int WaterPool::getIMID()
{
	return TID_WATER_POOL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// SQUIRT
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Squirt::Squirt(StudentWorld* wrld, int xpos, int ypos, Direction dir)
	: Actor(wrld, TID_WATER_SPURT, xpos, ypos, dir, 1, 1)
{
	m_length = 4;
}

Squirt::~Squirt()
{ }


void Squirt::doSomething()
{
	if (isAlive() == false)
		return;

	if (myW()->annoyProtesters(getX(), getY(), 2, 3.0))
	{
		m_length = 0;
		setDead();
		return;
	}

	if (getDirection() == right)
		moveTo(getX() + 1, getY());
	if (getDirection() == left)
		moveTo(getX() - 1, getY());
	if (getDirection() == up)
		moveTo(getX(), getY() + 1);
	if (getDirection() == down)
		moveTo(getX(), getY() - 1);

	m_length--;

	if (m_length == 0)
		setDead();
}

void Squirt::annoy(int hit)
{

}

int Squirt::getIMID()
{
	return TID_WATER_SPURT;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// REGULAR PROTESTOR
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RegularProtester::RegularProtester(StudentWorld* wrld, int IMID, int hp)
	: Actor(wrld, IMID, 60, 60, left, 1, 0, true)
{
	m_hlth = hp;
	
	m_leaving = false;
	m_rest = false;
	m_stunned = false;
	m_just_yelled = false;
	
	m_rest_cntr = 0;
	m_just_yelled_cntr = 0;
	m_intersection_cntr = 0;
	m_stunned_cntr = 0;
	m_num_steps_cur = rand() % 53 + 8; // 8 to 60 steps
}

RegularProtester::~RegularProtester()
{ 

}

void RegularProtester::doSomething()
{
	if (isAlive() == false)
		return;

	if (!m_leaving)
	{
		if (isCurrentlyStunned() || isAtRest())
			return;

		else // not resting
		{
			activateRestTimer(); 

			if (hasJustShouted() || yell()) 
				return;

			else if (moveTowardsTunnelman()) 
				return;

			else
				takeRegMove(); 
		}
	}

	else // leaving earth
	{
		if (isAtRest()) 
			return;

		if (getX() == 60 && getY() == 60)
		{
			setDead();
			return;
		}

		else
		{
			takeMoveToLeave(); 
			return;
		}
	}
}

bool RegularProtester::isAtRest()
{
	if (m_rest == true) 
	{
		m_rest_cntr++;
		if (m_rest_cntr > m_nextmove_cntr) 
		{
			m_rest = false;
			m_rest_cntr = 0;
		}
	}

	return m_rest; 
}

bool RegularProtester::isCurrentlyStunned()
{
	if (m_stunned == true)
	{
		m_stunned_cntr++;
		if (m_stunned_cntr > m_nextstunned_cntr) 
		{
			m_stunned = false;
			m_stunned_cntr = 0;
		}
	}
	return m_stunned; 
}

bool RegularProtester::hasJustShouted()
{
	if (m_just_yelled == true) 
	{
		m_just_yelled_cntr++;
		if (m_just_yelled_cntr >= 15) 
			m_just_yelled = false;
	}
	return m_just_yelled; 
}

bool RegularProtester::yell()
{
	int p_x = myW()->pointerToTunnelman()->getX();
	int p_y = myW()->pointerToTunnelman()->getY();

	if (myW()->radius(getX(), getY(), p_x, p_y) <= 4.0) 
	{
		bool facing = false;

		if (getDirection() == right)
		{
			if (getX() <= p_x && p_y - 4 <= getY() && getY() <= p_y + 4)
				facing = true;
		}

		if (getDirection() == left)
		{
			if (getX() >= p_x && p_y - 4 <= getY() && getY() <= p_y + 4)
				facing = true;
		}
		
		if (getDirection() == up)
		{
			if (getY() <= p_y && p_x - 4 <= getX() && getX() <= p_x + 4)
				facing = true;
		}

		if (getDirection() == down)
		{
			if (getY() >= p_y && p_x - 4 <= getX() && getX() <= p_x + 4)
				facing = true;
		}

		if (facing) 
		{
			if (m_just_yelled == false)
			{
				myW()->playSound(SOUND_PROTESTER_YELL); 
				myW()->annoyTunnelman(getX(), getY(), 2, 4.0);
				m_just_yelled = true;
				m_just_yelled_cntr = 0;
			}
			else
			{
				m_just_yelled_cntr++; 
				if (m_just_yelled_cntr >= 15)
					m_just_yelled = false;
			}
			return true; 
		}
	}

	return false;
}

bool RegularProtester::moveTowardsTunnelman()
{
	int p_x = myW()->pointerToTunnelman()->getX();
	int p_y = myW()->pointerToTunnelman()->getY();

	if (myW()->directSight(getX(), getY())) 
	{
		
		if (getY() == p_y && getX() < p_x)
		{
			setDirAtIntersection(right);
			moveTo(getX() + 1, getY());
		}
		else if (getY() == p_y && getX() > p_x) 
		{
			setDirAtIntersection(left);
			moveTo(getX() - 1, getY());
		}
		else if (getX() == p_x && getY() < p_y) 
		{
			setDirAtIntersection(up);
			moveTo(getX(), getY() + 1);
		}
		else if (getX() == p_x && getY() > p_y) 
		{
			setDirAtIntersection(down);
			moveTo(getX(), getY() - 1);
		}
		
		m_num_steps_cur = 0;
		return true; 
	}

	return false;
}

void RegularProtester::takeRegMove()
{
	if (m_num_steps_cur < 1)
	{
		bool canMove = false;
		Direction d = left;
		
		while (canMove == false)
		{
			int n = rand() % 4;

			if (n == 0)
				d = up;
			if (n == 1)
				d = down;
			if (n == 2)
				d = left;
			if (n == 3)
				d = right;

			canMove = !(myW()->blockedPath(getX(), getY(), d));
		}

		setDirAtIntersection(d);
	}

	if (m_intersection_cntr >= 200) 
	{
		string d = myW()->possibleTurns(getX(), getY(), getDirection()); 
				
		if (d == "ri")
			setDirAtIntersection(right);
		if (d == "le")
			setDirAtIntersection(left);
		if (d == "up")
			setDirAtIntersection(up);
		if (d == "do")
			setDirAtIntersection(down);
		if (d == "rile")
		{
			if (rand() % 2 == 0)
				setDirAtIntersection(right);
			else
				setDirAtIntersection(left);
		}
		if (d == "updo")
		{
			if (rand() % 2 == 0)
				setDirAtIntersection(up);
			else
				setDirAtIntersection(down);
		}
	}

	if (m_num_steps_cur > 0)
	{
		if (getDirection() == right)
			moveTo(getX() + 1, getY());
		if (getDirection() == left)
			moveTo(getX() - 1, getY());
		if (getDirection() == up)
			moveTo(getX(), getY() + 1);
		if (getDirection() == down)
			moveTo(getX(), getY() - 1);

		m_num_steps_cur--;

		if (myW()->blockedPath(getX(), getY(), getDirection()))
			m_num_steps_cur = 0;
	}
}


void RegularProtester::takeMoveToLeave()
{
	m_rest = true; 

	Direction d = myW()->protestorLeavesInThisDirection(getX(), getY());
	setDirAtIntersection(d);

	if (d == right)
	{
		moveTo(getX() + 1, getY());
		return;
	}
	if (d == left)
	{
		moveTo(getX() - 1, getY());
		return;
	}
	if (d == up)
	{
		moveTo(getX(), getY() + 1);
		return;
	}	
	if (d == down)
	{
		moveTo(getX(), getY() - 1);
		return;
	}	
}

void RegularProtester::activateRestTimer()
{
	m_rest = true; 
	m_intersection_cntr++;
}

void RegularProtester::takeGold()
{
	initiateGoingAwayFromOilField();
	myW()->increaseScore(25);
	myW()->playSound(SOUND_PROTESTER_FOUND_GOLD);
}

void RegularProtester::annoy(int hit)
{
	if (m_leaving == false)
	{
		m_hlth -= hit;

		if (m_hlth > 0) 
		{
			myW()->playSound(SOUND_PROTESTER_ANNOYED);
			getStunned();
		}
		
		if (m_hlth <= 0)
		{
			myW()->playSound(SOUND_PROTESTER_GIVE_UP);
			m_leaving = true; 
		}
		
		if (m_leaving == true)
		{
			if (hit == 100) 
				myW()->increaseScore(500);
			else
				myW()->increaseScore(100);
		}
	}
}

void RegularProtester::setDead()
{
	Actor::setDead(); 
	myW()->minusProtesters();
}

void RegularProtester::setDirAtIntersection(GraphObject::Direction d) 
{
	if (getDirection() == up || getDirection() == down) 
	{
		if (d == left || d == right)
			m_intersection_cntr = 0;
	}
	
	else if (getDirection() == right || getDirection() == left)
	{
		if (d == up || d == down)
			m_intersection_cntr = 0;
	}

	m_num_steps_cur = rand() % 53 + 8; // 8 to 60
	setDirection(d);
}

int RegularProtester::getIMID()
{
	return TID_PROTESTER;
}

void RegularProtester::getStunned()
{
	m_stunned = true;
}

void RegularProtester::setTicksToWaitBetweenMoves(int time)
{
	m_nextmove_cntr = time;
}

void RegularProtester::setStunTime(int time)
{
	m_nextstunned_cntr = time;
}

int RegularProtester::getHP()
{
	return m_hlth;
}

void RegularProtester::takeHit(int hit)
{
	m_hlth -= hit;
}

bool RegularProtester::getIsGoingAwayFromOilField()
{
	return m_leaving;
}

void RegularProtester::initiateGoingAwayFromOilField()
{
	m_leaving = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// HARDCORE PROTESTOR
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HardcoreProtester::HardcoreProtester(StudentWorld* wrld, int stare, int range)
	: RegularProtester(wrld, TID_HARD_CORE_PROTESTER, 20)
{
	m_stare = stare; 
	m_range = range; 

	m_stare_cntr = 0;
	m_is_staring = false;
}

HardcoreProtester::~HardcoreProtester()
{ }

void HardcoreProtester::doSomething()
{
	if (isAlive() == false || staringAtGold())
		return;

	if (getIsGoingAwayFromOilField() == false)
	{
		if (isCurrentlyStunned() || isAtRest()) 
			return;

		else 
		{
			activateRestTimer();

			if (hasJustShouted() || yell()) 
				return;

			else if (rangeStep()) 
				return;

			else if (moveTowardsTunnelman())
				return;

			else
				takeRegMove(); 
		}
	}

	else 
	{
		if (isAtRest())
			return;

		if (getX() == 60 && getY() == 60) 
		{
			setDead();
			return;
		}

		takeMoveToLeave();
	}
}

void HardcoreProtester::takeGold()
{
	myW()->increaseScore(50);
	myW()->playSound(SOUND_PROTESTER_FOUND_GOLD);
	m_is_staring = true;
}

void HardcoreProtester::annoy(int hit)
{
	if (!getIsGoingAwayFromOilField()) 
	{
		takeHit(hit); 
		
		if (getHP() > 0) 
		{
			myW()->playSound(SOUND_PROTESTER_ANNOYED);
			getStunned();
		}
		if (getHP() < 1) 
		{
			myW()->playSound(SOUND_PROTESTER_GIVE_UP);
			initiateGoingAwayFromOilField();
		}
		if (getIsGoingAwayFromOilField()) 
		{
			if (hit == 100) 
				myW()->increaseScore(500);
			else
				myW()->increaseScore(250);
		}
	}
}

bool HardcoreProtester::rangeStep()
{
	if (myW()->distanceFromTunnelman(getX(), getY()) <= m_range)
	{
		Direction d = myW()->findPlayerDirection(getX(), getY()); 
		setDirection(d);

		if (d == right)
		{
			moveTo(getX() + 1, getY());
			return true;
		}
		if (d == left)
		{
			moveTo(getX() - 1, getY());
			return true;
		}
		if (d == up)
		{
			moveTo(getX(), getY() + 1);
			return true;
		}
		if (d == down)
		{
			moveTo(getX(), getY() - 1);
			return true;
		}
	}

	return false; 
}

bool HardcoreProtester::staringAtGold()
{
	if (m_is_staring) 
	{
		m_stare_cntr++;

		if (m_stare_cntr > m_stare)
		{
			m_is_staring = false;
			m_stare_cntr = 0;
		}
	}
	return m_is_staring; 
}

int HardcoreProtester::getRange()
{
	return m_range;
}

int HardcoreProtester::getIMID()
{
	return TID_HARD_CORE_PROTESTER;
}