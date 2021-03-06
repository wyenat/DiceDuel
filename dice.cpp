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
#include <chrono>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

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
    int nbDice() { return dice.size(); };
    map<int, Die *> getDice() { return dice; };
};

void Player::showDice()
{
    for (const auto &it : dice)
    {
        Die *d = it.second;
        cout << d->getOwner() << ": " << d->getFaceup() << " at " << d->getPosition() << endl;
    }
}

class MoveTree
{
private:
    string current;
    set<int> visited;
    map<string, MoveTree *> sons;
    int position;

public:
    void setVisited(set<int> v) { visited = v; };
    map<string, MoveTree *> getSons() { return sons; };
    string getCurrent() { return current; };
    int getPosition() { return position; };
    set<int> getVisited() { return visited; };
    MoveTree(string move, int pos)
    {
        current = move;
        position = pos;
    };
    void visit(int pos) { visited.insert(pos); };
    void addSon(int direction);
};

void MoveTree::addSon(int direction)
{
    string cur = current;
    int pos = position;
    switch (direction)
    {
    case UP:
        if (visited.find(pos - 8) == visited.end())
        {
            cur += "U";
            MoveTree *t = new MoveTree(cur, pos - 8);
            t->setVisited(visited);
            t->visit(pos - 8);
            sons[cur] = t;
        }
        break;
    case DOWN:
        if (visited.find(pos + 8) == visited.end())
        {
            cur += "D";
            MoveTree *t = new MoveTree(cur, pos + 8);
            t->setVisited(visited);
            t->visit(pos + 8);
            sons[cur] = t;
        }
        break;
    case RIGHT:
        if (visited.find(pos + 1) == visited.end())
        {
            cur += "R";
            MoveTree *t = new MoveTree(cur, pos + 1);
            t->setVisited(visited);
            t->visit(pos + 1);
            sons[cur] = t;
        }
        break;
    case LEFT:
        if (visited.find(pos - 1) == visited.end())
        {
            cur += "L";
            MoveTree *t = new MoveTree(cur, pos - 1);
            t->setVisited(visited);
            t->visit(pos - 1);
            sons[cur] = t;
        }
        break;
    default:
        break;
    }
}

class StrategyTree
{
private:
    vector<string> moves;
    StrategyTree *father;
    map<string, StrategyTree *> sons;
    void cutSon(string move) { sons.erase(move); };
    bool forbiden = false;
    int winner = -1;
    int score = 0;

public:
    void forbid() { forbiden = true; };
    void incrementScore(int inc) { score += inc; };
    void setOnlySon(string move);
    StrategyTree *getBest();
    string getStrMoves();
    void setWinner(int w);
    int getScore() { return score; };
    bool getForbiden() { return forbiden; };
    StrategyTree(StrategyTree *father);
    void doNotCome();
    void setMoves(vector<string> m) { moves = m; };
    StrategyTree *addSon(string move);
    void insertMove(string move) { moves.push_back(move); };
    StrategyTree *getFather() { return father; };
    string prettyPrint(int depth);
    vector<string> getMoves() { return moves; };
};

StrategyTree *StrategyTree::getBest()
{
    int best = -1000000;
    string bestStr = "";
    //cout << "Looking into the " << sons.size() << " sons." << endl;
    for (auto it : sons)
    {
        if (it.second->score > best)
        {
            best = it.second->score;
            bestStr = it.first;
        }
    }
    // cout << "Best move was " << bestStr << " with " << best << endl;
    return sons.at(bestStr);
}

void StrategyTree::setWinner(int w)
{
    // Mark this branch as won.
    winner = w;

    // The idea is to propagate this winner to the upper branch, so the player can make a winning move
    // And propagate to the 2 x upper branch, so the player can avoid a losing move.
    // Exemple:
    // Export is D70531F11321, P0 plays first
    // |-D7 DDDDD
    // 	|-F1 LLU   -> P1 WON
    // So P0 must know it must not play D7 DDDDD because it's a loosing move.
    // The idea is to set the score to this branch to -10000, garanteeing it won't be picked.
    // F1 LLU is the only son of that branch, because it's a win, so P1 doesn't need to bother with a score.

    // if (father != nullptr)
    // {
    //     if (father->father != nullptr)
    //     {
    //         //cout << "For " << ge < tStrMoves() << "'s grandpa, score is " << father->father->score << endl;
    //         if (w == 0)
    //         {
    //             father->incrementScore(-10000);
    //             father->father->incrementScore(1);
    //         }
    //         else if (w == 1)
    //         {
    //             father->incrementScore(10000);
    //             father->father->incrementScore(-1);
    //         }
    //     }
    // }
}

