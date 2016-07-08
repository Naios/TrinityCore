/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptedCreature.h"

// Testing
#include "World.h"
#include "Duration.h"
#include "Spell.h"

class AsyncAIBase : public ScriptedAI
{
public:
    explicit AsyncAIBase(Creature* creature) : ScriptedAI(creature) { }
    virtual ~AsyncAIBase() { }

    void Reset() override
    {
    }

    void EnterCombat(Unit*) override
    {
    }
};

class RecurringEvent
{
public:
    RecurringEvent() { }

    RecurringEvent operator| (RecurringEvent event)
    {
        return event;
    }
};

template<typename... Args>
class RecurringEventEmitter
{
    RecurringEvent next;

public:
    RecurringEvent operator| (RecurringEvent event)
    {
        return event;
    }

    RecurringEvent AsEvent()
    {
        return { };
    }
};

class AsyncAI
{
public:
    explicit AsyncAI(Creature* creature) { }
    virtual ~AsyncAI() { }

    virtual void SetUp() = 0;

    RecurringEvent CastSpell(uint32 spellId) const
    {
        return { };
    }

    RecurringEvent MoveTo(Position pos) const
    {
        return { };
    }

    RecurringEvent MoveToHome() const
    {
        return { };
    }

    RecurringEvent In(Milliseconds) const
    {
        return { };
    }

    RecurringEvent Talk(std::string) const
    {
        return { };
    }

protected:
    RecurringEventEmitter<Unit*> onEnterCombat;
};

class MyAsyncAI : public AsyncAI
{
public:
    explicit MyAsyncAI(Creature* creature)
        : AsyncAI(creature)  { }

    RecurringEventEmitter<> startSpeechEvent;

    void SetUp() final override
    {
        onEnterCombat
            | CastSpell(12822)
            | MoveTo({ 1, 1, 1, 0 })
            | In(Seconds(10))
            | MoveToHome()
            | startSpeechEvent.AsEvent();

        startSpeechEvent
            | Talk("Hey")
            | Talk("2. sentence")
            | Talk("3. sentence");
    }
};

namespace FutureScripting {
    template<typename T = void>
    struct ScriptedFuture {
        template<typename C>
        auto Then(C&&) {
            return ScriptedFuture<>();
        }

        template<typename C>
        auto operator | (C&& c) {
            return Then(std::forward<C>(c));
        }

        template<typename C>
        auto operator && (C&& c) {
            return Then(std::forward<C>(c));
        }

        template<typename C>
        auto operator || (C&& c) {
            return Then(std::forward<C>(c));
        }

        void Repeat() { }
    };

    template<typename T>
    auto WhenAll(T&&...) {
        return ScriptedFuture<>();
    }

    template<typename T>
    auto WhenAny(T&&...) {
        return ScriptedFuture<>();
    }

    template<typename T>
    struct ScriptedEvent : ScriptedFuture<T> { };
}

using FutureScripting::ScriptedFuture;
using FutureScripting::ScriptedEvent;

struct FutureAI
{
    ScriptedFuture<SpellCastResult> AsyncCastSpell(uint32 spellId) {
        return { };
    }

    ScriptedFuture<bool> AsyncMoveTo(Position) {
        return {};
    }

    ScriptedFuture<Unit*> WhenOnEnterCombat() {
        return{};
    }

    ScriptedFuture<> AsyncYell(std::string) {
        return { };
    }

    ScriptedFuture<> AsyncMoveToHome() {
        return { };
    }

    ScriptedFuture<> AsyncRepeat() {
        return { };
    }

    template<typename Rep, typename Period>
    ScriptedFuture<> WaitFor(std::chrono::duration<Rep, Period> duration) {
        return { };
    }

    template<typename Rep, typename Period, typename Rep2, typename Period2>
    ScriptedFuture<> WaitFor(std::chrono::duration<Rep, Period>,
                             std::chrono::duration<Rep2, Period2>) {
        return { };
    }


    void Test()
    {
        auto other = Trinity::make_unique<FutureAI>();

        AsyncCastSpell(29283)
            .Then(AsyncCastSpell(37837))
            .Then([](SpellCastResult result) {

            })
            .Then(WhenAll(AsyncCastSpell(28273), AsyncCastSpell(38382)));

        WhenOnEnterCombat()
            | AsyncMoveTo({0, 0, 0, 0});

        WaitFor(Seconds(12), Seconds(14))
            | AsyncMoveTo({ 0, 0, 0, 0 }) && other->AsyncMoveTo({ 0, 0, 0, 0 })
            | AsyncYell("I'm at the point!")
            | other->AsyncYell("Me too!")
            | AsyncMoveToHome()
            | AsyncRepeat();
    }
};

// The name of this function should match:
// void Add${NameOfDirectory}Scripts()
void AddPromiseScripts()
{
    auto ai = Trinity::make_unique<FutureAI>();

    TC_LOG_INFO("server.loading", ">> Running promise tests...");
    ai->Test();
    TC_LOG_INFO("server.loading", ">> Done!");
}
