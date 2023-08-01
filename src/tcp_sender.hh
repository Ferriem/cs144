#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include <map>

class TCPSender
{
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;
  uint64_t RTO_ms_ {initial_RTO_ms_};
  uint64_t timer {0};
  bool set_syn {};
  bool set_fin {};
  uint64_t window_size {1};
  uint64_t out_seqno {0};
  uint64_t next_abs_seqno {0};
  uint64_t consecutive_retransmissions_ {0};
  std::queue<TCPSenderMessage> message_queue {};
  std::map<uint64_t, TCPSenderMessage> sent_messages {};
  Wrap32 get_next_seqno() const {return isn_ + next_abs_seqno;}
  uint64_t get_next_abs_seqno() const {return next_abs_seqno;}


public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
};
