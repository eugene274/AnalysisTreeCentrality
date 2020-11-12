//
// Created by eugene on 01/09/2020.
//

#ifndef ATCENTRALITYTASK_TASK_CENTRALITY_CENTRALITYPSD_H
#define ATCENTRALITYTASK_TASK_CENTRALITY_CENTRALITYPSD_H

#include "at_task/Task.h"

#include <centrality/Getter.h>
#include <core/Detector.hpp>
#include <TTree.h>
#include <TChain.h>

#include <centrality/Getter.h>
#include <TFile.h>

#include <AnalysisTree/EventHeader.hpp>

class CentralityPsd : public UserTask {

public:
  void Init(std::map<std::string, void *> &branches_map) override {
    psd = static_cast<ModuleDetector *>(branches_map.at("PsdModules"));
    in_chain_->SetBranchAddress("PsdModules", &psd);

    AnalysisTree::BranchConfig centrality_branch(out_branch_, AnalysisTree::DetType::kEventHeader);
    centrality_branch.AddField<float>("Centrality_Epsd");
    centrality_branch.AddField<float>("Epsd_total");
    centrality_Epsd_fid = centrality_branch.GetFieldId("Centrality_Epsd");
    Epsd_total_fid = centrality_branch.GetFieldId("Epsd_total");

    out_tree_->Branch(out_branch_.c_str(), &ana_event_header_);
    config_->AddBranchConfig(centrality_branch);
    out_config_->AddBranchConfig(centrality_branch);
    ana_event_header_->Init(centrality_branch);

    branches_map.emplace("Centrality", ana_event_header_);
  }
  void Exec() override {
    auto n_channel = psd->GetNumberOfChannels();

    float total_signal = 0.;
    for (int ich = 0; ich < n_channel; ++ich) {
      auto channel = psd->GetChannel(ich);
      auto signal = channel.GetSignal();
      total_signal += signal;
    }
    float centrality = centrality_getter_->GetCentrality(total_signal);
    ana_event_header_->SetField(centrality, centrality_Epsd_fid);
    ana_event_header_->SetField(total_signal, Epsd_total_fid);
  }
  void Finish() override {

  }
  boost::program_options::options_description GetBoostOptions() override {
    using namespace boost::program_options;
    options_description desc(GetName() + " options");
    desc.add_options()
        ("getter-file", value(&getter_file_)->required(), "Path to centrality getter")
        ("getter-name", value(&getter_name_)->default_value("centr_getter_1d"), "Name of the getter");
    return desc;
  }
  void PreInit() override {
    SetInputBranchNames({"PsdModules"});
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

private:
  std::string getter_file_;
  std::string getter_name_;

  typedef AnalysisTree::Detector<AnalysisTree::Module> ModuleDetector;

  AnalysisTree::EventHeader *ana_event_header_{nullptr};
  ModuleDetector *psd{nullptr};
  int centrality_Epsd_fid{-999};
  int Epsd_total_fid{-999};

  std::shared_ptr<Centrality::Getter> centrality_getter_;

TASK_DEF(CentralityPsd, 0);
};

#endif //ATCENTRALITYTASK_TASK_CENTRALITY_CENTRALITYPSD_H
