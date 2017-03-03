#ifndef WEIGHT_ENGINE_NIWG_H
#define WEIGHT_ENGINE_NIWG_H

#ifdef __NIWG_ENABLED__
#include "NIWGReWeight.h"
#include "NIWGReWeight1piAngle.h"
#include "NIWGReWeight2010a.h"
#include "NIWGReWeight2012a.h"
#include "NIWGReWeight2014a.h"
#include "NIWGReWeightDeltaMass.h"
#include "NIWGReWeightEffectiveRPA.h"
#include "NIWGReWeightHadronMultSwitch.h"
#include "NIWGReWeightMEC.h"
#include "NIWGReWeightPiMult.h"
#include "NIWGReWeightProtonFSIbug.h"
#include "NIWGReWeightRPA.h"
#include "NIWGReWeightSpectralFunc.h"
#include "NIWGReWeightSplineEnu.h"
#include "NIWGSyst.h"
#include "NIWGSystUncertainty.h"
#endif

#include "FitLogger.h"

#include "GeneratorUtils.h"
#include "WeightEngineBase.h"
#include "FitWeight.h"

class NIWGWeightEngine : public WeightEngineBase {
public:
	NIWGWeightEngine(std::string name);
	~NIWGWeightEngine() {};

	void IncludeDial(std::string name, double startval);

	void SetDialValue(std::string name, double val);
	void SetDialValue(int nuisenum, double val);

	void Reconfigure(bool silent = false);

	double CalcWeight(BaseFitEvt* evt);

	inline bool NeedsEventReWeight() { return true; };


#ifdef __NIWG_ENABLED__
	std::vector<niwg::rew::NIWGSyst_t> fNIWGSysts;
	niwg::rew::NIWGReWeight* fNIWGRW;
#endif
};

#endif