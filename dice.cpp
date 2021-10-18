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
#include <utility>

using namespace std;

enum Direction{UP, RIGHT, DOWN, LEFT} dir;

int getOppositeDirection(int direction){
    switch (direction){
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

class Die{
    private:
        // up, right, 
        int faces[6] = {0};
        int position;
        int owner;
        int remaining;
        int uniqueID;
    public:
        void rotate(int direction);
        int getRemaining(){return remaining;};
        int getFaceup(){return faces[0];};
        int getOwner(){return owner;};
        int getPosition(){return position;};
        int getUniqueID(){return uniqueID;};
        Die(int position, int owner, int up, int front, int bottom, int back, int left, int right);
};

Die::Die(int position, int owner, int up, int front, int bottom, int back, int left, int right){
    this->position = position;
    this->owner = owner;
    this->remaining = up;
    this->faces[0] = up;
    this->faces[1] = front;
    this->faces[2] = bottom;
    this->faces[3] = back;
    this->faces[4] = left;
    this->faces[5] = right;
    this->uniqueID = position;
}

void rotation(int face[], int pos1, int pos2, int pos3, int pos4){
    int temp = face[pos1];
    face[pos1] = face[pos2];
    face[pos2]=face[pos3];
    face[pos3]=face[pos4];
    face[pos4]=temp;
}

void Die::rotate(int direction){
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
    remaining--;
}

class Player{
    private:
        map<int,Die*> dice;
    public:
        void addDie(Die *d){dice.insert(make_pair(d->getUniqueID(),d));};
        void removeDie(int uniqueID){dice.erase(uniqueID);};
        void showDice();
};

void Player::showDice(){
    for (const auto &it: dice){
        Die *d = it.second;
        cout << d->getOwner() << ": " << d->getFaceup() << " at " << d->getPosition() << endl;
    }
}

class Board{
    private:
        Die *board[64];
        Player *me;
        Player *adv;
    public:
        Board(){me=new Player(); adv=new Player();}
        void showBoard();
        int toNumber(string position);
        string toString(int position);
        void addDice(Die *d);
        void testGrid();
        void populate();
        void rotation(int position, int direction);
        void removeDice(int position);
};

void Board::removeDice(int position){
    Die *d = this->board[position];
    if (d->getOwner()==0){
        me->removeDie(d->getUniqueID());
    } else if (d->getOwner()==1){
        adv->removeDie(d->getUniqueID());
    }
}

void Board::addDice(Die *d){
    cout << "Adding dice for " << d->getOwner() << " at " << toString(d->getPosition()) << endl; 
    this->board[d->getPosition()]=d;
    if (d->getOwner()==0){
        this->me->addDie(d);
    } else if (d->getOwner()==1){
        this->adv->addDie(d);
    }
}

void Board::rotation(int position, int direction){
    Die *d = this->board[position];
    int tmpPosition = d->getPosition();
    d->rotate(direction);
    Die *col = this->board[d->getPosition()];
    if (col->getOwner()==-1){
        this->board[d->getPosition()]=d;
        this->board[tmpPosition]=col;
    }
    else {
        if (d->getRemaining() != 0){
            cout << "Cannot cross path with another dice! Still " << d->getRemaining() << " steps and crossed " << col->getOwner() << " at " << toString(col->getPosition()) << endl;
        } else {
            if (d->getOwner()==col->getOwner()){
                cout << "Tried to eat yourself at " << toString(col->getPosition()) << "! Owner is " << d->getOwner() << endl;
            } else {
                if (col->getOwner()!=d->getOwner()){
                    this->removeDice(col->getPosition());
                    this->board[col->getPosition()] = d;
                    this->board[tmpPosition] = new Die(tmpPosition,-1,0,0,0,0,0,0);
                }
            }
        }
    }
}

void Board::populate(){
    Die *d;
    for (int i=0;i<64;i++){
        if (i<16){
            d = new Die(i, 0, 6, 5, 1, 2, 3, 4);
        } else if (i>=48)
        {
            d = new Die(i, 1, 6, 5, 1, 2, 3, 4);
        } else {
            d = new Die(i, -1, 0, 0, 0, 0, 0, 0);
        }
        this->addDice(d);
    }
}

int Board::toNumber(string position)
{
    return position.at(0) - 65 + 56 - 8*(position.at(1)-49);
};

void Board::showBoard(){
    string res = "  |A B C D E F G H\n";
    res       += "--+---------------\n";
    for (int i=0;i<8;i++){
        res += 56 - i;
        res += " |";
        for (int j=0;j<8;j++){
            Die *d = this->board[i*8+j];
            if (d->getOwner()!=-1){
                res+= to_string(d->getFaceup());
            } else {
                res+= " ";
            }
            res+=" ";
        }
        res += "\n";
    }
    cout << res << endl;
}

string Board::toString(int position){
    string pos = "__";
    pos[1] = 56 - position/8;
    pos[0] = position%8 + 65;
    return pos;
}

void Board::testGrid(){
    string pos;
    int num;
    cout << "Testing grid...\n" << endl; 
    for (int i=0; i<64;i++){
        pos=toString(i);
        num=toNumber(pos);
        cout << "[" << i << "] " << pos << " : " << num << endl; 
    }

    cout << "\nTesting population..." << endl;
    populate();
    showBoard();

    cout  << "\nTesting rotation..." << endl;
    Die *d = this->board[10];
    cout  << "\nMoving DOWN" << endl;
    rotation(d->getPosition(), DOWN);
    showBoard();
    cout  << "\nMoving DOWN" << endl;
    rotation(d->getPosition(), DOWN);
    showBoard();
    cout  << "\nMoving RIGHT" << endl;
    rotation(d->getPosition(), RIGHT);
    showBoard();
    cout  << "\nMoving RIGHT" << endl;
    rotation(d->getPosition(), RIGHT);
    showBoard();
    cout  << "\nMoving LEFT" << endl;
    rotation(d->getPosition(), LEFT);
    showBoard();
    cout  << "\nMoving UP" << endl;
    rotation(d->getPosition(), UP);
    showBoard();

    cout << "\nTesting Players..." << endl;
    cout << "My dice" << endl;
    me->showDice();
    cout << "Adv's dice" << endl;
    adv->showDice();
    cout << "Eating adv's dice" << endl;
    d = this->board[15];
    rotation(d->getPosition(), DOWN);
    rotation(d->getPosition(), DOWN);
    rotation(d->getPosition(), DOWN);
    rotation(d->getPosition(), DOWN);
    rotation(d->getPosition(), LEFT);
    rotation(d->getPosition(), DOWN);
    showBoard();
    adv->showDice();
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