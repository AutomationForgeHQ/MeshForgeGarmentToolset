// The Garment Studio, for agents that can see.

#pragma once

#include "CoreMinimal.h"
#include "ToolsetRegistry/UToolsetRegistry.h"
#include "ToolsetRegistry/ToolCallAsyncResult.h"
#include "MeshForgeGarmentToolset.generated.h"

/** Where the camera stands for a capture, around the character as it faces. Left and right are the character's own. */
UENUM(BlueprintType)
enum class EGarmentStudioCamera : uint8
{
	Front,
	Back,
	Left,
	Right,
	FrontLeft,
	FrontRight,
	/** From above and a little in front: a collar, the shoulders' line. */
	Top,
};

/** Who gets the mouse in the studio, and which viewport a capture shows. */
UENUM(BlueprintType)
enum class EGarmentStudioMode : uint8
{
	/** The body as you bent it, the garment where you placed it, before any wrap. */
	PoseAndPlace,
	/** The same scene, with the garment ready to sculpt before the wrap. */
	Sculpt,
	/** The last wrap, on the body in its reference pose, where the fit leaves it. */
	Wrap,
};

/** What a stroke does. */
UENUM(BlueprintType)
enum class EGarmentStudioBrush : uint8
{
	/** Epic's Vertex Sculpt with the brush a person last chose - Move by default: drags the surface along the stroke. */
	Sculpt,
	/** Relaxes the whole garment at once. Strokes are ignored; it shrinks a garment a little at its default strength. */
	Smooth,
};

/** One joint that can be turned. */
USTRUCT(BlueprintType)
struct MESHFORGEGARMENTTOOLSET_API FGarmentStudioJointInfo
{
	GENERATED_BODY()

	/** The skeleton's own name: upperarm_l, lowerarm_r, thigh_l. What Turn Garment Studio Joint takes. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	FName Bone;

	/** For a person: "Left shoulder". */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	FString Label;

	/** Where the joint is now, in the character's space, centimetres. Z is up. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	FVector Location = FVector::ZeroVector;

	/** How far it is turned from the reference pose, in the character's space, degrees. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	FRotator Turned = FRotator::ZeroRotator;
};

/** Where the studio stands. Every field is read from the studio as it is, never assumed. */
USTRUCT(BlueprintType)
struct MESHFORGEGARMENTTOOLSET_API FGarmentStudioState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	bool bOpen = false;

	/** PoseAndPlace, Sculpt or Wrap. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	FString Mode;

	/** The garment in the character's space, centimetres. Z is up; the character stands at the origin. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	FVector GarmentLocation = FVector::ZeroVector;

	/** The garment's turn about the vertical, degrees. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	float GarmentYawDegrees = 0.f;

	/** The garment's uniform scale. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	float GarmentScale = 1.f;

	/** The joints that can be turned, where they are and how far they are turned. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	TArray<FGarmentStudioJointInfo> Joints;

	/** Vertices the sculpt before the wrap has moved from the garment as it came, as last kept. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	int32 SculptedVertices = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	int32 GarmentVertices = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	bool bWrapping = false;

	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	bool bHasWrap = false;

	/** The wrap on screen predates a change to the pose, the placement, the sculpt or the settings. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	bool bWrapOutOfDate = false;

	/** The wrap was sculpted after the fit left it; wrapping again replaces that. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	bool bSculptedAfterWrap = false;

	/** The fit's own sentence about the last wrap: clearance, what it moved, how much is still inside the body. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	FString WrapSummary;

	/** Why the last wrap failed, when it did. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	FString WrapError;

	/** A garment was finished here and a chain run hands it on. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	bool bFinished = false;

	/** Finished on the garment the step now receives, so a chain run hands it on without opening the studio. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	bool bFinishedOnThisGarment = false;

	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	FString FinishedMesh;

	/** Changes not saved onto the step yet. Save Garment Studio or Finish Garment Studio keeps them. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	bool bUnsaved = false;

	/** The studio's own sentence about all of the above. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	FString Description;
};

/** A picture of the studio, written to disk for you to look at. */
USTRUCT(BlueprintType)
struct MESHFORGEGARMENTTOOLSET_API FGarmentStudioCapture
{
	GENERATED_BODY()

	/** Absolute path of the PNG. Open it with your own file or image reader. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	FString ImagePath;

	/** Which mode's viewport it shows. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	FString Mode;

	/** The studio as it was when the picture was taken. */
	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	FGarmentStudioState State;
};

/** One drag across the last capture, in 0..1 from its top-left corner. */
USTRUCT(BlueprintType)
struct MESHFORGEGARMENTTOOLSET_API FGarmentStudioStroke
{
	GENERATED_BODY()

