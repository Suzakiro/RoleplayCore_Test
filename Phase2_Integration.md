# Housing Phase 2 — Guide d'intégration

## Fichiers livrés (REMPLACENT les versions existantes)
```
src/server/game/Server/Packets/HousingPackets.h   ← REMPLACE l'existant
src/server/game/Server/Packets/HousingPackets.cpp  ← REMPLACE l'existant
src/server/game/Handlers/HousingHandler.cpp        ← REMPLACE l'existant
```

---

## 1. Opcodes.cpp — Remplacer STATUS_UNHANDLED par les vrais handlers

### Rechercher et remplacer EXACTEMENT ces lignes :

```cpp
// AVANT
DEFINE_HANDLER(CMSG_HOUSING_HOUSE_STATUS,                               STATUS_UNHANDLED, PROCESS_THREADUNSAFE, &WorldSession::Handle_NULL);
DEFINE_HANDLER(CMSG_HOUSING_GET_CURRENT_HOUSE_INFO,                     STATUS_UNHANDLED, PROCESS_THREADUNSAFE, &WorldSession::Handle_NULL);
DEFINE_HANDLER(CMSG_HOUSING_GET_PLAYER_PERMISSIONS,                     STATUS_UNHANDLED, PROCESS_THREADUNSAFE, &WorldSession::Handle_NULL);
DEFINE_HANDLER(CMSG_HOUSING_SVCS_GET_PLAYER_HOUSES_INFO,                STATUS_UNHANDLED, PROCESS_THREADUNSAFE, &WorldSession::Handle_NULL);
DEFINE_HANDLER(CMSG_HOUSING_SVCS_TELEPORT_TO_PLOT,                      STATUS_UNHANDLED, PROCESS_THREADUNSAFE, &WorldSession::Handle_NULL);
DEFINE_HANDLER(CMSG_HOUSING_SVCS_NEIGHBORHOOD_RESERVE_PLOT,             STATUS_UNHANDLED, PROCESS_THREADUNSAFE, &WorldSession::Handle_NULL);
DEFINE_HANDLER(CMSG_HOUSING_SVCS_RELINQUISH_HOUSE,                      STATUS_UNHANDLED, PROCESS_THREADUNSAFE, &WorldSession::Handle_NULL);
DEFINE_HANDLER(CMSG_HOUSING_SVCS_UPDATE_HOUSE_SETTINGS,                 STATUS_UNHANDLED, PROCESS_THREADUNSAFE, &WorldSession::Handle_NULL);
DEFINE_HANDLER(CMSG_HOUSING_DECOR_SET_EDIT_MODE,                        STATUS_UNHANDLED, PROCESS_THREADUNSAFE, &WorldSession::Handle_NULL);
DEFINE_HANDLER(CMSG_HOUSING_FIXTURE_SET_EDIT_MODE,                      STATUS_UNHANDLED, PROCESS_THREADUNSAFE, &WorldSession::Handle_NULL);
DEFINE_HANDLER(CMSG_HOUSING_ROOM_SET_LAYOUT_EDIT_MODE,                  STATUS_UNHANDLED, PROCESS_THREADUNSAFE, &WorldSession::Handle_NULL);
```

```cpp
// APRÈS
DEFINE_HANDLER(CMSG_HOUSING_HOUSE_STATUS,                               STATUS_LOGGEDIN,  PROCESS_THREADUNSAFE, &WorldSession::HandleHousingHouseStatus);
DEFINE_HANDLER(CMSG_HOUSING_GET_CURRENT_HOUSE_INFO,                     STATUS_LOGGEDIN,  PROCESS_THREADUNSAFE, &WorldSession::HandleHousingGetCurrentHouseInfo);
DEFINE_HANDLER(CMSG_HOUSING_GET_PLAYER_PERMISSIONS,                     STATUS_LOGGEDIN,  PROCESS_THREADUNSAFE, &WorldSession::HandleHousingGetPlayerPermissions);
DEFINE_HANDLER(CMSG_HOUSING_SVCS_GET_PLAYER_HOUSES_INFO,                STATUS_LOGGEDIN,  PROCESS_THREADUNSAFE, &WorldSession::HandleHousingSvcsGetPlayerHousesInfo);
DEFINE_HANDLER(CMSG_HOUSING_SVCS_TELEPORT_TO_PLOT,                      STATUS_LOGGEDIN,  PROCESS_THREADUNSAFE, &WorldSession::HandleHousingSvcsTeleportToPlot);
DEFINE_HANDLER(CMSG_HOUSING_SVCS_NEIGHBORHOOD_RESERVE_PLOT,             STATUS_LOGGEDIN,  PROCESS_THREADUNSAFE, &WorldSession::HandleHousingSvcsNeighborhoodReservePlot);
DEFINE_HANDLER(CMSG_HOUSING_SVCS_RELINQUISH_HOUSE,                      STATUS_LOGGEDIN,  PROCESS_THREADUNSAFE, &WorldSession::HandleHousingSvcsRelinquishHouse);
DEFINE_HANDLER(CMSG_HOUSING_SVCS_UPDATE_HOUSE_SETTINGS,                 STATUS_LOGGEDIN,  PROCESS_THREADUNSAFE, &WorldSession::HandleHousingSvcsUpdateHouseSettings);
DEFINE_HANDLER(CMSG_HOUSING_DECOR_SET_EDIT_MODE,                        STATUS_LOGGEDIN,  PROCESS_INPLACE,      &WorldSession::HandleHousingDecorSetEditMode);
DEFINE_HANDLER(CMSG_HOUSING_FIXTURE_SET_EDIT_MODE,                      STATUS_LOGGEDIN,  PROCESS_INPLACE,      &WorldSession::HandleHousingFixtureSetEditMode);
DEFINE_HANDLER(CMSG_HOUSING_ROOM_SET_LAYOUT_EDIT_MODE,                  STATUS_LOGGEDIN,  PROCESS_INPLACE,      &WorldSession::HandleHousingRoomSetLayoutEditMode);
```

