/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * Housing System Packets — Phase 2 + Phase 3
 * Covers: status, teleport, edit modes, settings, relinquish,
 *         decoration (place/move/remove/lock/storage),
 *         fixture (core/size/type/create/delete),
 *         room (add/remove/move/rotate/ceiling/door/theme)
 */

#ifndef TRINITYCORE_HOUSING_PACKETS_H
#define TRINITYCORE_HOUSING_PACKETS_H

#include "HousingDefines.h"
#include "ObjectGuid.h"
#include "Optional.h"
#include "Packet.h"
#include <string>
#include <vector>

namespace WorldPackets::Housing
{
    // =========================================================
    // Shared packet data structures
    // =========================================================

    struct HouseInfo
    {
        uint64      HouseID         = 0;
        uint64      PlotID          = 0;
        uint64      NeighborhoodID  = 0;
        uint8       Status          = 0;
        uint8       HouseSize       = 0;
        std::string Name;
        std::string Description;
        bool        IsPublic        = false;
        bool        AllowVisits     = true;
    };

    struct HouseDecorInfo
    {
        uint64  DecorID     = 0;
        uint32  ItemID      = 0;
        float   PosX        = 0.0f;
        float   PosY        = 0.0f;
        float   PosZ        = 0.0f;
        float   RotX        = 0.0f;
        float   RotY        = 0.0f;
        float   RotZ        = 0.0f;
        float   RotW        = 1.0f;
        float   Scale       = 1.0f;
        uint32  Type        = 0;
        bool    IsLocked    = false;
    };

    struct HouseFixtureInfo
    {
        uint64  FixtureID       = 0;
        uint32  Type            = 0;
        uint32  FixtureEntryID  = 0;
        uint32  Slot            = 0;
    };

    struct HouseRoomInfo
    {
        uint64  RoomID          = 0;
        uint32  RoomIndex       = 0;
        float   PosX            = 0.0f;
        float   PosY            = 0.0f;
        float   PosZ            = 0.0f;
        float   Orientation     = 0.0f;
        float   SizeX           = 10.0f;
        float   SizeY           = 10.0f;
        uint32  CeilingType     = 0;
        uint32  DoorType        = 0;
        uint32  ComponentTheme  = 0;
    };

    struct FullHouseData
    {
        HouseInfo                   Info;
        std::vector<HouseDecorInfo>   Decorations;
        std::vector<HouseFixtureInfo> Fixtures;
        std::vector<HouseRoomInfo>    Rooms;
    };

    struct DecorStorageItem
    {
        uint32 ItemID   = 0;
        uint32 Count    = 0;
    };

    // =========================================================
    // Phase 2 — CMSG
    // =========================================================

    class DeclineNeighborhoodInvites final : public ClientPacket
    {
    public:
        explicit DeclineNeighborhoodInvites(WorldPacket&& packet)
            : ClientPacket(CMSG_DECLINE_NEIGHBORHOOD_INVITES, std::move(packet)) { }
        void Read() override;
        bool Allow = false;
    };

    class HouseStatus final : public ClientPacket
    {
    public:
        explicit HouseStatus(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_HOUSE_STATUS, std::move(packet)) { }
        void Read() override { }
    };

    class GetCurrentHouseInfo final : public ClientPacket
    {
    public:
        explicit GetCurrentHouseInfo(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_GET_CURRENT_HOUSE_INFO, std::move(packet)) { }
        void Read() override { }
    };

    class GetPlayerPermissions final : public ClientPacket
    {
    public:
        explicit GetPlayerPermissions(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_GET_PLAYER_PERMISSIONS, std::move(packet)) { }
        void Read() override;
        ObjectGuid OwnerGuid;
    };

    class SvcsGetPlayerHousesInfo final : public ClientPacket
    {
    public:
        explicit SvcsGetPlayerHousesInfo(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_SVCS_GET_PLAYER_HOUSES_INFO, std::move(packet)) { }
        void Read() override;
        ObjectGuid PlayerGuid;
    };

    class SvcsTeleportToPlot final : public ClientPacket
    {
    public:
        explicit SvcsTeleportToPlot(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_SVCS_TELEPORT_TO_PLOT, std::move(packet)) { }
        void Read() override;
        uint64 PlotID = 0;
    };

