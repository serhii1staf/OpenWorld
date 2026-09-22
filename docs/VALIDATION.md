# Проверки исходного релиза

Дата: 2026-09-22. Среда: Linux sandbox, без UE / Windows SDK.

| Проверка | Результат |
|---|---|
| `python Scripts/validate_source.py` | PASS, 0 ошибок структуры |
| `python -m unittest discover -s Tests -v` | PASS, 10/10 |
| `python -m py_compile Scripts/*.py Tests/*.py` | PASS (без импорта `unreal`) |
| YAML parser, оба workflows | PASS, синтаксис; не выполнение jobs |
| `git diff --check` | PASS |
| Unreal Header Tool / C++ compile / link | NOT RUN |
| UE native automation (4 cases) | NOT RUN |
| UE Python example data execution | NOT RUN |
| PowerShell build/content/package scripts | NOT RUN |
| Cook / Windows packaging | NOT RUN; контент отсутствует |
| Installer build / install / uninstall | NOT RUN |
| Gameplay / streaming / Save crash recovery | NOT RUN |
| Multiplayer / dedicated server | NOT RUN |
| CPU/GPU/VRAM/FPS | NOT MEASURED |

Не следует интерпретировать статические проверки как подтверждение работоспособности игры. При первом UE build необходимо сохранить полный UBT/UHT log, устранить выявленные ошибки и обновить этот отчёт с точным engine patch, toolchain и commit hash.
