// Copyright Two Bit Studios 2025. All Rights Reserved.

#include "GridVisuals.h"

#if WITH_EDITOR

#include "Editor.h"
#include "RenderingThread.h"
#include "FileHelpers.h"

#endif

#include "Engine/Texture2D.h"

#include "GridUtils.h"
#include "GridSubsystem.h"
#include "EngineUtils.h"
#include "GridManager.h"
#include "GridDataStructures.h"
#include "Async/Async.h"
#include "Materials/MaterialInstanceConstant.h"

#define CHECK_GRIDPOSTPROCESS() if (!GridPostProcess) return;

// Sets default values
AGridVisuals::AGridVisuals()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bIsInitialized = false;

	// Create a post-process component for grid material
	GridPostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("Grid"));
	GridPostProcess->SetupAttachment(RootComponent);
}

void AGridVisuals::PostLoad()
{
	Super::PostLoad();
	World = GetWorld();

#if WITH_EDITOR
	World = GEditor->GetEditorWorldContext().World();
#endif

	GridSubsystem = UGridUtils::GetGridSubsystem();
}

// Called when the game starts or when spawned

void AGridVisuals::BeginPlay()
{
	Super::BeginPlay();
	GridSubsystem = UGridUtils::GetGridSubsystem();

	// Hide Editor visuals when game starts
#if WITH_EDITOR
	SetVisualsVisibility(EGridVisualContext::Editor, false);
#endif
	
	LoadAllVisuals(EGridVisualContext::Runtime);
}

void AGridVisuals::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Unhide Editor Visuals when game stops
#if WITH_EDITOR
	SetVisualsVisibility(EGridVisualContext::Editor, true);
#endif
}

void AGridVisuals::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	GridSubsystem = UGridUtils::GetGridSubsystem();
	bool bIsHex = GridSubsystem->GetIsHex();
}

#if WITH_EDITOR
void AGridVisuals::PostEditMove(bool bFinished)
{
	if (bFinished && !bIsPlaced)
	{
		bIsPlaced = true;

		// Load all visuals when placed for the first time.
		LoadAllVisuals(EGridVisualContext::Editor); 
	}
}
#endif

void AGridVisuals::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AGridVisuals::IsCoreValid()
{
	if (!GridSubsystem)
	{
		return false;
	}
	
	return true;
}

void AGridVisuals::PrecomputeColors()
{
	// Pre loads colors into respective TMaps for quicker lookups.        
	// Make sure to call after each table is updated.        
	 
	if (TileStateMapping)
	{
		StateColorMap.Empty();

		static const FString ContextString(TEXT("Tile State Mapping Context"));
		for (auto& RowName : TileStateMapping->GetRowNames())
		{
			FTileStateMapping* Row = TileStateMapping->FindRow<FTileStateMapping>(RowName, ContextString, false);
			if (Row)
			{
				StateColorMap.Add(Row->TileStateName, FVector4(Row->StateColor));
			}
		}
	}

	if (TileTypeMapping)
	{
		TypeColorMap.Empty();

		static const FString ContextString(TEXT("Tile Type Mapping Context"));
		for (auto& RowName : TileTypeMapping->GetRowNames())
		{
			FTileTypeMapping* Row = TileTypeMapping->FindRow<FTileTypeMapping>(RowName, ContextString, false);
			if (Row)
			{
				TypeColorMap.Add(Row->TileTypeName, FVector4(Row->TypeColor));
			}
		}
	}
}

