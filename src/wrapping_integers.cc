#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return Wrap32 { ( zero_point + static_cast<uint32_t>( n % Wrap32::mod ) ) };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  const uint32_t diff = ( this->raw_value_ - zero_point.raw_value_ + Wrap32::mod ) % Wrap32::mod;
  if ( diff < checkpoint ) {
    const uint64_t ans = checkpoint - diff + ( Wrap32::mod >> 1 );
    const uint64_t num = ans / Wrap32::mod;
    return num * Wrap32::mod + diff;
  }
  return diff;
}
