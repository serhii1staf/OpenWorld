# Реальная попытка сборки на GitHub-hosted Windows

2026-09-22 по запросу владельца запущен не только статический CI, но и Windows workflow:

- Run: https://github.com/serhii1staf/OpenWorld/actions/runs/35777285735
- Job: https://github.com/serhii1staf/OpenWorld/actions/runs/35777285735/job/106913603861
- Commit: dc9c76f
- Workflow: `.github/workflows/windows-hosted.yml`
- Отчёт: [hosted-windows-35777285735.json](reports/hosted-windows-35777285735.json)

## Наблюдаемый результат

| Шаг | Результат |
|---|---|
| Получить обычную Windows x64 VM GitHub (`windows-2022`) | Успешно |
| Checkout проекта | Успешно |
| Проверить VS C++ / память / диски / стандартные установки UE и registry | Успешно |
| Вызвать `Scripts/Build.ps1 -Mode Editor` | Ошибка: отсутствует UE 5.6 / UE_ROOT |
| Unreal Header Tool / C++ compilation | Не начались |
| Native automation | Пропущена после ошибки build |
| Загрузить diagnostic artifact | Успешно |

На этой VM: Visual Studio 2022 Enterprise C++, 4 CPU, 16 GB RAM. UE 5.6 не обнаружен в стандартных местах/registry; UE_ROOT пуст. Свободное место отражено в JSON; это свойства данного запуска, не гарантия ресурсов будущих runners.

Ошибка: `Set UE_ROOT to the UE 5.6 installation directory (not its Engine subdirectory).`

GitHub API также показал: 0 подключённых self-hosted runners, 0 repository Actions secrets, 0 repository variables. Запрос EpicGames/UnrealEngine с предоставленным владельцем GitHub token вернул HTTP 404: доступ к исходникам движка этим токеном не подтверждён. Значения credentials в отчёты не записывались.

## Что это означает

GitHub Actions действительно подходит для нативных Windows builds. Полные права токена на OpenWorld позволяют управлять репозиторием/workflows, но не устанавливают Unreal и не предоставляют Epic entitlement. Рабочая папка `GITHUB_WORKSPACE` — каталог checkout, не поставка игрового движка. Codespaces — отдельная dev-среда, не автоматически готовый UE Windows builder.

Нельзя считать неудачу обнаружения UE ошибкой C++ проекта: его компилятор ещё не запускался. Нельзя считать успешное получение VM готовой сборкой игры.

## Вариант A — готовый UE на Windows runner

На доверенном Windows-ПК или Windows VPS:

1. Установить UE 5.6 и совместимый VS C++/Windows SDK под собственным Epic account.
2. В GitHub: Settings → Actions → Runners → New self-hosted runner → Windows x64. Выполнить команды регистрации на той машине, **не передавать registration token в чат**.
3. Добавить label `ue-5.6`. Установить Git LFS и PowerShell 7; для draft release нужен GitHub CLI.
4. Задать `UE_ROOT` на каталог установки, например `C:\Program Files\Epic Games\UE_5.6`, в окружении runner service и перезапустить service.
5. Создать защищённый environment `windows-release`, ограничить запуск main и доверенными изменениями.
6. Запустить Native Windows build вручную. Не запускать недоверенные pull requests на личной машине.

Весь оркестрирующий pipeline всё равно работает через GitHub Actions; компиляция выполняется на подключённой машине. Это не обязательно компьютер владельца — возможен разрешённый арендованный Windows runner. Никакие платные ресурсы в этой попытке не приобретались.

## Вариант B — установка движка на временную VM GitHub

Технически возможна, но текущий workflow движок не скачивает. Нужен законно доступный источник: собственный private installed-engine archive/cache или Epic-authorized source/distribution access, затем установка зависимостей и engine build при необходимости. Не использовать случайные публичные перепаковки.

Для Epic GitHub source access: https://www.unrealengine.com/en-US/ue-on-github — связать аккаунты и завершить процедуру Epic/GitHub. GitHub PAT для OpenWorld не заменяет эту процедуру. Секреты доставки хранить в защищённых GitHub environments/secrets; не присылать Epic password, cookies или токены в чат.

После доступа потребуется отдельно реализовать и проверить загрузку/распаковку или сборку движка, cache version/hash, место/память и timeout. Сборка полного UE из исходников значительно тяжелее сборки игрового модуля; успех на стандартном hosted runner не обещается заранее.

## Независимый блокер после engine build

В source release нет authored world/character/vehicle/audio content. Даже после успешного Editor build необходимо выполнить docs/CONTENT.md. Packaging gate и ручной gameplay QA остаются обязательными. Отсутствие движка и отсутствие игрового контента — разные задачи; решение первой не создаёт вторую автоматически.
