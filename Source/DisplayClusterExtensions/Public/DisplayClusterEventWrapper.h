#pragma once

#include "ClusterSyncedMethodCaller.h"
#include "Templates/IsInvocable.h"

// A helper function to wrap the RegisterAutoTypedDelegate() call. This is necessary to extract the arguments types as a
// parameter pack.
template <typename ClassType, typename ReturnType, typename... ArgTypes>
inline void RegisterImpl(UClusterSyncedMethodCaller* ClusterCaller, ClassType* Receiver, const FString& MethodName,
	ReturnType (ClassType::*MethodPointer)(ArgTypes...))
{
}

template <typename MemberFunctionType, MemberFunctionType MemberFunction>
class ClusterEventWrapperEvent;

template <typename OwnerType, typename ReturnType, typename... ArgTypes, ReturnType (OwnerType::*MemberFunction)(ArgTypes...)>
class ClusterEventWrapperEvent<ReturnType (OwnerType::*)(ArgTypes...), MemberFunction>
{
public:
	using MemberFunctionType = decltype(MemberFunction);

	void Attach(OwnerType* Owner, UClusterSyncedMethodCaller* ClusterCaller)
	{
		check(this->Owner == nullptr);
		check(this->ClusterCaller == nullptr);
		this->Owner = Owner;
		this->ClusterCaller = ClusterCaller;
		ClusterCaller->RegisterSyncedAutoTypedMethod(
			Forward<const FString&>(MethodName), TBaseDelegate<ReturnType, ArgTypes...>::CreateUObject(Receiver, MemberFunction));
	}

	void Send(ArgTypes&&... Arguments)
	{
		ClusterCaller->CallAutoTypedMethodSynced(Forward<const FString&>(MethodName), Forward<ArgTypes>(Arguments)...);
	}

private:
	OwnerType* Owner = nullptr;
	UClusterSyncedMethodCaller* ClusterCaller = nullptr;
};

#define DECLARE_DISPLAY_CLUSTER_EVENT(OwningType, MethodIdentifier) \
	ClusterEventWrapperEvent<decltype(&OwningType::MethodIdentifier), &OwningType::MethodIdentifier> MethodIdentifier##Event

// #define DECLARE_DISPLAY_CLUSTER_EVENT(OwningType, MethodIdentifier)                                                            \
// 	class                                                                 \
// 	{                                                                                                                          \
// 	public:                                                                                                                    \
// 		inline void Register(OwningType* Receiver)                                                                             \
// 		{                                                                                                                      \
// 			if (ClusterCaller.Num() == 0)                                                                                      \
// 			{                                                                                                                  \
// 				ClusterCaller.Add(NewObject<UClusterSyncedMethodCaller>());                                                    \
// 				RegisterImpl(ClusterCaller[0], Receiver, #OwningType "_" #MethodIdentifier, &OwningType::MethodIdentifier);    \
// 			}                                                                                                                  \
// 			ClusterCaller[0]->RegisterListener();                                                                              \
// 		}                                                                                                                      \
// 		inline void Deregister()                                                                                               \
// 		{                                                                                                                      \
// 			ClusterCaller[0]->DeregisterListener();                                                                            \
// 		}                                                                                                                      \
// 		template <typename... ArgTypes>                                                                                        \
// 		void Send(ArgTypes&&... Arguments)                                                                                     \
// 		{                                                                                                                      \
// 			static_assert(                                                                                                     \
// 				TIsInvocable<decltype(&OwningType::MethodIdentifier), OwningType*, ArgTypes...>::Value, "Invalid Parameters"); \
// 			SendImpl<decltype(&OwningType::MethodIdentifier)>::Invoke(                                                         \
// 				ClusterCaller[0], #OwningType "_" #MethodIdentifier, Forward<ArgTypes>(Arguments)...);                         \
// 		}                                                                                                                      \
// 	} MethodIdentifier##Event;
