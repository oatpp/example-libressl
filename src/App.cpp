
#include "./controller/MyController.hpp"
#include "./AppComponent.hpp"

#include "oatpp-libressl/Callbacks.hpp"

#include "oatpp/network/Server.hpp"

#include <iostream>
#include <csignal>

void signal_handler(int signal) {
  if (signal == SIGHUP) {
    auto switcher = std::dynamic_pointer_cast<AppComponent::ProxyConnectionProvider>(OATPP_GET_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, "sslProvider"));
    if (switcher) {
      switcher->resetProvider(AppComponent::createNewProvider());
    }
  }
}

/**
 *  run() method.
 *  1) set Environment components.
 *  2) add ApiController's endpoints to router
 *  3) run server
 */
void run() {
  
  /* set lockingCallback for libressl */
  oatpp::libressl::Callbacks::setDefaultCallbacks();
  
  /* ignore SIGPIPE */

  #if !(defined(WIN32) || defined(_WIN32))
    std::signal(SIGPIPE, SIG_IGN);
  #endif
  
  AppComponent components; // Create scope Environment components
  std::signal(SIGHUP, signal_handler); // Install signal-handler for SIGHUP SSL-Certificate reloading

  /* create ApiControllers and add endpoints to router */
  auto router = components.httpRouter.getObject();

  router->addController(MyController::createShared());
  
  /* create server */
  oatpp::network::Server server(components.serverConnectionProvider.getObject(),
                                components.serverConnectionHandler.getObject());
  
  OATPP_LOGD("Server", "Running on port %s...", components.serverConnectionProvider.getObject()->getProperty("port").toString()->c_str());
  
  server.run();
  
}

/**
 *  main
 */
int main(int argc, const char * argv[]) {

  oatpp::base::Environment::init();

  run();
  
  /* Print how much objects were created during app running, and what have left-probably leaked */
  /* Disable object counting for release builds using '-D OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
  std::cout << "\nEnvironment:\n";
  std::cout << "objectsCount = " << oatpp::base::Environment::getObjectsCount() << "\n";
  std::cout << "objectsCreated = " << oatpp::base::Environment::getObjectsCreated() << "\n\n";
  
  oatpp::base::Environment::destroy();
  
  return 0;
}
