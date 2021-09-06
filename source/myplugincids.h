//------------------------------------------------------------------------
// Copyright(c) 2021 Alex Suprunov.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace MyCompanyName {
//------------------------------------------------------------------------
static const Steinberg::FUID kMyDistortionProcessorUID (0x28EC1380, 0x478D509F, 0x9156256B, 0x3BC3E1C9);
static const Steinberg::FUID kMyDistortionControllerUID (0x52DDBAFF, 0x7A1752A8, 0xB7F1C0D1, 0x40AF1A3C);

#define MyDistortionVST3Category "Fx"

//------------------------------------------------------------------------
} // namespace MyCompanyName
