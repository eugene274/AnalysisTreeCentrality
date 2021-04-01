//
// Created by eugene on 01/04/2021.
//

#ifndef ATCENTRALITYTASK_QA_CENTRALITY_QACENTRALITY_HPP_
#define ATCENTRALITYTASK_QA_CENTRALITY_QACENTRALITY_HPP_

#include <at_task/Task.h>
#include <ATI2.hpp>

#include <TFile.h>

/**
 * @brief
 */
class QACentrality : public UserFillTask {

 public:

  /*
   * Trigger
   * N (T1) vs N (T2) vs N (T4)
   *
   * For each T*:
   *
   * E
   * E_p1
   * E_p2
   * E_p3
   * EvsM
   * E_p1 vs M
   * E_p2 vs M
   * E_p3 vs M
   */

  /*
   * Affection of cuts:
   * wfa_s1
   * wfa_t4
   * vtx_z
   * fitted_vtx
   */

 protected:

  struct QAStruct;

  bool UseATI2() const override;
  void UserInit(std::map<std::string, void *> &map) override;
  void UserExec() override;
  void UserFinish() override;

  // Event
  ATI2::Branch *evt_branch;
  ATI2::Variable evt_t1;
  ATI2::Variable evt_t2;
  ATI2::Variable evt_t4;
  ATI2::Variable evt_vtx_z;
  ATI2::Variable evt_e_psd;
  ATI2::Variable evt_fitted_vtx;

  // Psd
  ATI2::Branch *psd_branch;
  ATI2::Variable psd_signal;
  ATI2::Variable psd_number;

  // Vtx tracks
  ATI2::Branch *vt_branch;
  ATI2::Variable v_dca_x;
  ATI2::Variable v_dca_y;
  ATI2::Variable v_nhits_vtpc1;
  ATI2::Variable v_nhits_vtpc2;
  ATI2::Variable v_nhits_mtpc;
  ATI2::Variable v_nhits_pot_vtpc1;
  ATI2::Variable v_nhits_pot_vtpc2;
  ATI2::Variable v_nhits_pot_mtpc;


  QAStruct *qa_struct_;







 TASK_DEF(QACentrality, 1)
};

#endif //ATCENTRALITYTASK_QA_CENTRALITY_QACENTRALITY_HPP_
