# OpenWorld / Port Meridian

**Активная игра: `Native/`, Godot 4.5.1, Windows x64.**

После согласованного изменения масштаба проект развивается как небольшая
оригинальная стилизованная нативная игра, а не как обещание GTA/AAA.
Unreal-исходники первой попытки сохранены, но **не участвуют в новой сборке**.
[Старая UE-документация](docs/UNREAL_README.md).

## Игрокам

Откройте [Releases](https://github.com/serhii1staf/OpenWorld/releases).
Для нативной альфы используйте **v0.2.0-alpha**, не старый `v0.0.1-source`.

- Installer: **OpenWorld-Setup-0.2.0.exe**.
- Без установки: **OpenWorld-Windows-x64-0.2.0.zip** → распаковать → **OpenWorld.exe**.
- Не требуется отдельно устанавливать движок, редактор, компилятор или ресурсы.
- Windows 10/11 x64 и OpenGL 3.3 видеодрайвер. Бинарники пока без подписи;
  проверяйте SHA256 и источник загрузки. Не отключайте антивирус.

[Как играть / управление](Native/PLAY_RU.txt) · [Описание альфы](Native/RELEASE_NOTES.md)

## Игровой маршрут

Курьер приезжает в небольшой портовый город Меридиан. Три цепочки:
медицинская доставка, заказ автомастерской, восстановление лесного ретранслятора.
Девять этапов, золотые маркеры, диалоги, награды и свободное исследование после финала.

Есть пешее движение, автомобиль, жители и упрощённый трафик, смена дня/ночи и погоды,
карта, оружие, деньги/груз, сохранения и меню настроек. Задания не требуют стрельбы.
Карта, стилизованная геометрия, анимация и синтетические звуки созданы для проекта.

**Ограничения:** маленький район, не бесшовный континент; low-poly модели, не AAA-люди;
аркадная физика, простой AI; нет multiplayer, развитых интерьеров, полиции и production QA.
Это первая playable-альфа, не замена многолетней разработке большого open world.

## Сборка — уже без Unreal/Epic

Workflow **Native game - Windows EXE and installer**:

1. Скачивает официальный Godot 4.5.1 и export templates, проверяет SHA256.
2. Импортирует проект, запускает реальные engine gameplay tests.
3. Экспортирует Windows EXE с embedded PCK.
4. На Windows runner запускает экспортированный EXE с теми же тестами.
5. Собирает Inno Setup installer и проверяет silent install/uninstall.
6. Только после всех успешных шагов публикует prerelease (при `publish=true`).

Никакие Epic credentials или заранее установленный UE не нужны. CI сам получает
инструменты сборки. Игрок скачивает только итоговый installer/ZIP.

## Проверки

`Native/tests/smoke_test.gd`: 34 проверки, включая физическое перемещение,
вход/движение/выход из авто, магазин/reload, все mission interactions,
reward-once, save/load, fallback при повреждении файла, день/ночь/погоду и recovery.
Тесты выполняют игровые методы, но **не заменяют полный ручной playthrough**:
например, переходы между заданиями в тесте телепортируют персонажа.

Linux OpenGL/Mesa render проверяется отдельно. Windows CI — headless запуск
нативного EXE; проверкой GPU и звука на реальном игровом ПК он не является.
Показатели FPS на пользовательском hardware пока не подтверждены.

Для разработки установить Godot 4.5.1 и открыть `Native/project.godot`:

```sh
godot --headless --path Native --editor --import --quit
godot --headless --path Native --script res://tests/smoke_test.gd
godot --path Native
```

## Каталоги

- `Native/scripts` — игровой код, мир, модели, AI, UI, сохранения.
- `Native/assets` — оригинальные звуки/шейдер/icon; скрипт воспроизводит аудио.
- `Native/licenses` — MIT исходников, Godot и third-party notices.
- `Native/installer.iss` — native installer recipe.
- `.github/workflows/native-godot.yml` — активный release pipeline.
- `Source`, `Config`, старые `Scripts`/UE workflows — legacy UE foundation, не новая игра.

Не публикуйте secrets в коде. Переданный ранее в чат PAT следует отозвать после
работы. Для дальнейшего CI используется краткоживущий GitHub Actions token.
