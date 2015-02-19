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


#define PLUGIN_NAME "XSPF Playlist"
#define PLUGIN_VERSION "1.0"
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


class playlist_loader_xspf_1 : public playlist_loader
{
		// XSPF spec: http://www.xspf.org/xspf-v1.html

#define CONSOLE_HEADER "foo_xspf_1: "

	public:
		const char *get_extension()
		{
			return "xspf";
		}

		bool is_our_content_type( const char *p_content_type )
		{
			const char mime[] = "application/xspf+xml";
			if( strcmp( p_content_type , mime ) == 0 )
				return true;
			return false;
		}

		bool is_associatable()
		{
			return true;
		}

		bool can_write()
		{
			return true;
		}

		void open( const char *p_path , const service_ptr_t<file> &p_file , playlist_loader_callback::ptr p_callback , abort_callback &p_abort )
		{
			// load file
			pfc::string8 in_file;
			p_file->seek( 0 , p_abort );  // required, said SDK
			p_file->read_string_raw( in_file , p_abort );

			tinyxml2::XMLDocument x;
			auto ret = x.Parse( in_file );
			if( ret != tinyxml2::XML_NO_ERROR )
			{
				console::printf( CONSOLE_HEADER"XML parse error: %d" , ret );
				throw exception_io_data();
			}

			pfc::string8 xml_base[XMLBASE_LEN];

			// 4.1.1 playlist
			const auto *x_playlist = x.FirstChildElement( "playlist" );
			if( x_playlist == nullptr )
			{
				console::printf( CONSOLE_HEADER"missing playlist element!" );
				throw exception_io_data();
			}
			// playlist xml:base
			const char *x_playlist_base = x_playlist->Attribute( "xml:base" );
			setXmlBase( xml_base , 0 , x_playlist_base );

			// 4.1.1.1.1 xmlns
			const char *x_playlist_ns = x_playlist->Attribute( "xmlns" );
			if( x_playlist_ns == nullptr )
			{
				console::printf( CONSOLE_HEADER"missing xmlns attribute!" );
				throw exception_io_data();
			}
			const pfc::string8 ns = "http://xspf.org/ns/0/";
			const int x_playlist_ns_eq = strncmp( x_playlist_ns , ns , ns.get_length() );
			if( x_playlist_ns_eq != 0 )
			{
				console::printf( CONSOLE_HEADER"namespace error: %s" , x_playlist_ns );
				throw exception_io_data();
			}

			// 4.1.1.1.2 version
			int x_playlist_version = -1;
			x_playlist->QueryIntAttribute( "version" , &x_playlist_version );
			if( ( x_playlist_version < 0 ) || ( x_playlist_version > 1 ) )
			{
				console::printf( CONSOLE_HEADER"version error: %d" , x_playlist_version );
				throw exception_io_data();
			}

			// 4.1.1.2.14 trackList
			const auto *x_tracklist = x_playlist->FirstChildElement( "trackList" );
			if( x_tracklist == nullptr )
			{
				console::printf( CONSOLE_HEADER"missing trackList element!" );
				throw exception_io_data();
			}
			// trackList xml:base
			const char *x_tracklist_base = x_tracklist->Attribute( "xml:base" );
			setXmlBase( xml_base , 1 , x_tracklist_base );

			// 4.1.1.2.14.1.1 track
			for( auto *x_track = x_tracklist->FirstChildElement( "track" ) ; x_track != nullptr ; x_track = x_track->NextSiblingElement( "track" ) )
			{
				// file info variables
				metadb_handle_ptr f_handle;
				file_info_impl f_info;

				// track xml:base
				const char *x_track_base = x_track->Attribute( "xml:base" );
				setXmlBase( xml_base , 2 , x_track_base );

				// 4.1.1.2.14.1.1.1.1 location
				bool have_location = false;
				const auto *track_location = x_track->FirstChildElement( "location" );
				if( ( track_location != nullptr ) && ( track_location->GetText() != nullptr ) )
				{
					have_location = true;

					// location xml:base
					const char *track_location_base = track_location->Attribute( "xml:base" );
					setXmlBase( xml_base , 3 , track_location_base );

					// ONLY HANDLE PLAYABLE FILES OR URLS, LINKING TO ANOTHER PLAYLIST IS NOT SUPPORTED
					pfc::string8 out_str;
					uriToLocation( track_location->GetText() , p_path , xml_base , &out_str );
					if( !out_str.is_empty() )
					{
						p_callback->on_progress( out_str );
						p_callback->handle_create( f_handle , make_playable_location( out_str , 0 ) );
					}
				}

				// 4.1.1.2.14.1.1.1.3 title
				const auto *track_title = x_track->FirstChildElement( "title" );
				if( ( track_title != nullptr ) && ( track_title->GetText() != nullptr ) )
				{
					f_info.meta_add( "TITLE" , track_title->GetText() );
				}

				// 4.1.1.2.14.1.1.1.4 creator
				const auto *track_creator = x_track->FirstChildElement( "creator" );
				if( ( track_creator != nullptr ) && ( track_creator->GetText() != nullptr ) )
				{
					f_info.meta_add( "ARTIST" , track_creator->GetText() );
				}

				// 4.1.1.2.14.1.1.1.5 annotation
				const auto *track_annotation = x_track->FirstChildElement( "annotation" );
				if( ( track_annotation != nullptr ) && ( track_annotation->GetText() != nullptr ) )
				{
					f_info.meta_add( "COMMENT" , track_annotation->GetText() );
				}

				// 4.1.1.2.14.1.1.1.5 album
				const auto *track_album = x_track->FirstChildElement( "album" );
				if( ( track_album != nullptr ) && ( track_album->GetText() != nullptr ) )
				{
					f_info.meta_add( "ALBUM" , track_album->GetText() );
				}

				// 4.1.1.2.14.1.1.1.9 trackNum
				const auto *track_num = x_track->FirstChildElement( "trackNum" );
				if( ( track_num != nullptr ) && ( track_num->GetText() != nullptr ) )
				{
					f_info.meta_add( "TRACKNUMBER" , track_num->GetText() );
				}

				// insert into playlist
				if( have_location )
				{
					const t_filestats f_stats = { 0 };
					p_callback->on_entry_info( f_handle , playlist_loader_callback::entry_from_playlist , f_stats , f_info , false );
				}
			}

			return;
		}

