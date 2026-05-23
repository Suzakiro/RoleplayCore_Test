/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * Housing System Handlers — Phase 2 + Phase 3
 */

#include "HousingMgr.h"
#include "HousingPackets.h"
#include "Item.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "WorldSession.h"

// =========================================================
// Internal helpers
// =========================================================

namespace
{
    WorldPackets::Housing::HouseInfo BuildHouseInfo(Housing::PlayerHouse const& house)
    {
        WorldPackets::Housing::HouseInfo info;
        info.HouseID        = house.Id;
        info.PlotID         = house.PlotId;
        info.NeighborhoodID = sHousingMgr->GetNeighborhoodIdForPlot(house.PlotId);
        info.Status         = static_cast<uint8>(house.Status);
        info.HouseSize      = static_cast<uint8>(house.Size);
        info.Name           = house.Name;
        info.Description    = house.Description;
        info.IsPublic       = house.IsPublic;
        info.AllowVisits    = house.AllowVisits;
        return info;
    }

    WorldPackets::Housing::HouseDecorInfo BuildDecorInfo(Housing::HousingDecoration const& d)
    {
        WorldPackets::Housing::HouseDecorInfo info;
        info.DecorID  = d.Id;
        info.ItemID   = d.ItemId;
        info.PosX     = d.PosX;    info.PosY  = d.PosY;    info.PosZ  = d.PosZ;
        info.RotX     = d.RotX;    info.RotY  = d.RotY;    info.RotZ  = d.RotZ;    info.RotW = d.RotW;
        info.Scale    = d.Scale;
        info.Type     = static_cast<uint32>(d.Type);
        info.IsLocked = d.IsLocked;
        return info;
    }

    WorldPackets::Housing::HouseFixtureInfo BuildFixtureInfo(Housing::HousingFixture const& f)
    {
        WorldPackets::Housing::HouseFixtureInfo info;
        info.FixtureID      = f.Id;
        info.Type           = static_cast<uint32>(f.Type);
        info.FixtureEntryID = f.FixtureId;
        info.Slot           = f.Slot;
        return info;
    }

    WorldPackets::Housing::HouseRoomInfo BuildRoomInfo(Housing::HousingRoom const& r)
    {
        WorldPackets::Housing::HouseRoomInfo info;
        info.RoomID         = r.Id;
        info.RoomIndex      = r.RoomIndex;
        info.PosX           = r.PosX;       info.PosY  = r.PosY;    info.PosZ  = r.PosZ;
        info.Orientation    = r.Orientation;
        info.SizeX          = r.SizeX;      info.SizeY = r.SizeY;
        info.CeilingType    = static_cast<uint32>(r.CeilingType);
        info.DoorType       = static_cast<uint32>(r.DoorType);
        info.ComponentTheme = r.ComponentTheme;
        return info;
    }

    WorldPackets::Housing::FullHouseData BuildFullHouseData(Housing::PlayerHouse const& house)
    {
        WorldPackets::Housing::FullHouseData data;
        data.Info = BuildHouseInfo(house);

        data.Decorations.reserve(house.Decorations.size());
        for (auto const& d : house.Decorations)
            if (!d.IsDeleted)
                data.Decorations.push_back(BuildDecorInfo(d));

        data.Fixtures.reserve(house.Fixtures.size());
        for (auto const& f : house.Fixtures)
            if (!f.IsDeleted)
                data.Fixtures.push_back(BuildFixtureInfo(f));

        data.Rooms.reserve(house.Rooms.size());
        for (auto const& r : house.Rooms)
            if (!r.IsDeleted)
                data.Rooms.push_back(BuildRoomInfo(r));

        return data;
    }

    void SendPermissionsFailure(WorldSession* session, Housing::HousingError error)
    {
        WorldPackets::Housing::SvcsNotifyPermissionsFailure response;
        response.Error = static_cast<uint32>(error);
        session->SendPacket(response.Write());
    }

