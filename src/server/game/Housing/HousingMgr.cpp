/*
 * This file is part of the RoleplayCore Project.
 * Housing System — Manager Implementation (Phase 1 + Phase 3, définitif)
 *
 * Correction principale : API PreparedStatement → setUInt8/setUInt32/setUInt64/
 *                         setString/setBool/setFloat (pas SetData)
 */

#include "HousingMgr.h"
#include "CharacterDatabase.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "PhasingHandler.h"
#include "Player.h"
#include "RoleplayDatabase.h"
#include "Timer.h"

namespace Housing
{

HousingMgr* HousingMgr::instance()
{
    static HousingMgr instance;
    return &instance;
}

// =============================================================================
// Lifecycle
// =============================================================================

void HousingMgr::LoadNeighborhoods()
{
    uint32 oldMSTime = getMSTime();

    std::unique_lock lock(_neighborhoodLock);
    _neighborhoods.clear();

    // 1) Neighborhoods
    {
        PreparedStatement<RoleplayDatabaseConnection>* stmt =
            RoleplayDatabase.GetPreparedStatement(Roleplay_SEL_HOUSING_NEIGHBORHOODS);
        PreparedQueryResult result = RoleplayDatabase.Query(stmt);

        if (!result)
        {
            TC_LOG_INFO("server.loading", ">> Loaded 0 housing neighborhoods.");
            InitIdGenerators();
            return;
        }

        do
        {
            Field* f = result->Fetch();
            HousingNeighborhood nb;
            nb.Id           = f[0].GetUInt64();
            nb.Name         = f[1].GetString();
            nb.MapId        = f[2].GetUInt32();
            nb.MaxPlots     = f[3].GetUInt32();
            nb.IsPublic     = f[4].GetBool();
            nb.OwnerGuildId = f[5].GetUInt64();
            nb.Type         = static_cast<NeighborhoodType>(f[6].GetUInt8());
            nb.PhaseId      = HOUSING_PHASE_BASE + static_cast<uint32>(nb.Id);
            _neighborhoods.emplace(nb.Id, std::move(nb));
        } while (result->NextRow());
    }

    // 2) Plots
    {
        PreparedStatement<RoleplayDatabaseConnection>* stmt =
            RoleplayDatabase.GetPreparedStatement(Roleplay_SEL_HOUSING_PLOTS);
        PreparedQueryResult result = RoleplayDatabase.Query(stmt);

        uint32 plotCount = 0;
        if (result)
        {
            do
            {
                Field* f = result->Fetch();
                HousingPlot plot;
                plot.Id             = f[0].GetUInt64();
                plot.NeighborhoodId = f[1].GetUInt64();
                plot.PlotIndex      = f[2].GetUInt32();
                plot.PosX           = f[3].GetFloat();
                plot.PosY           = f[4].GetFloat();
                plot.PosZ           = f[5].GetFloat();
                plot.Orientation    = f[6].GetFloat();
                plot.MapId          = f[7].GetUInt32();
                plot.Size           = static_cast<HouseSize>(f[8].GetUInt8());
                plot.Status         = static_cast<PlotStatus>(f[9].GetUInt8());
                plot.HouseId        = f[10].GetUInt64();

                auto it = _neighborhoods.find(plot.NeighborhoodId);
                if (it != _neighborhoods.end())
                {
                    it->second.Plots.push_back(std::move(plot));
                    ++plotCount;
                }
            } while (result->NextRow());
        }

        TC_LOG_INFO("server.loading", ">> Loaded %zu housing neighborhoods and %u plots in %u ms.",
            _neighborhoods.size(), plotCount, GetMSTimeDiffToNow(oldMSTime));
    }

    InitIdGenerators();
}

void HousingMgr::InitIdGenerators()
{
    {
        PreparedStatement<CharacterDatabaseConnection>* stmt =
            CharacterDatabase.GetPreparedStatement(CHAR_SEL_HOUSING_MAX_DECOR_ID);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);
        _decorIdGen.store(result ? result->Fetch()[0].GetUInt64() + 1 : 1);
    }
    {
        PreparedStatement<CharacterDatabaseConnection>* stmt =
            CharacterDatabase.GetPreparedStatement(CHAR_SEL_HOUSING_MAX_FIXTURE_ID);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);
        _fixtureIdGen.store(result ? result->Fetch()[0].GetUInt64() + 1 : 1);
    }
    {
        PreparedStatement<CharacterDatabaseConnection>* stmt =
            CharacterDatabase.GetPreparedStatement(CHAR_SEL_HOUSING_MAX_ROOM_ID);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);
        _roomIdGen.store(result ? result->Fetch()[0].GetUInt64() + 1 : 1);
    }

    TC_LOG_INFO("housing", ">> Housing ID generators: decor=%lu fixture=%lu room=%lu",
        _decorIdGen.load(), _fixtureIdGen.load(), _roomIdGen.load());
}

