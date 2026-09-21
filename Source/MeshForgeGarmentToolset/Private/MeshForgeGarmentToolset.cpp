#include "MeshForgeGarmentToolset.h"

#include "GarmentFitPipeline.h"
#include "GarmentStudio.h"
#include "MeshDef.h"
#include "MeshForgeGarmentToolsetModule.h"

#include "Containers/Ticker.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet/KismetSystemLibrary.h"

namespace MeshForgeGarmentToolsetPrivate
{
	/** The garment fit step, or null having raised a script error that names the tool to call next. */
	static UGarmentFitPipeline* FindStep(const FString& DefinitionPath, int32 StepIndex, FString& OutError)
	{
		UMeshDef* Def = DefinitionPath.IsEmpty() ? nullptr : LoadObject<UMeshDef>(nullptr, *DefinitionPath);
		if (Def == nullptr)
		{
			OutError = FString::Printf(TEXT("No mesh definition at '%s'. Call List Mesh Definitions (MeshForge Toolset) for the valid paths."), *DefinitionPath);
			return nullptr;
		}

		if (StepIndex < 0)
		{
			for (UMeshPostPipeline* Candidate : Def->PostPipelines)
			{
				if (UGarmentFitPipeline* Fit = Cast<UGarmentFitPipeline>(Candidate))
				{
					return Fit;
				}
			}
			OutError = FString::Printf(TEXT("'%s' has no garment fit step. Add one with Set Post Pipelines (MeshForge Toolset) and GarmentFitPipeline, "
				"then set its TargetCharacter with the object property tools."), *Def->GetName());
			return nullptr;
		}

		UGarmentFitPipeline* Step = Def->PostPipelines.IsValidIndex(StepIndex) ? Cast<UGarmentFitPipeline>(Def->PostPipelines[StepIndex]) : nullptr;
		if (Step == nullptr)
		{
			OutError = FString::Printf(TEXT("Step %d of '%s' is not a garment fit step. Pass -1 to use the first one."), StepIndex, *Def->GetName());
		}
		return Step;
	}

	static UGarmentFitPipeline* RequireStep(const FString& DefinitionPath, int32 StepIndex)
	{
		FString Error;
		UGarmentFitPipeline* Step = FindStep(DefinitionPath, StepIndex, Error);
		if (Step == nullptr)
		{
			UKismetSystemLibrary::RaiseScriptError(Error);
		}
		return Step;
	}

	/** The step, with its studio open, or null having said so. */
	static UGarmentFitPipeline* RequireOpenStudio(const FString& DefinitionPath, int32 StepIndex)
	{
		UGarmentFitPipeline* Step = RequireStep(DefinitionPath, StepIndex);
		if (Step != nullptr && !FGarmentStudio::GetStatus(Step).bOpen)
		{
			UKismetSystemLibrary::RaiseScriptError(TEXT("The Garment Studio is not open on this step. Call Open Garment Studio first."));
			return nullptr;
		}
		return Step;
	}

	static FGarmentStudioState ToState(const FGarmentStudioStatus& Status)
	{
		FGarmentStudioState State;
		State.bOpen = Status.bOpen;
		State.Mode = Status.Mode;
		State.GarmentLocation = Status.GarmentLocation;
		State.GarmentYawDegrees = static_cast<float>(Status.GarmentYaw);
		State.GarmentScale = static_cast<float>(Status.GarmentScale);
		for (const FGarmentStudioJoint& Joint : Status.Joints)
		{
			FGarmentStudioJointInfo& Info = State.Joints.AddDefaulted_GetRef();
			Info.Bone = Joint.Bone;
			Info.Label = Joint.Label;
			Info.Location = Joint.Location;
			Info.Turned = Joint.Turned;
		}
		State.SculptedVertices = Status.SculptMoved;
		State.GarmentVertices = Status.GarmentVertices;
		State.bWrapping = Status.bWrapping;
		State.bHasWrap = Status.bHasWrap;
		State.bWrapOutOfDate = Status.bWrapOutOfDate;
		State.bSculptedAfterWrap = Status.bSculptedAfterWrap;
		State.WrapSummary = Status.WrapSummary;
		State.WrapError = Status.WrapError;
		State.bFinished = Status.bFinished;
		State.bFinishedOnThisGarment = Status.bFinishedOnThisGarment;
		State.FinishedMesh = Status.FinishedMesh;
		State.bUnsaved = Status.bUnsaved;
		State.Description = Status.Description;
		return State;
	}

