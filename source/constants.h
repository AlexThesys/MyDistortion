#pragma once

#include "public.sdk/source/vst/vstparameters.h"

struct params {
    float coef_pos;
    float coef_neg;
    int32_t num_stages;
    int32_t invert_stages;
    float gain;
};

namespace Steinberg {

enum MyDistParams : Vst::ParamID
{
    kParamCoefPosID = 101,
    kParamCoefNegID = 102,
    kParamNumStagesID = 103,
    kParamInvertStagesID = 104,
    kParamGainID = 105,

    kBypassID = 106
};

namespace DistConst
{
    static constexpr float COEF_MIN = 0.1f;
    static constexpr float COEF_MAX = 2.0f;
    static constexpr float COEF_DEFAULT = 0.5f;
    static constexpr float NUM_STAGES_MIN = 1.0f;
    static constexpr float NUM_STAGES_MAX = 10.0f;
    static constexpr float NUM_STAGES_DEFAULT = 6.0f;
    static constexpr float GAIN_MIN = 0.0f;
    static constexpr float GAIN_MAX = 1.0f;
    static constexpr float GAIN_DEFAULT = 1.0f;
};

}