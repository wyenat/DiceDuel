/**
 * Rules
 * The game is played on an 8x8 grid. Each player has 8 dice to start with. Players move alternatingly.
 * In each turn a player chooses one of their own dice and rolls it exactly as many cells as the number on top of the die initially showed.
 * The path is a sequence of neighboring cells (diagonals are not considered as neighbors), it's allowed to make turns within the path.
 * It is however not allowed to visit the same cell twice.
 * The path may not cross any cells which are occupied by other dice. The last step can end on an opponent's die which will then be captured and is out of play.
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <utility>

using namespace std;

enum Direction
{
    UP,
    RIGHT,
    DOWN,
    LEFT
} dir;

int getOppositeDirection(int direction)
{
    switch (direction)
    {
    case UP:
        return DOWN;
    case LEFT:
        return RIGHT;
    case RIGHT:
        return LEFT;
    case DOWN:
        return UP;
    }
    return -1;
}

/**
 * This game is weirdly done, a dice is:
 *        [ 1 ]              [Front ]
 *   [ 5 ][ 3 ][ 4 ]  [Left ][Up    ][Right]
 *        [ 6 ]              [Back  ]
 *        [ 2 ]              [Bottom]
 */
class Die
{
private:
    // up, right,
    int faces[6] = {0};
    int position;
    int owner;
    int uniqueID;
    int opposite[7] = {0, 6, 3, 2, 5, 4, 1};

public:
    void
    rotate(int direction);
    int getFaceup() { return faces[0]; };
    int getFacefront() { return faces[1]; };
    int getFaceright() { return faces[5]; };
    int getOwner() { return owner; };
    int getPosition() { return position; };
    int getUniqueID() { return uniqueID; };
    Die(int position, int owner, int up, int front, int bottom, int back, int left, int right);
    Die(int position, int owner, int up, int front, int right);
};

Die::Die(int position, int owner, int up, int front, int bottom, int back, int left, int right)
{
    this->position = position;
    this->owner = owner;
    this->faces[0] = up;
    this->faces[1] = front;
    this->faces[2] = bottom;
    this->faces[3] = back;
    this->faces[4] = left;
    this->faces[5] = right;
    this->uniqueID = position;
}

Die::Die(int position, int owner, int up, int front, int right)
{
    this->position = position;
    this->owner = owner;
    this->faces[0] = up;
    this->faces[1] = front;
    this->faces[2] = opposite[up];
    this->faces[3] = opposite[front];
    this->faces[4] = opposite[right];
    this->faces[5] = right;
    this->uniqueID = position;
}

void rotation(int face[], int pos1, int pos2, int pos3, int pos4)
{
    int temp = face[pos1];
    face[pos1] = face[pos2];
    face[pos2] = face[pos3];
    face[pos3] = face[pos4];
    face[pos4] = temp;
}

void Die::rotate(int direction)
{
    switch (direction)
    {
    case UP:
        position -= 8;
        rotation(faces, 0, 1, 2, 3);
        break;
    case DOWN:
        position += 8;
        rotation(faces, 0, 3, 2, 1);
        break;
    case LEFT:
        position -= 1;
        rotation(faces, 0, 5, 2, 4);
        break;
    case RIGHT:
        position += 1;
        rotation(faces, 0, 4, 2, 5);
        break;
    default:
        break;
    }
}

class Player
{
private:
    map<int, Die *> dice;

public:
    void addDie(Die *d) { dice.insert(make_pair(d->getUniqueID(), d)); };
    void removeDie(int uniqueID) { dice.erase(uniqueID); };
    void showDice();
    int nbDice(){return dice.size();};
    map<int, Die *> getDice(){return dice;};
};

void Player::showDice()
{
    for (const auto &it : dice)
    {
        Die *d = it.second;
        cout << d->getOwner() << ": " << d->getFaceup() << " at " << d->getPosition() << endl;
    }
}

class MoveTree{
    private:
        string current;
        set<int> visited;
        map<string,MoveTree*> sons;
        int position;
    public:
        void setVisited(set<int> v){visited=v;};
        map<string,MoveTree*> getSons(){return sons;};
        string getCurrent(){return current;};
        int getPosition(){return position;};
        set<int> getVisited(){return visited;};
        MoveTree(string move, int pos) {current=move; position=pos;};
        void visit(int pos){visited.insert(pos);};
        void addSon(int direction);
};

