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

/** 
 * Radecky et al. Phys Rev D, 3rd series, volume 25, number 5, 1 March 1982, p 1161-1173
*/

#include "ANL_CC1pi0_XSec_1DEnu_nu.h"

// The constructor
ANL_CC1pi0_XSec_1DEnu_nu::ANL_CC1pi0_XSec_1DEnu_nu(std::string inputfile, FitWeight *rw, std::string type, std::string fakeDataFile){

  std::cout << type << " in ANL CC1pi0" << std::endl;

  fName = "ANL_CC1pi0_XSec_1DEnu_nu";
  fPlotTitles = "; E_{#nu} (GeV); #sigma(E_{#nu}) (cm^{2}/neutron)";
  EnuMin = 0.;
  EnuMax = 1.5;
  fIsDiag = true;
  fNormError = 0.20;
  fDefaultTypes = "FIX/DIAG";
  fAllowedTypes = "FIX,FREE,SHAPE/DIAG/UNCORR/W14/W16/NOW";
  Measurement1D::SetupMeasurement(inputfile, type, rw, fakeDataFile);

  // User can specify "UNCORR" for uncorrected data
  // Default is to use correction
  if (type.find("UNCORR") != std::string::npos) {
    UseCorrectedData = false;
    fName += "_uncorr";
  } else {
    UseCorrectedData = true;
    fName += "_corr";
  }

  // User can specify "W14" for W < 1.4 GeV cut
  //                  "W16" for W < 1.6 GeV cut
  //                  The default is no W cut (10 GeV)
  if (type.find("W14") != std::string::npos) {
    wTrueCut = 1.4;
    fName += "_w14";
  } else if (type.find("W16") != std::string::npos) {
    wTrueCut = 1.6;
    fName += "_w16";
  } else {
    wTrueCut = 10.0;
    fName += "_noW";
  }

  // Now read in different data depending on what the user has specified
  std::string DataLocation = GeneralUtils::GetTopLevelDir()+"/data/ANL/CC1pi0_on_n/";

  // If we're using corrected data
  if (UseCorrectedData) {
    if (wTrueCut == 1.4) {
      DataLocation += "anl82corr-numu-n-to-mu-p-pi0-lowW_edges.txt";
    } else if (wTrueCut == 10.0) {
      DataLocation += "anl82corr-numu-n-to-mu-p-pi0-noW_edges.txt";
    } else {
      ERR(FTL) << "Can not run ANL CC1pi01n W < 1.6 GeV with CORRECTION, because the data DOES NOT EXIST" << std::endl;
      ERR(FTL) << "Correction exists for W < 1.4 GeV and no W cut data ONLY" << std::endl;
      exit(-1);
    }

  // If we're using raw uncorrected data
  } else {

    if (wTrueCut == 1.4) {
      DataLocation += "anl82-numu-cc1pi0-14Wcut.txt";
    } else if (wTrueCut == 1.6) {
      DataLocation += "anl82-numu-cc1pi0-16Wcut.txt";
    } else if (wTrueCut == 10.0) {
      DataLocation += "anl82-numu-cc1pi0-noWcut.txt";
    } else {
      ERR(FTL) << "Can only run W = 1.4, 1.6 and no W cut" << std::endl;
      ERR(FTL) << "You specified: " << wTrueCut << std::endl;
      exit(-1);
    }
  }

  this->SetDataValues(DataLocation);
  this->SetupDefaultHist();

  fFullCovar = StatUtils::MakeDiagonalCovarMatrix(fDataHist);
  covar     = StatUtils::GetInvert(fFullCovar);

  this->fScaleFactor = this->fEventHist->Integral("width")*double(1E-38)/double(fNEvents+0.)*(16./8.);
};


void ANL_CC1pi0_XSec_1DEnu_nu::FillEventVariables(FitEvent *event) {

  if (event->NumFSParticle(2212) == 0 || event->NumFSParticle(111) == 0 || event->NumFSParticle(13) == 0) {
    return;
  }

  TLorentzVector Pnu  = event->GetNeutrinoIn()->fP;
  TLorentzVector Pp   = event->GetHMFSParticle(2212)->fP;
  TLorentzVector Ppi0 = event->GetHMFSParticle(111)->fP;
  TLorentzVector Pmu  = event->GetHMFSParticle(13)->fP;

  double hadMass = FitUtils::MpPi(Pp, Ppi0);
  double Enu     = -1.0;

  if (hadMass/1000. < wTrueCut) {
    Enu = Pnu.E()/1.E3;
  }

  fXVar = Enu;

  return;
};


bool ANL_CC1pi0_XSec_1DEnu_nu::isSignal(FitEvent *event) {
  return SignalDef::isCC1pi3Prong(event, 14, 111, 2212, EnuMin, EnuMax);
}


/*
void ANL_CC1pi0_XSec_1DEnu_nu::FillHistograms() {

  if (makeHadronicMassHist) {
    hadMassHist->Fill(hadMass);
  }

  Measurement1D::FillHistograms();

  return;
}


void ANL_CC1pi0_XSec_1DEnu_nu::ScaleEvents() {

  PlotUtils::FluxUnfoldedScaling(fMCHist, fFluxHist);
  PlotUtils::FluxUnfoldedScaling(fMCFine, fFluxHist);

  fMCHist->Scale(fScaleFactor);
  fMCFine->Scale(fScaleFactor);

  return;
}
*/