	/** Where the drag starts. It must start on the garment, or nothing is sculpted and the tool says so. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garment Studio", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float FromX = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garment Studio", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float FromY = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garment Studio", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float ToX = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garment Studio", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float ToY = 0.5f;
};

/** One setting of the fit, by the name Get Garment Fit Settings lists. */
USTRUCT(BlueprintType)
struct MESHFORGEGARMENTTOOLSET_API FGarmentFitSetting
{
	GENERATED_BODY()

	/** FitStyle, ClearanceCm, Mode, GarmentType, ShapePreservation... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garment Studio")
	FString Name;

	/** As text: Loose, 2.5, Fit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garment Studio")
	FString Value;
};

/** A wrap, completed. */
UCLASS(BlueprintType)
class MESHFORGEGARMENTTOOLSET_API UToolCallAsyncResultGarmentStudio : public UToolCallAsyncResult
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Garment Studio")
	bool SetValue(const FGarmentStudioState& InValue)
	{
		return MaybeBroadcastSuccessfulCompletion(FGarmentStudioState(InValue), Value);
	}

	UPROPERTY(BlueprintReadOnly, Category = "Garment Studio")
	FGarmentStudioState Value;
};

/**
 * The Garment Studio on a MeshForge garment fit step: pose the body into the garment, place it, sculpt it, wrap
 * it, look at the result, and finish - by sight.
 *
 * For agents that can read images. Capture Garment Studio writes a PNG of the studio from a named angle; everything
 * else changes the studio and returns where it stands. Each tool drives the window a person would use, so a person
 * can watch and take over at any point. A text-only agent can still use the plain command route: Send Interactive
 * Step Command in MeshForge Toolset.
 *
 * Every tool takes the definition's content path and the fit step's index; -1 finds the definition's first garment
 * fit step. Nothing here generates, spends money or deletes anything. A wrap runs Blender on this machine, about
 * fifteen seconds.
 */
UCLASS(BlueprintType)
class MESHFORGEGARMENTTOOLSET_API UMeshForgeGarmentToolset : public UToolsetDefinition
{
	GENERATED_BODY()

public:

	/** The version an agent is told it is talking to, read from this plugin's own descriptor. */
	virtual FString GetToolsetVersion() const override;

	/**
	 * Open the Garment Studio on a garment fit step, or bring it forward, and say where it stands.
	 *
	 * Start here. Then capture before changing anything: the studio opens where the step was last saved.
	 *
	 * @param DefinitionPath Content path of the MeshForge definition.
	 * @param StepIndex Index of the garment fit step in its post chain; -1 for the first one.
	 * @return Where the studio stands.
	 */
	UFUNCTION(meta = (AICallable), Category = "MeshForge|Garment Studio")
	static FGarmentStudioState OpenGarmentStudio(const FString& DefinitionPath, int32 StepIndex = -1);

	/** Where the studio stands, without changing anything. Poll this while a wrap runs if you did not wait for it. */
	UFUNCTION(meta = (AICallable), Category = "MeshForge|Garment Studio")
	static FGarmentStudioState GetGarmentStudioState(const FString& DefinitionPath, int32 StepIndex = -1);

	/**
	 * Take a picture of the studio and write it to a PNG to look at.
	 *
	 * The camera turns to the view you ask for - the whole body, or close on one joint - and stays there, so strokes
	 * sent next land where this picture shows. The viewport is the given mode's: switching to it first if needed.
	 *
	 * @param Mode Which viewport: the placement and pose, the sculpt before the wrap, or the last wrap.
	 * @param Camera Where the camera stands, around the character as it faces. Left and right are the character's.
	 * @param FocusJoint A joint's bone name to frame close (upperarm_l, spine_03, head); None for the whole body.
	 * @return The picture's path and the studio's state.
	 */
	UFUNCTION(meta = (AICallable), Category = "MeshForge|Garment Studio")
	static FGarmentStudioCapture CaptureGarmentStudio(const FString& DefinitionPath, int32 StepIndex, EGarmentStudioMode Mode,
		EGarmentStudioCamera Camera, FName FocusJoint);

	/**
	 * Turn one joint so the body sits inside the garment, the way a person turns it with the gizmo. The mirror
	 * joint on the other side follows.
	 *
	 * Turn by a few degrees at a time and look again. Arms are usually all it takes: bring the upper arms and
	 * elbows into the sleeves. The rotation is applied in the character's space, on top of where the joint is now.
	 *
	 * @param Joint The bone name, from the state's Joints.
	 * @param Turn Degrees to turn by: Roll about the character's X axis, Pitch about Y, Yaw about Z (up).
	 */
	UFUNCTION(meta = (AICallable), Category = "MeshForge|Garment Studio")
	static FGarmentStudioState TurnGarmentStudioJoint(const FString& DefinitionPath, int32 StepIndex, FName Joint, FRotator Turn);