	static FGarmentStudioState Current(UGarmentFitPipeline* Step)
	{
		return Step != nullptr ? ToState(FGarmentStudio::GetStatus(Step)) : FGarmentStudioState();
	}

	/** One command in the studio's own words; false having raised its reply as the error. */
	static bool Command(UGarmentFitPipeline* Step, const FString& Words, FString* OutReply = nullptr)
	{
		FString Reply;
		const bool bDone = FGarmentStudio::RunCommand(Step, Words, Reply);
		if (!bDone)
		{
			UKismetSystemLibrary::RaiseScriptError(Reply);
		}
		if (OutReply != nullptr)
		{
			*OutReply = Reply;
		}
		return bDone;
	}

	static const TCHAR* ModeWord(EGarmentStudioMode Mode)
	{
		switch (Mode)
		{
		case EGarmentStudioMode::Sculpt: return TEXT("sculpt");
		case EGarmentStudioMode::Wrap:   return TEXT("wrap");
		default:                         return TEXT("pose");
		}
	}

	static bool EnterMode(UGarmentFitPipeline* Step, EGarmentStudioMode Mode)
	{
		return Command(Step, FString::Printf(TEXT("mode %s"), ModeWord(Mode)));
	}
}

FString UMeshForgeGarmentToolset::GetToolsetVersion() const
{
	// The descriptor is the version; nothing here repeats it.
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT(UE_PLUGIN_NAME));
	return Plugin.IsValid() ? Plugin->GetDescriptor().VersionName : FString();
}

FGarmentStudioState UMeshForgeGarmentToolset::OpenGarmentStudio(const FString& DefinitionPath, int32 StepIndex)
{
	using namespace MeshForgeGarmentToolsetPrivate;
	UGarmentFitPipeline* Step = RequireStep(DefinitionPath, StepIndex);
	FString Error;
	if (Step != nullptr && !FGarmentStudio::Open(Step, Error))
	{
		UKismetSystemLibrary::RaiseScriptError(Error);
	}
	return Current(Step);
}

FGarmentStudioState UMeshForgeGarmentToolset::GetGarmentStudioState(const FString& DefinitionPath, int32 StepIndex)
{
	using namespace MeshForgeGarmentToolsetPrivate;
	return Current(RequireStep(DefinitionPath, StepIndex));
}

FGarmentStudioCapture UMeshForgeGarmentToolset::CaptureGarmentStudio(const FString& DefinitionPath, int32 StepIndex, EGarmentStudioMode Mode,
	EGarmentStudioCamera Camera, FName FocusJoint)
{
	using namespace MeshForgeGarmentToolsetPrivate;
	FGarmentStudioCapture Capture;
	UGarmentFitPipeline* Step = RequireOpenStudio(DefinitionPath, StepIndex);
	if (Step == nullptr || !EnterMode(Step, Mode))
	{
		return Capture;
	}

	FString Error;
	if (!FGarmentStudio::Capture(Step, static_cast<EGarmentStudioView>(Camera), FocusJoint, Capture.ImagePath, Error))
	{
		UKismetSystemLibrary::RaiseScriptError(Error);
	}
	Capture.State = Current(Step);
	Capture.Mode = Capture.State.Mode;
	return Capture;
}

FGarmentStudioState UMeshForgeGarmentToolset::TurnGarmentStudioJoint(const FString& DefinitionPath, int32 StepIndex, FName Joint, FRotator Turn)
{
	using namespace MeshForgeGarmentToolsetPrivate;
	UGarmentFitPipeline* Step = RequireOpenStudio(DefinitionPath, StepIndex);
	if (Step != nullptr)
	{
		Command(Step, FString::Printf(TEXT("pose %s %f %f %f"), *Joint.ToString(), Turn.Pitch, Turn.Yaw, Turn.Roll));
	}
	return Current(Step);
}