void HousingMgr::LoadPlayerHouse(ObjectGuid guid)
{
    // 1) House record
    PreparedStatement<CharacterDatabaseConnection>* stmt =
        CharacterDatabase.GetPreparedStatement(CHAR_SEL_HOUSING_BY_GUID);
    stmt->setUInt64(0, guid.GetCounter());

    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    if (!result)
        return;

    PlayerHouse house;
    {
        Field* f = result->Fetch();
        house.Id          = f[0].GetUInt64();
        house.OwnerGuid   = guid;
        house.PlotId      = f[1].GetUInt64();
        house.HouseType   = f[2].GetUInt32();
        house.Size        = static_cast<HouseSize>(f[3].GetUInt8());
        house.Status      = static_cast<HouseStatus>(f[4].GetUInt8());
        house.Name        = f[5].GetString();
        house.Description = f[6].GetString();
        house.IsPublic    = f[7].GetBool();
        house.AllowVisits = f[8].GetBool();
    }

    // 2) Decorations
    {
        PreparedStatement<CharacterDatabaseConnection>* ds =
            CharacterDatabase.GetPreparedStatement(CHAR_SEL_HOUSING_DECORATIONS);
        ds->setUInt64(0, house.Id);
        PreparedQueryResult dr = CharacterDatabase.Query(ds);
        if (dr)
        {
            do
            {
                Field* f = dr->Fetch();
                HousingDecoration d;
                d.Id       = f[0].GetUInt64();
                d.HouseId  = house.Id;
                d.ItemId   = f[1].GetUInt32();
                d.PosX     = f[2].GetFloat();  d.PosY = f[3].GetFloat();  d.PosZ = f[4].GetFloat();
                d.RotX     = f[5].GetFloat();  d.RotY = f[6].GetFloat();
                d.RotZ     = f[7].GetFloat();  d.RotW = f[8].GetFloat();
                d.Scale    = f[9].GetFloat();
                d.Type     = static_cast<DecorationType>(f[10].GetUInt32());
                d.IsLocked = f[11].GetBool();
                house.Decorations.push_back(std::move(d));
            } while (dr->NextRow());
        }
    }

    // 3) Fixtures
    {
        PreparedStatement<CharacterDatabaseConnection>* fs =
            CharacterDatabase.GetPreparedStatement(CHAR_SEL_HOUSING_FIXTURES);
        fs->setUInt64(0, house.Id);
        PreparedQueryResult fr = CharacterDatabase.Query(fs);
        if (fr)
        {
            do
            {
                Field* f = fr->Fetch();
                HousingFixture fix;
                fix.Id        = f[0].GetUInt64();
                fix.HouseId   = house.Id;
                fix.Type      = static_cast<FixtureType>(f[1].GetUInt32());
                fix.FixtureId = f[2].GetUInt32();
                fix.Slot      = f[3].GetUInt32();
                house.Fixtures.push_back(std::move(fix));
            } while (fr->NextRow());
        }
    }

    // 4) Rooms
    {
        PreparedStatement<CharacterDatabaseConnection>* rs =
            CharacterDatabase.GetPreparedStatement(CHAR_SEL_HOUSING_ROOMS);
        rs->setUInt64(0, house.Id);
        PreparedQueryResult rr = CharacterDatabase.Query(rs);
        if (rr)
        {
            do
            {
                Field* f = rr->Fetch();
                HousingRoom room;
                room.Id             = f[0].GetUInt64();
                room.HouseId        = house.Id;
                room.RoomIndex      = f[1].GetUInt32();
                room.PosX           = f[2].GetFloat(); room.PosY = f[3].GetFloat(); room.PosZ = f[4].GetFloat();
                room.Orientation    = f[5].GetFloat();
                room.SizeX          = f[6].GetFloat(); room.SizeY = f[7].GetFloat();
                room.CeilingType    = static_cast<RoomCeilingType>(f[8].GetUInt32());
                room.DoorType       = static_cast<RoomDoorType>(f[9].GetUInt32());
                room.ComponentTheme = f[10].GetUInt32();
                house.Rooms.push_back(std::move(room));
            } while (rr->NextRow());
        }
    }

    std::unique_lock lock(_houseLock);
    _playerHouses.emplace(guid, std::move(house));
}

