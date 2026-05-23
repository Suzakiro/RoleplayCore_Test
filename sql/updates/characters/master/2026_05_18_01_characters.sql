ALTER TABLE `housing_plots`
ADD COLUMN `phase_id` INT UNSIGNED NOT NULL DEFAULT 1 AFTER `id`,
ADD COLUMN `neighborhood` VARCHAR(64) NOT NULL DEFAULT 'founders_point' AFTER `phase_id`,
ADD COLUMN `plot_number` TINYINT UNSIGNED NOT NULL DEFAULT 1 AFTER `neighborhood`;

ALTER TABLE `housing_plots`
ADD UNIQUE KEY `uk_housing_neighborhood_plot_number`
(`neighborhood`, `plot_number`);

UPDATE `housing_plots`
SET `phase_id` = 1;

DELETE FROM `housing_plots`
WHERE `owner_guid` = 0 OR `owner_guid` IS NULL;