		void write( const char *p_path , const service_ptr_t<file> &p_file , metadb_handle_list_cref p_data , abort_callback &p_abort )
		{
			// new xml document
			tinyxml2::XMLDocument x;
			//x.SetBOM( true );

			// xml declaration
			auto x_decl = x.NewDeclaration( NULL );
			x.InsertEndChild( x_decl );

			// 4.1.1 playlist
			auto x_playlist = x.NewElement( "playlist" );
			x.InsertEndChild( x_playlist );
			x_playlist->SetAttribute( "version" , 1 );
			x_playlist->SetAttribute( "xmlns" , "http://xspf.org/ns/0/" );

			/*
			// 4.1.1.2.8 date,  XML schema dateTime
			const time_t now = time( NULL );
			char time_buf[24] = { 0 };
			struct tm tmp_tm = { 0 };
			gmtime_s( &tmp_tm , &now );
			strftime( time_buf , ( sizeof( time_buf ) - 1 ) , "%Y-%m-%dT%H:%M:%SZ" , &tmp_tm );

			auto x_date = x.NewElement( "date" );
			x_playlist->InsertEndChild( x_date );
			x_date->SetText( time_buf );
			*/

			// 4.1.1.2.14 trackList
			auto x_tracklist = x.NewElement( "trackList" );
			x_playlist->InsertEndChild( x_tracklist );

			// for each track
			const t_size num_entries = p_data.get_size();
			for( t_size i = 0 ; i < num_entries ; ++i )
			{
				// fetch track info
				const metadb_handle_ptr track_item = p_data.get_item( i );
				file_info_impl track_info;
				bool ret = track_item->get_info( track_info );
				if( !ret )
				{
					console::printf( CONSOLE_HEADER"get_info error" );
					continue;
				}

				// 4.1.1.2.14.1.1 track
				auto x_track = x.NewElement( "track" );
				x_tracklist->InsertEndChild( x_track );

				// 4.1.1.2.14.1.1.1.1 location
				const char *item_path = track_item->get_path();
				pfc::string8 track_path;
				locationToUri( item_path , p_path , &track_path );
				if( !track_path.is_empty() )
				{
					auto track_location = x.NewElement( "location" );
					x_track->InsertEndChild( track_location );
					track_location->SetText( track_path );
				}

				// 4.1.1.2.14.1.1.1.3 title
				if( track_info.meta_exists( "TITLE" ) )
				{
					const char *str = track_info.meta_get( "TITLE" , 0 );
					auto track_title = x.NewElement( "title" );
					x_track->InsertEndChild( track_title );
					track_title->SetText( str );
				}

				// 4.1.1.2.14.1.1.1.4 creator
				if( track_info.meta_exists( "ARTIST" ) )
				{
					const char *str = track_info.meta_get( "ARTIST" , 0 );
					auto track_creator = x.NewElement( "creator" );
					x_track->InsertEndChild( track_creator );
					track_creator->SetText( str );
				}

				// 4.1.1.2.14.1.1.1.5 annotation
				if( track_info.meta_exists( "COMMENT" ) )
				{
					const char *str = track_info.meta_get( "COMMENT" , 0 );
					auto track_annotation = x.NewElement( "annotation" );
					x_track->InsertEndChild( track_annotation );
					track_annotation->SetText( str );
				}

				// 4.1.1.2.14.1.1.1.8 album
				if( track_info.meta_exists( "ALBUM" ) )
				{
					const char *str = track_info.meta_get( "ALBUM" , 0 );
					auto track_album = x.NewElement( "album" );
					x_track->InsertEndChild( track_album );
					track_album->SetText( str );
				}

				// 4.1.1.2.14.1.1.1.9 trackNum
				if( track_info.meta_exists( "TRACKNUMBER" ) )
				{
					const char *str = track_info.meta_get( "TRACKNUMBER" , 0 );
					const long int num = strtol( str , NULL , 10 );
					if( num > 0 )
					{
						auto track_tracknum = x.NewElement( "trackNum" );
						x_track->InsertEndChild( track_tracknum );
						track_tracknum->SetText( num );
					}
				}

				// 4.1.1.2.14.1.1.1.10 duration, in MILLISECONDS!
				const double track_len = track_item->get_length() * 1000;
				if( track_len > 0 )
				{
					auto track_duration = x.NewElement( "duration" );
					x_track->InsertEndChild( track_duration );
					track_duration->SetText( ( t_size ) track_len );
				}
			}

			// output
			tinyxml2::XMLPrinter x_printer;
			x.Print( &x_printer );
			try
			{
				p_file->write_string_raw( x_printer.CStr() , p_abort );
			}
			catch( ... )
			{
				//try {filesystem::g_remove(p_path,p_abort);} catch(...) {}
				console::printf( CONSOLE_HEADER"write_string_raw exception" );
				throw;
			}

			return;
		}


