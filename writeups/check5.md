## Check-5

- [check5.pdf](https://cs144.github.io/assignments/check5.pdf)

- Implement the route to forward datagram to the next hop.

- We need some space to restore the routing table.

  ```c++
  typedef struct {
    uint32_t route_prefix;
    uint8_t prefix_length;
    std::optional<Address> next_hop;
    size_t interface_num;
  }route_t;
  class Router
  {
    std::list<route_t> route_table {};
  }
  ```

- ```c++
  void Router::add_route( const uint32_t route_prefix,
                          const uint8_t prefix_length,
                          const optional<Address> next_hop,
                          const size_t interface_num )
  ```

  This function is to add to route_table for following matching.

  ```c++
  	route_table.emplace_back( route_t{ route_prefix, prefix_length, next_hop, interface_num } );
  ```

- As for `route()`
  - When a router route the datagram, it receive the datagram from interfaces, search for the route_table to find the max prefix_length, and then send it to the proper place.
  - Go through all the interface and get the dgram in them.
  - If we already have a datagram, inc its ttl, if the ttl decreased to 0, it means the datagram should be dropped, do nothing to it.
  - Then we need to find the max prefix_length.
  - Go through the route_table, find the max prefix_length matching datagram's destination address.
  - If matches(`max_prefix_length >= 0`), recompute the checksum(becase we modify the ttl), send the datagram(using `send datagram` in *network_interface.cc* we've handled before).

