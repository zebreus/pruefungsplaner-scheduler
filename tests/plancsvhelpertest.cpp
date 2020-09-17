#ifndef PLANCSVHELPER_TEST_CPP
#define PLANCSVHELPER_TEST_CPP

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTemporaryDir>
#include <QTextStream>
#include "plan.h"
#include "plancsvhelper.h"

using namespace testing;

TEST(planCsvHelperTests, constructorSetsPath) {
  QString path = QDir::currentPath();
  PlanCsvHelper helper(path);
  EXPECT_EQ(helper.getPath(), path);
}

TEST(planCsvHelperTests, constructorCreatesTemporaryDirectory) {
  PlanCsvHelper helper;
  EXPECT_NE(helper.getPath(), "");
  EXPECT_TRUE(QDir().exists(helper.getPath()))
      << "PlanCsvHelper did not create a temporary directory at "
      << helper.getPath().constData() << ".";
}

TEST(planCsvHelperTests, temporaryDirectoryGetsRemovedAfterDestruction) {
  PlanCsvHelper* helper = new PlanCsvHelper();
  QString path = helper->getPath();
  delete helper;
  EXPECT_FALSE(QDir().exists(path))
      << "PlanCsvHelper did not remove temporary directory at "
      << helper->getPath().constData() << " after destruction.";
}

TEST(planCsvHelperTests, normalDirectoryDoesNotGetRemovedAfterDestruction) {
  QTemporaryDir tempdir;
  PlanCsvHelper* helper = new PlanCsvHelper(tempdir.path());
  QString path = helper->getPath();
  delete helper;
  EXPECT_TRUE(QDir().exists(path))
      << "PlanCsvHelper did remove directory at "
      << helper->getPath().constData() << " after destruction.";
}

TEST(planCsvHelperTests, isScheduledDetectsNoResultDirectory) {
  QTemporaryDir noResultDirectory;
  PlanCsvHelper helper(noResultDirectory.path());
  EXPECT_FALSE(helper.isScheduled()) << "Detected as scheduled, but it is not";
}

TEST(planCsvHelperTests, isScheduledDetectsEmptyResultDirectory) {
  QTemporaryDir emptyResultDirectory;
  QDir().mkdir(emptyResultDirectory.path().append("/SPA-ERGEBNIS-PP/"));
  PlanCsvHelper helper(emptyResultDirectory.path());
  EXPECT_FALSE(helper.isScheduled()) << "Detected as scheduled, but it is not";
}

TEST(planCsvHelperTests, isScheduledDetectsPartialEmptyResultDirectory) {
  QTemporaryDir partialResultDirectory1;
  QDir().mkdir(partialResultDirectory1.path().append("/SPA-ERGEBNIS-PP/"));
  QFile(partialResultDirectory1.path().append(
            "/SPA-ERGEBNIS-PP/SPA-planung-pruef.csv"))
      .open(QFile::ReadWrite);
  PlanCsvHelper helper1(partialResultDirectory1.path());
  EXPECT_FALSE(helper1.isScheduled()) << "Detected as scheduled, but it is not";

  QTemporaryDir partialResultDirectory2;
  QDir().mkdir(partialResultDirectory2.path().append("/SPA-ERGEBNIS-PP/"));
  QFile(partialResultDirectory2.path().append(
            "/SPA-ERGEBNIS-PP/SPA-zuege-pruef.csv"))
      .open(QFile::ReadWrite);
  PlanCsvHelper helper2(partialResultDirectory1.path());
  EXPECT_FALSE(helper2.isScheduled()) << "Detected as scheduled, but it is not";
}

TEST(planCsvHelperTests, isScheduledDetectsFullResultDirectory) {
  QTemporaryDir fullResultDirectory;
  QDir().mkdir(fullResultDirectory.path().append("/SPA-ERGEBNIS-PP/"));
  QFile(fullResultDirectory.path().append(
            "/SPA-ERGEBNIS-PP/SPA-planung-pruef.csv"))
      .open(QFile::ReadWrite);
  QFile(
      fullResultDirectory.path().append("/SPA-ERGEBNIS-PP/SPA-zuege-pruef.csv"))
      .open(QFile::ReadWrite);
  PlanCsvHelper helper(fullResultDirectory.path());
  EXPECT_TRUE(helper.isScheduled()) << "Did not detect full result directory";
}

