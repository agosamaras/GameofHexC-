#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string> 
using namespace std;

// implements an undirected graph with positive edge costs
// using an adjacency matrix representation
class Graph
{
    private:
    int V, E;
    const double NOT_CONNECTED;
    vector< vector<double> > adj_matrix;

    public:
    // constructor for an initially unconnected graph with the given number of vertices
    Graph(int V) :
        V(V), E(0),
        NOT_CONNECTED(-10.0),  // -10.0 signifies a missing edge
        adj_matrix(vector< vector<double> >(V, vector<double>(V, NOT_CONNECTED))) {}

    // copy constructor
    Graph(const Graph& other) :
        V(other.V), E(other.E),
        NOT_CONNECTED(other.NOT_CONNECTED),
        adj_matrix(other.adj_matrix) {}

    // constructor to initialize the graph from a file
    Graph(string filename) : E(0), NOT_CONNECTED(-10.0)
    {
        // if we could not open the file exit with an error message
        ifstream file(filename.c_str());
        if (!file.is_open())
        {
            cout << "Unable to open " << filename << endl;
            exit(EXIT_FAILURE);
        }

        // read the number of vertices and initialize the adjacency matrix
        file >> V;
        adj_matrix = vector< vector<double> >(V, vector<double>(V, NOT_CONNECTED));

        // read and add the edges of the graph
        int v, w;
        double cost;
        while (file >> v >> w >> cost)
            add(v,w,cost);

        // close the file
        file.close();
    }

    // getters for the vertices and edges
    inline int getV() { return V; }
    inline int getE() { return E; }

    // returns a vector containing all the vertices in the graph
    vector<int> getVlist()
    {
        // we name the vertices 0 through N so we just return an array containing that
        vector<int> v;
        for (int i = 0; i < V; i++)
            v.push_back(i);
        return v;
    }

    // returns true if an edge between v1 and v2 exists, false otherwise
    inline bool adjacent(int v1, int v2)
    {
        return adj_matrix[v1][v2] > -1.0; // non existend edges are stored as -10.0
    }

    // returns a list containing all the ID's of the neighbors of the given node
    vector<int> neighbors(int v)
    {
        vector<int> vect;
        for(int i = 0; i < V; i++)
            if (adjacent(v, i))
                vect.push_back(i);
        return vect;
    }

    // adds an edge from v1 to v2 if one does not already exist
    // if a cost is provided that will be the new cost of the edge
    // negative costs will be set to the default value of 1.0
    inline void add(int v1, int v2, double cost)
    {
        if (!adjacent(v1,v2)) E++;
        if (cost < 0)
            adj_matrix[v1][v2] = adj_matrix[v2][v1] = 1.0;
        else
            adj_matrix[v1][v2] = adj_matrix[v2][v1] = cost;
    }
    inline void add(int v1, int v2)
    {
        if (!adjacent(v1,v2)) E++;
        adj_matrix[v1][v2] = adj_matrix[v2][v1] = 1.0;
    }

    // removes the edge from v1 to v2 if it exists
    inline void remove(int v1, int v2)
    {
        if (adjacent(v1,v2)) E--;
        adj_matrix[v1][v2] = adj_matrix[v2][v1] = NOT_CONNECTED;
    }

    // returns the cost of the edge that connects v1 and v2
    // it will return a negative value if they are not connected
    inline double get_cost(int v1, int v2)
    {
        return adj_matrix[v1][v2];
    }
};


// enum for the three possible states of a hex position: red, blue or empty
enum class Color : short { NONE, RED, BLUE };

// << operator definition to be able to print the colors
inline ostream& operator<<(ostream& out, Color c)
{
    switch(c)
    {
        case Color::BLUE : out << "O"; break;
        case Color::RED  : out << "X"; break;
        default : out << ".";
    }
    return out;
}

// == operator for the colors to be able to compare them
inline bool operator==(Color c1, Color c2)
{
    return static_cast<int>(c1) == static_cast<int>(c2);
}


