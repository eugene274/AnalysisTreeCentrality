//
// Created by eugene on 18/03/2021.
//

#ifndef ATCENTRALITYTASK_TASK_PSD_MODULES_90_TASKPSDMODULES90_HPP_
#define ATCENTRALITYTASK_TASK_PSD_MODULES_90_TASKPSDMODULES90_HPP_

#include <at_task/UserTask.h>
#include <at_task/Task.h>

/**
 * @brief Recalculates deposit to PSD Modules excluding
 * first section in each module (to filter out pions)
 */
class TaskPsdModules90 : public UserFillTask {

 protected:
  void UserInit(std::map<std::string, void *> &map) override;
  void UserExec() override;
  void UserFinish() override;

  /* input */
  ATI2::Branch *psd_sections;
  ATI2::Variable psd_sections_module_no;
  ATI2::Variable psd_sections_signal;
  ATI2::Variable psd_section_ord_no;
  ATI2::Branch *psd_modules;
  ATI2::Variable psd_modules_no;

  /* output */
  ATI2::Branch *psd_modules_90;
  ATI2::Variable psd_modules_90_no;
  ATI2::Variable psd_modules_90_signal;


 TASK_DEF(TaskPsdModules90, 0)
};

#endif //ATCENTRALITYTASK_TASK_PSD_MODULES_90_TASKPSDMODULES90_HPP_
