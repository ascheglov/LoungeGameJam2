#pragma once

#include "Window.hpp"

#include <cassert>
#include <string>
#include <unordered_set>

struct Game;

struct Object : Drawable
{
    Game* m_game;
    char m_type;
    virtual void update() {}
    virtual bool wasHitBy(Object* other) { return true; }

    template<typename T>
    bool is() const
    {
        return m_type == T::kDefaultImage;
    }
};

template<char C, typename Derived, typename Base = Object>
struct ObjectHelper : Base
{
    ObjectHelper() { m_type = C; }
    static const char kDefaultImage = C;
    virtual char image() { return C; }
};

inline int distance(const Object* A, const Object* B)
{
    return abs(A->m_x - B->m_x) + abs(A->m_y - B->m_y);
}

struct Game
{
    std::unordered_set<Object*> m_objects;
    char m_input;
    bool m_isRunning = true;
    Window m_window;

    int m_cx, m_cy;

    int random(int n)
    {
        return rand() % n;
    }

    template<class T, typename... As>
    T* spawn(As&&... as)
    {
        auto obj = new T(std::forward<As>(as)...);
        obj->m_game = this;
        m_objects.emplace(obj);
        m_window.addObject(obj);
        return obj;
    }

    void remove(Object* obj)
    {
        m_window.removeObject(obj);
        m_objects.erase(obj);
        delete obj;
    }

    template<class...> struct Factory;

    template<class T, class... Ts>
    struct Factory<T, Ts...>
    {
        static Object* create(Game& game, char c)
        {
            if (c == T::kDefaultImage) return game.spawn<T>();
            return Factory<Ts...>::create(game, c);
        }
    };

    template<>
    struct Factory<>
    {
        static Object* create(Game&, char) { return nullptr; }
    };

    template<class... Ts>
    void init(const std::string& field)
    {
        m_cx = field.find('\n') + 1;
        m_cy = field.size() / m_cx;
        assert(m_cx * m_cy == field.size());

        for (std::size_t i = 0; i != field.size(); ++i)
        {
            auto x = i % m_cx;
            auto y = i / m_cx;
            if (Object* o = Factory<Ts...>::create(*this, field[i]))
            {
                o->m_x = x;
                o->m_y = y;
            }
        }

        m_window.init(m_cx, m_cy);
    }

    Object* get(int x, int y) const
    {
        for (auto&& o : m_objects)
        {
            if (o->m_x == x && o->m_y == y)
                return o;
        }

        return nullptr;
    }

    template<class T>
    Object* getNearest(Object* src)
    {
        int minDistance = INT_MAX;
        Object* nearest = nullptr;
        for (auto&& o : m_objects)
        {
            if (!o->is<T>())
                continue;

            auto d = distance(src, o);
            if (minDistance < d)
                continue;

            minDistance = d;
            nearest = o;
        }
        return nearest;
    }

    void run()
    {
        while (m_isRunning)
        {
            m_window.draw();
            m_input = m_window.getEvent();
            for (auto&& obj : m_objects)
                obj->update();
        }
    }
};

