//
// Created by eugene on 01/04/2021.
//

#include "QaCentrality.hpp"

#include <TH1.h>
#include <TH2.h>
#include <bitset>

TASK_IMPL(QACentrality)

struct QABasicStruct {
  TDirectory *wd{nullptr};
  TH1 *hE{nullptr};
  TH1 *hM{nullptr};
  TH2 *hEvsM{nullptr};
  // vtx
  TH1 *hFittexVtx{nullptr};
  TH1 *hVtxZ{nullptr};
  // wfa
  TH1 *hWFA_s1{nullptr};
  TH1 *hWFA_t4{nullptr};

  /* MC-related dependencies */
  TH1* hB{nullptr};

  std::map<int, TH1 *> hESubevents;
  std::map<int, TH2 *> hEvsMSubevents;

  void Write() const {
    wd->cd();
    if (hE) hE->Write();
    if (hM) hM->Write();
    if (hEvsM) hEvsM->Write();
    if (hFittexVtx) hFittexVtx->Write();
    if (hVtxZ) hVtxZ->Write();

    if (hWFA_s1) hWFA_s1->Write();
    if (hWFA_t4) hWFA_t4->Write();

    for (auto &subevent: hESubevents) {
      subevent.second->Write();
    }
    for (auto &subevent: hEvsMSubevents) {
      subevent.second->Write();
    }
    if (hB) hB->Write();
  }
};

struct CutQA {
  std::function<bool()> Test;
  QABasicStruct qa;
};

struct QACentrality::QAStruct {
  TFile *qa_file{nullptr};
  TH1I *hTriggers{nullptr};
  std::map<int, QABasicStruct> trigger_qa;
  std::vector<CutQA> cut_qa_vector;

  void Write() const {
    qa_file->cd();
    hTriggers->Write();
    for (auto &trig_qa : trigger_qa) {
      trig_qa.second.Write();
    }
    for (auto &cut_qa : cut_qa_vector) {
      cut_qa.qa.Write();
    }
  }
};

enum eTrigger {
  kT1 = 0,
  kT2 = 1,
  kT4 = 2
};

enum ePSD {
  kPSD1 = 0,
  kPSD2 = 1,
  kPSD3 = 2
};

