#pragma once

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum EInteractorType: int
{
	None = 0 UMETA(Hidden),
	Raycast = 1 << 0,
	Spherecast = 1 << 1,
	Grab = 1 << 2,
	Reserved2 = 1 << 3,
	Reserved3 = 1 << 4,
	Reserved4 = 1 << 5,
	Reserved5 = 1 << 6,
	Reserved6 = 1 << 7,
	Reserved7 = 1 << 8,
	Reserved8 = 1 << 9,
	Reserved9 = 1 << 10,
	Reserved10 = 1 << 11,
	Reserved11 = 1 << 12,
	Reserved12 = 1 << 13,
	Reserved13 = 1 << 14,
	Reserved14 = 1 << 15,
	Custom1 = 1 << 16,
	Custom2 = 1 << 17,
	Custom3 = 1 << 18,
	Custom4 = 1 << 19,
	Custom5 = 1 << 20,
	Custom6 = 1 << 21,
	Custom7 = 1 << 22,
	Custom8 = 1 << 23,
	Custom9 = 1 << 24,
	Custom10 = 1 << 25,
	Custom11 = 1 << 26,
	Custom12 = 1 << 27,
	Custom13 = 1 << 28,
	Custom14 = 1 << 29,
	Custom15 = 1 << 30
};


ENUM_CLASS_FLAGS(EInteractorType);
