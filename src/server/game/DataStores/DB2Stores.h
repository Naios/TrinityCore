/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#ifndef TRINITY_DB2STORES_H
#define TRINITY_DB2STORES_H

#include "DB2Store.h"
#include "DB2Structure.h"
#include "SharedDefines.h"
#include <boost/regex.hpp>
#include <array>

extern TRINITY_GAME_API DB2Storage<AchievementEntry>                     sAchievementStore;
extern TRINITY_GAME_API DB2Storage<AuctionHouseEntry>                    sAuctionHouseStore;
extern TRINITY_GAME_API DB2Storage<BarberShopStyleEntry>                 sBarberShopStyleStore;
extern TRINITY_GAME_API DB2Storage<BattlePetBreedQualityEntry>           sBattlePetBreedQualityStore;
extern TRINITY_GAME_API DB2Storage<BattlePetBreedStateEntry>             sBattlePetBreedStateStore;
extern TRINITY_GAME_API DB2Storage<BattlePetSpeciesEntry>                sBattlePetSpeciesStore;
extern TRINITY_GAME_API DB2Storage<BattlePetSpeciesStateEntry>           sBattlePetSpeciesStateStore;
extern TRINITY_GAME_API DB2Storage<BroadcastTextEntry>                   sBroadcastTextStore;
extern TRINITY_GAME_API DB2Storage<CharStartOutfitEntry>                 sCharStartOutfitStore;
extern TRINITY_GAME_API DB2Storage<CinematicSequencesEntry>              sCinematicSequencesStore;
extern TRINITY_GAME_API DB2Storage<CreatureDisplayInfoEntry>             sCreatureDisplayInfoStore;
extern TRINITY_GAME_API DB2Storage<CreatureTypeEntry>                    sCreatureTypeStore;
extern TRINITY_GAME_API DB2Storage<CriteriaEntry>                        sCriteriaStore;
extern TRINITY_GAME_API DB2Storage<CriteriaTreeEntry>                    sCriteriaTreeStore;
extern TRINITY_GAME_API DB2Storage<CurrencyTypesEntry>                   sCurrencyTypesStore;
extern TRINITY_GAME_API DB2Storage<DestructibleModelDataEntry>           sDestructibleModelDataStore;
extern TRINITY_GAME_API DB2Storage<DurabilityQualityEntry>               sDurabilityQualityStore;
extern TRINITY_GAME_API DB2Storage<GameObjectsEntry>                     sGameObjectsStore;
extern TRINITY_GAME_API DB2Storage<GameTablesEntry>                      sGameTablesStore;
extern TRINITY_GAME_API DB2Storage<GarrAbilityEntry>                     sGarrAbilityStore;
extern TRINITY_GAME_API DB2Storage<GarrBuildingEntry>                    sGarrBuildingStore;
extern TRINITY_GAME_API DB2Storage<GarrBuildingPlotInstEntry>            sGarrBuildingPlotInstStore;
extern TRINITY_GAME_API DB2Storage<GarrClassSpecEntry>                   sGarrClassSpecStore;
extern TRINITY_GAME_API DB2Storage<GarrFollowerEntry>                    sGarrFollowerStore;
extern TRINITY_GAME_API DB2Storage<GarrFollowerXAbilityEntry>            sGarrFollowerXAbilityStore;
extern TRINITY_GAME_API DB2Storage<GarrPlotBuildingEntry>                sGarrPlotBuildingStore;
extern TRINITY_GAME_API DB2Storage<GarrPlotEntry>                        sGarrPlotStore;
extern TRINITY_GAME_API DB2Storage<GarrPlotInstanceEntry>                sGarrPlotInstanceStore;
extern TRINITY_GAME_API DB2Storage<GarrSiteLevelEntry>                   sGarrSiteLevelStore;
extern TRINITY_GAME_API DB2Storage<GarrSiteLevelPlotInstEntry>           sGarrSiteLevelPlotInstStore;
extern TRINITY_GAME_API DB2Storage<GlyphSlotEntry>                       sGlyphSlotStore;
extern TRINITY_GAME_API DB2Storage<GuildPerkSpellsEntry>                 sGuildPerkSpellsStore;
extern TRINITY_GAME_API DB2Storage<HolidaysEntry>                        sHolidaysStore;
extern TRINITY_GAME_API DB2Storage<ImportPriceArmorEntry>                sImportPriceArmorStore;
extern TRINITY_GAME_API DB2Storage<ImportPriceQualityEntry>              sImportPriceQualityStore;
extern TRINITY_GAME_API DB2Storage<ImportPriceShieldEntry>               sImportPriceShieldStore;
extern TRINITY_GAME_API DB2Storage<ImportPriceWeaponEntry>               sImportPriceWeaponStore;
extern TRINITY_GAME_API DB2Storage<ItemClassEntry>                       sItemClassStore;
extern TRINITY_GAME_API DB2Storage<ItemCurrencyCostEntry>                sItemCurrencyCostStore;
extern TRINITY_GAME_API DB2Storage<ItemDisenchantLootEntry>              sItemDisenchantLootStore;
extern TRINITY_GAME_API DB2Storage<ItemEffectEntry>                      sItemEffectStore;
extern TRINITY_GAME_API DB2Storage<ItemEntry>                            sItemStore;
extern TRINITY_GAME_API DB2Storage<ItemExtendedCostEntry>                sItemExtendedCostStore;
extern TRINITY_GAME_API DB2Storage<ItemLimitCategoryEntry>               sItemLimitCategoryStore;
extern TRINITY_GAME_API DB2Storage<ItemPriceBaseEntry>                   sItemPriceBaseStore;
extern TRINITY_GAME_API DB2Storage<ItemRandomPropertiesEntry>            sItemRandomPropertiesStore;
extern TRINITY_GAME_API DB2Storage<ItemRandomSuffixEntry>                sItemRandomSuffixStore;
extern TRINITY_GAME_API DB2Storage<ItemSparseEntry>                      sItemSparseStore;
extern TRINITY_GAME_API DB2Storage<ItemSpecEntry>                        sItemSpecStore;
extern TRINITY_GAME_API DB2Storage<ItemSpecOverrideEntry>                sItemSpecOverrideStore;
extern TRINITY_GAME_API DB2Storage<ItemToBattlePetSpeciesEntry>          sItemToBattlePetSpeciesStore;
extern TRINITY_GAME_API DB2Storage<MailTemplateEntry>                    sMailTemplateStore;
extern TRINITY_GAME_API DB2Storage<ModifierTreeEntry>                    sModifierTreeStore;
extern TRINITY_GAME_API DB2Storage<MountCapabilityEntry>                 sMountCapabilityStore;
extern TRINITY_GAME_API DB2Storage<OverrideSpellDataEntry>               sOverrideSpellDataStore;
extern TRINITY_GAME_API DB2Storage<QuestMoneyRewardEntry>                sQuestMoneyRewardStore;
extern TRINITY_GAME_API DB2Storage<QuestSortEntry>                       sQuestSortStore;
extern TRINITY_GAME_API DB2Storage<QuestXPEntry>                         sQuestXPStore;
extern TRINITY_GAME_API DB2Storage<ScalingStatDistributionEntry>         sScalingStatDistributionStore;
extern TRINITY_GAME_API DB2Storage<SoundEntriesEntry>                    sSoundEntriesStore;
extern TRINITY_GAME_API DB2Storage<SpellAuraRestrictionsEntry>           sSpellAuraRestrictionsStore;
extern TRINITY_GAME_API DB2Storage<SpellCastTimesEntry>                  sSpellCastTimesStore;
extern TRINITY_GAME_API DB2Storage<SpellCastingRequirementsEntry>        sSpellCastingRequirementsStore;
extern TRINITY_GAME_API DB2Storage<SpellClassOptionsEntry>               sSpellClassOptionsStore;
extern TRINITY_GAME_API DB2Storage<SpellDurationEntry>                   sSpellDurationStore;
extern TRINITY_GAME_API DB2Storage<SpellItemEnchantmentConditionEntry>   sSpellItemEnchantmentConditionStore;
extern TRINITY_GAME_API DB2Storage<SpellLearnSpellEntry>                 sSpellLearnSpellStore;
extern TRINITY_GAME_API DB2Storage<SpellMiscEntry>                       sSpellMiscStore;
extern TRINITY_GAME_API DB2Storage<SpellPowerEntry>                      sSpellPowerStore;
extern TRINITY_GAME_API DB2Storage<SpellRadiusEntry>                     sSpellRadiusStore;
extern TRINITY_GAME_API DB2Storage<SpellRangeEntry>                      sSpellRangeStore;
extern TRINITY_GAME_API DB2Storage<SpellReagentsEntry>                   sSpellReagentsStore;
extern TRINITY_GAME_API DB2Storage<SpellRuneCostEntry>                   sSpellRuneCostStore;
extern TRINITY_GAME_API DB2Storage<SpellTotemsEntry>                     sSpellTotemsStore;
extern TRINITY_GAME_API DB2Storage<SpellXSpellVisualEntry>               sSpellXSpellVisualStore;
extern TRINITY_GAME_API DB2Storage<TaxiNodesEntry>                       sTaxiNodesStore;
extern TRINITY_GAME_API DB2Storage<TaxiPathEntry>                        sTaxiPathStore;
extern TRINITY_GAME_API DB2Storage<TotemCategoryEntry>                   sTotemCategoryStore;
extern TRINITY_GAME_API DB2Storage<ToyEntry>                             sToyStore;
extern TRINITY_GAME_API DB2Storage<UnitPowerBarEntry>                    sUnitPowerBarStore;
extern TRINITY_GAME_API DB2Storage<WorldMapOverlayEntry>                 sWorldMapOverlayStore;