    /// Validate that the player is the owner of a house and is in edit mode
    Housing::PlayerHouse* ValidateOwnerInEditMode(WorldSession* session, bool needDecorMode = false,
                                                   bool needFixtureMode = false, bool needRoomMode = false)
    {
        Player* player = session->GetPlayer();
        if (!player)
            return nullptr;

        Housing::PlayerHouse* house = sHousingMgr->GetPlayerHouse(player->GetGUID());
        if (!house)
        {
            SendPermissionsFailure(session, Housing::HousingError::NoHouse);
            return nullptr;
        }

        if (house->Status != Housing::HouseStatus::Active)
        {
            SendPermissionsFailure(session, Housing::HousingError::PermissionDenied);
            return nullptr;
        }

        if (needDecorMode && !sHousingMgr->IsInDecorEditMode(player->GetGUID()))
        {
            SendPermissionsFailure(session, Housing::HousingError::PermissionDenied);
            return nullptr;
        }

        if (needFixtureMode && !sHousingMgr->IsInFixtureEditMode(player->GetGUID()))
        {
            SendPermissionsFailure(session, Housing::HousingError::PermissionDenied);
            return nullptr;
        }

        if (needRoomMode && !sHousingMgr->IsInRoomLayoutEditMode(player->GetGUID()))
        {
            SendPermissionsFailure(session, Housing::HousingError::PermissionDenied);
            return nullptr;
        }

        return house;
    }
}

// =========================================================
// Phase 2 handlers
// =========================================================

void WorldSession::HandleDeclineNeighborhoodInvites(
    WorldPackets::Housing::DeclineNeighborhoodInvites const& packet)
{
    if (packet.Allow)
        GetPlayer()->SetPlayerFlagEx(PLAYER_FLAGS_EX_AUTO_DECLINE_NEIGHBORHOOD);
    else
        GetPlayer()->RemovePlayerFlagEx(PLAYER_FLAGS_EX_AUTO_DECLINE_NEIGHBORHOOD);
}

void WorldSession::HandleHousingHouseStatus(
    WorldPackets::Housing::HouseStatus const& /*packet*/)
{
    WorldPackets::Housing::HouseStatusResponse response;
    Housing::PlayerHouse* house = sHousingMgr->GetPlayerHouse(GetPlayer()->GetGUID());
    if (house && house->Status != Housing::HouseStatus::None)
    {
        response.HasHouse = true;
        response.House    = BuildHouseInfo(*house);
    }
    SendPacket(response.Write());
}

void WorldSession::HandleHousingGetCurrentHouseInfo(
    WorldPackets::Housing::GetCurrentHouseInfo const& /*packet*/)
{
    WorldPackets::Housing::GetCurrentHouseInfoResponse response;
    Housing::PlayerHouse* house = sHousingMgr->GetPlayerHouse(GetPlayer()->GetGUID());
    if (house && house->Status != Housing::HouseStatus::None)
    {
        response.HasHouse  = true;
        response.HouseData = BuildFullHouseData(*house);
    }
    SendPacket(response.Write());
}

void WorldSession::HandleHousingGetPlayerPermissions(
    WorldPackets::Housing::GetPlayerPermissions const& packet)
{
    WorldPackets::Housing::GetPlayerPermissionsResponse response;
    Housing::PlayerHouse* house = sHousingMgr->GetPlayerHouse(packet.OwnerGuid);

    if (!house)
        response.PermissionLevel = static_cast<uint32>(Housing::HousingPermission::None);
    else if (packet.OwnerGuid == GetPlayer()->GetGUID())
        response.PermissionLevel = static_cast<uint32>(Housing::HousingPermission::Owner);
    else if (house->AllowVisits)
        response.PermissionLevel = static_cast<uint32>(Housing::HousingPermission::Visit);
    else
        response.PermissionLevel = static_cast<uint32>(Housing::HousingPermission::None);

    SendPacket(response.Write());
}

void WorldSession::HandleHousingSvcsGetPlayerHousesInfo(
    WorldPackets::Housing::SvcsGetPlayerHousesInfo const& packet)
{
    WorldPackets::Housing::SvcsGetPlayerHousesInfoResponse response;
    Housing::PlayerHouse* house = sHousingMgr->GetPlayerHouse(packet.PlayerGuid);

    bool isSelf = (packet.PlayerGuid == GetPlayer()->GetGUID());
    if (house && house->Status != Housing::HouseStatus::None)
        if (isSelf || house->IsPublic)
            response.Houses.push_back(BuildHouseInfo(*house));

    SendPacket(response.Write());
}

