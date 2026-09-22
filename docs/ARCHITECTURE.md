# Архитектура

## Текущая граница

Один UE Runtime module `OpenWorld`, разделённый на независимые папки систем. Это **не** набор уже выделенных binary modules. В раннем исходном каркасе такое решение упрощает UHT и позволяет обнаружить реальные зависимости до выделения модулей. Editor automation находится в `Scripts`, а не в runtime.

```
GameMode -> выбирает Pawn / PlayerState / Controller / GameState
PlayerController -> пользовательские запросы, pause/save/load
Character -> authoritative input requests -> Weapon / Interaction
PlayerState -> Inventory + Missions (переживают смену pawn)
GameState -> реплицируемое время / погода
EnvironmentDirector -> только визуализация состояния
WorldStateSubsystem -> GUID records + weak registry загруженных actors
SaveSubsystem -> snapshot -> async double-slot I/O -> validate -> apply
PopulationSubsystem -> budgeted batches -> Citizen decisions
TrafficController -> soft road references -> Chaos inputs
```

## Правила authority

- Урон, списание патронов, экономика, mission events и interaction transactions выполняются на authority.
- Клиент передаёт намерение, а не цену, награду, попадание или произвольное имя события задания.
- Interaction повторно трассируется на сервере от головы pawn, с лимитом расстояния и частоты.
- События задания нельзя отправить через клиентский RPC. Definitions считаются доверенным игровым контентом.
- Визуальные эффекты стрельбы вызываются multicast unreliable; наносимый урон от визуализации не зависит.
- RPC ownership не равен полной защите. До сети нужны limits per connection, sprint prediction, lag compensation, vehicle input validation, relevance/dormancy policies и adversarial tests.
- Persistent actors используют GUID, не указатель и не имя actor. GUID задаётся и хранится на instance в карте; runtime duplicate GUID отвергается.

## World Partition и загрузка

Проект не создаёт геометрию всей карты при BeginPlay. Нет world-wide GetAllActorsOfClass в игровых циклах и синхронной подгрузки соседних дорог. Тяжёлый арт должен быть размещён в авторской WP-карте с OFPA/Data Layers. Наличие engine renderer settings не означает, что HLOD/Nanite/streaming контент уже построен.

Предлагаемая стартовая карта для **следующего** этапа: 2×2 км, порт + жилой квартал + мастерская + лесная окраина; игровой маршрут 10–15 минут. Это предложение для контентной команды, а не утверждение о существующем мире. Большую карту расширять после измерений и стабильного gameplay loop.

- World: World Partition / External Actors / grid initial cell 128–256 м, подбирается профилированием.
- HLOD: отдельные layers для городской оболочки, vegetation и дальних силуэтов. Nanite не заменяет streaming и не убирает стоимость материалов, overdraw, collision.
- Interiors: Data Layers/Level Instances, portals/occlusion, visibility bounds; важные NPC/двери не держат hard-ссылки на удалённые interiors.
- Global actors (директор, sun/moon/fog) назначать non-spatial; не связывать hard reference с районными actors.
- Vehicle persistence: нужен отдельный runtime ownership/spawn coordinator перед дальними поездками через выгрузку клеток. Нельзя пометить весь городской автопарк always-loaded и назвать это масштабируемым.
- Saved records выгруженных actors остаются в subsystem. Register применяет запись при повторной загрузке. Restore откатывает загруженные actors, не загружая весь мир.
- Следующий шаг Save v2: staging streaming source на destination, timeout/cancel, проверка collision readiness, then transactional teleport; current load далеко намеренно отклоняется.

## Работа без постоянного Tick

- Inventory, Health, Missions, Interaction не имеют Tick.
- Clock: 2 Гц; environment presentation: 5 Гц.
- Population: 5 Гц, до 16 записей за вызов; решения каждого awake NPC не чаще чем раз в 3 секунды.
- Traffic: 10 Гц на активную машину. Это начальная реализация: перед массовым трафиком перенести обслуживание в общий budget scheduler и дешёвые representations вдали.
- Ballistics: максимум 512 записей, заранее зарезервированная память; не отдельный Actor/PhysicsBody на пулю. 120 Гц integration, не более 8 catch-up steps.
- HUD interaction query: 10 Гц. Рендер HUD остаётся frame-based.
- AmbientSound загружается асинхронно; fade и lifecycle делегированы AudioComponent.

## Масштабирование: следующие инженерные задачи

1. Разделить Core contracts, Gameplay, Presentation и Editor modules после build validation.
2. Перейти на Enhanced Input assets + saved mappings; сохранить server intent API.
3. Mass citizens/traffic records вдали, actor representations вблизи, hysteresis и bounded activation queues.
4. FastArray inventory/mission deltas, replication graph/Iris budgets, server interest sets.
5. Asset Manager primary labels / cook chunks / async equipped-asset bundles; не hard-reference весь каталог вещей.
6. Missions: стабильные objective IDs вместо индексов для миграций, диалоги и conditions, transaction journal.
7. Save schema migration, checksums, byte limits до десериализации, runtime spawn registry, crash-injection tests.
8. Collision tiers, physics sleeping, pool VFX/audio emitters, material permutation budget.

Это расширения, а не уже реализованные свойства.
