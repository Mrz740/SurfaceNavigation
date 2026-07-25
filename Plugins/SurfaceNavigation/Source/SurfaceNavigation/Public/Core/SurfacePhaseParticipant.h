// Copyright (c) Mrz740. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SurfacePhaseParticipant.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USurfacePhaseParticipant : public UInterface
{
	GENERATED_BODY()
};

class SURFACENAVIGATION_API ISurfacePhaseParticipant
{
	GENERATED_BODY()

public:
	virtual void ExecuteReadPhase() = 0;
	virtual void ExecuteSimulatePhase() = 0;
	virtual void ExecuteCommitPhase() = 0;
};