void AGridVisuals::SetupDynamicMaterials()
{	
	// Setup all the necessary dynamic materials, including the grid and tile highlights
	
	if (!IsCoreValid())
	{
		return;
	}

	// If custom highlight materials are not specified, load default materials.
	// DON'T CHANGE THE FOLLOWING ASSET LOCATIO.

	if (!bCustomHighlightMaterial)
	{
		HexHighlightMaterial = Cast<UMaterialInterface>(StaticLoadObject(UObject::StaticClass(), nullptr, *HighlightPathHex));
		SquareHighlightMaterial = Cast<UMaterialInterface>(StaticLoadObject(UObject::StaticClass(), nullptr, *HighlightPathSq));
	}
	
	// If there's no DMI or if the parent has changed, create a new one.
	if (!SquareHighlightDMI || SquareHighlightDMI->Parent != SquareHighlightMaterial)
	{
		// Load the default Shape Texture.
		// DON'T CHANGE THE FOLLOWING ASSET LOCATION IN THE EDITOR.
		UTexture2D* SquareTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("Texture2D'/GridPlugin/Textures/SquareTile.SquareTile'")));

		SquareHighlightDMI = UMaterialInstanceDynamic::Create(SquareHighlightMaterial, this, FName("SquareHighlightDMI"));
		SquareHighlightDMI->SetScalarParameterValue("Opacity", 1);
		
		if (!bCustomHighlightMaterial)
		{
			SquareHighlightDMI->SetTextureParameterValue("ShapeTexture", SquareTexture);
		}

	}

	if (!HexHighlightDMI || HexHighlightDMI->Parent != HexHighlightMaterial)
	{
		// Load the default Shape Texture.
		// DON'T CHANGE THE FOLLOWING ASSET LOCATION.
		UTexture2D* HexTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("Texture2D'/GridPlugin/Textures/TX_SimpleHex.TX_SimpleHex'")));

		HexHighlightDMI = UMaterialInstanceDynamic::Create(HexHighlightMaterial, this, FName("HexHighlightDMI"));
		HexHighlightDMI->SetScalarParameterValue("Opacity", 1);

		if (!bCustomHighlightMaterial)
		{
			HexHighlightDMI->SetTextureParameterValue("ShapeTexture", HexTexture);
		}	
	}

	// If custom Grid materials are not specified, load default materials.
	// DON'T CHANGE THE FOLLOWING ASSET LOCATION.
	if (!bCustomGridMaterial)
	{
		SquareGridMaterial = Cast<UMaterialInterface>(StaticLoadObject(UObject::StaticClass(), nullptr, *SquareGridMaterialPath));
		HexGridMaterial = Cast<UMaterialInterface>(StaticLoadObject(UObject::StaticClass(), nullptr, *HexGridMaterialPath));
	}

	// If there's no DMI or if the parent has changed, create a new one.
	if (!SquareDMI || SquareDMI->Parent != SquareGridMaterial)
	{
		SquareDMI = UMaterialInstanceDynamic::Create(SquareGridMaterial, this, FName("SquareGridDMI"));
	}

	if (!HexDMI || HexDMI->Parent != HexGridMaterial)
	{
		HexDMI = UMaterialInstanceDynamic::Create(HexGridMaterial, this, FName("HexGridDMI"));
	}
		
	// Once created, set parameters
	bool bIsHex = GridSubsystem->GetIsHex();
	FVector TileCount = FVector(GridSubsystem->TileCount.X, GridSubsystem->TileCount.Y, 0);
	FVector GridSize = FVector(GridSubsystem->GridSize.X, GridSubsystem->GridSize.Y, 0);

	if (SquareGridMaterial && SquareDMI) 
	{
		SquareDMI->SetVectorParameterValue("TileCount", TileCount);
		SquareDMI->SetVectorParameterValue("GridSize", GridSize);
		SquareDMI->SetVectorParameterValue("LineColor", GridSubsystem->LineColor);
		SquareDMI->SetVectorParameterValue("TileColor", GridSubsystem->TileColor);

		SquareDMI->SetScalarParameterValue("LineThickness", GridSubsystem->LineWidth);
		SquareDMI->SetScalarParameterValue("LineOpacity", GridSubsystem->LineOpacity);
		SquareDMI->SetScalarParameterValue("TileOpacity", GridSubsystem->TileOpacity);

		// Adding grid material to post-process component
		GridPostProcess->AddOrUpdateBlendable(SquareDMI, bIsHex ? 0 : 1);
	}
	
	if (HexGridMaterial && HexDMI)
	{
		HexDMI->SetVectorParameterValue("TileCount", TileCount);
		HexDMI->SetVectorParameterValue("GridSize", GridSize);
		HexDMI->SetVectorParameterValue("LineColor", GridSubsystem->LineColor);
		HexDMI->SetVectorParameterValue("TileColor", GridSubsystem->TileColor);

		HexDMI->SetScalarParameterValue("LineThickness", GridSubsystem->LineWidth);
		HexDMI->SetScalarParameterValue("LineOpacity", GridSubsystem->LineOpacity);
		HexDMI->SetScalarParameterValue("TileOpacity", GridSubsystem->TileOpacity);

		// Adding grid material to post-process component
		GridPostProcess->AddOrUpdateBlendable(HexDMI, bIsHex ? 1 : 0);
	}

	// Modify and Mark Dirty for immediate updates
	GridPostProcess->Modify();
	GridPostProcess->MarkRenderStateDirty();
}

