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

#include "World.h"
#include "Duration.h"
#include "Spell.h"

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