bool QACentrality::UseATI2() const {
  return true;
}
void QACentrality::UserInit(std::map<std::string, void *> &map) {
  evt_branch = GetInBranch("RecEventHeader");
  evt_branch->GetConfig().Print();

  try {
    sim_evt_branch = GetInBranch("SimEventHeader");
    sim_evt_branch->UseFields({{"b", sim_evt_b}}, true);
  } catch (std::out_of_range& e) { /* IGNORE */ }

  psd_branch = GetInBranch("PsdModules");
  psd_branch->UseFields({
                            {"signal", psd_signal},
                            {"number", psd_number}
                        });

  evt_branch->UseFields({
                            {"t1", evt_t1},
                            {"t2", evt_t2},
                            {"t4", evt_t4},
                            {"wfa_s1", evt_wfa_s1},
                            {"wfa_t4", evt_wfa_t4},
                            // vtx
                            {"fitted_vtx", evt_fitted_vtx},
                            {"vtx_z", evt_vtx_z},
                            {"Epsd", evt_e_psd}
                            // wfa
                        }, true);

  vt_branch = GetInBranch("VtxTracks");
  std::tie(v_nhits_vtpc1, v_nhits_vtpc2, v_nhits_mtpc,
           v_nhits_pot_vtpc1, v_nhits_pot_vtpc2, v_nhits_pot_mtpc,
           v_dca_x, v_dca_y) = vt_branch->GetVars(
      "nhits_vtpc1", "nhits_vtpc2", "nhits_mtpc",
      "nhits_pot_vtpc1", "nhits_pot_vtpc2", "nhits_pot_mtpc",
      "dcax", "dcay");


  /* Init QA struct */
  qa_struct_ = new QAStruct;
  qa_struct_->qa_file = TFile::Open("qa_centrality.root", "recreate");
  qa_struct_->hTriggers = new TH1I("hTriggers", "N events per trigger", 3, 0, 3);
  qa_struct_->hTriggers->GetXaxis()->SetBinLabel(1, "T1 (Beam)");
  qa_struct_->hTriggers->GetXaxis()->SetBinLabel(2, "T2 (Int)");
  qa_struct_->hTriggers->GetXaxis()->SetBinLabel(3, "T4 (M.b.)");

  const std::map<int, std::string> trig_names{
      {kT1, "T1"},
      {kT2, "T2"},
      {kT4, "T4"},
  };

  auto init_basic_qa = [this](QABasicStruct &qa_basic_struct) {
    const std::map<int, std::string> psd_subs_names{
        {kPSD1, "PSD1"},
        {kPSD2, "PSD2"},
        {kPSD3, "PSD3"},
    };

    qa_basic_struct.hE = new TH1F("hE", "Energy in PSD;E (GeV)", 600, 0., 6000.);
    qa_basic_struct.hM = new TH1I("hM",
                                  "Multiplicity of charged tracks in TPC;(-)", 300, 0., 300.);
    qa_basic_struct.hEvsM = new TH2F("hEvsM", "Energy in PSD vs multiplicity of TPC tracks"
                                              ";M_{TPC}, charged tracks;E (GeV)",
                                     300, 0, 300,
                                     600, 0., 6000.);

    qa_basic_struct.hFittexVtx = new TH1I("hFittedVtx", "Vtx fit OK", 2, 0, 2);
    qa_basic_struct.hVtxZ = new TH1F("hVtxZ", "Z of fitted vertex;Z_{vtx} (cm)", 800, -620., -560.);

    if (evt_wfa_s1) qa_basic_struct.hWFA_s1 = new TH1F("hWFA_s1", "WFA #Delta t_{min}", 500, 0., 50000);
    if (evt_wfa_t4) qa_basic_struct.hWFA_t4 = (TH1*) qa_basic_struct.hWFA_s1->Clone("hWFA_t4");


    for (auto psd_id : {kPSD1, kPSD2, kPSD3}) {
      qa_basic_struct.hESubevents.emplace(psd_id, new TH1F(Form("hE_%s", psd_subs_names.at(psd_id).c_str()), "",
                                                           400, 0., 4000.));
      qa_basic_struct.hEvsMSubevents
          .emplace(psd_id,
                   (TH2 *) qa_basic_struct.hEvsM->Clone(Form("hEvsM_%s", psd_subs_names.at(psd_id).c_str())));
    }

    if (sim_evt_b) qa_basic_struct.hB = new TH1F("hB", "; impact parameter (fm)", 200, 0, 20);
  };

  for (auto trig_id : {kT1, kT2, kT4}) {
    auto trig_wd = qa_struct_->qa_file->mkdir(trig_names.at(trig_id).c_str());
    trig_wd->cd();

    QABasicStruct qa_trigger;
    qa_trigger.wd = trig_wd;
    init_basic_qa(qa_trigger);

    qa_struct_->trigger_qa.emplace(trig_id, qa_trigger);
  }

  /* no cuts */
  {
    auto qa_no_cuts = qa_struct_->qa_file->mkdir("no_cuts");
    CutQA qa_t4_fitted_vtx;
    qa_t4_fitted_vtx.qa.wd = qa_no_cuts;
    init_basic_qa(qa_t4_fitted_vtx.qa);
    qa_t4_fitted_vtx.Test = [this]() -> bool { return true; };

    qa_struct_->cut_qa_vector.emplace_back(qa_t4_fitted_vtx);
  }
  /* no cuts */
  {
    auto qa_vtx_z_dir = qa_struct_->qa_file->mkdir("vtx_z_standard");
    CutQA qa_vtx_z_standard;
    qa_vtx_z_standard.qa.wd = qa_vtx_z_dir;
    init_basic_qa(qa_vtx_z_standard.qa);
    qa_vtx_z_standard.Test = [this]() -> bool {
      return (-594 < (*evt_branch)[evt_vtx_z] && (*evt_branch)[evt_vtx_z] < -590); };
    qa_struct_->cut_qa_vector.emplace_back(qa_vtx_z_standard);
  }

  if (evt_t4)
  {
    auto qa_fitted_vtx_dir = qa_struct_->qa_file->mkdir("t4__fitted_vtx");
    CutQA qa_t4_fitted_vtx;
    qa_t4_fitted_vtx.qa.wd = qa_fitted_vtx_dir;
    init_basic_qa(qa_t4_fitted_vtx.qa);
    qa_t4_fitted_vtx.Test = [this]() -> bool {
      return (*evt_branch)[evt_t4].GetBool() && (*evt_branch)[evt_fitted_vtx].GetBool();
    };
    qa_struct_->cut_qa_vector.emplace_back(qa_t4_fitted_vtx);
  }

  if (evt_t4)
  {
    auto qa_t4_vtx_z_dir = qa_struct_->qa_file->mkdir("t4__vtx_z");
    CutQA qa_t4_vtx_z;
    qa_t4_vtx_z.qa.wd = qa_t4_vtx_z_dir;
    init_basic_qa(qa_t4_vtx_z.qa);
    qa_t4_vtx_z.Test = [this]() -> bool {
      return (*evt_branch)[evt_t4].GetBool() &&
          (-594 < (*evt_branch)[evt_vtx_z] && (*evt_branch)[evt_vtx_z] < -590) &&
          TMath::Abs((*evt_branch)[evt_vtx_z] - 591.9000244) > 1e-5
          ;
    };
    qa_struct_->cut_qa_vector.emplace_back(qa_t4_vtx_z);
  }

  if (evt_t4)
  {
    auto qa_t4_vtx_z_wfa_s1_dir = qa_struct_->qa_file->mkdir("t4__vtx_z__wfa_s1");
    CutQA qa_t4_vtx_z_wfa_s1;
    qa_t4_vtx_z_wfa_s1.qa.wd = qa_t4_vtx_z_wfa_s1_dir;
    init_basic_qa(qa_t4_vtx_z_wfa_s1.qa);
    qa_t4_vtx_z_wfa_s1.Test = [this]() -> bool {
      return (*evt_branch)[evt_t4].GetBool() &&
          (-594 < (*evt_branch)[evt_vtx_z] && (*evt_branch)[evt_vtx_z] < -590) &&
          TMath::Abs((*evt_branch)[evt_vtx_z] - (-591.900024)) > 1e-5 &&
          (*evt_branch)[evt_wfa_s1].GetVal() > 4000.;
    };
    qa_struct_->cut_qa_vector.emplace_back(qa_t4_vtx_z_wfa_s1);
  }

  if (evt_t2)
  {
    auto qa_t2_vtx_z_wfa_s1_dir = qa_struct_->qa_file->mkdir("t2__vtx_z__wfa_s1");
    CutQA qa_t2_vtx_z_wfa_s1;
    qa_t2_vtx_z_wfa_s1.qa.wd = qa_t2_vtx_z_wfa_s1_dir;
    init_basic_qa(qa_t2_vtx_z_wfa_s1.qa);
    qa_t2_vtx_z_wfa_s1.Test = [this]() -> bool {
      return (*evt_branch)[evt_t2].GetBool() &&
          (-594 < (*evt_branch)[evt_vtx_z] && (*evt_branch)[evt_vtx_z] < -590) &&
          TMath::Abs((*evt_branch)[evt_vtx_z] - (-591.900024)) > 1e-5 &&
          (*evt_branch)[evt_wfa_s1].GetVal() > 4000.;
    };
    qa_struct_->cut_qa_vector.emplace_back(qa_t2_vtx_z_wfa_s1);
  }
}
void QACentrality::UserExec() {
  auto &evt = *evt_branch;


  /* get psd subevents energy */
  auto psd_subevt_e = [this]() {
    std::array<float, 3> psd_subevents_energy{0, 0, 0};
    for (auto psd_ch : psd_branch->Loop()) {
      auto mod_no = psd_ch[psd_number].GetInt();
      auto mod_signal = psd_ch[psd_signal].GetVal();
      int subevent_id;
      if ((0 <= mod_no && mod_no < 16) || mod_no == 44)
        subevent_id = kPSD1;
      else if (16 <= mod_no && mod_no < 29)
        subevent_id = kPSD2;
      else if (29 <= mod_no && mod_no < 44)
        subevent_id = kPSD3;
      else
        assert(false);
      psd_subevents_energy[subevent_id] += mod_signal;
    }
    return psd_subevents_energy;
  }();

  auto tpc_chrgd_tracks_mult = [this]() {
    int m = 0;
    for (auto &vtx_track : vt_branch->Loop()) {
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

      bool is_ok = nhits_total >= 30 &&
          nhits_vtpc > 15 &&
          nhits_pot_total > 0 &&
          ratio_nhits_nhits_pot > 0.55 &&
          ratio_nhits_nhits_pot < 1.1 &&
          abs(dca_x) < 2.0 &&
          abs(dca_y) < 1.0;
      if (is_ok) {
        ++m;
      }
    }
    return m;
  }();

  auto fill_basic_qa = [this, psd_subevt_e, tpc_chrgd_tracks_mult](QABasicStruct &qa_basic_struct) {
    auto &evt = *(this->evt_branch);

    float e_full = evt[evt_e_psd].GetVal();
    auto fit_vtx_ok = evt[evt_fitted_vtx].GetBool() ? 1 : 0;

    qa_basic_struct.hE->Fill(e_full);
    qa_basic_struct.hM->Fill(tpc_chrgd_tracks_mult);
    qa_basic_struct.hEvsM->Fill(tpc_chrgd_tracks_mult, e_full);

    qa_basic_struct.hFittexVtx->Fill(fit_vtx_ok);
    qa_basic_struct.hVtxZ->Fill(evt[evt_vtx_z]);

    if (evt_wfa_s1) qa_basic_struct.hWFA_s1->Fill(evt[evt_wfa_s1]);
    if (evt_wfa_t4) qa_basic_struct.hWFA_t4->Fill(evt[evt_wfa_t4]);
    for (auto psd_id : {kPSD1, kPSD2, kPSD3}) {
      qa_basic_struct.hESubevents[psd_id]->Fill(psd_subevt_e[psd_id]);
      qa_basic_struct.hEvsMSubevents[psd_id]->Fill(tpc_chrgd_tracks_mult, psd_subevt_e[psd_id]);
    }

    if (sim_evt_b) qa_basic_struct.hB->Fill((*sim_evt_branch)[sim_evt_b]);
  };


  if (bool(evt_t1) && bool(evt_t2) && bool(evt_t4)) {
    /* map triggers to the bitset */
    std::bitset<3> triggers_bits{0};
    triggers_bits.set(kT1, evt[evt_t1].GetBool());
    triggers_bits.set(kT2, evt[evt_t2].GetBool());
    triggers_bits.set(kT4, evt[evt_t4].GetBool());

    for (auto trig_id : {kT1, kT2, kT4}) {
      if (!triggers_bits.test(trig_id)) {
        continue;
      }

      qa_struct_->hTriggers->Fill(trig_id);

      fill_basic_qa(qa_struct_->trigger_qa[trig_id]);
    }
  }


  for (auto &qa_cut : qa_struct_->cut_qa_vector) {
    if (qa_cut.Test()) {
      fill_basic_qa(qa_cut.qa);
    }
  }

}
void QACentrality::UserFinish() {

  qa_struct_->Write();
  std::cout << "Written QA to '" << qa_struct_->qa_file->GetName() << "'" << std::endl;
  qa_struct_->qa_file->Close();
}
