//
// Created by eugene on 14/04/2021.
//

#ifndef ATCENTRALITYTASK_TASK_COMMONS_TRACKCUTS_HPP_
#define ATCENTRALITYTASK_TASK_COMMONS_TRACKCUTS_HPP_

#include <ati2/ATI2.hpp>
#include <at_task/UserTask.h>

class TrackCuts {

 public:
  explicit TrackCuts(ATI2::Branch *vt_branch);
  void InitCuts(ATI2::Branch* branch);
  bool TestTrack(ATI2::BranchChannel& channel) const ;

  std::size_t NPassed() const;


 private:
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

};

#endif //ATCENTRALITYTASK_TASK_COMMONS_TRACKCUTS_HPP_
