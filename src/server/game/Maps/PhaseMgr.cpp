/*
* Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 3 of the License, or (at your
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

#include "PhaseMgr.h"
#include "Chat.h"
#include "DBCStores.h"

//////////////////////////////////////////////////////////////////
// Updating

PhaseMgr::PhaseMgr(Player* _player) : player(_player), phaseData(_player)
{
    _PhasingStore = sObjectMgr->GetPhasingDefinitionStore();
}

void PhaseMgr::SendPhaseDataToPlayer()
{
    // Never update the phasemask if an update is in progress
    if (_UpdateFlags)
        return;

    phaseData.SendDataToPlayer();
}

void PhaseMgr::RemoveUpdateFlag(PhaseUpdateFlag updateFlag)
{
    _UpdateFlags &= ~updateFlag;

    if (updateFlag == PHASE_UPDATE_FLAG_ZONE_UPDATE)
    {
        // Update zone changes
        phaseData.Reset();

        if (_PhasingStore->find(player->GetZoneId()) != _PhasingStore->end())
            Recalculate();
    }

    SendPhaseDataToPlayer();
}

/////////////////////////////////////////////////////////////////
// Notifier

void PhaseMgr::NotifyConditionChanged(PhaseUpdateData const updateData)
{
    if (NeedsPhaseUpdateWithData(updateData))
    {
        Recalculate();
        SendPhaseDataToPlayer();
    }
}

//////////////////////////////////////////////////////////////////
// Phasing Definitions

void PhaseMgr::Recalculate()
{
    phaseData.Reset();

    PhasingDefinitionStore::const_iterator itr = _PhasingStore->find(player->GetZoneId());
    if (itr != _PhasingStore->end())
    {
        uint32 phaseId = 0;

        for (PhasingDefinitionContainer::const_iterator phase = itr->second.begin(); phase != itr->second.end(); ++phase)
        {
            if (CheckDefinition(&(*phase)))
            {
                AddPhasingDefinitionToPhase(phaseData._PhasemaskThroughDefinitions, &(*phase));
                phaseId = phase->phaseId;

                if (phase->IsLastDefinition())
                    break;
            }
        }

        if (phaseId)
        {
            TerrainSwapDefinition const* terrainSwap = sObjectMgr->GetTerrainSwap(phaseId, player->GetZoneId());
            if (terrainSwap)
                phaseData.UseTerrainSwap(terrainSwap);
        }
    }
}

inline bool PhaseMgr::CheckDefinition(PhasingDefinition const* phasingDefinition)
{
    return sConditionMgr->IsObjectMeetToConditions(player, sConditionMgr->GetConditionsForPhaseDefinition(phasingDefinition->zoneId, phasingDefinition->entry));
}

bool PhaseMgr::NeedsPhaseUpdateWithData(PhaseUpdateData const updateData) const
{
    PhasingDefinitionStore::const_iterator itr = _PhasingStore->find(player->GetZoneId());
    if (itr != _PhasingStore->end())
    {
        for (PhasingDefinitionContainer::const_iterator phase = itr->second.begin(); phase != itr->second.end(); ++phase)
        {
            ConditionList conditionList = sConditionMgr->GetConditionsForPhaseDefinition(phase->zoneId, phase->entry);
            for (ConditionList::const_iterator condition = conditionList.begin(); condition != conditionList.end(); ++condition)
                if (updateData.IsConditionRelated(*condition))
                    return true;
        }
    }
    return false;
}

void PhaseMgr::AddPhasingDefinitionToPhase(uint32 &phaseMask, PhasingDefinition const* phasingDefinition)
{
    if (phasingDefinition->IsOverwritingExistingPhases())
        phaseMask = phasingDefinition->phasemask;
    else
    {
        if (phasingDefinition->IsNegatingPhasemask())
            phaseMask &= ~phasingDefinition->phasemask;
        else
            phaseMask |= phasingDefinition->phasemask;
    }

    if (phaseData.flag == PHASEFLAG_NORMAL_PHASE)
        phaseData.flag = PHASEFLAG_NO_TERRAINSWAP;
}

//////////////////////////////////////////////////////////////////
// Auras

void PhaseMgr::RegisterPhasingAuraEffect(AuraEffect const* auraEffect)
{
    phaseData._PhasemaskThroughAuras |= auraEffect->GetMiscValue();

    SendPhaseDataToPlayer();
}

void PhaseMgr::UnRegisterPhasingAuraEffect(AuraEffect const* auraEffect)
{
    phaseData._PhasemaskThroughAuras = 0;
    Unit::AuraEffectList const& auraPhases = player->GetAuraEffectsByType(SPELL_AURA_PHASE);
    if (!auraPhases.empty())
        for (Unit::AuraEffectList::const_iterator itr = auraPhases.begin(); itr != auraPhases.end(); ++itr)
            phaseData._PhasemaskThroughAuras |= (*itr)->GetMiscValue();

    SendPhaseDataToPlayer();
}

//////////////////////////////////////////////////////////////////
// Commands

void PhaseMgr::SendDebugReportToPlayer(Player* const debugger)
{
    ChatHandler(debugger).PSendSysMessage(LANG_PHASING_REPORT_STATUS, player->GetName(), player->GetZoneId(), player->getLevel(), player->GetTeamId(), phaseData.flag);

    PhasingDefinitionStore::const_iterator itr = _PhasingStore->find(player->GetZoneId());
    if (itr == _PhasingStore->end())
    {
        ChatHandler(debugger).PSendSysMessage(LANG_PHASING_NO_DEFINITIONS, player->GetZoneId());
        return;
    }

    for (PhasingDefinitionContainer::const_iterator phase = itr->second.begin(); phase != itr->second.end(); ++phase)
    {
        if (CheckDefinition(&(*phase)))
            ChatHandler(debugger).PSendSysMessage(LANG_PHASING_SUCCESS, phase->IsNegatingPhasemask() ? "negated Phase" : "Phase", phase->phasemask);
        else
            ChatHandler(debugger).PSendSysMessage(LANG_PHASING_FAILED, phase->phasemask, phase->entry, phase->zoneId);

        if (phase->IsLastDefinition())
        {
            ChatHandler(debugger).PSendSysMessage(LANG_PHASING_LAST_PHASE, phase->phasemask, phase->entry, phase->zoneId);
            break;
        }
    }

    ChatHandler(debugger).PSendSysMessage(LANG_PHASING_LIST, phaseData._PhasemaskThroughDefinitions, phaseData._PhasemaskThroughAuras, phaseData._CustomPhasemask);

    ChatHandler(debugger).PSendSysMessage(LANG_PHASING_PHASEMASK, phaseData.GetPhaseMaskForSpawn(), player->GetPhaseMask());
}

void PhaseMgr::SetCustomPhase(uint32 const phaseMask)
{
    phaseData._CustomPhasemask = phaseMask;

    SendPhaseDataToPlayer();
}

//////////////////////////////////////////////////////////////////
// Phase Data

void PhaseData::UseTerrainSwap(TerrainSwapDefinition const* _terrainswap)
{
    terrainswap = _terrainswap->map;
    phaseid     = _terrainswap->phaseId;

    /*
    PhaseEntry const* entry = sPhaseStore.LookupEntry(phaseid);
    flag = entry->flag;
    */
}