void MoveTree::addSon(int direction){
    string cur = current;
    int pos = position;
    switch (direction)
    {
    case UP:
        if (visited.find(pos-8) == visited.end()){
            cur += "U";
            MoveTree *t = new MoveTree(cur, pos-8);
            t->setVisited(visited);
            t->visit(pos-8);
            sons[cur]=t;
        }
        break;
    case DOWN:
        if (visited.find(pos+8) == visited.end()){
                cur += "D";
                MoveTree *t = new MoveTree(cur, pos+8);
                t->setVisited(visited);
                t->visit(pos+8);
                sons[cur]=t;
        }
        break;
    case RIGHT:
        if (visited.find(pos+1) == visited.end()){
                cur += "R";
                MoveTree *t = new MoveTree(cur, pos+1);
                t->setVisited(visited);
                t->visit(pos+1);
                sons[cur]=t;
        }
        break;
    case LEFT:
        if (visited.find(pos-1) == visited.end()){
                cur += "L";
                MoveTree *t = new MoveTree(cur, pos-1);
                t->setVisited(visited);
                t->visit(pos-1);
                sons[cur]=t;
        }
        break;
    default:
        break;
    }
    
}

class StrategyTree{
    private:
        int score;
        vector<string> moves;
        StrategyTree *father;
        map<string, StrategyTree*> sons;
        void cutSon(string move){sons.erase(move);};
    public:
        StrategyTree(string move, StrategyTree *father, int score);    
        void doNotCome();
};

StrategyTree::StrategyTree(string move, StrategyTree *f, int s){
    moves.push_back(move);
    father = f;
    score = s;
}

void StrategyTree::doNotCome(){
    if (father!=nullptr){
        if (father->father != nullptr){
            father->father->cutSon(moves.at(moves.size() - 2));
        }
    }
}

class Board
{
private:
    map<int, Die *> board;
    Player *me;
    Player *adv;
    void getNeighbours(int position, int neighbours[4]);
    void addDice(Die *d);
    void generateAllMoves(Die *d, int length, MoveTree *tree,  vector<string> *moves);
    map<char,string> oppo; 
    map<char, int> direc;

public:
    Board()
    {
        me = new Player();
        adv = new Player();
        oppo['U']="D";
        oppo['D']="U";
        oppo['L']="R";
        oppo['R']="L";
        direc['U']=UP;
        direc['D']=DOWN;
        direc['L']=LEFT;
        direc['R']=RIGHT;
    }
    Board(string state);
    string exportState();
    void showBoard();
    int toNumber(string position);
    string toString(int position);
    Die *rotation(Die *d, int direction);
    void buildTree(int player, StrategyTree *tree, int depth);
    Die  *simulateMove(string move);
    void revertMove(string move, Die *d);
    void testGrid();
    void populate();
    void removeDice(int position);
    void getMoves(int player, map<int, vector<string>> allMoves);
    int isOver();
};

void Board::revertMove(string move, Die *d){
    // Invert move:
    string reverted = toString(d->getPosition()) + " ";
    string inv;
    for (char c: move.substr(3)){
        inv = oppo.at(c) + inv;
    }
    simulateMove(reverted + inv);

    if (d==nullptr){
        return;
    }

    addDice(d);
} 

Die *Board::simulateMove(string move){
    Die *toMove = board.at(toNumber(move.substr(0,2)));
    Die *deleted;
    for (char m: move.substr(3)){
        deleted = rotation(toMove, direc.at(m));
    }
    return deleted;
}

void Board::buildTree(int player, StrategyTree *tree, int depth){
    if (depth==0){
        return;
    }
    map<int, vector<string>> allMoves;
    getMoves(player, allMoves);
    for (auto it: allMoves){
        int position = it.first;
        Die *d= board.at(position);
        vector<string> moves = it.second;
        for (int i=0; i<moves.size();i++){
            string move = moves[i];
            Die *d = simulateMove(move);
            buildTree(player*-1 +1, tree, depth-1);
            revertMove(move, d);
        }
    }

}