void HousingMgr::SavePlayerHouse(ObjectGuid guid)
{
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(guid);
    if (it == _playerHouses.end())
        return;

    PlayerHouse& house = it->second;

    if (house.IsNew)
    {
        house.Id  = _DB_InsertHouse(house);
        house.IsNew   = false;
        house.IsDirty = false;
    }
    else if (house.IsDirty)
    {
        _DB_UpdateHouse(house);
        house.IsDirty = false;
    }

    _DB_SaveDecorations(house);
    _DB_SaveFixtures(house);
    _DB_SaveRooms(house);
}

void HousingMgr::UnloadPlayerHouse(ObjectGuid guid)
{
    std::unique_lock lock(_houseLock);
    _playerHouses.erase(guid);
    _editModes.erase(guid);
}

// =============================================================================
// House management
// =============================================================================

HousingError HousingMgr::CreateHouse(Player* player, uint64 plotId, HouseSize size)
{
    ASSERT(player);
    if (PlayerHasHouse(player->GetGUID()))
        return HousingError::HouseAlreadyExists;
    if (!PlotIsAvailable(plotId))
        return HousingError::PlotNotAvailable;
    if (size >= HouseSize::Max)
        return HousingError::InvalidHouseSize;

    PlayerHouse house;
    house.OwnerGuid   = player->GetGUID();
    house.PlotId      = plotId;
    house.Size        = size;
    house.Status      = HouseStatus::Active;
    house.Name        = player->GetName() + "'s Home";
    house.IsPublic    = false;
    house.AllowVisits = true;
    house.IsNew       = true;

    house.Id = _DB_InsertHouse(house);
    house.IsNew = false;

    _DB_UpdatePlotStatus(plotId, PlotStatus::Occupied, house.Id);

    {
        std::unique_lock nbLock(_neighborhoodLock);
        for (auto& [nbId, nb] : _neighborhoods)
            for (auto& plot : nb.Plots)
                if (plot.Id == plotId) { plot.Status = PlotStatus::Occupied; plot.HouseId = house.Id; break; }
    }

    {
        std::unique_lock hLock(_houseLock);
        _playerHouses.emplace(player->GetGUID(), std::move(house));
    }

    return HousingError::Success;
}

HousingError HousingMgr::DeleteHouse(Player* player)
{
    ASSERT(player);
    std::unique_lock hLock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end())
        return HousingError::NoHouse;

    uint64 plotId  = it->second.PlotId;
    uint64 houseId = it->second.Id;

    _DB_DeleteHouse(houseId);
    _DB_UpdatePlotStatus(plotId, PlotStatus::Empty, 0);

    {
        std::unique_lock nbLock(_neighborhoodLock);
        for (auto& [nbId, nb] : _neighborhoods)
            for (auto& plot : nb.Plots)
                if (plot.Id == plotId) { plot.Status = PlotStatus::Empty; plot.HouseId = 0; break; }
    }

    _playerHouses.erase(it);
    return HousingError::Success;
}

HousingError HousingMgr::UpdateHouseSettings(Player* player, std::string const& name,
                                              std::string const& description,
                                              bool isPublic, bool allowVisits)
{
    ASSERT(player);
    if (name.empty() || name.size() > MAX_HOUSE_NAME_LEN)
        return HousingError::InvalidHouseName;

    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end())
        return HousingError::NoHouse;

    PlayerHouse& h = it->second;
    h.Name        = name;
    h.Description = description.size() <= MAX_HOUSE_DESC_LEN ? description : description.substr(0, MAX_HOUSE_DESC_LEN);
    h.IsPublic    = isPublic;
    h.AllowVisits = allowVisits;
    h.IsDirty     = true;
    return HousingError::Success;
}

// =============================================================================
// Decoration
// =============================================================================

HousingError HousingMgr::PlaceDecorationAndGetResult(Player* player, uint32 itemId,
                                                      float x, float y, float z,
                                                      float rx, float ry, float rz, float rw,
                                                      float scale, DecorationType type,
                                                      HousingDecoration& outDecor)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end())
        return HousingError::NoHouse;

    PlayerHouse& house = it->second;
    if (house.GetActiveDecorationCount() >= house.GetMaxDecorations())
        return HousingError::MaxDecorationsReached;

    HousingDecoration d;
    d.Id       = NextDecorId();
    d.HouseId  = house.Id;
    d.ItemId   = itemId;
    d.PosX = x;  d.PosY = y;  d.PosZ = z;
    d.RotX = rx; d.RotY = ry; d.RotZ = rz; d.RotW = rw;
    d.Scale    = scale;
    d.Type     = type;
    d.IsLocked = false;
    d.IsNew    = true;
    d.IsDirty  = true;

    outDecor = d;
    house.Decorations.push_back(std::move(d));
    return HousingError::Success;
}