extern TRINITY_GAME_API TaxiMask                                         sTaxiNodesMask;
extern TRINITY_GAME_API TaxiMask                                         sOldContinentsNodesMask;
extern TRINITY_GAME_API TaxiMask                                         sHordeTaxiNodesMask;
extern TRINITY_GAME_API TaxiMask                                         sAllianceTaxiNodesMask;
extern TRINITY_GAME_API TaxiPathSetBySource                              sTaxiPathSetBySource;
extern TRINITY_GAME_API TaxiPathNodesByPath                              sTaxiPathNodesByPath;

struct HotfixNotify
{
    uint32 TableHash;
    uint32 Timestamp;
    uint32 Entry;
};

typedef std::vector<HotfixNotify> HotfixData;

#define DEFINE_DB2_SET_COMPARATOR(structure) \
    struct structure ## Comparator : public std::binary_function<structure const*, structure const*, bool> \
    { \
        bool operator()(structure const* left, structure const* right) const { return Compare(left, right); } \
        static bool Compare(structure const* left, structure const* right); \
    };

class TRINITY_GAME_API DB2Manager
{
public:
    DEFINE_DB2_SET_COMPARATOR(ChrClassesXPowerTypesEntry);
    DEFINE_DB2_SET_COMPARATOR(GlyphSlotEntry);
    DEFINE_DB2_SET_COMPARATOR(MountTypeXCapabilityEntry);