	private:
		void locationToUri( const char *in_path , const char *ref_path , pfc::string8 *out ) const
		{
			out->reset();

			pfc::string8 path_str = in_path;
			if( !filesystem::g_is_remote_safe( in_path ) )
			{
				// local path
				// try extract relative path
				pfc::string8 tmp_str;
				if( filesystem::g_relative_path_create( in_path , ref_path , tmp_str ) )
				{
					// have relative path
					path_str = tmp_str;
				}
				path_str.replace_string( "file://" , "file:///" );
			}

			// create URI
			path_str.replace_char( '\\' , '/' );  // note: linux can have '\' in file name
			*out = urlEncodeUtf8( path_str );

			return;
		}

		void uriToLocation( const char *in_uri , const char *ref_path , const pfc::string8 base[] , pfc::string8 *out ) const
		{
			out->reset();

			// add xml:base
			const pfc::string8 base_path = getXmlBase( base );
			if( !base_path.is_empty() )
			{
				*out += base_path;
			}

			// check "file:" scheme
			pfc::string8 in_str = urlDecodeUtf8( in_uri );
			const bool is_local = in_str.has_prefix( "file:" );
			if( is_local )
			{
				// prepare
				in_str.replace_string( "file:///" , "" );
				in_str.replace_string( "/" , "\\" );

				// check if absolute path
				const t_size col_pos = in_str.find_first( ':' );
				const bool is_relative_path = ( col_pos < in_str.get_length() ) ? false : true ;
				if( is_relative_path )
				{
					// add relative path
					pfc::string8 rel_path = ref_path;
					rel_path.truncate_to_parent_path();
					rel_path.fix_dir_separator();
					*out += rel_path;
				}
			}
			*out += in_str;

			return;
		}