HousingError HousingMgr::MoveDecoration(Player* player, uint64 decorId,
                                         float x, float y, float z,
                                         float rx, float ry, float rz, float rw)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end())
        return HousingError::NoHouse;

    HousingDecoration* d = _FindDecoration(it->second, decorId);
    if (!d)
        return HousingError::DecorationNotFound;
    if (d->IsLocked)
        return HousingError::PermissionDenied;

    d->PosX = x; d->PosY = y; d->PosZ = z;
    d->RotX = rx; d->RotY = ry; d->RotZ = rz; d->RotW = rw;
    d->IsDirty = true;
    return HousingError::Success;
}

HousingError HousingMgr::RemoveDecoration(Player* player, uint64 decorId)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end())
        return HousingError::NoHouse;

    HousingDecoration* d = _FindDecoration(it->second, decorId);
    if (!d)
        return HousingError::DecorationNotFound;
    if (d->IsLocked)
        return HousingError::PermissionDenied;

    d->IsDeleted = true;
    d->IsDirty   = true;
    return HousingError::Success;
}

HousingError HousingMgr::LockDecoration(Player* player, uint64 decorId, bool locked)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end())
        return HousingError::NoHouse;

    HousingDecoration* d = _FindDecoration(it->second, decorId);
    if (!d)
        return HousingError::DecorationNotFound;

    d->IsLocked = locked;
    d->IsDirty  = true;
    return HousingError::Success;
}

// =============================================================================
// Fixture
// =============================================================================

HousingError HousingMgr::SetCoreFixture(Player* player, uint32 fixtureId)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end())
        return HousingError::NoHouse;

    for (auto& f : it->second.Fixtures)
    {
        if (f.Type == FixtureType::Core && !f.IsDeleted)
        {
            f.FixtureId = fixtureId;
            f.IsDirty   = true;
            return HousingError::Success;
        }
    }

    HousingFixture fix;
    fix.Id        = NextFixtureId();
    fix.HouseId   = it->second.Id;
    fix.Type      = FixtureType::Core;
    fix.FixtureId = fixtureId;
    fix.Slot      = 0;
    fix.IsNew     = true;
    fix.IsDirty   = true;
    it->second.Fixtures.push_back(std::move(fix));
    return HousingError::Success;
}

HousingError HousingMgr::CreateFixture(Player* player, FixtureType type, uint32 fixtureId, uint32 slot)
{
    HousingFixture dummy;
    return CreateFixtureAndGetResult(player, type, fixtureId, slot, dummy);
}

HousingError HousingMgr::CreateFixtureAndGetResult(Player* player, FixtureType type,
                                                    uint32 fixtureId, uint32 slot,
                                                    HousingFixture& outFixture)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end())
        return HousingError::NoHouse;

    HousingFixture fix;
    fix.Id        = NextFixtureId();
    fix.HouseId   = it->second.Id;
    fix.Type      = type;
    fix.FixtureId = fixtureId;
    fix.Slot      = slot;
    fix.IsNew     = true;
    fix.IsDirty   = true;
    outFixture    = fix;
    it->second.Fixtures.push_back(std::move(fix));
    return HousingError::Success;
}

HousingError HousingMgr::DeleteFixture(Player* player, uint64 fixtureId)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end())
        return HousingError::NoHouse;

    HousingFixture* f = _FindFixture(it->second, fixtureId);
    if (!f)
        return HousingError::InvalidFixture;
    f->IsDeleted = true;
    f->IsDirty   = true;
    return HousingError::Success;
}

// =============================================================================
// Room
// =============================================================================

HousingError HousingMgr::AddRoom(Player* player, float x, float y, float z, float o, float sx, float sy)
{
    HousingRoom dummy;
    return AddRoomAndGetResult(player, x, y, z, o, sx, sy, dummy);
}

