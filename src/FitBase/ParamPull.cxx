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
#include "ParamPull.h"

//*******************************************************************************
ParamPull::ParamPull(std::string name, std::string inputfile, std::string type, std::string dials){
//*******************************************************************************

  fMinHist = NULL;
  fMaxHist = NULL;
  fTypeHist = NULL;
  fDialSelection = dials;
  
  fName  = name;
  fInput = inputfile;
  fType  = type;
  
  // Set the pull type
  SetType(fType);
  
  // Setup Histograms from input file
  SetupHistograms(fInput);    
};

//*******************************************************************************
void ParamPull::SetType(std::string type){
//*******************************************************************************

  fType = type;
  // Assume Default if empty
  if (type.empty()){
    ERR(WRN) << "No type specified for ParmPull class " << fName << endl;
    ERR(WRN) << "Assuming GAUSTHROW/GAUSPULL" << endl;

    type = "GAUSTHROW/GAUSPULL";
  }
  
  // Set Dial options
  if (type.find("FRAC") != std::string::npos){

    fDialOptions = "FRAC";
    fPlotTitles  = ";; Fractional RW Value";

  } else if (type.find("ABS") != std::string::npos){

    fDialOptions = "ABS";
    fPlotTitles= ";; ABS RW Value";

  } else {

    fDialOptions = "";
    fPlotTitles= ";; RW Value";

  }

  // Parse throw types
  if (type.find("GAUSPULL") != std::string::npos) fCalcType = kGausPull;
  else fCalcType = kNoPull;

  if (type.find("GAUSTHROW") != std::string::npos) fThrowType = kGausThrow;
  else fThrowType = kNoThrow;

  // Extra check to see if throws or pulls are turned off
  if (type.find("NOPULL") != std::string::npos)  fCalcType = kNoPull;
  if (type.find("NOTHROW") != std::string::npos) fThrowType = kNoThrow;
  
}

//*******************************************************************************
void ParamPull::SetupHistograms(std::string input){
//*******************************************************************************

  // Extract Types from Input
  fFileType = "";
  const int nfiletypes = 4;
  const std::string filetypes[nfiletypes] = {"FIT","ROOT","TXT","DIAL"};

  for (int i = 0; i < nfiletypes; i++) {
    std::string tempTypes = filetypes[i] + ":";
    if (input.find(tempTypes) != std::string::npos) {
      fFileType = filetypes[i];
      input.replace(input.find(tempTypes), tempTypes.size(), "");
      break;
    }
  }

  // Read Files
  if      (!fFileType.compare("FIT")) ReadFitFile(input);
  else if (!fFileType.compare("ROOT")) ReadRootFile(input);
  else if (!fFileType.compare("VECT")) ReadVectFile(input);
  else if (!fFileType.compare("DIAL")) ReadDialInput(input);
  else {
    ERR(FTL) << "Unknown ParamPull Type: " << input << endl;
    throw;
  }
  
  // Check Dials are all good
  CheckDialsValid();

  // Setup MC Histogram
  fMCHist = (TH1D*) fDataHist->Clone();
  fMCHist->Reset();
  fMCHist->SetNameTitle( (fName + "_MC").c_str(),
			 (fName + " MC" + fPlotTitles).c_str() );

  // If no Covar input make an uncorrelated one
  if (!fCovar){
    fCovar = StatUtils::MakeDiagonalCovarMatrix(fDataHist, 1.0);
  }

  // If no types or limits are provided give them a default option
  if (!fMinHist){
    fMinHist = (TH1D*) fDataHist->Clone();
    fMinHist->SetNameTitle( (fName + "_min").c_str(),
			    (fName + " min" + fPlotTitles).c_str() );
    for (int i = 0; i < fMinHist->GetNbinsX(); i++){
      // TODO (P.Stowell) Change this to a NULL system where limits are actually free!    
      fMinHist->SetBinContent(i+1, fDataHist->GetBinContent(i+1) - 1E6); 
    }
  }

  if (!fMaxHist){
    fMaxHist = (TH1D*) fDataHist->Clone();
    fMaxHist->SetNameTitle( (fName + "_min").c_str(),
			    (fName + " min" + fPlotTitles).c_str() );
    for(int i = 0; i < fMaxHist->GetNbinsX(); i++){
      fMaxHist->SetBinContent(i+1, fDataHist->GetBinContent(i+1) - 1E6);
    }
  }

  // Set types from state, or to unknown
  if (!fTypeHist){

    int deftype = -1;
    if (fType.find("T2K")        != std::string::npos){ deftype = kT2K;   }
    else if (fType.find("NEUT")  != std::string::npos){ deftype = kNEUT;  }
    else if (fType.find("NIWG")  != std::string::npos){ deftype = kNIWG;  }
    else if (fType.find("GENIE") != std::string::npos){ deftype = kGENIE; }
    else if (fType.find("NORM")  != std::string::npos){ deftype = kNORM;  }
    else if (fType.find("NUWRO") != std::string::npos){ deftype = kNUWRO; }
    
    fTypeHist = new TH1I( (fName + "_type").c_str(),
			  (fName + " type" + fPlotTitles).c_str(),
			  fDataHist->GetNbinsX(), 0, fDataHist->GetNbinsX() );
    
    for(int i = 0; i < fTypeHist->GetNbinsX(); i++){
      fTypeHist->SetBinContent(i+1, deftype );
    }
  }
  
  
  // Sort Covariances
  fInvCovar = StatUtils::GetInvert(fCovar);
  fDecomp   = StatUtils::GetDecomp(fCovar);

  // Create DataTrue for Throws
  fDataTrue = (TH1D*) fDataHist->Clone();
  fDataTrue->SetNameTitle( (fName + "_truedata").c_str(),
			   (fName + " truedata" + fPlotTitles).c_str() );

  fDataOrig = (TH1D*) fDataHist->Clone();
  fDataOrig->SetNameTitle( (fName + "_origdata").c_str(),
			   (fName + " origdata" + fPlotTitles).c_str() );

  // Select only dials we want
  if (!fDialSelection.empty()){
    (*fDataHist) = RemoveBinsNotInString(*fDataHist, fDialSelection);
    

    
  }
  
}