    class SvcsNeighborhoodReservePlot final : public ClientPacket
    {
    public:
        explicit SvcsNeighborhoodReservePlot(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_SVCS_NEIGHBORHOOD_RESERVE_PLOT, std::move(packet)) { }
        void Read() override;
        uint64 PlotID     = 0;
        uint8  HouseSize  = 0;
    };

	class SvcsStartTutorial final : public ClientPacket
	{
	public:
		explicit SvcsStartTutorial(WorldPacket&& packet)
			: ClientPacket(CMSG_HOUSING_SVCS_START_TUTORIAL, std::move(packet)) { }

		void Read() override { }
	};

    class SvcsRelinquishHouse final : public ClientPacket
    {
    public:
        explicit SvcsRelinquishHouse(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_SVCS_RELINQUISH_HOUSE, std::move(packet)) { }
        void Read() override { }
    };

    class SvcsUpdateHouseSettings final : public ClientPacket
    {
    public:
        explicit SvcsUpdateHouseSettings(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_SVCS_UPDATE_HOUSE_SETTINGS, std::move(packet)) { }
        void Read() override;
        std::string Name;
        std::string Description;
        bool        IsPublic    = false;
        bool        AllowVisits = true;
    };

    class DecorSetEditMode final : public ClientPacket
    {
    public:
        explicit DecorSetEditMode(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_DECOR_SET_EDIT_MODE, std::move(packet)) { }
        void Read() override;
        bool Enable = false;
    };

    class FixtureSetEditMode final : public ClientPacket
    {
    public:
        explicit FixtureSetEditMode(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_FIXTURE_SET_EDIT_MODE, std::move(packet)) { }
        void Read() override;
        bool Enable = false;
    };

    class RoomSetLayoutEditMode final : public ClientPacket
    {
    public:
        explicit RoomSetLayoutEditMode(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_ROOM_SET_LAYOUT_EDIT_MODE, std::move(packet)) { }
        void Read() override;
        bool Enable = false;
    };

    // =========================================================
    // Phase 3 — CMSG Decoration
    // =========================================================

    /// CMSG_HOUSING_DECOR_PLACE
    class DecorPlace final : public ClientPacket
    {
    public:
        explicit DecorPlace(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_DECOR_PLACE, std::move(packet)) { }
        void Read() override;

        uint32  ItemID  = 0;
        float   PosX    = 0.0f;
        float   PosY    = 0.0f;
        float   PosZ    = 0.0f;
        float   RotX    = 0.0f;
        float   RotY    = 0.0f;
        float   RotZ    = 0.0f;
        float   RotW    = 1.0f;
        float   Scale   = 1.0f;
        uint32  Type    = 0;    // DecorationType cast
    };

    /// CMSG_HOUSING_DECOR_MOVE
    class DecorMove final : public ClientPacket
    {
    public:
        explicit DecorMove(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_DECOR_MOVE, std::move(packet)) { }
        void Read() override;

        uint64  DecorID = 0;
        float   PosX    = 0.0f;
        float   PosY    = 0.0f;
        float   PosZ    = 0.0f;
        float   RotX    = 0.0f;
        float   RotY    = 0.0f;
        float   RotZ    = 0.0f;
        float   RotW    = 1.0f;
    };

    /// CMSG_HOUSING_DECOR_REMOVE
    class DecorRemove final : public ClientPacket
    {
    public:
        explicit DecorRemove(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_DECOR_REMOVE, std::move(packet)) { }
        void Read() override;
        uint64 DecorID = 0;
    };

    /// CMSG_HOUSING_DECOR_LOCK
    class DecorLock final : public ClientPacket
    {
    public:
        explicit DecorLock(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_DECOR_LOCK, std::move(packet)) { }
        void Read() override;
        uint64 DecorID = 0;
        bool   Locked  = false;
    };

    /// CMSG_HOUSING_DECOR_REQUEST_STORAGE
    class DecorRequestStorage final : public ClientPacket
    {
    public:
        explicit DecorRequestStorage(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_DECOR_REQUEST_STORAGE, std::move(packet)) { }
        void Read() override { }  // No payload — server derives from player bag/collection
    };

    // =========================================================
    // Phase 3 — CMSG Fixture
    // =========================================================

    /// CMSG_HOUSING_FIXTURE_SET_CORE_FIXTURE
    class FixtureSetCoreFixture final : public ClientPacket
    {
    public:
        explicit FixtureSetCoreFixture(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_FIXTURE_SET_CORE_FIXTURE, std::move(packet)) { }
        void Read() override;
        uint32 FixtureID = 0;
    };

