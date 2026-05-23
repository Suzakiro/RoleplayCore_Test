/*
 * This file is part of the RoleplayCore Project.
 * Housing System — Manager (Phase 1 + Phase 3, définitif)
 */

#ifndef HOUSING_MGR_H
#define HOUSING_MGR_H

#include "HousingDefines.h"
#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

class Player;

namespace Housing
{

class TC_GAME_API HousingMgr
{
private:
    HousingMgr()  = default;
    ~HousingMgr() = default;

public:
    static HousingMgr* instance();

    // =========================================================
    // Lifecycle
    // =========================================================
    void LoadNeighborhoods();
    void LoadPlayerHouse(ObjectGuid guid);
    void SavePlayerHouse(ObjectGuid guid);
    void UnloadPlayerHouse(ObjectGuid guid);

    // =========================================================
    // House management
    // =========================================================
    HousingError CreateHouse(Player* player, uint64 plotId, HouseSize size);
    HousingError DeleteHouse(Player* player);
    HousingError UpdateHouseSettings(Player* player, std::string const& name,
                                     std::string const& description,
                                     bool isPublic, bool allowVisits);

    // =========================================================
    // Decoration — Phase 1 (dirty-only)
    // =========================================================
    HousingError MoveDecoration(Player* player, uint64 decorId,
                                float x, float y, float z,
                                float rx, float ry, float rz, float rw);
    HousingError RemoveDecoration(Player* player, uint64 decorId);
    HousingError LockDecoration(Player* player, uint64 decorId, bool locked);

    // =========================================================
    // Decoration — Phase 3 (with immediate ID assignment)
    // =========================================================
    HousingError PlaceDecorationAndGetResult(Player* player, uint32 itemId,
                                             float x, float y, float z,
                                             float rx, float ry, float rz, float rw,
                                             float scale, DecorationType type,
                                             HousingDecoration& outDecor);

    // =========================================================
    // Fixture
    // =========================================================
    HousingError SetCoreFixture(Player* player, uint32 fixtureId);
    HousingError CreateFixture(Player* player, FixtureType type, uint32 fixtureId, uint32 slot);
    HousingError DeleteFixture(Player* player, uint64 fixtureId);
    HousingError CreateFixtureAndGetResult(Player* player, FixtureType type,
                                           uint32 fixtureId, uint32 slot,
                                           HousingFixture& outFixture);

    // =========================================================
    // Room
    // =========================================================
    HousingError AddRoom(Player* player, float x, float y, float z, float o, float sx, float sy);
    HousingError RemoveRoom(Player* player, uint64 roomId);
    HousingError SetRoomCeilingType(Player* player, uint64 roomId, RoomCeilingType type);
    HousingError SetRoomDoorType(Player* player, uint64 roomId, RoomDoorType type);
    HousingError SetRoomComponentTheme(Player* player, uint64 roomId, uint32 themeId);

    HousingError AddRoomAndGetResult(Player* player,
                                     float x, float y, float z, float o,
                                     float sx, float sy, HousingRoom& outRoom);
    HousingError MoveRoomAndGetResult(Player* player, uint64 roomId,
                                      float x, float y, float z, float o,
                                      HousingRoom& outRoom);
    HousingError RotateRoomAndGetResult(Player* player, uint64 roomId,
                                        float o, HousingRoom& outRoom);

    // =========================================================
    // House size / type
    // =========================================================
    HousingError SetHouseSize(Player* player, HouseSize newSize);
    HousingError SetHouseType(Player* player, uint32 houseTypeId);

    // =========================================================
    // Navigation
    // =========================================================
    HousingError TeleportToPlot(Player* player, uint64 plotId);
    HousingError TeleportToOwnHouse(Player* player);
    void         OnPlayerLeavePlot(Player* player);

    // =========================================================
    // Getters
    // =========================================================
    PlayerHouse*            GetPlayerHouse(ObjectGuid guid);
    HousingNeighborhood*    GetNeighborhood(uint64 id);
    HousingPlot*            GetPlot(uint64 plotId);
    HousingDecoration*      FindDecoration(PlayerHouse* house, uint64 decorId);

    // =========================================================
    // Queries
    // =========================================================
    bool    PlayerHasHouse(ObjectGuid guid) const;
    bool    PlotIsAvailable(uint64 plotId) const;
    uint64  GetNeighborhoodIdForPlot(uint64 plotId) const;

    // =========================================================
    // Edit mode
    // =========================================================
    void SetDecorEditMode(Player* player, bool enabled);
    void SetFixtureEditMode(Player* player, bool enabled);
    void SetRoomLayoutEditMode(Player* player, bool enabled);
    bool IsInDecorEditMode(ObjectGuid guid) const;
    bool IsInFixtureEditMode(ObjectGuid guid) const;
    bool IsInRoomLayoutEditMode(ObjectGuid guid) const;

private:
    // =========================================================
    // ID generators (atomic — initialized from DB MAX on startup)
    // =========================================================
    void    InitIdGenerators();
    uint64  NextDecorId()   { return _decorIdGen++;   }
    uint64  NextFixtureId() { return _fixtureIdGen++; }
    uint64  NextRoomId()    { return _roomIdGen++;    }

    std::atomic<uint64> _decorIdGen  { 1 };
    std::atomic<uint64> _fixtureIdGen{ 1 };
    std::atomic<uint64> _roomIdGen   { 1 };

    // =========================================================
    // DB internals
    // =========================================================
    uint64  _DB_InsertHouse(PlayerHouse const& house);
    void    _DB_UpdateHouse(PlayerHouse const& house);
    void    _DB_DeleteHouse(uint64 houseId);
    void    _DB_SaveDecorations(PlayerHouse& house);
    void    _DB_SaveFixtures(PlayerHouse& house);
    void    _DB_SaveRooms(PlayerHouse& house);
    void    _DB_UpdatePlotStatus(uint64 plotId, PlotStatus status, uint64 houseId);

    // =========================================================
    // Internal helpers
    // =========================================================
    HousingDecoration* _FindDecoration(PlayerHouse& house, uint64 decorId);
    HousingFixture*    _FindFixture(PlayerHouse& house, uint64 fixtureId);
    HousingRoom*       _FindRoom(PlayerHouse& house, uint64 roomId);

    // =========================================================
    // State
    // =========================================================
    std::unordered_map<uint64, HousingNeighborhood>  _neighborhoods;
    std::unordered_map<ObjectGuid, PlayerHouse>      _playerHouses;
    std::unordered_map<ObjectGuid, uint8>            _editModes;  // 0x1=decor, 0x2=fixture, 0x4=room

    mutable std::shared_mutex _neighborhoodLock;
    mutable std::shared_mutex _houseLock;
};

} // namespace Housing

#define sHousingMgr Housing::HousingMgr::instance()

#endif // HOUSING_MGR_H
