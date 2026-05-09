# BakeCheckpointData.py
# Scans the CURRENTLY OPEN level for ACheckpoint actors and bakes their
# sorted CheckpointIDs into the matching UNovaLevelData asset.
#
# Usage:
#   1. Open the level you want to bake (e.g. MAP_World1)
#   2. In the UE Python console: py BakeCheckpointData.py
#   3. Repeat for each level
#
# The script determines which UNovaLevelData to write to by matching the
# current level's package path against BP_GameInstance's LevelMap entries.

import unreal

GAME_INSTANCE_PATH = "/Game/Dynamic/Blueprints/BP_GameInstance.BP_GameInstance_C"


def run():
    # Get current editor world
    editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    world = editor_subsystem.get_editor_world()
    if world is None:
        unreal.log_error("BakeCheckpointData: No editor world found.")
        return

    current_package = world.get_path_name().rsplit('.', 1)[0]
    unreal.log(f"BakeCheckpointData: Current level: {current_package}")

    # Load BP_GameInstance CDO to find the matching LevelData asset
    bp_class = unreal.load_class(None, GAME_INSTANCE_PATH)
    if bp_class is None:
        unreal.log_error(f"BakeCheckpointData: Could not load class {GAME_INSTANCE_PATH}")
        return

    cdo = unreal.get_default_object(bp_class)
    level_map = cdo.get_editor_property("LevelMap")
    if level_map is None:
        unreal.log_error("BakeCheckpointData: LevelMap is None on the CDO.")
        return

    # Find the entry whose MapAsset matches the current level
    level_data = None
    matched_key = None
    for key, entry in level_map.items():
        map_asset = entry.get_editor_property("MapAsset")
        if map_asset is None:
            continue
        entry_package = map_asset.get_path_name().rsplit('.', 1)[0]
        if entry_package == current_package:
            level_data = entry.get_editor_property("LevelData")
            matched_key = str(key)
            break

    if level_data is None:
        unreal.log_error(
            f"BakeCheckpointData: No LevelMap entry with a LevelData asset matches the current level '{current_package}'.\n"
            f"Make sure BP_GameInstance has a LevelMap entry with this level's MapAsset AND a LevelData asset assigned."
        )
        return

    unreal.log(f"BakeCheckpointData: Matched ENovaLevel.{matched_key} -> {level_data.get_path_name()}")

    # Scan for checkpoint actors
    checkpoint_class = unreal.load_class(None, "/Script/CoopPlatformer.Checkpoint")
    if checkpoint_class is None:
        unreal.log_error("BakeCheckpointData: Could not load ACheckpoint class.")
        return

    actors = unreal.GameplayStatics.get_all_actors_of_class(world, checkpoint_class)

    checkpoint_ids = []
    for actor in actors:
        try:
            cid = actor.get_editor_property("CheckpointID")
            checkpoint_ids.append(int(cid))
        except Exception as e:
            unreal.log_warning(f"BakeCheckpointData: Could not read CheckpointID from {actor.get_actor_label()}: {e}")

    checkpoint_ids.sort()
    unreal.log(f"BakeCheckpointData: Found {len(checkpoint_ids)} checkpoints: {checkpoint_ids}")

    # Write to the data asset
    level_data.set_editor_property("CheckpointIDs", checkpoint_ids)
    asset_subsystem = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    asset_subsystem.save_loaded_asset(level_data)

    unreal.log(f"BakeCheckpointData: Successfully saved {level_data.get_path_name()} with {len(checkpoint_ids)} checkpoint IDs.")


run()
