// Copyright 2016 L. Pickering, P Stowell, R. Terri, C. Wilkinson, C. Wret

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

#include "MINERvA_SignalDef.h"

#include "MINERvA_CC0pi_XSec_1DThetae_nue.h"

//********************************************************************
MINERvA_CC0pi_XSec_1DThetae_nue::MINERvA_CC0pi_XSec_1DThetae_nue(std::string inputfile, FitWeight *rw, std::string  type, std::string fakeDataFile){
//********************************************************************

  // Define Measurement
  fName = "MINERvA_CC0pi_XSec_1DThetae_nue";
  fPlotTitles = "; E_{e} (GeV); d#sigma/dE_{e} (cm^{2}/GeV)";
  EnuMin = 0.0;
  EnuMax = 10.0;
  fNormError = 0.101;
  fDefaultTypes = "FIX/FULL";
  fAllowedTypes = "FIX,FREE,SHAPE/DIAG,FULL/NORM/MASK";
  Measurement1D::SetupMeasurement(inputfile, type, rw, fakeDataFile);

  // Setup Data File
  std::string datafile = FitPar::GetDataBase()+"/MINERvA/CC0pi/MINERvA_CC0pi_nue_Data_ARX1509_05729.root";
  std::string dist_name = "";

  dist_name = "1DThetae";
  fPlotTitles = "; Q_{QE}^{2} (GeV^{2}); d#sigma/dQ_{QE}^{2} (cm^{2}/GeV^{2})";

  SetDataFromFile(datafile, "Data_" + dist_name);
  SetCovarFromDataFile(datafile, "Covar_" + dist_name);

  // Setup Default MC Hists
  SetupDefaultHist();

  // Different generators require slightly different rescaling factors.
  fScaleFactor = (this->fEventHist->Integral("width")*1E-38/(fNEvents+0.))/this->TotalIntegratedFlux();

};

//********************************************************************
void MINERvA_CC0pi_XSec_1DThetae_nue::FillEventVariables(FitEvent *event){
//********************************************************************

  if (event->NumFSParticle(11) == 0)
    return;

  TLorentzVector Pnu  = event->GetNeutrinoIn()->fP;
  TLorentzVector Pe   = event->GetHMFSParticle(11)->fP;

  Thetae   = Pnu.Vect().Angle(Pe.Vect()) * 180. / TMath::Pi();
  Ee       = Pe.E()/1000.0;

  fXVar = Thetae;
  LOG(EVT) << "fXVar = "<<fXVar<<std::endl;
  return;
}



//********************************************************************
bool MINERvA_CC0pi_XSec_1DThetae_nue::isSignal(FitEvent *event){
//*******************************************************************

  // Check this is a nue CC0pi event
  if (!SignalDef::isCC0pi(event, 12, EnuMin, EnuMax)) return false;

  // Restrict EE
  if (Ee < 0.5) return false;

  return true;
};

