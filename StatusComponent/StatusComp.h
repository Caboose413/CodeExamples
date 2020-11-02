// /*=================================================* File Created by: Caboose/Devin* Project name: Celestial* Unreal Engine version: 4.21.1* -------------------------------------------------* For parts referencing UE4 code, the following copyright applies:* Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.*=================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Core.h"
#include "UnrealNetwork.h"
#include "Celestial/Game/BasePS.h"
//#include "Engine/DataTable.h"
#include "StatusComp.generated.h"



UENUM(BlueprintType)
enum EStatus
{
    Health        UMETA(DisplayName = "Health"),
    HealthDelta   UMETA(DisplayName = "HealthDelta"),
    
    Oxygen        UMETA(DisplayName = "Oxygen"),
    OxygenDelta   UMETA(DisplayName = "OxygenDelta"),
    
    Water         UMETA(DisplayName = "Water"),    
    WaterDelta    UMETA(DisplayName = "WaterDelta"),
    
    Food          UMETA(DisplayName = "Food"), 
    FoodDelta     UMETA(DisplayName = "FoodDelta"),
    
    Heat          UMETA(DisplayName = "Heat"),
    HeatDelta     UMETA(DisplayName = "HeatDelta"),

    //Keep this the last entry so we know the lenght.
    LastEntry          UMETA(DisplayName = "LastEntry")
};    

UENUM(BlueprintType)
enum EStatusInfo
{
    Base         UMETA(DisplayName = "Base"),  
    Max          UMETA(DisplayName = "Max"),
    Min          UMETA(DisplayName = "Min")
};  

USTRUCT(BlueprintType)
struct FStatusType
{
    GENERATED_USTRUCT_BODY()
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Status")
    FString Name;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float Base;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float Min;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float Max;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CELESTIAL_API UStatusComp : public UActorComponent
{
    GENERATED_UCLASS_BODY()

protected:
    // Called when the game starts
    virtual void BeginPlay() override;
    

    UPROPERTY()
    AActor* Parent = GetOwner();
public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;


public:
    /*
    // Example how to access DataTable
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
    UDataTable* statusDataTable;
    */
    FOutputDeviceNull ar;


    UFUNCTION()
    ABasePS* GetPlayerState() const; 

    UFUNCTION(BlueprintCallable, Category = "iStatus|Functions")
    float& GetStatus(EStatus Name, EStatusInfo Info);

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Status")
    TArray<FStatusType> StatusType;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "iStatus|Functions")
    float& GetStatusPure(EStatus Name, EStatusInfo Info);
    
    UFUNCTION(BlueprintCallable, Category = "iStatus|Functions")
    float& SetStatus(float Value,  EStatus Name, EStatusInfo Info); // set a status (not increasing or decrease current value)

    UFUNCTION(BlueprintCallable, Category = "iStatus|Functions")
    float& AddStatus(float Value,  EStatus Name, EStatusInfo Info); // increase or decrease status

    UFUNCTION(BlueprintCallable, Category = "iStatus|Functions")
    float& AddStatusSave(float Value,  EStatus Name); // increase or decrease maximum of a status

    float& GetStatusBase(const EStatus Name);
    float& GetStatusMin(const EStatus Name);
    float& GetStatusMax(const EStatus Name);

    
    UFUNCTION(BlueprintCallable, Category = "iStatus|Functions")
    bool InShade();
    
    UFUNCTION()
    void ReciveOnStat();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "iStatus|Health")
    bool useHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "iStatus|Oxygen")
    bool useOxygen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "iStatus|Water")
    bool useWater;

    UPROPERTY(BlueprintReadWrite, Category = "iStatus")
    bool bUnderEffect;

    UPROPERTY(Replicated ,BlueprintReadOnly, Category = "iStatus")
    bool bIsLosingHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "iStatus|Food")
    bool useFood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "iStatus|Heat")
    bool useHeat;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "iStatus|Energy")
    bool useEnergy;

    UPROPERTY()
    AActor* SunLight;

    FCollisionQueryParams SunHitParams;
    FHitResult SunHitResult;
    
    UPROPERTY(BlueprintReadWrite, Category = "iStatus|Heat")
    float HotAlpha;
    
    UPROPERTY(BlueprintReadWrite, Category = "iStatus|Heat")
    float ColdAlpha;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FonLosingHealth);

    UPROPERTY(BlueprintAssignable, Category = "inventory")
    FonLosingHealth SendLosingHealthUpdate;

    void SetLosingLife();


    UPROPERTY(BlueprintReadWrite, Category = "iStatus")
    bool death = false;
    UPROPERTY(BlueprintReadWrite, Category = "iStatus")
    bool suffocate = false;
    UPROPERTY(BlueprintReadWrite, Category = "iStatus")
    bool thirsty = false;
    UPROPERTY(BlueprintReadWrite, Category = "iStatus")
    bool hungry = false;
    UPROPERTY(BlueprintReadWrite, Category = "iStatus")
    bool outOfEnergy = false;

private:

    float StatusNone = 0.0f;
};