//******************************************************************************* 
TH1D ParamPull::RemoveBinsNotInString(TH1D hist, std::string mystr){
//******************************************************************************* 

  // Make list of allowed bins
  std::vector<std::string> allowedbins;
  for (int i = 0; i < hist.GetNbinsX(); i++){
    std::string syst = std::string(hist.GetXaxis()->GetBinLabel(i+1));
    
    if (mystr.find(syst) != std::string::npos){
      allowedbins.push_back(syst);
    }
  }

  // Make new histogram
  UInt_t nbins = allowedbins.size();
  TH1D newhist = TH1D( hist.GetName(), hist.GetTitle(),
		       (Int_t)nbins, 0.0, (Double_t)nbins);

  // Setup bins
  for (UInt_t i = 0; i < nbins; i++){
    // Set Labels
    newhist.GetXaxis()->SetBinLabel(i+1, allowedbins[i].c_str());

    // Copy Values
    for (UInt_t j = 0; j < hist.GetNbinsX(); j++){
      if (!allowedbins[i].compare(hist.GetXaxis()->GetBinLabel(j+1))){
	newhist.SetBinContent(i+1, hist.GetBinContent(j+1) );
	newhist.SetBinError(i+1, hist.GetBinError(j+1) );
      }
    }
  }

  return newhist;
}

//*******************************************************************************
TH1I ParamPull::RemoveBinsNotInString(TH1I hist, std::string mystr){
//*******************************************************************************

  // Make list of allowed bins
  std::vector<std::string> allowedbins;
  for (int i = 0; i < hist.GetNbinsX(); i++){
    std::string syst = std::string(hist.GetXaxis()->GetBinLabel(i+1));

    if (mystr.find(syst) != std::string::npos){
      allowedbins.push_back(syst);
    }
  }

  // Make new histogram
  UInt_t nbins = allowedbins.size();
  TH1I newhist = TH1I( hist.GetName(), hist.GetTitle(),
		       (Int_t)nbins, 0.0, (Int_t)nbins);

  // Setup bins
  for (UInt_t i = 0; i < nbins; i++){
    // Set Labels
    newhist.GetXaxis()->SetBinLabel(i+1, allowedbins[i].c_str());

    // Copy Values
    for (UInt_t j = 0; j < hist.GetNbinsX(); j++){
      if (!allowedbins[i].compare(hist.GetXaxis()->GetBinLabel(j+1))){
	newhist.SetBinContent(i+1, hist.GetBinContent(j+1) );
	newhist.SetBinError(i+1, hist.GetBinError(j+1) );
      }
    }
  }

  return newhist;
}