		void setXmlBase( pfc::string8 base[] , const t_size num , const char *val )
		{
			if( ( num < 0 ) || ( num >= XMLBASE_LEN ) )
			{
				console::printf( CONSOLE_HEADER"setXmlBase num error: %d" , num );
				return;
			}

			if( val == nullptr )
			{
				base[num].reset();
				return;
			}

			base[num] = val;
			return;
		}

		pfc::string8 getXmlBase( const pfc::string8 base[] ) const
		{
			pfc::string8 out;
			for( t_size i = 0 ; i < XMLBASE_LEN ; ++i )
			{
				out += base[i];
			}
			return out;
		}


		pfc::string8 urlEncodeUtf8( const char *in ) const
		{
			// percent-encoding
			// `in` must be a utf-8 encoded & null terminated string

			std::string out;
			for( const char *i = in ; *i != '\0' ; )
			{
				if( ( *i >= 0 ) && ( isalnum( *i ) || ( *i == '-' ) || ( *i == '_' ) || ( *i == '.' ) || ( *i == '~' ) /*special case for URI, safe for windows*/ || ( *i == '/' ) || ( *i == ':' ) ) )
				{
					// RFC 3986 section 2.3, Unreserved Characters
					out += *i;
					++i;
				}
				else
				{
					int byte_len = 4;
					if( ( unsigned char ) *i < 192 )
						byte_len = 1;
					else if( ( unsigned char ) *i < 224 )
						byte_len = 2;
					else if( ( unsigned char ) *i < 240 )
						byte_len = 3;
					for( int j = 0; ( j < byte_len ) && ( *( i + j ) != '\0' ) ; ++j )
					{
						char tmp[4] = { 0 };
						sprintf_s( tmp , "%%%X" , ( unsigned char ) * ( i ) );
						out += tmp;
						++i;
					}
				}
			}

			return out.c_str();
		}

		pfc::string8 urlDecodeUtf8( const char *in ) const
		{
			// `in` must be a percent-encoded utf-8 & null terminated string

			std::string out;
			for( const char *i = in ; *i != '\0' ; )
			{
				if( *i == '%' )
				{
					const char tmp[] = { *( i + 1 ) , *( i + 2 ) , '\0' };
					out += ( char ) strtol( tmp , NULL , 16 );
					i += 3;
				}
				else
				{
					out += *i;
					++i;
				}
			}

			return out.c_str();
		}


		static const t_size XMLBASE_LEN = 4;  // 1 string for each playlist.trackList.track.location
};


playlist_loader_factory_t<playlist_loader_xspf_1> foo_xspf_1;