    /// CMSG_HOUSING_FIXTURE_SET_HOUSE_SIZE
    class FixtureSetHouseSize final : public ClientPacket
    {
    public:
        explicit FixtureSetHouseSize(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_FIXTURE_SET_HOUSE_SIZE, std::move(packet)) { }
        void Read() override;
        uint8 HouseSize = 0;   // 0=Small, 1=Medium, 2=Large
    };

    /// CMSG_HOUSING_FIXTURE_SET_HOUSE_TYPE
    class FixtureSetHouseType final : public ClientPacket
    {
    public:
        explicit FixtureSetHouseType(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_FIXTURE_SET_HOUSE_TYPE, std::move(packet)) { }
        void Read() override;
        uint32 HouseTypeID = 0;
    };

    /// CMSG_HOUSING_FIXTURE_CREATE_FIXTURE
    class FixtureCreateFixture final : public ClientPacket
    {
    public:
        explicit FixtureCreateFixture(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_FIXTURE_CREATE_FIXTURE, std::move(packet)) { }
        void Read() override;
        uint32 Type       = 0;
        uint32 FixtureID  = 0;
        uint32 Slot       = 0;
    };

    /// CMSG_HOUSING_FIXTURE_DELETE_FIXTURE
    class FixtureDeleteFixture final : public ClientPacket
    {
    public:
        explicit FixtureDeleteFixture(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_FIXTURE_DELETE_FIXTURE, std::move(packet)) { }
        void Read() override;
        uint64 FixtureID = 0;
    };

    // =========================================================
    // Phase 3 — CMSG Room
    // =========================================================

    /// CMSG_HOUSING_ROOM_ADD
    class RoomAdd final : public ClientPacket
    {
    public:
        explicit RoomAdd(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_ROOM_ADD, std::move(packet)) { }
        void Read() override;
        float PosX        = 0.0f;
        float PosY        = 0.0f;
        float PosZ        = 0.0f;
        float Orientation = 0.0f;
        float SizeX       = 10.0f;
        float SizeY       = 10.0f;
    };

    /// CMSG_HOUSING_ROOM_REMOVE
    class RoomRemove final : public ClientPacket
    {
    public:
        explicit RoomRemove(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_ROOM_REMOVE, std::move(packet)) { }
        void Read() override;
        uint64 RoomID = 0;
    };

    /// CMSG_HOUSING_ROOM_MOVE
    class RoomMove final : public ClientPacket
    {
    public:
        explicit RoomMove(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_ROOM_MOVE, std::move(packet)) { }
        void Read() override;
        uint64 RoomID       = 0;
        float  PosX         = 0.0f;
        float  PosY         = 0.0f;
        float  PosZ         = 0.0f;
        float  Orientation  = 0.0f;
    };

    /// CMSG_HOUSING_ROOM_ROTATE
    class RoomRotate final : public ClientPacket
    {
    public:
        explicit RoomRotate(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_ROOM_ROTATE, std::move(packet)) { }
        void Read() override;
        uint64 RoomID       = 0;
        float  Orientation  = 0.0f;
    };

    /// CMSG_HOUSING_ROOM_SET_CEILING_TYPE
    class RoomSetCeilingType final : public ClientPacket
    {
    public:
        explicit RoomSetCeilingType(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_ROOM_SET_CEILING_TYPE, std::move(packet)) { }
        void Read() override;
        uint64 RoomID      = 0;
        uint32 CeilingType = 0;
    };

    /// CMSG_HOUSING_ROOM_SET_DOOR_TYPE
    class RoomSetDoorType final : public ClientPacket
    {
    public:
        explicit RoomSetDoorType(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_ROOM_SET_DOOR_TYPE, std::move(packet)) { }
        void Read() override;
        uint64 RoomID    = 0;
        uint32 DoorType  = 0;
    };

    /// CMSG_HOUSING_ROOM_SET_COMPONENT_THEME
    class RoomSetComponentTheme final : public ClientPacket
    {
    public:
        explicit RoomSetComponentTheme(WorldPacket&& packet)
            : ClientPacket(CMSG_HOUSING_ROOM_SET_COMPONENT_THEME, std::move(packet)) { }
        void Read() override;
        uint64 RoomID   = 0;
        uint32 ThemeID  = 0;
    };

    // =========================================================
    // Phase 2 — SMSG
    // =========================================================

