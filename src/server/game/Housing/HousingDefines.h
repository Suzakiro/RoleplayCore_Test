/*
 * This file is part of the RoleplayCore Project.
 *
 * Housing System - Defines, Enums and Data Structures
 */

#ifndef HOUSING_DEFINES_H
#define HOUSING_DEFINES_H

#include "Define.h"
#include "ObjectGuid.h"
#include <string>
#include <vector>

namespace Housing
{
    // =========================================================
    // Enums
    // =========================================================

    enum class HouseSize : uint8
    {
        Small   = 0,
        Medium  = 1,
        Large   = 2,
        Max     = 3
    };

    enum class HouseStatus : uint8
    {
        None        = 0,
        Active      = 1,
        Inactive    = 2,
        Locked      = 3
    };

    enum class PlotStatus : uint8
    {
        Empty       = 0,
        Reserved    = 1,
        Occupied    = 2,
        Locked      = 3
    };

    enum class NeighborhoodType : uint8
    {
        Player  = 0,
        Guild   = 1
    };

    enum class MemberStatus : uint8
    {
        Invited     = 0,
        Accepted    = 1,
        Declined    = 2
    };

    enum class DecorationType : uint32
    {
        Object  = 0,
        Light   = 1,
        Npc     = 2,
        Music   = 3,
        Effect  = 4
    };

    enum class FixtureType : uint32
    {
        Core        = 0,
        Door        = 1,
        Window      = 2,
        Roof        = 3,
        Foundation  = 4
    };

    enum class RoomCeilingType : uint32
    {
        Standard    = 0,
        Vaulted     = 1,
        Open        = 2
    };

    enum class RoomDoorType : uint32
    {
        Standard    = 0,
        Arch        = 1,
        Open        = 2,
        Closed      = 3
    };

    // Error codes mapped to client SMSG responses
    enum class HousingError : uint32
    {
        Success                 = 0,
        InternalError           = 1,
        InvalidPlot             = 2,
        PlotNotAvailable        = 3,
        NotOwner                = 4,
        NoHouse                 = 5,
        HouseAlreadyExists      = 6,
        InvalidDecoration       = 7,
        DecorationNotFound      = 8,
        InvalidFixture          = 9,
        InvalidRoom             = 10,
        NeighborhoodFull        = 11,
        NotInNeighborhood       = 12,
        InvitePending           = 13,
        AlreadyMember           = 14,
        PermissionDenied        = 15,
        MaxDecorationsReached   = 16,
        InvalidHouseSize        = 17,
        InvalidHouseName        = 18
    };


    // Permission levels — used by GetPlayerPermissions handler and HousingMgr
    enum class HousingPermission : uint32
    {
        None            = 0,
        Visit           = 1,
        Decorate        = 2,
        Owner           = 3,
        SecondaryOwner  = 4
    };

    // =========================================================
    // Constants
    // =========================================================

    // Max active decorations per house size
    constexpr uint32 MAX_DECORATIONS_SMALL      = 100;
    constexpr uint32 MAX_DECORATIONS_MEDIUM     = 200;
    constexpr uint32 MAX_DECORATIONS_LARGE      = 400;

    // Max rooms per house size
    constexpr uint32 MAX_ROOMS_SMALL            = 4;
    constexpr uint32 MAX_ROOMS_MEDIUM           = 8;
    constexpr uint32 MAX_ROOMS_LARGE            = 16;

    // Phase ID base for housing neighborhoods
    // Each neighborhood gets: HOUSING_PHASE_BASE + neighborhood_id
    constexpr uint32 HOUSING_PHASE_BASE         = 100000;

    // Max neighborhood name length
    constexpr uint32 MAX_NEIGHBORHOOD_NAME_LEN  = 128;

    // Max house name / description length
    constexpr uint32 MAX_HOUSE_NAME_LEN         = 64;
    constexpr uint32 MAX_HOUSE_DESC_LEN         = 512;

    // =========================================================
    // Data Structures
    // =========================================================

    struct HousingDecoration
    {
        uint64          Id          = 0;
        uint64          HouseId     = 0;
        uint32          ItemId      = 0;
        float           PosX        = 0.0f;
        float           PosY        = 0.0f;
        float           PosZ        = 0.0f;
        float           RotX        = 0.0f;
        float           RotY        = 0.0f;
        float           RotZ        = 0.0f;
        float           RotW        = 1.0f;
        float           Scale       = 1.0f;
        DecorationType  Type        = DecorationType::Object;
        bool            IsLocked    = false;

