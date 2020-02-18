#include <vector> //c++ library for vectors
#include <iostream> //standard input/output library of c++
#include <ctime> //c++ library for timing
using namespace std;
class point;//forward declaration
class player;//forward declaration
class hexg;//forward declaration
class machine;//forward declaration
bool simulate(int , int );//forward declaration
typedef enum coverage{NONE, RED, BLUE} coverage; //this will be used for deciding which palyer owns which point
typedef vector<point*> vp; //instead of writing "vector< vector<point> >" all the time, i'll just type "vp"
typedef vector<int> vi; //instead of writing "vector<int>" all the time, i'll just type "vi"
inline void itest(int i){cout<<"test "<<i<<endl;};

class point //this is the data type that'll be used as our board game's squares
{
private:
	int x,y;
	coverage cov; //this shows which player owns this position
public:
	friend bool check(vp , point*); //friendly access declaration
	friend bool move(int dimension, point& , point& ); //friendly access declaration
	friend bool isLegal(int , player& ); //friendly access declaration
	friend bool gameOver(hexg , player& ); //friendly access declaration
	friend ostream& operator<<(ostream& , point& ); //friendly access declaration
	point(int x, int y):x(x),y(y),cov(NONE){}; //constructor for point
	~point(){}; //destructor for point
	void setCoverage(int k, int l, coverage c) //this function sets this point under a player's colour (coverage c)
	{
		if(k==x && l==y)
			cov =c;
	}
	coverage getCoverage() //this function returns the coverage of the point
	{
		return cov;
	}
};
class player //this is the data type that 'll be used as the player
{
protected:
	vp path; //vector containing the player's path
	coverage colour; 
	int x,y;
public:
	friend class hexg; //friendly access declaration
	friend bool isLegal(point& ); //friendly access declaration
	friend class point; //friendly access declaration
	friend bool gameOver(hexg , player& ); //friendly access declaration
	friend int simulate(int , int , hexg* ); //friendly access declaration
	player(){} //constructor
	~player(){} //destructor
	virtual void chooseMove() //this function is used for making the next move of each palyer
	{ //obviously it will be changed for the AI, so it is virtual
		cout<<"Make a move."<<endl;
		cout<<"First, choose horizontal axis:\t";
		cin>>x; 
		cout<<"Now choose the vertical axis:\t";
		cin>>y;
		cout<<endl;
	}
	void turn(coverage c) //this is used for allocating the symbols according to the turn of the players (first or second)
	{
		colour=c;
	}
	coverage getTurn() //this is used for checking the player's turn
	{
		return colour;
	}
};