void WorldSession::HandleHousingSvcsTeleportToPlot(
    WorldPackets::Housing::SvcsTeleportToPlot const& packet)
{
    Player* player = GetPlayer();
    if (player->IsInCombat())
    {
        SendPermissionsFailure(this, Housing::HousingError::PermissionDenied);
        return;
    }
    Housing::HousingError result = sHousingMgr->TeleportToPlot(player, packet.PlotID);
    if (result != Housing::HousingError::Success)
        SendPermissionsFailure(this, result);
}

void WorldSession::HandleHousingSvcsNeighborhoodReservePlot(
    WorldPackets::Housing::SvcsNeighborhoodReservePlot const& packet)
{
    Player* player = GetPlayer();
    WorldPackets::Housing::SvcsNeighborhoodReservePlotResponse response;
    response.PlotID = packet.PlotID;

    Housing::HouseSize size = static_cast<Housing::HouseSize>(packet.HouseSize);
    Housing::HousingError result = sHousingMgr->CreateHouse(player, packet.PlotID, size);
    response.Error = static_cast<uint32>(result);
    SendPacket(response.Write());

    if (result == Housing::HousingError::Success)
    {
        TC_LOG_INFO("housing", "Player %s (%s) created house on plot %lu",
            player->GetName().c_str(), player->GetGUID().ToString().c_str(), packet.PlotID);
        HandleHousingGetCurrentHouseInfo(WorldPackets::Housing::GetCurrentHouseInfo(WorldPacket()));
    }
}

void WorldSession::HandleHousingSvcsRelinquishHouse(
    WorldPackets::Housing::SvcsRelinquishHouse const& /*packet*/)
{
    WorldPackets::Housing::SvcsRelinquishHouseResponse response;
    response.Error = static_cast<uint32>(sHousingMgr->DeleteHouse(GetPlayer()));
    SendPacket(response.Write());
}

void WorldSession::HandleHousingSvcsUpdateHouseSettings(
    WorldPackets::Housing::SvcsUpdateHouseSettings const& packet)
{
    WorldPackets::Housing::SvcsUpdateHouseSettingsResponse response;
    response.Error = static_cast<uint32>(
        sHousingMgr->UpdateHouseSettings(GetPlayer(), packet.Name, packet.Description,
                                          packet.IsPublic, packet.AllowVisits));
    SendPacket(response.Write());
}

void WorldSession::HandleHousingDecorSetEditMode(
    WorldPackets::Housing::DecorSetEditMode const& packet)
{
    WorldPackets::Housing::DecorSetEditModeResponse response;
    if (packet.Enable && !sHousingMgr->PlayerHasHouse(GetPlayer()->GetGUID()))
    {
        response.Error = static_cast<uint32>(Housing::HousingError::NoHouse);
    }
    else
    {
        sHousingMgr->SetDecorEditMode(GetPlayer(), packet.Enable);
        response.Error   = static_cast<uint32>(Housing::HousingError::Success);
        response.Enabled = packet.Enable;
    }
    SendPacket(response.Write());
}

void WorldSession::HandleHousingFixtureSetEditMode(
    WorldPackets::Housing::FixtureSetEditMode const& packet)
{
    WorldPackets::Housing::FixtureSetEditModeResponse response;
    if (packet.Enable && !sHousingMgr->PlayerHasHouse(GetPlayer()->GetGUID()))
    {
        response.Error = static_cast<uint32>(Housing::HousingError::NoHouse);
    }
    else
    {
        sHousingMgr->SetFixtureEditMode(GetPlayer(), packet.Enable);
        response.Error   = static_cast<uint32>(Housing::HousingError::Success);
        response.Enabled = packet.Enable;
    }
    SendPacket(response.Write());
}

void WorldSession::HandleHousingRoomSetLayoutEditMode(
    WorldPackets::Housing::RoomSetLayoutEditMode const& packet)
{
    WorldPackets::Housing::RoomSetLayoutEditModeResponse response;
    if (packet.Enable && !sHousingMgr->PlayerHasHouse(GetPlayer()->GetGUID()))
    {
        response.Error = static_cast<uint32>(Housing::HousingError::NoHouse);
    }
    else
    {
        sHousingMgr->SetRoomLayoutEditMode(GetPlayer(), packet.Enable);
        response.Error   = static_cast<uint32>(Housing::HousingError::Success);
        response.Enabled = packet.Enable;
    }
    SendPacket(response.Write());
}

