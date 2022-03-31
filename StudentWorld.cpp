#include "StudentWorld.h"
#include <string>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>

using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// STUDENTWORLD CONSTRUCTOR AND DESTRUCTOR
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

StudentWorld::StudentWorld(std::string assetDir)
	: GameWorld(assetDir)
{
	m_oil = 0;
	m_time_new_protester = 0;
	m_just_added_protester = false;
	m_protestors = 0;
	m_max_protestors = 0;

	for (int r = 0; r < 61; r++)
		for (int c = 0; c < 61; c++)
			m_patch[r][c].m_empty = false;
}

StudentWorld::~StudentWorld()
{
	// delete Earth
	for (int r = 0; r < 64; r++)
		for (int c = 0; c < 60; c++)
			delete m_earth[r][c];

	// delete Actors
	std::vector<Actor*>::iterator it = allActors.begin();
	
	while (it != allActors.end())
	{
		delete (*it);
		it++;
	}

	while (!allActors.empty())
		allActors.pop_back();

	// delete Tunnelman
	delete m_tunnelman;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// INIT, MOVE, AND CLEANUP METHODS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int StudentWorld::init()
{
	// set Patch objects
	for (int i = 0; i <= 60; i++)
		for (int j = 0; j <= 60; j++)
			m_patch[i][j].m_empty = false;

	// create all Earth and Patch objects
	for (int r = 0; r < 64; r++)
		for (int c = 0; c < 60; c++)
		{
			if (r >= 30 && r <= 33 && c >= 4 && c <= 59)
			{
				m_earth[r][c] = nullptr;
				if (r == 30 && c >= 4) 
					m_patch[r][c].m_empty = true;
			}
			else
				m_earth[r][c] = new Earth(this, r, c);
		}

	for (int i = 0; i <= 60; i++)
		m_patch[i][60].m_empty = true;

	// create Tunnelman
	m_tunnelman = new Tunnelman(this);
	
	m_max_protestors = minimum(15, 2 + getLevel() * 1.5);
	m_protestors = 0;
	m_time_new_protester = 0;
	m_just_added_protester = false;

	// initiialize Actors
	initProtester();
	initBoulders();
	initOil();
	initGold();

	return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
	createDisplayText(); 
	initPowerUp();
	initProtester();
	dirToLeavePatch();
	changeDirSign();
	
	for (int i = 0; i < allActors.size(); i++)
	{
		allActors[i]->doSomething();

		if (!m_tunnelman->isAlive())
		{
			decLives();
			return GWSTATUS_PLAYER_DIED;
		}
	}

	m_tunnelman->doSomething();
	killActors();

	if (!m_tunnelman->isAlive())
	{
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}

	if (getOil() == 0)
	{
		playSound(SOUND_FINISHED_LEVEL);
		return GWSTATUS_FINISHED_LEVEL;
	}

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
	// delete any other Earth objects
	for (int r = 0; r < 64; r++)
		for (int c = 0; c < 60; c++)
			delete m_earth[r][c];

	// delete Tunnelman
	delete m_tunnelman;

	// delete Actors
	std::vector<Actor*>::iterator it = allActors.begin();

	while (it != allActors.end())
	{
		delete (*it);
		it++;
	}

	while (!allActors.empty())
	{
		allActors.pop_back();
	}
}


void StudentWorld::initBoulders()
{
	int B = minimum(getLevel() / 2 + 2, 9);

	for (int i = 0; i < B; i++)
	{
		int xpos, ypos;

		bool placement = false;
		while (!placement)
		{
			xpos = rand() % 61;
			ypos = rand() % 37 + 20;

			if (xpos < 27 || xpos > 33) // not in center
				placement = true;

			if (nearSameLocation(xpos, ypos))
				placement = false;
		}

		allActors.push_back(new Boulder(this, xpos, ypos)); 
	}
}


void StudentWorld::initOil()
{
	int L = minimum(2 + getLevel(), 20); 
	m_oil = L; // update oil to find
	
	for (int i = 0; i < L; i++)
	{
		int xpos, ypos;

		bool placement = false;
		while (!placement)
		{
			xpos = rand() % 61;
			ypos = rand() % 37 + 20;

			if (xpos < 27 || xpos > 33) // not in center
				placement = true;

			if (nearSameLocation(xpos, ypos)) 
				placement = false;
		}

		allActors.push_back(new Oil(this, xpos, ypos));
	}
}


void StudentWorld::initGold()
{
	int G = maximum(5 - getLevel() / 2, 2); 

	for (int i = 0; i < G; i++)
	{
		int xpos, ypos;

		bool placement = false;
		while (!placement)
		{
			xpos = rand() % 61;
			ypos = rand() % 37 + 20;

			if (xpos < 27 || xpos > 33) // not in center
				placement = true;

			if (nearSameLocation(xpos, ypos))
				placement = false;
		}

		allActors.push_back(new Gold(this, xpos, ypos));
	}
}

void StudentWorld::initProtester()
{
	if (!m_just_added_protester && m_protestors < m_max_protestors) 
	{
		RegularProtester* next;

		int POH = minimum(90, getLevel() * 10 + 30); 

		if (rand() % 100 < POH)
		{ 
			int N = maximum(50, 100 - getLevel() * 10);
			int M = 16 + getLevel() * 2;
			next = new HardcoreProtester(this, N, M);
		}
		else
		{ 
			next = new RegularProtester(this, TID_PROTESTER, 5);
		}
			
		next->setTicksToWaitBetweenMoves(maximum(0, 3 - getLevel() / 4));
		next->setStunTime(maximum(50, 100 - getLevel() * 10));

		allActors.push_back(next); 
		m_protestors++;
		m_just_added_protester = true;
		m_time_new_protester = maximum(25, 200 - getLevel());
	}

	else 
	{
		m_time_new_protester--;
		if (m_time_new_protester == 0)
			m_just_added_protester = false;
	}
}

void StudentWorld::initPowerUp()
{
	int G = getLevel() * 25 + 300;

	if (rand() % G == 10) 
	{
		PowerUp* p;

		if (rand() % 5 == 0)
		{
			p = new Sonar(this);
		}
		else 
		{
			int xpos, ypos;
			bool placement = false;

			while (!placement)
			{
				ypos = rand() % 57; // 0 to 56
				xpos = rand() % 61; // 0 to 60

				if (m_patch[xpos][ypos].m_empty)
					placement = true;
			}

			p = new WaterPool(this, xpos, ypos);
		}

		p->setTickCount(maximum(100, 300 - 10 * getLevel()));
		allActors.push_back(p);
	}
}

void StudentWorld::initSquirt(int xpos, int ypos, GraphObject::Direction dir)
{
	if (dir == GraphObject::right && m_patch[xpos + 4][ypos].m_empty && xpos + 4 < 61)
	{
		allActors.push_back(new Squirt(this, xpos + 4, ypos, dir));
		playSound(SOUND_PLAYER_SQUIRT);
		return;
	}
	

	if (dir == GraphObject::left && m_patch[xpos - 4][ypos].m_empty && xpos - 4 >= 0)
	{
		allActors.push_back(new Squirt(this, xpos - 4, ypos, dir));
		playSound(SOUND_PLAYER_SQUIRT);
		return;
	}

	if (dir == GraphObject::up && m_patch[xpos][ypos + 4].m_empty && ypos + 4 < 61)
	{
		allActors.push_back(new Squirt(this, xpos, ypos + 4, dir)); 
		playSound(SOUND_PLAYER_SQUIRT);
		return;
	}

	if (dir == GraphObject::down && m_patch[xpos][ypos - 4].m_empty && ypos - 4 >= 0)
	{
		allActors.push_back(new Squirt(this, xpos, ypos - 4, dir));
		playSound(SOUND_PLAYER_SQUIRT);
		return;
	}

	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// ALL OTHER METHODS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double StudentWorld::radius(int x0, int y0, int x1, int y1) 
{
	return sqrt((x0 - x1)*(x0 - x1) + (y0 - y1)*(y0 - y1));
}

bool StudentWorld::nearSameLocation(int xpos, int ypos) 
{
	for (int i = 0; i < allActors.size(); i++)
		if (radius(xpos, ypos, allActors[i]->getX(), allActors[i]->getY()) <= 6.)
			return true;

	return false;
}

bool StudentWorld::boulderPresent(int xpos, int ypos) 
{
	for (int i = 0; i < allActors.size(); i++)
		if (allActors[i]->getIMID() == TID_BOULDER)
			if (radius(xpos + 2, ypos + 2, allActors[i]->getX() + 2, allActors[i]->getY() + 2) <= 3.)
				return true; 

	return false;
}

bool StudentWorld::annoyProtesters(int xpos, int ypos, int hit, double max)
{
	bool didAnnoy = false;
	for (int i = 0; i < allActors.size(); i++)
		if (allActors[i]->getIMID() == TID_PROTESTER || allActors[i]->getIMID() == TID_HARD_CORE_PROTESTER) 
			if (radius(xpos, ypos, allActors[i]->getX(), allActors[i]->getY()) <= max && !allActors[i]->getIsGoingAwayFromOilField()) 
			{
				(*allActors[i]).annoy(hit); 
				return true;
			}
	
	return false;
}

void StudentWorld::annoyTunnelman(int xpos, int ypos, int hit, double max)
{

	if (radius(xpos, ypos, m_tunnelman->getX(), m_tunnelman->getY()) <= max) 
	{
		(*m_tunnelman).annoy(hit); 

		if (!m_tunnelman->isAlive())
			playSound(SOUND_PLAYER_GIVE_UP);
	}
}

bool StudentWorld::directSight(int xpos, int ypos)
{
	int diff;
	
	if (xpos == m_tunnelman->getX()) // see if can move in y dir
	{
		diff = ypos - m_tunnelman->getY();

		if (diff > 0)
		{
			for (int i = ypos; i > m_tunnelman->getY(); i--) // check path down
				if (!m_patch[xpos][i].m_empty)
					return false;
		}
		else if (diff < 0)
		{
			for (int j = ypos; j < m_tunnelman->getY(); j++) // check path up
				if (!m_patch[xpos][j].m_empty)
					return false;
		}
		return true; 
	}

	else if (ypos == m_tunnelman->getY()) // see if can move in x dir
	{
		diff = xpos - m_tunnelman->getX();

		if (diff > 0)
		{
			for (int i = xpos; i > m_tunnelman->getX(); i--) // check path left
				if (!m_patch[i][ypos].m_empty)
					return false;
		}
		else if (diff < 0)
		{
			for (int j = xpos; j < m_tunnelman->getX(); j++) // check path right
				if (!m_patch[j][ypos].m_empty)
					return false;
		}
		return true; 
	}

	return false;
}

bool StudentWorld::blockedPath(int xpos, int ypos, GraphObject::Direction dir)
{
	if (dir == GraphObject::left)
		return !(xpos - 1 >= 0 && m_patch[xpos - 1][ypos].m_empty);
	if (dir == GraphObject::right)
		return !(xpos + 1 <= 60 && m_patch[xpos + 1][ypos].m_empty);
	if (dir == GraphObject::up)
		return !(ypos + 1 <= 60 && m_patch[xpos][ypos + 1].m_empty);
	else
		return !(ypos - 1 >= 0 && m_patch[xpos][ypos - 1].m_empty); // looking down
}

string StudentWorld::possibleTurns(int xpos, int ypos, GraphObject::Direction dir)
{
	string count = "";
	
	if (dir == GraphObject::left || GraphObject::right)
	{
		if (m_patch[xpos][ypos + 1].m_empty && ypos < 60)
			count += "up";
		if (m_patch[xpos][ypos - 1].m_empty && ypos >= 1)
			count += "do";
		return count;
	}

	else
	{
		if (m_patch[xpos + 1][ypos].m_empty && xpos < 60)
			count += "ri";
		if (m_patch[xpos - 1][ypos].m_empty && xpos >= 1)
			count += "le";
		return count;
	}
}

bool StudentWorld::tookGoldNugget(int xpos, int ypos)
{
	for (int i = 0; i < allActors.size(); i++)
		if (allActors[i]->getIMID() == TID_PROTESTER || allActors[i]->getIMID() == TID_HARD_CORE_PROTESTER)
			if (radius(xpos, ypos, allActors[i]->getX(), allActors[i]->getY()) <= 3. && !allActors[i]->getIsGoingAwayFromOilField())
			{
				allActors[i]->takeGold(); 
				return true; 
			}
	
	return false;
}

void StudentWorld::leftGoldNugget(int xpos, int ypos)
{
	allActors.push_back(new Gold(this, xpos, ypos, false)); 
}

void StudentWorld::revealNearbyObjects(int xpos, int ypos)
{
	for (int i = 0; i < allActors.size(); i++)
		if (radius(xpos, ypos, allActors[i]->getX(), allActors[i]->getY()) <= 12.)
			allActors[i]->setVisible(true);
}

void StudentWorld::createDisplayText()
{
	int level = getLevel();
	int lives = getLives();
	int hlth = m_tunnelman->getHP();
	int squirts = m_tunnelman->getSquirts();
	int gold = m_tunnelman->getGold();
	int oil = getOil();
	int sonar = m_tunnelman->getSonarKits();
	int score = getScore();

	string s = formatText(level, lives, hlth, squirts, gold, oil, sonar, score);
	setGameStatText(s);
}

string StudentWorld::formatText(int level, int lives, int hlth, int squirts, int gold, int oil, int sonar, int score)
{
	stringstream disp;

	disp << "Lvl: ";					
	disp << setw(2) << level;

	disp << "  Lives: ";				
	disp << setw(1) << lives;

	disp << "  Hlth: ";				
	disp << setw(3) << hlth * 10;
	disp << '%';

	disp << "  Wtr: ";					
	disp << setw(2) << squirts;

	disp << "  Gld: ";					
	disp << setw(2) << gold;

	disp << "  Oil Left: ";			
	disp << setw(2) << oil;

	disp << "  Sonar: ";	
	disp << setw(2) << sonar;

	disp.fill('0');
	disp << "  Scr: ";					
	disp << setw(6) << score;

	return disp.str();
}

bool StudentWorld::isEarthHere(int xpos, int ypos)
{
	if (ypos >= 60) 
		return false;
	return m_earth[xpos][ypos] != nullptr;
}

void StudentWorld::removeEarth(int xpos, int ypos)
{
	if (ypos >= 60) 
		return;
	delete m_earth[xpos][ypos];
	m_earth[xpos][ypos] = nullptr;
}

bool StudentWorld::isEmptyPatch(int xpos, int ypos)
{
	return m_patch[xpos][ypos].m_empty;
}

void StudentWorld::updatePatch(int x, int y)
{
	m_patch[x][y] = true;

	if (x + 4 < 61 && m_patch[x + 4][y].m_empty) // RIGHT 4x4
	{
		m_patch[x + 3][y].m_empty = m_patch[x + 2][y].m_empty = m_patch[x + 1][y].m_empty = true;
	}
	else if (x + 3 < 61 && m_patch[x + 3][y].m_empty)
	{
		m_patch[x + 2][y].m_empty = m_patch[x + 1][y].m_empty = true;
	}
	else if (x + 2 < 61 && m_patch[x + 2][y].m_empty)
	{
		m_patch[x + 1][y].m_empty = true;
	}
	
	if (x - 4 > -1 && m_patch[x - 4][y].m_empty) // LEFT 4x4
	{
		m_patch[x - 3][y].m_empty = m_patch[x - 2][y].m_empty = m_patch[x - 1][y].m_empty = true;
	}
	else if (x - 3 > -1 && m_patch[x - 3][y].m_empty)
	{
		m_patch[x - 2][y].m_empty = m_patch[x - 1][y].m_empty = true;
	}
	else if (x - 2 > -1 && m_patch[x - 2][y].m_empty)
	{
		m_patch[x - 1][y].m_empty = true;
	}

	if (y + 4 < 61 && m_patch[x][y + 4].m_empty) // UP 4x4
	{
		m_patch[x][y + 3].m_empty = m_patch[x][y + 2].m_empty = m_patch[x][y + 1].m_empty = true;
	}
	else if (y + 3 < 61 && m_patch[x][y + 3].m_empty)
	{
		m_patch[x][y + 2].m_empty = m_patch[x][y + 1].m_empty = true;
	}
	else if (y + 2 < 61 && m_patch[x][y + 2].m_empty)
	{
		m_patch[x][y + 1].m_empty = true;
	}

	if (y - 4 > -1 && m_patch[x][y - 4].m_empty) // DOWN 4x4
	{
		m_patch[x][y - 3].m_empty = m_patch[x][y - 2].m_empty = m_patch[x][y - 1].m_empty = true;
	}
	else if (y - 3 > -1 && m_patch[x][y - 3].m_empty)
	{
		m_patch[x][y - 2].m_empty = m_patch[x][y - 1].m_empty = true;
	}
	else if (y - 2 > -1 && m_patch[x][y - 2].m_empty)
	{
		m_patch[x][y - 1].m_empty = true;
	}
}

void StudentWorld::dirToLeavePatch()
{
	hidePatch();
	queue<Coord> q;
	q.push(Coord(60, 60));

	while (!q.empty())
	{
		Coord temp = q.front();
		q.pop();

		m_patch[temp.x()][temp.y()].m_available = true;

		if (temp.x() + 1 < 61 && m_patch[temp.x() + 1][temp.y()].m_empty && !m_patch[temp.x() + 1][temp.y()].m_available)
		{
			m_patch[temp.x() + 1][temp.y()].m_available = true;
			m_patch[temp.x() + 1][temp.y()].m_direction_leave = GraphObject::left;
			q.push(Coord(temp.x() + 1, temp.y()));
		}

		if (temp.x() - 1 >= 0 && m_patch[temp.x() - 1][temp.y()].m_empty && !m_patch[temp.x() - 1][temp.y()].m_available) 
		{
			m_patch[temp.x() - 1][temp.y()].m_available = true; 
			m_patch[temp.x() - 1][temp.y()].m_direction_leave = GraphObject::right;
			q.push(Coord(temp.x() - 1, temp.y()));
		}

		if (temp.y() + 1 < 61 && m_patch[temp.x()][temp.y() + 1].m_empty && !m_patch[temp.x()][temp.y() + 1].m_available) 
		{
			m_patch[temp.x()][temp.y() + 1].m_available = true;
			m_patch[temp.x()][temp.y() + 1].m_direction_leave = GraphObject::down;
			q.push(Coord(temp.x(), temp.y() + 1));
		}

		if (temp.y() - 1 >= 0 && m_patch[temp.x()][temp.y() - 1].m_empty && !m_patch[temp.x()][temp.y() - 1].m_available) 
		{
			m_patch[temp.x()][temp.y() - 1].m_available = true;
			m_patch[temp.x()][temp.y() - 1].m_direction_leave = GraphObject::up;
			q.push(Coord(temp.x(), temp.y() - 1));
		}
	}

	return;
}

GraphObject::Direction StudentWorld::protestorLeavesInThisDirection(int xpos, int ypos)
{
	return m_patch[xpos][ypos].m_direction_leave;
}

void StudentWorld::hidePatch() 
{
	for (int r = 0; r <= 60; r++)
		for (int c = 0; c <= 60; c++)
		{
			m_patch[r][c].m_available = false;
			m_patch[r][c].m_radius_to_tunnelman = 0;
		}
}

void StudentWorld::changeDirSign()
{
	hidePatch();

	queue<Coord> q;

	int x = m_tunnelman->getX();
	int y = m_tunnelman->getY();
	int radius;

	q.push(Coord(x, y));

	while (!q.empty())
	{
		Coord temp = q.front();
		q.pop();

		radius = m_patch[temp.x()][temp.y()].m_radius_to_tunnelman + 1;
		m_patch[temp.x()][temp.y()].m_available = true;

		m_patch[temp.x()][temp.y()].m_available = true;

		if (temp.x() + 1 < 61 && (m_patch[temp.x() + 1][temp.y()].m_empty && !m_patch[temp.x() + 1][temp.y()].m_available)) // check right
		{
			m_patch[temp.x() + 1][temp.y()].m_available = true;
			m_patch[temp.x() + 1][temp.y()].m_direction_to_tunnelman = Actor::left; 
			m_patch[temp.x() + 1][temp.y()].m_radius_to_tunnelman = radius;
			q.push(Coord(temp.x() + 1, temp.y()));
		}

		if (temp.x() - 1 >= 0 && (m_patch[temp.x() - 1][temp.y()].m_empty && !m_patch[temp.x() - 1][temp.y()].m_available)) // check left
		{
			m_patch[temp.x() - 1][temp.y()].m_available = true; 
			m_patch[temp.x() - 1][temp.y()].m_direction_to_tunnelman = Actor::right; 
			m_patch[temp.x() - 1][temp.y()].m_radius_to_tunnelman = radius; 
			q.push(Coord(temp.x() - 1, temp.y()));
		}

		if (temp.y() + 1 < 61 && (m_patch[temp.x()][temp.y() + 1].m_empty && !m_patch[temp.x()][temp.y() + 1].m_available)) // check up
		{
			m_patch[temp.x()][temp.y() + 1].m_available = true;
			m_patch[temp.x()][temp.y() + 1].m_direction_to_tunnelman = Actor::down; 
			m_patch[temp.x()][temp.y() + 1].m_radius_to_tunnelman = radius;
			q.push(Coord(temp.x(), temp.y() + 1));
		}

		if (temp.y() - 1 >= 0 && (m_patch[temp.x()][temp.y() - 1].m_empty && !m_patch[temp.x()][temp.y() - 1].m_available)) // check down
		{
			m_patch[temp.x()][temp.y() - 1].m_available = true;
			m_patch[temp.x()][temp.y() - 1].m_direction_to_tunnelman = Actor::up; 
			m_patch[temp.x()][temp.y() - 1].m_radius_to_tunnelman = radius;
			q.push(Coord(temp.x(), temp.y() - 1));
		}
	}

	return;
}

GraphObject::Direction StudentWorld::findPlayerDirection(int xpos, int ypos)
{
	return m_patch[xpos][ypos].m_direction_to_tunnelman;
}

int StudentWorld::distanceFromTunnelman(int xpos, int ypos)
{
	return m_patch[xpos][ypos].m_radius_to_tunnelman;
}

void StudentWorld::killActors()
{
	int index[20]; // asumption of no more than 20 actors dying in a single tick (LOL would be funny if not true)
	int count = 0;

	for (int i = 0; i < allActors.size(); i++)
		if (allActors[i]->isAlive() == false)
		{
			index[count] = i;
			count++;
		}

	int i = count - 1;

	while (i >= 0)
	{
		int del = index[i];
		delete allActors[del]; 
		allActors[del] = allActors.back();
		allActors.pop_back(); 
		i--;
	}
}

// ACCESSORS AND MUTATORS

void StudentWorld::minusOil()
{
	m_oil--;
}
int StudentWorld::getOil()
{
	return m_oil;
}

void StudentWorld::minusProtesters()
{
	m_protestors--;
}

Tunnelman* StudentWorld::pointerToTunnelman()
{
	return m_tunnelman;
}

int minimum(int x, int y)
{
	if (x < y)
		return x;
	else
		return y;
}

int maximum(int x, int y)
{
	if (x > y)
		return x;
	else
		return y;
}