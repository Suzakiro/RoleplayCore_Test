# Housing System — Ajouts aux fichiers Database existants

## 1. RoleplayDatabase.h
### Ajouter dans l'enum `RoleplayDatabaseStatements`, avant `MAX_RoleplayDATABASE_STATEMENTS` :

```cpp
    // Housing System
    Roleplay_SEL_HOUSING_NEIGHBORHOODS,
    Roleplay_SEL_HOUSING_PLOTS,
    Roleplay_UPD_HOUSING_PLOT_STATUS,
```

---

## 2. RoleplayDatabase.cpp
### Ajouter dans `DoPrepareStatements()`, à la fin, avant la fermeture de la fonction :

```cpp
    // Housing System
    PrepareStatement(Roleplay_SEL_HOUSING_NEIGHBORHOODS,
        "SELECT id, name, map_id, max_plots, is_public, owner_guild_id, type "
        "FROM housing_neighborhoods",
        CONNECTION_SYNCH);

    PrepareStatement(Roleplay_SEL_HOUSING_PLOTS,
        "SELECT id, neighborhood_id, plot_index, pos_x, pos_y, pos_z, orientation, "
        "       map_id, size, status, house_id "
        "FROM housing_plots",
        CONNECTION_SYNCH);

    PrepareStatement(Roleplay_UPD_HOUSING_PLOT_STATUS,
        "UPDATE housing_plots SET status = ?, house_id = ? WHERE id = ?",
        CONNECTION_ASYNC);
```

---

## 3. CharacterDatabase.h
### Ajouter dans l'enum `CharacterDatabaseStatements`, juste avant `MAX_CHARACTERDATABASE_STATEMENTS` :

```cpp
    // Housing System
    CHAR_SEL_HOUSING_BY_GUID,
    CHAR_INS_HOUSING,
    CHAR_UPD_HOUSING,
    CHAR_DEL_HOUSING,

    CHAR_SEL_HOUSING_DECORATIONS,
    CHAR_INS_HOUSING_DECORATION,
    CHAR_UPD_HOUSING_DECORATION,
    CHAR_DEL_HOUSING_DECORATION,

    CHAR_SEL_HOUSING_FIXTURES,
    CHAR_INS_HOUSING_FIXTURE,
    CHAR_UPD_HOUSING_FIXTURE,
    CHAR_DEL_HOUSING_FIXTURE,

    CHAR_SEL_HOUSING_ROOMS,
    CHAR_INS_HOUSING_ROOM,
    CHAR_UPD_HOUSING_ROOM,
    CHAR_DEL_HOUSING_ROOM,

    CHAR_SEL_HOUSING_MEMBERS,
    CHAR_INS_HOUSING_MEMBER,
    CHAR_UPD_HOUSING_MEMBER_STATUS,
    CHAR_DEL_HOUSING_MEMBER,
```

---

## 4. CharacterDatabase.cpp
### Ajouter dans `DoPrepareStatements()`, à la fin :

