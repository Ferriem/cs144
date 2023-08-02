#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  uint32_t addr_numeric = next_hop.ipv4_numeric();
  if(arp_table.contains(addr_numeric)){
    EthernetFrame eth_frame;
    eth_frame.header.src = ethernet_address_;
    eth_frame.header.dst = arp_table[addr_numeric].eth_addr;
    eth_frame.header.type = EthernetHeader::TYPE_IPv4;
    eth_frame.payload = serialize(dgram);
    outbound_frames.push(eth_frame);
  }else{
    if(arp_life.find(addr_numeric) == arp_life.end()){
      ARPMessage arp_msg;
      arp_msg.opcode = ARPMessage::OPCODE_REQUEST;
      arp_msg.sender_ethernet_address = ethernet_address_;
      arp_msg.sender_ip_address = ip_address_.ipv4_numeric();
      arp_msg.target_ethernet_address = {};
      arp_msg.target_ip_address = addr_numeric;

      EthernetFrame arp_eth_frame;
      arp_eth_frame.header.src = ethernet_address_;
      arp_eth_frame.header.dst = ETHERNET_BROADCAST;
      arp_eth_frame.header.type = EthernetHeader::TYPE_ARP;
      arp_eth_frame.payload = serialize(arp_msg);
      outbound_frames.push(arp_eth_frame);
      
      arp_life.emplace(addr_numeric, ARP_requese_ttl);
    }
  }
  arp_list.emplace_back(next_hop, dgram);
}

// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  if(frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST){
    return nullopt;
  }
  if(frame.header.type == EthernetHeader::TYPE_IPv4){
    InternetDatagram dgram;
    if(not parse(dgram, frame.payload)){
      return dgram;
    }
    return nullopt;
  }else if (frame.header.type == EthernetHeader::TYPE_ARP){
    ARPMessage arp_msg;
    if(not parse(arp_msg, frame.payload)){
      return nullopt;
    }
    bool arp_request = arp_msg.opcode == ARPMessage::OPCODE_REQUEST && arp_msg.target_ip_address == ip_address_.ipv4_numeric();
    bool arp_reply = arp_msg.opcode == ARPMessage::OPCODE_REPLY && arp_msg.target_ip_address == ip_address_.ipv4_numeric();
    if(arp_request){
      ARPMessage arp_reply_msg;
      arp_reply_msg.opcode = ARPMessage::OPCODE_REPLY;
      arp_reply_msg.sender_ethernet_address = ethernet_address_;
      arp_reply_msg.sender_ip_address = ip_address_.ipv4_numeric();
      arp_reply_msg.target_ethernet_address = arp_msg.sender_ethernet_address;
      arp_reply_msg.target_ip_address = arp_msg.sender_ip_address;

      EthernetFrame arp_reply_eth_frame;
      arp_reply_eth_frame.header.src = ethernet_address_;
      arp_reply_eth_frame.header.dst = arp_msg.sender_ethernet_address;
      arp_reply_eth_frame.header.type = EthernetHeader::TYPE_ARP;
      arp_reply_eth_frame.payload = serialize(arp_reply_msg);
      outbound_frames.push(arp_reply_eth_frame);
    }
    if(arp_request || arp_reply){
      arp_table.emplace(arp_msg.sender_ip_address, arp_t{arp_msg.sender_ethernet_address, ARP_ttl});
      if(arp_reply){
        for(auto iter = arp_list.begin(); iter != arp_list.end();){
          auto &[ipv4_addr, datagram] = *iter;
          if(ipv4_addr.ipv4_numeric() == arp_msg.sender_ip_address){
            send_datagram(datagram, ipv4_addr);
            iter = arp_list.erase(iter);
          }else{
            iter++;
          }
        }
        arp_life.erase(arp_msg.sender_ip_address);
      }
    }
  }
  return nullopt;
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  for(auto iter = arp_table.begin(); iter != arp_table.end();){
    auto &[ipv4_addr, arp_] = *iter;
    arp_.ttl -= ms_since_last_tick;
    if(arp_.ttl <= 0){
      iter = arp_table.erase(iter);
    }else{
      iter++;
    }
  }
  for(auto &[ipv4_addr, ttl] : arp_life){
    ttl -= ms_since_last_tick;
    if(ttl <= 0){
      ARPMessage arp_msg;
      arp_msg.opcode = ARPMessage::OPCODE_REQUEST;
      arp_msg.sender_ethernet_address = ethernet_address_;
      arp_msg.sender_ip_address = ip_address_.ipv4_numeric();
      arp_msg.target_ethernet_address = {};
      arp_msg.target_ip_address = ipv4_addr;
      EthernetFrame arp_eth_frame;
      arp_eth_frame.header.src = ethernet_address_;
      arp_eth_frame.header.dst = ETHERNET_BROADCAST;
      arp_eth_frame.header.type = EthernetHeader::TYPE_ARP;
      arp_eth_frame.payload = serialize(arp_msg);
      outbound_frames.push(arp_eth_frame);
      ttl = ARP_requese_ttl;
    }
  }
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if(!outbound_frames.empty()){
    EthernetFrame eth_frame = outbound_frames.front();
    outbound_frames.pop();
    return eth_frame;
  }
  return nullopt;
}
