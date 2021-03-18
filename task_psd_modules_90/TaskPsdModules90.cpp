//
// Created by eugene on 18/03/2021.
//

#include "TaskPsdModules90.hpp"

TASK_IMPL(TaskPsdModules90)

void TaskPsdModules90::UserInit(std::map<std::string, void *> &map) {
  BypassBranches();
  psd_sections = GetInBranch("PsdSections");
  psd_modules = GetInBranch("PsdModules");
  psd_modules->UseFields({
                             {"number", psd_modules_no}
                         });
  psd_sections->UseFields({
                              {"module_no", psd_sections_module_no},
                              {"section_ord_no", psd_section_ord_no},
                              {"signal", psd_sections_signal}
                          });
  psd_modules_90 = NewBranch("PsdModules90", MODULES);
  psd_modules_90->CloneVariables(GetInBranch("PsdModules")->GetConfig());
  psd_modules_90->UseFields({
                                {"number", psd_modules_90_no},
                                {"signal", psd_modules_90_signal}
  });

}
void TaskPsdModules90::UserExec() {

  psd_modules_90->ClearChannels();

  int imodule = 0;
  for (auto &psd_module : psd_modules->Loop()) {
    assert(psd_module[psd_modules_no] == imodule);
    auto module90 = psd_modules_90->NewChannel();
    module90[psd_modules_90_no] = psd_module[psd_modules_no];
    module90[psd_modules_90_signal] = 0.f;
    imodule++;
  }

  for (auto &psd_section : psd_sections->Loop()) {
    auto section_ord_no = psd_section[psd_section_ord_no].GetInt();
    auto section_module_no = psd_section[psd_sections_module_no].GetInt();
    auto section_signal = psd_section[psd_sections_signal].GetVal();
    if (section_ord_no == 0) {
      continue;
    }
    auto module90 = (*psd_modules_90)[std::size_t(section_module_no)];
    module90[psd_modules_90_signal] = module90[psd_modules_90_signal].GetVal() + section_signal;
  }



}
void TaskPsdModules90::UserFinish() {
}