void AGridVisuals::SwitchGridShape(EGridShape GridShape)
{
	CHECK_GRIDPOSTPROCESS();
	
	// Clear existing visuals
	DestroyVisuals(EGridVisualContext::Auto);

	// Switch grid shape.
	if (GridShape == EGridShape::Square) {
		GridPostProcess->AddOrUpdateBlendable(SquareDMI, 1);
		GridPostProcess->AddOrUpdateBlendable(HexDMI, 0);
	}

	else 
	{
		GridPostProcess->AddOrUpdateBlendable(SquareDMI, 0);
		GridPostProcess->AddOrUpdateBlendable(HexDMI, 1);
	}
}


void AGridVisuals::LoadAllVisuals(EGridVisualContext Context)
{
	Context = UGridUtils::ResolveGridVisualContext(Context);

	FScopeLock Lock(&GridSubsystem->Mutex);

	if (Context == EGridVisualContext::Runtime)
	{
		UE_LOG(LogTemp, Warning, TEXT("GridVisuals Loaded! Runtime"));

		if (GridPostProcess)
		{
			GridPostProcess->Settings.WeightedBlendables.Array.Empty();
			SetupDynamicMaterials();

			if (TileTypeMapping && TileStateMapping)
			{
				// Preload colors for faster lookup.
				PrecomputeColors();
			}

			// If GridData exists, recalculate the visuals.
			if (!GridSubsystem->GetGridData().IsEmpty())
			{
				ChunkToISMCMapRuntime.Empty();
				TileToInstanceMapRuntime.Empty();
				DestroyVisuals(EGridVisualContext::Runtime);

				// This is the function that handles visual generation. Can be
				// called in runtime with proper context.
				ProcessVisualsAsync(EGridVisualContext::Runtime);
			}
		}
	}

#if WITH_EDITOR
	else if (Context == EGridVisualContext::Editor) 
	{
		// If the context is in editor but not PIE, load visuals. Meant for level loading.
		if (GetWorld() && GetWorld()->IsEditorWorld() && !GetWorld()->IsPlayInEditor())
		{
			UE_LOG(LogTemp, Warning, TEXT("GridVisuals Loaded! Editor"));

			if (GridPostProcess)
			{
				GridPostProcess->Settings.WeightedBlendables.Array.Empty();
				SetupDynamicMaterials();

				if (TileTypeMapping && TileStateMapping)
				{
					// Preload colors for faster lookup.
					PrecomputeColors();
				}

				// If GridData exists, recalculate the visuals.
				if (!GridSubsystem->GetGridData().IsEmpty())
				{
					ChunkToISMCMapEditor.Empty();
					TileToInstanceMapEditor.Empty();
					DestroyVisuals(EGridVisualContext::Editor);

					// This is the function that handles visual generation. Can be
					// called in runtime with proper context.
					ProcessVisualsAsync(EGridVisualContext::Editor);
				}
			}
		}
	}
#endif
	
}


UHierarchicalInstancedStaticMeshComponent* AGridVisuals::GetOrCreateISMCForChunk(FIntPoint ChunkIndex,
	EGridVisualContext Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_GetOrCreateISMCForChunk);

	// Procedurally create or get HISMC from the given ChunkIndex.

	bool bIsHex = GridSubsystem->GetIsHex();
	UHierarchicalInstancedStaticMeshComponent* NewHISMC = nullptr;

	if (Context == EGridVisualContext::Runtime)
	{
		if (UHierarchicalInstancedStaticMeshComponent** FoundHISMC = 
			ChunkToISMCMapRuntime.Find(ChunkIndex))
		{
			return *FoundHISMC; // Return existing HISMC
		}

		// Create a new HISMC
		NewHISMC = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
		NewHISMC->RegisterComponent();
		NewHISMC->SetWorldLocation(FVector(ChunkIndex.X * ChunkSize, ChunkIndex.Y * ChunkSize, 0));
		NewHISMC->NumCustomDataFloats = 4;
		NewHISMC->SetMaterial(0, bIsHex ? HexHighlightDMI : SquareHighlightDMI);
		NewHISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		// Add tags for retrieval.
		NewHISMC->ComponentTags.Add("RuntimeVisuals");
		ChunkToISMCMapRuntime.Add(ChunkIndex, NewHISMC);
	}

#if WITH_EDITOR
	else if (Context == EGridVisualContext::Editor)
	{
		if (UHierarchicalInstancedStaticMeshComponent** FoundHISMC = 
			ChunkToISMCMapEditor.Find(ChunkIndex))
		{
			return *FoundHISMC; // Return existing HISMC
		}

		NewHISMC = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);

		// Register the component with the world
		NewHISMC->RegisterComponent();
		NewHISMC->SetWorldLocation(FVector(ChunkIndex.X * ChunkSize, ChunkIndex.Y * ChunkSize, 0));
		NewHISMC->NumCustomDataFloats = 6;
		NewHISMC->SetMaterial(0, bIsHex ? HexHighlightDMI : SquareHighlightDMI);
		NewHISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Add tags for retrieval.
		NewHISMC->ComponentTags.Add("EditorVisuals");
		NewHISMC->SetIsVisualizationComponent(true);

		ChunkToISMCMapEditor.Add(ChunkIndex, NewHISMC);
	}