```cpp
    // Housing System — House record
    PrepareStatement(CHAR_SEL_HOUSING_BY_GUID,
        "SELECT id, plot_id, house_type, size, status, name, description, is_public, allow_visits "
        "FROM character_housing WHERE owner_guid = ?",
        CONNECTION_SYNCH);

    PrepareStatement(CHAR_INS_HOUSING,
        "INSERT INTO character_housing "
        "(owner_guid, plot_id, house_type, size, status, name, description, is_public, allow_visits) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
        CONNECTION_SYNCH);

    PrepareStatement(CHAR_UPD_HOUSING,
        "UPDATE character_housing "
        "SET house_type = ?, size = ?, status = ?, name = ?, description = ?, "
        "    is_public = ?, allow_visits = ? "
        "WHERE id = ?",
        CONNECTION_ASYNC);

    PrepareStatement(CHAR_DEL_HOUSING,
        "DELETE FROM character_housing WHERE id = ?",
        CONNECTION_ASYNC);

    // Housing System — Decorations
    PrepareStatement(CHAR_SEL_HOUSING_DECORATIONS,
        "SELECT id, item_id, pos_x, pos_y, pos_z, rot_x, rot_y, rot_z, rot_w, "
        "       scale, type, is_locked "
        "FROM housing_decorations WHERE house_id = ?",
        CONNECTION_SYNCH);

    PrepareStatement(CHAR_INS_HOUSING_DECORATION,
        "INSERT INTO housing_decorations "
        "(house_id, item_id, pos_x, pos_y, pos_z, rot_x, rot_y, rot_z, rot_w, scale, type, is_locked) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
        CONNECTION_ASYNC);

    PrepareStatement(CHAR_UPD_HOUSING_DECORATION,
        "UPDATE housing_decorations "
        "SET pos_x = ?, pos_y = ?, pos_z = ?, rot_x = ?, rot_y = ?, rot_z = ?, rot_w = ?, "
        "    scale = ?, is_locked = ? "
        "WHERE id = ?",
        CONNECTION_ASYNC);

    PrepareStatement(CHAR_DEL_HOUSING_DECORATION,
        "DELETE FROM housing_decorations WHERE id = ?",
        CONNECTION_ASYNC);

    // Housing System — Fixtures
    PrepareStatement(CHAR_SEL_HOUSING_FIXTURES,
        "SELECT id, type, fixture_id, slot "
        "FROM housing_fixtures WHERE house_id = ?",
        CONNECTION_SYNCH);

    PrepareStatement(CHAR_INS_HOUSING_FIXTURE,
        "INSERT INTO housing_fixtures (house_id, type, fixture_id, slot) VALUES (?, ?, ?, ?)",
        CONNECTION_ASYNC);

    PrepareStatement(CHAR_UPD_HOUSING_FIXTURE,
        "UPDATE housing_fixtures SET fixture_id = ?, slot = ? WHERE id = ?",
        CONNECTION_ASYNC);

    PrepareStatement(CHAR_DEL_HOUSING_FIXTURE,
        "DELETE FROM housing_fixtures WHERE id = ?",
        CONNECTION_ASYNC);

    // Housing System — Rooms
    PrepareStatement(CHAR_SEL_HOUSING_ROOMS,
        "SELECT id, room_index, pos_x, pos_y, pos_z, orientation, size_x, size_y, "
        "       ceiling_type, door_type, component_theme "
        "FROM housing_rooms WHERE house_id = ?",
        CONNECTION_SYNCH);

    PrepareStatement(CHAR_INS_HOUSING_ROOM,
        "INSERT INTO housing_rooms "
        "(house_id, room_index, pos_x, pos_y, pos_z, orientation, size_x, size_y, "
        " ceiling_type, door_type, component_theme) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
        CONNECTION_ASYNC);

    PrepareStatement(CHAR_UPD_HOUSING_ROOM,
        "UPDATE housing_rooms "
        "SET pos_x = ?, pos_y = ?, pos_z = ?, orientation = ?, size_x = ?, size_y = ?, "
        "    ceiling_type = ?, door_type = ?, component_theme = ? "
        "WHERE id = ?",
        CONNECTION_ASYNC);

    PrepareStatement(CHAR_DEL_HOUSING_ROOM,
        "DELETE FROM housing_rooms WHERE id = ?",
        CONNECTION_ASYNC);

    // Housing System — Neighborhood members
    PrepareStatement(CHAR_SEL_HOUSING_MEMBERS,
        "SELECT player_guid, invited_by_guid, status, is_secondary_owner "
        "FROM housing_neighborhood_members WHERE neighborhood_id = ?",
        CONNECTION_SYNCH);

    PrepareStatement(CHAR_INS_HOUSING_MEMBER,
        "INSERT INTO housing_neighborhood_members "
        "(neighborhood_id, player_guid, invited_by_guid, status, is_secondary_owner) "
        "VALUES (?, ?, ?, ?, ?)",
        CONNECTION_ASYNC);

    PrepareStatement(CHAR_UPD_HOUSING_MEMBER_STATUS,
        "UPDATE housing_neighborhood_members SET status = ? "
        "WHERE neighborhood_id = ? AND player_guid = ?",
        CONNECTION_ASYNC);

    PrepareStatement(CHAR_DEL_HOUSING_MEMBER,
        "DELETE FROM housing_neighborhood_members "
        "WHERE neighborhood_id = ? AND player_guid = ?",
        CONNECTION_ASYNC);
```

---

## 5. Opcodes.cpp — Fix doublon CMSG_HOUSING_SVCS_RELINQUISH_HOUSE

### Chercher et supprimer la ligne dupliquée (garder une seule occurrence) :

```
DEFINE_HANDLER(CMSG_HOUSING_SVCS_RELINQUISH_HOUSE, STATUS_UNHANDLED, PROCESS_THREADUNSAFE, &WorldSession::Handle_NULL);
```

Il y en a **2 occurrences** dans le fichier. Supprimer la seconde.

---

## 6. CMakeLists.txt — Ajouter les fichiers Housing

Dans `src/server/game/CMakeLists.txt`, dans la section des sources game, ajouter :

```cmake
# Housing System
Housing/HousingDefines.h
Housing/HousingMgr.h
Housing/HousingMgr.cpp
```

---

## 7. World.cpp — Appel LoadNeighborhoods au démarrage

Dans `World::SetInitialWorldSettings()`, après le chargement du `sRoleplay` :

```cpp
TC_LOG_INFO("server.loading", "Loading Housing Neighborhoods...");
sHousingMgr->LoadNeighborhoods();
```

---

## 8. Player.cpp — Chargement/Sauvegarde par joueur

### Dans `Player::LoadFromDB()` :
```cpp
sHousingMgr->LoadPlayerHouse(GetGUID());
```

### Dans `Player::SaveToDB()` ou le logout handler :
```cpp
sHousingMgr->SavePlayerHouse(GetGUID());
sHousingMgr->UnloadPlayerHouse(GetGUID());
```

---

## 9. Includes requis dans HousingMgr.cpp

Vérifier que les headers suivants sont disponibles dans ton projet :

```cpp
#include "PhasingHandler.h"   // Pour AddPhase / RemovePhase
#include "Player.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Timer.h"            // Pour getMSTime() / GetMSTimeDiffToNow()
```
