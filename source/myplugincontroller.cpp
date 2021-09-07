//------------------------------------------------------------------------
// Copyright(c) 2021 Alex Suprunov.
//------------------------------------------------------------------------

#include "myplugincontroller.h"
#include "myplugincids.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "constants.h"

using namespace Steinberg;

namespace MyCompanyName {

//------------------------------------------------------------------------
// MyDistortionController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API MyDistortionController::initialize (FUnknown* context)
{
	// Here the Plug-in will be instanciated

	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize (context);
	if (result != kResultOk)
	{
		return result;
	}

	// Here you could register some parameters
	Vst::Parameter* param;

	//-----------------------------------
	param = new Vst::RangeParameter(STR16("Coef Positive"), MyDistParams::kParamCoefPosID,
									STR16(""), DistConst::COEF_MIN,
									DistConst::COEF_MAX,
									DistConst::COEF_DEFAULT);

	param->setPrecision(1);
	parameters.addParameter(param);
	//-----------------------------------
	param = new Vst::RangeParameter(STR16("Coef Negative"), MyDistParams::kParamCoefNegID,
									STR16(""), DistConst::COEF_MIN,
									DistConst::COEF_MAX,
									DistConst::COEF_DEFAULT);

	param->setPrecision(1);
	parameters.addParameter(param);
	//-----------------------------------
	param = new Vst::RangeParameter(STR16("Num Stages"), MyDistParams::kParamNumStagesID,
									STR16(""), DistConst::NUM_STAGES_MIN,
									DistConst::NUM_STAGES_MAX,
									DistConst::NUM_STAGES_DEFAULT, 9);

	param->setPrecision(1);
	parameters.addParameter(param);
	//-----------------------------------
	param = new Vst::StringListParameter(STR16("Invert Stages"), MyDistParams::kParamInvertStagesID,
										nullptr, Vst::ParameterInfo::kIsList);
	Vst::StringListParameter* strParam = static_cast<Vst::StringListParameter*>(param);
	strParam = static_cast<Vst::StringListParameter*>(param);
	strParam->appendString(STR16("Off"));	// 0
	strParam->appendString(STR16("On"));  // 1
	parameters.addParameter(param);
	//-----------------------------------
	param = new Vst::RangeParameter(STR16("Gain"), MyDistParams::kParamGainID,
									STR16(""), DistConst::GAIN_MIN,
									DistConst::GAIN_MAX,
									DistConst::GAIN_DEFAULT);

	param->setPrecision(1);
	parameters.addParameter(param);
	//---------------------------------
	parameters.addParameter(STR16("Bypass"), nullptr, 1, 0,
							Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass,
							MyDistParams::kBypassID);

	//------------------------------------

	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MyDistortionController::terminate ()
{
	// Here the Plug-in will be de-instanciated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API MyDistortionController::setComponentState (IBStream* state)
{
	// Here you get the state of the component (Processor part)
	if (!state)
		return kResultFalse;

	IBStreamer streamer(state, kLittleEndian);

	float savedParam1;
	if (streamer.readFloat(savedParam1) == false)
		return kResultFalse;
	Vst::Parameter* pParam = EditController::getParameterObject(MyDistParams::kParamCoefPosID);
	setParamNormalized(MyDistParams::kParamCoefPosID, pParam->toNormalized(savedParam1));

	if (streamer.readFloat(savedParam1) == false)
		return kResultFalse;
	pParam = EditController::getParameterObject(MyDistParams::kParamCoefNegID);
	setParamNormalized(MyDistParams::kParamCoefNegID, pParam->toNormalized(savedParam1));

	if (streamer.readFloat(savedParam1) == false)
		return kResultFalse;
	pParam = EditController::getParameterObject(MyDistParams::kParamNumStagesID);
	setParamNormalized(MyDistParams::kParamNumStagesID, pParam->toNormalized(savedParam1));

	int32 savedParam2;
	if (streamer.readInt32(savedParam2) == false)
		return kResultFalse;
	setParamNormalized(MyDistParams::kParamInvertStagesID, savedParam2 ? 1 : 0);

	if (streamer.readFloat(savedParam1) == false)
		return kResultFalse;
	setParamNormalized(MyDistParams::kParamGainID, savedParam1);

	// read the bypass
	if (streamer.readInt32(savedParam2) == false)
		return kResultFalse;
	setParamNormalized(MyDistParams::kBypassID, savedParam2 ? 1 : 0);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MyDistortionController::setState (IBStream* state)
{
	// Here you get the state of the controller

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MyDistortionController::getState (IBStream* state)
{
	// Here you are asked to deliver the state of the controller (if needed)
	// Note: the real state of your plug-in is saved in the processor

	return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API MyDistortionController::createView (FIDString name)
{
	// Here the Host wants to open your editor (if you have one)
	if (FIDStringsEqual (name, Vst::ViewType::kEditor))
	{
		// create your editor here and return a IPlugView ptr of it
        return nullptr;
	}
	return nullptr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MyDistortionController::setParamNormalized (Vst::ParamID tag, Vst::ParamValue value)
{
	// called by host to update your parameters
	tresult result = EditControllerEx1::setParamNormalized (tag, value);
	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API MyDistortionController::getParamStringByValue (Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string)
{
	// called by host to get a string for given normalized value of a specific parameter
	// (without having to set the value!)
	return EditControllerEx1::getParamStringByValue (tag, valueNormalized, string);
}

//------------------------------------------------------------------------
tresult PLUGIN_API MyDistortionController::getParamValueByString (Vst::ParamID tag, Vst::TChar* string, Vst::ParamValue& valueNormalized)
{
	// called by host to get a normalized value from a string representation of a specific parameter
	// (without having to set the value!)
	return EditControllerEx1::getParamValueByString (tag, string, valueNormalized);
}

//------------------------------------------------------------------------
} // namespace MyCompanyName
