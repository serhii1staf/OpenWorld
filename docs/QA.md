# Проверка сборки, gameplay и производительности

## Gate 0 — текущая среда без Unreal

```sh
python Scripts/validate_source.py
python -m unittest discover -s Tests -v
```

Проверяется только структура и согласованность исходного репозитория. Не измеряются FPS, физика, компиляция UE или качество изображения.

## Gate 1 — Windows / Unreal 5.6

1. Чистый checkout + Git LFS pull.
2. `Scripts/Build.ps1 -Mode Editor`: UBT/UHT/compile/link без ошибок.
3. `Scripts/Test.ps1`: 4 native automation cases; обязательно сохранить `Artifacts/Tests/index.json`, не считать exit code единственным доказательством.
4. Native cases: ограничения/транзакции Inventory, порядок целей и однократная награда Missions, GUID validation Persistence.
5. В Editor выполнить content integration и DataValidation. Packaging разрешать только при реальном manifest.
6. `Scripts/Build.ps1 -Mode Package -Configuration Development`, затем Shipping.

**Эти шаги не были выполнены в исходной sandbox-среде.**

## Gate 2 — ручной smoke test на контентной карте

- Новая игра: корректный PlayerStart, visible character, камера не внутри геометрии.
- 10 минут walk/run/crouch/jump: склоны, ступени, двери, углы и низкие потолки.
- Interaction: нельзя активировать через стену/с дальней дистанции; быстрые повторы не дублируют деньги/предметы.
- Оружие: fire cadence, empty mag, reload cancel, отсутствие расходования чужих патронов, hit за препятствиями, смерть прекращает огонь.
- Missions: выполнить три авторски размещённых цепочки; prerequisite/branch exclusion, repeated event, reward once после save/load.
- NPC: day/night home-work, навигация при streaming, реакция на gunshot, отсутствие накопления MoveTo requests вдали.
- Vehicle: neutral/brake/reverse, пандусы, collision, blocked door exit, slope exit; mesh/wheel axes, COM и chassis clearance.
- Traffic: красный/переходный сигнал, затор, светофор выгружен, следующий lane выгружен, препятствие и остановка. Пока нет гарантии отсутствия взаимных deadlock.
- Weather: целый игровой день; параметры light/fog/MPC действительно подключены; нет NaN и скачков экспозиции.
- Save: обе записи, более новая испорчена/удалена, предыдущая валидна; запрет nested save/load; player changes vehicle/map во время I/O; отсутствующее определение задания.
- Streaming: выгрузить и загрузить взаимодействованный объект — использованное состояние сохраняется. Дальний restore сейчас должен безопасно отказать, а не телепортировать под карту.
- Pause: перестать стрелять, не двигаться при наведении мыши на UI; сохранить настройки, перезапустить приложение.

## Gate 3 — измерения (цели, не результаты)

Первый профиль: Windows 11, 1080p output, фиксированные GPU/CPU/driver/build hash. Цель 60 FPS — бюджет 16.67 ms, но она пока не доказана.

| Метрика | Начальный плановый бюджет |
|---|---:|
| Game thread p95 | ≤ 5 ms |
| Render/RHI p95 | ≤ 5 ms |
| GPU p95 | ≤ 15 ms |
| Периодические активные NPC decisions | ≤ 16 записей / 0.2 s |
| Ballistic records | ≤ 512 |
| VRAM | запас не менее 20% на целевой карте |
| Hitch | отдельно фиксировать кадры > 33 / 50 / 100 ms |

CPU/GPU времена не складывать арифметически как последовательные: их overlap зависит от pipeline. Измерять p50/p95/p99 и latency, не только средний FPS.

Для Development использовать Unreal Insights (`-trace=cpu,gpu,frame,bookmark,loadtime,file,memory`), `stat unit`, `stat gpu`, `stat streaming`, `stat game`, `memreport -full`, GPU Visualizer. TRACE scopes: OW_Ballistics, OW_Population, OW_Traffic. Проверить поддержку выбранных channels установленным UE.

Маршруты: пешком в плотном квартале; езда через несколько WP cells; поворот камеры на 180°; ночь/дождь; интерьер→улица; перестрелка при traffic/NPC stress. Отдельно cold disk cache и warm cache. Не «оптимизировать» через исчезновение критических collisions, NPC или миссий.

## Gate 4 — реальный релиз

- ZIP запускается на чистой Windows-машине **без UE Editor**; runtime prerequisites устанавливаются.
- Shipping не зависит от файлов абсолютного пути разработчика, DerivedDataCache или Content исходного проекта.
- Installer install/uninstall/upgrade, Unicode пути, обычный пользователь, отсутствие write в Program Files.
- Сохранения остаются в Saved/SaveGames пользовательской игры, а не рядом с executable.
- Проверить license/redistribution approvals, SHA256, antivirus/signing policy.
- При подписи использовать защищённое хранилище сертификатов CI. Ключи/пароли никогда не коммитить.
- Опубликовать screenshot/video только реального запуска и отчёты профилирования. Source prerelease не выдавать за successful Gate 4.