#endif

	if (TileMesh)
	{
		NewHISMC->SetStaticMesh(TileMesh);
	}

	return NewHISMC;
}

void AGridVisuals::SetInstanceCulling(EGridVisualContext Context, float StartCullDistance,
float EndCullDistance)
{
	// Dynamically set culling distances for all HISMCs of given context.
	// Good for switching between eye level and top down perspectives.
	// To turn off, set culling to 0.

	Context = UGridUtils::ResolveGridVisualContext(Context);

	TArray<UHierarchicalInstancedStaticMeshComponent*> FoundComponents;
	GetComponents<UHierarchicalInstancedStaticMeshComponent>(FoundComponents);
	FName Tag = "RuntimeVisuals";

	if (Context == EGridVisualContext::Runtime)
	{
		Tag = "RuntimeVisuals";
	}

#if WITH_EDITOR
	else if (Context == EGridVisualContext::Editor)
	{
		Tag = "EditorVisuals";
	}
#endif

	for (UActorComponent* Component : FoundComponents)
	{
		if (Component->ComponentHasTag(Tag))
		{
			if (UHierarchicalInstancedStaticMeshComponent* HISMC = Cast<UHierarchicalInstancedStaticMeshComponent>(Component))
			{
				if (Component->ComponentHasTag(Tag))
				{
					// Set culling distances
					HISMC->InstanceStartCullDistance = StartCullDistance;
					HISMC->InstanceEndCullDistance = EndCullDistance;
				}

				HISMC->MarkRenderStateDirty();
			}
		}
	}
}

bool AGridVisuals::GetInstanceInfoForTile(FIntPoint TileIndex, FTileInstanceInfo& OutInfo, 
	TMap<FIntPoint, FTileInstanceInfo>& InTileToInstanceMap)
{	
	// Returns TileInstanceInfo for the given tile index.
	// TileInstanceInfo contains which chunk the tile belongs to, 
	// its instance index within that chunk (HISMC).

	QUICK_SCOPE_CYCLE_COUNTER(STAT_GetInstanceInfoForTile);

	if (FTileInstanceInfo* FoundInfo = InTileToInstanceMap.Find(TileIndex))
	{
		OutInfo = *FoundInfo;
		return true;
	}
	return false; // Tile not found
}

FIntPoint AGridVisuals::GetChunkFromTileIndex(const FIntPoint& TileIndex)
{
	// Procedurally generate the ChunkIndex the given tile belongs to.
	
	int32 ChunkX = (TileIndex.X >= 0) ? (TileIndex.X / ChunkSize) : ((TileIndex.X - ChunkSize + 1) / ChunkSize);
	int32 ChunkY = (TileIndex.Y >= 0) ? (TileIndex.Y / ChunkSize) : ((TileIndex.Y - ChunkSize + 1) / ChunkSize);

	return FIntPoint(ChunkX, ChunkY);
}

