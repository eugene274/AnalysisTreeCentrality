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

class TrackCuts;

class CentralityPsd : public UserFillTask {

 public:
 protected:
  bool UseATI2() const override { return true; }
 public:
  boost::program_options::options_description GetBoostOptions() override;
  void PreInit() override;
  void UserInit(std::map<std::string, void *> &branches_map) override;
  void UserExec() override;
  void UserFinish() override;

 private:
  std::string getter_file_;
  std::string getter_name_;
  float energy_scale_factor{1.0};

  std::shared_ptr<Centrality::Getter> centrality_getter_;

  ATI2::Branch *psd_branch;
  ATI2::Variable psd_channel_signal;

  TrackCuts *track_cuts_;

  ATI2::Branch *centrality_branch;
  ATI2::Variable centrality_Epsd;
  ATI2::Variable multiplicity_good;

  ATI2::Branch *event_header_branch;
  ATI2::Branch *new_evt_branch;


 TASK_DEF(CentralityPsd, 0);
  ATI2::Variable e_psd;
};

#endif //ATCENTRALITYTASK_TASK_CENTRALITY_CENTRALITYPSD_H
