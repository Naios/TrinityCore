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

#include "Log.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"

struct hey
{
    hey()
    {
        TC_LOG_INFO("scripts.hotswap", "hey from my module blub!");

        for (uint32 i = 0; i < 10; ++i)
            if (auto const t = sObjectMgr->GetCreatureTemplate(i))
                TC_LOG_INFO("scripts.hotswap", "the name is %s", t->Name.c_str());
    }
};

static hey const h;


class npc_test_script_1 : public CreatureScript
{
public:
    npc_test_script_1() : CreatureScript("npc_test_script_1") { }

    struct npc_test_script_1AI : public CreatureAI
    {
        npc_test_script_1AI(Creature* creature) : CreatureAI(creature)
        {
            TC_LOG_INFO("scripts.hotswap", "npc_test_script_1 instance created");
        }

        virtual ~npc_test_script_1AI()
        {
            TC_LOG_INFO("scripts.hotswap", "npc_test_script_1 instance destroyed");
        }

        TaskScheduler scheduler;

        void Reset() override
        {
            me->Yell("Script reset!", LANG_UNIVERSAL);
        }

        void EnterCombat(Unit* /*victim*/) override
        {
            me->Yell("Entering combat now!", LANG_UNIVERSAL);

            scheduler.Schedule(Seconds(2), [&](TaskContext context)
            {
                me->Yell("huhu and swapped!", LANG_UNIVERSAL);
                context.Repeat();
            });
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            scheduler.Update(diff);

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_test_script_1AI(creature);
    }
};

// The name of this function should match:
// void Add${NameOfDirectory}Scripts()
void AddCustomScripts()
{
    new npc_test_script_1();
}
