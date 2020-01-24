#pragma once

#include "ClusterSyncedMethodCaller.h"
#include "Templates/IsInvocable.h"

template <typename ClassType, typename ReturnType, typename... ArgTypes>
inline void RegisterImpl(UClusterSyncedMethodCaller* ClusterCaller, ClassType* Recevier, const FString& MethodName,
	ReturnType (ClassType::*MethodPointer)(ArgTypes...))
{
	ClusterCaller->RegisterAutoTypedDelegate(
		Forward<const FString&>(MethodName), TBaseDelegate<ReturnType, ArgTypes...>::CreateUObject(Recevier, MethodPointer));
}

template <typename T>
struct SendImpl;

template <typename ClassType, typename ReturnType, typename... ArgTypes>
struct SendImpl<ReturnType (ClassType::*)(ArgTypes...)>
{
	static void Invoke(UClusterSyncedMethodCaller* ClusterCaller, const FString& MethodName, ArgTypes&&... Arguments)
	{
		ClusterCaller->CallAutoTypedDelegate(Forward<const FString&>(MethodName), Forward<ArgTypes>(Arguments)...);
	}
};

#define DECLARE_DISPLAY_CLUSTER_EVENT(OwningType, MethodIdentifier)                                                            \
	class                                                                                                                      \
	{                                                                                                                          \
		UPROPERTY()                                                                                                            \
		UClusterSyncedMethodCaller* ClusterCaller = nullptr;                                                                   \
                                                                                                                               \
	public:                                                                                                                    \
		inline void Register(OwningType* Receiver)                                                                             \
		{                                                                                                                      \
			if (ClusterCaller == nullptr)                                                                                      \
			{                                                                                                                  \
				ClusterCaller = NewObject<UClusterSyncedMethodCaller>();                                                       \
				RegisterImpl(ClusterCaller, Receiver, #OwningType "_" #MethodIdentifier, &OwningType::MethodIdentifier);       \
			}                                                                                                                  \
			ClusterCaller->RegisterListener();                                                                                 \
		}                                                                                                                      \
		inline void Deregister()                                                                                               \
		{                                                                                                                      \
			ClusterCaller->DeregisterListener();                                                                               \
		}                                                                                                                      \
		template <typename... ArgTypes>                                                                                        \
		void Send(ArgTypes&&... Arguments)                                                                                     \
		{                                                                                                                      \
			static_assert(                                                                                                     \
				TIsInvocable<decltype(&OwningType::MethodIdentifier), OwningType*, ArgTypes...>::Value, "Invalid Parameters"); \
			SendImpl<decltype(&OwningType::MethodIdentifier)>::Invoke(                                                         \
				ClusterCaller, #OwningType "_" #MethodIdentifier, Forward<ArgTypes>(Arguments)...);                            \
		}                                                                                                                      \
	} MethodIdentifier##Event;
