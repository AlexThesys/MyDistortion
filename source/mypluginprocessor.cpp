//------------------------------------------------------------------------
// Copyright(c) 2021 Alex Suprunov.
//------------------------------------------------------------------------

#include "mypluginprocessor.h"
#include "myplugincids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

#include "constants.h"

extern void waveshaper(float* in, float* out, int buf_len, const params p);
extern void waveshaper_simd(float* in, float* out, int buf_len, const params p);

using namespace Steinberg;

namespace MyCompanyName {

template <typename T = float>
inline T scale_range(T newRangeMax, T newRangeMin, T originalValue) noexcept {
	return (newRangeMin + (newRangeMax - newRangeMin) * originalValue);
}

//------------------------------------------------------------------------
// MyDistortionProcessor
//------------------------------------------------------------------------
MyDistortionProcessor::MyDistortionProcessor () :	_coef_pos(DistConst::COEF_DEFAULT), 
													_coef_neg(DistConst::COEF_DEFAULT),
													_num_stages(DistConst::NUM_STAGES_DEFAULT),
													_invert_stages(1),
													_gain(DistConst::GAIN_DEFAULT),
													_bypass(0)
{
	//--- set the wanted controller for our processor
	setControllerClass (kMyDistortionControllerUID);
}

//------------------------------------------------------------------------
MyDistortionProcessor::~MyDistortionProcessor ()
{}

//------------------------------------------------------------------------
tresult PLUGIN_API MyDistortionProcessor::initialize (FUnknown* context)
{
	// Here the Plug-in will be instanciated
	
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	// if everything Ok, continue
	if (result != kResultOk)
	{
		return result;
	}

	//--- create Audio IO ------
	addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

	/* If you don't need an event bus, you can remove the next line */
	addEventInput (STR16 ("Event In"), 1);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MyDistortionProcessor::terminate ()
{
	// Here the Plug-in will be de-instanciated, last possibility to remove some memory!
	
	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API MyDistortionProcessor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----

	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API MyDistortionProcessor::process (Vst::ProcessData& data)
{
	//--- First : Read inputs parameter changes-----------

	if (data.inputParameterChanges)
	{
		int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
		for (int32 index = 0; index < numParamsChanged; index++)
		{
			Vst::IParamValueQueue* paramQueue =
				data.inputParameterChanges->getParameterData(index);
			if (paramQueue)
			{
				Vst::ParamValue value;
				int32 sampleOffset;
				int32 numPoints = paramQueue->getPointCount();
				switch (paramQueue->getParameterId())
				{
				case MyDistParams::kParamCoefPosID:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) ==
						kResultTrue)
						_coef_pos = scale_range<Steinberg::Vst::ParamValue>(DistConst::COEF_MAX, DistConst::COEF_MIN, value);
					break;
				case MyDistParams::kParamCoefNegID:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) ==
						kResultTrue)
						_coef_neg = scale_range<Steinberg::Vst::ParamValue>(DistConst::COEF_MAX, DistConst::COEF_MIN, value);
					break;
				case MyDistParams::kParamNumStagesID:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) ==
						kResultTrue)
						_num_stages = scale_range<Steinberg::Vst::ParamValue>(DistConst::NUM_STAGES_MAX, DistConst::NUM_STAGES_MIN, value);
					break;
				case MyDistParams::kParamInvertStagesID:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) ==
						kResultTrue)
						_invert_stages = value > 0.5f;
					break;
				case MyDistParams::kParamGainID:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) ==
						kResultTrue)
						_gain = value;
					break;
				case MyDistParams::kBypassID:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) ==
						kResultTrue)
						_bypass = value > 0.5f;
					break;
				}
			}
		}
	}
	
	//--- Here you have to implement your processing
	if (data.numInputs == 0 || data.numOutputs == 0)
	{
		// nothing to do
		return kResultOk;
	}

	if (data.numSamples > 0)
	{
		Vst::SpeakerArrangement arr;
		getBusArrangement(Vst::kOutput, 0, arr);
		int32 numChannels = Vst::SpeakerArr::getChannelCount(arr);

		if (_bypass) {
			for (int32 channel = 0; channel < numChannels; channel++) {
				for (int32 sample = 0, sz = data.numSamples; sample < sz; sample++) {
					data.outputs[0].channelBuffers32[channel][sample] = data.inputs[0].channelBuffers32[channel][sample];
				}
			}
		} else {
			// Process Algorithm
			for (int32 channel = 0; channel < numChannels; channel++) {
				params p = { (float)_coef_pos, (float)_coef_neg, (int32_t)_num_stages, (int32_t)_invert_stages, (float)_gain };
				waveshaper_simd((float*)data.inputs[0].channelBuffers32[channel], (float*)data.outputs[0].channelBuffers32[channel], data.numSamples, p);
			}
		}
	}


	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MyDistortionProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API MyDistortionProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	/* if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue; */

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MyDistortionProcessor::setState (IBStream* state)
{
	// called when we load a preset, the model has to be reloaded
	IBStreamer streamer (state, kLittleEndian);
	
	float res;
	if (streamer.readFloat(res) == false)
		return kResultFalse;
	_coef_pos = res;

	if (streamer.readFloat(res) == false)
		return kResultFalse;
	_coef_neg = res;

	if (streamer.readFloat(res) == false)
		return kResultFalse;
	_num_stages = res;

	if (streamer.readInt32(_invert_stages) == false)
		return kResultFalse;

	if (streamer.readFloat(res) == false)
		return kResultFalse;
	_gain = res;

	if (streamer.readInt32(_bypass) == false)
		return kResultFalse;

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MyDistortionProcessor::getState (IBStream* state)
{
	IBStreamer streamer(state, kLittleEndian);
	streamer.writeFloat((float)_coef_pos);
	streamer.writeFloat((float)_coef_neg);
	streamer.writeFloat((float)_num_stages);
	streamer.writeInt32(_invert_stages);
	streamer.writeFloat((float)_gain);
	streamer.writeInt32(_bypass);

	return kResultOk;
}

//------------------------------------------------------------------------
} // namespace MyCompanyName
