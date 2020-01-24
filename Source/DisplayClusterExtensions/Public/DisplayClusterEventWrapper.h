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

template <typename ClassType, typename ReturnType, typename... ArgTypes>
inline void SendImpl()
{
}

#define DECLARE_DISPLAY_CLUSTER_EVENT(OwningType, MethodName)                                                                      \
	class                                                                                                                          \
	{                                                                                                                              \
		UClusterSyncedMethodCaller* ClusterCaller = nullptr;                                                                       \
                                                                                                                                   \
	public:                                                                                                                        \
		inline void Register(OwningType* Receiver)                                                                                 \
		{                                                                                                                          \
			if (ClusterCaller == nullptr)                                                                                          \
			{                                                                                                                      \
				ClusterCaller = NewObject<UClusterSyncedMethodCaller>();                                                           \
				RegisterImpl(ClusterCaller, Receiver, #OwningType "_" #MethodName, &OwningType::MethodName);                       \
			}                                                                                                                      \
			ClusterCaller->RegisterListener();                                                                                     \
		}                                                                                                                          \
		inline void Deregister()                                                                                                   \
		{                                                                                                                          \
			ClusterCaller->DeregisterListener();                                                                                   \
		}                                                                                                                          \
		template <typename... ArgTypes>                                                                                            \
		void Send(ArgTypes&&... Arguments)                                                                                         \
		{                                                                                                                          \
			static_assert(TIsInvocable<decltype(&OwningType::MethodName), OwningType*, ArgTypes...>::Value, "Invalid Parameters"); \
			ClusterCaller->CallAutoTypedDelegate(#OwningType "_" #MethodName, Forward<ArgTypes>(Arguments)...);                    \
		}                                                                                                                          \
	} MethodName##Event;
