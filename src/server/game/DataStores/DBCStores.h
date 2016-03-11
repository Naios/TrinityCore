/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#ifndef TRINITY_DBCSTORES_H
#define TRINITY_DBCSTORES_H

#include "DBCStore.h"
#include "DBCStructure.h"
#include "DB2Structure.h"
#include "SharedDefines.h"

// CharSections
TRINITY_GAME_API CharSectionsEntry const* GetCharSectionEntry(uint8 race, CharSectionType genType, uint8 gender, uint8 type, uint8 color);

// ChrClasses
TRINITY_GAME_API char const* GetClassName(uint8 class_, uint8 locale);

// ChrRaces
TRINITY_GAME_API char const* GetRaceName(uint8 race, uint8 locale);

// ChrSpecialization
typedef ChrSpecializationEntry const* ChrSpecializationByIndexArray[MAX_CLASSES][MAX_SPECIALIZATIONS];

// CreatureFamilty
TRINITY_GAME_API char const* GetCreatureFamilyPetName(uint32 petfamily, uint32 locale);

// Faction
TRINITY_GAME_API std::vector<uint32> const* GetFactionTeamList(uint32 faction);

// ItemSetSpells
typedef std::vector<ItemSetSpellEntry const*> ItemSetSpells;
typedef std::unordered_map<uint32, ItemSetSpells> ItemSetSpellsStore;

// LfgDungeons
TRINITY_GAME_API LFGDungeonEntry const* GetLFGDungeon(uint32 mapId, Difficulty difficulty);

// Light
TRINITY_GAME_API uint32 GetDefaultMapLight(uint32 mapId);

// LiquidType
TRINITY_GAME_API uint32 GetLiquidFlags(uint32 liquidType);

// MapDifficulty
typedef std::unordered_map<uint32, std::unordered_map<uint32, MapDifficultyEntry const*>> MapDifficultyMap;
TRINITY_GAME_API MapDifficultyEntry const* GetDefaultMapDifficulty(uint32 mapId, Difficulty* difficulty = nullptr);
TRINITY_GAME_API MapDifficultyEntry const* GetMapDifficultyData(uint32 mapId, Difficulty difficulty);
TRINITY_GAME_API MapDifficultyEntry const* GetDownscaledMapDifficultyData(uint32 mapId, Difficulty &difficulty);

// PvpDifficulty
TRINITY_GAME_API PvPDifficultyEntry const* GetBattlegroundBracketByLevel(uint32 mapid, uint32 level);
TRINITY_GAME_API PvPDifficultyEntry const* GetBattlegroundBracketById(uint32 mapid, BattlegroundBracketId id);

// SkillRaceClassInfo
typedef std::unordered_multimap<uint32, SkillRaceClassInfoEntry const*> SkillRaceClassInfoMap;
typedef std::pair<SkillRaceClassInfoMap::iterator, SkillRaceClassInfoMap::iterator> SkillRaceClassInfoBounds;
TRINITY_GAME_API SkillRaceClassInfoEntry const* GetSkillRaceClassInfo(uint32 skill, uint8 race, uint8 class_);

// SpellEffectScaling
typedef std::unordered_map<uint32, uint32> SpellEffectScallingByEffectId;

// Talent
typedef std::vector<TalentEntry const*> TalentsByPosition[MAX_CLASSES][MAX_TALENT_TIERS][MAX_TALENT_COLUMNS];

// TotemCategory
TRINITY_GAME_API bool IsTotemCategoryCompatibleWith(uint32 itemTotemCategoryId, uint32 requiredTotemCategoryId);

// WMOAreaTable
TRINITY_GAME_API WMOAreaTableEntry const* GetWMOAreaTableEntryByTripple(int32 rootid, int32 adtid, int32 groupid);

// WorldMapArea
TRINITY_GAME_API uint32 GetVirtualMapForMapAndZone(uint32 mapid, uint32 zoneId);
TRINITY_GAME_API void Zone2MapCoordinates(float &x, float &y, uint32 worldMapAreaId);
TRINITY_GAME_API void Map2ZoneCoordinates(float &x, float &y, uint32 worldMapAreaId);

// WorldMapTransforms
TRINITY_GAME_API void DeterminaAlternateMapPosition(uint32 mapId, float x, float y, float z, uint32* newMapId = nullptr, DBCPosition2D* newPos = nullptr);

TRINITY_GAME_API uint32 GetMaxLevelForExpansion(uint32 expansion);
TRINITY_GAME_API uint32 GetExpansionForLevel(uint32 level);