// a graph that stores additional color information for its nodes
// also implements monochromatic path finding
class ColoredGraph : public Graph
{
    protected:
    vector<Color> colors; // store the colors of the nodes

    private:
    vector<bool> visited; // the nodes that have been visited for the path finding

    // depth first search to find out if two nodes are monochromatically connected
    bool dfs(int start, int end)
    {
        for (int n : neighbors(start))
            // only consider unvisited neighbors with the same color
            if (int(getColor(start)) == int(getColor(n)) && !visited[n])
            {
                visited[n] = true;            // mark as visited
                if (n == end) return true;    // we reached the end
                if (dfs(n, end)) return true; // a recursive call succeeded
            }

        return false; // we didn't find it
    }

    public:
    // initialize a graph with no colors
    ColoredGraph(int size) : Graph(size), colors(size, Color::NONE) {}

    // copy constructor
    ColoredGraph(const ColoredGraph& other) : Graph(other), colors(other.colors) {}

    // get the color of a node
    inline Color getColor(int index) { return colors[index]; }

    // set the color of a node
    inline void setColor(int index, Color c) { colors[index] = c; }

    // are the two nodes connected with a monochromatic path?
    bool colorConnected(int start, int end)
    {
        // no monochromatic path can exist between nodes of different colors    
        if (getColor(start) != getColor(end)) return false;

        // initially only the starting node is visited
        visited = vector<bool>(getV(), false);
        visited[start] = true;

        // run the depth first search and return the result
        return dfs(start, end);
    }
};


// the representation of a hexboard using a graph
class HexBoard : public ColoredGraph
{
    private:
    const int size;       // the dimension of the board
    int numEmpty;         // the number of empty positions

    // the position of the node that represents the (x,y) hex
    // node zero is the bottom left node, and we store the nodes by row
    // ^ y
    //  \             position = y * size + x
    //   \      x
    //    0----->
    inline int pos(int x, int y) { return y * size + x; }

    // place a blue or red hex on the board
    // returns false and does nothing on an invalid move, returns true otherwise
    bool place(int x, int y, Color c)
    {
        if (x < 0 || x >= size || y < 0 || y >= size || // out of bounds checks
            int(c) == int(Color::NONE) ||                         // must play blue or red
            getColor(pos(x,y)) != Color::NONE)          // the position must be empty
            return false; // invalid move
        
        setColor(pos(x,y), c);
        numEmpty--;
        return true;      // valid move
    }

    // check if someone has won and return his color
    inline Color getWinner()
    {
        if (colorConnected(size * size, size * size + 1))          // check blue master nodes
            return Color::BLUE;
        else if (colorConnected(size * size + 2, size * size + 3)) // check red master nodes
            return Color::RED;
        else
            return Color::NONE;
    }

    // plays a move for the given player using a Monte Carlo AI agent with 1000 trials
    void playAIMove(Color AIColor)
    {
        vector<int> evaluations(size * size, -1); // the results of the evaluations
        Color playerColor = (int(AIColor) == int(Color::RED)) ? Color::BLUE : Color::RED;

        // loop through all positions
        for (int x = 0; x < size; x++)
            for (int y = 0; y < size; y++)
            {
                // skip occupied positions
                if (getColor(pos(x,y)) != Color::NONE) continue;

                // count the number of won games for that position
                evaluations[pos(x,y)] = 0;
                for (int i = 0; i < 1000; i++)
                {
                    // make a copy of the board and play the x,y position
                    HexBoard temp(*this);
                    temp.place(x, y, AIColor);

                    // create an array with the colors to be played and shuffle it
                    vector<Color> moves(temp.numEmpty, AIColor);
                    for (int j = 0, n = temp.numEmpty; j < n; j = j + 2)
                        moves[j] = playerColor;
                    random_shuffle(moves.begin(), moves.end());
                    
                    // place the blocks in the randomly created order
                    int n = 0;
                    for (int j = 0; j < size; j++)
                        for (int k = 0; k < size; k++)
                            if (temp.place(j, k, moves[n])) n++;

                    // check if the AI won and count it
                    if (int(temp.getWinner()) == int(AIColor)) evaluations[pos(x,y)] += 1;
                }
            }

        // get the move with the most wins
        int bestPos = 0;
        int bestNum = evaluations[0];
        for (int i = 1; i < size * size; i++)
            if (evaluations[i] > bestNum)
            {
                bestNum = evaluations[i];
                bestPos = i;
            }

        // play the best position
        place(bestPos % size, bestPos / size, AIColor);
    }

