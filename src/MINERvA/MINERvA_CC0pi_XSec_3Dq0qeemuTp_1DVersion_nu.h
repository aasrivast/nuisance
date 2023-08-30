// Copyright 2016-2021 L. Pickering, P Stowell, R. Terri, C. Wilkinson, C. Wret

/*******************************************************************************
*    This file is part of NUISANCE.
*
*    NUISANCE is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    NUISANCE is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with NUISANCE.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#ifndef MINERvA_CC0pi_XSec_3Dq0qeemuTp_1DVersion_nu_H_SEEN
#define MINERvA_CC0pi_XSec_3Dq0qeemuTp_1DVersion_nu_H_SEEN

#include "Measurement1D.h"
#include "TH3D.h"

// Implemented 6 May 2021 by S. Gardiner
class MINERvA_CC0pi_XSec_3Dq0qeemuTp_1DVersion_nu : public Measurement1D {

public:

  MINERvA_CC0pi_XSec_3Dq0qeemuTp_1DVersion_nu( nuiskey samplekey );
  virtual ~MINERvA_CC0pi_XSec_3Dq0qeemuTp_1DVersion_nu() {}

  void FillEventVariables( FitEvent* event );
  bool isSignal( FitEvent* event );

  int GetBinIndexFromKinamatics(double sum, double q0qe, double emu);

 protected:

 TH3D *hist_forBinning;

};

#endif