template<class T>
class GameTable
{
public:
    GameTable(char const* format) : _storage(format), _gtEntry(nullptr) { }

    void SetGameTableEntry(GameTablesEntry const* gtEntry) { _gtEntry = gtEntry; }

    T const* EvaluateTable(uint32 row, uint32 column) const
    {
        ASSERT(row < _gtEntry->NumRows, "Requested row %u from GameTable %s but there are only %u rows!", row, _gtEntry->Name->Str[0], _gtEntry->NumRows);
        ASSERT(column < _gtEntry->NumColumns, "Requested column %u from GameTable %s but there are only %u columns!", column, _gtEntry->Name->Str[0], _gtEntry->NumColumns);

        return _storage.LookupEntry(_gtEntry->NumRows * column + row);
    }

    char const* GetFormat() const { return _storage.GetFormat(); }
    uint32 GetFieldCount() const { return _storage.GetFieldCount(); }
    bool Load(char const* fileName) { return _storage.Load(fileName, nullptr); }

    uint32 GetTableRowCount() const { return _gtEntry->NumRows; }
    uint32 GetTableColumnCount() const { return _gtEntry->NumColumns; }

private:
    DBCStorage<T> _storage;
    GameTablesEntry const* _gtEntry;
};

TRINITY_GAME_API extern DBCStorage<AnimKitEntry>                     sAnimKitStore;
TRINITY_GAME_API extern DBCStorage<AreaTableEntry>                   sAreaTableStore;
TRINITY_GAME_API extern DBCStorage<AreaTriggerEntry>                 sAreaTriggerStore;
TRINITY_GAME_API extern DBCStorage<ArmorLocationEntry>               sArmorLocationStore;
TRINITY_GAME_API extern DBCStorage<BankBagSlotPricesEntry>           sBankBagSlotPricesStore;
TRINITY_GAME_API extern DBCStorage<BannedAddOnsEntry>                sBannedAddOnsStore;
TRINITY_GAME_API extern DBCStorage<BattlemasterListEntry>            sBattlemasterListStore;
TRINITY_GAME_API extern DBCStorage<CharSectionsEntry>                sCharSectionsStore;
TRINITY_GAME_API extern DBCStorage<CharTitlesEntry>                  sCharTitlesStore;
TRINITY_GAME_API extern DBCStorage<ChatChannelsEntry>                sChatChannelsStore;
TRINITY_GAME_API extern DBCStorage<ChrClassesEntry>                  sChrClassesStore;
TRINITY_GAME_API extern DBCStorage<ChrRacesEntry>                    sChrRacesStore;
TRINITY_GAME_API extern DBCStorage<ChrSpecializationEntry>           sChrSpecializationStore;
TRINITY_GAME_API extern ChrSpecializationByIndexArray                sChrSpecializationByIndexStore;
TRINITY_GAME_API extern DBCStorage<CreatureDisplayInfoExtraEntry>    sCreatureDisplayInfoExtraStore;
TRINITY_GAME_API extern DBCStorage<CreatureFamilyEntry>              sCreatureFamilyStore;
TRINITY_GAME_API extern DBCStorage<CreatureModelDataEntry>           sCreatureModelDataStore;
TRINITY_GAME_API extern DBCStorage<DifficultyEntry>                  sDifficultyStore;
TRINITY_GAME_API extern DBCStorage<DungeonEncounterEntry>            sDungeonEncounterStore;
TRINITY_GAME_API extern DBCStorage<DurabilityCostsEntry>             sDurabilityCostsStore;
TRINITY_GAME_API extern DBCStorage<EmotesEntry>                      sEmotesStore;
TRINITY_GAME_API extern DBCStorage<EmotesTextEntry>                  sEmotesTextStore;
TRINITY_GAME_API extern DBCStorage<FactionEntry>                     sFactionStore;
TRINITY_GAME_API extern DBCStorage<FactionTemplateEntry>             sFactionTemplateStore;
TRINITY_GAME_API extern DBCStorage<GameObjectDisplayInfoEntry>       sGameObjectDisplayInfoStore;
TRINITY_GAME_API extern DBCStorage<GemPropertiesEntry>               sGemPropertiesStore;
TRINITY_GAME_API extern DBCStorage<GlyphPropertiesEntry>             sGlyphPropertiesStore;
TRINITY_GAME_API extern DBCStorage<GuildColorBackgroundEntry>        sGuildColorBackgroundStore;
TRINITY_GAME_API extern DBCStorage<GuildColorBorderEntry>            sGuildColorBorderStore;
TRINITY_GAME_API extern DBCStorage<GuildColorEmblemEntry>            sGuildColorEmblemStore;
TRINITY_GAME_API extern DBCStorage<ItemArmorQualityEntry>            sItemArmorQualityStore;
TRINITY_GAME_API extern DBCStorage<ItemArmorShieldEntry>             sItemArmorShieldStore;
TRINITY_GAME_API extern DBCStorage<ItemArmorTotalEntry>              sItemArmorTotalStore;
TRINITY_GAME_API extern DBCStorage<ItemBagFamilyEntry>               sItemBagFamilyStore;
TRINITY_GAME_API extern DBCStorage<ItemDamageEntry>                  sItemDamageAmmoStore;
TRINITY_GAME_API extern DBCStorage<ItemDamageEntry>                  sItemDamageOneHandCasterStore;
TRINITY_GAME_API extern DBCStorage<ItemDamageEntry>                  sItemDamageOneHandStore;
TRINITY_GAME_API extern DBCStorage<ItemDamageEntry>                  sItemDamageRangedStore;
TRINITY_GAME_API extern DBCStorage<ItemDamageEntry>                  sItemDamageThrownStore;
TRINITY_GAME_API extern DBCStorage<ItemDamageEntry>                  sItemDamageTwoHandCasterStore;
TRINITY_GAME_API extern DBCStorage<ItemDamageEntry>                  sItemDamageTwoHandStore;
TRINITY_GAME_API extern DBCStorage<ItemDamageEntry>                  sItemDamageWandStore;
TRINITY_GAME_API extern DBCStorage<ItemSetEntry>                     sItemSetStore;
TRINITY_GAME_API extern DBCStorage<ItemSetSpellEntry>                sItemSetSpellStore;
TRINITY_GAME_API extern ItemSetSpellsStore                           sItemSetSpellsStore;
TRINITY_GAME_API extern DBCStorage<LFGDungeonEntry>                  sLFGDungeonStore;
TRINITY_GAME_API extern DBCStorage<LiquidTypeEntry>                  sLiquidTypeStore;
TRINITY_GAME_API extern DBCStorage<LockEntry>                        sLockStore;
TRINITY_GAME_API extern DBCStorage<MapEntry>                         sMapStore;
TRINITY_GAME_API extern MapDifficultyMap                             sMapDifficultyMap;
TRINITY_GAME_API extern DBCStorage<MovieEntry>                       sMovieStore;
TRINITY_GAME_API extern DBCStorage<PhaseEntry>                       sPhaseStore;
TRINITY_GAME_API extern DBCStorage<PowerDisplayEntry>                sPowerDisplayStore;
TRINITY_GAME_API extern DBCStorage<QuestFactionRewEntry>             sQuestFactionRewardStore;
TRINITY_GAME_API extern DBCStorage<RandomPropertiesPointsEntry>      sRandomPropertiesPointsStore;
TRINITY_GAME_API extern DBCStorage<SkillLineAbilityEntry>            sSkillLineAbilityStore;
TRINITY_GAME_API extern DBCStorage<SkillLineEntry>                   sSkillLineStore;
TRINITY_GAME_API extern DBCStorage<SkillRaceClassInfoEntry>          sSkillRaceClassInfoStore;
TRINITY_GAME_API extern DBCStorage<SpellAuraOptionsEntry>            sSpellAuraOptionsStore;
TRINITY_GAME_API extern DBCStorage<SpellCategoriesEntry>             sSpellCategoriesStore;
TRINITY_GAME_API extern DBCStorage<SpellCategoryEntry>               sSpellCategoryStore;
TRINITY_GAME_API extern DBCStorage<SpellCooldownsEntry>              sSpellCooldownsStore;
TRINITY_GAME_API extern DBCStorage<SpellEffectEntry>                 sSpellEffectStore;
TRINITY_GAME_API extern DBCStorage<SpellEffectScalingEntry>          sSpellEffectScalingStore;
TRINITY_GAME_API extern SpellEffectScallingByEffectId                sSpellEffectScallingByEffectId;
TRINITY_GAME_API extern DBCStorage<SpellEntry>                       sSpellStore;
TRINITY_GAME_API extern DBCStorage<SpellEquippedItemsEntry>          sSpellEquippedItemsStore;
TRINITY_GAME_API extern DBCStorage<SpellFocusObjectEntry>            sSpellFocusObjectStore;
TRINITY_GAME_API extern DBCStorage<SpellInterruptsEntry>             sSpellInterruptsStore;
TRINITY_GAME_API extern DBCStorage<SpellItemEnchantmentEntry>        sSpellItemEnchantmentStore;
TRINITY_GAME_API extern DBCStorage<SpellLevelsEntry>                 sSpellLevelsStore;
TRINITY_GAME_API extern DBCStorage<SpellScalingEntry>                sSpellScalingStore;
TRINITY_GAME_API extern DBCStorage<SpellShapeshiftEntry>             sSpellShapeshiftStore;
TRINITY_GAME_API extern DBCStorage<SpellShapeshiftFormEntry>         sSpellShapeshiftFormStore;
TRINITY_GAME_API extern DBCStorage<SpellTargetRestrictionsEntry>     sSpellTargetRestrictionsStore;
TRINITY_GAME_API extern DBCStorage<SummonPropertiesEntry>            sSummonPropertiesStore;
TRINITY_GAME_API extern DBCStorage<TalentEntry>                      sTalentStore;
TRINITY_GAME_API extern TalentsByPosition                            sTalentByPos;
TRINITY_GAME_API extern DBCStorage<VehicleEntry>                     sVehicleStore;
TRINITY_GAME_API extern DBCStorage<VehicleSeatEntry>                 sVehicleSeatStore;
TRINITY_GAME_API extern DBCStorage<WMOAreaTableEntry>                sWMOAreaTableStore;
TRINITY_GAME_API extern DBCStorage<WorldSafeLocsEntry>               sWorldSafeLocsStore;

