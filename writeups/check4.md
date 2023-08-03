## Check-4

- [check4.pdf](https://cs144.github.io/assignments/check4.pdf)

- **Target**: Implement network interface to pass and send the message with correct dst and src.

- As we known, an interface must record its **MAC address**. **IP address**, **arp table**. Thus we keep some necessary variables to record the status in *network_interface.hh*

- For arp, we need two invariables of request time and valid time. One is to judge the arp request's timeout status, another is to judge the valid status in arp_table.

  ```c++
  class NetworkInterface
  {
  private:
    // Ethernet (known as hardware, network-access, or link-layer) address of the interface
    EthernetAddress ethernet_address_; // MAC address 
    size_t ARP_requese_ttl = static_cast<size_t> (5000); // 5 seconds
    size_t ARP_ttl = static_cast<size_t> (30000); // 30 seconds
    // IP (known as Internet-layer or network-layer) address of the interface
    Address ip_address_; // IP address
    typedef struct arp{
      EthernetAddress eth_addr;
      size_t ttl;
    }arp_t; // ARP table entry
    std::unordered_map<uint32_t, arp_t> arp_table {}; // ARP table
    std::queue<EthernetFrame> outbound_frames {}; // Frames to be sent
    std::unordered_map<uint32_t, size_t> arp_life {}; //arp request sent table
    std::list<std::pair<Address, InternetDatagram>> arp_list{}; //arp request sent list, pass time ,resend
  }
  ```

- Now implement the main functions

- First we have something to notice. In a Ethernetframe, the destination and resource ip address are needed, also the resource mac address. But the destination's mac address is not necessary, since we need to get it by Ethernetframe(ARP request).

- `void NetworkInterface::send datagram(const InternetDatagram &dgram, const Address &next_hop)` 

  - This function is just to send a datagram to next_hop.
  - First, search for the arp cache to find whether the next_hop's MAC address is buffered.
  -  If buffered, create a `EthernetFrame` using the data in arp_table to prepare to be sent. 
  - Else, send a arp request datagram to get the Mac address of next_hop and buffer it to the arp cache.
  - Inorder to record the arp reply and arp timeout status, we buffer the dst in `arp_life`, buffer the datagram in `arp_list`.

- `optional NetworkInterface::recv_frame(const EthernetFrame &frame)`

  - This function is when we receive a `EthernetFrame` and do some actions. Remember that if the datagram's dst's ip address isn't the interface's ip address, do nothing.
  - If the frame is a IPv4 datagram,  parse the payload as an `InternetDatagram` and, if successful (meaning the parse() method returned true), return the resulting InternetDatagram to the caller.
  - If the frame is a ARP datagram, there are two situations.
    - Request ARP datagram, return with the interface's MAC address, wrap the arp message in EthernetFrame, push it to outbound, waiting to send.
    - Reply ARP datagram, it means last time we send a datagram but don't know the destination's MAC address, now we get the address, resend the datagram from arp_list, then remove it. Remember to remove it from arp_life because it is already rerurned.
    - When we get the ARP datagram, we all should cache the map between ip address and mac address in arp_table with a valid time.

- `optional<EthernetFrame> NetworkInterface::maybe_send()`

  - Just as we do in tcpsender, search the outbound queue.
  - If it is not empty, return the front one. 
  - Else, return nullopt

- `void NetworkInterface::tick( const size_t ms_since_last_tick )`

  - It is also similar to the `tcpsender`'s tick function.
  - First go through the arp table, if the valid time is passed, remove it from the arp_table.
  - As for the timeout situation, we buffered the ttl in arp_life, if the ttl is passed, that means the arp request is timeout and we need to resend it. Generate the datagram, update the ttl.