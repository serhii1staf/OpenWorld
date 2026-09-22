# OpenWorld
### Нативный проект Unreal Engine 5.6 · C++ · Windows PC

> **v0.0.1-source — исходная техническая основа, не готовая игра.**
> В этом релизе **нет `.exe`, installer, игровой карты или 3D/аудиоассетов**. UE-компиляция, запуск и производительность **не проверены**: исходники созданы в Linux-среде без Unreal Engine и Windows toolchain. Это не браузерный проект; но playable vertical slice также ещё не получен.

## Что находится в репозитории

- UE Game / Editor / Server targets, renderer settings DX12/SM6, Lumen, Nanite, VSM и TSR.
- C++ персонаж, камера, движение; authority-side взаимодействия, здоровье и повреждения.
- Инвентарь/деньги, mission definitions с prerequisites и исключающими ветками, однократные награды.
- Баллистика с гравитацией и swept collision, ограниченный пул записей, магазин и reload.
- Chaos Vehicle integration: управление, possession, проверки выхода; lane-following трафик и сигналы.
- NPC home/work и реакция на шум, централизованный бюджет обслуживания.
- Часы/расписание погоды и presentation director света/тумана/MPC.
- Асинхронный ambient audio loader; базовые нативные HUD и меню паузы/качества графики.
- World GUID records и async Save/Load с двумя слотами и ограниченным восстановлением.
- Build/package/test scripts, Windows Actions workflow и шаблон Inno Setup installer.

**Наличие исходника не означает, что соответствующая функция уже работает в собранной игре.** Полная [матрица реализации и ограничений](docs/STATUS.md) обязательна к прочтению.

## Что потребуется для первого настоящего запуска

1. Windows 10/11 x64, Unreal Engine **5.6** через Epic Games Launcher (или лицензированная source build).
2. Visual Studio 2022 с Game development with C++, рекомендованные установленным UE 5.6 MSVC и Windows SDK. Не подменять toolchain произвольной версией: UBT сообщает допустимые версии.
3. Git и Git LFS. Для CI: PowerShell 7 и, если нужен draft GitHub release, GitHub CLI. Для installer отдельно Inno Setup 6.3+.
4. Достаточное место под UE, компилятор, DerivedDataCache и cooked assets. Двухъядерная sandbox-среда с ~1 GB RAM для такого pipeline непригодна.
5. Авторские или законно лицензированные игровой мир, персонажи, машины, анимации, текстуры и звуки. Внешний контент здесь не был скачан и не объявляется лицензированным заочно.

```powershell
git clone https://github.com/serhii1staf/OpenWorld.git
cd OpenWorld
git lfs install
git lfs pull
$env:UE_ROOT = 'C:\Program Files\Epic Games\UE_5.6'

# Первый UHT/C++ build. Может выявить ещё не обнаруженные ошибки UE API.
.\Scripts\Build.ps1 -Mode Editor

# 4 native automation cases, требуют успешного Editor build.
.\Scripts\Test.ps1

# Открыть редактор
& "$env:UE_ROOT\Engine\Binaries\Win64\UnrealEditor.exe" "$PWD\OpenWorld.uproject"
```

Для IDE: Generate Visual Studio project files из контекстного меню `.uproject` после установки shell integration UE; `.sln` не хранится в Git.

Затем выполнить [контентную интеграцию](docs/CONTENT.md). Нет обещания, что открытие `.uproject` без этих шагов запустит готовую игру. Отсутствие карты намеренное: пустой template или сцена из кубов не выдаются за выполненную задачу.

## Development / Shipping

После реального контента, valid manifest и QA:

```powershell
.\Scripts\Build.ps1 -Mode Package -Configuration Development
.\Scripts\Build.ps1 -Mode Package -Configuration Shipping
```

Скрипт запускает UBT, UE DataValidation и UAT BuildCookRun. Он **откажется** паковать релиз при `ContentManifest.status = content-required`. Это защита от публикации заведомо неполного проекта, а не уже существующая игровая сборка.

Ожидаемые выходы успешного packaging:

```
Artifacts/Shipping/Windows/OpenWorld.exe
Artifacts/OpenWorld-Win64-Shipping.zip
Artifacts/OpenWorld-Win64-Shipping.zip.sha256
```

