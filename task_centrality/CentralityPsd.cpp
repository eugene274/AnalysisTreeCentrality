//
// Created by eugene on 01/09/2020.
//

#include "CentralityPsd.h"
#include "TrackCuts.hpp"

TASK_IMPL(CentralityPsd)


void CentralityPsd::UserInit(std::map<std::string, void *> &branches_map) {
  /* input */
  psd_branch = GetInBranch("PsdModules");
  psd_branch->UseFields({{"signal", psd_channel_signal}});

  track_cuts_ = new TrackCuts(GetInBranch("VtxTracks"));

  /* dedicated branch for centrality */
  centrality_branch = NewBranch("Centrality", EVENT_HEADER);
  centrality_Epsd = centrality_branch->NewVariable("Centrality_Epsd", FLOAT);
  multiplicity_good = centrality_branch->NewVariable("Mgood", INTEGER);
  centrality_branch->Freeze();

  /* attach value to the event header */
  event_header_branch = GetInBranch("RecEventHeader");
  new_evt_branch = NewBranch("RecEventHeaderProc", EVENT_HEADER);
  new_evt_branch->CloneVariables(event_header_branch->GetConfig());
  new_evt_branch->CloneVariables(centrality_branch->GetConfig());
  new_evt_branch->Freeze();
}

void CentralityPsd::UserExec() {
  auto n_passed_cuts = track_cuts_->NPassed();

  float total_signal = 0.;
  for (auto &channel : psd_branch->Loop()) {
    auto signal = channel[psd_channel_signal];
    total_signal += signal;
  }
  total_signal *= energy_scale_factor;
  auto centrality = float(centrality_getter_->GetCentrality(total_signal));
  (*centrality_branch)[centrality_Epsd] = centrality;
  (*centrality_branch)[multiplicity_good] = n_passed_cuts;

  /* centrality branch is taken first due to stupid default fields related to the vertex */
  new_evt_branch->CopyContents(centrality_branch);
  new_evt_branch->CopyContents(event_header_branch);
}

void CentralityPsd::UserFinish() {

}

boost::program_options::options_description CentralityPsd::GetBoostOptions() {
  using namespace boost::program_options;
  options_description desc(GetName() + " options");
  desc.add_options()
      ("getter-file", value(&getter_file_)->required(), "Path to centrality getter")
      ("getter-name", value(&getter_name_)->default_value("centr_getter_1d"), "Name of the getter")
      ("scale-energy-factor", value(&energy_scale_factor)->default_value(1.0),
       "Energy scale factor")
      ;
  return desc;
}

void CentralityPsd::PreInit() {
  SetOutputBranchName("Centrality");

  TFile centrality_file(getter_file_.c_str(), "read");
  if (!centrality_file.IsOpen()) {
    throw std::runtime_error("Unable to open ROOT file with centrality");
  }

  centrality_getter_.reset(centrality_file.Get<Centrality::Getter>(getter_name_.c_str()));

  if (!centrality_getter_) {
    throw std::runtime_error("Centrality getter is nullptr");
  }

}
