#pragma once

#include <Windows.h>
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_set>

struct Drawable
{
    int m_x, m_y;
    virtual char image() = 0;

protected:
    ~Drawable() = default;
};

struct Window
{
    HANDLE m_hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE m_hStdIn = GetStdHandle(STD_INPUT_HANDLE);

    char getEvent()
    {
        for (;;)
        {
            INPUT_RECORD inputRec;
            DWORD nRead;
            auto ok = ReadConsoleInput(m_hStdIn, &inputRec, 1, &nRead);
            assert(ok != 0);
            if (inputRec.EventType == KEY_EVENT)
            {
                auto&& e = inputRec.Event.KeyEvent;
                if (!e.bKeyDown)
                    return e.uChar.AsciiChar;
            }
        }
    }

    std::unordered_set<Drawable*> m_objects;
    int m_cx;
    int m_cy;

    void addObject(Drawable* obj) { m_objects.emplace(obj); }
    void removeObject(Drawable* obj) { m_objects.erase(obj); }

    void init(int cx, int cy)
    {
        m_cx = cx + 1;
        m_cy = cy;
    }

    std::string m_helpStr;

    void draw()
    {
        clear();
        std::string field(m_cx * m_cy, ' ');
        for (auto y = 0; y != m_cy; ++y)
            field[m_cx - 1 + y * m_cx] = '\n';

        for (auto obj : m_objects)
        {
            auto idx = obj->m_x + m_cx * obj->m_y;
            assert(field[idx] == ' ');
            field[idx] = obj->image();
        }

        std::cout << field;
        std::cout << "\n" << m_helpStr << std::endl;
    }

    void clear()
    {
        COORD topLeft = {0, 0};
        SetConsoleCursorPosition(m_hStdOut, topLeft);
    }
};