#if WITH_EDITOR
void AGridVisuals::AddTileVisualEditor(const FIntPoint& Index)
{
	// Adds visual to the given tile index. EDITOR USE ONLY.
	
	CHECK_GRIDPOSTPROCESS(); // Check if post-process component is valid.
	
	QUICK_SCOPE_CYCLE_COUNTER(STAT_AddTileVisualEditor);
	
	// Not locking here because the function is usually called in a broader context.
	//FScopeLock Lock(&GridSubsystem->Mutex);

	if (!IsCoreValid())
	{
		GridSubsystem = UGridUtils::GetGridSubsystem();
	}

	World = GEditor->GetEditorWorldContext().World();

	FVector2D GridSize = GridSubsystem->GetGridSize();
	FVector2D TileCount = GridSubsystem->GetTileCount();

	bool bIsHex = GridSubsystem->GetIsHex();

	// Change GridSize based on shape.
	GridSize = bIsHex ? GridSize * 2 : GridSize;

	// NOTE: Instance locations depends entirely on precompute.
	FTileData TileData = GridSubsystem->GetTileData(Index);
	FVector TileLocation = TileData.TileTransform.GetLocation();
	FVector TileScale = TileData.TileTransform.GetScale3D();

	float HexWidthScale = (GridSize.X / 100) * 0.866; 
	float HexHeightScale = (GridSize.Y / 100) * 0.866;

	// HISMC instance scale
	TileScale = bIsHex ? FVector(HexWidthScale, HexHeightScale, TileScale.Z) : FVector(GridSize.X / SquareTileScaleFactor.X, GridSize.Y / SquareTileScaleFactor.Y, TileScale.Z);

	// Update the highlight materials' scale
	if (SquareHighlightDMI && HexHighlightDMI)
	{
		SquareHighlightDMI->SetVectorParameterValue("Scale", FVector(GridSize.X * SQHighlightScaleMultiplier,
			GridSize.Y * SQHighlightScaleMultiplier, 0));
		HexHighlightDMI->SetVectorParameterValue("Scale", FVector(GridSize.X * HexHighlightScaleMultiplier,
			GridSize.Y * HexHighlightScaleMultiplier, 0));
	}

	FTransform Transform(FRotator(0), TileLocation, TileScale);
	int32 InstanceIndex;

	// Get Color for the given tile index.
	FVector4 Color = GetColorFromType(TileData.TileType);

	// Get the chunk index for the tile.
	FIntPoint ChunkIndex = GetChunkFromTileIndex(Index);

	// Get or create the ISMC for this chunk
	UHierarchicalInstancedStaticMeshComponent* HISMC = GetOrCreateISMCForChunk(ChunkIndex,
		EGridVisualContext::Editor);

	if (!HISMC)
	{
		return;
	}

	FTileInstanceInfo Info;

	// If tile doesn't already exists within a chunk, create a new one.
	if (!GetInstanceInfoForTile(Index, Info,
		TileToInstanceMapEditor))
	{
		InstanceIndex = HISMC->AddInstance(Transform, true);

		if (HISMC->IsValidInstance(InstanceIndex)) {

			HISMC->SetCustomDataValue(InstanceIndex, 0, Color.X);
			HISMC->SetCustomDataValue(InstanceIndex, 1, Color.Y);
			HISMC->SetCustomDataValue(InstanceIndex, 2, Color.Z);
			HISMC->SetCustomDataValue(InstanceIndex, 3, Color.W);

			TileToInstanceMapEditor
				.Add(Index, FTileInstanceInfo(HISMC, InstanceIndex));

			HISMC->MarkRenderStateDirty();
		}
	}

	// Else just update the existing chunk.
	else
	{
		HISMC->SetCustomDataValue(Info.InstanceIndex, 0, Color.X);
		HISMC->SetCustomDataValue(Info.InstanceIndex, 1, Color.Y);
		HISMC->SetCustomDataValue(Info.InstanceIndex, 2, Color.Z);
		HISMC->SetCustomDataValue(Info.InstanceIndex, 3, Color.W);

		HISMC->MarkRenderStateDirty();
	}
}
#endif

