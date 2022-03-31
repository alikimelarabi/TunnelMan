#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"

#include <string>
#include <vector>
#include <queue>

class StudentWorld : public GameWorld
{
public:
	// CONSTRUCTOR AND DESTRUCTOR
	StudentWorld(std::string assetDir);
	~StudentWorld();

	// GAMECONTROLLER METHODS
	virtual int init();
	virtual int move();
	virtual void cleanUp();

	// OBJECT INITIALIZERS
	void initBoulders();
	void initOil();
	void initGold();
	void initProtester();
	void initPowerUp();
	void initSquirt(int xpos, int ypos, GraphObject::Direction dir);

	// OTHER METHODS
	double radius(int x1, int y1, int x2, int y2);
	bool nearSameLocation(int xpos, int ypos);
	bool boulderPresent(int xpos, int ypos);
	bool annoyProtesters(int xpos, int ypos, int hit, double max);
	void annoyTunnelman(int xpos, int ypos, int hit, double max);
	bool directSight(int xpos, int ypos);
	bool blockedPath(int xpos, int ypos, GraphObject::Direction dir);
	std::string possibleTurns(int xpos, int ypos, GraphObject::Direction dir);
	bool tookGoldNugget(int xpos, int ypos);
	void leftGoldNugget(int xpos, int ypos);
	void revealNearbyObjects(int xpos, int ypos);
	void createDisplayText();
	std::string formatText(int level, int lives, int hlth, int squirts, int gold, int oil, int sonar, int score);
	bool isEarthHere(int xpos, int ypos);
	void removeEarth(int xpos, int ypos); 
	bool isEmptyPatch(int xpos, int ypos); 
	void updatePatch(int x, int y);
	void dirToLeavePatch();
	GraphObject::Direction protestorLeavesInThisDirection(int xpos, int ypos);
	void hidePatch();
	void changeDirSign();
	GraphObject::Direction findPlayerDirection(int xpos, int ypos);
	int distanceFromTunnelman(int xpos, int ypos);
	void killActors();

	// ACCESSORS AND MUTATORS
	void minusOil();
	int getOil();
	void minusProtesters();
	Tunnelman* pointerToTunnelman();

private:

	struct Patch
	{
	public:
		Patch(bool value = false)
		{
			m_empty = value;
			m_available = false;
			m_radius_to_tunnelman = 0;
			m_direction_to_tunnelman = GraphObject::none;
			m_direction_leave = GraphObject::none;
		}

		bool m_empty; 
		GraphObject::Direction m_direction_leave; 
		bool m_available;
		int m_radius_to_tunnelman; 
		GraphObject::Direction m_direction_to_tunnelman; 
	};

	Earth* m_earth[64][60]; 
	Patch m_patch[61][61];
	Tunnelman* m_tunnelman;
	std::vector<Actor*> allActors;

	int m_oil;
	int m_time_new_protester;
	bool m_just_added_protester;
	int m_protestors;
	int m_max_protestors;
};

// HELPER FUNCTIONS AND CLASS (LEVEL CALCS, TICK COUNTS, ETC.)

int minimum(int x, int y); 
int maximum(int x, int y);

class Coord
{
public:
	Coord(int x, int y) :m_x(x), m_y(y) {}
	int x() const { return m_x; }
	int y() const { return m_y; }
private:
	int m_x;
	int m_y;
};

#endif // STUDENTWORLD_H_