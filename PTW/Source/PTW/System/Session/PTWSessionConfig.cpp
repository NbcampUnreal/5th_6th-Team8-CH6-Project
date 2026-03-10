// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWSessionConfig.h"

void FAWSSessionConfig::Dump() const
{
	UE_LOG(LogTemp, Log, TEXT("=== Game Session Info ==="));
	UE_LOG(LogTemp, Log, TEXT("CreationTime: %f"), CreationTime);
	UE_LOG(LogTemp, Log, TEXT("CreatorId: %s"), *CreatorId);
	UE_LOG(LogTemp, Log, TEXT("CurrentPlayerSessionCount: %d"), CurrentPlayerSessionCount);
	UE_LOG(LogTemp, Log, TEXT("DNSName: %s"), *DNSName);
	UE_LOG(LogTemp, Log, TEXT("FleetArn: %s"), *FleetArn);
	UE_LOG(LogTemp, Log, TEXT("FleetId: %s"), *FleetId);
	
	UE_LOG(LogTemp, Log, TEXT("GameProperties: "));
	for (const auto& Pair : GameProperties)
	{
		UE_LOG(LogTemp, Log, TEXT("	%s: %s"), *Pair.Key, *Pair.Value);
	}
	
	UE_LOG(LogTemp, Log, TEXT("GameSessionData: %s"), *GameSessionData);
	UE_LOG(LogTemp, Log, TEXT("GameSessionId: %s"), *GameSessionId);
	UE_LOG(LogTemp, Log, TEXT("IPAddress: %s"), *IPAddress);
	UE_LOG(LogTemp, Log, TEXT("Location: %s"), *Location);
	UE_LOG(LogTemp, Log, TEXT("MatchMakerData: %s"), *MatchMakerData);
	UE_LOG(LogTemp, Log, TEXT("MaximumPlayerSessionCount: %d"), MaximumPlayerSessionCount);
	UE_LOG(LogTemp, Log, TEXT("Name: %s"), *Name);
	UE_LOG(LogTemp, Log, TEXT("PlayerSessionCreationPolicy: %s"), *PlayerSessionCreationPolicy);
	UE_LOG(LogTemp, Log, TEXT("Port: %d"), Port);
	UE_LOG(LogTemp, Log, TEXT("Status: %s"), *Status);
	UE_LOG(LogTemp, Log, TEXT("StatusReason: %s"), *StatusReason);
	UE_LOG(LogTemp, Log, TEXT("TerminationTime: %f"), TerminationTime);
	UE_LOG(LogTemp, Log, TEXT("========================="));
	
}
