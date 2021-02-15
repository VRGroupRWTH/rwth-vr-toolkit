# :card_index_dividers: nDisplayExtensions
The nDisplayExtensions are a collection of extensions of the VR-Group at the RWTH which are used in many of our applications.
The contents of this plugin are constantly extended and improved to reflect changes in the Unreal nDisplay Plugin that we use to support our aixCAVE with Unreal.

In the following the main features are explained:

## :open_file_folder:  Cluster
This folder contains cluster specific helpers or components

### :arrow_forward: :open_file_folder: Events
This folder contains some cluster-event helpers. *TODO*

## :open_file_folder: Fixes
This folder contains fixes for problems that exist in the Unreal Engine, which can be fixed at runtime. The description of every single fix can be found in the specific file.

## :open_file_folder:  Interaction
This folder contains interaction related components, which can be attached to actors in your application to reflect a specific feature. Some of these components require the corresponding component on your pawn.
*Todo*

## :open_file_folder: Pawn
This folder consists of our pawn implementation and some components that are attached to it.

### :diamond_shape_with_a_dot_inside: Virtual Reality Pawn
This simple pawn implementation only attaches the following components to itself and handles some input. All the attached components do not depend on this pawn and can thus be added to your pawn implementation as well.

### :diamond_shape_with_a_dot_inside: Universal Tracked Component
The universal tracked component can be added to your pawn and is configured via two properties:
* `ProxyType` states for which type of tracked device this component is meant. E.g. this component can behave like the users head, left or right hand.
* `AttachementType` states which controller is used as a tracked device in the aixCAVE. E.g. this component will use the flystick as a tracking source in the aixCAVE.

This component determines on startup if it is used in a desktop, VR or cluster application and handles the attachment of itself for the corresponding scenario:
* In desktop mode, all components of this type are attached to the pawn root, except for the head, which is attached to the pawn's camera.
* In VR mode, the head component is attached to the HMD. All other types spawn a `MotionControllerComponent` on your Pawn and attaches themself to this component. The spawned component is configured according to the `ProxyType` above.
* In cluster mode, these components attach themself to the tracked nDisplay components as specified by `ProxyType` and `AttachementType`

The usage of this component is simple: Attach it to your pawn, set the `ProxyType` and `AttachementType` to your needs and you can attach everything to these components are needed. The components handle the tracking for you.

### :diamond_shape_with_a_dot_inside: VR Pawn Movement
This component adds a configurable movement to your pawn implementation. It can be configured via the `NavigationMode`, which behaves as follows:
* `NAV_NONE` No input is handled
* `NAV_GHOST` Simple flying movement, but no collision with walls
* `NAV_FLY` Flying movement with collision checking
* `NAV_WALK` Walking movement that simulates gravity and collisions for the pawn

To use this component attach it to your pawn and make sure to call
```cpp
PawnMovement->SetUpdatedComponent(RootComponent);
PawnMovement->SetCameraComponent(CameraComponent);
```