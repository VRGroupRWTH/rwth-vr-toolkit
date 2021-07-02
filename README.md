# :card_index_dividers: RWTH VR Toolkit
The **RWTH VR Toolkit** are a collection of extensions of the VR-Group at the RWTH which are used in many of our applications.
The contents of this plugin are constantly extended and improved to reflect changes in the Unreal nDisplay Plugin that we use to support our aixCAVE with Unreal.

In the following the main features are explained:

## :open_file_folder:  Cluster
This folder contains cluster specific helpers or components

### :open_file_folder: Events
This folder contains a wrapper around the Cluster Events that enables calling a method from an object on all nodes in the cluster by exposing a `DECLARE_DISPLAY_CLUSTER_EVENT` macro. This can be used to react to events that only occur on one node, e.g., input events on the master node. An example and more explanation on how to use the wrapper can be found [here](https://devhub.vr.rwth-aachen.de/VR-Group/unreal-development/unrealprojecttemplate/-/snippets/44).

## :open_file_folder: Fixes
This folder contains fixes for problems that exist in the Unreal Engine, which can be fixed at runtime. The description of every single fix can be found in the specific file.

## :open_file_folder:  Interaction
This folder contains interaction related components, which can be attached to actors in your application to reflect a specific feature. Some of these components require the corresponding component on your pawn.

By simply inheriting from the `IGrabable` or `IClickable` interface, your actor will respond to the pawn's clicks. A grabable actor (an actor who inherited from `IGrabable`) is automatically attached to the controller when grabbed and can be dragged around. If you wish to alter this behavior you can give a `GrabbingBehaviorComponent` to the actor, which will constraint his movement on a line or circle. For details on the customization you can make, take a look at the header files of these GrabbingBeaviorComponents.
Aditionally, you can override the `OnGrabbed` and `OnReleased` functions which will be called by the pawn through delegates. 

A clickable actor should override the `OnClicked` function for interaction to take place. If the pawn clicks on an actor who inherited from `IClickable`, he calls the actor's `OnClicked` function.

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

## :open_file_folder: Utility
This folder contains a collection of functions/classes which can be helpful in every application

### :diamond_shape_with_a_dot_inside: VirtualRealityUtilities
The VirtualRealityUtilities.h is a collection of static functions which are specifically useful for all nDisplay related applications

### :diamond_shape_with_a_dot_inside: DemoConfig
The DemoConfig is an extension to the normally used `UDeveloperSettings`, which enable the storing and altering of ini file as usual within the project, but extends this functionality to generate these ini file in the shipped project on startup, if they are not there.