	/** Put every joint back to the reference pose. */
	UFUNCTION(meta = (AICallable), Category = "MeshForge|Garment Studio")
	static FGarmentStudioState ResetGarmentStudioPose(const FString& DefinitionPath, int32 StepIndex = -1);

	/**
	 * Move, turn and scale the garment, in the character's space.
	 *
	 * @param bAutoPlace Ignore the numbers and place it from its garment type and the body's proportions: the best
	 *        first guess, and the way back from a placement gone wrong.
	 * @param Location Centimetres; the character stands at the origin, Z up.
	 * @param YawDegrees Turn about the vertical. 180 turns a garment worn back to front.
	 * @param Scale Uniform scale.
	 */
	UFUNCTION(meta = (AICallable), Category = "MeshForge|Garment Studio")
	static FGarmentStudioState PlaceGarmentInStudio(const FString& DefinitionPath, int32 StepIndex, bool bAutoPlace,
		FVector Location, float YawDegrees = 0.f, float Scale = 1.f);

	/**
	 * Sculpt the garment with drags across the last capture: before the wrap in Sculpt mode, or the wrapped
	 * garment in Wrap mode. The studio switches to that mode if it is not in it.
	 *
	 * Capture first, in the same mode, and give each drag in that picture's coordinates. A drag must start on the
	 * garment. Keep sculpting to small fixes - loosening a neck, pushing a clip out - since the wrap does the rest.
	 * The work is kept on the garment (not saved yet); the state's counts say how much moved.
	 *
	 * @param Mode Sculpt (before the wrap) or Wrap (the wrapped garment).
	 * @param Brush Sculpt drags the surface along each stroke; Smooth relaxes the whole garment and ignores strokes.
	 * @param Strokes Drags across the last capture, 0..1 from its top-left corner.
	 */
	UFUNCTION(meta = (AICallable), Category = "MeshForge|Garment Studio")
	static FGarmentStudioState SculptGarmentInStudio(const FString& DefinitionPath, int32 StepIndex, EGarmentStudioMode Mode,
		EGarmentStudioBrush Brush, const TArray<FGarmentStudioStroke>& Strokes);

	/** Throw the sculpt before the wrap away and go back to the garment as the step receives it. */
	UFUNCTION(meta = (AICallable), Category = "MeshForge|Garment Studio")
	static FGarmentStudioState ResetGarmentStudioSculpt(const FString& DefinitionPath, int32 StepIndex = -1);

	/** The fit's settings, their values and the choices each takes, as one sentence. Change them through Wrap Garment In Studio. */
	UFUNCTION(meta = (AICallable), Category = "MeshForge|Garment Studio")
	static FString GetGarmentFitSettings(const FString& DefinitionPath, int32 StepIndex = -1);

	/**
	 * Wrap the garment onto the body here, with the placement and pose as they stand, and wait for it (about
	 * fifteen seconds). Then capture in Wrap mode to judge it.
	 *
	 * @param bFromSculpt Start from the sculpt before the wrap; false starts from the garment as it came.
	 * @param Settings Fit settings to change first, kept on the step: FitStyle=Loose, ClearanceCm=2.5.
	 * @param bReplaceSculptAfterWrap Wrap even though the last wrap was sculpted, which replaces that sculpt.
	 */
	UFUNCTION(meta = (AICallable), Category = "MeshForge|Garment Studio")
	static UToolCallAsyncResultGarmentStudio* WrapGarmentInStudio(const FString& DefinitionPath, int32 StepIndex, bool bFromSculpt,
		const TArray<FGarmentFitSetting>& Settings, bool bReplaceSculptAfterWrap = false);

	/** Keep the placement, the pose, the sculpt and the last wrap on the step, and save the definition. */
	UFUNCTION(meta = (AICallable), Category = "MeshForge|Garment Studio")
	static FGarmentStudioState SaveGarmentStudio(const FString& DefinitionPath, int32 StepIndex = -1);

	/**
	 * Hand the wrap on screen to the chain as the fit step's output, as it is. A chain run then uses it without
	 * wrapping again until the garment changes, and a chain waiting on the studio carries on.
	 *
	 * @param bEvenIfOutOfDate Finish a wrap older than the last change to the pose, placement, sculpt or settings.
	 *        Normally wrap again instead.
	 */
	UFUNCTION(meta = (AICallable), Category = "MeshForge|Garment Studio")
	static FGarmentStudioState FinishGarmentStudio(const FString& DefinitionPath, int32 StepIndex, bool bEvenIfOutOfDate = false);

	/** Close the studio, saving first or dropping what was not saved. */
	UFUNCTION(meta = (AICallable), Category = "MeshForge|Garment Studio")
	static FGarmentStudioState CloseGarmentStudio(const FString& DefinitionPath, int32 StepIndex, bool bSave);
};
