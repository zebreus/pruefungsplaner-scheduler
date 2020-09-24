#include <QCoreApplication>
#include "server.h"
#include "src/schedulerservice.h"

int main(int argc, char* argv[]) {
  QCoreApplication a(argc, argv);
  jsonrpc::Server<SchedulerService> server(9094);
  server.setConstructorArguments();
  server.startListening();

  return a.exec();
}
