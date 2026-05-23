-- ============================================================
-- RoleplayCore - Housing System Schema
-- Version: 1.0.0
-- Databases: roleplay (static world data), characters (player data)
-- ============================================================

-- ============================================================
-- ROLEPLAY DB — Static world data
-- ============================================================

USE roleplay;

-- ----------------------------
-- Table: housing_neighborhoods
-- Defines a named housing zone. One phase per neighborhood.
-- phase_id is computed server-side as HOUSING_PHASE_BASE + id.
-- ----------------------------
DROP TABLE IF EXISTS `housing_neighborhoods`;
CREATE TABLE `housing_neighborhoods` (
    `id`               BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `name`             VARCHAR(128)    NOT NULL DEFAULT '',
    `map_id`           INT UNSIGNED    NOT NULL DEFAULT 0     COMMENT 'World map ID where this neighborhood exists',
    `max_plots`        INT UNSIGNED    NOT NULL DEFAULT 0     COMMENT 'Total plot capacity (0 = use count from housing_plots)',
    `is_public`        TINYINT UNSIGNED NOT NULL DEFAULT 1    COMMENT '1 = any player can see it, 0 = invite-only',
    `owner_guild_id`   BIGINT UNSIGNED NOT NULL DEFAULT 0     COMMENT '0 = player neighborhood, >0 = guild neighborhood',
    `type`             TINYINT UNSIGNED NOT NULL DEFAULT 0    COMMENT '0=Player, 1=Guild',
    `created`          DATETIME        NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
  COMMENT='Housing neighborhood definitions. One phase per row.';

-- ----------------------------
-- Table: housing_plots
-- One row per available housing slot inside a neighborhood.
-- status: 0=Empty, 1=Reserved, 2=Occupied, 3=Locked
-- size:   0=Small, 1=Medium, 2=Large
-- house_id: FK to characters.character_housing.id (0 if empty)
-- ----------------------------
DROP TABLE IF EXISTS `housing_plots`;
CREATE TABLE `housing_plots` (
    `id`               BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `neighborhood_id`  BIGINT UNSIGNED NOT NULL                COMMENT 'FK → housing_neighborhoods.id',
    `plot_index`       INT UNSIGNED    NOT NULL DEFAULT 0      COMMENT 'Slot index within the neighborhood (0-based)',
    `pos_x`            FLOAT           NOT NULL DEFAULT 0      COMMENT 'World X position of the plot entrance',
    `pos_y`            FLOAT           NOT NULL DEFAULT 0,
    `pos_z`            FLOAT           NOT NULL DEFAULT 0,
    `orientation`      FLOAT           NOT NULL DEFAULT 0,
    `map_id`           INT UNSIGNED    NOT NULL DEFAULT 0      COMMENT 'May differ from neighborhood map_id for instanced plots',
    `size`             TINYINT UNSIGNED NOT NULL DEFAULT 0     COMMENT '0=Small, 1=Medium, 2=Large',
    `status`           TINYINT UNSIGNED NOT NULL DEFAULT 0     COMMENT '0=Empty, 1=Reserved, 2=Occupied, 3=Locked',
    `house_id`         BIGINT UNSIGNED NOT NULL DEFAULT 0      COMMENT 'FK → characters.character_housing.id',
    PRIMARY KEY (`id`),
    KEY `idx_neighborhood` (`neighborhood_id`),
    KEY `idx_status` (`status`),
    CONSTRAINT `fk_plot_neighborhood`
        FOREIGN KEY (`neighborhood_id`) REFERENCES `housing_neighborhoods` (`id`)
        ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
  COMMENT='Plot/parcel slots within a neighborhood';

-- ============================================================
-- CHARACTERS DB — Player data
-- ============================================================

USE characters;

-- ----------------------------
-- Table: character_housing
-- One row per player house. Owns all decorations/fixtures/rooms.
-- status: 0=None, 1=Active, 2=Inactive, 3=Locked
-- size:   0=Small, 1=Medium, 2=Large
-- ----------------------------
DROP TABLE IF EXISTS `character_housing`;
CREATE TABLE `character_housing` (
    `id`               BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `owner_guid`       BIGINT UNSIGNED NOT NULL                COMMENT 'Character GUID',
    `plot_id`          BIGINT UNSIGNED NOT NULL                COMMENT 'FK → roleplay.housing_plots.id',
    `house_type`       INT UNSIGNED    NOT NULL DEFAULT 0      COMMENT 'HouseTypeDB2 ID (0 = default)',
    `size`             TINYINT UNSIGNED NOT NULL DEFAULT 0     COMMENT '0=Small, 1=Medium, 2=Large',
    `status`           TINYINT UNSIGNED NOT NULL DEFAULT 1     COMMENT '0=None, 1=Active, 2=Inactive, 3=Locked',
    `name`             VARCHAR(64)     NOT NULL DEFAULT ''     COMMENT 'House display name',
    `description`      VARCHAR(512)    NOT NULL DEFAULT ''     COMMENT 'House description / lore text',
    `is_public`        TINYINT UNSIGNED NOT NULL DEFAULT 0     COMMENT '1 = visible in House Finder',
    `allow_visits`     TINYINT UNSIGNED NOT NULL DEFAULT 1     COMMENT '1 = anyone can visit',
    `created`          DATETIME        NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `last_modified`    DATETIME        NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    UNIQUE KEY `idx_owner` (`owner_guid`),
    KEY `idx_plot` (`plot_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
  COMMENT='Player house ownership records';

-- ----------------------------
-- Table: housing_decorations
-- Placed objects/lights/npcs inside a house.
-- type: 0=Object, 1=Light, 2=Npc, 3=Music, 4=Effect
-- ----------------------------
DROP TABLE IF EXISTS `housing_decorations`;
CREATE TABLE `housing_decorations` (
    `id`        BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `house_id`  BIGINT UNSIGNED NOT NULL              COMMENT 'FK → character_housing.id',
    `item_id`   INT UNSIGNED    NOT NULL DEFAULT 0    COMMENT 'Item entry (HousingDecorDB2)',
    `pos_x`     FLOAT           NOT NULL DEFAULT 0,
    `pos_y`     FLOAT           NOT NULL DEFAULT 0,
    `pos_z`     FLOAT           NOT NULL DEFAULT 0,
    `rot_x`     FLOAT           NOT NULL DEFAULT 0    COMMENT 'Quaternion X',
    `rot_y`     FLOAT           NOT NULL DEFAULT 0    COMMENT 'Quaternion Y',
    `rot_z`     FLOAT           NOT NULL DEFAULT 0    COMMENT 'Quaternion Z',
    `rot_w`     FLOAT           NOT NULL DEFAULT 1    COMMENT 'Quaternion W',
    `scale`     FLOAT           NOT NULL DEFAULT 1    COMMENT 'Object scale multiplier',
    `type`      INT UNSIGNED    NOT NULL DEFAULT 0    COMMENT '0=Object, 1=Light, 2=Npc, 3=Music, 4=Effect',
    `is_locked` TINYINT UNSIGNED NOT NULL DEFAULT 0  COMMENT '1 = cannot be moved without unlock',
    PRIMARY KEY (`id`),
    KEY `idx_house` (`house_id`),
    CONSTRAINT `fk_decor_house`
        FOREIGN KEY (`house_id`) REFERENCES `character_housing` (`id`)
        ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
  COMMENT='Placed decoration objects inside player houses';

-- ----------------------------
-- Table: housing_fixtures
-- Structural fixtures: doors, windows, roof, core house model.
-- type: 0=Core, 1=Door, 2=Window, 3=Roof, 4=Foundation
-- ----------------------------
DROP TABLE IF EXISTS `housing_fixtures`;
CREATE TABLE `housing_fixtures` (
    `id`          BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `house_id`    BIGINT UNSIGNED NOT NULL           COMMENT 'FK → character_housing.id',
    `type`        INT UNSIGNED    NOT NULL DEFAULT 0 COMMENT '0=Core, 1=Door, 2=Window, 3=Roof, 4=Foundation',
    `fixture_id`  INT UNSIGNED    NOT NULL DEFAULT 0 COMMENT 'HousingFixtureDB2 entry ID',
    `slot`        INT UNSIGNED    NOT NULL DEFAULT 0 COMMENT 'Slot index for multi-fixture types',
    PRIMARY KEY (`id`),
    KEY `idx_house` (`house_id`),
    CONSTRAINT `fk_fixture_house`
        FOREIGN KEY (`house_id`) REFERENCES `character_housing` (`id`)
        ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
  COMMENT='Structural fixture assignments for player houses';

-- ----------------------------
-- Table: housing_rooms
-- Room layout inside a house.
-- ceiling_type: 0=Standard, 1=Vaulted, 2=Open
-- door_type:    0=Standard, 1=Arch, 2=Open, 3=Closed
-- ----------------------------
DROP TABLE IF EXISTS `housing_rooms`;
CREATE TABLE `housing_rooms` (
    `id`               BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `house_id`         BIGINT UNSIGNED NOT NULL           COMMENT 'FK → character_housing.id',
    `room_index`       INT UNSIGNED    NOT NULL DEFAULT 0 COMMENT 'Room slot index (0-based)',
    `pos_x`            FLOAT           NOT NULL DEFAULT 0,
    `pos_y`            FLOAT           NOT NULL DEFAULT 0,
    `pos_z`            FLOAT           NOT NULL DEFAULT 0,
    `orientation`      FLOAT           NOT NULL DEFAULT 0,
    `size_x`           FLOAT           NOT NULL DEFAULT 10 COMMENT 'Room width',
    `size_y`           FLOAT           NOT NULL DEFAULT 10 COMMENT 'Room depth',
    `ceiling_type`     INT UNSIGNED    NOT NULL DEFAULT 0  COMMENT '0=Standard, 1=Vaulted, 2=Open',
    `door_type`        INT UNSIGNED    NOT NULL DEFAULT 0  COMMENT '0=Standard, 1=Arch, 2=Open, 3=Closed',
    `component_theme`  INT UNSIGNED    NOT NULL DEFAULT 0  COMMENT 'HousingComponentThemeDB2 entry ID',
    PRIMARY KEY (`id`),
    KEY `idx_house` (`house_id`),
    CONSTRAINT `fk_room_house`
        FOREIGN KEY (`house_id`) REFERENCES `character_housing` (`id`)
        ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
  COMMENT='Room layout data for player houses';

-- ----------------------------
-- Table: housing_neighborhood_members
-- Players invited/accepted into a neighborhood.
-- status: 0=Invited, 1=Accepted, 2=Declined
-- ----------------------------
DROP TABLE IF EXISTS `housing_neighborhood_members`;
CREATE TABLE `housing_neighborhood_members` (
    `neighborhood_id`    BIGINT UNSIGNED  NOT NULL          COMMENT 'FK → roleplay.housing_neighborhoods.id',
    `player_guid`        BIGINT UNSIGNED  NOT NULL          COMMENT 'Invited player character GUID',
    `invited_by_guid`    BIGINT UNSIGNED  NOT NULL DEFAULT 0,
    `status`             TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0=Invited, 1=Accepted, 2=Declined',
    `is_secondary_owner` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `joined`             DATETIME         NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`neighborhood_id`, `player_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
  COMMENT='Neighborhood membership/invite records';

-- ============================================================
-- SAMPLE DATA — 1 neighborhood, 4 plots
-- Replace map_id and coordinates with your actual housing map
-- ============================================================

USE roleplay;

INSERT INTO `housing_neighborhoods`
    (`id`, `name`, `map_id`, `max_plots`, `is_public`, `owner_guild_id`, `type`)
VALUES
    (1, 'Elwynn Gardens', 0, 4, 1, 0, 0);

INSERT INTO `housing_plots`
    (`neighborhood_id`, `plot_index`, `pos_x`, `pos_y`, `pos_z`, `orientation`, `map_id`, `size`, `status`)
VALUES
    (1, 0, -9016.0, 466.0, 93.0, 1.57, 0, 0, 0),
    (1, 1, -9030.0, 466.0, 93.0, 1.57, 0, 0, 0),
    (1, 2, -9016.0, 450.0, 93.0, 1.57, 0, 1, 0),
    (1, 3, -9030.0, 450.0, 93.0, 1.57, 0, 2, 0);

-- ============================================================
-- RBAC — Housing permissions
-- ============================================================

USE auth;

INSERT IGNORE INTO `rbac_permissions` VALUES (3100, 'Housing: use housing system');
INSERT IGNORE INTO `rbac_permissions` VALUES (3101, 'Housing: admin all houses');
INSERT IGNORE INTO `rbac_permissions` VALUES (3102, 'Command: .housing create');
INSERT IGNORE INTO `rbac_permissions` VALUES (3103, 'Command: .housing delete');
INSERT IGNORE INTO `rbac_permissions` VALUES (3104, 'Command: .housing teleport');

-- Link to role 2 (Player) and role 1 (Admin) as appropriate
INSERT IGNORE INTO `rbac_linked_permissions` VALUES (2, 3100);
INSERT IGNORE INTO `rbac_linked_permissions` VALUES (2, 3102);
INSERT IGNORE INTO `rbac_linked_permissions` VALUES (2, 3104);
INSERT IGNORE INTO `rbac_linked_permissions` VALUES (1, 3101);
INSERT IGNORE INTO `rbac_linked_permissions` VALUES (1, 3103);