// =========================================================
// Phase 3 — Decoration Handlers
// =========================================================

/// CMSG_HOUSING_DECOR_PLACE → SMSG_HOUSING_DECOR_PLACE_RESPONSE
/// Places a new decoration in the house. ID assigned immediately via
/// atomic sequence generator — no sync DB round-trip required.
void WorldSession::HandleHousingDecorPlace(
    WorldPackets::Housing::DecorPlace const& packet)
{
    Player* player = GetPlayer();
    if (!ValidateOwnerInEditMode(this, /*needDecorMode=*/true))
        return;

    Housing::HousingDecoration decor;
    Housing::HousingError result = sHousingMgr->PlaceDecorationAndGetResult(
        player,
        packet.ItemID,
        packet.PosX, packet.PosY, packet.PosZ,
        packet.RotX, packet.RotY, packet.RotZ, packet.RotW,
        packet.Scale,
        static_cast<Housing::DecorationType>(packet.Type),
        decor);

    WorldPackets::Housing::DecorPlaceResponse response;
    response.Error = static_cast<uint32>(result);
    if (result == Housing::HousingError::Success)
        response.Decor = BuildDecorInfo(decor);

    SendPacket(response.Write());
}

/// CMSG_HOUSING_DECOR_MOVE → SMSG_HOUSING_DECOR_MOVE_RESPONSE
/// Moves an existing decoration to a new position/rotation.
/// Marks dirty — saved on logout or periodic flush.
void WorldSession::HandleHousingDecorMove(
    WorldPackets::Housing::DecorMove const& packet)
{
    if (!ValidateOwnerInEditMode(this, /*needDecorMode=*/true))
        return;

    Housing::HousingError result = sHousingMgr->MoveDecoration(
        GetPlayer(), packet.DecorID,
        packet.PosX, packet.PosY, packet.PosZ,
        packet.RotX, packet.RotY, packet.RotZ, packet.RotW);

    WorldPackets::Housing::DecorMoveResponse response;
    response.Error   = static_cast<uint32>(result);
    response.DecorID = packet.DecorID;
    SendPacket(response.Write());
}

/// CMSG_HOUSING_DECOR_REMOVE → SMSG_HOUSING_DECOR_REMOVE_RESPONSE
/// Removes a decoration. Deletes from DB immediately (no orphan risk).
void WorldSession::HandleHousingDecorRemove(
    WorldPackets::Housing::DecorRemove const& packet)
{
    if (!ValidateOwnerInEditMode(this, /*needDecorMode=*/true))
        return;

    // Retrieve ItemID before removal (for response)
    uint32 itemId = 0;
    if (Housing::PlayerHouse* house = sHousingMgr->GetPlayerHouse(GetPlayer()->GetGUID()))
        if (Housing::HousingDecoration* d = sHousingMgr->FindDecoration(house, packet.DecorID))
            itemId = d->ItemId;

    Housing::HousingError result = sHousingMgr->RemoveDecoration(GetPlayer(), packet.DecorID);

    WorldPackets::Housing::DecorRemoveResponse response;
    response.Error   = static_cast<uint32>(result);
    response.DecorID = packet.DecorID;
    response.ItemID  = itemId;  // 0 = no item refund (by design for RP server)
    SendPacket(response.Write());
}

/// CMSG_HOUSING_DECOR_LOCK → SMSG_HOUSING_DECOR_LOCK_RESPONSE
void WorldSession::HandleHousingDecorLock(
    WorldPackets::Housing::DecorLock const& packet)
{
    if (!ValidateOwnerInEditMode(this, /*needDecorMode=*/true))
        return;

    Housing::HousingError result = sHousingMgr->LockDecoration(
        GetPlayer(), packet.DecorID, packet.Locked);

    WorldPackets::Housing::DecorLockResponse response;
    response.Error   = static_cast<uint32>(result);
    response.DecorID = packet.DecorID;
    response.Locked  = packet.Locked;
    SendPacket(response.Write());
}

