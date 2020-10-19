//
// Created by nbaiot@126.com on 2020/10/15.
//

#include "webrtc_connection.h"
#include "webrtc_wrapper.h"

#include <glog/logging.h>

namespace nbaiot {

/// TODO: fixme
class DummySetSessionDescriptionObserver
    : public webrtc::SetSessionDescriptionObserver {
public:
  static DummySetSessionDescriptionObserver* Create(bool remote = false) {
    return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>(remote);
  }

  void OnSuccess() override {
    LOG(INFO) << ">>>>>>>>>>>>>>>>>> SetSessionDescription "
              << (remote_ ? "remote" : "local") << " sdp success";
  }

  void OnFailure(webrtc::RTCError error) override {
    LOG(ERROR) << ">>>>>>>>>>>>>>>>>> SetSessionDescription "
               << (remote_ ? "remote" : "local") << " sdp failed."
               << webrtc::ToString(error.type()) << ": "
               << error.message();
  }

protected:
  explicit DummySetSessionDescriptionObserver(bool remote) : remote_(remote) {}

private:
  bool remote_;
};

class CreateSdpObserver : public webrtc::CreateSessionDescriptionObserver {
public:
  explicit CreateSdpObserver(WebrtcConnection* peer) : peer_(peer) {}

  void OnSuccess(webrtc::SessionDescriptionInterface* desc) override {
    std::string out;
    desc->ToString(&out);
    LOG(INFO) << ">>>>>>WebrtcConnection OnSuccess:" << desc->type() << "\n" << out;
    if (peer_ && peer_->webrtc_observer_) {
      peer_->webrtc_observer_->OnSdpCreateSuccess(peer_, desc->GetType(), out);
    }
  }

  void OnFailure(webrtc::RTCError error) override {
    LOG(ERROR) << ">>>>>>WebrtcConnection OnFailure:" << error.message();
    if (peer_ && peer_->webrtc_observer_) {
      peer_->webrtc_observer_->OnSdpCreateFailure(peer_, error);
    }
  }

private:
  WebrtcConnection* peer_;
};

WebrtcConnection::WebrtcConnection(uint64_t id, WebrtcPeerConnectionObserver* observer,
                                   const webrtc::PeerConnectionInterface::RTCConfiguration& config)
    : id_(id), webrtc_observer_(observer), rtc_config_(config) {
  auto pcFactory = WebrtcWrapper::Instance()->CreatePCFactory();
  pc_ = pcFactory->CreatePeerConnection(config, nullptr, nullptr, this);
}

void WebrtcConnection::SetRemoteDescription(std::unique_ptr<webrtc::SessionDescriptionInterface> desc) {
  pc_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(true), desc.release());
}

void WebrtcConnection::SetLocalDescription(std::unique_ptr<webrtc::SessionDescriptionInterface> desc) {
  pc_->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc.release());
}

void WebrtcConnection::CreateOffer(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions& options) {
  /// The CreateSessionDescriptionObserver callback will be called when done
  pc_->CreateAnswer(new rtc::RefCountedObject<CreateSdpObserver>(this), options);
}

void WebrtcConnection::CreateAnswer(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions& options) {
  /// The CreateSessionDescriptionObserver callback will be called when done.
  pc_->CreateAnswer(new rtc::RefCountedObject<CreateSdpObserver>(this), options);
}

void WebrtcConnection::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {
  LOG(INFO) << ">>>>>>WebrtcConnection OnSignalingChange:" << WebrtcWrapper::SignalingStateToString(new_state);
}

void WebrtcConnection::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {
  LOG(INFO) << ">>>>>>WebrtcConnection OnDataChannel label:" << data_channel->label();
}

void WebrtcConnection::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
  LOG(INFO) << ">>>>>>WebrtcConnection OnIceGatheringChange:" << WebrtcWrapper::IceGatheringStateToString(new_state);
}

void WebrtcConnection::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  std::string out;
  candidate->ToString(&out);
  LOG(INFO) << ">>>>>>WebrtcConnection OnIceCandidate:" << out;
  if (webrtc_observer_) {
    webrtc_observer_->OnIceCandidate(this, candidate);
  }
}

void WebrtcConnection::OnRenegotiationNeeded() {
  LOG(INFO) << ">>>>>>WebrtcConnection OnRenegotiationNeeded";
}

void WebrtcConnection::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
  LOG(INFO) << ">>>>>>WebrtcConnection OnIceConnectionChange:" << WebrtcWrapper::IceConnectionStateToString(new_state);
}

void WebrtcConnection::OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) {
  LOG(INFO) << ">>>>>>WebrtcConnection OnConnectionChange:"
            << WebrtcWrapper::PeerConnectionStateToString(static_cast<int>(new_state));

  std::optional<bool> connected;
  if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kConnected) {
    connected = true;
  } else if (new_state == webrtc::PeerConnectionInterface::PeerConnectionState::kDisconnected) {
    connected = false;
  }

  if (connected.has_value() && webrtc_observer_) {
    if (connected)
      webrtc_observer_->OnPeerConnect(this);
    else
      webrtc_observer_->OnPeerDisconnect(this);
  }
}

void WebrtcConnection::OnIceConnectionReceivingChange(bool receiving) {
  LOG(INFO) << ">>>>>>WebrtcConnection OnIceConnectionReceivingChange:" << receiving;
}

void WebrtcConnection::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
                                  const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) {
  LOG(INFO) << ">>>>>>WebrtcConnection OnAddTrack " << "\n"
            << "media type:" << receiver->media_type() << "\n"
            << "track id:" << receiver->track()->id() << "\n"
            << "track kind:" << receiver->track()->kind() << "\n";
  if (receiver->track()->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
    auto* video_track = static_cast<webrtc::VideoTrackInterface*>(receiver->track().get());
    if (webrtc_observer_) {
      webrtc_observer_->OnAddRemoteVideoTrack(this, video_track);
    }
  } else if (receiver->track()->kind() == webrtc::MediaStreamTrackInterface::kAudioKind) {
    auto* audio_track = static_cast<webrtc::AudioTrackInterface*>(receiver->track().get());
    if (webrtc_observer_) {
      webrtc_observer_->OnAddRemoteAudioTrack(this, audio_track);
    }
  }
}

void WebrtcConnection::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
  /// This will only be called if Unified Plan semantics are specified
  LOG(INFO) << ">>>>>>WebrtcConnection OnTrack";
}

void WebrtcConnection::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
  LOG(INFO) << ">>>>>>WebrtcConnection OnRemoveTrack " << "\n"
            << "media type:" << receiver->media_type() << "\n"
            << "track id:" << receiver->track()->id() << "\n"
            << "track kind:" << receiver->track()->kind() << "\n";
  if (receiver->track()->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
    auto* video_track = static_cast<webrtc::VideoTrackInterface*>(receiver->track().get());
    if (webrtc_observer_) {
      webrtc_observer_->OnRemoveRemoteVideoTrack(this, video_track);
    }
  } else if (receiver->track()->kind() == webrtc::MediaStreamTrackInterface::kAudioKind) {
    auto* audio_track = static_cast<webrtc::AudioTrackInterface*>(receiver->track().get());
    if (webrtc_observer_) {
      webrtc_observer_->OnRemoveRemoteAudioTrack(this, audio_track);
    }
  }
}

}