void AGridVisuals::AddTileVisualRuntime(const FIntPoint& Index)
{
	// Adds visual to the given tile index.

	CHECK_GRIDPOSTPROCESS(); // Check if post-process component is valid.

	QUICK_SCOPE_CYCLE_COUNTER(STAT_AddTileVisualRuntime);

	// Not locking here because the function is usually called in a broader context.
	//FScopeLock Lock(&GridSubsystem->Mutex);

	if (!IsCoreValid())
	{
		return;
	}

	FTileData TileData = GridSubsystem->GetTileData(Index);
	FVector2D GridSize = GridSubsystem->GetGridSize();
	FVector2D TileCount = GridSubsystem->GetTileCount();
	
	// Change GridSize based on shape.
	bool bIsHex = GridSubsystem->bIsHex;
	GridSize = bIsHex ? GridSize * 2 : GridSize;
	
	// NOTE: Instance locations depends entirely on precompute.
	FVector TileLocation = TileData.TileTransform.GetLocation();
	FVector TileScale = TileData.TileTransform.GetScale3D();

	float HexWidthScale = (GridSize.X / 100) * 0.866; // Scaling for the width
	float HexHeightScale = (GridSize.Y / 100) * 0.866; // Height is scaled by sqrt(3)/2
	
	// HISMC instance scale
	TileScale = bIsHex ? FVector(HexWidthScale, HexHeightScale, TileScale.Z) : FVector(GridSize.X / SquareTileScaleFactor.X, GridSize.Y / SquareTileScaleFactor.Y, TileScale.Z);
	
	// Update the highlight materials' scale
	if (SquareHighlightDMI && HexHighlightDMI)
	{
		SquareHighlightDMI->SetVectorParameterValue("Scale", FVector(GridSize.X * SQHighlightScaleMultiplier,
			GridSize.Y * SQHighlightScaleMultiplier, 0));
		HexHighlightDMI->SetVectorParameterValue("Scale", FVector(GridSize.X * HexHighlightScaleMultiplier,
			GridSize.Y * HexHighlightScaleMultiplier, 0));
	}

	FTransform Transform(FRotator(0), TileLocation, TileScale);
	
	// Get Color for the given tile index. Taking states and types into account.
	FVector4 Color = GetColorFromState(TileData);
	
	// Get the chunk index for the tile.
	FIntPoint ChunkIndex = GetChunkFromTileIndex(Index);

	// Get or create the ISMC for this chunk
	UHierarchicalInstancedStaticMeshComponent* HISMC = GetOrCreateISMCForChunk(ChunkIndex,
		EGridVisualContext::Runtime);

	FTileInstanceInfo Info;

	// If tile doesn't already exists within a chunk, create a new one.
	if (!GetInstanceInfoForTile(Index, Info, TileToInstanceMapRuntime))
	{
		int32 InstanceIndex = HISMC->AddInstance(Transform, true);

		HISMC->SetCustomDataValue(InstanceIndex, 0, Color.X);
		HISMC->SetCustomDataValue(InstanceIndex, 1, Color.Y);
		HISMC->SetCustomDataValue(InstanceIndex, 2, Color.Z);
		HISMC->SetCustomDataValue(InstanceIndex, 3, Color.W);

		TileToInstanceMapRuntime
			.Add(Index, FTileInstanceInfo(HISMC, InstanceIndex));

		HISMC->MarkRenderStateDirty();
	}

	else
	{
		// Else just update the existing chunk.
		HISMC->SetCustomDataValue(Info.InstanceIndex, 0, Color.X);
		HISMC->SetCustomDataValue(Info.InstanceIndex, 1, Color.Y);
		HISMC->SetCustomDataValue(Info.InstanceIndex, 2, Color.Z);
		HISMC->SetCustomDataValue(Info.InstanceIndex, 3, Color.W);
		
		HISMC->MarkRenderStateDirty();
	}
}

void AGridVisuals::UpdateVisualTransform(const FIntPoint& TileIndex, FTransform InTransform, EGridVisualContext Context)
{
	if (!IsCoreValid())
	{
		return;
	}

	Context = UGridUtils::ResolveGridVisualContext(Context);

	// Get the chunk index for the tile.
	FIntPoint ChunkIndex = GetChunkFromTileIndex(TileIndex);

	// Get or create the ISMC for this chunk
	UHierarchicalInstancedStaticMeshComponent* HISMC = GetOrCreateISMCForChunk(ChunkIndex,
		Context);

	FTileInstanceInfo Info;

	if (Context == EGridVisualContext::Editor)
	{
		if (GetInstanceInfoForTile(TileIndex, Info, TileToInstanceMapEditor))
		{
			HISMC->UpdateInstanceTransform(Info.InstanceIndex, InTransform, true, true, false);
			HISMC->MarkRenderStateDirty();
		}
	}
	else {

		if (GetInstanceInfoForTile(TileIndex, Info, TileToInstanceMapRuntime))
		{
			HISMC->UpdateInstanceTransform(Info.InstanceIndex, InTransform, true, true, false);
			HISMC->MarkRenderStateDirty();
		}
	}
	
}

void AGridVisuals::DestroyVisuals(EGridVisualContext Context)
{
	// Destroys all HISMCs of the given context.

	TArray<UHierarchicalInstancedStaticMeshComponent*> FoundComponents;
	GetComponents<UHierarchicalInstancedStaticMeshComponent>(FoundComponents);

	FName Tag = "RuntimeVisuals";

	if (Context == EGridVisualContext::Runtime)
	{
		Tag = "RuntimeVisuals";
	}

#if WITH_EDITOR
	else if (Context == EGridVisualContext::Editor)
	{
		Tag = "EditorVisuals";
	}
#endif

	for (UActorComponent* Component : FoundComponents)
	{
		
		if (Component->ComponentHasTag(Tag))
		{
			if (UHierarchicalInstancedStaticMeshComponent* HISMC = Cast<UHierarchicalInstancedStaticMeshComponent>(Component))
			{
				// Properly detach and destroy the component
				HISMC->DestroyComponent();
			}
		}
		
	}
	
	// After destroying each component, the related data
	// is also wiped.
	if (Context == EGridVisualContext::Runtime)
	{
		TileToInstanceMapRuntime.Empty();
		ChunkToISMCMapRuntime.Empty();
	}

#if WITH_EDITOR
	else
	{
		TileToInstanceMapEditor.Empty();
		ChunkToISMCMapEditor.Empty();
	}
#endif

}

