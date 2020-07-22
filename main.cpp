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
	string name;
	int x;
	int y;
	int type;
	int hp;
	int dmg;
	int armor;
	
	int x_relative;
	int y_relative;
};

struct room
{
	int x;
	int y;
	int x_size;
	int y_size;
	string content[10];
};

//general
int screenSetup();
//player
entity * playerSetup();
int handleInput(char input, entity * player, string location_current[], int &inv_select, entity * monster_list, int monster_amount);
int checkMonster(entity * player, int y, int x, string location_current[], entity * monster_list, int monster_amount);
int movePlayer(entity * player, int y, int x, string location_current[]);
int drawUserTab(entity * player, int maxy, int maxx, int inv_select);
//world
entity * generateLevel(string location_current[], entity * player, int max_map_y, entity * monster_types, int &monster_amount);
int generateRoom(room &Room, int sizey, int sizex, int y, int x);
int drawWorld(string location_current[], entity * player, int maxy, int maxx);
bool checkEmpty(int y, int x, string location_current[]);
entity * monstersSetup();
int handleMonsters(string location_current[], entity * monster_list, int monster_amount, entity * player);

int main()
{
	srand(time(NULL));
	
	//variables
	int screen_max_y, screen_max_x;
	int inv_select=0;
	char input;
	int game_state = 0;
	
	entity * monster_types = monstersSetup();
	int monster_amount=0;
	
	//location variable
	const int max_map_y = 46;
	string location_current[max_map_y];
	
	//initialize ncurses
	screenSetup();
	
	//create player
	entity * player = playerSetup();
	
	//generate level
	entity * monster_list = generateLevel(location_current, player, max_map_y, monster_types, monster_amount);
	
	//main game loop
	while((input = getch()) != 'q')
	{
		clear();
		refresh();
		getmaxyx(stdscr, screen_max_y, screen_max_x);
		
		if(input == 'm') monster_list = generateLevel(location_current, player, max_map_y, monster_types, monster_amount);
		
		if(game_state == 0)
		{
			handleInput(input, player, location_current, inv_select, monster_list, monster_amount);
			handleMonsters(location_current, monster_list, monster_amount, player);
			drawWorld(location_current, player, screen_max_y, screen_max_x);
			drawUserTab(player, screen_max_y, screen_max_x, inv_select);
			move(screen_max_y, 0);
			
			move(screen_max_y, 0);
			printw("%d", (monster_amount));
			
			move(player->y_relative, player->x_relative);
		}
		
		refresh();
	}
	
	delete player;
	
	endwin();
	return 0;
}

int checkMonster(entity * player, int y, int x, string location_current[], entity * monster_list, int monster_amount)
{
	char c_loc = location_current[y][x];
	if(c_loc=='s' || c_loc=='g' || c_loc=='O' || c_loc=='D')
	{
		int m_sub;
		//find monster in current location and deal damage to it
		for(int i = 0; i <monster_amount; i++)
		{
			if(monster_list[i].x == x && monster_list[i].y == y)
			{
				m_sub = i;
				break;
			}
		}
		
		monster_list[m_sub].hp -= player->dmg - monster_list[m_sub].armor;
		
	}
	
	return 0;
}

