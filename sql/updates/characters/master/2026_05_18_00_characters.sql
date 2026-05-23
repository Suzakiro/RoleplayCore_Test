ALTER TABLE `character_house_ownership`
    ADD COLUMN IF NOT EXISTS `plot_id` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `guid`;

CREATE TABLE IF NOT EXISTS `housing_plots` (
    `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
    `map_id` SMALLINT UNSIGNED NOT NULL,
    `pos_x` FLOAT NOT NULL,
    `pos_y` FLOAT NOT NULL,
    `pos_z` FLOAT NOT NULL,
    `orientation` FLOAT NOT NULL DEFAULT 0,
    `active` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `owner_guid` INT UNSIGNED NOT NULL DEFAULT 0,
    `owned_at` INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`id`),
    KEY `idx_active_owner` (`active`, `owner_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `character_house_storage` (
    `owner_guid` INT UNSIGNED NOT NULL,
    `item_guid` BIGINT UNSIGNED NOT NULL,
    `slot` INT UNSIGNED NOT NULL DEFAULT 0,
    `count` INT UNSIGNED NOT NULL DEFAULT 1,
    `created_at` INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`owner_guid`, `item_guid`),
    KEY `idx_owner_slot` (`owner_guid`, `slot`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `character_house_decor` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `owner_guid` INT UNSIGNED NOT NULL,
    `decor_item_id` INT UNSIGNED NOT NULL,
    `pos_x` FLOAT NOT NULL,
    `pos_y` FLOAT NOT NULL,
    `pos_z` FLOAT NOT NULL,
    `rot_x` FLOAT NOT NULL DEFAULT 0,
    `rot_y` FLOAT NOT NULL DEFAULT 0,
    `rot_z` FLOAT NOT NULL DEFAULT 0,
    `updated_at` INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`id`),
    KEY `idx_owner` (`owner_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `character_house_permissions` (
    `owner_guid` INT UNSIGNED NOT NULL,
    `guest_guid` INT UNSIGNED NOT NULL,
    `permission_mask` INT UNSIGNED NOT NULL DEFAULT 1,
    `created_at` INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`owner_guid`, `guest_guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `housing_plots` (`map_id`, `pos_x`, `pos_y`, `pos_z`, `orientation`, `active`)
VALUES
    (0, -8842.09, 626.358, 95.547, 0, 1),
    (1, 1633.33, -4440.34, 15.4067, 0, 1),
    (0, -8913.23, 554.633, 94.794, 0, 1),
    (1, 1538.47, -4412.22, 12.832, 0, 1)
ON DUPLICATE KEY UPDATE `active` = VALUES(`active`);