### FIX bug doublon — supprimer la 2ème occurrence de :
```cpp
DEFINE_HANDLER(CMSG_HOUSING_SVCS_RELINQUISH_HOUSE, STATUS_UNHANDLED, PROCESS_THREADUNSAFE, &WorldSession::Handle_NULL);
```
Il en reste UNE seule (celle déjà remplacée ci-dessus).

---

## 2. WorldSession.h — Ajouter les déclarations de handlers

### Dans le bloc `namespace Housing`, ajouter les forward declarations des nouveaux packets :
```cpp
namespace Housing
{
    class DeclineNeighborhoodInvites;
    // Ajouter :
    class HouseStatus;
    class GetCurrentHouseInfo;
    class GetPlayerPermissions;
    class SvcsGetPlayerHousesInfo;
    class SvcsTeleportToPlot;
    class SvcsNeighborhoodReservePlot;
    class SvcsRelinquishHouse;
    class SvcsUpdateHouseSettings;
    class DecorSetEditMode;
    class FixtureSetEditMode;
    class RoomSetLayoutEditMode;
}
```

### Juste après la ligne existante :
```cpp
void HandleDeclineNeighborhoodInvites(WorldPackets::Housing::DeclineNeighborhoodInvites const& declineNeighborhoodInvites);
```
### Ajouter :
```cpp
void HandleHousingHouseStatus(WorldPackets::Housing::HouseStatus const& packet);
void HandleHousingGetCurrentHouseInfo(WorldPackets::Housing::GetCurrentHouseInfo const& packet);
void HandleHousingGetPlayerPermissions(WorldPackets::Housing::GetPlayerPermissions const& packet);
void HandleHousingSvcsGetPlayerHousesInfo(WorldPackets::Housing::SvcsGetPlayerHousesInfo const& packet);
void HandleHousingSvcsTeleportToPlot(WorldPackets::Housing::SvcsTeleportToPlot const& packet);
void HandleHousingSvcsNeighborhoodReservePlot(WorldPackets::Housing::SvcsNeighborhoodReservePlot const& packet);
void HandleHousingSvcsRelinquishHouse(WorldPackets::Housing::SvcsRelinquishHouse const& packet);
void HandleHousingSvcsUpdateHouseSettings(WorldPackets::Housing::SvcsUpdateHouseSettings const& packet);
void HandleHousingDecorSetEditMode(WorldPackets::Housing::DecorSetEditMode const& packet);
void HandleHousingFixtureSetEditMode(WorldPackets::Housing::FixtureSetEditMode const& packet);
void HandleHousingRoomSetLayoutEditMode(WorldPackets::Housing::RoomSetLayoutEditMode const& packet);
```

---

## 3. HousingDefines.h — Ajouter HousingPermission (Phase 1 manquant)

### Dans l'enum section de HousingDefines.h (Phase 1), ajouter après `HousingError` :
```cpp
enum class HousingPermission : uint32
{
    None            = 0,
    Visit           = 1,
    Decorate        = 2,
    Owner           = 3,
    SecondaryOwner  = 4
};
```

---

## 4. Récapitulatif des opcodes reliés après Phase 2