    typedef std::map<uint32 /*hash*/, DB2StorageBase*> StorageMap;
    typedef std::unordered_map<uint32 /*areaGroupId*/, std::vector<uint32/*areaId*/>> AreaGroupMemberContainer;
    typedef std::unordered_map<uint32, CharStartOutfitEntry const*> CharStartOutfitContainer;
    typedef std::set<GlyphSlotEntry const*, GlyphSlotEntryComparator> GlyphSlotContainer;
    typedef std::map<uint32 /*curveID*/, std::map<uint32/*index*/, CurvePointEntry const*, std::greater<uint32>>> HeirloomCurvesContainer;
    typedef std::vector<ItemBonusEntry const*> ItemBonusList;
    typedef std::unordered_map<uint32 /*bonusListId*/, ItemBonusList> ItemBonusListContainer;
    typedef std::unordered_multimap<uint32 /*itemId*/, uint32 /*bonusTreeId*/> ItemToBonusTreeContainer;
    typedef std::unordered_map<uint32 /*itemId | appearanceMod << 24*/, uint32> ItemDisplayIdContainer;
    typedef std::unordered_map<uint32, std::set<ItemBonusTreeNodeEntry const*>> ItemBonusTreeContainer;
    typedef std::unordered_map<uint32, std::vector<ItemSpecOverrideEntry const*>> ItemSpecOverridesContainer;
    typedef std::unordered_map<uint32, MountEntry const*> MountContainer;
    typedef std::set<MountTypeXCapabilityEntry const*, MountTypeXCapabilityEntryComparator> MountTypeXCapabilitySet;
    typedef std::unordered_map<uint32, MountTypeXCapabilitySet> MountCapabilitiesByTypeContainer;
    typedef std::unordered_map<uint32, std::array<std::vector<NameGenEntry const*>, 2>> NameGenContainer;
    typedef std::array<std::vector<boost::regex>, TOTAL_LOCALES + 1> NameValidationRegexContainer;
    typedef std::unordered_map<uint32, std::set<uint32>> PhaseGroupContainer;
    typedef std::unordered_map<uint32, std::vector<QuestPackageItemEntry const*>> QuestPackageItemContainer;
    typedef std::unordered_map<uint32, std::vector<SpecializationSpellsEntry const*>> SpecializationSpellsContainer;
    typedef std::unordered_map<uint32, std::vector<SpellPowerEntry const*>> SpellPowerContainer;
    typedef std::unordered_map<uint32, std::unordered_map<uint32, std::vector<SpellPowerEntry const*>>> SpellPowerDifficultyContainer;
    typedef std::vector<uint32> ToyItemIdsContainer;