void AGridVisuals::SetVisualsVisibility(EGridVisualContext Context, bool Visibility)
{
	// Toggles the visibility of all HISMCs of the given context.
	
	Context = UGridUtils::ResolveGridVisualContext(Context);

	if (Context == EGridVisualContext::Runtime) 
	{
		if (!ChunkToISMCMapRuntime.IsEmpty())
		{
			for (auto& ChunkPair : ChunkToISMCMapRuntime)
			{
				UHierarchicalInstancedStaticMeshComponent* ChunkHISMC = ChunkPair.Value;
				if (ChunkHISMC)
				{
					ChunkHISMC->SetVisibility(Visibility, true);
					ChunkHISMC->bHiddenInGame = !Visibility;
				}
			}
		}
	}

#if WITH_EDITOR

	else if(Context == EGridVisualContext::Editor)
	{
		if (!ChunkToISMCMapEditor.IsEmpty())
		{
			for (auto& ChunkPair : ChunkToISMCMapEditor)
			{
				UHierarchicalInstancedStaticMeshComponent* ChunkHISMC = ChunkPair.Value;
				if (ChunkHISMC)
				{
					ChunkHISMC->SetVisibility(Visibility, false);
					ChunkHISMC->bHiddenInGame = !Visibility;
				}
			}
		}
	}

#endif

}

void AGridVisuals::ProcessVisualsAsync(EGridVisualContext Context)
{
	// Process visuals of the entire grid async.
	
	FScopeLock Lock(&GridSubsystem->Mutex);

	static TArray<FIntPoint> GridKeys;
	static int32 CurrentIndex = 0;
	const int32 BatchSize = 50; // Size of tiles to process at a time.

	// Initialize the array of keys only on the first call
	if (GridKeys.Num() == 0)
	{
		GridSubsystem->GetGridData().GenerateKeyArray(GridKeys);
		CurrentIndex = 0;
	}

	// Process a batch of tiles
	AsyncTask(ENamedThreads::GameThread, [this, BatchSize, Context]()
		{
			int32 ProcessedTiles = 0;

			while (ProcessedTiles < BatchSize && CurrentIndex < GridKeys.Num())
			{
				const FIntPoint& Key = GridKeys[CurrentIndex];
				
				if (Context == EGridVisualContext::Runtime)
				{
					AddTileVisualRuntime(Key);
				}

#if WITH_EDITOR

				else if(Context == EGridVisualContext::Editor)
				{
					AddTileVisualEditor(Key);
				}
#endif				
				
				CurrentIndex++;
				ProcessedTiles++;
			}

			// If there are more tiles to process, schedule the next batch
			if (CurrentIndex < GridKeys.Num())
			{
				ProcessVisualsAsync(Context);
			}
			else
			{
				// Reset for future processing
				GridKeys.Empty();
				CurrentIndex = 0;
			}
		});

}

FVector4 AGridVisuals::GetColorFromType(FName TileType)
{
	// Returns the color for the given type.
	
	FVector4 Color = FVector4(0, 0, 0, 0);

	if (TypesToNotRender.Contains(TileType))
	{
		return FVector4(0, 0, 0, 0);
	}

	if (TileTypeMapping)
	{
		FVector4* RowCol = TypeColorMap.Find(TileType);
		
		if (RowCol)
		{
			Color = *RowCol;
		}
	}

	return Color;
}

FVector4 AGridVisuals::GetColorFromState(FTileData TileData)
{
	// Returns the overall color for the given tile.
	// States take priority over types and both are considered
	// depending on the situation.
	
	// Base color is set as the type color.
	FVector4 BaseColor = GetColorFromType(TileData.TileType);
	FVector4 Color = FVector4(0, 0, 0, 0);
	TArray<FName> States = TileData.TileState;

	FName ChosenState = "None";
	bool bFoundMatch = false;

	// If there are no states, return BaseColor.
	if (States.IsEmpty())
	{
		Color = BaseColor;
		return Color;
	}

	// If there's only one state...
	else if (States.Num() == 1)
	{
		// if it's None, return BaseColor;
		if (States[0] == "None")
		{
			Color = BaseColor;
			return Color;
		}

		// otherwise set the state as ChosenState
		else 
		{
			ChosenState = States[0];
		}

	}

	// If multiple states exist, choose the state
	// according to the state priority list.
	else if (States.Num() > 1)
	{
		if (StatePriority.IsEmpty())
		{
			return Color;
		}
		
		for (FName Priority : StatePriority)
		{
			if (bFoundMatch) break;

			for (FName State : States)
			{
				if (Priority == State)
				{
					ChosenState = Priority;
					bFoundMatch = true; 
					break;
				}
			}
		}
	}

	// If the chosen state has been flagged, then return BaseColor.
	if (StateToNotRender.Contains(ChosenState))
	{
		return BaseColor;
	}

	if (TileStateMapping)
	{
		FVector4* RowCol = StateColorMap.Find(ChosenState);
		
		if (RowCol)
		{
			Color = *RowCol;
		}
	}

	// Fake transparency blending.
	float Alpha = Color.W / 1.2;

	// Perform alpha blending between the base color and the chosen color.
	FVector4 FinalColor;
	FinalColor.X = FMath::Lerp(BaseColor.X, Color.X, Alpha);
	FinalColor.Y = FMath::Lerp(BaseColor.Y, Color.Y, Alpha);
	FinalColor.Z = FMath::Lerp(BaseColor.Z, Color.Z, Alpha);

	// Choose the bigger alpha.
	FinalColor.W = Color.W > BaseColor.W ? Color.W : BaseColor.W;
	
	return FinalColor;
}