FGarmentStudioState UMeshForgeGarmentToolset::ResetGarmentStudioPose(const FString& DefinitionPath, int32 StepIndex)
{
	using namespace MeshForgeGarmentToolsetPrivate;
	UGarmentFitPipeline* Step = RequireOpenStudio(DefinitionPath, StepIndex);
	if (Step != nullptr)
	{
		Command(Step, TEXT("reset-pose"));
	}
	return Current(Step);
}

FGarmentStudioState UMeshForgeGarmentToolset::PlaceGarmentInStudio(const FString& DefinitionPath, int32 StepIndex, bool bAutoPlace,
	FVector Location, float YawDegrees, float Scale)
{
	using namespace MeshForgeGarmentToolsetPrivate;
	UGarmentFitPipeline* Step = RequireOpenStudio(DefinitionPath, StepIndex);
	if (Step != nullptr)
	{
		Command(Step, bAutoPlace ? FString(TEXT("auto-place"))
			: FString::Printf(TEXT("place %f %f %f %f %f"), Location.X, Location.Y, Location.Z, YawDegrees, Scale));
	}
	return Current(Step);
}

FGarmentStudioState UMeshForgeGarmentToolset::SculptGarmentInStudio(const FString& DefinitionPath, int32 StepIndex, EGarmentStudioMode Mode,
	EGarmentStudioBrush Brush, const TArray<FGarmentStudioStroke>& Strokes)
{
	using namespace MeshForgeGarmentToolsetPrivate;
	UGarmentFitPipeline* Step = RequireOpenStudio(DefinitionPath, StepIndex);
	if (Step == nullptr)
	{
		return FGarmentStudioState();
	}
	if (Mode == EGarmentStudioMode::PoseAndPlace)
	{
		UKismetSystemLibrary::RaiseScriptError(TEXT("Sculpt in Sculpt mode (before the wrap) or Wrap mode (the wrapped garment), not PoseAndPlace."));
		return Current(Step);
	}
	if (!EnterMode(Step, Mode))
	{
		return Current(Step);
	}

	if (Brush == EGarmentStudioBrush::Smooth)
	{
		// Smooth works its result out in the background; accept waits for it.
		if (Command(Step, TEXT("tool smooth")) && Command(Step, TEXT("accept")))
		{
			Command(Step, TEXT("keep"));
		}
		return Current(Step);
	}

	if (Strokes.Num() == 0)
	{
		UKismetSystemLibrary::RaiseScriptError(TEXT("No strokes. Capture Garment Studio in this mode, then give drags in that picture's coordinates."));
		return Current(Step);
	}
	if (!Command(Step, TEXT("tool sculpt")))
	{
		return Current(Step);
	}
	for (const FGarmentStudioStroke& Stroke : Strokes)
	{
		if (!Command(Step, FString::Printf(TEXT("stroke %f %f %f %f 16"), Stroke.FromX, Stroke.FromY, Stroke.ToX, Stroke.ToY)))
		{
			// Keep what the strokes before this one did.
			break;
		}
	}
	FString Reply;
	FGarmentStudio::RunCommand(Step, TEXT("keep"), Reply);
	return Current(Step);
}

FGarmentStudioState UMeshForgeGarmentToolset::ResetGarmentStudioSculpt(const FString& DefinitionPath, int32 StepIndex)
{
	using namespace MeshForgeGarmentToolsetPrivate;
	UGarmentFitPipeline* Step = RequireOpenStudio(DefinitionPath, StepIndex);
	if (Step != nullptr)
	{
		Command(Step, TEXT("reset-sculpt"));
	}
	return Current(Step);
}

FString UMeshForgeGarmentToolset::GetGarmentFitSettings(const FString& DefinitionPath, int32 StepIndex)
{
	using namespace MeshForgeGarmentToolsetPrivate;
	UGarmentFitPipeline* Step = RequireOpenStudio(DefinitionPath, StepIndex);
	FString Reply;
	if (Step != nullptr)
	{
		Command(Step, TEXT("options"), &Reply);
	}
	return Reply;
}

