# AssignCheckpointIDs.py
# Finds all ACheckpoint actors in the current level, sorts them by X position
# (left to right), and assigns sequential CheckpointID values starting from 1.
#
# Usage:
#   1. Open the level (e.g. MAP_World1)
#   2. In the UE Python console: py AssignCheckpointIDs.py

import unreal

CHECKPOINT_CLASS_PATH = "/Script/CoopPlatformer.Checkpoint"


def run():
    editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    world = editor_subsystem.get_editor_world()
    if world is None:
        print("ERROR: No editor world found.")
        return

    current_level = world.get_path_name().rsplit('.', 1)[0]
    print(f"AssignCheckpointIDs: Current level: {current_level}")

    checkpoint_class = unreal.load_class(None, CHECKPOINT_CLASS_PATH)
    if checkpoint_class is None:
        print(f"ERROR: Could not load {CHECKPOINT_CLASS_PATH}")
        return

    actors = unreal.GameplayStatics.get_all_actors_of_class(world, checkpoint_class)
    if not actors:
        print("No Checkpoint actors found in this level.")
        return

    # Sort by X position (left to right)
    sorted_actors = sorted(actors, key=lambda a: a.get_actor_location().x)

    for i, actor in enumerate(sorted_actors, start=1):
        actor.set_editor_property("CheckpointID", i)
        loc = actor.get_actor_location()
        label = actor.get_actor_label()
        print(f"  [{i}] {label} @ X={loc.x:.1f}")

    print(f"Assigned IDs 1-{len(sorted_actors)} to {len(sorted_actors)} checkpoints (sorted left-to-right).")


run()
