#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return out_seqno;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return consecutive_retransmissions_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  if(set_syn && !message_queue.empty()){
    TCPSenderMessage message;
    message = message_queue.front();
    message_queue.pop();
    return message;
  }
  return nullopt;
}

void TCPSender::push(Reader &outbound_stream) {
    const uint64_t curr_window_size = window_size ? window_size : 1;
    while (curr_window_size > out_seqno) {
        TCPSenderMessage msg;

        if (!set_syn) {
            msg.SYN = true;
            set_syn = true;
        }

        msg.seqno = get_next_seqno();
        const uint64_t payload_size
                = min(TCPConfig::MAX_PAYLOAD_SIZE, curr_window_size - out_seqno - msg.SYN);
        std::string payload = std::string(outbound_stream.peek()).substr(0, payload_size);
        outbound_stream.pop(payload_size);

        if (!set_fin && outbound_stream.is_finished()
            && payload.size() + out_seqno + msg.SYN < curr_window_size) {
            msg.FIN = true;
            set_fin = true;
        }

        msg.payload = Buffer(std::move(payload));

        // no data, stop sending
        if (msg.sequence_length() == 0) {
            break;
        }

        // no outstanding segments, restart timer
        if (message_queue.empty()) {
            RTO_ms_ = initial_RTO_ms_;
            timer = 0;
        }

        message_queue.push(msg);

        out_seqno += msg.sequence_length();
        sent_messages.insert(std::make_pair(next_abs_seqno, msg));
        next_abs_seqno += msg.sequence_length();

        if (msg.FIN) {
            break;
        }
    }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  TCPSenderMessage message;
  message.seqno = get_next_seqno();
  return message;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  if(msg.ackno.has_value()){
    uint64_t recv_abs_seqno = msg.ackno.value().unwrap(isn_, next_abs_seqno);
    if(recv_abs_seqno > next_abs_seqno){
      return;
    }
    for(auto iter = sent_messages.begin(); iter != sent_messages.end();){
      auto &[seqno, message] = *iter;
      if(seqno + message.sequence_length() <= recv_abs_seqno){
        out_seqno -= message.sequence_length();
        iter = sent_messages.erase(iter);
        RTO_ms_ = initial_RTO_ms_;
        if(!message_queue.empty()){
          timer = 0;
        }
      }else{
        break;
      }
    }
    consecutive_retransmissions_ = 0;
  }
  window_size = msg.window_size;
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  timer += ms_since_last_tick;
  if(timer >= RTO_ms_ && !sent_messages.empty()){
    auto &[seqno, message] = *sent_messages.begin();
    message_queue.push(message);
    consecutive_retransmissions_++;
    if(window_size){
      RTO_ms_ *= 2;
    }
    timer = 0;
  }
}