//******************************************************************************* 
void ParamPull::ReadFitFile(std::string input){
//*******************************************************************************

  TFile* tempfile = new TFile(input.c_str(),"READ");

  // Read Data
  fDataHist = (TH1D*) tempfile->Get("fit_dials_free");
  if (!fDataHist){
    ERR(FTL) << "Can't find TH1D hist fit_dials in " << fName << endl;
    ERR(FTL) << "File Entries:" << endl;
    tempfile->ls();

    throw;
  }

  fDataHist->SetDirectory(0);
  fDataHist->SetNameTitle( (fName + "_data").c_str(),
			   (fName + " data" + fPlotTitles).c_str() );

  // Read Covar
  TH2D* tempcov = (TH2D*) tempfile->Get("covariance_free");
  if (!tempcov){
    ERR(FTL) << "Can't find TH2D covariance_free in " << fName << endl;
    ERR(FTL) << "File Entries:" << endl;
    tempfile->ls();

    throw;
  }

  // Setup Covar
  int nbins = fDataHist->GetNbinsX();
  fCovar = new TMatrixDSym( nbins );

  for (int i = 0; i < nbins; i++){
    for(int j = 0; j < nbins; j++){
      (*fCovar)(i,j) = tempcov->GetBinContent(i+1,j+1);
    }
  }
  
  return;
}

//*******************************************************************************
void ParamPull::ReadRootFile(std::string input){
//******************************************************************************* 

  std::vector<std::string> inputlist = PlotUtils::ParseToStr(input,";");

  // Check all given
  if (inputlist.size() < 2){
    ERR(FTL) << "Covar supplied in 'ROOT' format should have 3 semi-colon seperated entries!" << endl
	     << "ROOT:filename;histname[;covarname]" << endl;
    ERR(FTL) << "histname = TH1D, covarname = TH2D" << endl;
    throw;
  }

  // Get Entries
  std::string filename  = inputlist[0];
  cout << filename << endl;
  std::string histname  = inputlist[1];
  cout << histname << endl;
  cout << input<< endl;

  // Read File
  TFile* tempfile = new TFile(filename.c_str(),"READ");
  if (tempfile->IsZombie()){
    ERR(FTL) << "Can't find file in " << fName << endl;
    ERR(FTL) << "location = " << filename << endl;
    throw;
  }

  // Read Hist
  fDataHist = (TH1D*) tempfile->Get(histname.c_str());
  if (!fDataHist){
    ERR(FTL) << "Can't find TH1D hist " << histname << " in " << fName << endl;
    ERR(FTL) << "File Entries:" << endl;
    tempfile->ls();

    throw;
  }
  fDataHist->SetDirectory(0);
  fDataHist->SetNameTitle( (fName + "_data").c_str(),
			   (fName + " data" + fPlotTitles).c_str() );

  cout << "READING COVAR" << endl;
  // Read Covar
  if (inputlist.size() > 2){
    std::string covarname = inputlist[2];
    cout << "COVARNAME = " <<covarname << endl;
    
    TH2D* tempcov = (TH2D*) tempfile->Get(covarname.c_str());
    if (!tempcov){
      ERR(FTL) << "Can't find TH2D covar " << covarname << " in " << fName << endl;
      ERR(FTL) << "File Entries:" << endl;
      tempfile->ls();
      
      throw;
    }
    
    // Setup Covar
    int nbins = fDataHist->GetNbinsX();
    fCovar = new TMatrixDSym( nbins );
    
    for (int i = 0; i < nbins; i++){
      for (int j = 0; j < nbins; j++){
	(*fCovar)(i,j) = tempcov->GetBinContent(i+1,j+1);
      }
    }
    
  // Uncorrelated
  } else {
    LOG(SAM) <<"No Covar provided so using diagonal errors for "
	     << fName << endl;
    fCovar = NULL;
  }
}

