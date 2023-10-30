#pragma once

#include "IDisplayCluster.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "Cluster/DisplayClusterClusterEvent.h"
#include "DisplayClusterEventParameterHelper.h"
#include "Templates/IsInvocable.h"
#include "Templates/RemoveCV.h"

static constexpr int32 CLUSTER_EVENT_WRAPPER_EVENT_ID = 1337420;

template <typename MemberFunctionType, MemberFunctionType MemberFunction>
class ClusterEventWrapperEvent;

template <typename ObjectType, typename ReturnType, typename... ArgTypes, ReturnType (ObjectType::*
	          MemberFunction)(ArgTypes...)>
class ClusterEventWrapperEvent<ReturnType (ObjectType::*)(ArgTypes...), MemberFunction>
{
	static_assert(TIsDerivedFrom<ObjectType, UObject>::IsDerived, "Object needs to derive from UObject");

public:
	using MemberFunctionType = decltype(MemberFunction);

	ClusterEventWrapperEvent(const TCHAR* MethodName) : MethodName{MethodName}
	{
	}

	void Attach(ObjectType* NewObject)
	{
		checkf(Object == nullptr, TEXT("The event is already attached."));
		Object = NewObject;
		ObjectId = Object->GetUniqueID();

		EDisplayClusterOperationMode OperationMode = IDisplayCluster::Get().GetOperationMode();
		if (OperationMode == EDisplayClusterOperationMode::Cluster)
		{
			IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
			check(ClusterManager != nullptr);

			check(!ClusterEventListenerDelegate.IsBound());
			ClusterEventListenerDelegate = FOnClusterEventBinaryListener::CreateLambda(
				[this](const FDisplayClusterClusterEventBinary& Event)
				{
					if (Event.EventId != CLUSTER_EVENT_WRAPPER_EVENT_ID)
					{
						return;
					}

					FMemoryReader MemoryReader(Event.EventData);

					uint32 EventObjectId;
					MemoryReader << EventObjectId; // This is reads the value!
					if (EventObjectId != ObjectId)
					{
						// Event does not belong to this object.
						return;
					}

					FString EventMethodName;
					MemoryReader << EventMethodName; // This is reads the value!
					if (EventMethodName != MethodName)
					{
						// This event does not belong to this method.
						return;
					}

					// Create a tuple that holds all arguments. This assumes that all
					// argument types are default constructible. However, all
					// types that overload the FArchive "<<" operator probably are.
					TTuple<typename TRemoveCV<typename TRemoveReference<ArgTypes>::Type>::Type...> ArgumentTuple;

					// This call will deserialze the values and fill all values in the tuple appropriately.
					FillArgumentTuple<0>(&MemoryReader, &ArgumentTuple);

					ArgumentTuple.ApplyBefore([this](const ArgTypes&... Arguments)
					{
						(Object->*MemberFunction)(Forward<const ArgTypes&>(Arguments)...);
					});
				});
			ClusterManager->AddClusterEventBinaryListener(ClusterEventListenerDelegate);
		}
	}

	void Detach()
	{
		checkf(Object != nullptr, TEXT("The event was never attached."));

		EDisplayClusterOperationMode OperationMode = IDisplayCluster::Get().GetOperationMode();
		if (OperationMode == EDisplayClusterOperationMode::Cluster)
		{
			IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
			check(ClusterManager != nullptr);

			// check(ClusterEventListenerDelegate.IsBound());
			ClusterManager->RemoveClusterEventBinaryListener(ClusterEventListenerDelegate);
		}
	}

	void Send(ArgTypes... Arguments)
	{
		checkf(Object != nullptr, TEXT("The event was not attached."));

		EDisplayClusterOperationMode OperationMode = IDisplayCluster::Get().GetOperationMode();
		if (OperationMode != EDisplayClusterOperationMode::Cluster)
		{
			(Object->*MemberFunction)(Forward<ArgTypes>(Arguments)...);
		}
		else
		{
			IDisplayClusterClusterManager* ClusterManager = IDisplayCluster::Get().GetClusterMgr();
			check(ClusterManager != nullptr);

			FDisplayClusterClusterEventBinary ClusterEvent;
			ClusterEvent.EventId = CLUSTER_EVENT_WRAPPER_EVENT_ID;
			ClusterEvent.bShouldDiscardOnRepeat = false;

			FMemoryWriter MemoryWriter(ClusterEvent.EventData);
			MemoryWriter << ObjectId;
			MemoryWriter << const_cast<FString&>(MethodName); // const_cast... thanks unreal!
			SerializeParameters(&MemoryWriter, Forward<ArgTypes>(Arguments)...);

			ClusterManager->EmitClusterEventBinary(ClusterEvent, true);
		}
	}

private:
	const FString MethodName;
	uint32 ObjectId;
	ObjectType* Object = nullptr;
	FOnClusterEventBinaryListener ClusterEventListenerDelegate;
};

#define DCEW_STRINGIFY(x) #x
#define DCEW_TOSTRING(x) DCEW_STRINGIFY(x)

#define DECLARE_DISPLAY_CLUSTER_EVENT(OwningType, MethodIdentifier)                                                          \
	ClusterEventWrapperEvent<decltype(&OwningType::MethodIdentifier), &OwningType::MethodIdentifier> MethodIdentifier##Event \
	{                                                                                                                        \
		TEXT(DCEW_TOSTRING(OwningType) DCEW_TOSTRING(MethodIdentifier))                                                      \
	}
