/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * Housing System Packets — Phase 2 + Phase 3
 */

#include "HousingPackets.h"
#include "PacketOperators.h"

// ===========================================================================
// Fix: Bits<N> et Size<T> sont dans namespace WorldPackets.
// Les opérateurs ByteBuffer ci-dessous sont des fonctions globales (hors
// namespace), donc sans ces using declarations le compilateur ne les trouve pas.
// ===========================================================================
using WorldPackets::Bits;
using WorldPackets::Size;

// =========================================================
// ByteBuffer stream operators for shared structures
// =========================================================

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Housing::HouseInfo const& info)
{
    data << uint64(info.HouseID);
    data << uint64(info.PlotID);
    data << uint64(info.NeighborhoodID);
    data << uint8(info.Status);
    data << uint8(info.HouseSize);

    // Bits first, then flush, then string data (TrinityCore packet convention)
    data.WriteBits(uint32(info.Name.size()),        7);   // max 128 chars → 7 bits
    data.WriteBits(uint32(info.Description.size()), 10);  // max 512 chars → 10 bits
    data.WriteBit(info.IsPublic);
    data.WriteBit(info.AllowVisits);
    data.FlushBits();

    if (!info.Name.empty())        data.WriteString(info.Name);
    if (!info.Description.empty()) data.WriteString(info.Description);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Housing::HouseDecorInfo const& decor)
{
    data << uint64(decor.DecorID);
    data << uint32(decor.ItemID);
    data << uint32(decor.Type);
    data << float(decor.PosX);
    data << float(decor.PosY);
    data << float(decor.PosZ);
    data << float(decor.RotX);
    data << float(decor.RotY);
    data << float(decor.RotZ);
    data << float(decor.RotW);
    data << float(decor.Scale);
    data.WriteBit(decor.IsLocked);
    data.FlushBits();
    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Housing::HouseFixtureInfo const& fixture)
{
    data << uint64(fixture.FixtureID);
    data << uint32(fixture.Type);
    data << uint32(fixture.FixtureEntryID);
    data << uint32(fixture.Slot);
    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Housing::HouseRoomInfo const& room)
{
    data << uint64(room.RoomID);
    data << uint32(room.RoomIndex);
    data << float(room.PosX);
    data << float(room.PosY);
    data << float(room.PosZ);
    data << float(room.Orientation);
    data << float(room.SizeX);
    data << float(room.SizeY);
    data << uint32(room.CeilingType);
    data << uint32(room.DoorType);
    data << uint32(room.ComponentTheme);
    return data;
}

namespace WorldPackets::Housing
{
// À l'intérieur de WorldPackets::Housing, Bits<>/Size<> sont visibles
// par lookup de namespace englobant — pas de qualification supplémentaire.

// =========================================================
// Phase 2 — CMSG Read()
// =========================================================

void DeclineNeighborhoodInvites::Read()
{
    Allow = _worldPacket.ReadBit();
}

void GetPlayerPermissions::Read()
{
    _worldPacket >> OwnerGuid;
}

void SvcsGetPlayerHousesInfo::Read()
{
    _worldPacket >> PlayerGuid;
}

void SvcsTeleportToPlot::Read()
{
    _worldPacket >> PlotID;
}

void SvcsNeighborhoodReservePlot::Read()
{
    _worldPacket >> PlotID;
    _worldPacket >> HouseSize;
}

void SvcsUpdateHouseSettings::Read()
{
    uint32 nameLen = _worldPacket.ReadBits(7);
    uint32 descLen = _worldPacket.ReadBits(10);
    IsPublic    = _worldPacket.ReadBit();
    AllowVisits = _worldPacket.ReadBit();
    Name        = _worldPacket.ReadString(nameLen);
    Description = _worldPacket.ReadString(descLen);
}

void DecorSetEditMode::Read()      { Enable = _worldPacket.ReadBit(); }
void FixtureSetEditMode::Read()    { Enable = _worldPacket.ReadBit(); }
void RoomSetLayoutEditMode::Read() { Enable = _worldPacket.ReadBit(); }

// =========================================================
// Phase 3 — CMSG Read() — Decoration
// =========================================================

void DecorPlace::Read()
{
    _worldPacket >> ItemID;
    _worldPacket >> Type;
    _worldPacket >> PosX >> PosY >> PosZ;
    _worldPacket >> RotX >> RotY >> RotZ >> RotW;
    _worldPacket >> Scale;
}

void DecorMove::Read()
{
    _worldPacket >> DecorID;
    _worldPacket >> PosX >> PosY >> PosZ;
    _worldPacket >> RotX >> RotY >> RotZ >> RotW;
}

void DecorRemove::Read()
{
    _worldPacket >> DecorID;
}

void DecorLock::Read()
{
    _worldPacket >> DecorID;
    Locked = _worldPacket.ReadBit();
}

// =========================================================
// Phase 3 — CMSG Read() — Fixture
// =========================================================

void FixtureSetCoreFixture::Read()  { _worldPacket >> FixtureID;  }
void FixtureSetHouseSize::Read()    { _worldPacket >> HouseSize;  }
void FixtureSetHouseType::Read()    { _worldPacket >> HouseTypeID; }

void FixtureCreateFixture::Read()
{
    _worldPacket >> Type;
    _worldPacket >> FixtureID;
    _worldPacket >> Slot;
}

void FixtureDeleteFixture::Read()   { _worldPacket >> FixtureID; }

// =========================================================
// Phase 3 — CMSG Read() — Room
// =========================================================

void RoomAdd::Read()
{
    _worldPacket >> PosX >> PosY >> PosZ >> Orientation;
    _worldPacket >> SizeX >> SizeY;
}

void RoomRemove::Read()  { _worldPacket >> RoomID; }

void RoomMove::Read()
{
    _worldPacket >> RoomID;
    _worldPacket >> PosX >> PosY >> PosZ >> Orientation;
}

void RoomRotate::Read()
{
    _worldPacket >> RoomID;
    _worldPacket >> Orientation;
}

void RoomSetCeilingType::Read()
{
    _worldPacket >> RoomID;
    _worldPacket >> CeilingType;
}

void RoomSetDoorType::Read()
{
    _worldPacket >> RoomID;
    _worldPacket >> DoorType;
}

void RoomSetComponentTheme::Read()
{
    _worldPacket >> RoomID;
    _worldPacket >> ThemeID;
}

// =========================================================
// Phase 2 — SMSG Write()
// =========================================================

WorldPacket const* HouseStatusResponse::Write()
{
    _worldPacket.WriteBit(HasHouse);
    _worldPacket.FlushBits();
    if (HasHouse && House)
        _worldPacket << *House;
    return &_worldPacket;
}

WorldPacket const* GetCurrentHouseInfoResponse::Write()
{
    _worldPacket.WriteBit(HasHouse);
    _worldPacket.FlushBits();
    if (HasHouse && HouseData)
    {
        FullHouseData const& hd = *HouseData;
        _worldPacket << hd.Info;
        _worldPacket << uint32(hd.Decorations.size());
        for (HouseDecorInfo const& d : hd.Decorations)
            _worldPacket << d;
        _worldPacket << uint32(hd.Fixtures.size());
        for (HouseFixtureInfo const& f : hd.Fixtures)
            _worldPacket << f;
        _worldPacket << uint32(hd.Rooms.size());
        for (HouseRoomInfo const& r : hd.Rooms)
            _worldPacket << r;
    }
    return &_worldPacket;
}

WorldPacket const* GetPlayerPermissionsResponse::Write()
{
    _worldPacket << uint32(PermissionLevel);
    return &_worldPacket;
}

WorldPacket const* SvcsGetPlayerHousesInfoResponse::Write()
{
    _worldPacket << uint32(Houses.size());
    for (HouseInfo const& h : Houses)
        _worldPacket << h;
    return &_worldPacket;
}

WorldPacket const* SvcsNotifyPermissionsFailure::Write()
{
    _worldPacket << uint32(Error);
    return &_worldPacket;
}

WorldPacket const* SvcsRelinquishHouseResponse::Write()
{
    _worldPacket << uint32(Error);
    return &_worldPacket;
}

WorldPacket const* SvcsUpdateHouseSettingsResponse::Write()
{
    _worldPacket << uint32(Error);
    return &_worldPacket;
}

WorldPacket const* SvcsNeighborhoodReservePlotResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << uint64(PlotID);
    return &_worldPacket;
}

