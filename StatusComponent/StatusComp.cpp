// /*=================================================* File Created by: Caboose/Devin* Project name: Celestial* Unreal Engine version: 4.21.1* -------------------------------------------------* For parts referencing UE4 code, the following copyright applies:* Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.*=================================================*/


#include "StatusComp.h"
#include "Engine/DirectionalLight.h"

#include "Components/DirectionalLightComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UStatusComp::UStatusComp(const FObjectInitializer& ObjectInitializer)
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;

    SetIsReplicatedByDefault(true);
    StatusType.AddDefaulted(EStatus::LastEntry);

    //Health
    StatusType[Health].Name = "Health";
    StatusType[Health].Base = 100.0f;
    StatusType[Health].Min = 0.0f;
    StatusType[Health].Max = 100.0f;

    StatusType[HealthDelta].Name = "HealthDelta";
    StatusType[HealthDelta].Base = 0.0f;
    StatusType[HealthDelta].Min = 0.0f;
    StatusType[HealthDelta].Max = 10.0f;

    //Oxygen
    StatusType[Oxygen].Name = "Oxygen";
    StatusType[Oxygen].Base = 400.0f;
    StatusType[Oxygen].Min = 0.0f;
    StatusType[Oxygen].Max = 400.0f;

    StatusType[OxygenDelta].Name = "OxygenDelta";
    StatusType[OxygenDelta].Base = 0.2f;
    StatusType[OxygenDelta].Min = 0.1f;
    StatusType[OxygenDelta].Max = 0.8f;

    //Water
    StatusType[Water].Name = "Water";
    StatusType[Water].Base = 200.0f;
    StatusType[Water].Min = 0.0f;
    StatusType[Water].Max = 200.0f;

    StatusType[WaterDelta].Name = "WaterDelta";
    StatusType[WaterDelta].Base = 0.2f;
    StatusType[WaterDelta].Min = 0.1f;
    StatusType[WaterDelta].Max = 0.8f;

    //Food
    StatusType[Food].Name = "Food";
    StatusType[Food].Base = 300.0f;
    StatusType[Food].Min = 0.0f;
    StatusType[Food].Max = 300.0f;

    StatusType[FoodDelta].Name = "FoodDelta";
    StatusType[FoodDelta].Base = 0.2f;
    StatusType[FoodDelta].Min = 0.1f;
    StatusType[FoodDelta].Max = 0.8f;

    //Heat
    StatusType[Heat].Name = "Heat";
    StatusType[Heat].Base = 25.0f;
    StatusType[Heat].Min = -50.0f;
    StatusType[Heat].Max = 50.0f;

    StatusType[HeatDelta].Name = "HeatDelta";
    StatusType[HeatDelta].Base = 0.0f;
    StatusType[HeatDelta].Min = -1.0f;
    StatusType[HeatDelta].Max = 1.0f;
}


// Called when the game starts
void UStatusComp::BeginPlay()
{
    Super::BeginPlay();

    
}


ABasePS* UStatusComp::GetPlayerState() const 
{
    if (GetOwner())
    {
        return Cast<ABasePS>(Cast<ACharacter>(GetOwner())->GetPlayerState());
    }
    return nullptr;
}

float& UStatusComp::GetStatus(EStatus Name, EStatusInfo Info)
{
    FStatusType& Type = StatusType[Name];
    switch (Info)
    {
        case Base:
            return Type.Base;
        case Min:
            return Type.Min;
        case Max:
            return Type.Max;
    }
    return  StatusNone;
}

float& UStatusComp::GetStatusPure(EStatus Name, EStatusInfo Info)
{
   return GetStatus(Name, Info);      
}

/** set a status (not increasing or decrease current value)  */
float& UStatusComp::SetStatus(float Value, EStatus Name, EStatusInfo Info)
{
    return GetStatus(Name, Info) = Value;
}

/** increase or decrease status  */
float& UStatusComp::AddStatus(float Value, EStatus Name, EStatusInfo Info)
{
    return GetStatus(Name, Info) += Value;
}

/** add or reduce a status clamped to it's min/max  */
float& UStatusComp::AddStatusSave(float Value, EStatus Name)
{
    const float Base = GetStatusBase(Name);
    const float Max = GetStatusMax(Name);
    const float Min = GetStatusMin(Name);
    const float Result = Base + Value;
    
    return SetStatus(FMath::Clamp(Result, Min, Max), Name, EStatusInfo::Base);
}

float& UStatusComp::GetStatusBase(const EStatus Name)
{
    return GetStatus(Name, Base);
}

float& UStatusComp::GetStatusMin(const EStatus Name)
{
      return GetStatus(Name, Min);
}

float& UStatusComp::GetStatusMax(const EStatus Name)
{
    return GetStatus(Name, Max);
}

