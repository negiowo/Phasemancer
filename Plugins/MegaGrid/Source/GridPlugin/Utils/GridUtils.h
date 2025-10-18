// Copyright Two Bit Studios 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GridDataStructures.h"
#include "GridUtils.generated.h"

/*
* Utilities class that provides many
* helper functions. Called in blueprint via GridBlueprintLibrary.
*/

class UGridSubsystem;

UCLASS(Blueprintable)
class GRIDPLUGIN_API UGridUtils : public UObject
{
	GENERATED_BODY()

public:

	UGridUtils();
	
public:
	/// <summary>
	/// Cached GridSubsystem for efficient retrieval.
	/// </summary>
	static UGridSubsystem* CachedGridSubsystem;

	/// <summary>
	/// Returns a new or cached GridSubsystem.
	/// </summary>
	/// <returns></returns>
	UFUNCTION(BlueprintPure, Category = "Global Grid Data",
	meta = (ToolTip = "Returns GridSubsystem."))
	static UGridSubsystem* GetGridSubsystem();

// Global Variables
public:

	/// <summary>
	/// Returns current grid size.
	/// </summary>
	/// <returns></returns>
	UFUNCTION(BlueprintPure, Category = "Global Grid Data",
		meta = (ToolTip = "Returns current grid size."))
	static FVector2D GetGridSize();

	/// <summary>
	/// Returns current bIsHex.
	/// </summary>
	/// <returns></returns>
	UFUNCTION(BlueprintPure, Category = "Global Grid Data",
		meta = (ToolTip = "Returns current bIsHex."))
	static bool GetIsHex();

	/// <summary>
	/// Returns the current tile count.
	/// </summary>
	/// <returns></returns>
	UFUNCTION(BlueprintPure, Category = "Global Grid Data",
		meta = (ToolTip = "Returns current tile count."))
	static FIntPoint GetTileCount();

	/// <summary>
	/// Returns tile opacity.
	/// </summary>
	/// <returns></returns>
	UFUNCTION(BlueprintPure, Category = "Global Grid Data",
		meta = (ToolTip = "Returns current tile opacity."))
	static float GetTileOpacity();

	/// <summary>
	/// Returns line opacity.
	/// </summary>
	/// <returns></returns>
	UFUNCTION(BlueprintPure, Category = "Global Grid Data",
		meta = (ToolTip = "Returns current line opacity."))
	static float GetLineOpacity();

	/// <summary>
	/// Returns line width.
	/// </summary>
	/// <returns></returns>
	UFUNCTION(BlueprintPure, Category = "Global Grid Data",
		meta = (ToolTip = "Returns current line width."))
	static float GetLineWidth();

	/// <summary>
	/// Returns line color.
	/// </summary>
	/// <returns></returns>
	UFUNCTION(BlueprintPure, Category = "Global Grid Data",
		meta = (ToolTip = "Returns current line color."))
	static FLinearColor GetLineColor();

	/// <summary>
	/// Returns tile color.
	/// </summary>
	/// <returns></returns>
	UFUNCTION(BlueprintPure, Category = "Global Grid Data",
		meta = (ToolTip = "Returns current tile color."))
	static FLinearColor GetTileColor();

	/// <summary>
	/// Returns the current grid offset.
	/// </summary>
	/// <returns></returns>
	UFUNCTION(BlueprintPure, Category = "Global Grid Data",
		meta = (ToolTip = "Returns current grid offset."))
	static FVector2D GetGridOffset();

public:
	
	/// <summary>
	/// Returns the tile index from given location.
	/// </summary>
	/// <param name="Location"></param>
	/// <param name="bIsHexagon"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Utilities",
		meta = (ToolTip = "Returns tile index for given location."))
	static FIntPoint GetIndexFromLocation(const FVector& Location, bool bIsHexagon);
	
	/// <summary>
	/// Returns the location of the given tile index.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <param name="bIsHexagon"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Utilities",
		meta = (ToolTip = "Returns location for given tile index."))
	static FVector GetLocationFromIndex(const FIntPoint& TileIndex, bool bIsHexagon);
	
	/// <summary>
	/// Returns the surface location for the given location.
	/// Hits anything on the GridSurface channel.
	/// </summary>
	/// <param name="WorldContextObject"></param>
	/// <param name="Location"></param>
	/// <param name="IgnoredObject"></param>
	/// <param name="bOutHit"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Utilities", 
		meta = (WorldContext = "WorldContextObject"), 
		meta = (ToolTip = "Returns surface location."))
	static FVector GetSurfaceLocation(UObject* WorldContextObject, const FVector& Location, 
		UObject* IgnoredObject, bool& bOutHit);

	/// <summary>
	/// Gets the tile under cursor.
	/// </summary>
	/// <param name="WorldContextObject"></param>
	/// <param name="PlayerController"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintPure, Category = "Grid Utilities", 
		meta = (WorldContext = "WorldContextObject"), 
		meta = (ToolTip = "Returns tile index under cursor."))
	static FIntPoint GetTileUnderCursor(UObject* WorldContextObject,
		APlayerController* PlayerController);

	/// <summary>
	/// Snaps given vector to square grid.
	/// </summary>
	/// <param name="Vector"></param>
	/// <param name="Grid"></param>
	/// <param name="Offset"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Utilities",
		meta = (ToolTip = "Snaps vector to square grid. DEPRECATED. See SnapLocationToGrid."))
	static FVector SnapVectorToGrid(const FVector& Vector, const FVector2D& Grid,
		const FVector& Offset);

	/// <summary>
	/// Snaps location to grid.
	/// </summary>
	/// <param name="WorldContextObject"></param>
	/// <param name="Location"></param>
	/// <param name="LimitToGrid"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Utilities", 
		meta = (ToolTip = "Returns location snapped to grid."))
	static FVector SnapLocationToGrid(const FVector& Location,
		bool LimitToGrid);

