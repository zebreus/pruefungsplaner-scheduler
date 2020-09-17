#include "plancsvhelper.h"

PlanCsvHelper::PlanCsvHelper(QString path) : basePath(path) {
  initializeFilePaths();
}

PlanCsvHelper::PlanCsvHelper() : temporaryDirectory(new QTemporaryDir()) {
  basePath = temporaryDirectory->path();
  initializeFilePaths();
}

QSharedPointer<Plan> PlanCsvHelper::readPlan() {
  // TODO implement
  return nullptr;
}

bool PlanCsvHelper::writePlan(QSharedPointer<Plan> plan) {
  return writeExamsIntervalsFile(plan) && writeExamsFile(plan) &&
         writeGroupsExamsFile(plan) && writeGroupsExamsPrefFile(plan);
}

bool PlanCsvHelper::isWritten() {
  return examsIntervalsFile.exists() && examsFile.exists() &&
         groupsExamsFile.exists() && groupsExamsPrefFile.exists();
}

bool PlanCsvHelper::isScheduled() {
  return planningExamsResultFile.exists() && groupsExamsResultFile.exists();
}

QString PlanCsvHelper::getPath() {
  return basePath;
}

void PlanCsvHelper::initializeFilePaths() {
  examsIntervalsFile.setFileName(basePath + "/pruef-intervalle.csv");
  examsFile.setFileName(basePath + "/pruefungen.csv");
  groupsExamsFile.setFileName(basePath + "/zuege-pruef.csv");
  groupsExamsPrefFile.setFileName(basePath + "/zuege-pruef-pref2.csv");
  planningExamsResultFile.setFileName(basePath +
                                      "/SPA-ERGEBNIS-PP/SPA-planung-pruef.csv");
  groupsExamsResultFile.setFileName(basePath +
                                    "/SPA-ERGEBNIS-PP/SPA-zuege-pruef.csv");
}

bool PlanCsvHelper::writeExamsIntervalsFile(QSharedPointer<Plan> plan) {
  if (!examsIntervalsFile.open(QFile::ReadWrite)) {
    return false;
  }
  QTextStream fileStream(&examsIntervalsFile);

  fileStream << "Block;";
  for (auto constraint : plan->constraints) {
    fileStream << constraint->name() << ";";
  }
  fileStream << "-ENDE-\n";

  fileStream << QString("Maximale Prü/Tag;");
  for (int i = 0; i<plan->constraints.size();i++) {
    //TODO add option for max exams per day for constraints to datamodel
    fileStream << "6;";
  }
  fileStream << "\n";

  // TODO check that plan has the right amount of days
  for (int week = 0; week < 3; week++) {
    for (int day = 0; day < 6; day++) {
      for (int timeslot = 0; timeslot < 6; timeslot++) {
        fileStream << blockNames[week * 6 * 6 + day * 6 + timeslot] << ";";
        for (Group* constraint : plan->constraints) {
          if (plan->weeks[week]
                  ->getDays()[day]
                  ->getTimeslots()[timeslot]
                  ->containsActiveGroup(constraint)) {
            fileStream << "FREI;";
          } else {
            fileStream << "BLOCKIERT;";
          }
        }
        fileStream << "\n";
      }
    }
  }

  /*
  //TODO check if correct blocknames are neccesary
  for (Week* week : plan->weeks) {
    for (Day* day : week->getDays()) {
      for (Timeslot* timeslot : day->getTimeslots()) {
        fileStream << day->name().chopped(2) << week->name().chopped(1) << "_"
                   << timeslot->name() << ";";
        for (Group* constraint : plan->constraints) {
          if (timeslot->containsActiveGroup(constraint)) {
            fileStream << "FREI;";
          } else {
            fileStream << "BLOCKIERT;";
          }
        }
      }
    }
  }
  */

  fileStream << "-ENDE-;";
  for (int i = 0; i<plan->constraints.size();i++) {
    fileStream << ";";
  }

  return true;

  // TODO Check if the second part of the csv file is needed
}

bool PlanCsvHelper::writeExamsFile(QSharedPointer<Plan> plan) {
  if (!examsFile.open(QFile::ReadWrite)) {
    return false;
  }
  QTextStream fileStream(&examsFile);

  for (Module* module : plan->modules) {
    // TODO Check somewhere else
    if (module->getOrigin() == "EIT") {
      // SPA-algorithmus fails if there are EIT exams, because "Prüfungen von
      // EIT müssen fest abgesprochen sein!!"
      break;
    }

    // Only one constraint is possible, because the legacy algorithm does not
    // support more
    if (module->constraints.size() >= 1) {
      fileStream << module->constraints[0]->name();
    }
    fileStream << ";";

    // TODO Check somewhere, that groupnames do not contain commas
    QString divider = "";
    for (Group* group : module->groups) {
      fileStream << divider << group->name();
      divider = ",";
    }
    fileStream << ";";

    fileStream << module->getName() << ";";
    fileStream << module->getNumber() << ";";
    fileStream << module->getOrigin() << ";";
    // TODO find out what K or P means and add to datamodel
    fileStream << "K"
               << ";";
    // TODO add duration to datamodel and add it here
    fileStream << "";

    fileStream << "\n";
  }
  fileStream << "-ENDE-;;;;;;";

  return true;
}

bool PlanCsvHelper::writeGroupsExamsFile(QSharedPointer<Plan> plan) {
  if (!groupsExamsFile.open(QFile::ReadWrite)) {
    return false;
  }
  QTextStream fileStream(&groupsExamsFile);

  fileStream << "Block;";
  for (Group* group : plan->groups) {
    fileStream << group->name() << ";";
  }
  fileStream << "-ENDE-\n";

  fileStream << QString("Maximale Prü/Tag;");
  for (int i = 0; i<plan->groups.size();i++) {
    fileStream << "2;";
  }
  fileStream << "\n";

  // TODO check that plan has the right amount of days
  for (int week = 0; week < 3; week++) {
    for (int day = 0; day < 6; day++) {
      for (int timeslot = 0; timeslot < 6; timeslot++) {
        fileStream << blockNames[week * 6 * 6 + day * 6 + timeslot] << ";";
        for (Group* group : plan->groups) {
          if (plan->weeks[week]
                  ->getDays()[day]
                  ->getTimeslots()[timeslot]
                  ->containsActiveGroup(group)) {
            fileStream << "FREI;";
          } else {
            fileStream << "BLOCKIERT;";
          }
        }
        fileStream << "\n";
      }
    }
  }

  /*
  //TODO check if correct blocknames are neccesary
  for (Week* week : plan->weeks) {
    for (Day* day : week->getDays()) {
      for (Timeslot* timeslot : day->getTimeslots()) {
        fileStream << day->name().chopped(2) << week->name().chopped(1) << "_"
                   << timeslot->name() << ";";
        for (Group* group : plan->groups) {
          if (timeslot->containsActiveGroup(group)) {
            fileStream << "FREI;";
          } else {
            fileStream << "BLOCKIERT;";
          }
        }
      }
    }
  }
  */

  fileStream << "-ENDE-;";
  for (int i = 0; i<plan->groups.size();i++) {
    fileStream << ";";
  }

  return true;
}

bool PlanCsvHelper::writeGroupsExamsPrefFile(QSharedPointer<Plan>) {
  if (!groupsExamsPrefFile.open(QFile::ReadWrite)) {
    return false;
  }
  QTextStream fileStream(&groupsExamsPrefFile);

  fileStream << "I Bach 1A;;;;;\n";
  fileStream << "-ENDE-;;;;;";

  return true;
}
