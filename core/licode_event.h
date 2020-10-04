//
// Created by nbaiot@126.com on 2020/10/2.
//

#ifndef LICODECPPCLIENT_LICODE_EVENT_H
#define LICODECPPCLIENT_LICODE_EVENT_H

#include <string>

namespace nbaiot {


struct LicodeEvent {
  explicit LicodeEvent(std::string type) : type(std::move(type)) {}

  std::string type;
};

/// 'stream-added' - a stream has been added to the connection.
/// 'stream-removed' - a stream has been removed from the connection.
/// 'ice-state-change' - ICE state changed
/// There is currently only one possible event type called
/// `connection-failed` that is triggered when the signaling negotiation inside a connection times out.
struct ConnectionEvent : public LicodeEvent {

};

/// 'room-connected' - points out that the user has been successfully connected to the room.
/// 'room-disconnected' - shows that the user has been already disconnected
/// 'room-error' - indicates that it hasn't been possible a succesufully connection to the room.
struct RoomEvent : public LicodeEvent {
//  A list with the streams that are published in the room.
//  that.streams = spec.streams;
//  that.message = spec.message;
};


/// 'stream-added' - indicates that there is a new stream available in the room.
/// 'stream-removed' - shows that a previous available stream has been removed from the room.
/// 'stream-data' - thrown by the stream it indicates new data received in the stream.
/// 'stream-attributes-update' - notifies when the owner of the given stream updates its attributes
/// 'bandwidth-aler' -  thrown when a subscriber stream is reporting less than the `minVideoBW` specified in the publisher. The event has three parts:
///    - `streamEvent.stream` is the problematic subscribe stream.
///    - `streamEvent.bandwidth` is the available bandwidth reported by that stream.
///    - `streamEvent.msg` the status of that stream, depends on the adaptation.
struct StreamEvent : public LicodeEvent {
//  The stream related to this event.
//  that.stream = spec.stream;
//
//  that.msg = spec.msg;
//  that.bandwidth = spec.bandwidth;
//  that.attrs = spec.attrs;

};

/// 'access-accepted' - indicates that the user has accepted to share his camera and microphone
struct PublisherEvent : public LicodeEvent {

};

}


#endif //LICODECPPCLIENT_LICODE_EVENT_H
