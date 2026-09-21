# MeshForgeGarmentToolset

<!-- forge:version -->**Version 0.1.0. Experimental.**<!-- /forge:version -->

MeshForge Garment's **Garment Studio** as Model Context Protocol tools, for agents that can see: open it, look
at the character from named angles, pose the body into the garment, place it, sculpt, wrap, judge the wrap from
pictures, and finish.

```
open  →  capture  →  place and pose  →  capture  →  wrap  →  capture  →  fix  →  finish
```

Every tool drives the window a person would use, so a person can watch an agent work and take over at any
point. This is optional: a person fits a garment better and faster. The tools are here for the frontier models
that can read an image well enough to do it, and for a chain left waiting on the studio with nobody at the desk.

---

## The tools

| | |
|---|---|
| **Studio** | `OpenGarmentStudio`, `GetGarmentStudioState`, `SaveGarmentStudio`, `CloseGarmentStudio` |
| **Looking** | `CaptureGarmentStudio`: a PNG of the pose, sculpt or wrap viewport from Front, Back, Left, Right, FrontLeft, FrontRight or Top, the whole body or close on one joint. The camera stays, so strokes land where the picture shows. |
| **Pose and place** | `TurnGarmentStudioJoint`, `ResetGarmentStudioPose`, `PlaceGarmentInStudio` |
| **Sculpt** | `SculptGarmentInStudio` (drags in the last capture's coordinates, before or after the wrap), `ResetGarmentStudioSculpt` |
| **Wrap** | `GetGarmentFitSettings`, `WrapGarmentInStudio` (waits for the wrap, about fifteen seconds) |
| **Finish** | `FinishGarmentStudio`: hands the wrap to the chain as the fit step's output |

The skill **Fit a garment by sight** (`UGarmentStudioSkill`) is the loop an agent follows and what to look for
in each picture. It is found automatically.

A text-only agent can still drive the studio through MeshForge Toolset's **Send Interactive Step Command**.

---

## Why a separate plugin

The family rule is that a capability and its toolset ship apart, so a registry that fails to load cannot take
the capability down with it. MeshForge Garment is sold; this surface is published, because a tool surface nobody
can read is a tool surface nobody integrates with.

Requires MeshForge, MeshForge Garment and the engine's ToolsetRegistry. A wrap runs Blender on this machine.