int handleMonsters(string location_current[], entity * monster_list, int monster_amount, entity * player)
{
	for(int i=0; i<monster_amount; i++)
	{
		if(monster_list[i].hp > 0)
		{
			//erase from previous position
			location_current[monster_list[i].y][monster_list[i].x] = '.';
			
			//make move
			//check if the monster is 10 cells away from player
			if(abs(monster_list[i].x-player->x)<=10 && abs(monster_list[i].y-player->y)<=10)
			{
				//can attack player?
				if((abs(monster_list[i].x-player->x)<=1 && abs(monster_list[i].y-player->y)<=1) && (abs(monster_list[i].x-player->x)+abs(monster_list[i].y-player->y)==1))
				{
					//damage player
					player->hp -= monster_list[i].dmg - player->armor;
				}
				//if not then just try to get to the player
				else
				{
					//do it half of the time so player can escape
					if(rand()%2)
					{
						bool moved = false;
						
						if(monster_list[i].x < player->x)
						{
							if(checkEmpty(monster_list[i].y, monster_list[i].x+1, location_current))
							{
								monster_list[i].x++;
								moved = true;
							}
						}
						else if(monster_list[i].x > player->x)
						{
							if(checkEmpty(monster_list[i].y, monster_list[i].x-1, location_current))
							{
								monster_list[i].x--;
								moved = true;
							}
						}
						
						if(monster_list[i].y < player->y && !moved)
						{
							if(checkEmpty(monster_list[i].y+1, monster_list[i].x, location_current))
							{
								monster_list[i].y++;
								moved = true;
							}
						}
						else if(monster_list[i].y > player->y && !moved)
						{
							if(checkEmpty(monster_list[i].y-1, monster_list[i].x, location_current))
							{
								monster_list[i].y--;
								moved = true;
							}
						}
					}
				}
			}
			//the monster is far from player
			else
			{
				int monster_move = rand()%6;
				
				if(monster_move == 0 && checkEmpty(monster_list[i].y, monster_list[i].x+1, location_current))
				{
					monster_list[i].x++;
				}
				else if(monster_move == 1 && checkEmpty(monster_list[i].y, monster_list[i].x-1, location_current))
				{
					monster_list[i].x--;
				}
				else if(monster_move == 2 && checkEmpty(monster_list[i].y+1, monster_list[i].x, location_current))
				{
					monster_list[i].y++;
				}
				else if(monster_move == 3 && checkEmpty(monster_list[i].y-1, monster_list[i].x, location_current))
				{
					monster_list[i].y--;
				}
			}
			
			//draw the monster to the world
			location_current[monster_list[i].y][monster_list[i].x] = monster_list[i].symbol;
		}
		//if it's dead
		else
		{
			location_current[monster_list[i].y][monster_list[i].x] = '.';
		}
	}
	
	
	return 0;
}

entity * monstersSetup()
{
	entity * monster_types = new entity[4];
	
	//monster types
	
	monster_types[0].symbol = 's';
	monster_types[0].name = "snake";
	monster_types[0].type = 1;
	monster_types[0].hp = 2;
	monster_types[0].dmg = 1;
	monster_types[0].armor = 0;
	
	monster_types[1].symbol = 'g';
	monster_types[1].name = "goblin";
	monster_types[1].type = 1;
	monster_types[1].hp = 5;
	monster_types[1].dmg = 3;
	monster_types[1].armor = 1;
	
	monster_types[2].symbol = 'O';
	monster_types[2].name = "orc";
	monster_types[2].type = 1;
	monster_types[2].hp = 12;
	monster_types[2].dmg = 5;
	monster_types[2].armor = 3;
	
	monster_types[3].symbol = 'D';
	monster_types[3].name = "demon";
	monster_types[3].type = 1;
	monster_types[3].hp = 16;
	monster_types[3].dmg = 10;
	monster_types[3].armor = 5;
	
	return monster_types;
}

entity * generateLevel(string location_current[], entity * player, int max_map_y, entity * monster_types, int &monster_amount)
{
	int room_count = rand()%16+9;
	room * rooms = new room[room_count];
	int connx;
	int conny;
	int connt = 0;
	int conny_d;
	int connx_d;
	
	for(int i=0; i<max_map_y; i++)
	{
		location_current[i]="                                                                                                                                                                ";
	}
	
	for(int i = 0; i < room_count; i++)
	{
		int roomposy = rand()%4*10;
		int roomposx = rand()%8*20;
		//look for free space to place the room in
		while(location_current[roomposy][roomposx]!=' ')
		{
			roomposy = rand()%4*10;
			roomposx = rand()%8*20;
		}
		
		generateRoom(rooms[i], rand()%4+5, rand()%12+7, roomposy, roomposx);
		
		//draw the room to location
		for(int j=0; j<rooms[i].y_size; j++)
		{
			location_current[rooms[i].y+j].replace(rooms[i].x, rooms[i].x_size, rooms[i].content[j]);
		}
		
	}
	
	//connect rooms
	for(int i = 1; i < room_count; i++)
	{
		room to_connect;
			to_connect = rooms[rand()%(i)];
			conny = rooms[i].y+rooms[i].y_size/2;
			connx = rooms[i].x+rooms[i].x_size/2;
			
			conny_d = to_connect.y+to_connect.y_size/2+1;
			connx_d = to_connect.x+to_connect.x_size/2+1;
			
			while(conny != conny_d || connx != connx_d)
			{
				connt = rand()%2;
				if(connt == 0)
				{
					if(conny < conny_d) conny ++;
					else if(conny > conny_d) conny --;
					else if(connx > connx_d) connx --;
					else if(connx < connx_d) connx ++;
				}
				else
				{
					if(connx > connx_d) connx --;
					else if(connx < connx_d) connx ++;
					else if(conny < conny_d) conny ++;
					else if(conny > conny_d) conny --;
				}
				
				location_current[conny][connx] = '.';
			}
	}
	
	//generate enemies
	monster_amount = rand()%20+5;
	entity * monster_list = new entity[monster_amount];
	
	for(int i=0; i<monster_amount; i++)
	{
		//assign monster type
		monster_list[i] = monster_types[rand()%4];
		
		//assign room and location
		room monster_rand_room = rooms[(rand()%(room_count-1))+1];
		monster_list[i].x = monster_rand_room.x + (rand()%(monster_rand_room.x_size-1))+1;
		monster_list[i].y = monster_rand_room.y + (rand()%(monster_rand_room.y_size-1))+1;
	}
	
	//generate chests
	
	
	
	//move player to the first room generated
	movePlayer(player, rooms[0].y+rooms[0].y_size/2, rooms[0].x+rooms[0].x_size/2, location_current);
	
	return monster_list;
}