Board::Board(string state)
{
    me = new Player();
    adv = new Player();
    for (int i = 0; i < state.length(); i += 6)
    {
        int position = toNumber(state.substr(i, 2));
        int owner = stoi(state.substr(i + 2, 1));
        int top = stoi(state.substr(i + 3, 1));
        int front = stoi(state.substr(i + 4, 1));
        int right = stoi(state.substr(i + 5, 1));
        addDice(new Die(position, owner, top, front, right));
    }
}

string Board::exportState()
{
    string res = "";
    for (auto it : board)
    {
        Die *d = it.second;
        res += toString(d->getPosition()) + to_string(d->getOwner()) + to_string(d->getFaceup()) + to_string(d->getFacefront()) + to_string(d->getFaceright());
    }
    return res;
}

void Board::removeDice(int position)
{
    Die *d = board.at(position);
    board.erase(position);
    if (d->getOwner() == 0)
    {
        me->removeDie(d->getUniqueID());
    }
    else if (d->getOwner() == 1)
    {
        adv->removeDie(d->getUniqueID());
    }
}

int Board::isOver(){
    if (me->nbDice()==0){
        return -1;
    } else if (adv->nbDice()==0){
        return 1;
    } 
    return 0;
}

void Board::addDice(Die *d)
{
    cout << "Adding dice for " << d->getOwner() << " at " << toString(d->getPosition()) << endl;
    this->board[d->getPosition()] = d;
    if (d->getOwner() == 0)
    {
        this->me->addDie(d);
    }
    else if (d->getOwner() == 1)
    {
        this->adv->addDie(d);
    }
}

Die *Board::rotation(Die *d, int direction)
{
    int tmpPosition = d->getPosition();
    d->rotate(direction);
    if (this->board.find(d->getPosition()) != board.end())
    {
        Die *col = this->board[d->getPosition()];
        if (d->getOwner() == col->getOwner())
        {
            cout << "Tried to eat yourself at " << toString(col->getPosition()) << "! Owner is " << d->getOwner() << endl;
        }
        else
        {
            if (col->getOwner() != d->getOwner())
            {
                Die *advDie = board.at(col->getPosition());
                this->removeDice(col->getPosition());
                this->board[col->getPosition()] = d;
                this->board.erase(tmpPosition);
                return advDie;
            }
        }
    }
    else
    {
        this->board[d->getPosition()] = d;
        this->board.erase(tmpPosition);
    }
    return nullptr;
}

void Board::getNeighbours(int position, int neighbours[4]){
    if (position-8 < 0){
        neighbours[UP] =2;
    } else if (board.find(position-8)!=board.end()){
        neighbours[UP] = board.at(position-8)->getOwner();
    }

    if (position+8 > 63){
        neighbours[DOWN] =2;
    } else if (board.find(position+8)!=board.end()){
        neighbours[DOWN] = board.at(position+8)->getOwner();
    }

    if (position%8==7){
        neighbours[RIGHT] =2;
    } else if (board.find(position+1)!=board.end()){
        neighbours[RIGHT] = board.at(position+1)->getOwner();
    }

     if (position%8 == 0){
        neighbours[LEFT] =2;
    } else if (board.find(position-1)!=board.end()){
        neighbours[LEFT] = board.at(position-1)->getOwner();
    }
}

void Board::generateAllMoves(Die *d, int length, MoveTree *tree, vector<string> *moves){
    if (length == 0){
        moves->push_back(tree->getCurrent());
        set<int> v = tree->getVisited();
        return;
    }
    int neighbours[4] = {-1,-1,-1,-1};
    int player = d->getOwner();
    getNeighbours(tree->getPosition(), neighbours);
    for (int dir=0; dir<4; dir++){
        if (neighbours[dir]==-1){
            tree->addSon(dir);
        } else if (length==1){
            if (neighbours[dir]!=2 and neighbours[dir]!=player){
                tree ->addSon(dir);
            }
        }
    }

    for (auto it: tree->getSons()){
        generateAllMoves(d, length-1, it.second, moves);
    }
}

void Board::getMoves(int player, map<int, vector<string>> allMoves){
    map<int, Die *> iter;
    if (player==0){
        iter = me->getDice();
    } else {
        iter = adv->getDice();
    }
    for (auto it: iter){
        Die *d = it.second;
        int length = d->getFaceup();
        cout << "Starting tree for dice at " << to_string(d->getPosition()) << endl; 
        MoveTree *tree = new MoveTree(toString(d->getPosition()) + " ", d->getPosition());
        vector<string> moves;
        generateAllMoves(d, length, tree, &moves);
        allMoves[d->getPosition()] = moves;
        // cout << "For " << length << ", found the following valid moves: " << endl;
        // for (int i=0; i<moves.size(); i++){
        //     cout << toString(d->getPosition()) <<moves[i] << ",  ";
        // }
        // cout << endl;
    }
}

