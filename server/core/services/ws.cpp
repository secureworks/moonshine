#include "ws.h"

namespace services {

void wsconnection::onPing(const WebSocket& socket, const oatpp::String& message) {
  socket.sendPong(message);
}

void wsconnection::onPong(const WebSocket& socket, const oatpp::String& message) {}

void wsconnection::onClose(const WebSocket& socket, v_uint16 code, const oatpp::String& message) {}

void wsconnection::readMessage(const WebSocket& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) {
  /*
  if(size == 0) { // message transfer finished
    auto wholeMessage = m_messageBuffer.toString();
    m_messageBuffer.setCurrentPosition(0);

    OATPP_LOGD(TAG, "onMessage message='%s'", wholeMessage->c_str());

    // Send message in reply
    socket.sendOneFrameText( "Hello! : " + wholeMessage);

  } else if(size > 0) { // message frame received
    m_messageBuffer.writeSimple(data, size);
  }
   */
}

void wsconnection::sendMessage(const oatpp::String& message) {
  _socket.sendOneFrameText(message);
}


/*** wslistener ***/

void wslistener::onAfterCreate(const oatpp::websocket::WebSocket& socket, const std::shared_ptr<const ParameterMap>& params) {
  std::scoped_lock<std::mutex> lock(_connections_mutex);
  auto connection = std::make_shared<wsconnection>(socket);
  _connections.insert(connection);
  socket.setListener(connection);
}

void wslistener::onBeforeDestroy(const oatpp::websocket::WebSocket& socket) {
  std::scoped_lock<std::mutex> lock(_connections_mutex);
  auto peer = std::static_pointer_cast<wsconnection>(socket.getListener());
  _connections.erase(peer);
}

void wslistener::broadcast(const oatpp::String& message) {
  std::lock_guard<std::mutex> guard(_connections_mutex);
  for(const auto& connection : _connections) {
    connection->sendMessage(message);
  }
}

/*** websockets ***/

websockets::websockets() : _messages_thread{std::async(std::launch::async, [this] { service_messages(); })} {}

websockets::~websockets() {
  _messages_queue.stop();
}

void websockets::service_messages() {
  while (true) {
    try {
      auto message = _messages_queue.wait_and_pop();
      _wslistener->broadcast(message);
    }
    catch (const queue_stopped& e) {
      break;
    }
  }
}

void websockets::send(std::string message) {
  _messages_queue.push(message);
}

}
