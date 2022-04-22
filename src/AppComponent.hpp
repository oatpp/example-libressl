
#ifndef AppComponent_hpp
#define AppComponent_hpp

#include "client/MyApiClient.hpp"

#include "oatpp-libressl/server/ConnectionProvider.hpp"
#include "oatpp-libressl/client/ConnectionProvider.hpp"
#include "oatpp-libressl/Config.hpp"

#include "oatpp/web/client/HttpRequestExecutor.hpp"

#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"

#include "oatpp/network/tcp/client/ConnectionProvider.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"

#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include "oatpp/core/macro/component.hpp"

/**
 *  Class which creates and holds Application components and registers components in oatpp::base::Environment
 *  Order of components initialization is from top to bottom
 */
class AppComponent {
public:

  /**
   * Helper-Function to create an SSL-Provider on the fly
   */
  static std::shared_ptr<oatpp::network::ServerConnectionProvider> createNewProvider() {
    /**
     * if you see such error:
     * oatpp::libressl::server::ConnectionProvider:Error on call to 'tls_configure'. ssl context failure
     * It might be because you have several ssl libraries installed on your machine.
     * Try to make sure you are using libtls, libssl, and libcrypto from the same package
     */
    OATPP_LOGD("AppComponent::createNewProvider()", "(Re-)Loading Certificates");
    OATPP_LOGD("AppComponent::createNewProvider()", "pem='%s'", CERT_PEM_PATH);
    OATPP_LOGD("AppComponent::createNewProvider()", "crt='%s'", CERT_CRT_PATH);
    auto config = oatpp::libressl::Config::createDefaultServerConfigShared(CERT_CRT_PATH, CERT_PEM_PATH /* private key */);
    return oatpp::libressl::server::ConnectionProvider::createShared(config, OATPP_GET_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, "streamProvider"));
  }

  /**
   * Proxy-Provider to quickly switch connection-providers using the component system.
   */
  class ProxyConnectionProvider : public oatpp::network::ServerConnectionProvider {
   private:
    std::shared_ptr<oatpp::network::ServerConnectionProvider> m_currentProvider;
   public:

    std::shared_ptr<oatpp::data::stream::IOStream> get() override {
      return m_currentProvider->get();
    }

    /* call this method when certificates changed and a new SSL connection provider created */
    void resetProvider(const std::shared_ptr<oatpp::network::ServerConnectionProvider>& newProvider) {
      m_currentProvider = newProvider;
    }

    oatpp::async::CoroutineStarterForResult<const std::shared_ptr<oatpp::data::stream::IOStream> &> getAsync() override {
      return m_currentProvider->getAsync();
    }

    void invalidate(const std::shared_ptr<oatpp::data::stream::IOStream> &resource) override {
      m_currentProvider->invalidate(resource);
    }

    void stop() override {
      m_currentProvider->stop();
    }

  };

  /**
   * Create the streamProvider, this is the ConnectionProvider who does the actual socket handling which is passed to
   * the sslProvider which does the SSL-Handshaking
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, streamProvider)("streamProvider", []{
    return oatpp::network::tcp::server::ConnectionProvider::createShared({"0.0.0.0", 8443, oatpp::network::Address::IP_4});
  }());

  /**
   *  Create ConnectionProvider "sslProvider" which does the SSL-Handshaking and can be replaced on the fly
   *  with new ssl certificates
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)("sslProvider", [] {
    auto proxyProvider = std::make_shared<ProxyConnectionProvider>();
    proxyProvider->resetProvider(createNewProvider());

    return proxyProvider;
  }());
  
  /**
   *  Create Router component
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([] {
    return oatpp::web::server::HttpRouter::createShared();
  }());
  
  /**
   *  Create ConnectionHandler component which uses Router component to route requests
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)([] {
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router); // get Router component
    /* Async ConnectionHandler for Async IO and Coroutine based endpoints */
    return oatpp::web::server::AsyncHttpConnectionHandler::createShared(router);
  }());
  
  /**
   *  Create ObjectMapper component to serialize/deserialize DTOs in Contoller's API
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([] {
    auto serializerConfig = oatpp::parser::json::mapping::Serializer::Config::createShared();
    auto deserializerConfig = oatpp::parser::json::mapping::Deserializer::Config::createShared();
    deserializerConfig->allowUnknownFields = false;
    auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared(serializerConfig, deserializerConfig);
    return objectMapper;
  }());
  
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ClientConnectionProvider>, sslClientConnectionProvider) ("clientConnectionProvider", [] {
    auto config = oatpp::libressl::Config::createShared();
    tls_config_insecure_noverifycert(config->getTLSConfig());
    tls_config_insecure_noverifyname(config->getTLSConfig());
    return oatpp::libressl::client::ConnectionProvider::createShared(config, {"httpbin.org", 443});
  }());
  
  OATPP_CREATE_COMPONENT(std::shared_ptr<MyApiClient>, myApiClient)([] {
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ClientConnectionProvider>, connectionProvider, "clientConnectionProvider");
    OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper);
    auto requestExecutor = oatpp::web::client::HttpRequestExecutor::createShared(connectionProvider);
    return MyApiClient::createShared(requestExecutor, objectMapper);
  }());

};

#endif /* AppComponent_hpp */
