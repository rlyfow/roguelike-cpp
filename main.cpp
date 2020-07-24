#include <ncurses.h>
#include <string>
using namespace std;

struct item
{
	string name;
	int type;
	int dmg;
	int hp;
	int armor;
	int value;
};

struct entity
{
	char symbol;
	string name;
	int x;
	int y;
	int type;
	int hp; //max
	int c_hp; //current hp
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

struct player_profile
{
	int gold;
	int level;
	int exp;
	string name;
	item weapon;
	item armor;
	item shield;
	item inventory[30];
	int item_amount;
};

struct combat_log
{
	entity monster;
	int cooldown;
};

//general
int screenSetup();

//player
entity * playerSetup();
player_profile * profileSetup();
int handleInput(char input, entity * player, string location_current[], int &inv_select, entity * monster_list, int monster_amount, combat_log * c_log);
int checkMonster(entity * player, int y, int x, string location_current[], entity * monster_list, int monster_amount);
int movePlayer(entity * player, int y, int x, string location_current[]);
int drawUserTab(entity * player, int maxy, int maxx, int &inv_select, player_profile * p_profile, char input, int &inv_select_2);
int mantainPlayer(player_profile * p_profile, entity * player, int &game_state);
int drawFullInventory(char input, player_profile * p_profile, entity * player, int& inv_select, int maxy, int maxx, int& game_s, int& inv_select_2);
int giveLoot(player_profile * p_profile, int difficulty, entity * player, entity chest);
item generateItem(int type);
int initCombatLog(entity * monster_list, int monster_id, combat_log * c_log);
int displayCombatLog(combat_log * c_log);

//world
entity * generateLevel(string location_current[], entity * player, int max_map_y, entity * monster_types, int &monster_amount, int difficulty);
int generateRoom(room &Room, int sizey, int sizex, int y, int x);
int drawWorld(string location_current[], entity * player, int maxy, int maxx);
bool checkEmpty(int y, int x, string location_current[]);
entity * monstersSetup();
int handleMonsters(string location_current[], entity * monster_list, int monster_amount, entity * player, player_profile * p_profile, int difficulty, entity * monster_types);

int main()
{
	srand(time(NULL));
	
	//variables
	int screen_max_y, screen_max_x;
	int inv_select=1;
	int inv_select_2=0;
	char input;
	int game_state = 0;
	int difficulty = 3;
	
	// BTW SET INV SELECT 2 TO 0 WHEN EXITING ANYTHING THAT MANIPUATES EQ
	

	combat_log * c_log = new combat_log;
	c_log -> cooldown = 0;
	
	entity * monster_types = monstersSetup();
	int monster_amount=0;
	
	//location variable
	const int max_map_y = 46;
	string location_current[max_map_y];
	
	//initialize ncurses
	screenSetup();
	
	//create player
	entity * player = playerSetup();
	player_profile * p_profile = profileSetup();
	
	//generate level
	entity * monster_list = generateLevel(location_current, player, max_map_y, monster_types, monster_amount, difficulty);
	handleMonsters(location_current, monster_list, monster_amount, player, p_profile, difficulty, monster_types);
	//main game loop
	
	
	while(true)
	{
		clear();
		refresh();
		getmaxyx(stdscr, screen_max_y, screen_max_x);
		
		if(input == 'm') monster_list = generateLevel(location_current, player, max_map_y, monster_types, monster_amount, difficulty);
		
		if(game_state == 0) // dungeon display
		{
			bool step = handleInput(input, player, location_current, inv_select, monster_list, monster_amount, c_log);
			if(step) handleMonsters(location_current, monster_list, monster_amount, player, p_profile, difficulty, monster_types);
			drawWorld(location_current, player, screen_max_y, screen_max_x);
			drawUserTab(player, screen_max_y, screen_max_x, inv_select, p_profile, input, inv_select_2);

			move(player->y_relative, player->x_relative);
			
			if(inv_select == 5) 
			{
				game_state = 1;
				inv_select_2 = 0;
				inv_select = 0;
				input= '1';
			}
			
			if(c_log -> cooldown > 0 && game_state == 0)
			{
				displayCombatLog(c_log);
				if(step) c_log -> cooldown--;
			}
			
			if(game_state == 0)
				input = getch();
		}
		
		if(game_state==1) // display full inventory screen
		{
			drawFullInventory(input, p_profile, player, inv_select_2, screen_max_y, screen_max_x, game_state, inv_select);

			if(game_state == 0) 
			{
				inv_select_2 = 0;
				inv_select = 1;
				input= '1';
			}
			
			if(game_state == 1)
				input = getch();
		}
		
		refresh();
		
	}
	
	delete player;
	delete p_profile;
	
	endwin();
	return 0;
}

// functions

int displayCombatLog(combat_log * c_log)
{
	move(0,0);
	printw("                    |");
	move(1,0);
	printw("                    |");
	move(2,0);
	printw("                    |");
	move(3,0);
	printw("---------------------");
	
	move(0,1);
	printw("%s", c_log->monster.name.c_str());
	move(1,1);
	printw("dmg:%d  armor:%d", c_log->monster.dmg, c_log->monster.armor);
	move(2,1);
	printw("[");
	for(int i=0; i<=14; i++)
	{
		if(i<(c_log->monster.c_hp*14)/c_log->monster.hp)
		{
			printw("#");
		}
		else
		{
			printw(".");
		}
	}
	printw("]");
	
	return 0;
}

item generateItem(int type)
{
	item new_item;
	string weapon1[13] = {"might", "sharp", "long", "short", "cursed", "blessed", "holy", "demon", "mage", "warrior", "forbidden", "mysterious", "corrupted"};
	string weapon2[13] = {"cube", "blade", "knife", "sword", "machete", "axe", "dagger", "spear", "hammer", "gloves", "hurricane", "stone", "katana"};
	
	string armor1[13] = {"titan", "shiny", "leather", "steel", "iron", "scale", "holy", "blessed", "mage", "warrior", "forbidden", "mysterious", "corrupted"};
	string armor2[13] = {"shell", "armor", "protection", "blessing", "armlet", "hidding", "obsession", "soul", "stones", "leaf", "cape", "helmet", "hat"};
	
	string shield1[13] = {"defender", "protective", "spiked", "mystery", "iron", "steel", "wooden", "blessed", "guards", "warrior", "forbidden", "mysterious", "corrupted"};
	string shield2[13] = {"guard", "orb", "buckler", "blocker", "counter", "protection", "armlet", "power", "boots", "pants", "book", "stone", "might"};
	
	if(type==1)
		new_item.name = weapon1[rand()%13]+" "+weapon2[rand()%13];
		
	if(type==2)
		new_item.name = armor1[rand()%13]+" "+armor2[rand()%13];
	
	if(type==3)
		new_item.name = shield1[rand()%13]+" "+shield2[rand()%13];
	
	return new_item;
}

int giveLoot(player_profile * p_profile, int difficulty, entity * player, entity chest)
{
	
	if(p_profile->item_amount<30)
	{
		int item_type = rand()%3+1;
		item loot = generateItem(item_type);
		loot.type = item_type;
		//give stats
		
		if(chest.type == 3)
		{
			//loot from chest
			if(item_type == 1)
			{
				loot.dmg = (rand()%chest.dmg)+rand()%max((p_profile->level/3),2)+max(rand()%difficulty,1);
			}
			if(item_type == 2)
			{
				loot.hp = (rand()%chest.hp)+rand()%max((p_profile->level/3),2)+max(rand()%difficulty,1);
			}
			if(item_type == 3)
			{
				loot.armor = (rand()%chest.armor)+rand()%max((p_profile->level/3),2)+max(rand()%difficulty,1);
			}
		}
		else
		{
			//loot from monster
			if(item_type == 1)
			{
				loot.dmg = rand()%max((p_profile->level/3),2)+max(rand()%difficulty,1);
			}
			if(item_type == 2)
			{
				loot.hp = rand()%max((p_profile->level/3),2)+max(rand()%difficulty,1);
			}
			if(item_type == 3)
			{
				loot.armor = rand()%max((p_profile->level/3),2)+max(rand()%difficulty,1);
			}
		}
		
		p_profile->inventory[p_profile->item_amount] = loot;
		p_profile->item_amount++;
	}
	
	return 0;
}

int organizeInventory(player_profile * p_profile, int& inv_select)
{
	
	//check if the slot is actually empty
	if(p_profile->inventory[inv_select].type == 0 && inv_select < 29)
		for(int i=inv_select; i<29; i++)
		{
			if(p_profile->inventory[i+1].type == 0) break;
			item temp;
			temp = p_profile->inventory[i+1];
			p_profile->inventory[i+1] = p_profile->inventory[i];
			p_profile->inventory[i] = temp;
		}
	
	if(inv_select > 0) inv_select--;
	
	return 0;
}

int mantainPlayer(player_profile * p_profile, entity * player, int &game_state)
{
	if(p_profile->exp >= 100) //level up
	{
		p_profile->exp -= 100;
		p_profile->level+=1;
	}
	if(player->hp <= 0) //death
	{
		game_state = 99;
	}
	
	return 0;
}

int drawFullInventory(char input, player_profile * p_profile, entity * player, int& inv_select, int maxy, int maxx, int& game_s, int& inv_select_2)
{
	
	
	if(inv_select_2 == 300 || inv_select_2 == 200)
	{
		inv_select_2 = 0;
		input = '1';
	}
	if(input=='e' || input == 'E')
	{
		if(inv_select < p_profile->item_amount-1) inv_select++;
	}
	if(input=='q' || input == 'Q')
	{
		if(inv_select > 0) inv_select--;
	}
	if(input=='s' || input == 'S')
	{
		inv_select_2 = 100;
	}
	
	//equip/swap
	if(input=='w' || input == 'W')
	{
		if(p_profile->inventory[inv_select].type == 1)
		{
			if(p_profile->weapon.type == 0)
			{
				//put selected item to empty slot
				p_profile->weapon = p_profile->inventory[inv_select];
				p_profile->inventory[inv_select].type=0;
				organizeInventory(p_profile, inv_select);
				p_profile->item_amount--;
				
				player->dmg += p_profile->weapon.dmg;
			}
			else
			{
				//swap the items
				player->dmg -= p_profile->weapon.dmg;
				
				item temp = p_profile->weapon;
				p_profile->weapon = p_profile->inventory[inv_select];
				p_profile->inventory[inv_select] = temp;
				
				player->dmg += p_profile->weapon.dmg;
			}
		}
		else if(p_profile->inventory[inv_select].type == 2)
		{
			if(p_profile->armor.type == 0)
			{
				//put selected item to empty slot
				p_profile->armor = p_profile->inventory[inv_select];
				p_profile->inventory[inv_select].type=0;
				organizeInventory(p_profile, inv_select);
				p_profile->item_amount--;
				
				player->hp += p_profile->armor.hp;
			}
			else
			{
				//swap the items
				player->hp -= p_profile->armor.hp;
				
				item temp = p_profile->armor;
				p_profile->armor = p_profile->inventory[inv_select];
				p_profile->inventory[inv_select] = temp;
				
				player->hp += p_profile->armor.hp;
			}
		}
		else if(p_profile->inventory[inv_select].type == 3)
		{
			if(p_profile->shield.type == 0)
			{
				//put selected item to empty slot
				
				
				p_profile->shield = p_profile->inventory[inv_select];
				p_profile->inventory[inv_select].type=0;
				organizeInventory(p_profile, inv_select);
				p_profile->item_amount--;
				player->armor += p_profile->shield.armor;
			}
			else
			{
				//swap the items
				player->armor -= p_profile->shield.hp;
				
				item temp = p_profile->shield;
				p_profile->shield = p_profile->inventory[inv_select];
				p_profile->inventory[inv_select] = temp;
				
				player->armor += p_profile->shield.armor;
			}
		}
	}
	
	//unequip
	if(inv_select_2 == 100)
	{
		if(input=='u' || input == 'U')
		{
			if(p_profile->item_amount<30)
			{
				if(p_profile->weapon.type == 1)
				{
					player->dmg -= p_profile->weapon.dmg;
					p_profile->inventory[p_profile->item_amount] = p_profile->weapon;
					p_profile->weapon.type = 0;
					p_profile->item_amount++;
				}
				else
				{
					//no item equipped
					inv_select_2 = 300;
				}
			}
			else
			{
				//no space
				inv_select_2 = 200;
			}
		}
		else if(input=='i' || input == 'I')
		{
			if(p_profile->item_amount<30)
			{
				if(p_profile->armor.type == 2)
				{
					player->hp -= p_profile->armor.hp;
					p_profile->inventory[p_profile->item_amount] = p_profile->armor;
					p_profile->armor.type = 0;
					p_profile->item_amount++;
				}
				else
				{
					//no item equipped
					inv_select_2 = 300;
				}
			}
			else
			{
				//no space
				inv_select_2 = 200;
			}
		}
		else if(input=='o' || input == 'O')
		{
			if(p_profile->item_amount<30)
			{
				if(p_profile->armor.type == 3)
				{
					player->armor -= p_profile->shield.armor;
					p_profile->inventory[p_profile->item_amount] = p_profile->shield;
					p_profile->shield.type = 0;
					p_profile->item_amount++;
				}
				else
				{
					//no item equipped
					inv_select_2 = 300;
				}
			}
			else
			{
				//no space
				inv_select_2 = 200;
			}
		}
		else if(input=='p' || input == 'P')
		{
			inv_select_2 = 0;
		}
	}
	
	
	
	
	
	//destroy
	if(input=='x' || input == 'X')
	{
		if(p_profile->item_amount>0)
		{
			p_profile->inventory[inv_select].type = 0;
			organizeInventory(p_profile, inv_select);
			p_profile->item_amount--;
		}
	}
	
	//main screen - equipment
	maxy-=5;
	move((maxy+3)/2-4, (maxx/2-4)/2);
	printw("O");
	move((maxy+3)/2-3, (maxx/2-4)/2-2);
	printw("/ | \\");
	move((maxy+3)/2-2, (maxx/2-4)/2-3);
	printw("/  |  \\");
	move((maxy+3)/2-1, (maxx/2-4)/2-2);
	printw(" / \\");
	move((maxy+3)/2, (maxx/2-4)/2-3);
	printw(" |   | ");
	
	
	move((maxy+3)/2+1, (maxx/2-4)/2-3);
	printw("dmg: %d", player->dmg);
	move((maxy+3)/2+2, (maxx/2-4)/2-3);
	printw("hp: %d", player->hp);
	move((maxy+3)/2+3, (maxx/2-4)/2-3);
	printw("armor: %d", player->armor);
	
	//weapon
	if(p_profile->weapon.type==1)
	{
		move(((maxy-6)/2-4)/2+1, (maxx/2-5)/2-5);
		printw("weapon:");
		move(((maxy-6)/2-4)/2+2, (maxx/2-5)/2-5);
		printw("%s", p_profile->weapon.name.c_str());
		move(((maxy-6)/2-4)/2+3, (maxx/2-5)/2-5);
		printw("dmg: +%d", p_profile->weapon.dmg);
		move(((maxy-6)/2-4)/2+4, (maxx/2-5)/2-5);
		if(p_profile->inventory[inv_select].type == 1)
			printw("##############");
	}
	else
	{
		move(((maxy-6)/2-4)/2+1, (maxx/2-5)/2-5);
		printw("weapon:");
		move(((maxy-6)/2-4)/2+2, (maxx/2-5)/2-5);
		printw("none");
		move(((maxy-6)/2-4)/2+3, (maxx/2-5)/2-5);
		printw("dmg: +0");
		move(((maxy-6)/2-4)/2+4, (maxx/2-5)/2-5);
		if(p_profile->inventory[inv_select].type == 1)
			printw("##############");
	}
	
	
	//armor
	if(p_profile->armor.type==2)
	{
		move((maxy-5)/2-5, ((maxx/2-2)+maxx)/2-10);
		printw("armor:");
		move((maxy-5)/2-4, ((maxx/2-2)+maxx)/2-10);
		printw("%s", p_profile->armor.name.c_str());
		move((maxy-5)/2-3, ((maxx/2-2)+maxx)/2-10);
		printw("hp: +%d", p_profile->armor.hp);
		move((maxy-5)/2-2, ((maxx/2-2)+maxx)/2-10);
		if(p_profile->inventory[inv_select].type == 2)
			printw("##############");
	}
	else
	{
		move((maxy-5)/2-5, ((maxx/2-2)+maxx)/2-10);
		printw("armor:");
		move((maxy-5)/2-4, ((maxx/2-2)+maxx)/2-10);
		printw("none");
		move((maxy-5)/2-3, ((maxx/2-2)+maxx)/2-10);
		printw("hp: +0");
		move((maxy-5)/2-2, ((maxx/2-2)+maxx)/2-10);
		if(p_profile->inventory[inv_select].type == 2)
			printw("##############");
	}
	
	//shield
	if(p_profile->shield.type==3)
	{
		move(((maxy-5)/2+maxy)/2-4, ((maxx/2-4)/2+maxx)/2-5);
		printw("shield:");
		move(((maxy-5)/2+maxy)/2-3, ((maxx/2-4)/2+maxx)/2-5);
		printw("%s", p_profile->shield.name.c_str());
		move(((maxy-5)/2+maxy)/2-2, ((maxx/2-4)/2+maxx)/2-5);
		printw("armor: +%d", p_profile->shield.armor);
		move(((maxy-5)/2+maxy)/2-1, ((maxx/2-4)/2+maxx)/2-5);
		if(p_profile->inventory[inv_select].type == 3)
			printw("##############");
	}
	else
	{
		move(((maxy-5)/2+maxy)/2-4, ((maxx/2-4)/2+maxx)/2-5);
		printw("shield:");
		move(((maxy-5)/2+maxy)/2-3, ((maxx/2-4)/2+maxx)/2-5);
		printw("none");
		move(((maxy-5)/2+maxy)/2-2, ((maxx/2-4)/2+maxx)/2-5);
		printw("armor: +0");
		move(((maxy-5)/2+maxy)/2-1, ((maxx/2-4)/2+maxx)/2-5);
		if(p_profile->inventory[inv_select].type == 3)
			printw("##############");
	}
	
	maxy+=5;
	
	int inv_item_max = (maxx)/21;
	int j=0;
	
	if(inv_select_2 == 0)
	for(int i=inv_select; j<inv_item_max; i++)
	{
		if(i == p_profile->item_amount) break;
		move(maxy-4, j*21);
		if((p_profile->inventory[i].name).length() > 16)
		{
			printw(" | %s... |", (p_profile->inventory[i].name).substr(0, 13).c_str());
		}
		else
		{
			printw(" |                  |");
			move(maxy-4, j*21+3);
			printw("%s", p_profile->inventory[i].name.c_str());
		}
		
		move(maxy-3, j*21);
		if(p_profile->inventory[i].type == 0) // neutral
		{
			printw(" |                  |");
		}
		if(p_profile->inventory[i].type == 1) // weapon
		{
			printw(" | dmg +            |");
			move(maxy-3, j*21+8);
			printw("%d", p_profile->inventory[i].dmg);
		}
		if(p_profile->inventory[i].type == 2) // armor
		{
				printw(" | hp +             |");
		move(maxy-3, j*21+7);
			printw("%d", p_profile->inventory[i].hp);
		}
		if(p_profile->inventory[i].type == 3) // shield
		{
			printw(" | armor +          |");
			move(maxy-3, j*21+10);
			printw("%d", p_profile->inventory[i].armor);
		}
		
		move(maxy-2, j*21);
		if(i==inv_select)
			printw(" ####################");
		else
			printw(" --------------------");
		j++;
	}
	
	
	
	
	
	
	
	//bottom item panel
	move(maxy-5, 0);
	for(int i=0; i<maxx; i++)
			printw("=");
	
	move(maxy-1, 1);
	printw("  <-- Q");
	move(maxy-1, maxx/2-4);
	printw("v SPACE v");
	move(maxy-1, maxx-7);
	printw("E -->  ");
	
	move(maxy-7, 1); 
	printw(" ^");
	move(maxy-6, 1);
	if(maxx > 45)
		printw(" | W - equip");
	else
		printw(" | W");
	
	if(maxx > 45)
	{
		move(maxy-6, maxx/2-10);
		printw("X - destroy selected");
	}
	else
	{
		move(maxy-6, maxx/2);
		printw("X");
	}
	move(maxy-7, maxx-4);
	printw("  |");
	
	if(maxx > 45)
	{
		move(maxy-6, maxx-14);
		printw("unequip - S v");
	}
	else
	{
		move(maxy-6, maxx-5);
		printw(" S v");
	}
	
	
	
	
	
	
	if(inv_select_2 == 100)
	{
		move(maxy-4, 1);
		printw("What do you want to unequip?");
		move(maxy-3, 1);
		printw("U - weapon");
		move(maxy-2, 1);
		printw("I - armor");
		move(maxy-3, 16);
		printw("O - shield");
		move(maxy-2, 16);
		printw("P - cancel");
	}
	
	if(inv_select_2 == 200)
	{
		move(maxy-4, 1);
		printw("Not enough space in inventory.");
		move(maxy-3, 1);
		printw("Press any key");
	}
	
	if(inv_select_2 == 300)
	{
		move(maxy-4, 1);
		printw("There is no item to unequip.");
		move(maxy-3, 1);
		printw("Press any key");
	}
	
	if(input==' ')
	{
		clear();
		inv_select = -1;
		input = '1';
		game_s=0;
	}
	
	return 1;
}

int checkMonster(entity * player, int y, int x, string location_current[], entity * monster_list, int monster_amount, combat_log * c_log)
{
	//find monster in current location and deal damage to it
	for(int i = 0; i <monster_amount; i++)
	{
		if(monster_list[i].x == x && monster_list[i].y == y && monster_list[i].type == 1)
		{
			int damage = max((player->dmg - monster_list[i].armor + rand()%4-1), 0);
			monster_list[i].c_hp -= damage;
			initCombatLog(monster_list, i, c_log);
		}
		if(monster_list[i].x == x && monster_list[i].y == y && monster_list[i].type == 2)
		{
			int damage = max((player->dmg - monster_list[i].armor + rand()%4-1), 0);
			monster_list[i].c_hp -= damage;
			initCombatLog(monster_list, i, c_log);
		}
		if(monster_list[i].x == x && monster_list[i].y == y && monster_list[i].type == 3)
		{
			int damage = max((player->dmg - monster_list[i].armor + rand()%4-1), 0);
			monster_list[i].c_hp -= damage;
			initCombatLog(monster_list, i, c_log);
		}
	}
	
	return 0;
}

int initCombatLog(entity * monster_list, int monster_id, combat_log * c_log)
{
	c_log -> cooldown = 4;
	c_log -> monster = monster_list[monster_id];
	return 0;
}

int handleMonsters(string location_current[], entity * monster_list, int monster_amount, entity * player, player_profile * p_profile, int difficulty, entity * monster_types)
{
	for(int i=0; i<monster_amount; i++)
	{
		if(monster_list[i].c_hp > 0 && monster_list[i].type == 1)
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
					player->hp -= max((monster_list[i].dmg - player->armor + rand()%3-1), 0);
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
		//check for dead
		else if(monster_list[i].type == 1 && monster_list[i].c_hp <= 0)
		{
			monster_list[i].type=-1;
			location_current[monster_list[i].y][monster_list[i].x] = '.';
			
			//give loot
			giveLoot(p_profile, difficulty, player, monster_list[i]);
			
			//give exp
			p_profile->exp += monster_list[i].dmg;
		}
		
		//neutral ai
		if(monster_list[i].c_hp > 0 && monster_list[i].type == 2)
		{
			//erase from previous position
			location_current[monster_list[i].y][monster_list[i].x] = '.';
			
			//make move
			//check if the monster is 5 cells away from player
			if(abs(monster_list[i].x-player->x)<=5 && abs(monster_list[i].y-player->y)<=5)
			{
				//if yes, run away from player
				bool moved = false;
				
				if(monster_list[i].x < player->x)
				{
					if(checkEmpty(monster_list[i].y, monster_list[i].x-1, location_current))
					{
						monster_list[i].x--;
						moved = true;
					}
				}
				else if(monster_list[i].x > player->x)
				{
					if(checkEmpty(monster_list[i].y, monster_list[i].x+1, location_current))
					{
						monster_list[i].x++;
						moved = true;
					}
				}
					
				if(monster_list[i].y < player->y && !moved)
				{
					if(checkEmpty(monster_list[i].y-1, monster_list[i].x, location_current))
					{
						monster_list[i].y--;
						moved = true;
					}
				}
				else if(monster_list[i].y > player->y && !moved)
				{
					if(checkEmpty(monster_list[i].y+1, monster_list[i].x, location_current))
					{
						monster_list[i].y++;
						moved = true;
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
		
		if(monster_list[i].c_hp > 0 && monster_list[i].type == 3)
		{
			location_current[monster_list[i].y][monster_list[i].x] = monster_list[i].symbol;
		}
		else if(monster_list[i].type == 3 && monster_list[i].c_hp <= 0)
		{
			monster_list[i].type=-1;
			location_current[monster_list[i].y][monster_list[i].x] = '.';
			
			//give loot
			int j=14;
			for(; j<=20; j++)
			{
				if(monster_list[i].symbol==monster_types[j].symbol)
					break;
			}
			giveLoot(p_profile, difficulty, player, monster_types[j]);
		}
	}
	
	
	return 0;
}

entity * monstersSetup()
{
	entity * monster_types = new entity[21];
	
	// ---- monsters
	
	monster_types[0].symbol = 's';
	monster_types[0].name = "snake";
	monster_types[0].type = 1;
	monster_types[0].hp = 2;
	monster_types[0].dmg = 1;
	monster_types[0].armor = 0;
	
	monster_types[1].symbol = 'S';
	monster_types[1].name = "big snake";
	monster_types[1].type = 1;
	monster_types[1].hp = 3;
	monster_types[1].dmg = 2;
	monster_types[1].armor = 0;
	
	monster_types[2].symbol = 'w';
	monster_types[2].name = "skeleton warrior";
	monster_types[2].type = 1;
	monster_types[2].hp = 5;
	monster_types[2].dmg = 4;
	monster_types[2].armor = 2;
	
	monster_types[3].symbol = 'g';
	monster_types[3].name = "goblin";
	monster_types[3].type = 1;
	monster_types[3].hp = 6;
	monster_types[3].dmg = 5;
	monster_types[3].armor = 3;
	
	monster_types[4].symbol = 'o';
	monster_types[4].name = "orc";
	monster_types[4].type = 1;
	monster_types[4].hp = 12;
	monster_types[4].dmg = 5;
	monster_types[4].armor = 3;
	
	monster_types[5].symbol = 'O';
	monster_types[5].name = "orc gladiator";
	monster_types[5].type = 1;
	monster_types[5].hp = 16;
	monster_types[5].dmg = 10;
	monster_types[5].armor = 5;
	
	monster_types[6].symbol = 'D';
	monster_types[6].name = "demon";
	monster_types[6].type = 1;
	monster_types[6].hp = 25;
	monster_types[6].dmg = 25;
	monster_types[6].armor = 10;
	
	monster_types[7].symbol = 'V';
	monster_types[7].name = "cursed spectre";
	monster_types[7].type = 1;
	monster_types[7].hp = 10;
	monster_types[7].dmg = 5;
	monster_types[7].armor = 20;
	
	monster_types[8].symbol = 'b';
	monster_types[8].name = "small ironhorn";
	monster_types[8].type = 1;
	monster_types[8].hp = 25;
	monster_types[8].dmg = 7;
	monster_types[8].armor = 25;
	
	monster_types[9].symbol = 'B';
	monster_types[9].name = "big ironhorn";
	monster_types[9].type = 1;
	monster_types[9].hp = 35;
	monster_types[9].dmg = 20;
	monster_types[9].armor = 35;
	
	monster_types[10].symbol = 'R';
	monster_types[10].name = "roar legend";
	monster_types[10].type = 1;
	monster_types[10].hp = 50;
	monster_types[10].dmg = 26;
	monster_types[10].armor = 40;
	
	monster_types[11].symbol = 'Z';
	monster_types[11].name = "zar";
	monster_types[11].type = 1;
	monster_types[11].hp = 100;
	monster_types[11].dmg = 51;
	monster_types[11].armor = 95;
	
	// ----- neutrals
	
	monster_types[12].symbol = 'c';
	monster_types[12].name = "cat";
	monster_types[12].type = 2;
	monster_types[12].hp = 2;
	monster_types[12].dmg = 0;
	monster_types[12].armor = 0;
	
	monster_types[13].symbol = ',';
	monster_types[13].name = "mimic";
	monster_types[13].type = 2;
	monster_types[13].hp = 1;
	monster_types[13].dmg = 0;
	monster_types[13].armor = 5;
	
	// ----- chests
	
	monster_types[14].symbol = '1';
	monster_types[14].name = "small chest";
	monster_types[14].type = 3;
	monster_types[14].hp = 2;
	monster_types[14].dmg = 2;
	monster_types[14].armor = 2;
	
	monster_types[15].symbol = '2';
	monster_types[15].name = "medium chest";
	monster_types[15].type = 3;
	monster_types[15].hp = 5;
	monster_types[15].dmg = 5;
	monster_types[15].armor = 5;
	
	monster_types[16].symbol = '3';
	monster_types[16].name = "big chest";
	monster_types[16].type = 3;
	monster_types[16].hp = 17;
	monster_types[16].dmg = 6;
	monster_types[16].armor = 19;
	
	monster_types[17].symbol = '4';
	monster_types[17].name = "rare chest";
	monster_types[17].type = 3;
	monster_types[17].hp = 11;
	monster_types[17].dmg = 40;
	monster_types[17].armor = 65;
	
	monster_types[18].symbol = '5';
	monster_types[18].name = "legendary chest";
	monster_types[18].type = 3;
	monster_types[18].hp = 71;
	monster_types[18].dmg = 60;
	monster_types[18].armor = 52;
	
	monster_types[19].symbol = '6';
	monster_types[19].name = "void chest";
	monster_types[19].type = 3;
	monster_types[19].hp = 99;
	monster_types[19].dmg = 99;
	monster_types[19].armor = 99;
	
	monster_types[20].symbol = '7';
	monster_types[20].name = "THE CHEST";
	monster_types[20].type = 3;
	monster_types[20].hp = 150;
	monster_types[20].dmg = 150;
	monster_types[20].armor = 150;
	
	for(int i=0; i<21; i++)
		monster_types[i].c_hp = monster_types[i].hp;
	
	return monster_types;
}

entity * generateLevel(string location_current[], entity * player, int max_map_y, entity * monster_types, int &monster_amount, int difficulty)
{
	//difficulty max = 9
	int room_count = rand()%10+(rand()%(difficulty*2))+6;
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
	
	//enemies amount
	int how_many_neutrals = rand()%3;
	int how_many_chests = rand()%difficulty+2;
	monster_amount = rand()%10+4+rand()%room_count+(rand()%(difficulty*3)+2*difficulty)+how_many_chests+how_many_neutrals;
	entity * monster_list = new entity[monster_amount];
	
	
	int i=0;
	
	for(; i<(monster_amount-how_many_chests-how_many_neutrals); i++)
	{
		//assign monster type
		monster_list[i] = monster_types[rand()%(6)+max((difficulty-3), 0)];
		
		//assign room and location
		room monster_rand_room = rooms[(rand()%(room_count-1))+1];
		monster_list[i].x = monster_rand_room.x + (rand()%(monster_rand_room.x_size-1))+1;
		monster_list[i].y = monster_rand_room.y + (rand()%(monster_rand_room.y_size-1))+1;
		
		while(location_current[monster_list[i].y][monster_list[i].x]!='.')
		{
			monster_rand_room = rooms[(rand()%(room_count-1))+1];
			monster_list[i].x = monster_rand_room.x + (rand()%(monster_rand_room.x_size-1))+1;
			monster_list[i].y = monster_rand_room.y + (rand()%(monster_rand_room.y_size-1))+1;
		}
		
		location_current[monster_list[i].y][monster_list[i].x] = monster_list[i].symbol;
		

	}
	
	//generate chests
	for(; i<(monster_amount-how_many_neutrals); i++)
	{
		//assign chest type
		monster_list[i] = monster_types[max(rand()%difficulty+14-2,14)];
		
		//assign room and location
		room monster_rand_room = rooms[(rand()%(room_count-1))+1];
		monster_list[i].x = monster_rand_room.x + (rand()%(monster_rand_room.x_size-1))+1;
		monster_list[i].y = monster_rand_room.y + (rand()%(monster_rand_room.y_size-1))+1;
		
		while(location_current[monster_list[i].y][monster_list[i].x]!='.')
		{
			monster_rand_room = rooms[(rand()%(room_count-1))+1];
			monster_list[i].x = monster_rand_room.x + (rand()%(monster_rand_room.x_size-1))+1;
			monster_list[i].y = monster_rand_room.y + (rand()%(monster_rand_room.y_size-1))+1;
		}
		
		location_current[monster_list[i].y][monster_list[i].x] = monster_list[i].symbol;
	}
	
	//generate neutrals
	if(difficulty<3)
	{
		for(; i<monster_amount; i++)
		{
			//assign neutral type
			monster_list[i] = monster_types[12];
			
			//assign room and location
			room monster_rand_room = rooms[(rand()%(room_count-1))+1];
			monster_list[i].x = monster_rand_room.x + (rand()%(monster_rand_room.x_size-1))+1;
			monster_list[i].y = monster_rand_room.y + (rand()%(monster_rand_room.y_size-1))+1;
			
			while(location_current[monster_list[i].y][monster_list[i].x]!='.')
			{
				monster_rand_room = rooms[(rand()%(room_count-1))+1];
				monster_list[i].x = monster_rand_room.x + (rand()%(monster_rand_room.x_size-1))+1;
				monster_list[i].y = monster_rand_room.y + (rand()%(monster_rand_room.y_size-1))+1;
			}
			
			location_current[monster_list[i].y][monster_list[i].x] = monster_list[i].symbol;
		}
	}
	else if(difficulty>=3)
	{
		for(; i<monster_amount; i++)
		{
			//assign neutral type
			monster_list[i] = monster_types[13];
			
			//assign room and location
			room monster_rand_room = rooms[(rand()%(room_count-1))+1];
			monster_list[i].x = monster_rand_room.x + (rand()%(monster_rand_room.x_size-1))+1;
			monster_list[i].y = monster_rand_room.y + (rand()%(monster_rand_room.y_size-1))+1;
			
			while(location_current[monster_list[i].y][monster_list[i].x]!='.')
			{
				monster_rand_room = rooms[(rand()%(room_count-1))+1];
				monster_list[i].x = monster_rand_room.x + (rand()%(monster_rand_room.x_size-1))+1;
				monster_list[i].y = monster_rand_room.y + (rand()%(monster_rand_room.y_size-1))+1;
			}
			
			location_current[monster_list[i].y][monster_list[i].x] = monster_list[i].symbol;
		}
	}
	
	
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
	
	 return 0;
}

int drawUserTab(entity * player, int maxy, int maxx, int &inv_select, player_profile * p_profile, char input, int &inv_select_2)
{
	//selection bar
	string menu[4] = {"profile", "inventory", "equipped", "menu"};
	string menu_sh[4] = {"prof", "inv", "eq", "menu"};
	move(maxy-5, 0);
	printw("=");
	if((int) (5+menu[0].length()+menu[1].length()+menu[2].length()+menu[3].length()) > maxx)
	{
		for(int i=0; i<4; i++)
		{
			if(inv_select==i)
				attron(A_REVERSE);
				printw(menu_sh[i].c_str());
			attroff(A_REVERSE);
			
			printw("=");
		}
		for(int i=5+menu_sh[0].length()+menu_sh[1].length()+menu_sh[2].length()+menu_sh[3].length(); i<maxx; i++)
			printw("=");
	}
	else
	{
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
	}
	//tabs
	if(inv_select == 0)
	{
		move(maxy-4, 1);
		printw("name: %s", (p_profile->name).c_str());
		move(maxy-3, 1);
		printw("gold: %d", (p_profile->gold));
		move(maxy-2, 1);
		printw("level: %d", (p_profile->level));
		move(maxy-1, 1);
		printw("exp: %d", (p_profile->exp));
		
		move(maxy-4, 6+p_profile->name.length()+5);
		printw("health: %d", (player->hp));
		move(maxy-3, 6+p_profile->name.length()+5);
		printw("dmg: %d", (player->dmg));
		move(maxy-2, 6+p_profile->name.length()+5);
		printw("armor: %d", (player->armor));
	}
	if(inv_select == 1)
	{
		//inventory
		move(maxy-1, 1);
		printw("  <-- Q");
		move(maxy-1, maxx/2-4);
		printw("^ SPACE ^");
		move(maxy-1, maxx-7);
		printw("E -->  ");
		
		if(input=='e' || input == 'E')
		{
			if(inv_select_2 < p_profile->item_amount-1) inv_select_2++;
		}
		if(input=='q' || input == 'Q')
		{
			if(inv_select_2 > 0) inv_select_2--;
		}
		if(input==' ' || input == ' ')
		{
			inv_select = 5;
			clear();
			//input = '1';
			//drawFullInventory(input, p_profile, player, inv_select, maxy, maxx);
		}
		//how many items can fit
		int inv_item_max = (maxx)/21;
		int j=0;
		for(int i=inv_select_2; j<inv_item_max; i++)
		{
			if(i == p_profile->item_amount) break;
			move(maxy-4, j*21);
			if((p_profile->inventory[i].name).length() > 16)
			{
				printw(" | %s... |", (p_profile->inventory[i].name).substr(0, 13).c_str());
			}
			else
			{
				printw(" |                  |");
				move(maxy-4, j*21+3);
				printw("%s", p_profile->inventory[i].name.c_str());
			}
			
			move(maxy-3, j*21);
			if(p_profile->inventory[i].type == 0) // neutral
			{
				printw(" |                  |");
			}
			if(p_profile->inventory[i].type == 1) // weapon
			{
				printw(" | dmg +            |");
				move(maxy-3, j*21+8);
				printw("%d", p_profile->inventory[i].dmg);
			}
			if(p_profile->inventory[i].type == 2) // armor
			{
				printw(" | hp +             |");
				move(maxy-3, j*21+7);
				printw("%d", p_profile->inventory[i].hp);
			}
			if(p_profile->inventory[i].type == 3) // shield
			{
				printw(" | armor +          |");
				move(maxy-3, j*21+10);
				printw("%d", p_profile->inventory[i].armor);
			}
			
			move(maxy-2, j*21);
			if(j==0)
				printw(" ####################");
			else
				printw(" --------------------");
			j++;
		}
		
	}
	
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

int handleInput(char input, entity * player, string location_current[], int &inv_select, entity * monster_list, int monster_amount, combat_log * c_log)
{
	if(input=='w' || input=='W')
	{
		if(player->y>0)
		{
			if(checkEmpty(player->y-1,player->x, location_current))
				movePlayer(player, player->y-1, player->x,location_current);
			else if(checkMonster(player,player->y-1,player->x, location_current, monster_list, monster_amount, c_log));
		}
		return 1;
	}
	if(input=='s' || input=='S')
	{
		if(player->y<40)
		{
			if(checkEmpty(player->y+1,player->x, location_current))
				movePlayer(player, player->y+1, player->x,location_current);
			else if(checkMonster(player,player->y+1,player->x, location_current, monster_list, monster_amount, c_log));
		}
		return 1;
	}
	if(input=='d' || input=='D')
	{
		if(player->x<159)
		{
			if(checkEmpty(player->y,player->x+1, location_current))
				movePlayer(player, player->y, player->x+1,location_current);
			else if(checkMonster(player, player->y, player->x+1, location_current, monster_list, monster_amount, c_log));
		}
		return 1;
	}
	if(input=='a' || input=='A')
	{
		if(player->x>0)
		{
			if(checkEmpty(player->y,player->x-1, location_current))
				movePlayer(player, player->y, player->x-1,location_current);
			else if(checkMonster(player, player->y, player->x-1, location_current, monster_list, monster_amount, c_log));
		}
		return 1;
	}
	
	if(input=='x' || input=='X') return 1;
	
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
	player->dmg=8;
	player->symbol='@';
	player->y=40;
	player->x=0;
	
	return player;
}

player_profile * profileSetup()
{
	player_profile * profile = new player_profile;
	profile->gold=0;
	profile->level=1;
	profile->exp=0;
	profile->item_amount=0;
	profile->name = "player";
	
	profile->weapon.type = 0;
	profile->shield.type = 0;
	profile->armor.type = 0;
	
	profile->item_amount = 0;
	
	for(int i=0; i<30; i++)
	{
		profile->inventory[i].type = 0;
	}
	
	return profile;
}

int screenSetup()
{
	initscr();
	noecho();
	refresh();
	
	return 0;
}
