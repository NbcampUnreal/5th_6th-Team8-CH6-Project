#pragma once

#include "PTWHTTPRequestTypes.generated.h"

namespace HTTPStatusMessages
{
	extern PTW_API const FString SomethingWentWrong;
}

USTRUCT()
struct FPTWMetaData
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	int32 httpStatusCode{};
	
	UPROPERTY()
	FString requestId;
	
	UPROPERTY()
	int32 attempts{};
	
	UPROPERTY()
	double totalRetryDelay{};
	
	void Dump() const;
};

USTRUCT()
struct FPTWListFleetsResponse
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<FString> FleetIds{};
	
	UPROPERTY()
	FString NextToken;
	
	void Dump() const;
};