    static DB2Manager& Instance()
    {
        static DB2Manager instance;
        return instance;
    }

    void LoadStores(std::string const& dataPath, uint32 defaultLocale);
    DB2StorageBase const* GetStorage(uint32 type) const;

    void LoadHotfixData();
    HotfixData const* GetHotfixData() const { return &_hotfixData; }
    time_t GetHotfixDate(uint32 entry, uint32 type) const;

    std::vector<uint32> GetAreasForGroup(uint32 areaGroupId) const;
    static char const* GetBroadcastTextValue(BroadcastTextEntry const* broadcastText, LocaleConstant locale = DEFAULT_LOCALE, uint8 gender = GENDER_MALE, bool forceGender = false);
    CharStartOutfitEntry const* GetCharStartOutfitEntry(uint8 race, uint8 class_, uint8 gender) const;
    uint32 GetPowerIndexByClass(uint32 powerType, uint32 classId) const;
    GlyphSlotContainer const& GetGlyphSlots() const { return _glyphSlots; }
    uint32 GetHeirloomItemLevel(uint32 curveId, uint32 level) const;
    ItemBonusList const* GetItemBonusList(uint32 bonusListId) const;
    std::set<uint32> GetItemBonusTree(uint32 itemId, uint32 itemBonusTreeMod) const;
    uint32 GetItemDisplayId(uint32 itemId, uint32 appearanceModId) const;
    std::vector<ItemSpecOverrideEntry const*> const* GetItemSpecOverrides(uint32 itemId) const;
    std::string GetNameGenEntry(uint8 race, uint8 gender, LocaleConstant locale) const;
    MountEntry const* GetMount(uint32 spellId) const;
    MountEntry const* GetMountById(uint32 id) const;
    MountTypeXCapabilitySet const* GetMountCapabilities(uint32 mountType) const;
    ResponseCodes ValidateName(std::string const& name, LocaleConstant locale) const;
    std::vector<QuestPackageItemEntry const*> const* GetQuestPackageItems(uint32 questPackageID) const;
    uint32 GetQuestUniqueBitFlag(uint32 questId);
    std::set<uint32> GetPhasesForGroup(uint32 group) const;
    std::vector<SpecializationSpellsEntry const*> const* GetSpecializationSpells(uint32 specId) const;
    std::vector<SpellPowerEntry const*> GetSpellPowers(uint32 spellId, Difficulty difficulty = DIFFICULTY_NONE, bool* hasDifficultyPowers = nullptr) const;
    bool GetToyItemIdMatch(uint32 toy) const;

private:
    StorageMap _stores;
    HotfixData _hotfixData;

    AreaGroupMemberContainer _areaGroupMembers;
    CharStartOutfitContainer _charStartOutfits;
    uint32 _powersByClass[MAX_CLASSES][MAX_POWERS];
    GlyphSlotContainer _glyphSlots;
    HeirloomCurvesContainer _heirloomCurvePoints;
    ItemBonusListContainer _itemBonusLists;
    ItemBonusTreeContainer _itemBonusTrees;
    ItemDisplayIdContainer _itemDisplayIDs;
    ItemToBonusTreeContainer _itemToBonusTree;
    ItemSpecOverridesContainer _itemSpecOverrides;
    MountContainer _mountsBySpellId;
    MountCapabilitiesByTypeContainer _mountCapabilitiesByType;
    NameGenContainer _nameGenData;
    NameValidationRegexContainer _nameValidators;
    PhaseGroupContainer _phasesByGroup;
    QuestPackageItemContainer _questPackages;
    SpecializationSpellsContainer _specializationSpellsBySpec;
    SpellPowerContainer _spellPowers;
    SpellPowerDifficultyContainer _spellPowerDifficulties;
    ToyItemIdsContainer _toys;
};

#define sDB2Manager DB2Manager::Instance()

#endif