//*******************************************************************************  
void ParamPull::ReadVectFile(std::string input){
//*******************************************************************************

  std::vector<std::string> inputlist = PlotUtils::ParseToStr(input,";");
  if (inputlist.size() < 4){
    ERR(FTL) << "Need 3 inputs for vector input in " << fName << endl;
    ERR(FTL) << "Inputs: " << input << endl;
    throw;
  }

  // Open File
  std::string rootname   = inputlist[0];
  TFile* tempfile = new TFile(rootname.c_str(),"READ");
  if (tempfile->IsZombie()){
    ERR(FTL) << "Can't find file in " << fName << endl;
    ERR(FTL) << "location = " << rootname << endl;
    throw;
  }

  // Get Name
  std::string tagname = inputlist[1];
  //  TVector<std::string> dialtags = tempfile->Get(tagname.c_str());
  //  if (!dialtags){
  //    ERR(FTL) << "Can't find list of dial names!" << endl;
  //  }
  
  // Get Values 
  std::string valuename  = inputlist[2];
  TVectorD* dialvals = (TVectorD*)tempfile->Get(valuename.c_str());
  if (!dialvals){
    ERR(FTL) << "Can't find dial values" << endl;
  }

  // Get Matrix 
  std::string matrixname = inputlist[3];
  TMatrixD* matrixvals = (TMatrixD*)tempfile->Get(matrixname.c_str());
  if (!matrixvals){
    ERR(FTL) << "Can't find matirx values" << endl;
  }
 
  // Get Types
  if (inputlist.size() > 4){
    std::string typesname  = inputlist[3];
  }

  // Get Minimum
  if (inputlist.size() > 5){
    std::string minname = inputlist[4];
  }

  // Get Maximum
  if (inputlist.size() > 6){
    std::string maxname = inputlist[5];
  }
  
}

//*******************************************************************************  
void ParamPull::ReadDialInput(std::string input){
//*******************************************************************************  

  std::vector<std::string> inputlist = PlotUtils::ParseToStr(input,";");
  if (inputlist.size() < 3){
    ERR(FTL) << "Need 3 inputs for dial input in " << fName << endl;
    ERR(FTL) << "Inputs: " << input << endl;
    throw;
  }

  std::vector<double> inputvals = PlotUtils::ParseToDbl(input,";");
  std::string dialname = inputlist[0];
  double val = inputvals[1];
  double err = inputvals[2];
  
  fDataHist = new TH1D( (fName + "_data").c_str(),
			(fName + "_data" + fPlotTitles).c_str(), 1, 0, 1);
  fDataHist->SetBinContent(1, val);
  fDataHist->SetBinError(1, err);
  fDataHist->GetXaxis()->SetBinLabel(1, dialname.c_str());

  fCovar = NULL;
}

//*******************************************************************************   
std::map<std::string, int> ParamPull::GetAllDials(){
//*******************************************************************************   

  std::map<std::string, int> dialtypemap;
  
  for (int i = 0; i < fDataHist->GetNbinsX(); i++){

    std::string name = fDataHist->GetXaxis()->GetBinLabel(i+1);
    int type = fTypeHist->GetBinContent(i+1);
    
    dialtypemap[name] = type;
    
  }

  return dialtypemap;
}


//******************************************************************************* 
bool ParamPull::CheckDialsValid(){
//*******************************************************************************

  return true;
  std::string helpstring = "";
  
  for (int i = 0; i < fDataHist->GetNbinsX(); i++){
    std::string name = std::string(fDataHist->GetXaxis()->GetBinLabel(i+1));

    // If dial exists its all good
    if (FitBase::GetRW()->DialIncluded(name)) continue;

    // If it doesn't but its a sample norm also continue
    if (name.find("_norm") != std::string::npos){
      ERR(WRN) << "Norm dial included in covar but not set in FitWeight." << endl;
      ERR(WRN) << "Assuming its a sample norm and skipping..." << endl;
    }

    // Dial unknown so print a help statement
    std::ostringstream tempstr;
    tempstr << "unknown_parameter " << name << " "
	    << fDataHist->GetBinContent(i+1) << " "
	    << fDataHist->GetBinContent(i+1) - fDataHist->GetBinError(i+1) << " "
	    << fDataHist->GetBinContent(i+1) + fDataHist->GetBinError(i+1) << " "
	    << fDataHist->GetBinError(i+1) << " ";

    if (!fType.empty()) tempstr << fType << endl;
    else tempstr << "FREE" << endl;
    helpstring += tempstr.str(); 
  }

  // Show statement before failing
  if (!helpstring.empty()){
    
    ERR(WRN) <<"Dial(s) included in covar but not set in FitWeight." << endl
             << "ParamPulls needs to know how you want it to be treated." << endl
             <<"Include the following lines into your card:" << endl;

    std::cout << helpstring << endl;
    sleep(10);
    throw;
    return false;
  } else {
    return true;
  }
  
}

    

