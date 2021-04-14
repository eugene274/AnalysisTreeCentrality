//
// Created by eugene on 14/04/2021.
//

#include "TrackCuts.hpp"

void TrackCuts::InitCuts(ATI2::Branch *branch) {
  assert(branch);
  std::tie(v_nhits_vtpc1, v_nhits_vtpc2, v_nhits_mtpc,
           v_nhits_pot_vtpc1, v_nhits_pot_vtpc2, v_nhits_pot_mtpc,
           v_dca_x, v_dca_y) = vt_branch->GetVars(
      "nhits_vtpc1", "nhits_vtpc2", "nhits_mtpc",
      "nhits_pot_vtpc1", "nhits_pot_vtpc2", "nhits_pot_mtpc",
      "dcax", "dcay");
}
TrackCuts::TrackCuts(ATI2::Branch *vt_branch) : vt_branch(vt_branch) {
  InitCuts(vt_branch);
}
bool TrackCuts::TestTrack(ATI2::BranchChannel &vtx_track) const {
  int nhits_total =
      vtx_track[v_nhits_vtpc1].GetInt() +
          vtx_track[v_nhits_vtpc2].GetInt() +
          vtx_track[v_nhits_mtpc].GetInt();
  int nhits_vtpc =
      vtx_track[v_nhits_vtpc1].GetInt() +
          vtx_track[v_nhits_vtpc2].GetInt();
  int nhits_pot_total =
      vtx_track[v_nhits_pot_vtpc1].GetInt() +
          vtx_track[v_nhits_pot_vtpc2].GetInt() +
          vtx_track[v_nhits_pot_mtpc].GetInt();
  float ratio_nhits_nhits_pot = float(nhits_total) / float(nhits_pot_total);

  auto dca_x = vtx_track[v_dca_x];
  auto dca_y = vtx_track[v_dca_y];

  return nhits_total >= 30 &&
      nhits_vtpc > 15 &&
      nhits_pot_total > 0 &&
      ratio_nhits_nhits_pot > 0.55 &&
      ratio_nhits_nhits_pot < 1.1 &&
      abs(dca_x) < 2.0 &&
      abs(dca_y) < 1.0;
}
std::size_t TrackCuts::NPassed() const {
  std::size_t multiplicity = 0;
  for (auto &vtx_track : vt_branch->Loop()) {
    if (TestTrack(vtx_track)) {
      ++multiplicity;
    }
  }
  return multiplicity;
}