    class HouseStatusResponse final : public ServerPacket
    {
    public:
        explicit HouseStatusResponse() : ServerPacket(SMSG_HOUSING_HOUSE_STATUS_RESPONSE) { }
        WorldPacket const* Write() override;
        bool                    HasHouse = false;
        Optional<HouseInfo>     House;
    };

    class GetCurrentHouseInfoResponse final : public ServerPacket
    {
    public:
        explicit GetCurrentHouseInfoResponse() : ServerPacket(SMSG_HOUSING_GET_CURRENT_HOUSE_INFO_RESPONSE) { }
        WorldPacket const* Write() override;
        bool                    HasHouse = false;
        Optional<FullHouseData> HouseData;
    };

    class GetPlayerPermissionsResponse final : public ServerPacket
    {
    public:
        explicit GetPlayerPermissionsResponse() : ServerPacket(SMSG_HOUSING_GET_PLAYER_PERMISSIONS_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 PermissionLevel = 0;
    };

    class SvcsGetPlayerHousesInfoResponse final : public ServerPacket
    {
    public:
        explicit SvcsGetPlayerHousesInfoResponse() : ServerPacket(SMSG_HOUSING_SVCS_GET_PLAYER_HOUSES_INFO_RESPONSE) { }
        WorldPacket const* Write() override;
        std::vector<HouseInfo> Houses;
    };

    class SvcsNotifyPermissionsFailure final : public ServerPacket
    {
    public:
        explicit SvcsNotifyPermissionsFailure() : ServerPacket(SMSG_HOUSING_SVCS_NOTIFY_PERMISSIONS_FAILURE) { }
        WorldPacket const* Write() override;
        uint32 Error = 0;
    };

    class SvcsRelinquishHouseResponse final : public ServerPacket
    {
    public:
        explicit SvcsRelinquishHouseResponse() : ServerPacket(SMSG_HOUSING_SVCS_RELINQUISH_HOUSE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error = 0;
    };

    class SvcsUpdateHouseSettingsResponse final : public ServerPacket
    {
    public:
        explicit SvcsUpdateHouseSettingsResponse() : ServerPacket(SMSG_HOUSING_SVCS_UPDATE_HOUSE_SETTINGS_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error = 0;
    };

    class SvcsNeighborhoodReservePlotResponse final : public ServerPacket
    {
    public:
        explicit SvcsNeighborhoodReservePlotResponse() : ServerPacket(SMSG_HOUSING_SVCS_NEIGHBORHOOD_RESERVE_PLOT_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error  = 0;
        uint64 PlotID = 0;
    };

    class DecorSetEditModeResponse final : public ServerPacket
    {
    public:
        explicit DecorSetEditModeResponse() : ServerPacket(SMSG_HOUSING_DECOR_SET_EDIT_MODE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error   = 0;
        bool   Enabled = false;
    };

    class FixtureSetEditModeResponse final : public ServerPacket
    {
    public:
        explicit FixtureSetEditModeResponse() : ServerPacket(SMSG_HOUSING_FIXTURE_SET_EDIT_MODE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error   = 0;
        bool   Enabled = false;
    };

    class RoomSetLayoutEditModeResponse final : public ServerPacket
    {
    public:
        explicit RoomSetLayoutEditModeResponse() : ServerPacket(SMSG_HOUSING_ROOM_SET_LAYOUT_EDIT_MODE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error   = 0;
        bool   Enabled = false;
    };

    // =========================================================
    // Phase 3 — SMSG Decoration
    // =========================================================

    class DecorPlaceResponse final : public ServerPacket
    {
    public:
        explicit DecorPlaceResponse() : ServerPacket(SMSG_HOUSING_DECOR_PLACE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32              Error   = 0;
        HouseDecorInfo      Decor;          // Full decor data with assigned ID
    };

    class DecorMoveResponse final : public ServerPacket
    {
    public:
        explicit DecorMoveResponse() : ServerPacket(SMSG_HOUSING_DECOR_MOVE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error   = 0;
        uint64 DecorID = 0;
    };

    class DecorRemoveResponse final : public ServerPacket
    {
    public:
        explicit DecorRemoveResponse() : ServerPacket(SMSG_HOUSING_DECOR_REMOVE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error   = 0;
        uint64 DecorID = 0;
        uint32 ItemID  = 0;   // Item returned to bag (0 = nothing returned)
    };

