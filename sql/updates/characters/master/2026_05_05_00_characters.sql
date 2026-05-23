-- One-house-per-player prototype persistence for housing buy guard
CREATE TABLE IF NOT EXISTS `character_house_ownership` (
  `guid` bigint unsigned NOT NULL,
  `created_at` int unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;