/// CMSG_HOUSING_DECOR_REQUEST_STORAGE → SMSG_HOUSING_DECOR_REQUEST_STORAGE_RESPONSE
/// Returns available decorations from the player's inventory tagged as housing items.
/// For a RP server we scan the player bag for items with HousingDecorDB2 entry.
void WorldSession::HandleHousingDecorRequestStorage(
    WorldPackets::Housing::DecorRequestStorage const& /*packet*/)
{
    Player* player = GetPlayer();
    WorldPackets::Housing::DecorRequestStorageResponse response;

    if (!sHousingMgr->PlayerHasHouse(player->GetGUID()))
    {
        response.Error = static_cast<uint32>(Housing::HousingError::NoHouse);
        SendPacket(response.Write());
        return;
    }

    // Scan player main bag for ITEM_CLASS_HOUSING items
    std::unordered_map<uint32, uint32> decorItems; // itemId -> count

    for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (Item const* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            ItemTemplate const* proto = item->GetTemplate();
            if (proto && proto->GetClass() == ITEM_CLASS_HOUSING)
                decorItems[proto->GetId()] += item->GetCount();
        }
    }

    response.Error = static_cast<uint32>(Housing::HousingError::Success);
    response.Items.reserve(decorItems.size());
    for (auto it = decorItems.begin(); it != decorItems.end(); ++it)
    {
        WorldPackets::Housing::DecorStorageItem item;
        item.ItemID = it->first;
        item.Count  = it->second;
        response.Items.push_back(item);
    }

    SendPacket(response.Write());
}

// =========================================================
// Phase 3 — Fixture Handlers
// =========================================================

/// CMSG_HOUSING_FIXTURE_SET_CORE_FIXTURE → SMSG_HOUSING_FIXTURE_SET_CORE_FIXTURE_RESPONSE
void WorldSession::HandleHousingFixtureSetCoreFixture(
    WorldPackets::Housing::FixtureSetCoreFixture const& packet)
{
    if (!ValidateOwnerInEditMode(this, false, /*needFixtureMode=*/true))
        return;

    Housing::HousingError result = sHousingMgr->SetCoreFixture(GetPlayer(), packet.FixtureID);

    WorldPackets::Housing::FixtureSetCoreFixtureResponse response;
    response.Error     = static_cast<uint32>(result);
    response.FixtureID = packet.FixtureID;
    SendPacket(response.Write());
}

/// CMSG_HOUSING_FIXTURE_SET_HOUSE_SIZE → SMSG_HOUSING_FIXTURE_SET_HOUSE_SIZE_RESPONSE
void WorldSession::HandleHousingFixtureSetHouseSize(
    WorldPackets::Housing::FixtureSetHouseSize const& packet)
{
    if (!ValidateOwnerInEditMode(this, false, /*needFixtureMode=*/true))
        return;

    Housing::HouseSize newSize = static_cast<Housing::HouseSize>(packet.HouseSize);
    Housing::HousingError result = sHousingMgr->SetHouseSize(GetPlayer(), newSize);

    WorldPackets::Housing::FixtureSetHouseSizeResponse response;
    response.Error     = static_cast<uint32>(result);
    response.HouseSize = packet.HouseSize;
    SendPacket(response.Write());
}

/// CMSG_HOUSING_FIXTURE_SET_HOUSE_TYPE → SMSG_HOUSING_FIXTURE_SET_HOUSE_TYPE_RESPONSE
void WorldSession::HandleHousingFixtureSetHouseType(
    WorldPackets::Housing::FixtureSetHouseType const& packet)
{
    if (!ValidateOwnerInEditMode(this, false, /*needFixtureMode=*/true))
        return;

    Housing::HousingError result = sHousingMgr->SetHouseType(GetPlayer(), packet.HouseTypeID);

    WorldPackets::Housing::FixtureSetHouseTypeResponse response;
    response.Error       = static_cast<uint32>(result);
    response.HouseTypeID = packet.HouseTypeID;
    SendPacket(response.Write());
}

