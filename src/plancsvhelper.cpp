#include "plancsvhelper.h"

PlanCsvHelper::PlanCsvHelper(QString path) : basePath(path) {
  initializeFilePaths();
}

PlanCsvHelper::PlanCsvHelper() : temporaryDirectory(new QTemporaryDir()) {
  basePath = temporaryDirectory->path();
  initializeFilePaths();
}

QSharedPointer<Plan> PlanCsvHelper::readPlan() {
  QSharedPointer<Plan> newPlan(new Plan());
  // TODO add support for custom names
  newPlan->setName("new plan");

  // Add base
  newPlan->weeks.append(new Week(newPlan.get()));
  newPlan->weeks.append(new Week(newPlan.get()));
  newPlan->weeks.append(new Week(newPlan.get()));
  newPlan->weeks[0]->setName("Woche 1");
  newPlan->weeks[1]->setName("Woche 2");
  newPlan->weeks[2]->setName("Woche 3");
  for (Week* week : newPlan->weeks) {
    QList<Day*> days;
    for (int x = 0; x < 6; x++) {
      days.append(new Day(week));
    }
    days[0]->setName("Montag");
    days[1]->setName("Dienstag");
    days[2]->setName("Mittwoch");
    days[3]->setName("Donnerstag");
    days[4]->setName("Freitag");
    days[5]->setName("Samstag");
    for (Day* day : days) {
      QList<Timeslot*> timeslots;
      for (int x = 0; x < 6; x++) {
        timeslots.append(new Timeslot(day));
      }
      timeslots[0]->setName("Block 1");
      timeslots[1]->setName("Block 2");
      timeslots[2]->setName("Block 3");
      timeslots[3]->setName("Block 4");
      timeslots[4]->setName("Block 5");
      timeslots[5]->setName("Block 6");
      day->setTimeslots(timeslots);
    }
    week->setDays(days);
  }

  if (!readExamsIntervalsFile(newPlan)) {
    return nullptr;
  }

  if (!readGroupsExamsFile(newPlan)) {
    return nullptr;
  }

  if (!readExamsFile(newPlan)) {
    return nullptr;
  }

  return newPlan;
}