TRINITY_GAME_API extern GameTable<GtBarberShopCostBaseEntry>         sGtBarberShopCostBaseStore;
TRINITY_GAME_API extern GameTable<GtChanceToMeleeCritBaseEntry>      sGtChanceToMeleeCritBaseStore;
TRINITY_GAME_API extern GameTable<GtChanceToMeleeCritEntry>          sGtChanceToMeleeCritStore;
TRINITY_GAME_API extern GameTable<GtChanceToSpellCritBaseEntry>      sGtChanceToSpellCritBaseStore;
TRINITY_GAME_API extern GameTable<GtChanceToSpellCritEntry>          sGtChanceToSpellCritStore;
TRINITY_GAME_API extern GameTable<GtCombatRatingsEntry>              sGtCombatRatingsStore;
TRINITY_GAME_API extern GameTable<GtItemSocketCostPerLevelEntry>     sGtItemSocketCostPerLevelStore;
TRINITY_GAME_API extern GameTable<GtNPCManaCostScalerEntry>          sGtNPCManaCostScalerStore;
TRINITY_GAME_API extern GameTable<GtNpcTotalHpEntry>                 sGtNpcTotalHpStore;
TRINITY_GAME_API extern GameTable<GtNpcTotalHpExp1Entry>             sGtNpcTotalHpExp1Store;
TRINITY_GAME_API extern GameTable<GtNpcTotalHpExp2Entry>             sGtNpcTotalHpExp2Store;
TRINITY_GAME_API extern GameTable<GtNpcTotalHpExp3Entry>             sGtNpcTotalHpExp3Store;
TRINITY_GAME_API extern GameTable<GtNpcTotalHpExp4Entry>             sGtNpcTotalHpExp4Store;
TRINITY_GAME_API extern GameTable<GtNpcTotalHpExp5Entry>             sGtNpcTotalHpExp5Store;
TRINITY_GAME_API extern GameTable<GtOCTBaseHPByClassEntry>           sGtOCTBaseHPByClassStore;
TRINITY_GAME_API extern GameTable<GtOCTBaseMPByClassEntry>           sGtOCTBaseMPByClassStore;
TRINITY_GAME_API extern GameTable<GtOCTLevelExperienceEntry>         sGtOCTLevelExperienceStore;
TRINITY_GAME_API extern GameTable<GtRegenMPPerSptEntry>              sGtRegenMPPerSptStore;
TRINITY_GAME_API extern GameTable<GtSpellScalingEntry>               sGtSpellScalingStore;
TRINITY_GAME_API extern GameTable<GtOCTHpPerStaminaEntry>            sGtOCTHpPerStaminaStore;

TRINITY_GAME_API void LoadDBCStores(const std::string& dataPath, uint32 defaultLocale);
TRINITY_GAME_API void LoadGameTables(const std::string& dataPath, uint32 defaultLocale);

#endif
