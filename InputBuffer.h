#ifndef INPUT_BUFFER_H
#define INPUT_BUFFER_H

#include <fstream>
#include <memory>

class InputBuffer
{
  public:
    InputBuffer( const char* filename )
    {
        // Open the stream at the end, so we can get the file size.
        std::ifstream in( filename, std::ifstream::ate | std::ifstream::binary );
        m_isValid   = !in.fail();
        size_t size = m_isValid ? static_cast<size_t>( in.tellg() ) : 0;

        // Allocate buffer.
        m_buffer.reset( new char[size + 1] );

        if( m_isValid )
        {
            // Rewind and read entire file
            in.clear();  // clear EOF
            in.seekg( 0, std::ios::beg );
            in.read( m_buffer.get(), size );
        }

        // The buffer is null-terminated (for the benefit of the Lexer).
        m_buffer.get()[size] = '\0';
    }

    bool IsValid() const { return m_isValid; }

    const char* Get() const { return m_buffer.get(); }

  private:
    std::unique_ptr<char[]> m_buffer;
    bool                    m_isValid;

    size_t getSize( const char* filename )
    {
        std::ifstream in( filename, std::ifstream::ate | std::ifstream::binary );
        return in.fail() ? 0 : static_cast<size_t>( in.tellg() );
    }
};

#endif