bool PlanCsvHelper::writePlan(QSharedPointer<Plan> plan) {
  if (plan == nullptr) {
    return false;
  }
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

bool PlanCsvHelper::readSchedule(QSharedPointer<Plan> plan) {
  if (!planningExamsResultFile.open(QFile::ReadOnly)) {
    return false;
  }
  QTextStream fileStream(&planningExamsResultFile);

  // Check, that firstline is valid
  if (fileStream.readLine() !=
      QString("BelegNr;Zug;Modul;Import;Prüfungsform;Zuordnung;Tag;Block;")) {
    planningExamsResultFile.close();
    return false;
  }
  /*
  QString firstLine = fileStream.readLine();
  if (firstLine == "") {
    return false;
  }
  QList<QString> firstWords = firstLine.split(";");
  if (firstWords.size() != 9) {
    return false;
  }
  if (firstWords[0] != "BelegNr" || firstWords[1] != "Zug" ||
      firstWords[2] != "Modul" || firstWords[3] != "Import" ||
      firstWords[4] != QString("Prüfungsform") ||
      firstWords[5] != "Zuordnung" || firstWords[6] != "Tag" ||
      firstWords[6] != "Tag" || firstWords[7] != "Block" ||
      firstWords[8] != "") {
    return false;
  }
  */

  // The modules will only be added to the slots, if the file is correct. Until
  // then they will be stored here.
  QList<QPair<Module*, Timeslot*>> modulesToAdd;

  while (!fileStream.atEnd()) {
    QString line = fileStream.readLine();
    if (line == "") {
      planningExamsResultFile.close();
      return true;
    }
    QList<QString> words = line.split(";");
    if (words.size() != 9) {
      planningExamsResultFile.close();
      return false;
    }

    // Generate module number (BelegNr[,Zug])
    QString moduleNumber = words[0];
    if (words[1] != "") {
      moduleNumber.append(",").append(words[1]);
    }

    // Find matching module
    Module* matchingModule = nullptr;
    for (Module* module : plan->modules) {
      if (module->getNumber() == moduleNumber) {
        if (module->getName() == words[2]) {
          matchingModule = module;
          break;
        }
      }
    }
    if (matchingModule == nullptr) {
      planningExamsResultFile.close();
      return false;
    }

    // Find matching timeslot
    Timeslot* matchingTimeslot;
    bool dayOk = false;
    int day = words[6].toInt(&dayOk) - 1;
    if (!dayOk) {
      planningExamsResultFile.close();
      return false;
    }
    bool slotOk = false;
    int slot = words[7].toInt(&slotOk) - 1;
    if (!slotOk) {
      planningExamsResultFile.close();
      return false;
    }

    if (day < 0 || day / 7 >= plan->weeks.size()) {
      planningExamsResultFile.close();
      return false;
    }
    Week* weekPointer = plan->weeks[day / 7];

    if (day % 7 >= weekPointer->getDays().size()) {
      planningExamsResultFile.close();
      return false;
    }
    Day* dayPointer = weekPointer->getDays()[day % 7];

    if (slot < 0 || slot >= dayPointer->getTimeslots().size()) {
      planningExamsResultFile.close();
      return false;
    }
    matchingTimeslot = dayPointer->getTimeslots()[slot];

    modulesToAdd.append(
        QPair<Module*, Timeslot*>(matchingModule, matchingTimeslot));
  }

  // Finally add modules to timeslots
  for (auto moduleTimeslotPair : modulesToAdd) {
    for (Week* week : plan->weeks) {
      for (Day* day : week->getDays()) {
        for (Timeslot* timeslot : day->getTimeslots()) {
          timeslot->removeModule(moduleTimeslotPair.first);
        }
      }
    }
    moduleTimeslotPair.second->addModule(moduleTimeslotPair.first);
  }

  planningExamsResultFile.close();
  return true;
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
  for (int i = 0; i < plan->constraints.size(); i++) {
    // TODO add option for max exams per day for constraints to datamodel
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
  for (int i = 0; i < plan->constraints.size(); i++) {
    fileStream << ";";
  }

  examsIntervalsFile.close();
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
      continue;
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

  examsFile.close();
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
  for (int i = 0; i < plan->groups.size(); i++) {
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
  for (int i = 0; i < plan->groups.size(); i++) {
    fileStream << ";";
  }

  groupsExamsFile.close();
  return true;
}

bool PlanCsvHelper::writeGroupsExamsPrefFile(QSharedPointer<Plan>) {
  if (!groupsExamsPrefFile.open(QFile::ReadWrite)) {
    return false;
  }
  QTextStream fileStream(&groupsExamsPrefFile);

  fileStream << "I Bach 1A;;;;;\n";
  fileStream << "-ENDE-;;;;;";

  groupsExamsPrefFile.close();
  return true;
}

bool PlanCsvHelper::readExamsIntervalsFile(QSharedPointer<Plan> plan) {
  if (!examsIntervalsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }

  QTextStream fileStream(&examsIntervalsFile);

  QList<Group*> groups;

  // Read and check first line
  int wordsPerLine;
  QList<QString> firstLine = fileStream.readLine().split(";");
  if (firstLine.size() < 2 || firstLine[0] != "Block" ||
      firstLine.last() != "-ENDE-") {
    examsIntervalsFile.close();
    return false;
  }
  for (int i = 1; i < firstLine.size() - 1; i++) {
    Group* group = new Group(plan.get());
    group->setName(firstLine[i]);
    plan->constraints.append(group);
    groups.append(group);
  }
  wordsPerLine = firstLine.size();

  // Read and check second line
  QList<QString> secondLine = fileStream.readLine().split(";");
  if (secondLine.size() != wordsPerLine ||
      secondLine[0] != QString("Maximale Prü/Tag") || secondLine.last() != "") {
    examsIntervalsFile.close();
    return false;
  }

  // Read a line for each timeslot
  for (Week* week : plan->weeks) {
    for (Day* day : week->getDays()) {
      for (Timeslot* timeslot : day->getTimeslots()) {
        QList<QString> words = fileStream.readLine().split(";");
        if (words.size() != wordsPerLine) {
          examsIntervalsFile.close();
          return false;
        }
        for (int i = 0; i < groups.size(); i++) {
          if (words[i + 1] == "FREI") {
            timeslot->addActiveGroup(groups[i]);
          } else if (words[i + 1] != "BLOCKIERT") {
            examsIntervalsFile.close();
            return false;
          }
        }
      }
    }
  }

  examsIntervalsFile.close();
  return true;
}

bool PlanCsvHelper::readExamsFile(QSharedPointer<Plan> plan) {
  if (!examsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }

  QTextStream fileStream(&examsFile);

  QList<QString> words = fileStream.readLine().split(";");
  while (!words.isEmpty() && words.first() != "-ENDE-") {
    if (words.first().startsWith("//")) {
      words = fileStream.readLine().split(";");
      continue;
    }
    if (words.size() != 7) {
      examsFile.close();
      return false;
    }
    Module* module = new Module(plan.get());
    module->setName(words[2]);
    module->setOrigin(words[4]);
    module->setNumber(words[3]);
    for (QString groupName : words[1].split(",")) {
      bool foundGroup = false;
      for (Group* group : plan->groups) {
        if (group->name() == groupName) {
          module->groups.append(group);
          foundGroup = true;
          break;
        }
      }
      if (!foundGroup) {
        examsFile.close();
        return false;
      }
    }

    bool foundConstraint = false;
    if (words[0] != "") {
      for (Group* constraint : plan->constraints) {
        if (constraint->name() == words[0]) {
          module->constraints.append(constraint);
          foundConstraint = true;
          break;
        }
      }
      if (!foundConstraint) {
        examsFile.close();
        return false;
      }
    }
    plan->modules.append(module);

    words = fileStream.readLine().split(";");
  }

  return true;
}

bool PlanCsvHelper::readGroupsExamsFile(QSharedPointer<Plan> plan) {
  if (!groupsExamsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }

  QTextStream fileStream(&groupsExamsFile);

  QList<Group*> groups;

  // Read and check first line
  int wordsPerLine;
  QList<QString> firstLine = fileStream.readLine().split(";");
  if (firstLine.size() < 2 || firstLine[0] != "Block" ||
      firstLine.last() != "-ENDE-") {
    groupsExamsFile.close();
    return false;
  }
  for (int i = 1; i < firstLine.size() - 1; i++) {
    Group* group = new Group(plan.get());
    group->setName(firstLine[i]);
    plan->groups.append(group);
    groups.append(group);
  }
  wordsPerLine = firstLine.size();

  // Read and check second line
  QList<QString> secondLine = fileStream.readLine().split(";");
  if (secondLine.size() != wordsPerLine ||
      secondLine[0] != QString("Maximale Prü/Tag") || secondLine.last() != "") {
    groupsExamsFile.close();
    return false;
  }

  // Read a line for each timeslot
  for (Week* week : plan->weeks) {
    for (Day* day : week->getDays()) {
      for (Timeslot* timeslot : day->getTimeslots()) {
        QList<QString> words = fileStream.readLine().split(";");
        if (words.size() != wordsPerLine) {
          groupsExamsFile.close();
          return false;
        }
        for (int i = 0; i < groups.size(); i++) {
          if (words[i + 1] == "FREI") {
            timeslot->addActiveGroup(groups[i]);
          } else if (words[i + 1] != "BLOCKIERT") {
            groupsExamsFile.close();
            return false;
          }
        }
      }
    }
  }

  groupsExamsFile.close();
  return true;
}

bool PlanCsvHelper::readGroupsExamsPrefFile(QSharedPointer<Plan>) {
  // The contents of the groupsExamsPrefFile are not relevant for plan
  return true;
}
