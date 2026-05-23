CREATE TABLE IF NOT EXISTS `battlepay_storefront` (
    `id` INT UNSIGNED NOT NULL,
    `name` VARCHAR(120) NOT NULL,
    `active` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `battlepay_product` (
    `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
    `storefront_id` INT UNSIGNED NOT NULL,
    `product_type` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `group_id` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `item_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `name` VARCHAR(120) NOT NULL,
    `description` VARCHAR(255) NOT NULL DEFAULT '',
    `price_cents` INT UNSIGNED NOT NULL DEFAULT 0,
    `currency` VARCHAR(8) NOT NULL DEFAULT 'EUR',
    `active` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `sort_order` INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`id`),
    KEY `idx_storefront_active` (`storefront_id`, `active`),
    UNIQUE KEY `uk_storefront_name` (`storefront_id`, `name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `battlepay_groups` (
    `id` TINYINT UNSIGNED NOT NULL,
    `code` VARCHAR(32) NOT NULL,
    `name` VARCHAR(64) NOT NULL,
    `sort_order` SMALLINT UNSIGNED NOT NULL DEFAULT 0,
    `active` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uk_bpay_group_code` (`code`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `battlepay_storefront` (`id`, `name`, `active`)
VALUES
    (1, 'Default Storefront', 1)
ON DUPLICATE KEY UPDATE
    `name` = VALUES(`name`),
    `active` = VALUES(`active`);

INSERT INTO `battlepay_groups` (`id`, `code`, `name`, `sort_order`, `active`)
VALUES
    (1, 'services', 'Services', 10, 1),
    (2, 'mounts', 'Montures', 20, 1),
    (3, 'battlepets', 'Battle Pets', 30, 1),
    (4, 'heirloom', 'Heirloom', 40, 1),
    (5, 'potions', 'Potions', 50, 1)
ON DUPLICATE KEY UPDATE
    `name` = VALUES(`name`),
    `sort_order` = VALUES(`sort_order`),
    `active` = VALUES(`active`);

INSERT INTO `battlepay_product` (`storefront_id`, `product_type`, `group_id`, `item_id`, `name`, `description`, `price_cents`, `currency`, `active`, `sort_order`)
VALUES
    -- Montures boutique officielles (sélection)
    (1, 0, 2, 54811, 'Celestial Steed', 'Monture boutique.', 2500, 'EUR', 1, 10),
    (1, 0, 2, 78924, 'Heart of the Aspects', 'Monture boutique.', 2500, 'EUR', 1, 11),
    (1, 0, 2, 122703, 'Grinning Reaver', 'Monture boutique.', 2500, 'EUR', 1, 12),
    (1, 0, 2, 143660, 'Dreadwake Mount', 'Monture boutique.', 2500, 'EUR', 1, 13),
    (1, 0, 2, 153539, 'Seabraid Stallion', 'Monture boutique.', 2500, 'EUR', 1, 14),
    (1, 0, 2, 153540, 'Gilded Ravasaur', 'Monture boutique.', 2500, 'EUR', 1, 15),

    -- Sésame niveau 90
    (1, 1, 1, 0, 'Sésame Niveau 90', 'Boost personnage niveau 90.', 6000, 'EUR', 1, 100),

    -- Potions d''XP
    (1, 0, 5, 128312, 'Potion of Accelerated Learning', 'Potion d''XP.', 500, 'EUR', 1, 110),
    (1, 0, 5, 120182, 'Elixir of the Rapid Mind', 'Potion d''XP.', 900, 'EUR', 1, 111),

    -- Heirloom
    (1, 0, 4, 122349, 'Bloodied Arcanite Reaper', 'Heirloom.', 1200, 'EUR', 1, 120),
    (1, 0, 4, 122351, 'Charmed Ancient Bone Bow', 'Heirloom.', 1200, 'EUR', 1, 121),

    -- Services compte/personnage
    (1, 1, 1, 0, 'Character Rename', 'Service de renommage de personnage.', 1000, 'EUR', 1, 200),
    (1, 1, 1, 0, 'Faction Change', 'Service de changement de faction.', 3000, 'EUR', 1, 201),
    (1, 1, 1, 0, 'Appearance Customization', 'Service de personnalisation.', 1500, 'EUR', 1, 202),
    (1, 1, 1, 0, 'Race Change', 'Service de changement de race.', 2500, 'EUR', 1, 203),

    -- Battle pets boutique (cages/items)
    (1, 0, 3, 49693, 'Lil'' KT', 'Battle pet boutique.', 1000, 'EUR', 1, 300),
    (1, 0, 3, 90173, 'Cenarion Hatchling', 'Battle pet boutique.', 1000, 'EUR', 1, 301),
    (1, 0, 3, 93040, 'Anubisath Idol', 'Battle pet boutique.', 1000, 'EUR', 1, 302),
    (1, 0, 1, 23720, 'Tabard of Frost', 'Objet cosmétique boutique.', 700, 'EUR', 1, 400)
ON DUPLICATE KEY UPDATE
    `item_id` = VALUES(`item_id`),
    `product_type` = VALUES(`product_type`),
    `group_id` = VALUES(`group_id`),
    `description` = VALUES(`description`),
    `price_cents` = VALUES(`price_cents`),
    `currency` = VALUES(`currency`),
    `active` = VALUES(`active`),
    `sort_order` = VALUES(`sort_order`);