void StrategyTree::setOnlySon(string move)
{
    // cout << "Called set only son with move " << move << " for tree ending with " << getStrMoves() << endl;
    sons.clear();
    addSon(move);
    // cout << "Now, " << getStrMoves() << " only has " << sons.size() << " son." << endl;
};

string StrategyTree::getStrMoves()
{
    string ret = "";
    for (string m : getMoves())
    {
        ret += m + " --> ";
    }
    return ret;
}

StrategyTree *StrategyTree::addSon(string move)
{
    StrategyTree *son = new StrategyTree(this);
    son->setMoves(moves);
    son->insertMove(move);
    sons[move] = son;
    return son;
}

string StrategyTree::prettyPrint(int depth)
{
    string ret = "";
    if (moves.size() == 0)
    {
        ret += "Origin";
    }
    for (auto it : sons)
    {
        string tabs(depth, '\t');
        ret += "\n" + tabs + "|-" + it.first;
        if (forbiden)
        {
            ret += " (F) ";
        }
        if (winner == 0)
        {
            ret += " (W) ";
        }
        else if (winner == 1)
        {
            ret += " (L) ";
        }
        ret += " [" + to_string(it.second->getScore()) + "] ";
        ret += it.second->prettyPrint(depth + 1);
    }
    return ret;
}

StrategyTree::StrategyTree(StrategyTree *f)
{
    father = f;
    score = 0;
}