| Opcode | Status | Handler |
|--------|--------|---------|
| `CMSG_DECLINE_NEIGHBORHOOD_INVITES`        | ✅ LOGGEDIN  | `HandleDeclineNeighborhoodInvites` |
| `CMSG_HOUSING_HOUSE_STATUS`                | ✅ LOGGEDIN  | `HandleHousingHouseStatus` |
| `CMSG_HOUSING_GET_CURRENT_HOUSE_INFO`      | ✅ LOGGEDIN  | `HandleHousingGetCurrentHouseInfo` |
| `CMSG_HOUSING_GET_PLAYER_PERMISSIONS`      | ✅ LOGGEDIN  | `HandleHousingGetPlayerPermissions` |
| `CMSG_HOUSING_SVCS_GET_PLAYER_HOUSES_INFO` | ✅ LOGGEDIN  | `HandleHousingSvcsGetPlayerHousesInfo` |
| `CMSG_HOUSING_SVCS_TELEPORT_TO_PLOT`       | ✅ LOGGEDIN  | `HandleHousingSvcsTeleportToPlot` |
| `CMSG_HOUSING_SVCS_NEIGHBORHOOD_RESERVE_PLOT` | ✅ LOGGEDIN | `HandleHousingSvcsNeighborhoodReservePlot` |
| `CMSG_HOUSING_SVCS_RELINQUISH_HOUSE`       | ✅ LOGGEDIN  | `HandleHousingSvcsRelinquishHouse` |
| `CMSG_HOUSING_SVCS_UPDATE_HOUSE_SETTINGS`  | ✅ LOGGEDIN  | `HandleHousingSvcsUpdateHouseSettings` |
| `CMSG_HOUSING_DECOR_SET_EDIT_MODE`         | ✅ LOGGEDIN  | `HandleHousingDecorSetEditMode` |
| `CMSG_HOUSING_FIXTURE_SET_EDIT_MODE`       | ✅ LOGGEDIN  | `HandleHousingFixtureSetEditMode` |
| `CMSG_HOUSING_ROOM_SET_LAYOUT_EDIT_MODE`   | ✅ LOGGEDIN  | `HandleHousingRoomSetLayoutEditMode` |
| Tous les autres CMSG housing               | ⏳ UNHANDLED | Phase 3+ |

---

## 5. Validation en jeu — Séquence de test

### Test 1 — Créer une maison
```
1. Connecter un personnage
2. Ouvrir l'UI Housing (bouton dans l'interface ou commande GM)
3. → Vérifie : CMSG_HOUSING_HOUSE_STATUS envoyé par le client
4. → Vérifie : SMSG_HOUSING_HOUSE_STATUS_RESPONSE reçu (HasHouse = false)
5. Cliquer sur un plot disponible
6. → Vérifie : CMSG_HOUSING_SVCS_NEIGHBORHOOD_RESERVE_PLOT envoyé
7. → Vérifie : SMSG_HOUSING_SVCS_NEIGHBORHOOD_RESERVE_PLOT_RESPONSE (Error = 0)
8. → Vérifie : SMSG_HOUSING_GET_CURRENT_HOUSE_INFO_RESPONSE envoyé automatiquement
9. Confirmer : table character_housing a une nouvelle ligne
10. Confirmer : housing_plots.status = 2 pour le plot choisi
```

### Test 2 — Téléporter vers sa maison
```
1. Player avec maison existante
2. Cliquer "Aller à ma maison" dans l'UI
3. → Vérifie : CMSG_HOUSING_SVCS_TELEPORT_TO_PLOT envoyé
4. → Vérifie : Player téléporté aux coordonnées du plot
5. → Vérifie : Phase du neighborhood appliquée via PhasingHandler
```

### Test 3 — Edit mode
```
1. Player dans sa maison
2. Activer mode décoration
3. → Vérifie : CMSG_HOUSING_DECOR_SET_EDIT_MODE (Enable=true)
4. → Vérifie : SMSG_HOUSING_DECOR_SET_EDIT_MODE_RESPONSE (Error=0, Enabled=true)
5. Désactiver
6. → Vérifie : Enabled=false dans la réponse
```

### Test 4 — Supprimer sa maison
```
1. Player avec maison
2. Cliquer "Abandonner ma maison"
3. → Vérifie : CMSG_HOUSING_SVCS_RELINQUISH_HOUSE envoyé
4. → Vérifie : SMSG_HOUSING_SVCS_RELINQUISH_HOUSE_RESPONSE (Error=0)
5. Confirmer : character_housing ligne supprimée (CASCADE)
6. Confirmer : housing_decorations, housing_fixtures, housing_rooms vides
7. Confirmer : housing_plots.status = 0 pour le plot libéré
```

---

## 6. Notes importantes sur les structures de packets

> **AVERTISSEMENT** : Les structures de packets housing pour WoW 12.x n'ont pas été
> confirmées par un packet sniffer. Les implémentations `Read()`/`Write()` sont
> construites selon les conventions TrinityCore standards et la logique sémantique
> des opcodes.
>
> Si le client se déconnecte au moment de la réception d'un SMSG ou que les données
> semblent décalées, ajuster les structures dans `HousingPackets.cpp` en fonction
> d'un packet capture réel.
>
> Outil recommandé pour capturer : **WoWDev PacketLogger** ou équivalent sur ton
> build client custom.