    // print the board
    void print()
    {
        // print top down by row
        for (int y = size - 1; y > 0; y--)
        {
            // print 2*(size-y) spaces at the start of each row
            for (int x = 0; x < 2 * (size - y - 1); x++) cout << " ";
            // print the row number
            cout << setw(3) << left << y;
            // print the colors for the row
            for (int x = 0; x < size - 1; x++) cout << getColor(pos(x,y)) << " - ";
            cout << getColor(pos(size-1,y)) << endl;
            // print the spaces at the start of the intermediate row
            for (int x = 0; x < 2 * (size - y) + 1; x++) cout << " ";
            // print the intermediate row \ / \ /
            for (int x = 0; x < size - 1; x++) cout << " \\ /";
            cout << " \\" << endl;
        }
        // print the 0th row's spaces
        for (int x = 0; x < 2 * size - 2; x++) cout << " ";
        // print the 0th row's number
        cout << "0  ";
        // print the 0th row
        for (int x = 0; x < size - 1; x++) cout << getColor(pos(x,0)) << " - ";
        cout << getColor(pos(size-1,0)) << endl;
        // print the spaces before the column numbers
        for (int x = 0; x < 2 * size - 2; x++) cout << " ";
        // print the column numbers
        cout << "   ";
        for (int x = 0; x < size; x++) cout << setw(4) << left << x;
        cout << endl;
    }
 
    public:
    // initiate a size*size graph to represent the board
    // we will use four additional nodes to help determine if a player has won
    HexBoard(int size) : ColoredGraph(size * size + 4), size(size), numEmpty(size * size)
    {
        // connect all adjacent positions
        for (int x = 0; x < size; x++)
            for (int y = 0; y < size; y++)
            {
                if (x < size-1)               add(pos(x,y), pos(x+1,y));   // right
                if (x > 0)                    add(pos(x,y), pos(x-1,y));   // left
                if (y < size-1)               add(pos(x,y), pos(x,y+1));   // top
                if (y > 0)                    add(pos(x,y), pos(x,y-1));   // bottom
                if (x < size-1 && y < size-1) add(pos(x,y), pos(x+1,y+1)); // top right
                if (x > 0 && y > 0)           add(pos(x,y), pos(x-1,y-1)); // bottom left
            }
        
        // we will add a master node on each side of the board
        // and connect it with all the nodes in the side
        // this way we only have to check if the master nodes are connected
        // to determine if a player has won

        // left and right sides are blue
        setColor(size * size,     Color::BLUE); // left master node
        setColor(size * size + 1, Color::BLUE); // right master node 

        // top and bottom sides are red
        setColor(size * size + 2, Color::RED);  // top master node
        setColor(size * size + 3, Color::RED);  // bottom master node

        // connect the left side with the left master node
        for (int y = 0; y < size; y++)
            add(pos(0,y), size * size);

        // connect the right side with the right master node
        for (int y = 0; y < size; y++)
            add(pos(size-1,y), size * size + 1);

        // connect the top side with the top master node
        for (int x = 0; x < size; x++)
            add(pos(x,size-1), size * size + 2);

        // connect the bottom side with the bottom master node
        for (int x = 0; x < size; x++)
            add(pos(x,0), size * size + 3);
    }
    
