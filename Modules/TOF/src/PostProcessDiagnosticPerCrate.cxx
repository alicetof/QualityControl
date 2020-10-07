// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// \file   PostProcessDiagnosticPerCrate_OLD.cxx
/// \brief  Post processing to rearrange TOF information at the level of the crate (maybe we should do the opposite..)
/// \author Nicolo' Jacazio and Francesca Ercolessi
/// \since  11/09/2020
///

#include "TOF/PostProcessDiagnosticPerCrate.h"

#include "TH2F.h"
#include "TCanvas.h"

// QC includes
#include "QualityControl/MonitorObject.h"
#include "QualityControl/QcInfoLogger.h"


using namespace o2::quality_control::postprocessing;

namespace o2::quality_control_modules::tof
{

PostProcessDiagnosticPerCrate::~PostProcessDiagnosticPerCrate()
{
}

void PostProcessDiagnosticPerCrate::initialize(Trigger, framework::ServiceRegistry& services)
{
    ILOG(Info) << "CIAO!";

     int counter = 0;
   for (auto& i : mCrates) {
     i.reset(new TH2F(Form("hCrate%i", counter), Form("Crate%i;Word;Slot", counter++),32, 0, 32, 14, 0, 14));
   }

//    Setting up services
  mDatabase = &services.get<o2::quality_control::repository::DatabaseInterface>();
}

void PostProcessDiagnosticPerCrate::update(Trigger, framework::ServiceRegistry&)
{
    ILOG(Info) << "UPDATING !";
    auto moDRM = mDatabase->retrieveMO("qc/TOF/TaskDiagnostics/", "DRMCounter");
    TH2F* hDRM = static_cast<TH2F*>(moDRM ? moDRM->getObject() : nullptr);
    if (hDRM) {
        for(int i = 1; i <= hDRM->GetNbinsY(); i++){
            for(int j = 1; j <= hDRM->GetNbinsX(); j++){
                mCrates[i-1]->SetBinContent(1, j, hDRM->GetBinContent(i, j));
            }
        }
    }
    auto moLTM = mDatabase->retrieveMO("qc/TOF/TaskDiagnostics/", "LTMCounter");
    TH2F* hLTM = static_cast<TH2F*>(moLTM ? moLTM->getObject() : nullptr);
    if (hLTM) {
        for(int i = 1; i <= hLTM->GetNbinsY(); i++){
            for(int j = 1; j <= hLTM->GetNbinsX(); j++){
                mCrates[i-1]->SetBinContent(2, j, hLTM->GetBinContent(i, j));
            }
        }
    }
    for(int slot = 0; slot < 10; slot++){
        auto moTRM = mDatabase->retrieveMO("qc/TOF/TaskDiagnostics/", Form("TRMCounterSlot%i", slot));
        TH2F* hTRM = static_cast<TH2F*>(moTRM ? moTRM->getObject() : nullptr);
        if (hTRM) {
            for(int i = 1; i <= hTRM->GetNbinsY(); i++){
                for(int j = 1; j <= hTRM->GetNbinsX(); j++){
                    mCrates[i-1]->SetBinContent(4 + slot, j, hTRM->GetBinContent(i, j));
                }
            }
        }
    }
    ILOG(Info) << "DONE UPDATING !";

}

void PostProcessDiagnosticPerCrate::finalize(Trigger, framework::ServiceRegistry&)
{
    ILOG(Info) << "FINALIZING !";

    for (auto& i : mCrates) {
        TCanvas* c = new TCanvas(i->GetName(), i->GetName());
        i->Draw();
        auto mo = std::make_shared<o2::quality_control::core::MonitorObject>(c, "PostProcessDiagnosticPerCrate", "TOF");
        mo->setIsOwner(false);
        mDatabase->storeMO(mo);

        // It should delete everything inside. Confirmed by trying to delete histo after and getting a segfault.
        delete c;
    }
    ILOG(Info) << "DONE FINALIZING !";

}

} // namespace o2::quality_control_modules::tof
