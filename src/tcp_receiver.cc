#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.
  if ( !isn_set ) {
    if ( !message.SYN ) {
      return;
    }
    isn = message.seqno;
    isn_set = true;
  }
  uint64_t const checkpoint = inbound_stream.bytes_pushed() + 1;
  uint64_t const absolute_seqno = message.seqno.unwrap( isn, checkpoint );
  uint64_t const index = absolute_seqno - 1 + message.SYN;
  reassembler.insert( index, message.payload.release(), message.FIN, inbound_stream );
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  TCPReceiverMessage message {};
  uint16_t const window_size = inbound_stream.available_capacity() > UINT16_MAX ? UINT16_MAX : inbound_stream.available_capacity();
  if ( !isn_set ) {
    return { std::optional<Wrap32> {}, window_size };
  }
  message.window_size = window_size;
  message.ackno = isn + inbound_stream.bytes_pushed() + 1 + inbound_stream.is_closed();
  return message;
}