//*******************************************************************************
void ParamPull::Reconfigure(){
//*******************************************************************************
  
  FitWeight* rw = FitBase::GetRW();
  
  // Get Dial Names that are valid
  std::vector<std::string> namevec  = rw->GetDialNames();
  std::vector<double>      valuevec = rw->GetDialValues();

  // Set Bin Values from RW
  for (UInt_t i = 0; i < namevec.size(); i++){

    // Loop over bins and check name matches
    std::string syst = namevec.at(i);

    // Proper Reconf using RW
    for (int j = 0; j < fMCHist->GetNbinsX(); j++){

      // If Match set value
      if (!syst.compare( fMCHist->GetXaxis()->GetBinLabel(j+1) )){
	
	double curval = rw->GetDialValue(syst, fDialOptions);
	fMCHist->SetBinContent(j+1, curval);
      }
    }
  }

  return;
};

//*******************************************************************************  
void ParamPull::ResetToy(void){
//*******************************************************************************  

  if (fDataHist) delete fDataHist;

  cout << "Resetting toy" << endl;
  cout << fDataTrue << endl;
  fDataHist = (TH1D*)fDataTrue->Clone();
  cout << "Setting name" << endl;
  fDataHist->SetNameTitle( (fName + "_data").c_str(),
			   (fName + " data" + fPlotTitles).c_str() );
  
}


    
//******************************************************************************* 
void ParamPull::SetFakeData(std::string fakeinput){
//******************************************************************************* 

  // Set from MC Setting
  if (!fakeinput.compare("MC")){

    // Copy MC into data
    if (fDataHist) delete fDataHist;
    fDataHist = (TH1D*)fMCHist->Clone();
    fDataHist->SetNameTitle( (fName + "_data").c_str(),
			     (fName + " fakedata" + fPlotTitles).c_str() );

    // Copy original data errors
    for (int i = 0; i < fDataOrig->GetNbinsX(); i++){
      fDataHist->SetBinError(i+1, fDataOrig->GetBinError(i+1) );
    }

    // Make True Toy Central Value Hist
    fDataTrue = (TH1D*) fDataHist->Clone();
    fDataTrue->SetNameTitle( (fName + "_truedata").c_str(),
			     (fName + " truedata" + fPlotTitles).c_str() );
    
  } else {

    ERR(FTL) << "Trying to set fake data for ParamPulls not from MC!" << endl;
    ERR(FTL) << "Not currently implemented.." << endl;
    throw;
    
  }
}

//*******************************************************************************  
void ParamPull::RemoveFakeData(){
//*******************************************************************************

  delete fDataHist;
  fDataHist = (TH1D*)fDataOrig->Clone();
  fDataHist->SetNameTitle( (fName + "_data").c_str(),
			   (fName + " data" + fPlotTitles).c_str() );
  
  fDataTrue = (TH1D*) fDataHist->Clone();
  fDataTrue->SetNameTitle( (fName + "_truedata").c_str(),
			   (fName + " truedata" + fPlotTitles).c_str() );

}
  
//******************************************************************************* 
double ParamPull::GetLikelihood(){
//*******************************************************************************
  
  double like = 0.0;

  switch(fCalcType){

  // Gaussian Calculation with correlations
  case kGausPull:
    like = StatUtils::GetChi2FromCov(fDataHist, fMCHist, fInvCovar, NULL);
    like *= 1E-76;
    break;

  // Default says this has no pull
  case kNoThrow:
  default:
    like = 0.0;
    break;
  }


  cout << "Likelihood = " << like << " " << fCalcType << endl;
  sleep(1);
  return like;
  
};