class machine: public player //publicly derived class
{
private:
	int chosen1,chosen2; //temporary coordinates used for simulation
	hexg* base; //a pointer to the class that will be conatining this class
public:
	friend void setCoverage(); //friendly access declaration
	friend class hexg; //friendly access declaration
	void setBase(hexg* g) //set the value of pointer base
	{
			base=g;
	}
	void chooseMove(int dimension, vector< vp > board) //this chooses the move for AI
	{
		dimension++; //we offset the dimension by 1 to get right correspondence in the game's board
		cout<<"Hmm, interesting.. Let me think.."<<endl;
		vector< vi > odds; //two dimensional vector that conatains the odds
		for(int i=0; i<dimension; i++) //create a two-dimensional matrix with pointers to points
		{
			vi row; //a single dimensional matrix, only to be used for the initialization of odds
			for(int j=0; j<dimension; j++)
			{
				if(!i) //enumerate the nodes at the top and the leftmost of the graph
				{
					row.push_back(j-1);
				}
				else if(!j) //enumerate the nodes at the top and the leftmost of the graph
					{
						row.push_back(i-1);
					}
					else
					{
						row.push_back(NULL); //initialize every odd at the NULL value
					}
			}
			odds.push_back(row);
		}
		odds[0][0]=99; //of no importance
		time_t end = time(NULL) + 2; //set the time duration at 1 minute
		while(time(NULL) <= end)
		{
			for(int i=1; i<odds.size(); i++)
			{
				for(int j=1; j<odds.size(); j++)
				{
					if(isLegal(*(board[i-1][j-1]))) //if the move is plausible
					{
						odds[i][j] += simulate(i,j,base);
					}
					else //if not
					{
						odds[i][j]=0;
					}
				}
			}
		}
		chosen1 = 1; //initialization
		chosen2 = 1; //initialization
		for(int i=1; i<odds.size(); i++)
		{
			for(int j=1; j<odds.size(); j++)
			{
				if(odds[chosen1][chosen2]<odds[i][j] && isLegal(*(board[i-1][j-1])))
				{// use the plausible move with the highest success rate
					chosen1 = i;
					chosen2 = j;
				}
			}
		}
		chosen1--; //offset like above
		chosen2--; //offset like above
	}
};
void getTurn(player& p, machine& pc) //this function lets the player decide if he's playing first
{
	char ch;
	cout<<"Welcome to the game.! Do you want to play first? (y/n)"<<endl;
	cin>>ch;
	while(ch!='y' && ch!='n') //just checking for right input
	{
		cout<<"I didn't quite get that. Please type y for yes or n for no."<<endl;
		cin>>ch;
	}
	if(ch=='y')
	{
		p.turn(RED);
		pc.turn(BLUE);
	}
	else if (ch=='n')
	{
		p.turn(BLUE);
		pc.turn(RED);
	}
}
class hexg //this data type will be used as our game
{
private:
	vector< vp > board;
	int dimension;
public:
	friend bool gameOver(hexg , player& ); //friendly access declaration
	friend bool isLegal(point& ); //friendly access declaration
	friend int simulate(int , int , hexg* );
	player pl; //human player
	machine pc; //AI
	hexg() //constructor
	{
		getInfo(); //get the needed data to set the game
		for(int i=0; i<dimension; i++) //create a two-dimensional matrix with pointers to points
		{
			vp row;
			for(int j=0; j<dimension; j++)
			{
				row.push_back(new point(i,j));
			}
			board.push_back(row);
		}
		getTurn(pl,pc); //ask the player if he wants to play first
		pc.setBase(this);
	}
	hexg(hexg& a):board(a.board), dimension(a.dimension), pl(a.pl), pc(a.pc){}; //copy constructor
	~hexg(){} //destructor
	inline void getInfo() //this function acquires the data needed to set the dimension of the board
	{
		cout<<"Please select the dimensions of the game board:\t";
		cin>>dimension; //this is a square board so we only need one dimension
	}
	void printBoard() //this function shows the board on screen
	{
		for(int i=0; i<board.size(); i++)
		{
			cout<<"  "<<i;
		}
		cout<<endl;
		for(int i=0; i<board.size(); i++)
		{
			for(int space=0; space<i; space++)
			{
				cout<<" ";
			}
			if(i!=board.size())
			{
				cout<<i;
			}
			for(int j=0; j<board.size(); j++)
			{
				if(j!=board.size()-1)
				{
					cout<<*board[i][j];
				}
				else if(j==board.size()-1)
				{
					cout<<*board[i][j]<<endl;
				}
			}
		}
	}
	void setCoverage() //set the colour of the points
	{
		if(pc.getTurn()==RED)
		{
			pc.chooseMove(dimension, board);
			cout<<"next pc move = ("<<pc.chosen1<<" , "<<pc.chosen2<<")"<<endl;
			(board[pc.chosen1][pc.chosen2])->setCoverage(pc.chosen1,pc.chosen2,pc.colour);
			pc.path.push_back(board[pc.chosen1][pc.chosen2]);
			printBoard();
		}
		if(!(gameOver(*this, this->pc)))
		{
			pl.chooseMove(); //player makes move
			while(!(pl.path.size()==0 || (pl.path.size()!=0 && isLegal(*(board[pl.x][pl.y]))))) //checking the validation of the chosen move
			{
				cout<<"Please select a valid move."<<endl;
				pl.chooseMove(); //if the move isn't permitted, make another
			}
			for(int i=0; i<board.size();i++)
			{
				for(int j=0;j<board.size(); j++)
				{
					if(i==pl.x && j==pl.y)
					{
						(*board[i][j]).setCoverage(i,j,pl.colour); //if the move is permitted set the according colour to the chosen point
						pl.path.push_back(board[i][j]); //add the point to the path of each player
					}
				}
			}
		}
		if(pc.getTurn()==BLUE && !(gameOver(*this, this->pl)))
		{
			pc.chooseMove(dimension, board);
			cout<<"next pc move = ("<<pc.chosen1<<" , "<<pc.chosen2<<")"<<endl;
			(board[pc.chosen1][pc.chosen2])->setCoverage(pc.chosen1,pc.chosen2,pc.colour);
			pc.path.push_back(board[pc.chosen1][pc.chosen2]);
		}
	}
};
int simulate(int x, int y, hexg* test) //this function estimates the success rate of each move
{
	hexg a(*test); //create a temporary copycat of the game
	a.board[x][y]->setCoverage(x,y,RED);
	while(!(gameOver(a,a.pl)) && !(gameOver(a, a.pc)))
	{
		int randChoise1 = (rand()%(a.dimension-1))+1;//random number
		int randChoise2 = (rand()%(a.dimension-1))+1;//random number
		while(!(a.board[randChoise1][randChoise2]->getCoverage()==NONE))
		{
			randChoise1 = (rand()%(a.dimension-1))+1;
			randChoise2 = (rand()%(a.dimension-1))+1;
		}
		a.board[randChoise1][randChoise2]->setCoverage(randChoise1,randChoise2,BLUE);
		a.pl.path.push_back(a.board[randChoise1][randChoise2]);
		randChoise1 = (rand()%(a.dimension-1))+1;
		randChoise2 = (rand()%(a.dimension-1))+1;
		while(!(a.board[randChoise1][randChoise2]->getCoverage()==NONE))
		{
			randChoise1 = (rand()%(a.dimension-1))+1;
			randChoise2 = (rand()%(a.dimension-1))+1;
		}
		a.board[randChoise1][randChoise2]->setCoverage(randChoise1,randChoise2,RED);
		a.pc.path.push_back(a.board[randChoise1][randChoise2]);
	}
	if(gameOver(a,a.pl))
	{
		a.~hexg();
		return 1;
	}
	else if(gameOver(a,a.pc))
	{
		a.~hexg();
		return 0;
	}
};
ostream& operator<<(ostream& out, point& p) //overload the << operator to show the board in the right form
{ 
	if(p.getCoverage()==NONE)
	{
		out<<" . "; //if this point is free
	}
	else if(p.getCoverage()==RED)
	{
		out<<" X "; //if this is occupied
	}
	else
	{
		out<<" O "; //if this is occupied
	}
	return out;
};

