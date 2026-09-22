# Контентная интеграция в UE 5.6

**В репозитории нет .umap/.uasset и нет внешних 3D/аудио/анимационных ассетов.** Не добавлены поддельные бинарники или переименованные JSON. Не используются ассеты GTA/RDR.

MIT относится только к оригинальному коду. Лицензия UE и лицензия каждого импортируемого ассета действуют отдельно. Бесплатное скачивание не гарантирует право публично размещать исходный ассет в GitHub. При лицензии только на cooked redistribution хранить исходный контент в закрытом asset depot; в публичном репозитории — manifest и разрешённые исходники. Epic/Fab assets не считаются автоматически CC0.

## После первого C++ build

1. Открыть `OpenWorld.uproject` в UE 5.6. Создать авторскую карту через **File → New Level → Open World**, сохранить `/Game/World/Maps/OpenWorld`. Не выдавать этот engine template за готовый мир.
2. В Project Settings → Maps & Modes назначить эту карту Editor/Game default и созданный `BP_OpenWorldGameMode` (родитель `OWGameMode`). Настройки сохранить в Git.
3. В Tools → Execute Python Script выполнить `Scripts/CreateExampleData.py`. Скрипт создаёт три mission definitions и три weapon definitions; он **не** создаёт playable missions или арт.
4. Создать `BP_PlayerState : OWPlayerState`; в Missions.Definitions добавить три DA. В Inventory.InitialItems — `Ammo.9mm: 120`, `Ammo.556: 90`; InitialMoney = 200.
5. Создать `BP_Player : OWCharacter`. Назначить законно приобретённую/созданную skeletal model и AnimBP. Настроить Capsule, mesh root/offset, animation speed/direction/crouch/death. Weapon.Definition = `DA_Pistol`. OnShot привязать к weapon montage, FX и звуку; local presentation не наносит урон.
6. В BP GameMode назначить BP_Player и BP_PlayerState. Не оставлять invisible native pawn в release.
7. Создать `BP_Car : OWVehiclePawn`, skeletal mesh с корректными wheel bones, PhysicsAsset, ChaosWheel subclasses и wheel setups, torque/transmission/steering curves, AnimBP для WheelController. Размещение машины — не доказательство корректной физики: пройти QA.
8. Создать BP_Citizen с оригинальным персонажем и AnimBP; назначить Home/Work world coordinates, проверить NavMesh. Массовое разнообразие NPC ещё не реализовано.

## Три примера заданий

Data assets создаются скриптом; события отправляют `OWInteractionActor` с настоящей геометрией и collision Visibility Block:

| Mission ID | Prerequisite | Objective events | Reward |
|---|---|---|---|
| FirstShift | — | Harbor.Arrived | 150 |
| MedicalRun | FirstShift | Clinic.PickedUp → Clinic.Delivered | 350 |
| WorkshopContract | MedicalRun | Workshop.PartCollected ×3 → Workshop.Returned | 600 |

Разместить разные interaction actors для старта миссии и каждой цели. MissionToStart — на стартовом объекте; MissionEvent — на целевом. Для доставки использовать GrantedItem/RequiredItem с одинаковым ID; для трёх деталей — три отдельных actors (одноразовое использование). У всех persistent actors свой SaveId. Его можно задать в Details; для новой копии обязательно новый GUID. Runtime duplicate-check не заменяет content validation.

Эти задания не имеют написанных диалогов, постановки, озвучивания или локаций — их нужно авторски реализовать. Нельзя объявлять наличие DA «тремя готовыми миссиями».

## Окружение

- Director: назначить movable Sun/Moon/Fog, sun atmosphere index 0, moon 1; экспозицию и Skylight/RealTimeCapture выбирать после GPU измерений.
- MPC `Rain`, `Wetness`, `Snow`, `Night` создать и назначить; подключить к реальным материалам. Scalar сам по себе не создаёт мокрые PBR поверхности, лужи или снег.
- Rain/Snow/Storm визуальные эффекты и sound mixes подключаются отдельно. Water plugin включён, но водоёмы и шейдеры не созданы.
- Каждый тип окружения — отдельные LOD/HLOD/collision/material budgets. Использовать instancing там, где это измеримо выгодно, а не повторять одну модель квартала.
- NavMesh под WP настроить и построить для выбранного streaming workflow. Код не генерирует навигацию всего континента.

## Трафик

Создать `OWTrafficLane` instances со spline по центрам полос. Задать SpeedLimitKPH, soft Next, Signal, StopLineDistance в сантиметрах вдоль spline, SignalGroupB. `OWTrafficController::SetLane` вызвать на authority после possession каждой AI-машины. При отсутствии загруженного Next машина тормозит, не вызывает синхронную загрузку.

Контроллер ещё не разрешает все конфликтные траектории перекрёстков; не использовать как production traffic safety solution. Не держать тысячи Chaos bodies в памяти: дальний representation layer — отдельная задача.

## Audio

У `OWAmbientZone` назначить looping SoundCue/MetaSound/USoundBase, attenuation/sound class при необходимости. Загрузка — soft async. Для 3D emitters настроить AudioComponent attenuation/occlusion и legal sounds в Editor. У текущего volume нет arbitration пересекающихся biome zones, road/wind/weather mix graph или готового звукового набора.

## Manifest и packaging gate

`ContentManifest.json` заполняется реальными файлами (пример структуры, НЕ существующий ассет):

```json
{
  "schema": 1,
  "status": "integration-ready",
  "assets": [
    {
      "role": "world-map",
      "path": "Content/World/Maps/OpenWorld.umap",
      "origin": "original in-house environment; record author/commit",
      "license": "owned; redistribution approved"
    }
  ]
}
```

Нужны все обязательные roles: world-map, player-character, player-animation, vehicle-mesh, vehicle-physics, npc-character, ambient-audio. Пример выше намеренно неполный. Для каждого стороннего исходника хранить URL/дату/версию лицензии и условия redistribution. Нельзя ставить `integration-ready`, просто чтобы обойти отсутствие контента.

Проверка manifest выявляет отсутствующие файлы/LFS pointers/происхождение, но **не** доказывает права, качество арта или валидность gameplay. UE DataValidation и ручная проверка обязательны.