//*******************************************************************************
int ParamPull::GetNDOF(){
//*******************************************************************************

  int ndof = 0;

  if (fCalcType != kNoThrow){
    ndof += fDataHist->GetNbinsX();
  }

  return ndof;
};


//******************************************************************************* 
void ParamPull::ThrowCovariance(){
//*******************************************************************************

  // Reset toy for throw
  ResetToy();
  cout << "Toy Reset " << endl;
  
  // Generate random Gaussian throws
  std::vector<double> randthrows;
  for (int i = 0; i < fDataHist->GetNbinsX(); i++){
    double randtemp = 0.0;

    switch(fThrowType){
      
      // Gaussian Throws
    case kGausThrow:
      randtemp = gRandom->Gaus(0.0,1.0);
      break;
      
      // No Throws (DEFAULT)
    default:
      break;
    }
    
    randthrows.push_back(randtemp);
  }

  // Create Bin Modifications
  double totalres = 0.0;
  for (int i = 0; i < fDataHist->GetNbinsX(); i++){

    // Calc Bin Mod
    double binmod  = 0.0;
    for (int j = 0; j < fDataHist->GetNbinsX(); j++){
      binmod += (*fDecomp)(j,i) * randthrows.at(j);
    }

    // Add up fraction dif
    totalres += binmod / fDataHist->GetBinContent(i+1);
    
    // Add to current data
    fDataHist->SetBinContent(i+1,fDataHist->GetBinContent(i+1) + binmod);
  }

  // Rename
  fDataHist->SetNameTitle( (fName + "_data").c_str(),
			   (fName + " toydata" + fPlotTitles).c_str() );
  
  // Print Status
  LOG(REC) << "Created new toy histogram. Total Fractional Dif = "
	   << totalres << endl;
  return;
};


//*******************************************************************************
TH2D ParamPull::GetCovar(){
//*******************************************************************************

  TH2D tempCov = TH2D(*fInvCovar);

  for (int i = 0; i < tempCov.GetNbinsX(); i++){
    tempCov.GetXaxis()->SetBinLabel(i+1,fDataHist->GetXaxis()->GetBinLabel(i+1));
    tempCov.GetYaxis()->SetBinLabel(i+1,fDataHist->GetXaxis()->GetBinLabel(i+1));
  }

  tempCov.SetNameTitle((fName + "_INVCOV").c_str(),
			(fName + " InvertedCovariance;Dials;Dials").c_str());

  return tempCov;
}

//*******************************************************************************
TH2D ParamPull::GetFullCovar(){
//*******************************************************************************

  TH2D tempCov = TH2D(*fCovar);

  for (int i = 0; i < tempCov.GetNbinsX(); i++){
    tempCov.GetXaxis()->SetBinLabel(i+1,fDataHist->GetXaxis()->GetBinLabel(i+1));
    tempCov.GetYaxis()->SetBinLabel(i+1,fDataHist->GetXaxis()->GetBinLabel(i+1));
  }

  tempCov.SetNameTitle((fName + "_COV").c_str(),
			(fName + " Covariance;Dials;Dials").c_str());

  return tempCov;
}

//*******************************************************************************
TH2D ParamPull::GetDecompCovar(){
//*******************************************************************************

  TH2D tempCov = TH2D(*fCovar);

  for (int i = 0; i < tempCov.GetNbinsX(); i++){
    tempCov.GetXaxis()->SetBinLabel(i+1,fDataHist->GetXaxis()->GetBinLabel(i+1));
    tempCov.GetYaxis()->SetBinLabel(i+1,fDataHist->GetXaxis()->GetBinLabel(i+1));
  }

  tempCov.SetNameTitle((fName + "_DEC").c_str(),
			(fName + " Decomposition;Dials;Dials").c_str());

  return tempCov;
}


//******************************************************************************* 
void ParamPull::Write(std::string writeoptt){
//******************************************************************************* 

  fDataHist->Write();
  fMCHist->Write();
  
  GetCovar().Write();
  GetFullCovar().Write();
  GetDecompCovar().Write();
  
  return;
};