/// CMSG_HOUSING_FIXTURE_CREATE_FIXTURE → SMSG_HOUSING_FIXTURE_CREATE_FIXTURE_RESPONSE
void WorldSession::HandleHousingFixtureCreateFixture(
    WorldPackets::Housing::FixtureCreateFixture const& packet)
{
    if (!ValidateOwnerInEditMode(this, false, /*needFixtureMode=*/true))
        return;

    Housing::HousingFixture fixture;
    Housing::HousingError result = sHousingMgr->CreateFixtureAndGetResult(
        GetPlayer(),
        static_cast<Housing::FixtureType>(packet.Type),
        packet.FixtureID,
        packet.Slot,
        fixture);

    WorldPackets::Housing::FixtureCreateFixtureResponse response;
    response.Error = static_cast<uint32>(result);
    if (result == Housing::HousingError::Success)
        response.Fixture = BuildFixtureInfo(fixture);
    SendPacket(response.Write());
}

/// CMSG_HOUSING_FIXTURE_DELETE_FIXTURE → SMSG_HOUSING_FIXTURE_DELETE_FIXTURE_RESPONSE
void WorldSession::HandleHousingFixtureDeleteFixture(
    WorldPackets::Housing::FixtureDeleteFixture const& packet)
{
    if (!ValidateOwnerInEditMode(this, false, /*needFixtureMode=*/true))
        return;

    Housing::HousingError result = sHousingMgr->DeleteFixture(GetPlayer(), packet.FixtureID);

    WorldPackets::Housing::FixtureDeleteFixtureResponse response;
    response.Error     = static_cast<uint32>(result);
    response.FixtureID = packet.FixtureID;
    SendPacket(response.Write());
}

// =========================================================
// Phase 3 — Room Handlers
// =========================================================

/// CMSG_HOUSING_ROOM_ADD → SMSG_HOUSING_ROOM_ADD_RESPONSE
void WorldSession::HandleHousingRoomAdd(
    WorldPackets::Housing::RoomAdd const& packet)
{
    if (!ValidateOwnerInEditMode(this, false, false, /*needRoomMode=*/true))
        return;

    Housing::HousingRoom room;
    Housing::HousingError result = sHousingMgr->AddRoomAndGetResult(
        GetPlayer(),
        packet.PosX, packet.PosY, packet.PosZ, packet.Orientation,
        packet.SizeX, packet.SizeY,
        room);

    WorldPackets::Housing::RoomAddResponse response;
    response.Error = static_cast<uint32>(result);
    if (result == Housing::HousingError::Success)
        response.Room = BuildRoomInfo(room);
    SendPacket(response.Write());
}

/// CMSG_HOUSING_ROOM_REMOVE → SMSG_HOUSING_ROOM_REMOVE_RESPONSE
void WorldSession::HandleHousingRoomRemove(
    WorldPackets::Housing::RoomRemove const& packet)
{
    if (!ValidateOwnerInEditMode(this, false, false, /*needRoomMode=*/true))
        return;

    Housing::HousingError result = sHousingMgr->RemoveRoom(GetPlayer(), packet.RoomID);

    WorldPackets::Housing::RoomRemoveResponse response;
    response.Error  = static_cast<uint32>(result);
    response.RoomID = packet.RoomID;
    SendPacket(response.Write());
}

/// CMSG_HOUSING_ROOM_MOVE → SMSG_HOUSING_ROOM_UPDATE_RESPONSE
void WorldSession::HandleHousingRoomMove(
    WorldPackets::Housing::RoomMove const& packet)
{
    if (!ValidateOwnerInEditMode(this, false, false, /*needRoomMode=*/true))
        return;

    Housing::HousingRoom room;
    Housing::HousingError result = sHousingMgr->MoveRoomAndGetResult(
        GetPlayer(), packet.RoomID,
        packet.PosX, packet.PosY, packet.PosZ, packet.Orientation,
        room);

    WorldPackets::Housing::RoomUpdateResponse response;
    response.Error = static_cast<uint32>(result);
    if (result == Housing::HousingError::Success)
        response.Room = BuildRoomInfo(room);
    SendPacket(response.Write());
}