HousingError HousingMgr::AddRoomAndGetResult(Player* player,
                                              float x, float y, float z, float o,
                                              float sx, float sy, HousingRoom& outRoom)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end())
        return HousingError::NoHouse;

    PlayerHouse& house = it->second;
    uint32 active = 0;
    for (auto const& r : house.Rooms)
        if (!r.IsDeleted) ++active;

    if (active >= house.GetMaxRooms())
        return HousingError::PermissionDenied;

    HousingRoom room;
    room.Id          = NextRoomId();
    room.HouseId     = house.Id;
    room.RoomIndex   = active;
    room.PosX = x; room.PosY = y; room.PosZ = z;
    room.Orientation = o;
    room.SizeX = sx; room.SizeY = sy;
    room.IsNew    = true;
    room.IsDirty  = true;
    outRoom = room;
    house.Rooms.push_back(std::move(room));
    return HousingError::Success;
}

HousingError HousingMgr::RemoveRoom(Player* player, uint64 roomId)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end())
        return HousingError::NoHouse;

    HousingRoom* r = _FindRoom(it->second, roomId);
    if (!r) return HousingError::InvalidRoom;
    r->IsDeleted = true;
    r->IsDirty   = true;
    return HousingError::Success;
}

HousingError HousingMgr::MoveRoomAndGetResult(Player* player, uint64 roomId,
                                               float x, float y, float z, float o,
                                               HousingRoom& outRoom)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end())
        return HousingError::NoHouse;

    HousingRoom* r = _FindRoom(it->second, roomId);
    if (!r) return HousingError::InvalidRoom;
    r->PosX = x; r->PosY = y; r->PosZ = z; r->Orientation = o;
    r->IsDirty = true;
    outRoom = *r;
    return HousingError::Success;
}

HousingError HousingMgr::RotateRoomAndGetResult(Player* player, uint64 roomId,
                                                 float o, HousingRoom& outRoom)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end())
        return HousingError::NoHouse;

    HousingRoom* r = _FindRoom(it->second, roomId);
    if (!r) return HousingError::InvalidRoom;
    r->Orientation = o;
    r->IsDirty     = true;
    outRoom = *r;
    return HousingError::Success;
}

HousingError HousingMgr::SetRoomCeilingType(Player* player, uint64 roomId, RoomCeilingType type)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end()) return HousingError::NoHouse;
    HousingRoom* r = _FindRoom(it->second, roomId);
    if (!r) return HousingError::InvalidRoom;
    r->CeilingType = type; r->IsDirty = true;
    return HousingError::Success;
}

HousingError HousingMgr::SetRoomDoorType(Player* player, uint64 roomId, RoomDoorType type)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end()) return HousingError::NoHouse;
    HousingRoom* r = _FindRoom(it->second, roomId);
    if (!r) return HousingError::InvalidRoom;
    r->DoorType = type; r->IsDirty = true;
    return HousingError::Success;
}

HousingError HousingMgr::SetRoomComponentTheme(Player* player, uint64 roomId, uint32 themeId)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end()) return HousingError::NoHouse;
    HousingRoom* r = _FindRoom(it->second, roomId);
    if (!r) return HousingError::InvalidRoom;
    r->ComponentTheme = themeId; r->IsDirty = true;
    return HousingError::Success;
}

// =============================================================================
// House size / type
// =============================================================================

HousingError HousingMgr::SetHouseSize(Player* player, HouseSize newSize)
{
    ASSERT(player);
    if (newSize >= HouseSize::Max) return HousingError::InvalidHouseSize;
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end()) return HousingError::NoHouse;
    it->second.Size    = newSize;
    it->second.IsDirty = true;
    return HousingError::Success;
}

HousingError HousingMgr::SetHouseType(Player* player, uint32 houseTypeId)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    auto it = _playerHouses.find(player->GetGUID());
    if (it == _playerHouses.end()) return HousingError::NoHouse;
    it->second.HouseType = houseTypeId;
    it->second.IsDirty   = true;
    return HousingError::Success;
}

// =============================================================================
// Navigation
// =============================================================================

HousingError HousingMgr::TeleportToPlot(Player* player, uint64 plotId)
{
    ASSERT(player);
    HousingPlot* plot = GetPlot(plotId);
    if (!plot) return HousingError::InvalidPlot;

    HousingNeighborhood* nb = GetNeighborhood(plot->NeighborhoodId);
    if (!nb) return HousingError::InternalError;

    PhasingHandler::AddPhase(player, nb->PhaseId, true);
    player->TeleportTo(plot->MapId, plot->PosX, plot->PosY, plot->PosZ, plot->Orientation);
    return HousingError::Success;
}