TEST(planCsvHelperTests, isWrittenDetectsEmptyDirectory) {
  QTemporaryDir emptyResultDirectory;
  PlanCsvHelper helper(emptyResultDirectory.path());
  EXPECT_FALSE(helper.isWritten()) << "Detected as written, but it is not";
}

TEST(planCsvHelperTests, isWrittenDetectsPartialEmptyDirectory) {
  QTemporaryDir directory1;
  QTemporaryDir directory2;
  QFile(directory1.path().append("/pruef-intervalle.csv"))
      .open(QFile::ReadWrite);
  QFile(directory1.path().append("/pruefungen.csv")).open(QFile::ReadWrite);
  QFile(directory2.path().append("/zuege-pruef.csv")).open(QFile::ReadWrite);
  QFile(directory2.path().append("/zuege-pruef-pref2.csv"))
      .open(QFile::ReadWrite);
  PlanCsvHelper helper1(directory1.path());
  PlanCsvHelper helper2(directory2.path());
  EXPECT_FALSE(helper1.isWritten()) << "Detected as written, but it is not";
  EXPECT_FALSE(helper2.isWritten()) << "Detected as written, but it is not";
}

TEST(planCsvHelperTests, isWrittenDetectsFiles) {
  QTemporaryDir directory;
  QFile(directory.path().append("/pruef-intervalle.csv"))
      .open(QFile::ReadWrite);
  QFile(directory.path().append("/pruefungen.csv")).open(QFile::ReadWrite);
  QFile(directory.path().append("/zuege-pruef.csv")).open(QFile::ReadWrite);
  QFile(directory.path().append("/zuege-pruef-pref2.csv"))
      .open(QFile::ReadWrite);
  PlanCsvHelper helper(directory.path());
  EXPECT_TRUE(helper.isWritten());
}

TEST(planCsvHelperTests, writePlanCreatesFiles) {
  QFile file("./tests/data/plan.json");
  ASSERT_TRUE(file.exists())
      << "Example plan json file (" << file.fileName().constData()
      << ") does not exist.";
  ASSERT_TRUE(file.open(QFile::ReadOnly | QFile::Text))
      << "Cannot open example plan json file (" << file.fileName().constData()
      << ").";
  QString jsonString = QTextStream(&file).readAll();
  QJsonDocument document = QJsonDocument::fromJson(jsonString.toUtf8());
  ASSERT_TRUE(document.isObject()) << "Json file does not contain an object";
  QSharedPointer<Plan> plan(new Plan());
  plan->fromJsonObject(document.object());

  QTemporaryDir directory;
  PlanCsvHelper helper(directory.path());
  EXPECT_TRUE(helper.writePlan(plan));
  // helper.isWritten is already tested.
  EXPECT_TRUE(helper.isWritten()) << "Plan is not written after check";
}

TEST(planCsvHelperTests, writePlanDetectsMissingDirectory) {
  QFile file("./tests/data/plan.json");
  ASSERT_TRUE(file.exists())
      << "Example plan json file (" << file.fileName().constData()
      << ") does not exist.";
  ASSERT_TRUE(file.open(QFile::ReadOnly | QFile::Text))
      << "Cannot open example plan json file (" << file.fileName().constData()
      << ").";
  QString jsonString = QTextStream(&file).readAll();
  QJsonDocument document = QJsonDocument::fromJson(jsonString.toUtf8());
  ASSERT_TRUE(document.isObject()) << "Json file does not contain an object";
  QSharedPointer<Plan> plan(new Plan());
  plan->fromJsonObject(document.object());

  QTemporaryDir directory;
  PlanCsvHelper helper(directory.path().append("/doesnotexist/"));
  EXPECT_FALSE(helper.writePlan(plan))
      << "Write plan succeeded, but the target directory does not exist";
  EXPECT_FALSE(helper.isWritten()) << "Write plan created directory";
}

TEST(planCsvHelperTests, getPathWorks) {
  QString path = QDir::currentPath();
  PlanCsvHelper helper(path);
  EXPECT_NE(helper.getPath(), "");
  EXPECT_EQ(helper.getPath(), path);
}

#endif  // TEST_CPP