    // a copy constructor to duplicate an existing hex board
    HexBoard(const HexBoard& other) : 
        ColoredGraph(other), size(other.size), numEmpty(other.numEmpty) {}

    // play a game of hex with two human players
    void multiPlayer()
    {
        Color nextPlayer = Color::BLUE; // Blue plays first
        Color winner     = Color::NONE; // the winner's color

        // loop until we have a winner
        while (int(winner) == int(Color::NONE))
        {
            // print the board and ask for the next move
            print();
            if (int(nextPlayer) == int(Color::BLUE))
                cout << "Blue player's turn! You must make an \"X\" path from left to right.\n";
            else
                cout << "Red player's turn! You must make an \"O\" path from top to bottom.\n";

            int x, y;
            cout << "Enter a row number    : "; cin >> y;
            cout << "Enter a column number : "; cin >> x;

            // check if the move was valid, check if we have a winner and swap the next player
            if (place(x,y,nextPlayer))
            {
                winner = getWinner();
                nextPlayer = (int(nextPlayer) == int(Color::BLUE)) ? Color::RED : Color::BLUE;
            }
            else
                cout << "That move was illegal!" << endl;
        }

        // print the final board and the final message (there can be no draw in this game)
        print();
        if (int(winner) == int(Color::BLUE))
            cout << "Blue player wins! Congratulations!" << endl;
        else
            cout << "Red player wins! Congratulations!" << endl;
    }

    // play a game of hex versus an AI opponent
    void singlePlayer(bool playersTurn)
    {
        srand(time(NULL));              // seed the random generator
        Color nextPlayer = Color::BLUE; // Blue plays first
        Color winner     = Color::NONE; // the winner's color

        // loop until we have a winner
        while (int(winner) == int(Color::NONE))
        {
            print();
            if (playersTurn) // player's move
            {
                // ask the player for his move
                int x, y;
                cout << "It's your turn! Try to create an \"" << nextPlayer
                     << "\" path from "
                     << ((int(nextPlayer) == int(Color::BLUE)) ? "left to right.\n" : "top to bottom.\n");
                cout << "Enter the row number    : "; cin >> y;
                cout << "Enter the column number : "; cin >> x;

                // do the move and go on to the next player if it was valid
                if (place(x,y,nextPlayer))
                {
                    winner = getWinner();
                    nextPlayer = (int(nextPlayer) == int(Color::BLUE)) ? Color::RED : Color::BLUE;
                    playersTurn = false;
                }
                else
                    cout << "That move was illegal." << endl;
            }
            else // play an AI move and go on to the next player
            {
                cout << "Thinking... " << endl;
                playAIMove(nextPlayer);
                winner = getWinner();
                nextPlayer = (int(nextPlayer) == int(Color::BLUE)) ? Color::RED : Color::BLUE;
                playersTurn = true;
                cout << "Done! " << endl;
            }
        }

        // print the final board and the final message (there can be no draw in this game)
        print();
        if (playersTurn)
            cout << "You lost the game. Better luck next time." << endl;
        else
            cout << "You win! Congratulations!" << endl;
    }
};

// launches a game of hex
int main()
{
    // get the board size from the user and create the board
    int size = 0;
    while (size < 1 || size > 20)
    {
        cout << "Enter the dimension of the hex board (1-20) : ";
        cin >> size;
    }
    HexBoard hex(size);

    // ask if the user wants to play multiplayer or vs AI
    string input;
    cout << "Do you want to play versus an AI opponent? (Yes / No) : ";
    cin >> input;

    if (input[0] == 'N' || input[0] == 'n')
        hex.multiPlayer();
    else // if the user plays vs AI he can choose if he wants to play first
    {
        cout << "Do you want to play first? (Yes / No) : ";
        cin >> input;
        hex.singlePlayer(input[0] == 'Y' || input[0] == 'y');
    }

    return 0;
}