void Board::populate()
{
    Die *d;
    for (int i = 0; i < 64; i++)
    {
        if (i < 3)
        {
            d = new Die(i, 0, 6, 5, 1);
            this->addDice(d);
        }
        else if (i >= 61)
        {
            d = new Die(i, 1, 6, 5, 1);
            this->addDice(d);
        }
    }
}

int Board::toNumber(string position)
{
    return position.at(0) - 65 + 56 - 8 * (position.at(1) - 49);
};

void Board::showBoard()
{
    string res = "  |A B C D E F G H\n";
    res += "--+---------------\n";
    for (int i = 0; i < 8; i++)
    {
        res += 56 - i;
        res += " |";
        for (int j = 0; j < 8; j++)
        {
            if (this->board.find(i * 8 + j) != this->board.end())
            {
                Die *d = this->board[i * 8 + j];
                res += to_string(d->getFaceup());
                res += " ";
            }
            else
            {
                res += "  ";
            }
        }
        res += "\n";
    }
    cout << res << endl;
}

string Board::toString(int position)
{
    string pos = "__";
    pos[1] = 56 - position / 8;
    pos[0] = position % 8 + 65;
    return pos;
}

void Board::testGrid()
{
    string pos;
    int num;
    cout << "Testing grid...\n"
         << endl;
    for (int i = 0; i < 64; i++)
    {
        pos = toString(i);
        num = toNumber(pos);
        cout << "[" << i << "] " << pos << " : " << num << endl;
    }

    cout << "\nTesting population..." << endl;
    populate();
    showBoard();

    cout << "\nTesting rotation..." << endl;
    Die *d = this->board[1];
    cout << "\nMoving DOWN" << endl;
    rotation(d, DOWN);
    showBoard();
    cout << "\nMoving DOWN" << endl;
    rotation(d, DOWN);
    showBoard();
    cout << "\nMoving RIGHT" << endl;
    rotation(d, RIGHT);
    showBoard();
    cout << "\nMoving RIGHT" << endl;
    rotation(d, RIGHT);
    showBoard();
    cout << "\nMoving LEFT" << endl;
    rotation(d, LEFT);
    showBoard();
    cout << "\nMoving UP" << endl;
    rotation(d, UP);
    showBoard();

    cout << "\nTesting Players..." << endl;
    cout << "My dice" << endl;
    me->showDice();
    cout << "Adv's dice" << endl;
    adv->showDice();
    cout << "Eating adv's dice" << endl;
    d = this->board[10];
    string move = toString(d->getPosition()) + " DDDRDRRDD";
    cout << "Doing move " << move << endl;
    Die *eaten = simulateMove(toString(d->getPosition()) + " DDDRDRRDD");
    cout << "Die " << toString(eaten->getPosition()) << " of " << eaten->getOwner() << " was eaten." << endl;
    showBoard();
    adv->showDice();

    cout << "\nTesting reverting" << endl;
    revertMove(move, eaten);
    showBoard();
    adv->showDice();

    cout << "\nTesting Tree..." << endl;
    cout << "Current board export is:" << endl;
    cout << exportState() << endl;
    
    cout << "\nTesting moves..." << endl;
}

using namespace std;

int main()
{
    // game loop
    Board b;
    b.testGrid();
    // while (1) {
    //     int diceCount;
    //     // cin >> diceCount; cin.ignore();
    //     // for (int i = 0; i < diceCount; i++) {
    //     //     int owner;
    //     //     string cell;
    //     //     int top;
    //     //     int front;
    //     //     int bottom;
    //     //     int back;
    //     //     int left;
    //     //     int right;
    //     //     cin >> owner >> cell >> top >> front >> bottom >> back >> left >> right; cin.ignore();
    //     // }

    //     // Write an action using cout. DON'T FORGET THE "<< endl"
    //     // To debug: cerr << "Debug messages..." << endl;

    //     cout << "A1 URUL" << endl;
    // }
}