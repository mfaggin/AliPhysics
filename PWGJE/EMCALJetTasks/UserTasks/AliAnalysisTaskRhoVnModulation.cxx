/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* 
 * analysis task for jet flow preparation
 *
 * this task is part of the emcal jet framework and should be run in the emcaljet train
 * the following extensions to an accepted AliVEvent are expected:
 *      - (anti-kt) jets
 *      - background estimate rho
 *      - pico tracks
 *      aod's and esd's are handled transparently
 * the task will attempt to estimate a phi-dependent background density rho 
 * by fitting vn harmonics to the dpt/dphi distribution
 *
 * author: Redmer Alexander Bertens, Utrecht Univeristy, Utrecht, Netherlands
 * rbertens@cern.ch, rbertens@nikhef.nl, r.a.bertens@uu.nl 
 */

#include <TStyle.h>
#include <TRandom3.h>
#include <TChain.h>
#include <TMath.h>
#include <TF1.h>
#include <TF2.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TProfile.h>

#include <AliAnalysisTask.h>
#include <AliAnalysisManager.h>
#include <AliCentrality.h>
#include <AliVVertex.h>
#include <AliESDEvent.h>
#include <AliAODEvent.h>

#include <AliPicoTrack.h>
#include <AliEmcalJet.h>
#include <AliRhoParameter.h>

#include "AliAnalysisTaskRhoVnModulation.h"


class AliAnalysisTaskRhoVnModulation;
using namespace std;

ClassImp(AliAnalysisTaskRhoVnModulation)

