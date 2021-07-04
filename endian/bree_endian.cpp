#include "bree_endian.h"


#ifdef OS_WINDOWS
#include <Winsock2.h>
#elif defined OS_LINUX
#include <arpa/inet.h>
#elif defined OS_MAC
#include <arpa/inet.h>// ?
#endif

bool is_big_endian(){
    static bool it_is = (htonl(1) == 1);
    return it_is;
}

 uint16_t bree_swap_uint16( uint16_t& a_val )
{
    return (a_val << 8) | (a_val >> 8 );
}
 void bree_readUInt16LE( uint16_t& a_val )
{

    if(is_big_endian()){
    a_val= bree_swap_uint16(a_val);
    }
}
 void bree_writeUInt16LE( uint16_t& a_val )
{

    if(is_big_endian()){
    a_val = bree_swap_uint16(a_val);
    }
}


 void bree_readUInt16BE( uint16_t& a_val )
{
if(!is_big_endian()){
    a_val = bree_swap_uint16(a_val);
    }

}
 void bree_writeUInt16BE( uint16_t& a_val )
{
if(!is_big_endian()){
    a_val = bree_swap_uint16(a_val);
}

}

 int16_t bree_swap_int16( int16_t& a_val )
{
    return (a_val << 8) | ((a_val >> 8) & 0xFF);
}


 void bree_readInt16LE( int16_t& a_val )
{
    if(is_big_endian()){
    a_val = bree_swap_int16(a_val);
    }
}
 void bree_writeInt16LE( int16_t& a_val )
{
    if(is_big_endian()){
    a_val = bree_swap_int16(a_val);
    }
}


 void bree_readInt16BE( int16_t& a_val )
{
    if(!is_big_endian()){
    a_val = bree_swap_int16(a_val);
    }
}


 void bree_writeInt16BE( int16_t& a_val )
{
    if(!is_big_endian()){
    a_val = bree_swap_int16(a_val);
    }
}


 uint32_t bree_swap_uint32( uint32_t& a_val )
{
    a_val = ((a_val << 8) & 0xFF00FF00 ) | ((a_val >> 8) & 0xFF00FF );
    return (a_val << 16) | (a_val >> 16);
}

 void bree_readUInt32LE( uint32_t& a_val )
{
    if(is_big_endian()){
    a_val = bree_swap_uint32(a_val);
    }
}
 void bree_writeUInt32LE( uint32_t& a_val )
{
    if(is_big_endian()){
    a_val = bree_swap_uint32(a_val);
    }
}

 void bree_readUInt32BE( uint32_t& a_val )
{
    if(!is_big_endian()){
    a_val =  bree_swap_uint32(a_val);
    }
}
 void bree_writeUInt32BE( uint32_t& a_val )
{
    if(!is_big_endian()){
    a_val =  bree_swap_uint32(a_val);
    }
}


 int32_t bree_swap_int32( int32_t& a_val )
{
    a_val = ((a_val << 8) & 0xFF00FF00) | ((a_val >> 8) & 0xFF00FF );
    return (a_val << 16) | ((a_val >> 16) & 0xFFFF);
}

 void bree_readInt32LE( int32_t& a_val )
{
    if(is_big_endian()){
    a_val= bree_swap_int32(a_val);
    }
}
 void bree_writeInt32LE( int32_t& a_val )
{
    if(is_big_endian()){
    a_val = bree_swap_int32(a_val);
    }
}


  void bree_readInt32BE( int32_t& a_val )
{
    if(!is_big_endian()){
    a_val = bree_swap_int32(a_val);
    }
}


 void bree_writeInt32BE( int32_t& a_val )
{
    if(!is_big_endian()){
    a_val = bree_swap_int32(a_val);
    }
}

 uint64_t bree_swap_uint64( uint64_t& a_val )
{
    a_val = ((a_val << 8) & 0xFF00FF00FF00FF00ULL ) | ((a_val >> 8) & 0x00FF00FF00FF00FFULL );
    a_val = ((a_val << 16) & 0xFFFF0000FFFF0000ULL ) | ((a_val >> 16) & 0x0000FFFF0000FFFFULL );
    return (a_val << 32) | (a_val >> 32);
}

 void bree_readUInt64LE( uint64_t& a_val )
{

    if(is_big_endian()){
    a_val = bree_swap_uint64(a_val);
    }
}
 void bree_writeUInt64LE( uint64_t& a_val )
{
    if(is_big_endian()){
    a_val = bree_swap_uint64(a_val);
    }
}

 void bree_readUInt64BE( uint64_t& a_val )
{
    if(!is_big_endian()){
    a_val = bree_swap_uint64(a_val);
    }
}
 void bree_writeUInt64BE( uint64_t& a_val )
{
    if(!is_big_endian()){
    a_val = bree_swap_uint64(a_val);
    }
}



 int64_t bree_swap_int64( int64_t& a_val )
{
    a_val = ((a_val << 8) & 0xFF00FF00FF00FF00ULL ) | ((a_val >> 8) & 0x00FF00FF00FF00FFULL );
    a_val = ((a_val << 16) & 0xFFFF0000FFFF0000ULL ) | ((a_val >> 16) & 0x0000FFFF0000FFFFULL );
    return (a_val << 32) | ((a_val >> 32) & 0xFFFFFFFFULL);
}

 void bree_readInt64LE( int64_t& a_val )
{
    if(is_big_endian()){
    a_val = bree_swap_int64(a_val);
    }
}
 void bree_writeInt64LE( int64_t& a_val )
{
    if(is_big_endian()){
    a_val = bree_swap_int64(a_val);
    }
}


 void bree_readInt64BE( int64_t& a_val )
{
    if(is_big_endian()){
    a_val = bree_swap_int64(a_val);
    }
}


 void bree_writeInt64BE( int64_t& a_val )
{
    if(!is_big_endian()){
    a_val = bree_swap_int64(a_val);
    }
}


 float bree_swap_float(float& x)
{
    union
    {
        float f;
        uint32_t ui32;
    } swapper;
    swapper.f = x;
    swapper.ui32 = bree_swap_uint32(swapper.ui32);
    return swapper.f;
}


 void bree_readFloatLE( float& a_val )
{
    if(is_big_endian()){
    a_val = bree_swap_float(a_val);
    }
}
 void bree_writeFloatLE( float& a_val )
{
    if(is_big_endian()){
    a_val = bree_swap_float(a_val);
    }
}


 void bree_readFloatBE( float& a_val )
{
    if(!is_big_endian()){
    a_val = bree_swap_float(a_val);
    }
}


 void bree_writeFloatBE( float& a_val )
{
    if(!is_big_endian()){
    a_val = bree_swap_float(a_val);
    }
}




 double bree_swap_double(double& x)
{
    union
    {
        double f;
        uint64_t ui64;
    } swapper;
    swapper.f = x;
    swapper.ui64 = bree_swap_uint64(swapper.ui64);
    return swapper.f;
}


 void bree_readDoubleLE( double& a_val )
{
    if(is_big_endian()){
    a_val = bree_swap_double(a_val);
    }
}
 void bree_writeDoubleLE( double& a_val )
{
    if(is_big_endian()){
    a_val = bree_swap_double(a_val);
    }
}


 void bree_readDoubleBE( double& a_val )
{
    if(!is_big_endian()){
    a_val = bree_swap_double(a_val);
    }

}


 void bree_writeDoubleBE( double& a_val )
{
    if(!is_big_endian()){
    a_val = bree_swap_double(a_val);
    }
}

 void bree_readInt8(std::istream& a_stream,int8_t&a_int8)
{
    a_stream.read(reinterpret_cast<char*>(&a_int8),1);
}
 void bree_writeInt8(std::ostream& a_stream,int8_t a_int8)
{
    a_stream.write(reinterpret_cast<char*>(&a_int8),1);
}

 void bree_readUInt8(std::istream& a_stream,uint8_t&a_uint8)
{
    a_stream.read(reinterpret_cast<char*>(&a_uint8),1);
}

 void bree_writeUInt8(std::ostream& a_stream,uint8_t a_uint8)
{
    a_stream.write(reinterpret_cast<char*>(&a_uint8),1);
}


 void bree_readUInt16LE(std::istream& a_stream, uint16_t& a_val )
{
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readUInt16LE(a_val);
}
 void bree_writeUInt16LE(std::ostream& a_stream, uint16_t a_val )
{
    bree_writeUInt16LE(a_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}
 void bree_readUInt16BE(std::istream& a_stream, uint16_t& a_val )
{
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readUInt16BE(a_val);
}
 void bree_writeUInt16BE(std::ostream& a_stream, uint16_t a_val )
{
    bree_writeUInt16BE(a_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}


 void bree_readInt16LE(std::istream& a_stream, int16_t& a_val )
{
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readInt16LE(a_val);
}
 void bree_writeInt16LE(std::ostream& a_stream, int16_t a_val )
{
    bree_writeInt16LE(a_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}
 void bree_readInt16BE(std::istream& a_stream, int16_t& a_val )
{
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readInt16BE(a_val);
}
 void bree_writeInt16BE(std::ostream& a_stream, int16_t a_val )
{
    bree_writeInt16BE(a_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}


 void bree_readUInt32LE(std::istream& a_stream, uint32_t& a_val )
{
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readUInt32LE(a_val);
}
 void bree_writeUInt32LE(std::ostream& a_stream, uint32_t a_val )
{
    bree_writeUInt32LE(a_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}
 void bree_readUInt32BE(std::istream& a_stream, uint32_t& a_val )
{
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readUInt32BE(a_val);
}
 void bree_writeUInt32BE(std::ostream& a_stream, uint32_t a_val )
{
    bree_writeUInt32BE(a_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}


 void bree_readInt32LE(std::istream& a_stream,  int32_t& a_val )
{
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readInt32LE(a_val);
}
 void bree_writeInt32LE(std::ostream& a_stream, int32_t a_val )
{
    bree_writeInt32LE(a_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}
 void bree_readInt32BE(std::istream& a_stream,  int32_t& a_val )
{
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readInt32BE(a_val);
}
 void bree_writeInt32BE(std::ostream& a_stream, int32_t a_val )
{
    bree_writeInt32BE(a_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}


 void bree_readUInt64LE(std::istream& a_stream, uint64_t& a_val )
{
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readUInt64LE(a_val);
}
 void bree_writeUInt64LE(std::ostream& a_stream, uint64_t a_val )
{
    bree_writeUInt64LE(a_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}
 void bree_readUInt64BE(std::istream& a_stream, uint64_t& a_val )
{
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readUInt64BE(a_val);
}
 void bree_writeUInt64BE(std::ostream& a_stream, uint64_t a_val )
{
    bree_writeUInt64BE(a_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}




 void bree_readInt64LE(std::istream& a_stream, int64_t& a_val )
{
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readInt64LE(a_val);
}
 void bree_writeInt64LE(std::ostream& a_stream, int64_t a_val )
{
    bree_writeInt64LE(a_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}
 void bree_readInt64BE(std::istream& a_stream, int64_t& a_val )
{
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readInt64BE(a_val);
}
 void bree_writeInt64BE(std::ostream& a_stream, int64_t a_val )
{
    bree_writeInt64BE(a_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}

 void bree_readTimeLE(std::istream& a_stream, time_t& a_val )
{
    int64_t l_val = 0;
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readInt64LE(l_val);
    a_val = static_cast<time_t>(l_val);
}
 void bree_writeTimeLE(std::ostream& a_stream, time_t a_val )
{
    int64_t l_val = static_cast<int64_t>(a_val);
    bree_writeInt64LE(l_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}
 void bree_readTimeBE(std::istream& a_stream, time_t& a_val )
{
    int64_t l_val = 0;
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readInt64BE(l_val);
    a_val = static_cast<time_t>(l_val);
}
 void bree_writeTimeBE(std::ostream& a_stream, time_t a_val )
{
    int64_t l_val = static_cast<int64_t>(a_val);
    bree_writeInt64BE(l_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}

 void bree_readFloatLE(std::istream& a_stream, float& a_val )
{
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readFloatLE(a_val);
}
 void bree_writeFloatLE(std::ostream& a_stream, float a_val )
{
    bree_writeFloatLE(a_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}
 void bree_readFloatBE(std::istream& a_stream, float& a_val )
{
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readFloatBE(a_val);
}
 void bree_writeFloatBE(std::ostream& a_stream, float a_val )
{
    bree_writeFloatBE(a_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}


 void bree_readDoubleLE(std::istream& a_stream, double& a_val )
{
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readDoubleLE(a_val);
}
 void bree_writeDoubleLE(std::ostream& a_stream, double a_val )
{
    bree_writeDoubleLE(a_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}
 void bree_readDoubleBE(std::istream& a_stream, double& a_val )
{
    a_stream.read((char*)&a_val,sizeof(a_val));
    bree_readDoubleBE(a_val);
}
 void bree_writeDoubleBE(std::ostream& a_stream, double a_val )
{
    bree_writeDoubleBE(a_val);
    a_stream.write((char*)&a_val,sizeof(a_val));
}



#define MIN(x, y) ( ((x)<(y))?(x):(y) )
 void bree_writeUInt8_array(std::ostream& a_stream, uint8_t*a_array,uint32_t a_size)
{
    bree_writeUInt32LE(a_stream,a_size);
    a_stream.write(reinterpret_cast<char*>(a_array),a_size);
}


 uint32_t bree_readUInt8_array(std::istream& a_stream,uint8_t*a_array,uint32_t a_size)
{
    uint32_t l_size = 0;
    bree_readUInt32LE(a_stream,l_size);
    l_size = MIN(a_size,l_size);
    a_size = l_size;
    a_stream.read(reinterpret_cast<char*>(a_array),l_size);
    return a_size - a_size;
}

 void bree_readUInt8_array_explicit(std::istream& a_stream,uint8_t*a_array,uint32_t a_size)
{
    a_stream.read(reinterpret_cast<char*>(a_array),a_size);
}


 uint32_t bree_writeShortString(std::ostream&a_stream,const std::string& a_val){
    uint8_t l_size=static_cast<uint8_t>(a_val.size());
    bree_writeUInt8(a_stream,l_size);
    a_stream.write(a_val.data(),l_size);
    return static_cast<uint32_t>(a_val.size())+1;

}
 uint32_t bree_readShortString(std::istream&a_stream,std::string& a_val){
    uint8_t l_size = 0;
    bree_readUInt8(a_stream,l_size);
    std::vector<char> tmp(l_size);
    a_stream.read(tmp.data(), l_size);
    a_val.assign(tmp.data(), l_size);
    return static_cast<uint32_t>(a_val.size())+1;
}
 uint32_t bree_writeShortString(std::ostream&a_stream,const std::string& a_val,uint8_t a_max_size){
    bree_writeShortString(a_stream,a_val);
    if(a_max_size > static_cast<uint8_t>(a_val.size()))
    {
        int l_un_garb = a_max_size -   static_cast<uint8_t>(a_val.size());
        char*garb = new char[l_un_garb+1];
        memset(garb,0,l_un_garb);
        a_stream.write(garb,l_un_garb);
        delete [] garb;
    }
    return static_cast<uint32_t>(a_max_size)+1;
}
 uint32_t bree_readShortString(std::istream&a_stream,std::string& a_val,uint8_t a_max_size)
{
    bree_readShortString(a_stream,a_val);
    if(a_max_size > static_cast<uint8_t>(a_val.size()))
    {
        int l_un_garb = a_max_size -   static_cast<uint8_t>(a_val.size());
        char*garb = new char[l_un_garb+1];
        a_stream.read(garb,l_un_garb);
        delete [] garb;
    }
    return static_cast<uint32_t>(a_max_size)+1;
}