uint32 PhaseData::GetCurrentPhasemask() const
{
    return player->isGameMaster() ? PHASEMASK_ANYWHERE : GetPhaseMaskForSpawn();
}

inline uint32 PhaseData::GetPhaseMaskForSpawn() const
{
    uint32 const phase = (_PhasemaskThroughDefinitions | _PhasemaskThroughAuras | _CustomPhasemask);
    return (phase ? phase : PHASEMASK_NORMAL);
}

void PhaseData::SendDataToPlayer()
{
    uint32 const phasemask = GetCurrentPhasemask();
    if (player->GetPhaseMask() == phasemask)
        return;

    // ToDo Send Terrainswap and PhaseId

    player->SetPhaseMask(phasemask, false);

    if (player->IsVisible())
        player->UpdateObjectVisibility();
}

//////////////////////////////////////////////////////////////////
// Phase Update Data

void PhaseUpdateData::AddQuestUpdate(uint32 const questId)
{
    AddConditionType(CONDITION_QUESTREWARDED);
    AddConditionType(CONDITION_QUESTTAKEN);
    AddConditionType(CONDITION_QUEST_COMPLETE);
    AddConditionType(CONDITION_QUEST_NONE);

    _questId = questId;
}

bool PhaseUpdateData::IsConditionRelated(Condition const* condition) const
{
    switch (condition->ConditionType)
    {
    case CONDITION_QUESTREWARDED:
    case CONDITION_QUESTTAKEN:
    case CONDITION_QUEST_COMPLETE:
    case CONDITION_QUEST_NONE:
        return condition->ConditionValue1 == _questId && ((1 << condition->ConditionType) & _conditionTypeFlags);
    default:
        return (1 << condition->ConditionType) & _conditionTypeFlags;
    }
}

bool PhaseMgr::IsConditionTypeSupported(ConditionTypes const conditionType)
{
    switch (conditionType)
    {
    case CONDITION_QUESTREWARDED:
    case CONDITION_QUESTTAKEN:
    case CONDITION_QUEST_COMPLETE:
    case CONDITION_QUEST_NONE:
    case CONDITION_TEAM:
    case CONDITION_CLASS:
    case CONDITION_RACE:
    case CONDITION_INSTANCE_DATA:
    case CONDITION_LEVEL:
        return true;
    default:
        return false;
    }
}