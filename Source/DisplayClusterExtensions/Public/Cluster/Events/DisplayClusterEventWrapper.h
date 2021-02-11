#pragma once

#include "IDisplayCluster.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "Cluster/DisplayClusterClusterEvent.h"
#include "DisplayClusterEventParameterHelper.h"
#include "Templates/IsInvocable.h"

template <typename MemberFunctionType, MemberFunctionType MemberFunction>
class ClusterEventWrapperEvent;

template <typename ObjectType, typename ReturnType, typename... ArgTypes, ReturnType (ObjectType::*MemberFunction)(ArgTypes...)>
class ClusterEventWrapperEvent<ReturnType (ObjectType::*)(ArgTypes...), MemberFunction>
{
	static_assert(TIsDerivedFrom<ObjectType, AActor>::IsDerived, "Object needs to derive from AActor");

public:
	using MemberFunctionType = decltype(MemberFunction);

	ClusterEventWrapperEvent(const TCHAR* EventTypeName) : EventTypeName{EventTypeName}
	{
	}

	void Attach(ObjectType* NewObject)
	{
		IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
		check(ClusterManager != nullptr);

		checkf(Object == nullptr, TEXT("The event is already attached."));
		Object = NewObject;
		ObjectName = AActor::GetDebugName(Object);

		if (!ClusterManager->IsStandalone())
		{
			check(!ClusterEventListenerDelegate.IsBound());
			ClusterEventListenerDelegate = FOnClusterEventListener::CreateLambda([this](const FDisplayClusterClusterEvent& Event) {
				if (Event.Type == EventTypeName && Event.Name == ObjectName)
				{
					// Create a tuple that holds all arguments. This assumes that all
					// argument types are default constructible. However, all
					// types that overload the FArchive "<<" operator probably are.
					TTuple<typename TRemoveCV<typename TRemoveReference<ArgTypes>::Type>::Type...> ArgumentTuple;

					// This call will parse the string map and fill all values in the
					// tuple appropriately.
					FillArgumentTuple<0>(&ArgumentTuple, Event.Parameters);

					ArgumentTuple.ApplyBefore([this](const ArgTypes&... Arguments) {
						(Object->*MemberFunction)(Forward<const ArgTypes&>(Arguments)...);
					});
				}
			});
			ClusterManager->AddClusterEventListener(ClusterEventListenerDelegate);
		}
	}

	void Detach()
	{
		checkf(Object != nullptr, TEXT("The event was never attached."));

		IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
		check(ClusterManager != nullptr);

		if (!ClusterManager->IsStandalone())
		{
			// check(ClusterEventListenerDelegate.IsBound());
			ClusterManager->RemoveClusterEventListener(ClusterEventListenerDelegate);
		}
	}

	void Send(ArgTypes&&... Arguments)
	{
		IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
		check(ClusterManager != nullptr);

		checkf(Object != nullptr, TEXT("The event was not attached."));

		if (ClusterManager->IsStandalone())
		{
			(Object->*MemberFunction)(Forward<ArgTypes>(Arguments)...);
		}
		else
		{
			FDisplayClusterClusterEvent ClusterEvent;
			ClusterEvent.Category = "DisplayClusterEventWrapper";
			ClusterEvent.Type = EventTypeName;
			ClusterEvent.Name = ObjectName;
			ClusterEvent.Parameters = CreateParameterMap(Forward<ArgTypes>(Arguments)...);

			ClusterManager->EmitClusterEvent(ClusterEvent, true);
		}
	}

private:
	const TCHAR* EventTypeName;
	ObjectType* Object = nullptr;
	FString ObjectName;
	FOnClusterEventListener ClusterEventListenerDelegate;
};

#define DCEW_STRINGIFY(x) #x
#define DCEW_TOSTRING(x) DCEW_STRINGIFY(x)

#define DECLARE_DISPLAY_CLUSTER_EVENT(OwningType, MethodIdentifier)                                                          \
	ClusterEventWrapperEvent<decltype(&OwningType::MethodIdentifier), &OwningType::MethodIdentifier> MethodIdentifier##Event \
	{                                                                                                                        \
		TEXT(DCEW_TOSTRING(OwningType) DCEW_TOSTRING(MethodIdentifier))                                                      \
	}