void AGridVisuals::SetGridSize(FVector2D GridSize)
{
	// Sets the vector parameter in the DMI

	if (IsValid(SquareDMI))
	{
		SquareDMI->SetVectorParameterValue("GridSize", FVector(GridSize.X, GridSize.Y, 0));
	}

	if (IsValid(HexDMI))
	{
		HexDMI->SetVectorParameterValue("GridSize", FVector(GridSize.X, GridSize.Y, 0));
	}

	GridPostProcess->MarkRenderStateDirty();
}

void AGridVisuals::SetTileCount(FVector2D TileCount)
{
	// Sets the vector parameter in the DMI
	
	if (!IsValid(SquareDMI) || !IsValid(HexDMI) || !IsCoreValid())
	{
		return;
	}

	SquareDMI->SetVectorParameterValue("TileCount", FVector(TileCount.X, TileCount.Y, 0));
	HexDMI->SetVectorParameterValue("TileCount", FVector(TileCount.X, TileCount.Y, 0));

	GridPostProcess->MarkRenderStateDirty();

	FVector2D CurrentOffset = UGridUtils::GetGridOffset();
	FVector2D CurrentSize = UGridUtils::GetGridSize();

	// Calculate offset based on even/odd tile counts
	float OffsetX = (int(TileCount.X) % 2 == 0) ? 0.5f * CurrentSize.X : 0.0f;
	float OffsetY = (int(TileCount.Y) % 2 == 0) ? 0.5f * CurrentSize.Y : 0.0f;

	bool bIsHex = GridSubsystem->GetIsHex();

	if (!bIsHex)
	{
		// Set the offset using the calculated values
		GridSubsystem->SetGridOffset(FVector2D(OffsetX, OffsetY));
	}

}

void AGridVisuals::SetGridOffset(float Offset, FVector2D GridOffset, bool bIsX)
{	
	// Sets the vector parameter in the DMI

	if (bIsX) {
		GridOffset = FVector2D(Offset, GridOffset.Y);		
	}
	else
	{
		GridOffset = FVector2D(GridOffset.X, Offset);		
	}
}

void AGridVisuals::SetLineWidth(float Width)
{
	// Sets the scalar parameter in the DMI
	
	if (IsValid(SquareDMI))
	{
		SquareDMI->SetScalarParameterValue("LineThickness", Width);
	}

	if (IsValid(HexDMI))
	{
		HexDMI->SetScalarParameterValue("LineThickness", Width);
	}
}

void AGridVisuals::SetColor(FLinearColor Color, bool bIsLine)
{
	// Sets the vector parameter in the DMI
	
	if (!IsValid(SquareDMI) || !IsValid(HexDMI))
	{
		return;
	}

	if (bIsLine)
	{
		SquareDMI->SetVectorParameterValue("LineColor", Color);
		HexDMI->SetVectorParameterValue("LineColor", Color);
	}
	else 
	{
		SquareDMI->SetVectorParameterValue("TileColor", Color);
		HexDMI->SetVectorParameterValue("TileColor", Color);
	}
	
}

void AGridVisuals::SetOpacity(float Value, bool bIsLine)
{
	// Sets the scalar parameter in the DMI
	
	if (!IsValid(SquareDMI) || !IsValid(HexDMI))
	{
		return;
	}

	if (bIsLine) 
	{
		SquareDMI->SetScalarParameterValue("LineOpacity", Value);
		HexDMI->SetScalarParameterValue("LineOpacity", Value);
	}
	else
	{
		SquareDMI->SetScalarParameterValue("TileOpacity", Value);
		HexDMI->SetScalarParameterValue("TileOpacity", Value);
	}
}