HousingError HousingMgr::TeleportToOwnHouse(Player* player)
{
    ASSERT(player);
    PlayerHouse* house = GetPlayerHouse(player->GetGUID());
    if (!house) return HousingError::NoHouse;
    return TeleportToPlot(player, house->PlotId);
}

void HousingMgr::OnPlayerLeavePlot(Player* player)
{
    ASSERT(player);
    std::shared_lock lock(_neighborhoodLock);
    for (auto const& [nbId, nb] : _neighborhoods)
        PhasingHandler::RemovePhase(player, nb.PhaseId, true);
}

// =============================================================================
// Getters
// =============================================================================

PlayerHouse* HousingMgr::GetPlayerHouse(ObjectGuid guid)
{
    std::shared_lock lock(_houseLock);
    auto it = _playerHouses.find(guid);
    return (it != _playerHouses.end()) ? &it->second : nullptr;
}

HousingNeighborhood* HousingMgr::GetNeighborhood(uint64 id)
{
    std::shared_lock lock(_neighborhoodLock);
    auto it = _neighborhoods.find(id);
    return (it != _neighborhoods.end()) ? &it->second : nullptr;
}

HousingPlot* HousingMgr::GetPlot(uint64 plotId)
{
    std::shared_lock lock(_neighborhoodLock);
    for (auto& [nbId, nb] : _neighborhoods)
        for (auto& plot : nb.Plots)
            if (plot.Id == plotId) return &plot;
    return nullptr;
}

HousingDecoration* HousingMgr::FindDecoration(PlayerHouse* house, uint64 decorId)
{
    if (!house) return nullptr;
    return _FindDecoration(*house, decorId);
}

// =============================================================================
// Queries
// =============================================================================

bool HousingMgr::PlayerHasHouse(ObjectGuid guid) const
{
    std::shared_lock lock(_houseLock);
    return _playerHouses.count(guid) > 0;
}

bool HousingMgr::PlotIsAvailable(uint64 plotId) const
{
    std::shared_lock lock(_neighborhoodLock);
    for (auto const& [nbId, nb] : _neighborhoods)
        for (auto const& plot : nb.Plots)
            if (plot.Id == plotId) return plot.Status == PlotStatus::Empty;
    return false;
}

uint64 HousingMgr::GetNeighborhoodIdForPlot(uint64 plotId) const
{
    std::shared_lock lock(_neighborhoodLock);
    for (auto const& [nbId, nb] : _neighborhoods)
        for (auto const& plot : nb.Plots)
            if (plot.Id == plotId) return nb.Id;
    return 0;
}

// =============================================================================
// Edit mode
// =============================================================================

void HousingMgr::SetDecorEditMode(Player* player, bool enabled)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    if (enabled) _editModes[player->GetGUID()] |= 0x1;
    else         _editModes[player->GetGUID()] &= ~uint8(0x1);
}

void HousingMgr::SetFixtureEditMode(Player* player, bool enabled)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    if (enabled) _editModes[player->GetGUID()] |= 0x2;
    else         _editModes[player->GetGUID()] &= ~uint8(0x2);
}

void HousingMgr::SetRoomLayoutEditMode(Player* player, bool enabled)
{
    ASSERT(player);
    std::unique_lock lock(_houseLock);
    if (enabled) _editModes[player->GetGUID()] |= 0x4;
    else         _editModes[player->GetGUID()] &= ~uint8(0x4);
}

bool HousingMgr::IsInDecorEditMode(ObjectGuid guid) const
{
    std::shared_lock lock(_houseLock);
    auto it = _editModes.find(guid);
    return (it != _editModes.end()) && (it->second & 0x1);
}

bool HousingMgr::IsInFixtureEditMode(ObjectGuid guid) const
{
    std::shared_lock lock(_houseLock);
    auto it = _editModes.find(guid);
    return (it != _editModes.end()) && (it->second & 0x2);
}

bool HousingMgr::IsInRoomLayoutEditMode(ObjectGuid guid) const
{
    std::shared_lock lock(_houseLock);
    auto it = _editModes.find(guid);
    return (it != _editModes.end()) && (it->second & 0x4);
}

// =============================================================================
// DB internals — setUInt8/setUInt32/setUInt64/setString/setBool/setFloat
// =============================================================================

