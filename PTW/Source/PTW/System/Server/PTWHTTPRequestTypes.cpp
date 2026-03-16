#include "PTWHTTPRequestTypes.h"

namespace HTTPStatusMessages
{
	const FString SomethingWentWrong{TEXT("Something went wrong")};
}

void FPTWMetaData::Dump() const
{
	UE_LOG(LogTemp, Warning, TEXT("MetaData: "));
	UE_LOG(LogTemp, Warning, TEXT("MetaData: %d"), httpStatusCode );
	UE_LOG(LogTemp, Warning, TEXT("MetaData: %s"), *requestId );
	UE_LOG(LogTemp, Warning, TEXT("MetaData: %d"), attempts );
	UE_LOG(LogTemp, Warning, TEXT("MetaData: %f"), totalRetryDelay );
	
}

void FPTWListFleetsResponse::Dump() const
{
	UE_LOG(LogTemp, Warning, TEXT("ListFleetsResponse:"));
	for (const FString& FleetId : FleetIds)
	{
		UE_LOG(LogTemp, Warning, TEXT("FleetId: %s"), *FleetId);
	}
	if (!NextToken.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("NextToken: %s"), *NextToken);
	}
}
