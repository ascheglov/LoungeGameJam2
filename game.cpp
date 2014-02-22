#include "Window.hpp"
#include "Game.hpp"

struct Cabbage : ObjectHelper<'@', Cabbage>
{
    int hp = 3;

    virtual bool wasHitBy(Object* other) override
    {
        if (--hp > 0)
            return true;

        m_game->remove(this);
        return false;
    }

    virtual char image() override
    {
        return ".oO@"[hp];
    }
};

struct Water : ObjectHelper<'~', Water>
{
};

enum Direction
{
    DirUp = 0, DirRight, DirDown, DirLeft,
};

struct MoveableObject : Object
{
    Object* move(Direction dir)
    {
        struct DXY { int x, y; } ofs[4] = {
            {0, -1},
            {1, 0},
            {0, 1},
            {-1, 0},
        };

        auto x = m_x + ofs[dir].x;
        auto y = m_y + ofs[dir].y;
        auto other = m_game->get(x, y);
        if (other && other->wasHitBy(this))
            return other;

        m_x = x;
        m_y = y;
        return nullptr;
    }

    void randomMove()
    {
        move(static_cast<Direction>(m_game->random(4)));
    }

    Object* moveTo(int x, int y)
    {
        auto dx = x - m_x;
        auto dy = y - m_y;
        if (abs(dx) > abs(dy))
            return move(dx > 0 ? DirRight : DirLeft);

        return move(dy > 0 ? DirDown : DirUp);
    }
};

struct Player : ObjectHelper<'I', Player, MoveableObject>
{
    virtual void update() override
    {
        switch (m_game->m_input)
        {
        case 'w': move(DirUp); return;
        case 's': move(DirDown); return;
        case 'a': move(DirLeft); return;
        case 'd': move(DirRight); return;
        case '\x1b': m_game->m_isRunning = false;
        }
    }
};

struct Fence : ObjectHelper<'#', Fence>
{
    bool isBroken = false;

    virtual bool wasHitBy(Object* other) override
    {
        if (other->is<Player>())
        {
            isBroken = false;
            return true;
        }

        if (isBroken)
        {
            m_game->remove(this);
            return false;
        }

        isBroken = true;
        return true;
    }

    virtual char image() override
    {
        return isBroken ? '=' : '#';
    }
};

struct Sheep : ObjectHelper<'S', Sheep, MoveableObject>
{
    enum State { Idle, RunToCabbage, RunAway };
    State m_state = Idle;

    virtual void update() override
    {
        if (m_state == Idle)
        {
            if (m_game->random(100) < 50) { return; }
            if (m_game->random(100) < 25) { m_state = RunToCabbage; return; }
            randomMove();
        }

        if (m_state == RunToCabbage)
        {
            if (m_game->random(100) < 10) { m_state = Idle; return; }
            auto cabbage = m_game->getNearest<Cabbage>(this);
            if (!cabbage) { m_state = Idle; return; }
            if (auto blockingObject = moveTo(cabbage->m_x, cabbage->m_y))
            {
                if (blockingObject->is<Player>()) { m_state = RunAway; return; }
                if (blockingObject->is<Fence>()) { m_state = Idle; return; }
            }
        }

        if (m_state == RunAway)
        {
            auto blockingObject = move(DirDown);
            if (blockingObject) m_state = Idle;
        }
    }

    virtual bool wasHitBy(Object* other) override
    {
        m_state = RunAway;
        return true;
    }

    virtual char image() override
    {
        if (m_state == Idle) return 's';
        if (m_state == RunToCabbage) return 'S';
        if (m_state == RunAway) return '$';
        assert(!"unreachable");
        return '?';
    }
};

std::string field =
R"(~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~)" "\n"
R"(~~~~~~~                ~~~~~~~)" "\n"
R"(~~~~                      ~~~~)" "\n"
R"(~~       @   @   @   @     ~~~)" "\n"
R"(~                            ~)" "\n"
R"(~                            ~)" "\n"
R"(~~       @   @   @   @      ~~)" "\n"
R"(~~                          ~~)" "\n"
R"(~~              I          ~~~)" "\n"
R"(~~~~~                    ~~~~~)" "\n"
R"(~~~~~~##################~~~~~~)" "\n"
R"(~~~~~                     ~~~~)" "\n"
R"(~~~~~   S                  ~~~)" "\n"
R"(~~~~~~            S      ~~~~~)" "\n"
R"(~~~~~~~       S         ~~~~~~)" "\n"
R"(~~~~~~~~           ~~~~~~~~~~~)" "\n"
R"(~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~)" "\n"
"";

int main()
{
    Game game;
    game.init<Player, Sheep, Cabbage, Fence, Water>(field);
    game.m_window.m_helpStr = "WASD to move, ESC to exit";
    game.run();
}