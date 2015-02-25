/*
Copyright (c) 2015, Mike Tzou
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

* Neither the name of foo_xspf_1 nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

const GUID guid_xspf_branch = { 0xd544a73a , 0xc272 , 0x49b9 , { 0xba , 0xaa , 0x22 , 0x49 , 0x62 , 0x33 , 0x6c , 0xc6 } };
const advconfig_branch_factory cfg_xspf_branch( "XSPF playlist" , guid_xspf_branch , advconfig_branch::guid_branch_tools , 0.0 );


// read
const GUID guid_branch_read = { 0x85ac082d , 0x1117 , 0x4d9e , { 0xaa , 0x18 , 0xa9 , 0x3d , 0xba , 0xd3 , 0xeb , 0x45 } };
const advconfig_branch_factory cfg_branch_read( "Read" , guid_branch_read , guid_xspf_branch , 0.0 );

const advconfig_checkbox_factory cfg_read_no_location( "<location>" ,
{ 0x59aa57db , 0xa2a3 , 0x45d3 , { 0xbc , 0x2 , 0xc0 , 0x9 , 0x69 , 0xc8 , 0x7b , 0xf7 } },
guid_branch_read , 0.0 , true );

const advconfig_checkbox_factory cfg_read_no_title( "<title>" ,
{ 0x59589ffd , 0xf982 , 0x43d7 , { 0xae , 0x5e , 0xee , 0xda , 0xc1 , 0x3 , 0xe , 0xf8 } },
guid_branch_read , 0.0 , true );

const advconfig_checkbox_factory cfg_read_no_creator( "<creator>" ,
{ 0xd6d470ce , 0xafd9 , 0x4201 , { 0xb9 , 0x73 , 0x84 , 0x5a , 0x4f , 0xf8 , 0xa6 , 0xa3 } },
guid_branch_read , 0.0 , true );

const advconfig_checkbox_factory cfg_read_no_album( "<album>" ,
{ 0x6bc14e12 , 0x233e , 0x4b21 , { 0x93 , 0x4c , 0xeb , 0x97 , 0x94 , 0xf8 , 0x85 , 0x4d } },
guid_branch_read , 0.0 , true );

const advconfig_checkbox_factory cfg_read_no_tracknum( "<trackNum>" ,
{ 0xe1d75cda , 0xe4d9 , 0x4cf9 , { 0x93 , 0x33 , 0xf1 , 0x1c , 0x74 , 0x43 , 0x8e , 0x94 } },
guid_branch_read , 0.0 , true );

const advconfig_checkbox_factory cfg_read_mulitple_match( "Allow multiple match (break the spec!)" ,
{ 0xb763cd7b , 0x2444 , 0x4ce9 , { 0x8f , 0xb3 , 0x82 , 0x84 , 0xcd , 0xe7 , 0xf3 , 0x5a } },
guid_branch_read , 1 , false );

// write
const GUID guid_branch_write = { 0xf11e77ca , 0xf764 , 0x4564 , { 0xaf , 0xd8 , 0x0c , 0x58 , 0xa1 , 0xf0 , 0xd3 , 0x09 } };
const advconfig_branch_factory cfg_branch_write( "Write" , guid_branch_write , guid_xspf_branch , 0.0 );

const advconfig_checkbox_factory cfg_write_date( "<date>" ,
{ 0x40c1b282 , 0xf04a , 0x4a69 , { 0x90 , 0xfe , 0x40 , 0x51 , 0xa0 , 0x3e , 0x01 , 0xa2 } } ,
guid_branch_write , 0.0 , false );

const advconfig_checkbox_factory cfg_write_location( "<location>" ,
{ 0xa6b4a958 , 0xeac1 , 0x438e , { 0x98 , 0x61 , 0x21 , 0xc8 , 0xf7 , 0x14 , 0xa7 , 0x1c } } ,
guid_branch_write , 0.0 , true );

const advconfig_checkbox_factory cfg_write_title( "<title>" ,
{ 0x58013a51 , 0x8954 , 0x460e , { 0x82 , 0xe1 , 0xb0 , 0x0 , 0xd , 0xce , 0xb1 , 0x66 } } ,
guid_branch_write , 0.0 , true );

const advconfig_checkbox_factory cfg_write_creator( "<creator>" ,
{ 0x8ef3f81f , 0x6136 , 0x42cf , { 0x9c , 0x84 , 0xb0 , 0xbb , 0xca , 0xd8 , 0x31 , 0x7b } } ,
guid_branch_write , 0.0 , true );

const advconfig_checkbox_factory cfg_write_annotation( "<annotation>" ,
{ 0x715e6c31 , 0xeaaa , 0x4415 , { 0xbd , 0x81 , 0x8c , 0xfb , 0xfc , 0x6b , 0xd , 0x9b } } ,
guid_branch_write , 0.0 , false );

const advconfig_checkbox_factory cfg_write_album( "<album>" ,
{ 0x79ded13f , 0x6924 , 0x489a , { 0xa7 , 0x2a , 0xed , 0x18 , 0xa0 , 0x59 , 0x30 , 0x58 } } ,
guid_branch_write , 0.0 , true );

const advconfig_checkbox_factory cfg_write_tracknum( "<trackNum>" ,
{ 0x73e500c3 , 0x19c5 , 0x4926 , { 0x8b , 0xc7 , 0xd4 , 0x54 , 0xc0 , 0xb8 , 0x50 , 0xb0 } } ,
guid_branch_write , 0.0 , true );

const advconfig_checkbox_factory cfg_write_duration( "<duration>" ,
{ 0xec0b5a32 , 0x120a , 0x4526 , { 0x89 , 0xa2 , 0x9e , 0xa , 0x51 , 0x1f , 0x54 , 0x55 } } ,
guid_branch_write , 0.0 , false );

const advconfig_checkbox_factory cfg_write_compact( "Compact output" ,
{ 0xacd47ae3 , 0xc38 , 0x441d , { 0xac , 0xcf , 0x78 , 0x3b , 0x73 , 0x70 , 0xe4 , 0xe2 } } ,
guid_branch_write , 1 , false );