public:

	/// <summary>
	/// Checks if the given tile contains the given state.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <param name="State"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Utilities",
		meta = (ToolTip = "Checks if the tile contains the given state."))
	static bool IsTileOfState(FIntPoint TileIndex, FName State);

	/// <summary>
	/// Checks if the given tile is of given type.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <param name="Type"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Utilities",
		meta = (ToolTip = "Checks if the tile is of the given type."))
	static bool IsTileOfType(FIntPoint TileIndex, FName Type);

	/// <summary>
	/// Checks if int is even.
	/// </summary>
	/// <param name="Num"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Utilities", 
		meta = (ToolTip = "Is the given number even?"))
	static bool IsEven(const int& Num);

	/// <summary>
	/// Checks if the given tile is within the grid boundary.
	/// Requires precomputed Start and End index for hex grid.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <param name="bIsHex"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Utilities", 
		meta = (ToolTip = "Is the given tile index within grid boundaries?"))
	static bool IsTileWithinBounds(FIntPoint TileIndex, bool bIsHex);

public:

	// Hex Helpers

	/// <summary>
	/// Convert Doubled Hex coordinates to axial coordinates.
	/// </summary>
	/// <param name="Index"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Hex Helpers",
		meta = (ToolTip = "Convert Doubled Hex coordinates to axial coordinates."))
	static FIntPoint DoubleToAxial(FIntPoint Index);

	/// <summary>
	/// Convert Axial Hex coordinates to doubled hex coordinates.
	/// </summary>
	/// <param name="Axial"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Hex Helpers",
		meta = (ToolTip = "Convert Axial Hex coordinates to doubled hex coordinates."))
	static FIntPoint AxialToDouble(FIntPoint Axial);

	/// <summary>
	/// Convert given location to Axial Hex Coordinates.
	/// </summary>
	/// <param name="Location"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Hex Helpers",
		meta = (ToolTip = "Convert given location to Axial Hex Coordinates."))
	static FIntPoint ConvertLocationToAxial(FVector Location);

	/// <summary>
	/// Converts given Axial Coordinates to Cube Coordinates.
	/// </summary>
	/// <param name="Axial"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Hex Helpers",
		meta = (ToolTip = "Converts given Axial Coordinates to Cube Coordinates."))
	static FVector AxialToCube(FIntPoint Axial);

	/// <summary>
	/// Converts given Cube Coordinates to Axial Coordinates.
	/// </summary>
	/// <param name="Cube"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Hex Helpers",
		meta = (ToolTip = "Converts given Cube Coordinates to Axial Coordinates."))
	static FIntPoint CubeToAxial(FVector Cube);

	/// <summary>
	/// Get tile indices under the affected brush area.
	/// </summary>
	/// <param name="CursorIndex"></param>
	/// <param name="BrushSize"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Hex Helpers",
		meta = (ToolTip = "Get tile indices under the affected brush area."))
	static TArray<FIntPoint> GetIndicesUnderBrush(FIntPoint CursorIndex, int32 BrushSize, FIntPoint Offset);

public:

	/// <summary>
	/// Get neighboring tile indices.
	/// </summary>
	/// <param name="TileIndex"></param>
	/// <param name="bIsHex"></param>
	/// <param name="bUseDiagonals"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Utilities",
		meta = (ToolTip = "Get neighboring tile indices of given tile index."))
	static TArray<FIntPoint> GetNeighboringIndices(FIntPoint TileIndex, bool bIsHex, bool bUseDiagonals);

	/// <summary>
	/// Resolves grid visual context. 
	/// Sets to Editor mode if bIsRuntime is false.
	/// </summary>
	/// <param name="Context"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Utilities", 
		meta = (ToolTip = "Resolves grid visual context."))
	static EGridVisualContext ResolveGridVisualContext(EGridVisualContext Context);

	/// <summary>
	/// Returns a scaling factor [Min, Max] based on the surface hit normal.
	/// </summary>
	/// <param name="Normal"></param>
	/// <param name="Min"></param>
	/// <param name="Max"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid Utilities",
		meta = (ToolTip = "Returns a scaling factor [Min, Max] based on the normal."))
	static float GetScaleBasedOnNormal(const FVector& Normal, const float Min,
		const float Max);
};