uint64 HousingMgr::_DB_InsertHouse(PlayerHouse const& house)
{
    PreparedStatement<CharacterDatabaseConnection>* stmt =
        CharacterDatabase.GetPreparedStatement(CHAR_INS_HOUSING);
    stmt->setUInt64(0, house.OwnerGuid.GetCounter());
    stmt->setUInt64(1, house.PlotId);
    stmt->setUInt32(2, house.HouseType);
    stmt->setUInt8 (3, static_cast<uint8>(house.Size));
    stmt->setUInt8 (4, static_cast<uint8>(house.Status));
    stmt->setString(5, house.Name);
    stmt->setString(6, house.Description);
    stmt->setBool  (7, house.IsPublic);
    stmt->setBool  (8, house.AllowVisits);
    CharacterDatabase.DirectExecute(stmt);

    PreparedStatement<CharacterDatabaseConnection>* sel =
        CharacterDatabase.GetPreparedStatement(CHAR_SEL_HOUSING_BY_GUID);
    sel->setUInt64(0, house.OwnerGuid.GetCounter());
    PreparedQueryResult result = CharacterDatabase.Query(sel);
    return result ? result->Fetch()[0].GetUInt64() : 0;
}

void HousingMgr::_DB_UpdateHouse(PlayerHouse const& house)
{
    PreparedStatement<CharacterDatabaseConnection>* stmt =
        CharacterDatabase.GetPreparedStatement(CHAR_UPD_HOUSING);
    stmt->setUInt32(0, house.HouseType);
    stmt->setUInt8 (1, static_cast<uint8>(house.Size));
    stmt->setUInt8 (2, static_cast<uint8>(house.Status));
    stmt->setString(3, house.Name);
    stmt->setString(4, house.Description);
    stmt->setBool  (5, house.IsPublic);
    stmt->setBool  (6, house.AllowVisits);
    stmt->setUInt64(7, house.Id);
    CharacterDatabase.Execute(stmt);
}

void HousingMgr::_DB_DeleteHouse(uint64 houseId)
{
    PreparedStatement<CharacterDatabaseConnection>* stmt =
        CharacterDatabase.GetPreparedStatement(CHAR_DEL_HOUSING);
    stmt->setUInt64(0, houseId);
    CharacterDatabase.Execute(stmt);
}

void HousingMgr::_DB_SaveDecorations(PlayerHouse& house)
{
    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    for (auto& d : house.Decorations)
    {
        if (!d.IsDirty) continue;

        if (d.IsDeleted)
        {
            if (d.Id)
            {
                auto* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_HOUSING_DECORATION);
                stmt->setUInt64(0, d.Id);
                trans->Append(stmt);
            }
        }
        else if (d.IsNew)
        {
            auto* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_HOUSING_DECORATION);
            stmt->setUInt64(0, d.Id);
            stmt->setUInt64(1, d.HouseId);
            stmt->setUInt32(2, d.ItemId);
            stmt->setFloat (3, d.PosX);  stmt->setFloat(4, d.PosY);  stmt->setFloat(5, d.PosZ);
            stmt->setFloat (6, d.RotX);  stmt->setFloat(7, d.RotY);
            stmt->setFloat (8, d.RotZ);  stmt->setFloat(9, d.RotW);
            stmt->setFloat (10, d.Scale);
            stmt->setUInt32(11, static_cast<uint32>(d.Type));
            stmt->setBool  (12, d.IsLocked);
            trans->Append(stmt);
        }
        else
        {
            auto* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_HOUSING_DECORATION);
            stmt->setFloat(0, d.PosX); stmt->setFloat(1, d.PosY); stmt->setFloat(2, d.PosZ);
            stmt->setFloat(3, d.RotX); stmt->setFloat(4, d.RotY);
            stmt->setFloat(5, d.RotZ); stmt->setFloat(6, d.RotW);
            stmt->setFloat(7, d.Scale);
            stmt->setBool (8, d.IsLocked);
            stmt->setUInt64(9, d.Id);
            trans->Append(stmt);
        }
        d.IsDirty = false; d.IsNew = false;
    }

    house.Decorations.erase(
        std::remove_if(house.Decorations.begin(), house.Decorations.end(),
            [](HousingDecoration const& d) { return d.IsDeleted; }),
        house.Decorations.end());

    CharacterDatabase.CommitTransaction(trans);
}