    class DecorLockResponse final : public ServerPacket
    {
    public:
        explicit DecorLockResponse() : ServerPacket(SMSG_HOUSING_DECOR_LOCK_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error   = 0;
        uint64 DecorID = 0;
        bool   Locked  = false;
    };

    class DecorRequestStorageResponse final : public ServerPacket
    {
    public:
        explicit DecorRequestStorageResponse() : ServerPacket(SMSG_HOUSING_DECOR_REQUEST_STORAGE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32                          Error = 0;
        std::vector<DecorStorageItem>   Items;
    };

    // =========================================================
    // Phase 3 — SMSG Fixture
    // =========================================================

    class FixtureSetCoreFixtureResponse final : public ServerPacket
    {
    public:
        explicit FixtureSetCoreFixtureResponse() : ServerPacket(SMSG_HOUSING_FIXTURE_SET_CORE_FIXTURE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error     = 0;
        uint32 FixtureID = 0;
    };

    class FixtureSetHouseSizeResponse final : public ServerPacket
    {
    public:
        explicit FixtureSetHouseSizeResponse() : ServerPacket(SMSG_HOUSING_FIXTURE_SET_HOUSE_SIZE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error     = 0;
        uint8  HouseSize = 0;
    };

    class FixtureSetHouseTypeResponse final : public ServerPacket
    {
    public:
        explicit FixtureSetHouseTypeResponse() : ServerPacket(SMSG_HOUSING_FIXTURE_SET_HOUSE_TYPE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error       = 0;
        uint32 HouseTypeID = 0;
    };

    class FixtureCreateFixtureResponse final : public ServerPacket
    {
    public:
        explicit FixtureCreateFixtureResponse() : ServerPacket(SMSG_HOUSING_FIXTURE_CREATE_FIXTURE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32          Error = 0;
        HouseFixtureInfo Fixture;   // Full fixture with assigned ID
    };

    class FixtureDeleteFixtureResponse final : public ServerPacket
    {
    public:
        explicit FixtureDeleteFixtureResponse() : ServerPacket(SMSG_HOUSING_FIXTURE_DELETE_FIXTURE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error     = 0;
        uint64 FixtureID = 0;
    };

    // =========================================================
    // Phase 3 — SMSG Room
    // =========================================================

    class RoomAddResponse final : public ServerPacket
    {
    public:
        explicit RoomAddResponse() : ServerPacket(SMSG_HOUSING_ROOM_ADD_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32        Error = 0;
        HouseRoomInfo Room;     // Full room data with assigned ID
    };

    class RoomRemoveResponse final : public ServerPacket
    {
    public:
        explicit RoomRemoveResponse() : ServerPacket(SMSG_HOUSING_ROOM_REMOVE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error  = 0;
        uint64 RoomID = 0;
    };

    /// SMSG_HOUSING_ROOM_UPDATE_RESPONSE — shared for Move and Rotate
    class RoomUpdateResponse final : public ServerPacket
    {
    public:
        explicit RoomUpdateResponse() : ServerPacket(SMSG_HOUSING_ROOM_UPDATE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32        Error = 0;
        HouseRoomInfo Room;
    };

    class RoomSetCeilingTypeResponse final : public ServerPacket
    {
    public:
        explicit RoomSetCeilingTypeResponse() : ServerPacket(SMSG_HOUSING_ROOM_SET_CEILING_TYPE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error       = 0;
        uint64 RoomID      = 0;
        uint32 CeilingType = 0;
    };

    class RoomSetDoorTypeResponse final : public ServerPacket
    {
    public:
        explicit RoomSetDoorTypeResponse() : ServerPacket(SMSG_HOUSING_ROOM_SET_DOOR_TYPE_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error    = 0;
        uint64 RoomID   = 0;
        uint32 DoorType = 0;
    };

    class RoomSetComponentThemeResponse final : public ServerPacket
    {
    public:
        explicit RoomSetComponentThemeResponse() : ServerPacket(SMSG_HOUSING_ROOM_SET_COMPONENT_THEME_RESPONSE) { }
        WorldPacket const* Write() override;
        uint32 Error   = 0;
        uint64 RoomID  = 0;
        uint32 ThemeID = 0;
    };

} // namespace WorldPackets::Housing

// ByteBuffer operators
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Housing::HouseInfo const& info);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Housing::HouseDecorInfo const& decor);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Housing::HouseFixtureInfo const& fixture);
ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Housing::HouseRoomInfo const& room);

#endif // TRINITYCORE_HOUSING_PACKETS_H
