#ifndef MOONSHINE_SERVER_CORE_SERVICES_WS_H_
#define MOONSHINE_SERVER_CORE_SERVICES_WS_H_

#include <unordered_set>
#include <mutex>
#include <future>
#include <string>

#include <signals.hpp>

#include <oatpp-websocket/ConnectionHandler.hpp>
#include <oatpp-websocket/WebSocket.hpp>
#include <oatpp/core/macro/component.hpp>

#include <util/concurrency.hpp>

namespace services {

/**
 * Listen on incoming WebSocket events.
 */
class wsconnection : public oatpp::websocket::WebSocket::Listener {
 private:
  static constexpr const char* TAG = "Server_WSListener";
  //oatpp::data::stream::BufferOutputStream m_messageBuffer;
  const oatpp::websocket::WebSocket& _socket;

 public:
  explicit wsconnection(const oatpp::websocket::WebSocket& socket) : _socket(socket) {}

  /**
   * Called on "ping" frame.
   */
  void onPing(const WebSocket& socket, const oatpp::String& message) override;

  /**
   * Called on "pong" frame
   */
  void onPong(const WebSocket& socket, const oatpp::String& message) override;

  /**
   * Called on "close" frame
   */
  void onClose(const WebSocket& socket, v_uint16 code, const oatpp::String& message) override;

  /**
   * Called on each message frame. After the last message will be called once-again with size == 0 to designate end of the message.
   */
  void readMessage(const WebSocket& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;

  /**
   * Send message to peer.
   */
  void sendMessage(const oatpp::String& message);

};


/**
 * Listen on new WebSocket connections.
 */
struct wslistener : public oatpp::websocket::ConnectionHandler::SocketInstanceListener {

 private:
  static constexpr const char* TAG = "Server_WSInstanceListener";
  std::unordered_set<std::shared_ptr<wsconnection>> _connections;
  std::mutex _connections_mutex;

 public:

  /**
   *  Called when socket is created
   */
  void onAfterCreate(const oatpp::websocket::WebSocket& socket, const std::shared_ptr<const ParameterMap>& params) override;

  /**
   *  Called before socket instance is destroyed.
   */
  void onBeforeDestroy(const oatpp::websocket::WebSocket& socket) override;

  /**
   * Send message to all peers.
   */
  void broadcast(const oatpp::String& message);

};

struct websockets {

  static websockets* instance() {
    static websockets instance;
    return &instance;
  }

  websockets(const websockets&) = delete; // Disable copy
  void operator=(const websockets&) = delete; // Disable assignment

  void send(std::string message);

 private:
  websockets();
  ~websockets();

  void service_messages();

  using message_queue = util::concurrency::queue<std::string>;
  using queue_stopped = util::concurrency::queue_stopped;

  OATPP_COMPONENT(std::shared_ptr<services::wslistener>, _wslistener); // Inject wslistener component
  message_queue _messages_queue;
  std::future<void> _messages_thread;
};

}

namespace websocket {

inline void broadcast(std::string message) {
  services::websockets::instance()->send(message);
}

}

#endif //MOONSHINE_SERVER_CORE_SERVICES_WS_H_
