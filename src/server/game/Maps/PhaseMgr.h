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

#ifndef TRINITY_PHASEMGR_H
#define TRINITY_PHASEMGR_H

#include "SharedDefines.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"

class ObjectMgr;
class Player;

// Phasing (visibility)
enum PhasingFlags
{
    PHASING_FLAG_OVERWRITE_EXISTING = 0x01,       // don't stack with existing phases, overwrites existing phases
    PHASING_FLAG_NO_MORE_PHASES     = 0x02,       // stop calculating phases after this phase was applied (no more phases will be applied)
    PHASING_FLAG_NEGATE_PHASE       = 0x04        // negate instead to add the phasemask
};

enum PhaseUpdateFlag
{
    PHASE_UPDATE_FLAG_ZONE_UPDATE   = 0x01,
    PHASE_UPDATE_FLAG_AREA_UPDATE   = 0x02,
    PHASE_UPDATE_FLAG_STORE_UPDATE  = 0x04,
};

struct PhasingDefinition
{
    uint32 zoneId;
    uint32 entry;
    uint32 phasemask;
    uint32 phaseId;
    uint32 terrainswapmap;
    uint8 flags;

    bool IsOverwritingExistingPhases() const { return flags & PHASING_FLAG_OVERWRITE_EXISTING; }
    bool IsLastDefinition() const { return flags & PHASING_FLAG_NO_MORE_PHASES; }
    bool IsNegatingPhasemask() const { return flags & PHASING_FLAG_NEGATE_PHASE; }
};

typedef std::vector<PhasingDefinition> PhasingDefinitionContainer;
typedef UNORDERED_MAP<uint32 /*zoneId*/, PhasingDefinitionContainer> PhasingDefinitionStore;

// Flags from Phase.dbc
enum PhaseFlag
{
    PHASEFLAG_NO_TERRAINSWAP           = 0x0,
    PHASEFLAG_TERRAINSWAP              = 0x4,
    PHASEFLAG_NORMAL_PHASE             = 0x8
};

struct PhaseData
{
    PhaseData(Player* _player) : player(_player), _PhasemaskThroughAuras(0), _CustomPhasemask(0) { Reset(); }

    uint32 _PhasemaskThroughDefinitions;
    uint32 _PhasemaskThroughAuras;
    uint32 _CustomPhasemask;
    uint32 flag;

    uint32 GetCurrentPhasemask() const;
    inline uint32 GetPhaseMaskForSpawn() const;

    void Reset() { _PhasemaskThroughDefinitions = 0; terrainswap = 0; phaseid = 0; flag = PHASEFLAG_NORMAL_PHASE; }
    void SendDataToPlayer();

private:
    uint32 terrainswap;
    uint32 phaseid;
    Player* player;
};

struct PhaseUpdateData
{
    void AddConditionType(ConditionTypes const conditionType) { _conditionTypeFlags |= (1 << conditionType); }
    void AddQuestUpdate(uint32 const questId);

    bool IsConditionRelated(Condition const* condition) const;

private:
    uint32 _conditionTypeFlags;
    uint32 _questId;
};

class PhaseMgr
{
public:
    PhaseMgr(Player* _player);
    ~PhaseMgr() {}

    uint32 GetCurrentPhasemask() { return phaseData.GetCurrentPhasemask(); };
    inline uint32 GetPhaseMaskForSpawn() { return phaseData.GetCurrentPhasemask(); }

    // Phase definitions update handling
    void NotifyConditionChanged(PhaseUpdateData const updateData);
    void NotifyStoresReloaded() { Recalculate(); }

    void SendPhaseDataToPlayer();

    // Aura phase effects
    void RegisterPhasingAuraEffect(AuraEffect const* auraEffect);
    void UnRegisterPhasingAuraEffect(AuraEffect const* auraEffect);

    // Update flags (delayed phasing)
    void AddUpdateFlag(PhaseUpdateFlag updateFlag) { _UpdateFlags |= updateFlag; }
    void RemoveUpdateFlag(PhaseUpdateFlag updateFlag);

    // Needed for modify phase command
    void SetCustomPhase(uint32 const phaseMask);

    // Debug
    void SendDebugReportToPlayer(Player* const debugger);

    static bool IsConditionTypeSupported(ConditionTypes const conditionType);

private:
    void Recalculate();

    inline bool CheckDefinition(PhasingDefinition const* phasingDefinition);

    void AddPhasingDefinitionToPhase(uint32 &phase, PhasingDefinition const* phasingDefinition);

    bool NeedsPhaseUpdateWithData(PhaseUpdateData const updateData) const;

    PhasingDefinitionStore const* _PhasingStore;

    Player* player;
    PhaseData phaseData;
    uint8 _UpdateFlags;
};

#endif
