# Этапы до настоящего vertical slice

## 0. Исходная основа — текущий релиз

UE source, authority contracts, ограниченные системы, инструкции, native build/test pipeline. Нет верификации UE build и контентного мира.

## 1. Подтверждённый engine build

Выделить Windows runner, исправить выявленные UHT/API ошибки, запустить native automation; закрепить точный patch UE 5.6 и toolchain. Пока этот этап не пройден, любые слова «рабочая система» означали бы недоказанное обещание.

## 2. Авторский игровой маршрут

Небольшая часть большого мира (порт/мастерская/жилой квартал/лес), законные human/vehicle/environment assets, уникальные props, collision/nav, полноценный AnimBP. Три настоящие миссии поверх examples, постановка взаимодействий, базовые диалоги, аудио и FX. Это контентная разработка, а не генерация коробок.

## 3. Межсистемная надёжность

Vehicle streaming ownership, distant save/load coordinator, checkpoint/respawn, saved settings, item/equipment UI, complete weapon presentation, safe physics, time/weather sound/material integration. Расширить автоматизацию: lifecycle, save migration, performance regressions.

## 4. Измеренный Windows vertical slice

Development→Shipping, packaged smoke test, installer, подписанные/проверенные artifacts, ручной QA. Только после этого тег playable alpha и downloadable Windows release.

## 5. Большой мир

Расширять районы и interior library; HLOD builds, budgets, asset review; Mass/virtual populations и traffic, replicated representation tiers; новые задания/диалоги/кат-сцены. Большая площадь без плотности gameplay не является достигнутым масштабом AAA.

## 6. Multiplayer

Dedicated server на лицензированной source build UE, listen/dedicated integration tests, input prediction/validation, ownership/relevancy, persistence per identity, server versioning, reconnect, load/security testing. Существующие authority boundaries полезны, но не устраняют этот объём работ.

Полный AAA open-world требует длительной работы команды и значительного объёма авторского контента. Этот репозиторий не объявляет такую работу выполненной.