bool UStatusComp::InShade()
{
    if (!SunLight)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SunLight = FoundActors[0];
        }
        return false;
    }
 
    if (!SunHitParams.GetIgnoredActors().IsValidIndex(0))
    {
        SunHitParams.AddIgnoredActor(GetOwner());
    }
    
    const FVector Start = GetOwner()->GetActorLocation();
    const FVector End = Start + -SunLight->GetActorForwardVector() * (90000.0f * 1000.0f);
    
    GetWorld()->LineTraceSingleByChannel(SunHitResult, Start, End, ECC_Visibility, SunHitParams);

    //If we neither hit the sun or moon we are in shade.
    if (SunHitResult.Actor != nullptr)
    {
        const bool InShade = !(SunHitResult.Actor->ActorHasTag(TEXT("Sun")) || SunHitResult.Actor->ActorHasTag(TEXT("Moon")));
        //UE_LOG(LogTemp, Warning, TEXT("InShade %i"), InShade);
        return InShade;
    }
    return false;
}

void UStatusComp::ReciveOnStat()
{
    Parent->CallFunctionByNameWithArguments(TEXT("UpdateStatus"), ar, nullptr, true);
}

void UStatusComp::SetLosingLife()
{
    const bool NewStat = suffocate || thirsty || hungry || bUnderEffect;
    if (NewStat != bIsLosingHealth)
    {
        bIsLosingHealth = NewStat;
        SendLosingHealthUpdate.Broadcast();
    }
}

// Called every frame
void UStatusComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    //UE_LOG(LogTemp, Warning, TEXT("Water"));

    if (useHealth)
    {
        if (GetStatus(Health, Base) <= 0.0f)
        {
            death = true;
            Parent->CallFunctionByNameWithArguments(TEXT("death"), ar, nullptr, true);
        }
        else
        {
            death = false;
        }
        if (bIsLosingHealth)
        {
            AddStatusSave(-GetStatusBase(HealthDelta), Health);
        }
    }

    if (useOxygen)
    {
        if (GetStatus(Health, Base) <= 0.0f)
        {
            suffocate = true;
        }
        else
        {
            suffocate = false;
        }
        AddStatusSave(-GetStatusBase(WaterDelta), Oxygen);
    }

    if (useWater)
    {
        if (GetStatus(Health, Base) <= 0.0f)
        {
            thirsty = true;
        }
        else
        {
            thirsty = false;
        }
        AddStatusSave(-GetStatusBase(WaterDelta), Water);
    }

    if (useFood)
    {
        if (GetStatus(Health, Base) <= 0.0f)
        {
            hungry = true;
        }
        else
        {
            hungry = false;
        }
        AddStatusSave(-GetStatusBase(FoodDelta), Food);
    }

    if (useHeat)
    {
		ABasePS* PlayerState = GetPlayerState();
		
        if (!PlayerState)
        {
			return;
        }
        
        const float CurHeat = GetStatus(Heat, Base);
        float BiomeHeat = PlayerState->GetBiomeTampreture();
        
        if (PlayerState->Biome)
        {
            BiomeHeat -= (InShade() * PlayerState->Biome->InShadeOffset);
        }
        else
        {
            BiomeHeat -= (InShade() * 10.0f);
        }

        BiomeHeat += Cast<ABasePlanetCharacter>(GetOwner())->bSprinting * 8.0f;
        
    
        const float HeatDif = BiomeHeat - CurHeat;

        float HeatIncreaseDir;

        if (HeatDif >= 0.0f)
        {
            HeatIncreaseDir = 1.0f;
        }
        else
        {
            HeatIncreaseDir = -1.0f;
        }
           
        float HeatAlpha;
        
        if (HeatDif == 0.0f)
        {
            HeatAlpha = 0.0f;
        }
        else
        {
            HeatAlpha = abs(HeatDif) / 50.0f;
        }
        
        const float HeatIncrease = FMath::Lerp(0.0f, 5.0f, HeatAlpha);
        
        AddStatusSave(HeatIncrease * FMath::Clamp(HeatIncreaseDir, -1.0f, 1.0f), Heat);
       
		const float HotOverFlow = FMath::Clamp(CurHeat -35.0f, 0.0f, 40.0f);      
        HotAlpha = HotOverFlow / 35.0f;
        const float ColdOverFlow = FMath::Clamp(CurHeat + 5.0f, -40.0f, 0.0f);
        ColdAlpha = abs(ColdOverFlow) / 30.0f;
        
        SetStatus(FMath::Lerp(GetStatusMin(FoodDelta), GetStatusMax(FoodDelta), ColdAlpha), FoodDelta, Base);
        SetStatus(FMath::Lerp(GetStatusMin(WaterDelta), GetStatusMax(WaterDelta), HotAlpha), WaterDelta, Base);
        //UE_LOG(LogTemp, Error, TEXT("Increase %f Dif %f Heat %f Alpha %f"), HeatIncrease, HeatDif, GetStatus(Heat, Base), HeatAlpha);
        //UE_LOG(LogTemp, Error, TEXT("OverFlow %f HotAlpha %f Heat %f"), HotOverFlow, HotAlpha, CurHeat);
        //UE_LOG(LogTemp, Error, TEXT("OverFlow %f ColdAlpha %f Heat %f"), ColdOverFlow, ColdAlpha, CurHeat);
        //UE_LOG(LogTemp, Error, TEXT("FoodUse %f WaterUse %f"), Status.FoodUsage, Status.WaterUsage);
 
        suffocate = (Heat >= GetStatus(Heat, Max) || Heat <= GetStatus(Heat, Min));
    }

    SetLosingLife();
}

void UStatusComp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UStatusComp, StatusType);
    DOREPLIFETIME(UStatusComp, bIsLosingHealth);
}
