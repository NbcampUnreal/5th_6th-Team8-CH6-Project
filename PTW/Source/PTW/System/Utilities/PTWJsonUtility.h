#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PTWJsonUtility.generated.h"

/**
 * Json 데이터 직렬화 / 역직렬화 유틸리티 클래스
 */
UCLASS()
class PTW_API UPTWJsonUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// ------- HTTP Request에서 사용되는 Json 함수 -------
	// POST Request로 전송할 데이터 직렬화 함수 (Serialize)
	UFUNCTION(BlueprintCallable, Category = "JSON")
	static FString MakeHTTPRequestBody(const TMap<FString, FString>& Params);
	
	// 배열 원소명 == 필드명이 동일한 데이터들을 TMap으로 추출하는 함수
	UFUNCTION(BlueprintCallable, Category = "JSON")
	static TMap<FString, FString> ExtractFieldsFromHTTPData(const FString& JsonString, const TArray<FString>& TargetFields);
	
	// 단일 구조체 파싱 함수
	template <typename T>
	static bool ParseHTTPDataToStruct(const FString& JsonString, T& OutStruct)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
		if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
		{
			const TSharedPtr<FJsonObject>* DataObjPtr = nullptr;
			if (JsonObject->TryGetObjectField(TEXT("data"), DataObjPtr) && DataObjPtr->IsValid())
			{
				return FJsonObjectConverter::JsonObjectToUStruct(DataObjPtr->ToSharedRef(), &OutStruct);
			}
		}
		return false;
	}
	
	// 배열 구조체 파싱 함수
	template <typename T>
	static bool ParseHTTPDataToArray(const FString& JsonString, TArray<T>& OutArray)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

		if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
		{
			const TArray<TSharedPtr<FJsonValue>>* DataArrayPtr = nullptr;
			if (JsonObject->TryGetArrayField(TEXT("data"), DataArrayPtr))
			{
				return FJsonObjectConverter::JsonArrayToUStruct(*DataArrayPtr, &OutArray);
			}
		}
		return false;
	}
};
