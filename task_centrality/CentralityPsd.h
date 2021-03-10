//
// Created by eugene on 01/09/2020.
//

#ifndef ATCENTRALITYTASK_TASK_CENTRALITY_CENTRALITYPSD_H
#define ATCENTRALITYTASK_TASK_CENTRALITY_CENTRALITYPSD_H

#include "at_task/Task.h"

#include <centrality/Getter.h>
#include <AnalysisTree/Detector.hpp>
#include <TTree.h>
#include <TChain.h>

#include <centrality/Getter.h>
#include <TFile.h>

#include <AnalysisTree/EventHeader.hpp>

class CentralityPsd : public UserFillTask {

 public:
 protected:
  bool UseATI2() const override { return true; }
 public:
  boost::program_options::options_description GetBoostOptions() override {
    using namespace boost::program_options;
    options_description desc(GetName() + " options");
    desc.add_options()
        ("getter-file", value(&getter_file_)->required(), "Path to centrality getter")
        ("getter-name", value(&getter_name_)->default_value("centr_getter_1d"), "Name of the getter");
    return desc;
  }
  void PreInit() override {
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
  void UserInit(std::map<std::string, void *> &branches_map) override {
    /* input */
    psd_branch = GetInBranch("PsdModules");
    psd_branch->UseFields({{"signal", psd_channel_signal}});

    /* dedicated branch for centrality */
    centrality_branch = NewBranch("Centrality", EVENT_HEADER);
    centrality_Epsd_fid = centrality_branch->NewVariable("Centrality_Epsd", FLOAT);
    centrality_branch->Freeze();

    /* attach value to the event header */
    event_header_branch = GetInBranch("RecEventHeader");
    new_rec_event_header = NewBranch("RecEventHeaderProc", EVENT_HEADER);
    new_rec_event_header->CloneVariables(event_header_branch->GetConfig());
    new_rec_event_header->CloneVariables(centrality_branch->GetConfig());
    new_rec_event_header->Freeze();
  }
  void UserExec() override {
    float total_signal = 0.;
    for (auto &channel : psd_branch->Loop()) {
      auto signal = channel[psd_channel_signal];
      total_signal += signal;
    }
    auto centrality = float(centrality_getter_->GetCentrality(total_signal));
    (*centrality_branch)[centrality_Epsd_fid] = float(centrality);

    /* centrality branch is taken first due to stupid default fields related to the vertex */
    new_rec_event_header->CopyContents(centrality_branch);
    new_rec_event_header->CopyContents(event_header_branch);
  }
  void UserFinish() override {

  }

 private:
  std::string getter_file_;
  std::string getter_name_;

  typedef AnalysisTree::Detector<AnalysisTree::Module> ModuleDetector;

  AnalysisTree::EventHeader *ana_event_header_{nullptr};
  ModuleDetector *psd{nullptr};

  std::shared_ptr<Centrality::Getter> centrality_getter_;

  ATI2::Branch *psd_branch;
  ATI2::Variable psd_channel_signal;

  ATI2::Branch *centrality_branch;
  ATI2::Variable centrality_Epsd_fid;

  ATI2::Branch *event_header_branch;
  ATI2::Branch *new_rec_event_header;

 TASK_DEF(CentralityPsd, 0);
};

#endif //ATCENTRALITYTASK_TASK_CENTRALITY_CENTRALITYPSD_H