bool isLegal(point& p) //checks if the point is free
{
	if(p.getCoverage()==0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

bool move(int dimension, point& p, point& q) //this function checks if the chosen move is legal
{
	dimension--; //because the board starts from 0, the last point will be dimension-1
	//below are some fixed cases that may occur while checking (I named in the order I thought of them)
	bool first = ((p.x==q.x) && ((p.y==(q.y)-1) || (p.y==(q.y)+1)));
	bool second = ((p.x==((q.x)-1)) && ((p.y==(q.y)) || (p.y==(q.y)+1)));
	bool third = ((p.x==((q.x)+1)) && ((p.y==q.y) || (p.y==(q.y)-1)));
	bool fourth = ((p.x==q.x) && (p.y==(q.y)-1));
	bool fifth = ((p.x==q.x) && (p.y==(q.y)+1));
	bool sixth = ((p.x==((q.x)-1)) && (p.y==q.y));
	bool seventh = ((p.x==((q.x)-1)) && (p.y==(q.y)+1));
	bool eigth = ((p.x==((q.x)+1)) && (p.y==q.y));
	bool ninth = ((p.x==((q.x)+1)) && (p.y==(q.y)-1));

	if((q.x==0 && q.y==0) && ((p.x==(q.x) && (p.y==(q.y)+1)) || (p.x==(q.x)+1 && (p.y==(q.y)))))
	{ 
		return 1; //special point(0,0) has 2 edges
	}
	else if((q.x==0 && q.y==dimension) && ((p.x==(q.x) && (p.y==(q.y)-1)) || (p.x==((q.x)+1) && (p.y==(q.y)))))
	{
		return 1; //special point(0,dimension-1) has 2 edges
	}
	else if((q.x==dimension && q.y==0) && ((p.x==(q.x) && (p.y==(q.y)+1)) || (p.x==(q.x)-1 && (p.y==(q.y)))))
	{
		return 1; //special point(dimension-1,0) has 2 edges
	}
	else if((q.x==dimension && q.y==dimension) && ((p.x==(q.x) && (p.y==(q.y)-1)) || (p.x==((q.x)-1) && (p.y==(q.y)))))
	{
		return 1; //special point(dimension-1,dimension-1) has 2 edges
	}
	else if((q.x==0 && q.y!=0 && q.y!=dimension) && (first || third))
	{
		return 1; //special points in the x=0 axis have 4 edges
	}
	else if((q.x==dimension && q.y!=0 && q.y!=dimension) && (first || second))
	{
		return 1; //special points in the x=dimension-1 axis have 4 edges
	}
	else if((q.y==0 && q.x!=0 && q.x!=dimension) && (fifth || eigth || sixth || seventh))
	{
		return 1; //special points in the y=0 axis have 4 edges
	} 
	else if((q.y==dimension && q.x!=0 && q.x!=dimension) && (fourth || sixth || eigth || ninth))
	{
		return 1; //special points in the y=dimension-1 axis have 4 edges
	} 
	else if((q.x!=0 && q.x!=dimension && q.y!=0 && q.y!=dimension) && (first || second || third))
	{
		return 1; //every other point has 6 edges
	}
	else
	{
		return 0; //every other chosen move is illegal
	}
};

bool check(vp temp, point* next) //checks if a point is in the closed set for the path
{
	for(vp::iterator k=temp.begin(); k!=temp.end(); k++)
	{
		if(((*k)->x)==((*next).x) && ((*k)->y)==((*next).y))
		{
			return 1;
		}
	}
	return 0;
};

bool gameOver(hexg g, player& pl) //this function checks if there is a winner
{
	if(pl.colour==RED)
	{
		int counter=0;
		int counter2=0;
		vp temp;
		vp::iterator base;
		for(vp::iterator a=pl.path.begin(); a!=pl.path.end(); a++)
		{
			if(((*a)->y==g.dimension-1))
			{
				for(vp::iterator p=pl.path.begin(); p!=pl.path.end(); p++)
				{
					if(((*p)->y==0))
					{
						temp.push_back(*p);
						base=p;
						while(counter<pl.path.size() && counter2<pl.path.size())
						{
							counter2++;
							for(vp::iterator q=pl.path.begin(); q!=pl.path.end(); q++)
							{	
								counter++;
								if((move(g.dimension,*(*base),*(*q))) && !check(temp, *q))
								{
									base=q;
									temp.push_back(*base);
									counter=0;
								}
								if(((*base)->y)==(g.dimension-1))
								{
									return 1;
								}
							}
						}
					}
				}
			}
		return 0;
		}
	}
	else if(pl.colour==BLUE) //the same as above for the second player
	{
		int counter=0;
		int counter2=0;
		vp temp;
		vp::iterator base;
		for(vp::iterator a=pl.path.begin(); a!=pl.path.end(); a++)
		{
			if(((*a)->x==g.dimension-1))
			{
				for(vp::iterator p=pl.path.begin(); p!=pl.path.end(); p++)
				{
					if(((*p)->x==0))
					{
						temp.push_back(*p);
						base=p;
						while(counter<pl.path.size() && counter2<pl.path.size())
						{
							counter2++;
							for(vp::iterator q=pl.path.begin(); q!=pl.path.end(); q++)
							{	
								counter++;
								if((move(g.dimension,*(*base),*(*q))) && !check(temp, *q))
								{
									base=q;
									temp.push_back(*base);
									counter=0;
								}
								if(((*base)->x)==(g.dimension-1))
								{
									return 1;
								}
							}
						}
					}
				}
			}
		}
		return 0;
	}
};

int main()
{
	srand(time(NULL));
	int round=0; //just a counter for the duration of the game
	hexg game; //create a game
	game.printBoard(); //initialize the first round
	game.setCoverage(); //initialize the first round
	round++; //initialize the first round
	while(!(gameOver(game,game.pl)) && !(gameOver(game, game.pc))) //auto for every other round 
	{
		game.printBoard();
		game.setCoverage();
		round ++;
	}
	game.printBoard();
	cout<<"GAME OVER!"<<endl;
	cout<<"After "<<round<<" rounds."<<endl;
	return 0;
}