#pragma once

#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

template <typename ParameterType, typename... RemainingParameterTypes>
inline void SerializeParameters(FMemoryWriter* MemoryWriter, ParameterType&& Parameter, RemainingParameterTypes&&... RemainingParameters)
{
	using NonConstType = typename TRemoveCV<typename TRemoveReference<ParameterType>::Type>::Type;
	(*MemoryWriter) << const_cast<NonConstType&>(Parameter); // const_cast because the same operator (<<) is used for reading and writing
	SerializeParameters(MemoryWriter, Forward<RemainingParameterTypes>(RemainingParameters)...);
}

inline void SerializeParameters(FMemoryWriter* MemoryWriter)
{
}

// This is a wrapper function to recursively fill the argument tuple. This overload is only used if the index indicating the
// currently handled attribute is less than the number of total attributes. I.e., if the attribute index is valid.
template <int CurrentIndex, typename... ArgTypes>
inline typename TEnableIf<(CurrentIndex < sizeof...(ArgTypes))>::Type
FillArgumentTuple(FMemoryReader* MemoryReader, TTuple<ArgTypes...>* ArgumentTuple)
{
	// Read the "<<" as ">>" operator here. FArchive uses the same for both and decides based on an internal type on what to do. So
	// this statement parses the bytes that were passed into reader and puts the parsed object into the tuple at index CurrentIndex.
	(*MemoryReader) << ArgumentTuple->template Get<CurrentIndex>();

	// Recursive call for the remaining attributes.
	FillArgumentTuple<CurrentIndex + 1>(MemoryReader, Forward<TTuple<ArgTypes...>*>(ArgumentTuple));
}

// The overload that is called if we are "passed the end" of attributes.
template <int CurrentIndex, typename... ArgTypes>
inline typename TEnableIf<(CurrentIndex >= sizeof...(ArgTypes))>::Type
FillArgumentTuple(FMemoryReader* MemoryReader, TTuple<ArgTypes...>* ArgumentTuple)
{
}

template <typename RetType, typename... ArgTypes>
inline RetType CallDelegateWithParameterMap(
	const TDelegate<RetType, ArgTypes...>& Delegate, const TMap<FString, FString>& Parameters)
{
	// Create a tuple that holds all arguments. This assumes that all argument types are default constructible. However, all
	// types that overload the FArchive "<<" operator probably are.
	TTuple<typename TRemoveCV<typename TRemoveReference<ArgTypes>::Type>::Type...> ArgumentTuple;

	// This call will parse the string map and fill all values in the tuple appropriately.
	FillArgumentTuple<0>(&ArgumentTuple, Parameters);

	// The lambda function is only necessary because delegates do not overload the ()-operator but use the Execute() method
	// instead. So, the lambda acts as a wrapper.
	return ArgumentTuple.ApplyBefore([Delegate](ArgTypes&&... Arguments) { Delegate.Execute(Forward<ArgTypes>(Arguments)...); });
}