void HousingMgr::_DB_SaveFixtures(PlayerHouse& house)
{
    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    for (auto& f : house.Fixtures)
    {
        if (!f.IsDirty) continue;

        if (f.IsDeleted)
        {
            if (f.Id)
            {
                auto* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_HOUSING_FIXTURE);
                stmt->setUInt64(0, f.Id);
                trans->Append(stmt);
            }
        }
        else if (f.IsNew)
        {
            auto* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_HOUSING_FIXTURE);
            stmt->setUInt64(0, f.Id);
            stmt->setUInt64(1, f.HouseId);
            stmt->setUInt32(2, static_cast<uint32>(f.Type));
            stmt->setUInt32(3, f.FixtureId);
            stmt->setUInt32(4, f.Slot);
            trans->Append(stmt);
        }
        else
        {
            auto* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_HOUSING_FIXTURE);
            stmt->setUInt32(0, f.FixtureId);
            stmt->setUInt32(1, f.Slot);
            stmt->setUInt64(2, f.Id);
            trans->Append(stmt);
        }
        f.IsDirty = false; f.IsNew = false;
    }

    house.Fixtures.erase(
        std::remove_if(house.Fixtures.begin(), house.Fixtures.end(),
            [](HousingFixture const& f) { return f.IsDeleted; }),
        house.Fixtures.end());

    CharacterDatabase.CommitTransaction(trans);
}

void HousingMgr::_DB_SaveRooms(PlayerHouse& house)
{
    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    for (auto& r : house.Rooms)
    {
        if (!r.IsDirty) continue;

        if (r.IsDeleted)
        {
            if (r.Id)
            {
                auto* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_HOUSING_ROOM);
                stmt->setUInt64(0, r.Id);
                trans->Append(stmt);
            }
        }
        else if (r.IsNew)
        {
            auto* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_HOUSING_ROOM);
            stmt->setUInt64(0, r.Id);
            stmt->setUInt64(1, r.HouseId);
            stmt->setUInt32(2, r.RoomIndex);
            stmt->setFloat (3, r.PosX);  stmt->setFloat(4, r.PosY);  stmt->setFloat(5, r.PosZ);
            stmt->setFloat (6, r.Orientation);
            stmt->setFloat (7, r.SizeX); stmt->setFloat(8, r.SizeY);
            stmt->setUInt32(9,  static_cast<uint32>(r.CeilingType));
            stmt->setUInt32(10, static_cast<uint32>(r.DoorType));
            stmt->setUInt32(11, r.ComponentTheme);
            trans->Append(stmt);
        }
        else
        {
            auto* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_HOUSING_ROOM);
            stmt->setFloat (0, r.PosX);  stmt->setFloat(1, r.PosY);  stmt->setFloat(2, r.PosZ);
            stmt->setFloat (3, r.Orientation);
            stmt->setFloat (4, r.SizeX); stmt->setFloat(5, r.SizeY);
            stmt->setUInt32(6,  static_cast<uint32>(r.CeilingType));
            stmt->setUInt32(7,  static_cast<uint32>(r.DoorType));
            stmt->setUInt32(8,  r.ComponentTheme);
            stmt->setUInt64(9,  r.Id);
            trans->Append(stmt);
        }
        r.IsDirty = false; r.IsNew = false;
    }

    house.Rooms.erase(
        std::remove_if(house.Rooms.begin(), house.Rooms.end(),
            [](HousingRoom const& r) { return r.IsDeleted; }),
        house.Rooms.end());

    CharacterDatabase.CommitTransaction(trans);
}

void HousingMgr::_DB_UpdatePlotStatus(uint64 plotId, PlotStatus status, uint64 houseId)
{
    PreparedStatement<RoleplayDatabaseConnection>* stmt =
        RoleplayDatabase.GetPreparedStatement(Roleplay_UPD_HOUSING_PLOT_STATUS);
    stmt->setUInt8 (0, static_cast<uint8>(status));
    stmt->setUInt64(1, houseId);
    stmt->setUInt64(2, plotId);
    RoleplayDatabase.Execute(stmt);
}

// =============================================================================
// Internal helpers
// =============================================================================

HousingDecoration* HousingMgr::_FindDecoration(PlayerHouse& house, uint64 decorId)
{
    for (auto& d : house.Decorations)
        if (d.Id == decorId && !d.IsDeleted) return &d;
    return nullptr;
}

HousingFixture* HousingMgr::_FindFixture(PlayerHouse& house, uint64 fixtureId)
{
    for (auto& f : house.Fixtures)
        if (f.Id == fixtureId && !f.IsDeleted) return &f;
    return nullptr;
}

HousingRoom* HousingMgr::_FindRoom(PlayerHouse& house, uint64 roomId)
{
    for (auto& r : house.Rooms)
        if (r.Id == roomId && !r.IsDeleted) return &r;
    return nullptr;
}

} // namespace Housing
