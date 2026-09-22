#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OWWeaponDefinition.generated.h"
class USoundBase;
class USkeletalMesh;
UCLASS(BlueprintType)
class OPENWORLD_API UOWWeaponDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName AmmoItem = "Ammo.9mm";
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1",ClampMax="200")) int32 MagazineSize = 15;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.05")) float ShotInterval = 0.18f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.1")) float ReloadSeconds = 1.6f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1")) float MuzzleVelocityMPS = 380.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0")) float Damage = 25.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0",ClampMax="20")) float SpreadDegrees = 0.7f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bAutomatic = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<USkeletalMesh> Mesh;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<USoundBase> ShotSound;
};