int generateRoom(room &Room, int sizey, int sizex, int y, int x)
{
	//max y = 9
	//max x = 19
	
	Room.x = x;
	Room.y = y;
	Room.x_size = sizex;
	Room.y_size = sizey;
	
	//borders
	for(int i=0; i<sizex; i++)
	{
		Room.content[0] = Room.content[0] + '#';
	}
	for(int i=1; i<sizey-1; i++)
	{
		Room.content[i] = Room.content[i] + '#';
		for(int j=1; j<sizex-1; j++)
		{
			Room.content[i] = Room.content[i] + '.';
		}
		Room.content[i] = Room.content[i] + '#';
	}
	for(int i=0; i<sizex; i++)
	{
		Room.content[sizey-1] = Room.content[sizey-1] + '#';
	}
	
	/*//doors
	Room.dooramt = rand()%4+1;
	*/
	
	 return 0;
}

int drawUserTab(entity * player, int maxy, int maxx, int inv_select)
{
	//selection bar
	string menu[4] = {"stats", "inventory", "info", "menu"};
	move(maxy-5, 0);
	printw("=");
	for(int i=0; i<4; i++)
	{
		if(inv_select==i)
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
	if(location_current[y][x] == '.')
		return true;
	
	return false;
}

int drawWorld(string location_current[], entity * player, int maxy, int maxx)
{
	//calculate starting point of the display. location is 40x80 (y,x) rectangle
	int start_y, start_x;
	maxy-=5;
	if(maxx>159) maxx=159;
	if(maxy>40) maxy=40;
	
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
	
	
	//the displayed area will be from 0,0 to maxy,maxx (need space for bottom panel)
	for(int i=0; i<min(maxy, 39); i++)
	{
		move(i,0);
		printw((location_current[start_y+i].substr(min(start_x, 159) , min(start_x+maxx,159))).c_str());
	}
	
	player->y_relative=player->y-start_y;
	player->x_relative=player->x-start_x;
	
	return 0;
}

int handleInput(char input, entity * player, string location_current[], int &inv_select, entity * monster_list, int monster_amount)
{
	if(input=='w' || input=='W')
	{
		if(player->y>0)
		{
			if(checkEmpty(player->y-1,player->x, location_current))
				movePlayer(player, player->y-1, player->x,location_current);
			else if(checkMonster(player,player->y-1,player->x, location_current, monster_list, monster_amount));
		}
	}
	if(input=='s' || input=='S')
	{
		if(player->y<40)
		{
			if(checkEmpty(player->y+1,player->x, location_current))
				movePlayer(player, player->y+1, player->x,location_current);
			else if(checkMonster(player,player->y+1,player->x, location_current, monster_list, monster_amount));
		}
	}
	if(input=='d' || input=='D')
	{
		if(player->x<159)
		{
			if(checkEmpty(player->y,player->x+1, location_current))
				movePlayer(player, player->y, player->x+1,location_current);
			else if(checkMonster(player, player->y, player->x+1, location_current, monster_list, monster_amount));
		}
	}
	if(input=='a' || input=='A')
	{
		if(player->x>0)
		{
			if(checkEmpty(player->y,player->x-1, location_current))
				movePlayer(player, player->y, player->x-1,location_current);
			else if(checkMonster(player, player->y, player->x-1, location_current, monster_list, monster_amount));
		}
	}
	
	if(input=='u') inv_select=0;
	if(input=='i') inv_select=1;
	if(input=='o') inv_select=2;
	if(input=='p') inv_select=3;
	
	return 0;
}

int movePlayer(entity * player, int y, int x, string location_current[])
{
	location_current[player->y][player->x] = '.';
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
	player->dmg=4;
	player->symbol='@';
	player->y=40;
	player->x=0;
	
	return player;
}

int screenSetup()
{
	initscr();
	noecho();
	refresh();
	
	return 0;
}
