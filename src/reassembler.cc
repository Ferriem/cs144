#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  if ( is_last_substring )
    closed_ = true;

  if ( first_index + data.size() <= bytes_now_ || first_index >= bytes_now_ + output.available_capacity()
       || data.empty() || output.available_capacity() == 0 ) {
    if ( closed_ && bytes_pending() == 0 ) {
      output.close();
    }
    return;
  }

  if ( first_index < bytes_now_ ) {
    uint64_t overlap_len = bytes_now_ - first_index;
    data = data.substr( overlap_len, min( data.size() - overlap_len, output.available_capacity() ) );
    first_index = bytes_now_;
  } else {
    data = data.substr( 0, min( data.size(), output.available_capacity() ) );
    if ( first_index + data.size() > bytes_now_ + output.available_capacity() ) {
      data = data.substr( 0, output.available_capacity() + bytes_now_ - first_index );
    }
  }

  auto iter = data_.lower_bound( first_index );
  while ( iter != data_.end() ) {
    auto& [index, str] = *iter;
    if ( index >= first_index + data.size() )
      break;
    uint64_t overlap_len = 0;
    uint64_t next = 0;
    if ( first_index + data.size() < index + str.size() ) {
      overlap_len = first_index + data.size() - index;
      data = data.substr( 0, data.size() - overlap_len );
      next = index + str.size();
    } else {
      overlap_len = str.size();
      unreassembled_bytes_ -= overlap_len;
      next = index + str.size();
      data_.erase( index );
    }
    iter = data_.lower_bound( next );
  }

  if ( first_index > bytes_now_ ) {
    auto iter_front = data_.upper_bound( first_index );
    if ( iter_front != data_.begin() ) {
      iter_front--;
      auto& [index, str] = *iter_front;
      if ( index + str.size() > first_index ) {
        if ( first_index + data.size() <= index + str.size() ) {
          data = "";
        } else {
          uint64_t overlap_len = index + str.size() - first_index;
          data = data.substr( overlap_len, data.size() - overlap_len );
          first_index = first_index + overlap_len;
        }
      }
    }
  }

  if ( !data.empty() ) {
    unreassembled_bytes_ += data.size();
    data_.insert( { first_index, data } );
  }

  for ( auto iter_write = data_.begin(); iter_write != data_.end(); ) {
    auto& [index, str] = *iter_write;
    if ( index == bytes_now_ ) {
      uint64_t prev_bytes_pushed = output.bytes_pushed();
      output.push( str );
      uint64_t bytes_pushed = output.bytes_pushed();
      if ( bytes_pushed != prev_bytes_pushed + str.size() ) {
        uint64_t pushed_len = bytes_pushed - prev_bytes_pushed;
        str = str.substr( pushed_len, str.size() - pushed_len );
        unreassembled_bytes_ -= pushed_len;
        bytes_now_ += pushed_len;
        data_.erase( index );
        data_.insert( { bytes_now_, str } );
        break;
      }
      unreassembled_bytes_ -= str.size();
      bytes_now_ += str.size();
      data_.erase( index );
      iter_write = data_.find( bytes_now_ );
    } else {
      break;
    }
  }
  if ( closed_ && bytes_pending() == 0 ) {
    output.close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return unreassembled_bytes_;
}
