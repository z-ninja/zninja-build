#ifndef BREE_ENDIAN_H
#define BREE_ENDIAN_H
#include <stdint.h>
#include <iostream>
#include <vector>
#include <string.h>
bool is_big_endian();
uint16_t bree_swap_uint16( uint16_t& a_val);
int16_t bree_swap_int16( int16_t& a_val);
uint32_t bree_swap_uint32( uint32_t& a_val);
int32_t bree_swap_int32( int32_t& a_val);
int64_t bree_swap_int64( int64_t& a_val);
uint64_t bree_swap_uint64( uint64_t& a_val);
float bree_swap_float(float& a_val);
double bree_swap_double(double& a_val);

void bree_readUInt16LE( uint16_t& a_val );
void bree_writeUInt16LE( uint16_t& a_val );
void bree_readUInt16BE( uint16_t& a_val );
void bree_writeUInt16BE( uint16_t& a_val );


void bree_readInt16LE( int16_t& a_val );
void bree_writeInt16LE( int16_t& a_val );
void bree_readInt16BE( int16_t& a_val );
void bree_writeInt16BE( int16_t& a_val );

void bree_readUInt32LE( uint32_t& a_val );
void bree_writeUInt32LE( uint32_t& a_val );
void bree_readUInt32BE( uint32_t& a_val );
void bree_writeUInt32BE( uint32_t& a_val );


void bree_readInt32LE( int32_t& a_val );
void bree_writeInt32LE( int32_t& a_val );
void bree_readInt32BE( int32_t& a_val );
void bree_writeInt32BE( int32_t& a_val );


void bree_readUInt64LE( uint64_t& a_val );
void bree_writeUInt64LE( uint64_t& a_val );
void bree_readUInt64BE( uint64_t& a_val );
void bree_writeUInt64BE( uint64_t& a_val );

void bree_readInt64LE( int64_t& a_val );
void bree_writeInt64LE( int64_t& a_val );
void bree_readInt64BE( int64_t& a_val );
void bree_writeInt64BE( int64_t& a_val );


void bree_readFloatLE( float& a_val );
void bree_writeFloatLE( float& a_val );
void bree_readFloatBE( float& a_val );
void bree_writeFloatBE( float& a_val );


void bree_readDoubleLE( double& a_val );
void bree_writeDoubleLE( double& a_val );
void bree_readDoubleBE( double& a_val );
void bree_writeDoubleBE( double& a_val );

void bree_readInt8(std::istream& a_stream,int8_t&a_int8);
void bree_writeInt8(std::ostream& a_stream,int8_t a_int8);
void bree_readUInt8(std::istream& a_stream,uint8_t&a_uint8);
void bree_writeUInt8(std::ostream& a_stream,uint8_t a_uint8);

void bree_readUInt16LE(std::istream&, uint16_t& a_val );
void bree_writeUInt16LE(std::ostream&, uint16_t a_val );
void bree_readUInt16BE(std::istream&, uint16_t& a_val );
void bree_writeUInt16BE(std::ostream&, uint16_t a_val );

void bree_readInt16LE(std::istream&, int16_t& a_val );
void bree_writeInt16LE(std::ostream&, int16_t a_val );
void bree_readInt16BE(std::istream&, int16_t& a_val );
void bree_writeInt16BE(std::ostream&, int16_t a_val );


void bree_readUInt32LE(std::istream&, uint32_t& a_val );
void bree_writeUInt32LE(std::ostream&, uint32_t a_val );
void bree_readUInt32BE(std::istream&, uint32_t& a_val );
void bree_writeUInt32BE(std::ostream&, uint32_t a_val );


void bree_readInt32LE(std::istream&,  int32_t& a_val );
void bree_writeInt32LE(std::ostream&, int32_t a_val );
void bree_readInt32BE(std::istream&,  int32_t& a_val );
void bree_writeInt32BE(std::ostream&, int32_t a_val );


void bree_readUInt64LE(std::istream&, uint64_t& a_val );
void bree_writeUInt64LE(std::ostream&, uint64_t a_val );
void bree_readUInt64BE(std::istream&, uint64_t& a_val );
void bree_writeUInt64BE(std::ostream&, uint64_t a_val );


void bree_readInt64LE(std::istream&, int64_t& a_val );
void bree_writeInt64LE(std::ostream&, int64_t a_val );
void bree_readInt64BE(std::istream&, int64_t& a_val );
void bree_writeInt64BE(std::ostream&, int64_t a_val );

void bree_readTimeLE(std::istream&, time_t& a_val );
void bree_writeTimeLE(std::ostream&, time_t a_val );
void bree_readTimeBE(std::istream&, time_t& a_val );
void bree_writeTimeBE(std::ostream&, time_t a_val );


void bree_readFloatLE(std::istream&, float& a_val );
void bree_writeFloatLE(std::ostream&, float a_val );
void bree_readFloatBE(std::istream&, float& a_val );
void bree_writeFloatBE(std::ostream&, float a_val );


void bree_readDoubleLE(std::istream&, double& a_val );
void bree_writeDoubleLE(std::ostream&, double a_val );
void bree_readDoubleBE(std::istream&, double& a_val );
void bree_writeDoubleBE(std::ostream&, double a_val );



void    bree_writeUInt8_array(std::ostream& a_stream,uint8_t*a_array,uint32_t a_size);
uint32_t bree_readUInt8_array(std::istream& a_stream,uint8_t*a_array,uint32_t a_size);
void     bree_readUInt8_array_explicit(std::istream& a_stream,uint8_t*a_array,uint32_t a_size);


#endif // BREE_ENDIAN_H