WorldPacket const* DecorSetEditModeResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket.WriteBit(Enabled);
    _worldPacket.FlushBits();
    return &_worldPacket;
}

WorldPacket const* FixtureSetEditModeResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket.WriteBit(Enabled);
    _worldPacket.FlushBits();
    return &_worldPacket;
}

WorldPacket const* RoomSetLayoutEditModeResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket.WriteBit(Enabled);
    _worldPacket.FlushBits();
    return &_worldPacket;
}

// =========================================================
// Phase 3 — SMSG Write() — Decoration
// =========================================================

WorldPacket const* DecorPlaceResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << Decor;
    return &_worldPacket;
}

WorldPacket const* DecorMoveResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << uint64(DecorID);
    return &_worldPacket;
}

WorldPacket const* DecorRemoveResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << uint64(DecorID);
    _worldPacket << uint32(ItemID);
    return &_worldPacket;
}

WorldPacket const* DecorLockResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << uint64(DecorID);
    _worldPacket.WriteBit(Locked);
    _worldPacket.FlushBits();
    return &_worldPacket;
}

WorldPacket const* DecorRequestStorageResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << uint32(Items.size());
    for (DecorStorageItem const& item : Items)
    {
        _worldPacket << uint32(item.ItemID);
        _worldPacket << uint32(item.Count);
    }
    return &_worldPacket;
}

// =========================================================
// Phase 3 — SMSG Write() — Fixture
// =========================================================

WorldPacket const* FixtureSetCoreFixtureResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << uint32(FixtureID);
    return &_worldPacket;
}

WorldPacket const* FixtureSetHouseSizeResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << uint8(HouseSize);
    return &_worldPacket;
}

WorldPacket const* FixtureSetHouseTypeResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << uint32(HouseTypeID);
    return &_worldPacket;
}

WorldPacket const* FixtureCreateFixtureResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << Fixture;
    return &_worldPacket;
}

WorldPacket const* FixtureDeleteFixtureResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << uint64(FixtureID);
    return &_worldPacket;
}

// =========================================================
// Phase 3 — SMSG Write() — Room
// =========================================================

WorldPacket const* RoomAddResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << Room;
    return &_worldPacket;
}

WorldPacket const* RoomRemoveResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << uint64(RoomID);
    return &_worldPacket;
}

WorldPacket const* RoomUpdateResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << Room;
    return &_worldPacket;
}

WorldPacket const* RoomSetCeilingTypeResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << uint64(RoomID);
    _worldPacket << uint32(CeilingType);
    return &_worldPacket;
}

WorldPacket const* RoomSetDoorTypeResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << uint64(RoomID);
    _worldPacket << uint32(DoorType);
    return &_worldPacket;
}

WorldPacket const* RoomSetComponentThemeResponse::Write()
{
    _worldPacket << uint32(Error);
    _worldPacket << uint64(RoomID);
    _worldPacket << uint32(ThemeID);
    return &_worldPacket;
}

} // namespace WorldPackets::Housing