/// CMSG_HOUSING_ROOM_ROTATE → SMSG_HOUSING_ROOM_UPDATE_RESPONSE
void WorldSession::HandleHousingRoomRotate(
    WorldPackets::Housing::RoomRotate const& packet)
{
    if (!ValidateOwnerInEditMode(this, false, false, /*needRoomMode=*/true))
        return;

    Housing::HousingRoom room;
    Housing::HousingError result = sHousingMgr->RotateRoomAndGetResult(
        GetPlayer(), packet.RoomID, packet.Orientation, room);

    WorldPackets::Housing::RoomUpdateResponse response;
    response.Error = static_cast<uint32>(result);
    if (result == Housing::HousingError::Success)
        response.Room = BuildRoomInfo(room);
    SendPacket(response.Write());
}

/// CMSG_HOUSING_ROOM_SET_CEILING_TYPE → SMSG_HOUSING_ROOM_SET_CEILING_TYPE_RESPONSE
void WorldSession::HandleHousingRoomSetCeilingType(
    WorldPackets::Housing::RoomSetCeilingType const& packet)
{
    if (!ValidateOwnerInEditMode(this, false, false, /*needRoomMode=*/true))
        return;

    Housing::HousingError result = sHousingMgr->SetRoomCeilingType(
        GetPlayer(), packet.RoomID,
        static_cast<Housing::RoomCeilingType>(packet.CeilingType));

    WorldPackets::Housing::RoomSetCeilingTypeResponse response;
    response.Error       = static_cast<uint32>(result);
    response.RoomID      = packet.RoomID;
    response.CeilingType = packet.CeilingType;
    SendPacket(response.Write());
}

/// CMSG_HOUSING_ROOM_SET_DOOR_TYPE → SMSG_HOUSING_ROOM_SET_DOOR_TYPE_RESPONSE
void WorldSession::HandleHousingRoomSetDoorType(
    WorldPackets::Housing::RoomSetDoorType const& packet)
{
    if (!ValidateOwnerInEditMode(this, false, false, /*needRoomMode=*/true))
        return;

    Housing::HousingError result = sHousingMgr->SetRoomDoorType(
        GetPlayer(), packet.RoomID,
        static_cast<Housing::RoomDoorType>(packet.DoorType));

    WorldPackets::Housing::RoomSetDoorTypeResponse response;
    response.Error    = static_cast<uint32>(result);
    response.RoomID   = packet.RoomID;
    response.DoorType = packet.DoorType;
    SendPacket(response.Write());
}

/// CMSG_HOUSING_ROOM_SET_COMPONENT_THEME → SMSG_HOUSING_ROOM_SET_COMPONENT_THEME_RESPONSE
void WorldSession::HandleHousingRoomSetComponentTheme(
    WorldPackets::Housing::RoomSetComponentTheme const& packet)
{
    if (!ValidateOwnerInEditMode(this, false, false, /*needRoomMode=*/true))
        return;

    Housing::HousingError result = sHousingMgr->SetRoomComponentTheme(
        GetPlayer(), packet.RoomID, packet.ThemeID);

    WorldPackets::Housing::RoomSetComponentThemeResponse response;
    response.Error   = static_cast<uint32>(result);
    response.RoomID  = packet.RoomID;
    response.ThemeID = packet.ThemeID;
    SendPacket(response.Write());
}

// =============================================================================
// Phase X Stubs — handlers référencés dans Opcodes.cpp, pas encore implémentés
// =============================================================================

/// CMSG_NEIGHBORHOOD_BUY_HOUSE — Phase 4 (achat maison dans un neighborhood)
void WorldSession::HandleNeighborhoodBuyHouse(WorldPackets::Null& /*packet*/)
{
    // TODO Phase 4: implémentation achat de maison
    TC_LOG_DEBUG("housing", "HandleNeighborhoodBuyHouse: player %s — not yet implemented",
        GetPlayer()->GetGUID().ToString().c_str());
}

/// CMSG_HOUSING_START_TUTORIAL — tutoriel housing
void WorldSession::HandleHousingSvcsStartTutorial(WorldPackets::Housing::SvcsStartTutorial& /*packet*/)
{
    TC_LOG_DEBUG("housing", "HandleHousingSvcsStartTutorial: player %s — not yet implemented",
        GetPlayer()->GetGUID().ToString().c_str());
}