void StrategyTree::doNotCome()
{
    if (father != nullptr)
    {
        if (father->father != nullptr)
        {
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
    void generateAllMoves(Die *d, int length, MoveTree *tree, vector<string> *moves);
    map<char, string> oppo;
    map<char, int> direc;

public:
    Board()
    {
        me = new Player();
        adv = new Player();
        oppo['U'] = "D";
        oppo['D'] = "U";
        oppo['L'] = "R";
        oppo['R'] = "L";
        direc['U'] = UP;
        direc['D'] = DOWN;
        direc['L'] = LEFT;
        direc['R'] = RIGHT;
    }
    void addDice(Die *d);
    Board(string state);
    string exportState();
    void showBoard();
    int toNumber(string position);
    string toString(int position);
    Die *rotation(Die *d, int direction);
    void buildTree(int player, StrategyTree *tree, int depth);
    Die *simulateMove(string move);
    void revertMove(string move, string currentPos, Die *d);
    void testGrid();
    void populate();
    void removeDice(int position);
    void getMoves(int player, map<int, vector<string>> *allMoves);
    int isOver();
    void testManyTurns();
    int getScore() { return me->nbDice() - adv->nbDice(); };
};

void Board::revertMove(string move, string currentPos, Die *d)
{
    // Invert move:
    string reverted = currentPos + " ";
    string inv;
    for (char c : move.substr(3))
    {
        inv = oppo.at(c) + inv;
    }
    simulateMove(reverted + inv);

    if (d == nullptr)
    {
        return;
    }

    addDice(d);
}

Die *Board::simulateMove(string move)
{
    Die *toMove = board.at(toNumber(move.substr(0, 2)));
    Die *deleted;
    for (char m : move.substr(3))
    {
        deleted = rotation(toMove, direc.at(m));
    }
    return deleted;
}

void Board::buildTree(int player, StrategyTree *tree, int depth)
{
    if (depth == 0)
    {
        return;
    }
    map<int, vector<string>> allMoves;
    getMoves(player, &allMoves);
    for (auto it : allMoves)
    {
        int position = it.first;
        Die *d = board.at(position);
        vector<string> moves = it.second;
        for (int i = 0; i < moves.size(); i++)
        {
            string move = moves[i];
            string tmpPosition = move.substr(0, 2);

            Die *eaten = simulateMove(move);
            int over = isOver();
            tree->setWinner(over);

            // Case 1 : Most common, nobody lost or won with this move.
            // Continue simulating further the branch.
            if (over == -1)
            {
                tree = tree->addSon(move);
                if (eaten != nullptr)
                {
                    tree->incrementScore(eaten->getOwner() * 2 - 1);
                }
                buildTree(player * -1 + 1, tree, depth - 1);
            }

            // Case 2 : With this move, the opponent won.
            // Because he can win, I have to make sure he cannot get the opportunity to play this move,
            // so I do not play the move leading to this position.
            else if (over == 1)
            {
                tree->forbid();
                tree->setOnlySon(move);
                tree->incrementScore(-1000);
                revertMove(move, toString(d->getPosition()), eaten);
                break;
            }

            // Case 3: With this move, I won.
            // No need to explore further, if we arrive to this position I will play this and win.
            // Because this game is symetric, the opponent doesn't want me to arrive to this state either,
            // and won't play the move to come until here.
            else
            {
                // No need to build further this branch, if it gets to this point
                // the player wins.
                tree->forbid();
                tree->setOnlySon(move);
                tree->incrementScore(1000);
                revertMove(move, toString(d->getPosition()), eaten);
                break;
            }
            if (tree->getFather() != nullptr)
            {
                tree = tree->getFather();
            }
            revertMove(move, toString(d->getPosition()), eaten);
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

int Board::isOver()
{
    if (me->nbDice() == 0)
    {
        return 1;
    }
    else if (adv->nbDice() == 0)
    {
        return 0;
    }
    return -1;
}

void Board::addDice(Die *d)
{
    // cout << "Adding dice for " << d->getOwner() << " at " << toString(d->getPosition()) << endl;
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

void Board::getNeighbours(int position, int neighbours[4])
{
    if (position - 8 < 0)
    {
        neighbours[UP] = 2;
    }
    else if (board.find(position - 8) != board.end())
    {
        neighbours[UP] = board.at(position - 8)->getOwner();
    }

    if (position + 8 > 63)
    {
        neighbours[DOWN] = 2;
    }
    else if (board.find(position + 8) != board.end())
    {
        neighbours[DOWN] = board.at(position + 8)->getOwner();
    }

    if (position % 8 == 7)
    {
        neighbours[RIGHT] = 2;
    }
    else if (board.find(position + 1) != board.end())
    {
        neighbours[RIGHT] = board.at(position + 1)->getOwner();
    }

    if (position % 8 == 0)
    {
        neighbours[LEFT] = 2;
    }
    else if (board.find(position - 1) != board.end())
    {
        neighbours[LEFT] = board.at(position - 1)->getOwner();
    }
}

void Board::generateAllMoves(Die *d, int length, MoveTree *tree, vector<string> *moves)
{
    if (length == 0)
    {
        moves->push_back(tree->getCurrent());
        set<int> v = tree->getVisited();
        return;
    }
    int neighbours[4] = {-1, -1, -1, -1};
    int player = d->getOwner();
    getNeighbours(tree->getPosition(), neighbours);
    for (int dir = 0; dir < 4; dir++)
    {
        if (neighbours[dir] == -1)
        {
            tree->addSon(dir);
        }
        else if (length == 1)
        {
            if (neighbours[dir] != 2 and neighbours[dir] != player)
            {
                tree->addSon(dir);
            }
        }
    }

    for (auto it : tree->getSons())
    {
        generateAllMoves(d, length - 1, it.second, moves);
    }
}

void Board::getMoves(int player, map<int, vector<string>> *allMoves)
{
    map<int, Die *> iter;
    if (player == 0)
    {
        iter = me->getDice();
    }
    else
    {
        iter = adv->getDice();
    }
    for (auto it : iter)
    {
        Die *d = it.second;
        int length = d->getFaceup();
        // cout << "Starting tree for dice at " << to_string(d->getPosition()) << endl;
        MoveTree *tree = new MoveTree(toString(d->getPosition()) + " ", d->getPosition());
        vector<string> moves;
        generateAllMoves(d, length, tree, &moves);
        allMoves->insert({d->getPosition(), moves});
    }
}

void Board::populate()
{
    Die *d;
    d = new Die(32, 0, 4, 3, 1);
    this->addDice(d);
    d = new Die(56, 1, 4, 2, 1);
    this->addDice(d);
    d = new Die(57, 1, 6, 3, 2);
    this->addDice(d);
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

    cout << "\nTesting population..." << endl;
    populate();
    showBoard();

    cout << "\nTesting rotation..." << endl;
    Die *d = this->board[32];
    cout << "\nMoving DOWN" << endl;
    rotation(d, DOWN);
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
    d = this->board[32];
    string move = toString(d->getPosition()) + " DDRD";
    cout << "Doing move " << move << endl;
    Die *eaten = simulateMove(move);
    cout << "Die " << toString(eaten->getPosition()) << " of " << eaten->getOwner() << " was eaten." << endl;
    showBoard();
    adv->showDice();

    cout << "\nTesting reverting" << endl;
    revertMove(move, toString(d->getPosition()), eaten);
    showBoard();
    adv->showDice();

    cout << "\nTesting Tree..." << endl;
    cout << "Current board export is:" << endl;
    cout << exportState() << endl;
    StrategyTree *tree = new StrategyTree(nullptr);
    buildTree(0, tree, 2);
    cout << tree->prettyPrint(0);

    cout << "\nTesting best move..." << endl;
    StrategyTree *best = tree->getBest();
    cout << "Best was " << best->getMoves().back() << " with " << best->getScore() << endl;
}

void Board::testManyTurns()
{

    srand(time(0));

    Die *d;
    for (int i = 0; i < 8; i++)
    {
        d = new Die(i, 0, rand() % 6 + 1, rand() % 6 + 1, rand() % 6 + 1);
        this->addDice(d);
    }

    for (int i = 0; i < 8; i++)
    {
        d = new Die(63 - i, 1, rand() % 6 + 1, rand() % 6 + 1, rand() % 6 + 1);
        this->addDice(d);
    }
    showBoard();
    int player = 0;
    chrono::_V2::steady_clock::time_point start;
    chrono::_V2::steady_clock::time_point end;

    while (me->nbDice() != 0 && adv->nbDice() != 0)
    {
        cout << "\nP" << to_string(player) << " plays" << endl;
        start = chrono::steady_clock::now();

        StrategyTree *tree = new StrategyTree(nullptr);

        buildTree(player, tree, 2);
        tree = tree->getBest();
        simulateMove(tree->getMoves().back());
        end = chrono::steady_clock::now();

        // cout << tree->prettyPrint(0) << endl;

        cout << "Elapsed time in milliseconds: "
             << chrono::duration_cast<chrono::milliseconds>(end - start).count()
             << " ms" << endl;

        player = player * -1 + 1;

        showBoard();
        sleep(1);
    }
    cout << "Finished! P0: " << me->nbDice() << ", P1: " << adv->nbDice() << "..." << endl;
}

int main()
{

    // Board b;
    //b.testGrid();
    //b.testManyTurns();

    Die *d;
    while (1)
    {
        Board b;
        int diceCount;
        cin >> diceCount;
        cin.ignore();
        for (int i = 0; i < diceCount; i++)
        {
            int owner;
            string cell;
            int top;
            int front;
            int bottom;
            int back;
            int left;
            int right;
            cin >> owner >> cell >> top >> front >> bottom >> back >> left >> right;
            cin.ignore();
            d = new Die(b.toNumber(cell), owner, top, front, bottom, back, left, right);
            b.addDice(d);
        }

        // For now, disappointing solution : ignore the tree completely...
        StrategyTree *tree = new StrategyTree(nullptr);
        if (diceCount > 2)
        {
            b.buildTree(0, tree, 1);
        }
        else
        {
            b.buildTree(0, tree, 2);
        }
        tree = tree->getBest();
        cout << tree->getMoves().back() << endl;
    }
}