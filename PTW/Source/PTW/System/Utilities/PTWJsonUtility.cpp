#include "PTWJsonUtility.h"

FString UPTWJsonUtility::MakeHTTPRequestBody(const TMap<FString, FString>& Parameters)
{
	TSharedPtr<FJsonObject> ContentJsonObject = MakeShareable(new FJsonObject());
	
	for (const auto& Param : Parameters)
	{
		ContentJsonObject->SetStringField(Param.Key, Param.Value);
	}
	
	FString Content;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Content);
	FJsonSerializer::Serialize(ContentJsonObject.ToSharedRef(), JsonWriter);
	
	return Content;
}

TMap<FString, FString> UPTWJsonUtility::ExtractFieldsFromHTTPData(const FString& JsonString, const TArray<FString>& TargetFields)
{
	TMap<FString, FString> ResultMap;

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		const TSharedPtr<FJsonObject>* DataObjPtr = nullptr;

		if (JsonObject->TryGetObjectField(TEXT("data"), DataObjPtr) && DataObjPtr && DataObjPtr->IsValid())
		{
			for (const FString& FieldName : TargetFields)
			{
				TSharedPtr<FJsonValue> JsonValue = (*DataObjPtr)->TryGetField(FieldName);
          
				if (JsonValue.IsValid() && !JsonValue->IsNull())
				{
					ResultMap.Add(FieldName, JsonValue->AsString());
				}
				else
				{
					ResultMap.Add(FieldName, TEXT("")); 
				}
			}
		}
	}
	
	return ResultMap;
}
