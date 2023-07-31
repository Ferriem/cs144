#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  if ( closed_ || error_ || data.empty() )
    return;
  uint64_t len = min( data.size(), available_capacity() );
  buf.append( data.substr( 0, len ) );
  bytes_written_ += len;
}

void Writer::close()
{
  closed_ = true;
  // Your code here.
}

void Writer::set_error()
{
  error_ = true;
  // Your code here.
}

bool Writer::is_closed() const
{
  // Your code here.
  return closed_;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - bytes_written_ + bytes_read_;
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return bytes_written_;
}

string_view Reader::peek() const
{
  // Your code here.
  return buf;
}

bool Reader::is_finished() const
{
  // Your code here.
  return closed_ && buf.empty();
}

bool Reader::has_error() const
{
  // Your code here.
  return error_;
}

void Reader::pop( uint64_t len )
{
  if ( buf.empty() )
    return;
  // Your code here.
  len = min( len, buf.size() );
  buf.erase( buf.begin(), buf.begin() + static_cast<int64_t>( len ) );
  bytes_read_ += len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return bytes_written_ - bytes_read_;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return bytes_read_;
}
