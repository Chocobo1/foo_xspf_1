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


#include "stdafx.h"
#include "main.h"
#include "helper.h"


#define PLUGIN_NAME "XSPF Playlist"
#define PLUGIN_VERSION "1.1"
DECLARE_COMPONENT_VERSION
(
	PLUGIN_NAME , PLUGIN_VERSION ,

	PLUGIN_NAME"\n"
	"Compiled on: "__DATE__"\n"
	"https://github.com/Chocobo1/foo_xspf_1\n"
	"This plugin is released under BSD 3-Clause license\n"
	"\n"
	"Mike Tzou (Chocobo1)\n"
	"\n"
	"This plugin links statically with the following open source library:\n"
	"TinyXML-2, http://www.grinninglizard.com/tinyxml2/\n"
);


// settings
static advconfig_checkbox_factory cfg_write_location( "XSPF playlist write location element" ,
{ 0x40c1b282 , 0xf04a , 0x4a69 , { 0x90 , 0xfe , 0x40 , 0x51 , 0xa0 , 0x3e , 0x1 , 0xa2 } } ,
advconfig_branch::guid_branch_tagging , 0.0 , true );

// TODO: add compact output


const char *xspf::get_extension()
{
	return "xspf";
}

bool xspf::is_our_content_type( const char *p_content_type )
{
	const char mime[] = "application/xspf+xml";
	if( strcmp( p_content_type , mime ) == 0 )
		return true;
	return false;
}

bool xspf::is_associatable()
{
	return true;
}

bool xspf::can_write()
{
	return true;
}

void xspf::open( const char *p_path , const service_ptr_t<file> &p_file , playlist_loader_callback::ptr p_callback , abort_callback &p_abort )
{
	open_helper( p_path , p_file , p_callback , p_abort );
	return;
}

void xspf::write( const char *p_path , const service_ptr_t<file> &p_file , metadb_handle_list_cref p_data , abort_callback &p_abort )
{
	write_helper( p_path , p_file , p_data , p_abort , cfg_write_location );
	return;
}







class test : public initquit
{
	public:



		virtual void on_init()
		{
#if 0
			const static_api_ptr_t<library_manager>aaa;

			if( aaa->is_library_enabled() )
				console::printf( "library enabled" );
			else
			{
				console::printf( "library disabled" );
				return;
			}

			pfc::list_t<metadb_handle_ptr> p_out;
			aaa->get_all_items( p_out );

			console::printf( "library size: %d" , p_out.get_size() );

			filterField( &p_out , "ARTIST" , "ANIME THAT JAZZ" );
			//	filterField( &p_out , "TITLE" , "1" );
			filterField( &p_out , "TRACKNUMBER" , "5" );

			console::printf( "after size: %d" , p_out.get_size() );
#endif




			return;
		}




		virtual void on_quit()
		{
			return;
		}
};
initquit_factory_t<test> g_foo;
