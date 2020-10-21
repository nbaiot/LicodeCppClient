#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>

#include <glog/logging.h>


#include "thread/thread_pool.h"
#include "core/licode_signaling.h"
#include "core/licode_nuve_api.h"
#include "core/licode_room.h"

#include "core/webrtc_wrapper.h"
#include "core/licode_room_observer.h"
#include "core/webrtc_connection.h"
#include "core/webrtc_peer_connection_observer.h"
#include "core/video_render.h"
using namespace nbaiot;


class TestBasicExampleRoom : public LicodeRoomObserver,
                             public WebrtcPeerConnectionObserver {

public:
  TestBasicExampleRoom(std::shared_ptr<Worker> worker, LicodeToken token) {
    room_ = std::make_shared<LicodeRoom>(std::move(worker), std::move(token), this);
  }

  void Join() {
    room_->Join();
  }

  void Leave() {
    room_->Leave();
  }

  void OnJoinRoom(const std::string& roomId) override {
    LOG(INFO) << ">>>>>>>>>> join room:" << roomId << " success";
  }

  void OnLeaveRoom(const std::string& roomId) override {
    LOG(INFO) << ">>>>>>>>>> leave room:" << roomId;
  }

  void OnAddRemoteStream(const std::shared_ptr<LicodeStreamInfo>& info) override {
    CreatePeerConnection(info->Id());
    room_->SubscribeStream(info->Id());
  }

  void OnRemoveRemoteStream(const std::shared_ptr<LicodeStreamInfo>& info) override {

  }

  void OnSubscribeStreamAllocateConnId(const std::shared_ptr<LicodeStreamInfo>& info) override {
    LOG(INFO) << ">>>>>>>>>> subscribe stream connId:" << info->ConnectionId();
  }

  void OnStreamConnectReady(const std::shared_ptr<LicodeStreamInfo>& info) override {

  }

  void OnPublishStreamAllocateConnId(const std::shared_ptr<LicodeStreamInfo>& info) override {
    CreatePeerConnection(info->Id());
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
    /// for suppoer plan-b
    options.offer_to_receive_audio = 1;
    options.offer_to_receive_video = 1;
    peer_connections_[info->Id()]->CreateOffer(options);
  }

  void OnReceiveOffer(const std::shared_ptr<LicodeStreamInfo>& info, const std::string& sdp) override {
    webrtc::SdpParseError error;
    auto sessionDescription = WebrtcWrapper::CreateSessionDescription(webrtc::SdpType::kOffer, sdp, &error);
    if (!sessionDescription) {
      LOG(ERROR) << ">>>>>>>>>> CreateSessionDescription error:" << error.description;
      return;
    }
    peer_connections_[info->Id()]->SetRemoteDescription(std::move(sessionDescription));
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
    /// for suppoer plan-b
    options.offer_to_receive_audio = 1;
    options.offer_to_receive_video = 1;
    peer_connections_[info->Id()]->CreateAnswer(options);
  }

  void OnReceiveAnswer(const std::shared_ptr<LicodeStreamInfo>& info, const std::string& sdp) override {

  }

  void OnSdpCreateSuccess(WebrtcConnection* peer, webrtc::SdpType type, const std::string& sdp) override {
    webrtc::SdpParseError error;
    auto sessionDescription = WebrtcWrapper::CreateSessionDescription(type, sdp, &error);
    if (!sessionDescription) {
      LOG(ERROR) << ">>>>>>>>>> CreateSessionDescription error:" << error.description;
      return;
    }
    peer->SetLocalDescription(std::move(sessionDescription));
    room_->SendAnswer(peer->Id(), sdp);
  }

  void OnIceCandidate(WebrtcConnection* peer, const webrtc::IceCandidateInterface* candidate) override {
    std::string out;
    candidate->ToString(&out);
    auto mid = candidate->sdp_mid();
    auto index = candidate->sdp_mline_index();
    auto candidateAttribute = "a=" + out;
    room_->SendIceCandidate(peer->Id(), mid, index, candidateAttribute);
  }

  void OnPeerConnect(WebrtcConnection* peer) override {
    LOG(ERROR) << ">>>>>>>>>> OnPeerConnect:" << peer->Id();
  }

  void OnPeerDisconnect(WebrtcConnection* peer) override {
    LOG(ERROR) << ">>>>>>>>>> OnPeerDisconnect:" << peer->Id();
  }

  void OnAddRemoteVideoTrack(WebrtcConnection* peer, webrtc::VideoTrackInterface* video) override {
    video->AddOrUpdateSink(new VideoRender(), rtc::VideoSinkWants());
  }

  void OnRemoveRemoteVideoTrack(WebrtcConnection* peer, webrtc::VideoTrackInterface* video) override {

  }

  void OnAddRemoteAudioTrack(WebrtcConnection* peer, webrtc::AudioTrackInterface* audio) override {

  }

  void OnRemoveRemoteAudioTrack(WebrtcConnection* peer, webrtc::AudioTrackInterface* audio) override {

  }

  void OnDataChannel(WebrtcConnection* peer, webrtc::DataChannelInterface* dataChannel) override {

  }

private:
  void CreatePeerConnection(uint64_t streamId) {
    if (peer_connections_[streamId]) {
      return;
    }
    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.sdp_semantics = webrtc::SdpSemantics::kPlanB;
    config.enable_dtls_srtp = true;
    config.type = webrtc::PeerConnectionInterface::kNoHost; /// kRelay, kNoHost, kAll
    for (const auto& ice: room_->IceServers()) {
      webrtc::PeerConnectionInterface::IceServer server;
      server.uri = ice.url;
      server.username = ice.username;
      server.password = ice.credential;
      config.servers.push_back(server);
    }
    peer_connections_[streamId] = std::make_unique<WebrtcConnection>(streamId, this, config);
  }

private:
  std::shared_ptr<LicodeRoom> room_;
  std::unordered_map<uint64_t, std::unique_ptr<WebrtcConnection>> peer_connections_;
};


int main() {
  LOG(INFO) << "Hello licode !!!" << std::endl;

  WebrtcWrapper::Instance()->Init();
  WebrtcWrapper::SetWebrtcLogLevel(WebrtcWrapper::kWarning);

  auto pool = std::make_shared<ThreadPool>(1);
  pool->Start();

  auto nuve = std::make_shared<LicodeNuveApi>("5f9047504d2d972192a1dfc0",
                                              "3550", "192.168.1.5",
                                              3000);
  auto rooms = nuve->SyncListRoom();

  if (!rooms.has_value())
    return -1;

  auto listRooms = rooms.value();

  std::string exampleRoomId;
  for (const auto& room: listRooms) {
    if (room->Name() == "basicExampleRoom")
      exampleRoomId = room->Id();
  }

  if (exampleRoomId.empty())
    return -1;


  /// role: presenter, viewer, viewerWithData
  auto token = nuve->SyncCreateToken(exampleRoomId, "shajia", "presenter");

  if (!token.has_value())
    return -1;

  auto licodeToken = token.value();

  auto room = std::make_shared<TestBasicExampleRoom>(pool->GetLessUsedWorker(), licodeToken);

  room->Join();

  std::mutex mutex;
  std::unique_lock<std::mutex> lk(mutex);
  std::condition_variable cv;
  cv.wait(lk);
  return 0;
}