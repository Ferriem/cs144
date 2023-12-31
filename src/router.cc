#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  route_table.emplace_back( route_t { route_prefix, prefix_length, next_hop, interface_num } );
}

void Router::route()
{
  for ( auto& ani : interfaces_ ) {
    while ( auto dgram = ani.maybe_receive() ) {
      if ( !dgram )
        continue;
      auto datagram_ip = dgram.value();
      if ( datagram_ip.header.ttl-- <= 1 ) {
        continue;
      }
      int8_t max_prefix_length = -1;
      route_t ans_r {};
      for ( route_t& r : route_table ) {
        if ( r.prefix_length == 0 ) {
          max_prefix_length = r.prefix_length;
          ans_r = r;
          continue;
        }
        uint32_t mask = ~0U;
        mask <<= static_cast<uint32_t>( 32 - r.prefix_length );
        if ( ( r.route_prefix & mask ) == ( datagram_ip.header.dst & mask ) ) {
          if ( r.prefix_length > max_prefix_length ) {
            max_prefix_length = static_cast<uint8_t>( r.prefix_length );
            ans_r = r;
          }
        }
      }
      if ( max_prefix_length >= 0 ) {
        datagram_ip.header.compute_checksum();
        AsyncNetworkInterface& out_interface = interfaces_[ans_r.interface_num];
        out_interface.send_datagram( datagram_ip,
                                     ans_r.next_hop.has_value()
                                       ? ans_r.next_hop.value()
                                       : Address::from_ipv4_numeric( datagram_ip.header.dst ) );
      }
    }
  }
}