UToolCallAsyncResultGarmentStudio* UMeshForgeGarmentToolset::WrapGarmentInStudio(const FString& DefinitionPath, int32 StepIndex, bool bFromSculpt,
	const TArray<FGarmentFitSetting>& Settings, bool bReplaceSculptAfterWrap)
{
	using namespace MeshForgeGarmentToolsetPrivate;
	UToolCallAsyncResultGarmentStudio* Result = NewObject<UToolCallAsyncResultGarmentStudio>();

	FString Error;
	UGarmentFitPipeline* Step = FindStep(DefinitionPath, StepIndex, Error);
	if (Step == nullptr)
	{
		Result->SetError(Error);
		return Result;
	}
	if (!FGarmentStudio::GetStatus(Step).bOpen)
	{
		Result->SetError(TEXT("The Garment Studio is not open on this step. Call Open Garment Studio first."));
		return Result;
	}

	FString Words = FString::Printf(TEXT("wrap from=%s"), bFromSculpt ? TEXT("sculpt") : TEXT("original"));
	if (bReplaceSculptAfterWrap)
	{
		Words += TEXT(" force");
	}
	for (const FGarmentFitSetting& Setting : Settings)
	{
		if (Setting.Name.Contains(TEXT(" ")) || Setting.Value.Contains(TEXT(" ")) || Setting.Name.IsEmpty())
		{
			Result->SetError(FString::Printf(TEXT("'%s=%s' is not a setting. Call Get Garment Fit Settings for the names and values."), *Setting.Name, *Setting.Value));
			return Result;
		}
		Words += FString::Printf(TEXT(" %s=%s"), *Setting.Name, *Setting.Value);
	}

	FString Reply;
	if (!FGarmentStudio::RunCommand(Step, Words, Reply))
	{
		Result->SetError(Reply);
		return Result;
	}

	// Rooted until the wrap ends: the garbage collector cannot see a pointer held only by a lambda.
	Result->AddToRoot();
	const TWeakObjectPtr<UGarmentFitPipeline> WeakStep(Step);
	const double Deadline = FPlatformTime::Seconds() + 600.0;
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Result, WeakStep, Deadline](float) -> bool
	{
		UGarmentFitPipeline* Live = WeakStep.Get();
		const FGarmentStudioStatus Status = Live ? FGarmentStudio::GetStatus(Live) : FGarmentStudioStatus();
		if (Status.bOpen && Status.bWrapping && FPlatformTime::Seconds() < Deadline)
		{
			return true;
		}

		if (!Status.bOpen)
		{
			Result->SetError(TEXT("The Garment Studio closed while it was wrapping."));
		}
		else if (Status.bWrapping)
		{
			Result->SetError(TEXT("Stopped waiting after ten minutes; the wrap is still running. Call Get Garment Studio State to see when it ends."));
		}
		else if (!Status.WrapError.IsEmpty())
		{
			Result->SetError(Status.WrapError);
		}
		else
		{
			Result->SetValue(MeshForgeGarmentToolsetPrivate::ToState(Status));
		}
		Result->RemoveFromRoot();
		return false;
	}), 0.5f);
	return Result;
}

FGarmentStudioState UMeshForgeGarmentToolset::SaveGarmentStudio(const FString& DefinitionPath, int32 StepIndex)
{
	using namespace MeshForgeGarmentToolsetPrivate;
	UGarmentFitPipeline* Step = RequireOpenStudio(DefinitionPath, StepIndex);
	if (Step != nullptr)
	{
		Command(Step, TEXT("save"));
	}
	return Current(Step);
}

FGarmentStudioState UMeshForgeGarmentToolset::FinishGarmentStudio(const FString& DefinitionPath, int32 StepIndex, bool bEvenIfOutOfDate)
{
	using namespace MeshForgeGarmentToolsetPrivate;
	UGarmentFitPipeline* Step = RequireOpenStudio(DefinitionPath, StepIndex);
	if (Step != nullptr)
	{
		Command(Step, bEvenIfOutOfDate ? TEXT("finish force") : TEXT("finish"));
	}
	return Current(Step);
}

FGarmentStudioState UMeshForgeGarmentToolset::CloseGarmentStudio(const FString& DefinitionPath, int32 StepIndex, bool bSave)
{
	using namespace MeshForgeGarmentToolsetPrivate;
	UGarmentFitPipeline* Step = RequireOpenStudio(DefinitionPath, StepIndex);
	if (Step != nullptr)
	{
		Command(Step, bSave ? TEXT("close save") : TEXT("close discard"));
	}
	return Current(Step);
}