AliAnalysisTaskRhoVnModulation::AliAnalysisTaskRhoVnModulation() : AliAnalysisTaskEmcalJet("AliAnalysisTaskRhoVnModulation", kTRUE), 
    fDebug(0), fInitialized(0), fFillQAHistograms(kTRUE), fCentralityClasses(0), fNAcceptedTracks(0), fFitModulationType(kNoFit), fUsePtWeight(kTRUE), fDetectorType(kTPC), fFitModulationOptions("Q"), fRunModeType(kGrid), fDataType(kESD), fRandom(0), fMappedRunNumber(0), fInCentralitySelection(-1), fFitModulation(0), fMinPvalue(0), fMaxPvalue(1), fNameJetClones(0), fNamePicoTrackClones(0), fNameRho(0), fAbsVertexZ(10), fHistCentrality(0), fHistVertexz(0), fHistRunnumbersPhi(0), fHistRunnumbersEta(0), fHistPvaluePDF(0), fHistPvalueCDF(0), fMinDisanceRCtoLJ(0), fRandomConeRadius(0.4), fAbsVnHarmonics(kTRUE), fExcludeLeadingJetsFromFit(1.), fRebinSwapHistoOnTheFly(kTRUE), fPercentageOfFits(10.), fOutputList(0), fOutputListGood(0), fOutputListBad(0), fHistAnalysisSummary(0), fHistSwap(0), fProfV2(0), fProfV3(0), fHistPsiControl(0), fHistPsiSpread(0), fHistPsiVZEROA(0), fHistPsiVZEROC(0), fHistPsiTPC(0), fHistPsiTPCSUBA(0), fHistPsiTPCSUBB(0),
   fHistRhoVsMult(0), fHistRhoVsCent(0), fHistRhoAVsMult(0), fHistRhoAVsCent(0) {
    for(Int_t i(0); i < 10; i++) {
        fProfV2Resolution[i] = 0;
        fProfV3Resolution[i] = 0;
        fHistPicoTrackPt[i] = 0;
        fHistPicoCat1[i] = 0;
        fHistPicoCat2[i] = 0;
        fHistPicoCat3[i] = 0;
        /* fHistClusterPt[i] = 0; */
        /* fHistClusterPhi[i] = 0; */
        /* fHistClusterEta[i] = 0; */ 
        /* fHistClusterCorrPt[i] = 0; */
        /* fHistClusterCorrPhi[i] = 0; */
        /* fHistClusterCorrEta[i] = 0; */
        fHistRhoPackage[i] = 0;
        fHistRho[i] = 0;
        fHistRCPhiEta[i] = 0;
        fHistRhoVsRCPt[i] = 0;
        fHistRCPt[i] = 0;
        fHistDeltaPtDeltaPhi2[i] = 0;
        fHistDeltaPtDeltaPhi3[i] = 0;
        fHistRCPhiEtaExLJ[i] = 0;
        fHistRhoVsRCPtExLJ[i] = 0;
        fHistRCPtExLJ[i] = 0;
        fHistDeltaPtDeltaPhi2ExLJ[i] = 0;
        fHistDeltaPtDeltaPhi3ExLJ[i] = 0;
        fHistRCPhiEtaRand[i] = 0;
        fHistRhoVsRCPtRand[i] = 0;
        fHistRCPtRand[i] = 0;
        fHistDeltaPtDeltaPhi2Rand[i] = 0;
        fHistDeltaPtDeltaPhi3Rand[i] = 0;
        fHistJetPtRaw[i] = 0;
        fHistJetPt[i] = 0;
        fHistJetEtaPhi[i] = 0;
        fHistJetPtArea[i] = 0;
        fHistJetPtConstituents[i] = 0;
        fHistJetEtaRho[i] = 0;
        fHistJetPsiTPCPt[i] = 0;
        fHistJetPsiVZEROAPt[i] = 0;
        fHistJetPsiVZEROCPt[i] = 0;
        fHistDeltaPhi2VZEROA[i] = 0;
        fHistDeltaPhi2VZEROC[i] = 0;
        fHistDeltaPhi2TPC[i] = 0;
        fHistDeltaPhi3VZEROA[i] = 0;
        fHistDeltaPhi3VZEROC[i] = 0;
        fHistDeltaPhi3TPC[i] = 0;
   }
    // default constructor
}
//_____________________________________________________________________________
AliAnalysisTaskRhoVnModulation::AliAnalysisTaskRhoVnModulation(const char* name, runModeType type) : AliAnalysisTaskEmcalJet(name, kTRUE),
  fDebug(0), fInitialized(0), fFillQAHistograms(kTRUE), fCentralityClasses(0), fNAcceptedTracks(0), fFitModulationType(kNoFit), fUsePtWeight(kTRUE), fDetectorType(kTPC), fFitModulationOptions("Q"), fRunModeType(type), fDataType(kESD), fRandom(0), fMappedRunNumber(0), fInCentralitySelection(-1), fFitModulation(0), fMinPvalue(0), fMaxPvalue(1), fNameJetClones(0), fNamePicoTrackClones(0), fNameRho(0), fAbsVertexZ(10), fHistCentrality(0), fHistVertexz(0), fHistRunnumbersPhi(0), fHistRunnumbersEta(0), fHistPvaluePDF(0), fHistPvalueCDF(0), fMinDisanceRCtoLJ(0), fRandomConeRadius(0.4), fAbsVnHarmonics(kTRUE), fExcludeLeadingJetsFromFit(1.), fRebinSwapHistoOnTheFly(kTRUE), fPercentageOfFits(10.), fOutputList(0), fOutputListGood(0), fOutputListBad(0), fHistAnalysisSummary(0), fHistSwap(0), fProfV2(0), fProfV3(0), fHistPsiControl(0), fHistPsiSpread(0), fHistPsiVZEROA(0), fHistPsiVZEROC(0), fHistPsiTPC(0), fHistPsiTPCSUBA(0), fHistPsiTPCSUBB(0), 
   fHistRhoVsMult(0), fHistRhoVsCent(0), fHistRhoAVsMult(0), fHistRhoAVsCent(0) {
    for(Int_t i(0); i < 10; i++) {
        fProfV2Resolution[i] = 0;
        fProfV3Resolution[i] = 0;
        fHistPicoTrackPt[i] = 0;
        fHistPicoCat1[i] = 0;
        fHistPicoCat2[i] = 0;
        fHistPicoCat3[i] = 0;
        /* fHistClusterPt[i] = 0; */
        /* fHistClusterPhi[i] = 0; */
        /* fHistClusterEta[i] = 0; */ 
        /* fHistClusterCorrPt[i] = 0; */
        /* fHistClusterCorrPhi[i] = 0; */
        /* fHistClusterCorrEta[i] = 0; */
        fHistRhoPackage[i] = 0;
        fHistRho[i] = 0;
        fHistRCPhiEta[i] = 0;
        fHistRhoVsRCPt[i] = 0;
        fHistRCPt[i] = 0;
        fHistDeltaPtDeltaPhi2[i] = 0;
        fHistDeltaPtDeltaPhi3[i] = 0;
        fHistRCPhiEtaExLJ[i] = 0;
        fHistRhoVsRCPtExLJ[i] = 0;
        fHistRCPtExLJ[i] = 0;
        fHistDeltaPtDeltaPhi2ExLJ[i] = 0;
        fHistDeltaPtDeltaPhi3ExLJ[i] = 0;
        fHistRCPhiEtaRand[i] = 0;
        fHistRhoVsRCPtRand[i] = 0;
        fHistRCPtRand[i] = 0;
        fHistDeltaPtDeltaPhi2Rand[i] = 0;
        fHistDeltaPtDeltaPhi3Rand[i] = 0;
        fHistJetPtRaw[i] = 0;
        fHistJetPt[i] = 0;
        fHistJetEtaPhi[i] = 0;
        fHistJetPtArea[i] = 0;
        fHistJetPtConstituents[i] = 0;
        fHistJetEtaRho[i] = 0;
        fHistJetPsiTPCPt[i] = 0;
        fHistJetPsiVZEROAPt[i] = 0;
        fHistJetPsiVZEROCPt[i] = 0;
        fHistDeltaPhi2VZEROA[i] = 0;
        fHistDeltaPhi2VZEROC[i] = 0;
        fHistDeltaPhi2TPC[i] = 0;
        fHistDeltaPhi3VZEROA[i] = 0;
        fHistDeltaPhi3VZEROC[i] = 0;
        fHistDeltaPhi3TPC[i] = 0;       
    }
    // constructor
    DefineInput(0, TChain::Class());
    DefineOutput(1, TList::Class());
    switch (fRunModeType) {
        case kLocal : {
            gStyle->SetOptFit(1);
            DefineOutput(2, TList::Class());
            DefineOutput(3, TList::Class());
        } break;
        default: fDebug = -1;   // suppress debug info explicitely when not running locally
    }
}
//_____________________________________________________________________________
AliAnalysisTaskRhoVnModulation::~AliAnalysisTaskRhoVnModulation()
{
    // destructor
    if(fOutputList)             delete fOutputList;
    if(fOutputListGood)         delete fOutputListGood;
    if(fOutputListBad)          delete fOutputListBad;
    if(fFitModulation)          delete fFitModulation;
    if(fHistSwap)               delete fHistSwap;
    if(fCentralityClasses)      delete fCentralityClasses;
}
//_____________________________________________________________________________
Bool_t AliAnalysisTaskRhoVnModulation::InitializeAnalysis() 
{
    // initialize the anaysis
    if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    if(fMinDisanceRCtoLJ==0) fMinDisanceRCtoLJ = .5*fJetRadius;
    if(dynamic_cast<AliAODEvent*>(InputEvent())) fDataType = kAOD; // determine the datatype
    else if(dynamic_cast<AliESDEvent*>(InputEvent())) fDataType = kESD;
    fHistAnalysisSummary->SetBinContent(36, (int)fDataType);
    if(!fRandom) fRandom = new TRandom3(0);  // get a randomized if one hasn't been user-supplied
    switch (fFitModulationType)  {
        case kNoFit : { SetModulationFit(new TF1("fix_kNoFit", "[0]", 0, TMath::TwoPi())); } break;
        case kV2 : {
            SetModulationFit(new TF1("fit_kV2", "[0]*([1]+[2]*[3]*TMath::Cos([2]*(x-[4])))", 0, TMath::TwoPi()));
            fFitModulation->SetParameter(0, 0.);        // normalization
            fFitModulation->SetParameter(3, 0.2);       // v2
            fFitModulation->FixParameter(1, 1.);        // constant
            fFitModulation->FixParameter(2, 2.);        // constant
        } break;
        case kV3: {
            SetModulationFit(new TF1("fit_kV3", "[0]*([1]+[2]*[3]*TMath::Cos([2]*(x-[4])))", 0, TMath::TwoPi()));
            fFitModulation->SetParameter(0, 0.);        // normalization
            fFitModulation->SetParameter(3, 0.2);       // v3
            fFitModulation->FixParameter(1, 1.);        // constant
            fFitModulation->FixParameter(2, 3.);        // constant
        } break;
        default : { // for the combined fit and the 'direct fourier series' we use v2 and v3 
             SetModulationFit(new TF1("fit_kCombined", "[0]*([1]+[2]*([3]*TMath::Cos([2]*(x-[4]))+[7]*TMath::Cos([5]*(x-[6]))))", 0, TMath::TwoPi()));
             fFitModulation->SetParameter(0, 0.);       // normalization
             fFitModulation->SetParameter(3, 0.2);      // v2
             fFitModulation->FixParameter(1, 1.);       // constant
             fFitModulation->FixParameter(2, 2.);       // constant
             fFitModulation->FixParameter(5, 3.);       // constant
             fFitModulation->SetParameter(7, 0.2);      // v3
        } break;
    }
    switch (fRunModeType) {
        case kGrid : { fFitModulationOptions += "N0"; } break;
        default : break;
    }
    return kTRUE;
}
//_____________________________________________________________________________
TH1F* AliAnalysisTaskRhoVnModulation::BookTH1F(const char* name, const char* x, Int_t bins, Double_t min, Double_t max, Int_t c, Bool_t append)
{
    // book a TH1F and connect it to the output container
    if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    if(!fOutputList) return 0x0;
    TString title(name);
    if(c!=-1) { // format centrality dependent histograms accordingly
        name = Form("%s_%i", name, c);
        title += Form("_%i-%i", fCentralityClasses->At(c), fCentralityClasses->At(1+c));
    }
    title += Form(";%s;[counts]", x);
    TH1F* histogram = new TH1F(name, title.Data(), bins, min, max);
    histogram->Sumw2();
    if(append) fOutputList->Add(histogram);
    return histogram;   
}
//_____________________________________________________________________________
TH2F* AliAnalysisTaskRhoVnModulation::BookTH2F(const char* name, const char* x, const char*y, Int_t binsx, Double_t minx, Double_t maxx, Int_t binsy, Double_t miny, Double_t maxy, Int_t c, Bool_t append)
{
    // book a TH2F and connect it to the output container
    if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    if(!fOutputList) return 0x0;
    TString title(name);
    if(c!=-1) { // format centrality dependent histograms accordingly
        name = Form("%s_%i", name, c);
        title += Form("_%i-%i", fCentralityClasses->At(c), fCentralityClasses->At(1+c));
    }
    title += Form(";%s;%s", x, y);
    TH2F* histogram = new TH2F(name, title.Data(), binsx, minx, maxx, binsy, miny, maxy);
    histogram->Sumw2();
    if(append) fOutputList->Add(histogram);
    return histogram;   
}
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::UserCreateOutputObjects()
{
    // create output objects
    if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    fOutputList = new TList();
    fOutputList->SetOwner(kTRUE);
    if(!fCentralityClasses) {   // classes must be defined at this point
        Int_t c[] = {0, 20, 40, 60, 80, 100};
        fCentralityClasses = new TArrayI(sizeof(c)/sizeof(c[0]), c);
    }
    // global QA
    fHistCentrality =           BookTH1F("fHistCentrality", "centrality", 102, -2, 100);
    fHistVertexz =              BookTH1F("fHistVertexz", "vertex z (cm)", 100, -12, 12);

    // pico track kinematics
    for(Int_t i(0); i < fCentralityClasses->GetSize()-1; i++) { 
        fHistPicoTrackPt[i] =          BookTH1F("fHistPicoTrackPt", "p_{t} [GeV/c]", 100, 0, 50, i);
        if(fFillQAHistograms) {
            fHistPicoCat1[i] =             BookTH2F("fHistPicoCat1", "#eta", "#phi", 50, -1, 1, 50, 0, TMath::TwoPi(), i);
            fHistPicoCat2[i] =             BookTH2F("fHistPicoCat2", "#eta", "#phi", 50, -1, 1, 50, 0, TMath::TwoPi(), i);
            fHistPicoCat3[i] =             BookTH2F("fHistPicoCat3", "#eta", "#phi", 50, -1, 1, 50, 0, TMath::TwoPi(), i);
        }
        // emcal kinematics
        /* fHistClusterPt[i] =            BookTH1F("fHistClusterPt", "p_{t} [GeV/c]", 100, 0, 100, i); */
        /* fHistClusterPhi[i] =           BookTH1F("fHistClusterPhi", "#phi", 100, 0, TMath::TwoPi(), i); */
        /* fHistClusterEta[i] =           BookTH1F("fHistClusterEta", "#eta", 100, -5, 5); */

        // emcal kinematics after hadronic correction
        /* fHistClusterCorrPt[i] =        BookTH1F("fHistClusterCorrPt", "p_{t} [GeV/c]", 100, 0, 100, i); */
        /* fHistClusterCorrPhi[i] =       BookTH1F("fHistClusterCorrPhi", "#phi", 100, 0, TMath::TwoPi(), i); */
        /* fHistClusterCorrEta[i] =       BookTH1F("fHistClusterCorrEta", "#eta", 100, -5, 5, i); */
    }

    // event plane estimates and quality
    fHistPsiControl =           new TProfile("fHistPsiControl", "fHistPsiControl", 10, 0, 10);
    fHistPsiControl->Sumw2();
    fHistPsiSpread =            new TProfile("fHistPsiSpread", "fHistPsiSpread", 4, 0, 4);
    fHistPsiSpread->Sumw2();
    fHistPsiControl->GetXaxis()->SetBinLabel(1, "<#Psi_{2, VZEROA}>");
    fHistPsiControl->GetXaxis()->SetBinLabel(2, "<#Psi_{2, VZEROC}>");
    fHistPsiControl->GetXaxis()->SetBinLabel(3, "<#Psi_{2, TPC}>");
    fHistPsiControl->GetXaxis()->SetBinLabel(4, "<#Psi_{2, TPC, #eta < 0}>");
    fHistPsiControl->GetXaxis()->SetBinLabel(5, "<#Psi_{2, TPC, #eta > 0}>");
    fHistPsiControl->GetXaxis()->SetBinLabel(1, "<#Psi_{3, VZEROA}>");
    fHistPsiControl->GetXaxis()->SetBinLabel(2, "<#Psi_{3, VZEROC}>");
    fHistPsiControl->GetXaxis()->SetBinLabel(3, "<#Psi_{3, TPC}>");
    fHistPsiControl->GetXaxis()->SetBinLabel(4, "<#Psi_{3, TPC, #eta < 0}>");
    fHistPsiControl->GetXaxis()->SetBinLabel(5, "<#Psi_{3, TPC, #eta > 0}>");
    fHistPsiSpread->GetXaxis()->SetBinLabel(1, "<#Psi_{2, VZEROA} - #Psi_{2, VZEROC}>");
    fHistPsiSpread->GetXaxis()->SetBinLabel(2, "<#Psi_{2, VZEROC} - #Psi_{2, TPC}>");
    fHistPsiSpread->GetXaxis()->SetBinLabel(3, "<#Psi_{2, VZEROC} - #Psi_{2, TPC}>");
    fHistPsiSpread->GetXaxis()->SetBinLabel(4, "<#Psi_{2, TPC, #eta < 0} - #Psi_{2, TPC, #eta > 0}>");
    fOutputList->Add(fHistPsiControl);
    fOutputList->Add(fHistPsiSpread);
    fHistPsiVZEROA =            BookTH1F("fHistPsiVZEROA", "#Psi_{VZEROA}", 100, -.5*TMath::Pi(), .5*TMath::Pi());
    fHistPsiVZEROC =            BookTH1F("fHistPsiVZEROC", "#Psi_{VZEROC}", 100, -.5*TMath::Pi(), .5*TMath::Pi());
    fHistPsiTPC =               BookTH1F("fHistPsiTPC", "#Psi_{TPC}", 100, -.5*TMath::Pi(), .5*TMath::Pi());
    fHistPsiTPCSUBA =           BookTH1F("fHistPsiTPCSUBA", "#Psi_{TPC, #eta < 0}", 100, -.5*TMath::Pi(), .5*TMath::Pi());
    fHistPsiTPCSUBB =           BookTH1F("fHistPsiTPCSUBB", "#Psi_{TPC, #eta > 0}", 100, -.5*TMath::Pi(), .5*TMath::Pi());

    // background
    for(Int_t i(0); i < fCentralityClasses->GetSize()-1; i ++) {
        fHistRhoPackage[i] =           BookTH1F("fHistRhoPackage",  "#rho [GeV/c]", 100, 0, 150, i);
        fHistRho[i] =                  BookTH1F("fHistRho", "#rho [GeV/c]", 100, 0, 150, i);
    }
    fHistRhoVsMult =            BookTH2F("fHistRhoVsMult", "multiplicity", "#rho [GeV/c]", 100, 0, 4000, 100, 0, 250);
    fHistRhoVsCent =            BookTH2F("fHistRhoVsCent", "centrality", "#rho [GeV/c]", 100, 0, 100, 100, 0, 250);
    fHistRhoAVsMult =           BookTH2F("fHistRhoAVsMult", "multiplicity", "#rho * A (jet) [GeV/c]", 100, 0, 4000, 100, 0, 50);
    fHistRhoAVsCent =           BookTH2F("fHistRhoAVsCent", "centrality", "#rho * A (jet) [GeV/c]", 100, 0, 100, 100, 0, 50);

    // delta pt distributions
    for(Int_t i(0); i < fCentralityClasses->GetSize()-1; i ++) {
        fHistRCPhiEta[i] =             BookTH2F("fHistRCPhiEta", "#phi (RC)", "#eta (RC)", 100, 0, TMath::TwoPi(), 100, -1, 1, i);
        fHistRhoVsRCPt[i] =            BookTH2F("fHistRhoVsRCPt", "p_{t} (RC) [GeV/c]", "#rho * A (RC) [GeV/c]", 100, 0, 300, 100, 0, 350, i);
        fHistRCPt[i] =                 BookTH1F("fHistRCPt", "p_{t} (RC) [GeV/c]", 130, -20, 150, i);
        fHistRCPhiEtaExLJ[i] =         BookTH2F("fHistRCPhiEtaExLJ", "#phi (RC)", "#eta (RC)", 100, 0, TMath::TwoPi(), 100, -1, 1, i);
        fHistDeltaPtDeltaPhi2[i] =     BookTH2F("fHistDeltaPtDeltaPhi2", "#phi - #Psi_{TPC}", "#delta p_{t} [GeV/c]", 100, 0, TMath::TwoPi(), 100, -50, 100, i);
        fHistDeltaPtDeltaPhi3[i] =     BookTH2F("fHistDeltaPtDeltaPhi3", "#phi - #Psi_{TPC}", "#delta p_{t} [GeV/c]", 100, 0, TMath::TwoPi(), 100, -50, 100, i);
        fHistRhoVsRCPtExLJ[i] =        BookTH2F("fHistRhoVsRCPtExLJ", "p_{t} (RC) [GeV/c]", "#rho * A (RC) [GeV/c]", 100, 0, 300, 100, 0, 350, i);
        fHistRCPtExLJ[i] =             BookTH1F("fHistRCPtExLJ", "p_{t} (RC) [GeV/c]", 130, -20, 150, i);
        fHistRCPhiEtaRand[i] =         BookTH2F("fHistRCPhiEtaRand", "#phi (RC)", "#eta (RC)", 100, 0, TMath::TwoPi(), 100, -1, 1, i);
        fHistDeltaPtDeltaPhi2ExLJ[i] = BookTH2F("fHistDeltaPtDeltaPhi2ExLJ", "#phi - #Psi_{TPC}", "#delta p_{t} [GeV/c]", 100, 0, TMath::TwoPi(), 100, -50, 100, i);
        fHistDeltaPtDeltaPhi3ExLJ[i] = BookTH2F("fHistDeltaPtDeltaPhi3ExLJ", "#phi - #Psi_{TPC}", "#delta p_{t} [GeV/c]", 100, 0, TMath::TwoPi(), 100, -50, 100, i);
        fHistRhoVsRCPtRand[i] =        BookTH2F("fHistRhoVsRCPtRand", "p_{t} (RC) [GeV/c]", "#rho * A (RC) [GeV/c]", 100, 0, 300, 100, 0, 350, i);
        fHistRCPtRand[i] =             BookTH1F("fHistRCPtRand", "p_{t} (RC) [GeV/c]", 130, -20, 150, i);
        fHistDeltaPtDeltaPhi2Rand[i] =  BookTH2F("fHistDeltaPtDeltaPhi2Rand", "#phi - #Psi_{TPC}", "#delta p_{t} [GeV/c]", 100, 0, TMath::TwoPi(), 100, -50, 100, i);
        fHistDeltaPtDeltaPhi3Rand[i] =  BookTH2F("fHistDeltaPtDeltaPhi3Rand", "#phi - #Psi_{TPC}", "#delta p_{t} [GeV/c]", 100, 0, TMath::TwoPi(), 100, -50, 100, i);
        // jet histograms (after kinematic cuts)
        fHistJetPtRaw[i] =             BookTH1F("fHistJetPtRaw", "p_{t} RAW [GeV/c]", 200, -50, 150, i);
        fHistJetPt[i] =                BookTH1F("fHistJetPt", "p_{t} [GeV/c]", 350, -100, 250, i);
        fHistJetEtaPhi[i] =            BookTH2F("fHistJetEtaPhi", "#eta", "#phi", 100, -1, 1, 100, 0, TMath::TwoPi(), i);
        fHistJetPtArea[i] =            BookTH2F("fHistJetPtArea", "p_{t} [GeV/c]", "Area", 350, -100, 250, 60, 0, 0.3, i);
        fHistJetPtConstituents[i] =    BookTH2F("fHistJetPtConstituents", "p_{t} [GeV/c]", "Area", 350, -100, 250, 60, 0, 150, i);
        fHistJetEtaRho[i] =            BookTH2F("fHistJetEtaRho", "#eta", "#rho", 100, -1, 1, 100, 0, 300, i);
        // in plane and out of plane spectra
        fHistJetPsiTPCPt[i] =          BookTH2F("fHistJetPsiTPCPt", "#phi_{jet} - #Psi_{2, TPC}", "p_{t} [GeV/c]", 100, 0., TMath::TwoPi(), 700, -100, 250, i);
        fHistJetPsiVZEROAPt[i] =       BookTH2F("fHistJetPsiVZEROAPt", "#phi_{jet} - #Psi_{2, VZEROA}", "p_{t} [GeV/c]", 100, 0., TMath::TwoPi(), 700, -100, 250, i);
        fHistJetPsiVZEROCPt[i] =       BookTH2F("fHistJetPsiVZEROCPt", "#phi_{jet} - #Psi_{V2, ZEROC}", "p_{t} [GeV/c]", 100, 0., TMath::TwoPi(), 700, -100, 250, i);
        // phi minus psi
        fHistDeltaPhi2VZEROA[i] =       BookTH1F("fHistDeltaPhi2VZEROA", "#phi_{jet} - #Psi_{2, VZEROA}", 100, 0, TMath::TwoPi(), i);
        fHistDeltaPhi2VZEROC[i] =       BookTH1F("fHistDeltaPhi2VZEROC", "#phi_{jet} - #Psi_{2, VZEROC}", 100, 0, TMath::TwoPi(), i);
        fHistDeltaPhi2TPC[i] =          BookTH1F("fHistDeltaPhi2TPC", "#phi_{jet} - #Psi_{2, TPC}", 100, 0, TMath::TwoPi(), i);
        fHistDeltaPhi3VZEROA[i] =       BookTH1F("fHistDeltaPhi3VZEROA", "#phi_{jet} - #Psi_{2, VZEROA}", 100, 0, TMath::TwoPi(), i);
        fHistDeltaPhi3VZEROC[i] =       BookTH1F("fHistDeltaPhi3VZEROC", "#phi_{jet} - #Psi_{2, VZEROC}", 100, 0, TMath::TwoPi(), i);
        fHistDeltaPhi3TPC[i] =          BookTH1F("fHistDeltaPhi3TPC", "#phi_{jet} - #Psi_{2, TPC}", 100, 0, TMath::TwoPi(), i);

        fProfV2Resolution[i] = new TProfile(Form("fProfV2Resolution_%i", i), Form("fProfV2Resolution_%i", i), 8, -0.5, 7.5);
        fProfV2Resolution[i]->GetXaxis()->SetBinLabel(1, "<cos(2(#Psi_{a} - #Psi_{b}))>");
        fProfV2Resolution[i]->GetXaxis()->SetBinLabel(2, "<cos(2(#Psi_{b} - #Psi_{a}))>");
        fProfV2Resolution[i]->GetXaxis()->SetBinLabel(3, "<cos(2(#Psi_{VZEROA} - #Psi_{VZEROC}))>");
        fProfV2Resolution[i]->GetXaxis()->SetBinLabel(4, "<cos(2(#Psi_{VZEROC} - #Psi_{VZEROA}))>");
        fProfV2Resolution[i]->GetXaxis()->SetBinLabel(5, "<cos(2(#Psi_{VZEROA} - #Psi_{TPC}))>");
        fProfV2Resolution[i]->GetXaxis()->SetBinLabel(6, "<cos(2(#Psi_{TPC} - #Psi_{VZEROA}))>");
        fProfV2Resolution[i]->GetXaxis()->SetBinLabel(7, "<cos(2(#Psi_{VZEROC} - #Psi_{TPC}))>");
        fProfV2Resolution[i]->GetXaxis()->SetBinLabel(8, "<cos(2(#Psi_{TPC} - #Psi_{VZEROC}))>");
        fOutputList->Add(fProfV2Resolution[i]); 
        fProfV3Resolution[i] = new TProfile(Form("fProfV3Resolution_%i", i), Form("fProfV3Resolution_%i", i), 8, -0.5, 7.5);
        fProfV3Resolution[i]->GetXaxis()->SetBinLabel(1, "<cos(3(#Psi_{a} - #Psi_{b}))>");
        fProfV3Resolution[i]->GetXaxis()->SetBinLabel(2, "<cos(3(#Psi_{b} - #Psi_{a}))>");
        fProfV3Resolution[i]->GetXaxis()->SetBinLabel(3, "<cos(3(#Psi_{VZEROA} - #Psi_{VZEROC}))>");
        fProfV3Resolution[i]->GetXaxis()->SetBinLabel(4, "<cos(3(#Psi_{VZEROC} - #Psi_{VZEROA}))>");
        fProfV3Resolution[i]->GetXaxis()->SetBinLabel(5, "<cos(3(#Psi_{VZEROA} - #Psi_{TPC}))>");
        fProfV3Resolution[i]->GetXaxis()->SetBinLabel(6, "<cos(3(#Psi_{TPC} - #Psi_{VZEROA}))>");
        fProfV3Resolution[i]->GetXaxis()->SetBinLabel(7, "<cos(3(#Psi_{VZEROC} - #Psi_{TPC}))>");
        fProfV3Resolution[i]->GetXaxis()->SetBinLabel(8, "<cos(3(#Psi_{TPC} - #Psi_{VZEROC}))>");
        fOutputList->Add(fProfV3Resolution[i]); 
    }
    // cdf and pdf of chisquare distribution
    fHistPvaluePDF = BookTH1F("fHistPvaluePDF", "PDF #chi^{2}", 500, 0, 1);
    fHistPvalueCDF = BookTH1F("fHistPvalueCDF", "CDF #chi^{2}", 500, 0, 1);
    // vn profile
    Float_t temp[fCentralityClasses->GetSize()];
    for(Int_t i(0); i < fCentralityClasses->GetSize(); i++) temp[i] = fCentralityClasses->At(i);
    fProfV2 = new TProfile("fProfV2", "fProfV2", fCentralityClasses->GetSize()-1, temp);
    fProfV3 = new TProfile("fProfV3", "fProfV3", fCentralityClasses->GetSize()-1, temp);
    fOutputList->Add(fProfV2);
    fOutputList->Add(fProfV3);

    // analysis summary histrogram, saves all relevant analysis settigns
    fHistAnalysisSummary = BookTH1F("fHistAnalysisSummary", "flag", 42, -0.5, 42.5);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(1, "fJetRadius"); 
    fHistAnalysisSummary->SetBinContent(1, fJetRadius);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(2, "fPtBiasJetTrack");
    fHistAnalysisSummary->SetBinContent(2, fPtBiasJetTrack);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(3, "fPtBiasJetClus");
    fHistAnalysisSummary->SetBinContent(3, fPtBiasJetClus);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(4, "fJetPtCut");
    fHistAnalysisSummary->SetBinContent(4, fJetPtCut);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(5, "fJetAreaCut");
    fHistAnalysisSummary->SetBinContent(5, fJetAreaCut);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(6, "fPercAreaCut");
    fHistAnalysisSummary->SetBinContent(6, fPercAreaCut);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(7, "fAreaEmcCut");
    fHistAnalysisSummary->SetBinContent(7, fAreaEmcCut);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(8, "fJetMinEta");
    fHistAnalysisSummary->SetBinContent(8, fJetMinEta);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(9, "fJetMaxEta");
    fHistAnalysisSummary->SetBinContent(9, fJetMaxEta);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(10, "fJetMinPhi");
    fHistAnalysisSummary->SetBinContent(10, fJetMinPhi);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(11, "fJetMaxPhi");
    fHistAnalysisSummary->SetBinContent(11, fJetMaxPhi);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(12, "fMaxClusterPt");
    fHistAnalysisSummary->SetBinContent(12, fMaxClusterPt);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(13, "fMaxTrackPt");
    fHistAnalysisSummary->SetBinContent(13, fMaxTrackPt);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(14, "fLeadingHadronType");
    fHistAnalysisSummary->SetBinContent(14, fLeadingHadronType);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(15, "fAnaType");
    fHistAnalysisSummary->SetBinContent(15, fAnaType);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(16, "fForceBeamType");
    fHistAnalysisSummary->SetBinContent(16, fForceBeamType);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(17, "fMinCent");
    fHistAnalysisSummary->SetBinContent(17, fMinCent);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(18, "fMaxCent");
    fHistAnalysisSummary->SetBinContent(18, fMaxCent);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(19, "fMinVz");
    fHistAnalysisSummary->SetBinContent(19, fMinVz);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(20, "fMaxVz");
    fHistAnalysisSummary->SetBinContent(20, fMaxVz);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(21, "fOffTrigger");
    fHistAnalysisSummary->SetBinContent(21, fOffTrigger);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(22, "fClusPtCut");
    fHistAnalysisSummary->SetBinContent(22, fClusPtCut);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(23, "fTrackPtCut");
    fHistAnalysisSummary->SetBinContent(23, fTrackPtCut);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(24, "fTrackMinEta");
    fHistAnalysisSummary->SetBinContent(24, fTrackMinEta);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(25, "fTrackMaxEta");
    fHistAnalysisSummary->SetBinContent(25, fTrackMaxEta);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(26, "fTrackMinPhi");
    fHistAnalysisSummary->SetBinContent(26, fTrackMinPhi);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(27, "fTrackMaxPhi");
    fHistAnalysisSummary->SetBinContent(27, fTrackMaxPhi);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(28, "fClusTimeCutLow");
    fHistAnalysisSummary->SetBinContent(28, fClusTimeCutLow);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(29, "fClusTimeCutUp");
    fHistAnalysisSummary->SetBinContent(29, fClusTimeCutUp);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(30, "fMinPtTrackInEmcal");
    fHistAnalysisSummary->SetBinContent(30, fMinPtTrackInEmcal);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(31, "fEventPlaneVsEmcal");
    fHistAnalysisSummary->SetBinContent(31, fEventPlaneVsEmcal);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(32, "fMinEventPlane");
    fHistAnalysisSummary->SetBinContent(32, fMaxEventPlane);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(33, "fRandomConeRadius");
    fHistAnalysisSummary->SetBinContent(33, fRandomConeRadius);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(34, "fitModulationType");
    fHistAnalysisSummary->SetBinContent(34, (int)fFitModulationType);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(35, "runModeType");
    fHistAnalysisSummary->SetBinContent(35, (int)fRunModeType);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(36, "data type");
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(37, "iterator");
    fHistAnalysisSummary->SetBinContent(37, 1.);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(38, "fMinPvalue");
    fHistAnalysisSummary->SetBinContent(38, fMinPvalue);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(39, "fMaxPvalue");
    fHistAnalysisSummary->SetBinContent(39, fMaxPvalue);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(40, "fExcludeLeadingJetsFromFit");
    fHistAnalysisSummary->SetBinContent(40, fExcludeLeadingJetsFromFit);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(41, "fRebinSwapHistoOnTheFly");
    fHistAnalysisSummary->SetBinContent(41, (int)fRebinSwapHistoOnTheFly);
    fHistAnalysisSummary->GetXaxis()->SetBinLabel(42, "fUsePtWeight");
    fHistAnalysisSummary->SetBinContent(42, (int)fUsePtWeight);

    if(fFillQAHistograms) {
        fHistRunnumbersEta = new TH2F("fHistRunnumbersEta", "fHistRunnumbersEta", 100, -.5, 99.5, 100, -1.1, 1.1);
        fHistRunnumbersEta->Sumw2();
        fOutputList->Add(fHistRunnumbersEta);
        fHistRunnumbersPhi = new TH2F("fHistRunnumbersPhi", "fHistRunnumbersPhi", 100, -.5, 99.5, 100, -0.2, TMath::TwoPi()+0.2);
        fHistRunnumbersPhi->Sumw2();
        fOutputList->Add(fHistRunnumbersPhi);
    }

    fHistSwap = new TH1F("fHistSwap", "fHistSwap", 20, 0, TMath::TwoPi());
    fHistSwap->Sumw2();
    PostData(1, fOutputList);

    switch (fRunModeType) {
        case kLocal : {
            fOutputListGood = new TList();
            fOutputListGood->SetOwner(kTRUE);
            fOutputListBad = new TList();
            fOutputListBad->SetOwner(kTRUE);
            PostData(2, fOutputListGood);
            PostData(3, fOutputListBad);
        } break;
        default: break;
    }
}
//_____________________________________________________________________________
Bool_t AliAnalysisTaskRhoVnModulation::Run()
{
    // user exec: execute once for each event
    if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    if(!fInitialized) fInitialized = InitializeAnalysis();
    // reject the event if expected data is missing
    if(!PassesCuts(InputEvent())) return kFALSE;
    if(!(fTracks||fJets||fRho)) return kFALSE;
    if(!fCaloClusters && fDebug > 0) printf(" > Warning: couldn't retreive calo clusters! < \n");
    // [0][0] psi2a     [1,0]   psi2c
    // [0][1] psi3a     [1,1]   psi3c
    Double_t vzero[2][2];
    CalculateEventPlaneVZERO(vzero);
    // [0] psi2         [1] psi3
    // [2] psi2 a       [3] psi2 b
    // [4] psi3 a       [5] psi3 b
    Double_t tpc[6];
    CalculateEventPlaneTPC(tpc);
    // arrays which will hold the fit parameters
    Double_t fitParameters[] = {0,0,0,0,0,0,0,0,0};
    Double_t psi2(-1), psi3(-1), psi2b(-1), psi3b(-1);
    switch (fDetectorType) {    // determine the detector type for the rho fit
        case kTPC :     { psi2 = tpc[0];        psi3 = tpc[1]; }        break;
        case kTPCSUB :  { psi2 = tpc[2];        psi3 = tpc[4];
                          psi2b = tpc[3];       psi3b = tpc[5]; }       break;
        case kVZEROA :  { psi2 = vzero[0][0];   psi3 = vzero[0][1]; }   break;  
        case kVZEROC :  { psi2 = vzero[1][0];   psi3 = vzero[1][1]; }   break;
        default : break;
    }

    switch (fFitModulationType) { // do the fits
        case kNoFit : { fFitModulation->FixParameter(0, RhoVal()); } break;
        case kV2 : {
            if(CorrectRho(fitParameters, psi2, psi3, psi2b, psi3b)) {
                fProfV2->Fill(fCent, fFitModulation->GetParameter(3));
                CalculateEventPlaneResolution(vzero, tpc);
            }
        } break;
        case kV3 : {
            if(CorrectRho(fitParameters, psi2, psi3, psi2b, psi3b)) {
                fProfV3->Fill(fCent, fFitModulation->GetParameter(3));
                CalculateEventPlaneResolution(vzero, tpc);
            }
        } break;
        case kUser : {
             CorrectRho(fitParameters, psi2, psi3, psi2b, psi3b);
        } break;
        default : {
            if(CorrectRho(fitParameters, psi2, psi3, psi2b, psi3b)) {
                fProfV2->Fill(fCent, fFitModulation->GetParameter(3));
                fProfV3->Fill(fCent, fFitModulation->GetParameter(7));
                CalculateEventPlaneResolution(vzero, tpc);
            }
        } break;
    }
    // fill a number of histograms 
    FillHistogramsAfterSubtraction(vzero, tpc);

    // send the output to the connected output container
    PostData(1, fOutputList);
    switch (fRunModeType) {
        case kLocal : {
            PostData(2, fOutputListGood);
            PostData(3, fOutputListBad);
        } break;
        default: break;
    }
    return kTRUE;
}
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::CalculateEventPlaneVZERO(Double_t vzero[2][2]) const 
{
    // grab the UNCALIBRATED vzero event plane
    if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    Double_t qxa2(0), qya2(0), qxc2(0), qyc2(0);    // for psi2
    Double_t qxa3(0), qya3(0), qxc3(0), qyc3(0);    // for psi3
    for(Int_t iVZERO(0); iVZERO < 64; iVZERO++) {
        Double_t phi(TMath::PiOver4()*(.5+iVZERO%8)), /* eta(0), */ weight(InputEvent()->GetVZEROEqMultiplicity(iVZERO));
//        (iVZERO<32) ? eta = -3.45+.5*(iVZERO/8) : eta = 4.8-.6*((iVZERO/8)-4);
        if(iVZERO<32) {
            qxa2 += weight*TMath::Cos(2.*phi);
            qya2 += weight*TMath::Sin(2.*phi);
            qxa3 += weight*TMath::Cos(3.*phi);
            qya3 += weight*TMath::Sin(3.*phi);
        }
        else {
            qxc2 += weight*TMath::Cos(2.*phi);
            qyc2 += weight*TMath::Sin(2.*phi);
            qxc3 += weight*TMath::Cos(3.*phi);
            qyc3 += weight*TMath::Sin(3.*phi);
       }
    }
    vzero[0][0] = .5*TMath::ATan2(qya2, qxa2);
    vzero[1][0] = .5*TMath::ATan2(qyc2, qxc2);
    vzero[0][1] = (1./3.)*TMath::ATan2(qya3, qxa3);
    vzero[1][1] = (1./3.)*TMath::ATan2(qyc3, qxc3);
}
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::CalculateEventPlaneTPC(Double_t* tpc)
{
   // grab the TPC event plane
   if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
   fNAcceptedTracks = 0;                // reset the track counter
   Double_t qx2(0), qy2(0);     // for psi2
   Double_t qx3(0), qy3(0);     // for psi3
   Double_t qx2a(0), qy2a(0), qx2b(0), qy2b(0); // for psi2 a and b
   Double_t qx3a(0), qy3a(0), qx3b(0), qy3b(0); // for psi3 a and b
   if(fTracks) {
       Float_t excludeInEta[] = {-999, -999};
       if(fExcludeLeadingJetsFromFit > 0 ) {    // remove the leading jet from ep estimate
           AliEmcalJet* leadingJet[] = {0x0, 0x0};
           static Int_t lJets[9999] = {-1};
           GetSortedArray(lJets, fJets);
           for(Int_t i(0); i < fJets->GetEntriesFast(); i++) {     // get the two leading jets
               if (1 + i > fJets->GetEntriesFast()) break;
               leadingJet[0] = static_cast<AliEmcalJet*>(fJets->At(lJets[i]));
               leadingJet[1] = static_cast<AliEmcalJet*>(fJets->At(lJets[i+1]));
               if(PassesCuts(leadingJet[0]) && PassesCuts(leadingJet[1])) break;
           }
           if(leadingJet[0] && leadingJet[1]) {
               for(Int_t i(0); i < 2; i++) excludeInEta[i] = leadingJet[i]->Eta();
           }
       }
       Int_t iTracks(fTracks->GetEntriesFast());
       for(Int_t iTPC(0); iTPC < iTracks; iTPC++) {
           AliVTrack* track = static_cast<AliVTrack*>(fTracks->At(iTPC));
           if(!PassesCuts(track) || track->Pt() < .15 || track->Pt() > 5.) continue;
           if(fExcludeLeadingJetsFromFit > 0 &&( (TMath::Abs(track->Eta() - excludeInEta[0]) < fJetRadius*fExcludeLeadingJetsFromFit ) || (TMath::Abs(track->Eta()) - fJetRadius - fJetMaxEta ) > 0 )) continue;
           fNAcceptedTracks++;
           qx2+= TMath::Cos(2.*track->Phi());
           qy2+= TMath::Sin(2.*track->Phi());
           qx3+= TMath::Cos(3.*track->Phi());
           qy3+= TMath::Sin(3.*track->Phi());
           if(track->Eta() < 0) {       // A side, negative eta
               qx2a+= TMath::Cos(2.*track->Phi());
               qy2a+= TMath::Sin(2.*track->Phi());
               qx3a+= TMath::Cos(3.*track->Phi());
               qy3a+= TMath::Sin(3.*track->Phi());
           } else {                     // B side, positive eta
               qx2b+= TMath::Cos(2.*track->Phi());
               qy2b+= TMath::Sin(2.*track->Phi());
               qx3b+= TMath::Cos(3.*track->Phi());
               qy3b+= TMath::Sin(3.*track->Phi());
           }
       }
   }
   tpc[0] = .5*TMath::ATan2(qy2, qx2);
   tpc[1] = (1./3.)*TMath::ATan2(qy3, qx3);
   tpc[2] = .5*TMath::ATan2(qy2a, qx2a);
   tpc[3] = .5*TMath::ATan2(qy2b, qx2b);
   tpc[4] = (1./3.)*TMath::ATan2(qy3a, qx3a);
   tpc[5] = (1./3.)*TMath::ATan2(qy3b, qx3b);
} 
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::CalculateEventPlaneResolution(Double_t vzero[2][2], Double_t* tpc) const
{
    // fill the profiles for the resolution parameters
    if(fDebug > 1) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    fProfV2Resolution[fInCentralitySelection]->Fill(0., TMath::Cos(2.*(tpc[2] - tpc[3])));
    fProfV2Resolution[fInCentralitySelection]->Fill(1., TMath::Cos(2.*(tpc[3] - tpc[2])));
    fProfV2Resolution[fInCentralitySelection]->Fill(2., TMath::Cos(2.*(vzero[0][0] - vzero[1][0])));
    fProfV2Resolution[fInCentralitySelection]->Fill(3., TMath::Cos(2.*(vzero[1][0] - vzero[0][0])));
    fProfV2Resolution[fInCentralitySelection]->Fill(4., TMath::Cos(2.*(vzero[0][0] - tpc[0])));
    fProfV2Resolution[fInCentralitySelection]->Fill(5., TMath::Cos(2.*(tpc[0] - vzero[0][0])));
    fProfV2Resolution[fInCentralitySelection]->Fill(6., TMath::Cos(2.*(vzero[1][0] - tpc[0])));
    fProfV2Resolution[fInCentralitySelection]->Fill(7., TMath::Cos(2.*(tpc[0] - vzero[1][0])));
    fProfV3Resolution[fInCentralitySelection]->Fill(0., TMath::Cos(3.*(tpc[2] - tpc[3])));
    fProfV3Resolution[fInCentralitySelection]->Fill(1., TMath::Cos(3.*(tpc[3] - tpc[2])));
    fProfV3Resolution[fInCentralitySelection]->Fill(2., TMath::Cos(3.*(vzero[0][0] - vzero[1][0])));
    fProfV3Resolution[fInCentralitySelection]->Fill(3., TMath::Cos(3.*(vzero[1][0] - vzero[0][0])));
    fProfV3Resolution[fInCentralitySelection]->Fill(4., TMath::Cos(3.*(vzero[0][0] - tpc[0])));
    fProfV3Resolution[fInCentralitySelection]->Fill(5., TMath::Cos(3.*(tpc[0] - vzero[0][0])));
    fProfV3Resolution[fInCentralitySelection]->Fill(6., TMath::Cos(3.*(vzero[1][0] - tpc[0])));
    fProfV3Resolution[fInCentralitySelection]->Fill(7., TMath::Cos(3.*(tpc[0] - vzero[1][0])));
}
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::CalculateRandomCone(Float_t &pt, Float_t &eta, Float_t &phi, 
        AliEmcalJet* jet, Bool_t randomize) const
{
    // get a random cone
    if(fDebug > 1) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    pt = 0; eta = 0; phi = 0;
    Float_t etaJet(999), phiJet(999), dJet(999);        // no jet: same as jet very far away
    if(jet) { // if a leading jet is given, use its kinematic properties
        etaJet = jet->Eta();
        phiJet = jet->Phi();
    }
    // force the random cones to at least be within detector acceptance
    Float_t minPhi(fJetMinPhi), maxPhi(fJetMaxPhi);
    if(maxPhi > TMath::TwoPi()) maxPhi = TMath::TwoPi();
    if(minPhi < 0 ) minPhi = 0;
    Float_t diffRcRJR(TMath::Abs(fRandomConeRadius-fJetRadius));
    // construct a random cone and see if it's far away enough from the leading jet
    Int_t attempts(1000);
    while(kTRUE) {
        attempts--;
        eta = gRandom->Uniform(fJetMinEta+diffRcRJR, fJetMaxEta-diffRcRJR);
        phi = gRandom->Uniform(minPhi, maxPhi);

        dJet = TMath::Sqrt((etaJet-eta)*(etaJet-eta)+(phiJet-phi)*(phiJet-phi));
        if(dJet > fMinDisanceRCtoLJ) break;
        else if (attempts == 0) {
            printf(" > No random cone after 1000 tries, giving up ... !\n");
            return;
        }
    }
    if(fTracks) {
        Int_t iTracks(fTracks->GetEntriesFast());
        for(Int_t i(0); i < iTracks; i++) {
            AliVTrack* track = static_cast<AliVTrack*>(fTracks->At(i));
            if(!PassesCuts(track)) continue;
            Float_t etaTrack(track->Eta()), phiTrack(track->Phi()), ptTrack(track->Pt());
            // if requested, randomize eta and phi to destroy any correlated fluctuations
            if(randomize) {
                etaTrack = gRandom->Uniform(fTrackMinEta, fTrackMaxEta);
                phiTrack = gRandom->Uniform(minPhi, maxPhi);
            }
            // get distance from cone
            if(TMath::Abs(phiTrack-phi) > TMath::Abs(phiTrack - phi + TMath::TwoPi())) phiTrack+=TMath::TwoPi();
            if(TMath::Abs(phiTrack-phi) > TMath::Abs(phiTrack - phi - TMath::TwoPi())) phiTrack-=TMath::TwoPi();
            if(TMath::Sqrt(TMath::Abs((etaTrack-eta)*(etaTrack-eta)+(phiTrack-phi)*(phiTrack-phi))) <= fRandomConeRadius) pt+=ptTrack;
        }
    }
}
//_____________________________________________________________________________
Bool_t AliAnalysisTaskRhoVnModulation::CorrectRho(Double_t* params, Double_t psi2, Double_t psi3, Double_t psi2b, Double_t psi3b) 
{
    // get rho' -> rho(phi)
    // two routines are available
    //  [1]  fitting a fourier expansion to the de/dphi distribution
    //  [2]  getting vn from a fourier series around dn/dphi (see below for info)
    //  this function will return kTRUE if the fit passes a set of quality criteria
    if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    TString detector("");
    switch (fDetectorType) {
        case kTPC : detector+="TPC";
            break;
        case kTPCSUB : detector+="kTPCSUB";
            break;
        case kVZEROA : detector+="VZEROA";
            break;
        case kVZEROC : detector+="VZEROC";
            break;
        default: break;
    }
    Int_t iTracks(fTracks->GetEntriesFast());
    Double_t excludeInEta[] = {-999, -999};
    Double_t excludeInPhi[] = {-999, -999};
    Double_t excludeInPt[]  = {-999, -999};
    if(iTracks <= 0 || RhoVal() <= 0 ) return kFALSE;   // no use fitting an empty event ...
    if(fExcludeLeadingJetsFromFit > 0 ) {
        AliEmcalJet* leadingJet[] = {0x0, 0x0};
        static Int_t lJets[9999] = {-1};
        GetSortedArray(lJets, fJets);
        for(Int_t i(0); i < fJets->GetEntriesFast(); i++) {     // get the two leading jets
            if (1 + i > fJets->GetEntriesFast()) break;
            leadingJet[0] = static_cast<AliEmcalJet*>(fJets->At(lJets[i]));
            leadingJet[1] = static_cast<AliEmcalJet*>(fJets->At(lJets[i+1]));
            if(PassesCuts(leadingJet[0]) && PassesCuts(leadingJet[1])) break;
        }
        if(leadingJet[0] && leadingJet[1]) {
            for(Int_t i(0); i < 2; i++) {
                excludeInEta[i] = leadingJet[i]->Eta();
                excludeInPhi[i] = leadingJet[i]->Phi();
                excludeInPt[i]  = leadingJet[i]->Pt();
            }
        }
    }
    fHistSwap->Reset();                 // clear the histogram
    TH1F _tempSwap;
    if(fRebinSwapHistoOnTheFly) {
        if(fNAcceptedTracks < 49) fNAcceptedTracks = 49;       // avoid aliasing effects
        _tempSwap = TH1F("_tempSwap", "_tempSwap", TMath::CeilNint(TMath::Sqrt(fNAcceptedTracks)), 0, TMath::TwoPi());
    }
    else _tempSwap = *fHistSwap;         // now _tempSwap holds the desired histo
    for(Int_t i(0); i < iTracks; i++) {
            AliVTrack* track = static_cast<AliVTrack*>(fTracks->At(i));
            if(fExcludeLeadingJetsFromFit > 0 &&( (TMath::Abs(track->Eta() - excludeInEta[0]) < fJetRadius*fExcludeLeadingJetsFromFit ) || (TMath::Abs(track->Eta()) - fJetRadius - fJetMaxEta ) > 0 )) continue;
            if(!PassesCuts(track) || track->Pt() > 5 || track->Pt() < 0.15) continue;
            if(fDetectorType == kTPCSUB && psi2 > -1000 && track->Eta() < 0 ) continue;
            else if (fDetectorType == kTPCSUB && psi2 < -1000 && track->Eta() > 0 ) continue;
            if(fUsePtWeight) _tempSwap.Fill(track->Phi(), track->Pt());
            else _tempSwap.Fill(track->Phi());
    }
    for(Int_t i(0); i < _tempSwap.GetXaxis()->GetNbins(); i++) _tempSwap.SetBinError(1+i, TMath::Sqrt(_tempSwap.GetBinContent(1+i)));
    fFitModulation->SetParameter(0, RhoVal());
    switch (fFitModulationType) {
        case kNoFit : { fFitModulation->FixParameter(0, RhoVal() ); 
        } break;
        case kV2 : { 
            fFitModulation->FixParameter(4, psi2); 
        } break;
        case kV3 : { 
            fFitModulation->FixParameter(4, psi3); 
        } break;
        case kCombined : { 
            fFitModulation->FixParameter(4, psi2); 
            fFitModulation->FixParameter(6, psi3);
        } break;
        case kFourierSeries : {
            // in this approach, an explicit calculation will be made of vn = sqrt(xn^2+yn^2)
            // where x[y] = Integrate[r(phi)cos[sin](n phi)dphi, 0, 2pi]
            Double_t cos2(0), sin2(0), cos3(0), sin3(0), sumPt(0);
            for(Int_t i(0); i < iTracks; i++) {
                AliVTrack* track = static_cast<AliVTrack*>(fTracks->At(i));
                if(!PassesCuts(track) || track->Pt() > 5 || track->Pt() < 0.15) continue;
                sumPt += track->Pt();
                cos2 += track->Pt()*TMath::Cos(2*PhaseShift(track->Phi()-psi2)); 
                sin2 += track->Pt()*TMath::Sin(2*PhaseShift(track->Phi()-psi2));
                cos3 += track->Pt()*TMath::Cos(3*PhaseShift(track->Phi()-psi3)); 
                sin3 += track->Pt()*TMath::Sin(3*PhaseShift(track->Phi()-psi3));
            }
            fFitModulation->SetParameter(3, TMath::Sqrt(cos2*cos2+sin2*sin2)/RhoVal());
            fFitModulation->SetParameter(4, psi2);
            fFitModulation->SetParameter(6, psi3);
            fFitModulation->SetParameter(7, TMath::Sqrt(cos3*cos3+sin3*sin3)/RhoVal());
        }
        default : break;
    }
    if(fDetectorType == kTPCSUB && psi2 > -1000 ) { // do the magic for the subevent case
        Double_t v2(fFitModulation->GetParameter(3)), v3(fFitModulation->GetParameter(7));
        CorrectRho(params, -9999, -9999, psi2b, psi3b);
        v2 += fFitModulation->GetParameter(3);
        v3 += fFitModulation->GetParameter(7);
        fFitModulation->SetParameter(3, v2/2.);
        fFitModulation->SetParameter(7, v3/3.);
    }
    _tempSwap.Fit(fFitModulation, fFitModulationOptions.Data(), "", 0, TMath::TwoPi());
    // the quality of the fit is evaluated from 1 - the cdf of the chi square distribution
    Double_t CDF(1.-ChiSquareCDF(fFitModulation->GetNDF(), fFitModulation->GetChisquare()));
//    Double_t PDF(ChiSquarePDF(fFitModulation->GetNDF(), fFitModulation->GetChisquare()));
    fHistPvalueCDF->Fill(CDF);
//    fHistPvaluePDF->Fill(PDF);
    if(CDF > fMinPvalue && CDF < fMaxPvalue && ( fAbsVnHarmonics && fFitModulation->GetMinimum(0, TMath::TwoPi()) > 0)) { // fit quality
        // for LOCAL didactic purposes, save the  best and the worst fits
        // this routine can produce a lot of output histograms (it's not memory 'safe') and will not work on GRID 
        // since the output will become unmergeable (i.e. different nodes may produce conflicting output)
        switch (fRunModeType) {
            case kLocal : {
                if(fRandom->Uniform(0, 100) > fPercentageOfFits) break;
                static Int_t didacticCounterBest(0);
                TProfile* didacticProfile = (TProfile*)_tempSwap.Clone(Form("Fit_%i_1-CDF_%.3f_cen_%i_%s", didacticCounterBest, CDF, fInCentralitySelection, detector.Data()));
                TF1* didactifFit = (TF1*)fFitModulation->Clone(Form("fit_%i_CDF_%.3f_cen_%i_%s", didacticCounterBest, CDF, fInCentralitySelection, detector.Data()));
                didacticProfile->GetListOfFunctions()->Add(didactifFit);
                fOutputListGood->Add(didacticProfile);
                didacticCounterBest++;
                TH2F* didacticSurface = BookTH2F(Form("surface_%s", didacticProfile->GetName()), "#phi", "#eta", 50, 0, TMath::TwoPi(), 50, -1, 1, -1, kFALSE);
                for(Int_t i(0); i < iTracks; i++) {
                    AliVTrack* track = static_cast<AliVTrack*>(fTracks->At(i));
                    if(PassesCuts(track)) {
                        if(fUsePtWeight) didacticSurface->Fill(track->Phi(), track->Eta(), track->Pt());
                        else didacticSurface->Fill(track->Phi(), track->Eta());
                    }
                }
                if(fExcludeLeadingJetsFromFit) {       // visualize the excluded region
                    TF2 *f2 = new TF2(Form("%s_LJ", didacticSurface->GetName()),"[0]*TMath::Gaus(x,[1],[2])*TMath::Gaus(y,[3],[4])", 0, TMath::TwoPi(), -1, 1);
                    f2->SetParameters(excludeInPt[0]/3.,excludeInPhi[0],.1,excludeInEta[0],.1);
                    didacticSurface->GetListOfFunctions()->Add(f2);
                    TF2 *f3 = new TF2(Form("%s_NLJ", didacticSurface->GetName()),"[0]*TMath::Gaus(x,[1],[2])*TMath::Gaus(y,[3],[4])", 0, TMath::TwoPi(), -1, 1);
                    f3->SetParameters(excludeInPt[1]/3.,excludeInPhi[1],.1,excludeInEta[1],.1);
                    f3->SetLineColor(kGreen);
                    didacticSurface->GetListOfFunctions()->Add(f3);
                }
                fOutputListGood->Add(didacticSurface);
            } break;
            default : break;
        }
    } else {    // if the fit is of poor quality revert to the original rho estimate
        switch (fRunModeType) { // again see if we want to save the fit
            case kLocal : {
                static Int_t didacticCounterWorst(0);
                if(fRandom->Uniform(0, 100) > fPercentageOfFits) break;
                TProfile* didacticProfile = (TProfile*)_tempSwap.Clone(Form("Fit_%i_1-CDF_%.3f_cen_%i_%s", didacticCounterWorst, CDF, fInCentralitySelection, detector.Data() ));
                TF1* didactifFit = (TF1*)fFitModulation->Clone(Form("fit_%i_p_%.3f_cen_%i_%s", didacticCounterWorst, CDF, fInCentralitySelection, detector.Data()));
                didacticProfile->GetListOfFunctions()->Add(didactifFit);
                fOutputListBad->Add(didacticProfile);
                didacticCounterWorst++;
                } break;
            default : break;
        }
        switch (fFitModulationType) {
            case kNoFit : break;        // nothing to do
            case kUser : break;         // FIXME not implemented yet
            case kCombined : fFitModulation->SetParameter(7, 0);        // no break
            case kFourierSeries : fFitModulation->SetParameter(7, 0);   // no break
            default : { // needs to be done if there was a poor fit
                 fFitModulation->SetParameter(3, 0);
                 fFitModulation->SetParameter(0, RhoVal());
            } break;
        }
        return kFALSE;  // return false if the fit is rejected
    }
    for(Int_t i(0); i < fFitModulation->GetNpar(); i++) params[i] = fFitModulation->GetParameter(i);
    return kTRUE;
}
//_____________________________________________________________________________
Bool_t AliAnalysisTaskRhoVnModulation::PassesCuts(AliVEvent* event)
{
    // event cuts
    if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    if(!event) return kFALSE;
    if(TMath::Abs(InputEvent()->GetPrimaryVertex()->GetZ()) > 10.) return kFALSE;
    // aod and esd specific checks
    switch (fDataType) {
       case kESD: {
            AliESDEvent* esdEvent = static_cast<AliESDEvent*>(InputEvent());
            if( (!esdEvent) || (TMath::Abs(esdEvent->GetPrimaryVertexSPD()->GetZ() - esdEvent->GetPrimaryVertex()->GetZ()) > .5) ) return kFALSE; 
       } break;
       case kAOD: {
            AliAODEvent* aodEvent = static_cast<AliAODEvent*>(InputEvent());
            if( (!aodEvent) || (TMath::Abs(aodEvent->GetPrimaryVertexSPD()->GetZ() - aodEvent->GetPrimaryVertex()->GetZ()) > .5) ) return kFALSE; 
       } break;
       default: break;
    }
    fCent = InputEvent()->GetCentrality()->GetCentralityPercentile("V0M");
    if(fCent <= 0 || fCent >= 100 || TMath::Abs(fCent-InputEvent()->GetCentrality()->GetCentralityPercentile("TRK")) > 5.) return kFALSE;
    // determine centrality class
    for(Int_t i(0); i < fCentralityClasses->GetSize()-1; i++) {
        if(fCent > fCentralityClasses->At(i) && fCent < fCentralityClasses->At(1+i)) {
            fInCentralitySelection = i;
            break; }
    } 
    if(fFillQAHistograms) FillQAHistograms(event);
    return kTRUE;
}
//_____________________________________________________________________________
Bool_t AliAnalysisTaskRhoVnModulation::PassesCuts(const AliVCluster* cluster) const
{
    // cluster cuts
    if(fDebug > 1) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    if(!cluster) return kFALSE;
    return kTRUE;
}
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::FillHistogramsAfterSubtraction(Double_t vzero[2][2], Double_t* tpc) const
{
    // fill histograms 
    if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    FillTrackHistograms();
    /* FillClusterHistograms(); */
    FillJetHistograms(vzero, tpc); 
    /* FillCorrectedClusterHistograms(); */
    FillEventPlaneHistograms(vzero, tpc);
    FillRhoHistograms();
    switch (fDetectorType) {    // determine the detector type for the rho fit
        case kTPC :     { FillDeltaPtHistograms(tpc[0], tpc[1]); }              break;
        case kTPCSUB :  { FillDeltaPtHistograms(tpc[2], tpc[4]);
                          FillDeltaPtHistograms(tpc[3], tpc[5]); }              break;
        case kVZEROA :  { FillDeltaPtHistograms(vzero[0][0], vzero[0][1]); }    break;
        case kVZEROC :  { FillDeltaPtHistograms(vzero[1][0], vzero[1][1]); }    break;
        default : break;
    }
    FillDeltaPhiHistograms(vzero, tpc);
}
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::FillTrackHistograms() const
{
    // fill track histograms
    if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    Int_t iTracks(fTracks->GetEntriesFast());
    for(Int_t i(0); i < iTracks; i++) {
        AliVTrack* track = static_cast<AliVTrack*>(fTracks->At(i));
        if(!PassesCuts(track)) continue;
        fHistPicoTrackPt[fInCentralitySelection]->Fill(track->Pt());
        if(fFillQAHistograms) FillQAHistograms(track);
    }
}
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::FillClusterHistograms() const
{
    // fill cluster histograms
    if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    /* Int_t iClusters(fCaloClusters->GetEntriesFast());
    for(Int_t i(0); i < iClusters; i++) {
        AliVCluster* cluster = static_cast<AliVCluster*>(fCaloClusters->At(iClusters));
        if (!PassesCuts(cluster)) continue;
        TLorentzVector clusterLorentzVector;
        cluster->GetMomentum(clusterLorentzVector, const_cast<Double_t*>(fVertex));
        fHistClusterPt[fInCentralitySelection]->Fill(clusterLorentzVector.Pt());
        fHistClusterEta[fInCentralitySelection]->Fill(clusterLorentzVector.Eta());
        fHistClusterPhi[fInCentralitySelection]->Fill(clusterLorentzVector.Phi());
    }
    return; */
}
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::FillCorrectedClusterHistograms() const
{
    // fill clusters after hadronic correction FIXME implement
    if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
}
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::FillEventPlaneHistograms(Double_t vzero[2][2], Double_t* tpc) const
{
    // fill event plane histograms
    if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    fHistPsiControl->Fill(0.5, vzero[0][0]);    // vzero a psi2
    fHistPsiControl->Fill(1.5, vzero[1][0]);    // vzero c psi2
    fHistPsiControl->Fill(2.5, tpc[0]);         // tpc psi 2
    fHistPsiControl->Fill(3.5, tpc[2]);         // tpc sub a psi 2
    fHistPsiControl->Fill(4.5, tpc[3]);         // tpc sub b psi 2
    fHistPsiControl->Fill(5.5, vzero[0][1]);    // vzero a psi3
    fHistPsiControl->Fill(6.5, vzero[1][1]);    // vzero b psi3
    fHistPsiControl->Fill(7.5, tpc[1]);         // tpc psi 3
    fHistPsiControl->Fill(8.5, tpc[4]);         // tpc sub a psi3
    fHistPsiControl->Fill(9.5, tpc[5]);         // tpc sub b psi3
    fHistPsiVZEROA->Fill(vzero[0][0]);
    fHistPsiVZEROC->Fill(vzero[1][0]);
    fHistPsiTPC->Fill(tpc[0]);
    fHistPsiTPCSUBA->Fill(tpc[2]);
    fHistPsiTPCSUBB->Fill(tpc[3]);
    fHistPsiSpread->Fill(0.5, TMath::Abs(vzero[0][0]-vzero[1][0]));
    fHistPsiSpread->Fill(1.5, TMath::Abs(vzero[0][0]-tpc[0]));
    fHistPsiSpread->Fill(2.5, TMath::Abs(vzero[1][0]-tpc[0]));
    fHistPsiSpread->Fill(3.5, TMath::Abs(tpc[2]-tpc[3]));
}
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::FillRhoHistograms() const
{
    // fill rho histograms
    if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    fHistRhoPackage[fInCentralitySelection]->Fill(RhoVal());    // save the rho estimate from the emcal jet package
    // get multiplicity FIXME inefficient
    Int_t iTracks(fTracks->GetEntriesFast()), mult(0), iJets(fJets->GetEntriesFast());
    for(Int_t i(0); i < iTracks; i ++) { if(PassesCuts(static_cast<AliVTrack*>(fTracks->At(i)))) mult++; }
    Double_t rho(RhoVal(TMath::Pi(), TMath::Pi(), fRho->GetVal()));
    fHistRho[fInCentralitySelection]->Fill(rho);
    fHistRhoVsMult->Fill(mult, rho);
    fHistRhoVsCent->Fill(fCent, rho);
    for(Int_t i(0); i < iJets; i++) {
        AliEmcalJet* jet = static_cast<AliEmcalJet*>(fJets->At(i));
        if(!PassesCuts(jet)) continue;
        fHistRhoAVsMult->Fill(mult, rho * jet->Area());
        fHistRhoAVsCent->Fill(fCent, rho * jet->Area());
    }
}
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::FillDeltaPtHistograms(Double_t psi2, Double_t psi3) const
{
    // fill delta pt histograms
    if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    Int_t i(0), maxCones(20);
    AliEmcalJet* leadingJet(0x0);
    static Int_t sJets[9999] = {-1};
    GetSortedArray(sJets, fJets);
    do { // get the leading jet 
        leadingJet = static_cast<AliEmcalJet*>(fJets->At(sJets[i]));
        i++;
    }
    while (!PassesCuts(leadingJet)&&i<fJets->GetEntriesFast()); 
    if(!leadingJet && fDebug > 0) printf(" > failed to retrieve leading jet ! < \n");
    const Float_t areaRC = fRandomConeRadius*fRandomConeRadius*TMath::Pi();
    // we're retrieved the leading jet, now get a random cone
    for(i = 0; i < maxCones; i++) {
       Float_t pt(0), eta(0), phi(0);
       // get a random cone without constraints on leading jet position
       CalculateRandomCone(pt, eta, phi, 0x0);
       if(pt > 0) {
           fHistRCPhiEta[fInCentralitySelection]->Fill(phi, eta);
           fHistRhoVsRCPt[fInCentralitySelection]->Fill(pt, RhoVal(phi, fJetRadius, fRho->GetVal())*areaRC);
           fHistRCPt[fInCentralitySelection]->Fill(pt);
           fHistDeltaPtDeltaPhi2[fInCentralitySelection]->Fill(PhaseShift(phi-psi2), pt - areaRC*RhoVal(phi, fJetRadius, fRho->GetVal()));
           fHistDeltaPtDeltaPhi3[fInCentralitySelection]->Fill(PhaseShift(phi-psi3), pt - areaRC*RhoVal(phi, fJetRadius, fRho->GetVal()));
       }
       // get a random cone excluding leading jet area
       CalculateRandomCone(pt, eta, phi, leadingJet);
       if(pt > 0) {
           fHistRCPhiEtaExLJ[fInCentralitySelection]->Fill(phi, eta);
           fHistRhoVsRCPtExLJ[fInCentralitySelection]->Fill(pt, RhoVal(phi, fJetRadius, fRho->GetVal())*areaRC);
           fHistRCPtExLJ[fInCentralitySelection]->Fill(pt);
           fHistDeltaPtDeltaPhi2ExLJ[fInCentralitySelection]->Fill(PhaseShift(phi-psi2), pt - areaRC*RhoVal(phi, fJetRadius, fRho->GetVal()));
           fHistDeltaPtDeltaPhi3ExLJ[fInCentralitySelection]->Fill(PhaseShift(phi-psi3), pt - areaRC*RhoVal(phi, fJetRadius, fRho->GetVal()));
       }
       // get a random cone in an event with randomized phi and eta
       CalculateRandomCone(pt, eta, phi, 0x0, kTRUE);
       if( pt > 0) {
           fHistRCPhiEtaRand[fInCentralitySelection]->Fill(phi, eta);
           fHistRhoVsRCPtRand[fInCentralitySelection]->Fill(pt, RhoVal(phi, fJetRadius, fRho->GetVal())*areaRC);
           fHistRCPtRand[fInCentralitySelection]->Fill(pt);
           fHistDeltaPtDeltaPhi2Rand[fInCentralitySelection]->Fill(PhaseShift(phi-psi2), pt - areaRC*RhoVal(phi, fJetRadius, fRho->GetVal()));
           fHistDeltaPtDeltaPhi3Rand[fInCentralitySelection]->Fill(PhaseShift(phi-psi3), pt - areaRC*RhoVal(phi, fJetRadius, fRho->GetVal()));
       }
    } 
}
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::FillJetHistograms(Double_t vzero[2][2], Double_t* tpc) const
{
    // fill jet histograms
    if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
    Int_t iJets(fJets->GetEntriesFast());
    for(Int_t i(0); i < iJets; i++) {
        AliEmcalJet* jet = static_cast<AliEmcalJet*>(fJets->At(i));
        if(!PassesCuts(jet)) continue;
        Double_t pt(jet->Pt()), area(jet->Area()), eta(jet->Eta()), phi(jet->Phi());
        Double_t rho(RhoVal(phi, fJetRadius, fRho->GetVal()));
        fHistJetPtRaw[fInCentralitySelection]->Fill(pt);
        fHistJetPt[fInCentralitySelection]->Fill(pt-area*rho);
        fHistJetEtaPhi[fInCentralitySelection]->Fill(eta, phi);
        fHistJetPtArea[fInCentralitySelection]->Fill(pt-area*rho, area);
        fHistJetPsiTPCPt[fInCentralitySelection]->Fill(PhaseShift(phi-tpc[0]), pt-area*rho);
        fHistJetPsiVZEROAPt[fInCentralitySelection]->Fill(PhaseShift(phi-vzero[0][0]), pt-area*rho);
        fHistJetPsiVZEROCPt[fInCentralitySelection]->Fill(PhaseShift(phi-vzero[1][0]), pt-area*rho);
        fHistJetPtConstituents[fInCentralitySelection]->Fill(pt-area*rho, jet->Nch());
        fHistJetEtaRho[fInCentralitySelection]->Fill(eta, pt/area);
    }
}
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::FillDeltaPhiHistograms(Double_t vzero[2][2], Double_t* tpc) const
{
   // fill phi minus psi histograms
   if(fDebug > 0) printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
   if(fTracks) {
       Int_t iTracks(fTracks->GetEntriesFast());
       for(Int_t iTPC(0); iTPC < iTracks; iTPC++) {
           AliVTrack* track = static_cast<AliVTrack*>(fTracks->At(iTPC));
           if(!PassesCuts(track)) continue;
           fHistDeltaPhi2VZEROA[fInCentralitySelection]->Fill(PhaseShift(track->Phi()-vzero[0][0]));
           fHistDeltaPhi2VZEROC[fInCentralitySelection]->Fill(PhaseShift(track->Phi()-vzero[1][0]));
           fHistDeltaPhi2TPC[fInCentralitySelection]->Fill(PhaseShift(track->Phi()-tpc[0]));
           fHistDeltaPhi3VZEROA[fInCentralitySelection]->Fill(PhaseShift(track->Phi()-vzero[0][1]));
           fHistDeltaPhi3VZEROC[fInCentralitySelection]->Fill(PhaseShift(track->Phi()-vzero[1][1]));
           fHistDeltaPhi3TPC[fInCentralitySelection]->Fill(PhaseShift(track->Phi()-tpc[1]));
       }
   }
}
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::FillQAHistograms(AliVTrack* vtrack) const
{
    // fill qa histograms for pico tracks
    if(!vtrack) return;
    AliPicoTrack* track = static_cast<AliPicoTrack*>(vtrack);
    fHistRunnumbersPhi->Fill(fMappedRunNumber, track->Phi());
    fHistRunnumbersEta->Fill(fMappedRunNumber, track->Eta());
    Int_t type((int)(track->GetTrackType()));
    switch (type) {
        case 0:
           fHistPicoCat1[fInCentralitySelection]->Fill(track->Eta(), track->Phi()); 
           break;
        case 1:
           fHistPicoCat2[fInCentralitySelection]->Fill(track->Eta(), track->Phi()); 
           break;
        case 2:
           fHistPicoCat3[fInCentralitySelection]->Fill(track->Eta(), track->Phi()); 
           break;
        default: break;
    }
}
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::FillQAHistograms(AliVEvent* vevent) 
{
    // fill qa histograms for events
    if(!vevent) return;
    fHistVertexz->Fill(vevent->GetPrimaryVertex()->GetZ());
    fHistCentrality->Fill(fCent);
    Int_t runNumber(InputEvent()->GetRunNumber());
    Int_t runs[] = {167813, 167988, 168066, 168068, 168069, 168076, 168104, 168212, 168311, 168322, 168325, 168341, 168361, 168362, 168458, 168460, 168461, 168992, 169091, 169094, 169138, 169143, 169167, 169417, 169835, 169837, 169838, 169846, 169855, 169858, 169859, 169923, 169956, 170027, 170036, 170081, 169975, 169981, 170038, 170040, 170083, 170084, 170085, 170088, 170089, 170091, 170152, 170155, 170159, 170163, 170193, 170195, 170203, 170204, 170205, 170228, 170230, 170264, 170268, 170269, 170270, 170306, 170308, 170309};
    for(fMappedRunNumber = 0; fMappedRunNumber < 64; fMappedRunNumber++) {
        if(runs[fMappedRunNumber]==runNumber) break;
    }
}
//_____________________________________________________________________________
void AliAnalysisTaskRhoVnModulation::Terminate(Option_t *)
{
    // terminate
    switch (fRunModeType) {
        case kLocal : {
        printf("__FILE__ = %s \n __LINE __ %i , __FUNC__ %s \n ", __FILE__, __LINE__, __func__);
        if(fFillQAHistograms) {
            Int_t runs[] = {167813, 167988, 168066, 168068, 168069, 168076, 168104, 168212, 168311, 168322, 168325, 168341, 168361, 168362, 168458, 168460, 168461, 168992, 169091, 169094, 169138, 169143, 169167, 169417, 169835, 169837, 169838, 169846, 169855, 169858, 169859, 169923, 169956, 170027, 170036, 170081, 169975, 169981, 170038, 170040, 170083, 170084, 170085, 170088, 170089, 170091, 170152, 170155, 170159, 170163, 170193, 170195, 170203, 170204, 170205, 170228, 170230, 170264, 170268, 170269, 170270, 170306, 170308, 170309};
            for(Int_t i(0); i < 64; i++) { 
                fHistRunnumbersPhi->GetXaxis()->SetBinLabel(i+1, Form("%i", runs[i]));
                fHistRunnumbersEta->GetXaxis()->SetBinLabel(i+1, Form("%i", runs[i]));
            }
            fHistRunnumbersPhi->GetXaxis()->SetBinLabel(65, "undetermined");
            fHistRunnumbersEta->GetXaxis()->SetBinLabel(65, "undetermined");
        }
        AliAnalysisTaskRhoVnModulation::Dump();
        for(Int_t i(0); i < fHistAnalysisSummary->GetXaxis()->GetNbins(); i++) printf( " > flag: %s \t content %.2f \n", fHistAnalysisSummary->GetXaxis()->GetBinLabel(1+i), fHistAnalysisSummary->GetBinContent(1+i));
        } break;
        default : break;
    }
}
//_____________________________________________________________________________
