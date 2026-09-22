"""Execute inside the compiled UE 5.6 editor. Creates data, NOT a world or playable slice.
Tools > Execute Python Script > Scripts/CreateExampleData.py
Existing assets are retained; this script never overwrites authored content.
"""
import unreal

TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def data_asset(folder, name, cls):
    path = folder + '/' + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path), False
    unreal.EditorAssetLibrary.make_directory(folder)
    factory = unreal.DataAssetFactory()
    factory.set_editor_property('data_asset_class', cls)
    obj = TOOLS.create_asset(name, folder, cls, factory)
    if not obj:
        raise RuntimeError('Could not create ' + path)
    return obj, True


def mission(name, title, prerequisites, objectives, reward):
    asset, new = data_asset('/Game/Missions', 'DA_' + name, unreal.OWMissionDefinition)
    if not new:
        return asset
    asset.set_editor_property('mission_id', name)
    asset.set_editor_property('title', unreal.Text(title))
    asset.set_editor_property('prerequisites', prerequisites)
    entries = []
    for event, text, required in objectives:
        entry = unreal.OWObjective()
        entry.set_editor_property('event', event)
        entry.set_editor_property('description', unreal.Text(text))
        entry.set_editor_property('required', required)
        entries.append(entry)
    asset.set_editor_property('objectives', entries)
    asset.set_editor_property('reward', reward)
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return asset


mission('FirstShift', 'First shift', [], [
    ('Harbor.Arrived', 'Meet the harbor dispatcher', 1)
], 150)
mission('MedicalRun', 'Medical run', ['FirstShift'], [
    ('Clinic.PickedUp', 'Collect the medical parcel', 1),
    ('Clinic.Delivered', 'Deliver the parcel to the clinic', 1)
], 350)
mission('WorkshopContract', 'Workshop contract', ['MedicalRun'], [
    ('Workshop.PartCollected', 'Collect replacement parts', 3),
    ('Workshop.Returned', 'Return to the workshop', 1)
], 600)

for name, ammo, capacity, speed, damage, interval, automatic in [
    ('Pistol', 'Ammo.9mm', 15, 380.0, 25.0, 0.18, False),
    ('SMG', 'Ammo.9mm', 30, 420.0, 18.0, 0.085, True),
    ('Rifle', 'Ammo.556', 30, 900.0, 40.0, 0.1, True)
]:
    asset, new = data_asset('/Game/Weapons', 'DA_' + name, unreal.OWWeaponDefinition)
    if new:
        for key, value in dict(ammo_item=ammo, magazine_size=capacity, muzzle_velocity_mps=speed,
                               damage=damage, shot_interval=interval, b_automatic=automatic).items():
            # Python exposes the C++ bAutomatic property as 'automatic'.
            asset.set_editor_property('automatic' if key == 'b_automatic' else key, value)
        unreal.EditorAssetLibrary.save_loaded_asset(asset)

unreal.log('Example mission and weapon data created. No map, character, vehicle art or animations were generated.')