        // Dirty tracking — not persisted
        bool            IsNew       = false;
        bool            IsDirty     = false;
        bool            IsDeleted   = false;
    };

    struct HousingFixture
    {
        uint64      Id          = 0;
        uint64      HouseId     = 0;
        FixtureType Type        = FixtureType::Core;
        uint32      FixtureId   = 0;
        uint32      Slot        = 0;

        bool        IsDirty     = false;
        bool        IsNew       = false;
        bool        IsDeleted   = false;
    };

    struct HousingRoom
    {
        uint64          Id              = 0;
        uint64          HouseId         = 0;
        uint32          RoomIndex       = 0;
        float           PosX            = 0.0f;
        float           PosY            = 0.0f;
        float           PosZ            = 0.0f;
        float           Orientation     = 0.0f;
        float           SizeX           = 10.0f;
        float           SizeY           = 10.0f;
        RoomCeilingType CeilingType     = RoomCeilingType::Standard;
        RoomDoorType    DoorType        = RoomDoorType::Standard;
        uint32          ComponentTheme  = 0;

        bool            IsDirty         = false;
        bool            IsNew           = false;
        bool            IsDeleted       = false;
    };

    struct PlayerHouse
    {
        uint64      Id              = 0;
        ObjectGuid  OwnerGuid;
        uint64      PlotId          = 0;
        uint32      HouseType       = 0;
        HouseSize   Size            = HouseSize::Small;
        HouseStatus Status          = HouseStatus::None;
        std::string Name;
        std::string Description;
        bool        IsPublic        = false;
        bool        AllowVisits     = true;

        std::vector<HousingDecoration>  Decorations;
        std::vector<HousingFixture>     Fixtures;
        std::vector<HousingRoom>        Rooms;

        // Dirty tracking
        bool IsDirty    = false;
        bool IsNew      = false;

        uint32 GetMaxDecorations() const
        {
            switch (Size)
            {
                case HouseSize::Small:  return MAX_DECORATIONS_SMALL;
                case HouseSize::Medium: return MAX_DECORATIONS_MEDIUM;
                case HouseSize::Large:  return MAX_DECORATIONS_LARGE;
                default:                return MAX_DECORATIONS_SMALL;
            }
        }

        uint32 GetMaxRooms() const
        {
            switch (Size)
            {
                case HouseSize::Small:  return MAX_ROOMS_SMALL;
                case HouseSize::Medium: return MAX_ROOMS_MEDIUM;
                case HouseSize::Large:  return MAX_ROOMS_LARGE;
                default:                return MAX_ROOMS_SMALL;
            }
        }

        uint32 GetActiveDecorationCount() const
        {
            uint32 count = 0;
            for (auto const& d : Decorations)
                if (!d.IsDeleted)
                    ++count;
            return count;
        }
    };

    struct NeighborhoodMember
    {
        ObjectGuid  Guid;
        ObjectGuid  InvitedBy;
        MemberStatus Status              = MemberStatus::Invited;
        bool        IsSecondaryOwner     = false;
    };

    struct HousingPlot
    {
        uint64      Id              = 0;
        uint64      NeighborhoodId  = 0;
        uint32      PlotIndex       = 0;
        float       PosX            = 0.0f;
        float       PosY            = 0.0f;
        float       PosZ            = 0.0f;
        float       Orientation     = 0.0f;
        uint32      MapId           = 0;
        HouseSize   Size            = HouseSize::Small;
        PlotStatus  Status          = PlotStatus::Empty;
        uint64      HouseId         = 0; // FK → character_housing.id (0 if empty)
    };

    struct HousingNeighborhood
    {
        uint64              Id              = 0;
        std::string         Name;
        uint32              MapId           = 0;
        uint32              PhaseId         = 0; // Applied on enter
        uint32              MaxPlots        = 0;
        bool                IsPublic        = false;
        uint64              OwnerGuildId    = 0;
        NeighborhoodType    Type            = NeighborhoodType::Player;

        std::vector<HousingPlot>            Plots;
        std::vector<NeighborhoodMember>     Members;

        uint32 GetAvailablePlotCount() const
        {
            uint32 count = 0;
            for (auto const& p : Plots)
                if (p.Status == PlotStatus::Empty)
                    ++count;
            return count;
        }

        HousingPlot const* GetPlotByIndex(uint32 index) const
        {
            for (auto const& p : Plots)
                if (p.PlotIndex == index)
                    return &p;
            return nullptr;
        }
    };

} // namespace Housing

#endif // HOUSING_DEFINES_H
