#include <ncurses.h>
#include <string>
using namespace std;

struct item
{
	int id;
	string name;
	int type;
	int dmg;
	int hp;
	int armor;
};

struct entity
{
	char symbol;
	int x;
	int y;
	int type;
	int hp;
	int dmg;
	int armor;
	
	int x_relative;
	int y_relative;
};

//general
int screenSetup();
//player
entity * playerSetup();
int handleInput(char input, entity * player, string location_current[], int &invSelect);
int movePlayer(entity * player, int y, int x, string location_current[]);
int drawUserTab(entity * player, int maxy, int maxx, int invSelect);
//world
int generateLevel(string location_current[]);
int drawWorld(string location_current[], entity * player, int maxy, int maxx);
bool checkEmpty(int y, int x, string location_current[]);

int main()
{

	//variables
	int screenMaxY, screenMaxX;
	int invSelect=0;
	char input;
	
	//whole level with only rooms/corridors
	string location[41]={"                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "    ###########                                                                                                                                                 ",
								 "    #         #                                                                                                                                                 ",
								 "    #         #                                                                                                                                                 ",
								 "    ######    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    ##############################################################                                                                                    ",
								 "         #                                                                                                                                                      ",
								 "         ###################################################################                                                                                    ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                "};
	//whole level covered with entities - referenced for collisions
	string location_current[41]={"                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "    ###########                                                                                                                                                 ",
								 "    #         #                                                                                                                                                 ",
								 "    #         #                                                                                                                                                 ",
								 "    ######    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    #                                                                                                                                                 ",
								 "         #    ##############################################################                                                                                    ",
								 "         #                                                                                                                                                      ",
								 "         ###################################################################                                                                                    ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                ",
								 "                                                                                                                                                                "};;
	
	//initialize ncurses
	screenSetup();
	
	//create player
	entity * player = playerSetup();
	
	//generate level
	
	
	
	//main game loop
	while((input = getch()) != 'q')
	{
		clear();
		refresh();
		getmaxyx(stdscr, screenMaxY, screenMaxX);
		
		
		
		handleInput(input, player, location_current, invSelect);
		
		
		
		drawWorld(location_current, player, screenMaxY, screenMaxX);
		drawUserTab(player, screenMaxY, screenMaxX, invSelect);
		
		
		move(player->y_relative, player->x_relative);
		refresh();
	}
	
	delete player;
	
	endwin();
	return 0;
}

int drawUserTab(entity * player, int maxy, int maxx, int invSelect)
{
	//selection bar
	string menu[4] = {"stats", "inventory", "info", "menu"};
	move(maxy-5, 0);
	printw("=");
	for(int i=0; i<4; i++)
	{
		if(invSelect==i)
			attron(A_REVERSE);
		printw(menu[i].c_str());
		attroff(A_REVERSE);
		
		printw("=");
	}
	for(int i=5+menu[0].length()+menu[1].length()+menu[2].length()+menu[3].length(); i<maxx; i++)
		printw("=");
	
	//tabs
	
	return 0;
}

bool checkEmpty(int y, int x, string location_current[])
{
	if(location_current[y][x] == ' ' || location_current[y][x] == '.')
		return true;
	
	return false;
}

int drawWorld(string location_current[], entity * player, int maxy, int maxx)
{
	//calculate starting point of the display. location is 40x80 (y,x) rectangle
	int start_y, start_x;
	
	if(maxy>45) maxy=45;
	if(maxx>160) maxx=160;
	
	//x
	if(player->x < maxx/2+1)
	{
		start_x = 0;
	}
	else
	{
		if(player->x > 160-maxx+maxx/2) 
		{
			start_x = 160-maxx;
		}
		else
		{
			start_x = player->x - maxx/2;
		}
	}
	
	//y
	if(player->y < maxy/2+1)
	{
		start_y = 0;
	}
	else
	{
		if(player->y > 40-maxy+maxy/2) 
		{
			start_y = 40-maxy;
		}
		else
		{
			start_y = player->y - maxy/2;
		}
	}
	
	
	//the displayed area will be from 0,0 to maxy-5,maxx (need space for bottom panel)
	for(int i=0; i<maxy-5; i++)
	{
		move(i,0);
		printw((location_current[start_y+i].substr(min(start_x, 159) , min(start_x+maxx,159))).c_str());
	}
	
	player->y_relative=player->y-start_y;
	player->x_relative=player->x-start_x;
	
	return 0;
}

int handleInput(char input, entity * player, string location_current[], int &invSelect)
{
	if(input=='w' || input=='W')
	{
		if(checkEmpty(player->y-1,player->x, location_current))
			movePlayer(player, player->y-1, player->x,location_current);
	}
	if(input=='s' || input=='S')
	{
		if(checkEmpty(player->y+1,player->x, location_current))
			movePlayer(player, player->y+1, player->x,location_current);
	}
	if(input=='d' || input=='D')
	{
		if(checkEmpty(player->y,player->x+1, location_current))
			movePlayer(player, player->y, player->x+1,location_current);
	}
	if(input=='a' || input=='A')
	{
		if(checkEmpty(player->y,player->x-1, location_current))
			movePlayer(player, player->y, player->x-1,location_current);
	}
	
	if(input=='u') invSelect=0;
	if(input=='i') invSelect=1;
	if(input=='o') invSelect=2;
	if(input=='p') invSelect=3;
	
	return 0;
}

int movePlayer(entity * player, int y, int x, string location_current[])
{
	location_current[player->y][player->x] = ' ';
	location_current[y][x] = '@';
	
	player->y=y;
	player->x=x;
	
	return 0;
}

entity * playerSetup()
{
	entity * player = new entity;
	player->type=0;
	player->hp=100;
	player->armor=0;
	player->dmg=1;
	player->symbol='@';
	player->y=10;
	player->x=70;
	
	return player;
}

int screenSetup()
{
	initscr();
	noecho();
	refresh();
	
	return 0;
}