Нужен весь packaged directory, не один `.exe`. На машине игрока Unreal Editor не требуется, но понадобятся runtime prerequisites. Installer после packaging:

```powershell
& 'C:\Program Files (x86)\Inno Setup 6\ISCC.exe' `
  "/DBuildRoot=$PWD\Artifacts\Shipping\Windows" `
  '/DAppVersion=0.1.0' '.\Installer\OpenWorld.iss'
```

Ни packaging, ни installer в текущей среде не выполнялись. Неподписанный development installer может вызвать предупреждение SmartScreen.

## GitHub и автоматизация

- `Source integrity (not Unreal compilation)` — Ubuntu job, статическая структура и 10 Python-тестов. **Зелёный статус этого job не доказывает компиляцию Unreal.**
- `Native Windows build` — ручной workflow, только main, требует доверенный self-hosted Windows x64 runner с labels `ue-5.6`, машинный `UE_ROOT` и environment `windows-release`.
- В Settings → Environments защитить `windows-release` ручным approval. Никогда не выполнять untrusted PR на runner с личными файлами/секретами. В публичном репозитории runners предпочтительно изолировать и пересоздавать после job.
- Workflow проверяет Editor build и native tests, затем packaging; по запросу создаёт **draft prerelease** с реальным ZIP. Перед публикацией draft пройти [QA](docs/QA.md).
- Workflow не устанавливает UE автоматически и не получает доступ к Epic account. GitHub-hosted Windows runners не имеют готового UE 5.6.
- Server target предусмотрен как точка развития; dedicated server не собран и не протестирован, обычно требует source build UE.

Токены не нужны в коде, README или URL remote. Для собственных push использовать `gh auth login` либо SSH/системный credential manager. Если PAT был передан в чат, отозвать его после завершения работы.

## Управление после интеграции контента

| Действие | Ввод |
|---|---|
| Движение / автомобиль | WASD |
| Камера | Мышь |
| Прыжок / ручник | Space |
| Бег / приседание | Shift / Ctrl |
| Взаимодействие / вход-выход из авто | E |
| Огонь / reload | ЛКМ / R |
| Пауза / графические пресеты | Esc |
| Сохранить / загрузить | F5 / F9 |

Native legacy Input mappings; Enhanced Input rebinding ещё не внедрён. Загрузка сейчас только в той же карте, на ногах, в пределах 100 м от сохранённой позиции и при готовой collision. Эти ограничения нельзя считать полноценной open-world SaveSystem.

## Структура

```text
Source/OpenWorld/    Core, Character, Interaction, Inventory, Missions,
                    Weapons, Vehicles, AI, Environment, Audio, SaveGame, UI, Tests
Config/             renderer, packaging, input
Scripts/            build, tests, content validation, example data
Installer/          Inno Setup recipe — не собранный установщик
Content/            пока без бинарных игровых ассетов
ContentManifest.json  происхождение/лицензии будущего контента и packaging gate
.github/workflows/  source checks и ручной UE Windows pipeline
docs/               состояние, архитектура, контент, QA и roadmap
```

Системы пока в одном Runtime module с разделением по папкам. Git LFS настроен для будущих бинарных ассетов. Секреты, generated files и packaged artifacts не включаются в Git.

## Проверки без Unreal

```sh
python Scripts/validate_source.py
python -m unittest discover -s Tests -v
```

Это не mock-доказательство gameplay: native tests отдельно находятся в `Source/OpenWorld/Tests`, их выполнение возможно только в Unreal.

[Архитектура](docs/ARCHITECTURE.md) · [Состояние](docs/STATUS.md) · [Контент](docs/CONTENT.md) · [QA/профилирование](docs/QA.md) · [Дальнейшие этапы](docs/ROADMAP.md)

## Лицензия

Оригинальный код — MIT. Unreal Engine, сторонние plugins и будущий импортируемый контент — по собственным лицензиям. Нет использования защищённых ассетов GTA/Red Dead Redemption. AAA-масштаб и качество остаются долгосрочной целью, а не свойствами этого исходного релиза.
