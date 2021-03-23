#include <QCoreApplication>

#include "server.h"
#include "src/schedulerservice.h"

int main(int argc, char* argv[]) {
  QCoreApplication a(argc, argv);
  QSharedPointer<Configuration> configuration(new Configuration(a.arguments()));
  jsonrpc::Server<SchedulerService> server(configuration->getPort());
  server.setConstructorArguments(configuration);
  server.startListening();

  return a.exec();
}
