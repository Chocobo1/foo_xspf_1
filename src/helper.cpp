/*
Copyright (c) 2016, Mike Tzou
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

// includes
#include "stdafx.h"
#include "helper.h"
#include "settings.h"

// classes, typedefs
class MainThreadTask : public main_thread_callback
{
	public:
		enum class Task
		{
			IS_LIBRARY_ENABLED,
			GET_ALL_LIBRARY,
			PROCESS_LOCATIONS,
			TASK_MAX
		};

		explicit MainThreadTask( const Task t ) : task_sel( t )
		{
			return;
		}

		void add_callback()
		{
			static_api_ptr_t<main_thread_callback_manager> m;
			m->add_callback( this );
			return;
		}

		void callback_run()  // overwrite virtual func
		{
			// main thread runs here
			switch( task_sel )
			{
				case Task::IS_LIBRARY_ENABLED:
				{
					static_api_ptr_t<library_manager> m;
					is_library_enabled.set_value( m->is_library_enabled() );
					break;
				}

				case Task::GET_ALL_LIBRARY:
				{
					l_1.remove_all();

					static_api_ptr_t<library_manager> m;
					m->get_all_items( l_1 );
					list_out.set_value( &l_1 );
					break;
				}

				case Task::PROCESS_LOCATIONS:
				{
					l_2.remove_all();

					static_api_ptr_t<playlist_incoming_item_filter> p;
					p->process_locations( resolve_list_in , l_2 , false , nullptr , nullptr , NULL );
					resolve_list_out.set_value( &l_2 );
					break;
				}

				default:
				{
					console::printf( CONSOLE_HEADER"Invalid task_sel: %d" , task_sel );
					break;
				}
			};

			return;
		}

		// Task::IS_LIBRARY_ENABLED
		std::promise<bool> is_library_enabled;

		// Task::GET_ALL_LIBRARY
		std::promise<DbList *> list_out;

		// Task::PROCESS_LOCATIONS
		pfc::list_t<const char *> resolve_list_in;
		std::promise<DbList * > resolve_list_out;

	private:
		const Task task_sel;

		// Task::GET_ALL_LIBRARY
		DbList l_1;

		// Task::PROCESS_LOCATIONS
		DbList l_2;
};

class TrackQueue
{
	public:
		void add( const char *in )
		{
			str_list += in;
			return;
		}

		void reset()
		{
			str_list.remove_all();
			return;
		}

		void resolve( playlist_loader_callback::ptr p_callback )
		{
			// let fb2k handle all input

			if( str_list.get_count() == 0 )
				return;

			service_ptr_t<MainThreadTask> m_task( new service_impl_t<MainThreadTask>( MainThreadTask::Task::PROCESS_LOCATIONS ) );

			m_task->resolve_list_in.remove_all();
			for( t_size i = 0 , max = str_list.get_count() ; i < max ; ++i )
			{
				const char *tmp = str_list.get_item_ref( i );
				m_task->resolve_list_in += tmp;
				p_callback->on_progress( tmp );
			}
			auto cb_list = m_task->resolve_list_out.get_future();
			m_task->add_callback();

			// add
			const DbList l = *( cb_list.get() );
			for( t_size i = 0 , max = l.get_count() ; i < max ; ++i )
			{
				p_callback->on_entry( l.get_item_ref( i ) , playlist_loader_callback::entry_from_playlist , filestats_invalid , false );
			}

			this->reset();
			return;
		}

	private:
		pfc::list_t<pfc::string8> str_list;
};

class TrackInfoCache
{
	public:
		explicit TrackInfoCache()
		{
			// get library status
			{
				service_ptr_t<MainThreadTask> m_task( new service_impl_t<MainThreadTask>( MainThreadTask::Task::IS_LIBRARY_ENABLED ) );
				auto h_library = m_task->is_library_enabled.get_future();
				m_task->add_callback();
				have_library = h_library.get();
			}

			// get media library
			if( have_library )
			{
				service_ptr_t<MainThreadTask> m_task( new service_impl_t<MainThreadTask>( MainThreadTask::Task::GET_ALL_LIBRARY ) );
				auto list_ptr = m_task->list_out.get_future();
				m_task->add_callback();
				lib_list.move_from( *( list_ptr.get() ) );
//				lib_list.sort_by_path_quick();
			}

			return;
		}

		void session_restart()
		{
			session_list.remove_all();
			is_first = true;
			return;
		}

		void filter( const char *x_name , const char *x_val , const char *meta_name , const bool use_cache )
		{
			const DbList *list = &session_list;  // the "list" operates on
			if( is_first )
			{
				list = &lib_list;
			}

			LruCacheHandleList *cache_ptr = nullptr;
			if( is_first && use_cache )
			{
				// search for "type name", also insert a new element...
				cache_ptr = &( cache_map[x_name] );
				const auto j = cache_ptr->get( x_val );
				if( j != nullptr )
				{
					list = j;
				}
			}

			// scan through list
			std::multimap< size_t , t_size >out;  // <number of character matches, index in `list`>
			const size_t x_val_len = strlen( x_val );
			for( t_size i = 0 , max = list->get_count() ; i < max ; ++i )
			{
				// get meta string from db
				const auto item = list->get_item_ref( i );
				const auto info = item->get_async_info_ref();
				const char *str = info->info().meta_get( meta_name , 0 );
				if( str == nullptr )
					continue;

				// try best match, case-sensitive
				const bool b_match = ( strcmp( str , x_val ) == 0 ) ? true : false;
				if( b_match )
				{
					out.emplace_hint( out.end() , SIZE_MAX , i );  // special place for best match
					continue;
				}

				// try partial match, case-insensitive
				if( cfg_read_partial_match )
				{
					const size_t str_len = strlen( str );
					const bool p_match = ( str_len > x_val_len ) ? my_strcasestr( str , x_val ) : my_strcasestr( x_val , str );
					if( p_match )
					{
						out.emplace( min( str_len , x_val_len ) , i );
					}
				}
			}

			// handle the results
			DbList out_list;
			for( auto i = out.crbegin() , max = out.crend() ; i != max ; ++i )
			{
				// sorted from most likely to least likely
				out_list += list->get_item_ref( i->second );
			}

			if( is_first && use_cache )
			{
				// store back results
				cache_ptr->set( x_val , out_list );
			}

			session_list.move_from( out_list );
			is_first = false;
			return;
		}

		bool is_library_enabled() const
		{
			return have_library;
		}

		const DbList *getList() const
		{
			return &session_list;
		}

	private:
		DbList lib_list;
		std::map<std::string , LruCacheHandleList> cache_map;

		DbList session_list;
		bool is_first = true;

		bool have_library = false;

		bool my_strcasestr( const char *haystack , const char *needle )
		{
			// haystack & needle needs to be NULL terminated!
			const char *haystack_end = haystack + strlen( haystack );
			const char *needle_end = needle + strlen( needle );
			const auto it = std::search( haystack , haystack_end , needle , needle_end ,
			[]( const char &ch1 , const char &ch2 ) { return ( std::toupper( ch1 ) == std::toupper( ch2 ) ); } );
			return ( it != haystack_end );
		}
};


// prototypes
void openHelperLocation( const char *p_path , playlist_loader_callback::ptr p_callback , const tinyxml2::XMLElement *x_track , XmlBaseImpl *xml_base , TrackQueue *queue );
void openHelperNoLocation( playlist_loader_callback::ptr p_callback , const tinyxml2::XMLElement *x_track , TrackInfoCache *track_cache );

const char* addInfoHelper( file_info_impl *f , const char *meta_name , const tinyxml2::XMLElement *x , const char *e_name );
void filterHelper( const tinyxml2::XMLElement *x , const char *e_name , const char *meta_name , TrackInfoCache *track_cache , const bool use_cache );

tinyxml2::XMLElement *xAddElement( tinyxml2::XMLDocument *x_doc , tinyxml2::XMLNode *x_parent , const char *e_name );
void xAddMeta( tinyxml2::XMLDocument *x_doc , tinyxml2::XMLNode *x_parent , const char *e_name , const char *meta_text );
void xVerifyDocument( tinyxml2::XMLDocument *x , const char *f );
const tinyxml2::XMLElement *xVerifyElement( const tinyxml2::XMLNode *x , const char *e_name );
void xVerifyVersion( const tinyxml2::XMLElement *x );
void xVerifyNamespace( const tinyxml2::XMLElement *x );
const char *xVerifyAttribute( const tinyxml2::XMLElement *x , const char *a_name );
const char *xGetChildElementText( const tinyxml2::XMLElement *x , const char *e_name );

pfc::string8 pathToUri( const bool use_relative , const char *in_path , const char *ref_path );
pfc::string8 uriToPath( const char *in_uri , const char *ref_path , const pfc::string8 xbase_str );

pfc::string8 urlEncodeUtf8( const char *in );
pfc::string8 urlDecodeUtf8( const char *in );


// functions
void open_helper( const char *p_path , const service_ptr_t<file> &p_file , playlist_loader_callback::ptr p_callback , abort_callback &p_abort )
{
	// load file
	pfc::string8 in_file;
	try
	{
		p_file->seek( 0 , p_abort );  // required, said SDK
		p_file->read_string_raw( in_file , p_abort );
	}
	catch( ... )
	{
		console::printf( CONSOLE_HEADER"exception from seek(), read_string_raw()" );
		throw;
	}

	// parse doc
	tinyxml2::XMLDocument x;
	xVerifyDocument( &x , in_file );

	// 4.1.1 playlist
	const auto x_playlist = xVerifyElement( &x , "playlist" );

	// playlist xml:base
	XmlBaseImpl xml_base;
	const char *playlist_base = x_playlist->Attribute( "xml:base" );
	xml_base.set( 0 , playlist_base );

	// 4.1.1.1.1 xmlns
	xVerifyNamespace( x_playlist );

	// 4.1.1.1.2 version
	xVerifyVersion( x_playlist );

	// 4.1.1.2.14 trackList
	const auto x_tracklist = xVerifyElement( x_playlist , "trackList" );

	// trackList xml:base
	const char *tracklist_base = x_tracklist->Attribute( "xml:base" );
	xml_base.set( 1 , tracklist_base );

	// 4.1.1.2.14.1.1 track
	t_size counter = 0;
	TrackQueue t_queue;
	TrackInfoCache track_cache;
	for( auto *x_track = x_tracklist->FirstChildElement( "track" ) ; x_track != nullptr ; x_track = x_track->NextSiblingElement( "track" ) )
	{
		if( p_abort.is_aborting() )
			return;

		// track xml:base
		const char *track_base = x_track->Attribute( "xml:base" );
		xml_base.set( 2 , track_base );

		// 4.1.1.2.14.1.1.1.1 location
		const auto *track_location_text = xGetChildElementText( x_track , "location" );
		if( cfg_read_location && ( track_location_text != nullptr ) )
		{
			// have location
			openHelperLocation( p_path , p_callback , x_track , &xml_base , &t_queue );
		}
		else
		{
			t_queue.resolve( p_callback );  // maintain trackList order

			if( !track_cache.is_library_enabled() )
				continue;

			p_callback->on_progress( ( "track " + std::to_string( counter++ ) ).c_str() );

			openHelperNoLocation( p_callback , x_track , &track_cache );
		}
	}

	t_queue.resolve( p_callback );

	return;
}

void openHelperLocation( const char *p_path , playlist_loader_callback::ptr p_callback , const tinyxml2::XMLElement *x_track , XmlBaseImpl *xml_base , TrackQueue *queue )
{
	const auto *x_track_location = x_track->FirstChildElement( "location" );

	// location xml:base
	const char *track_location_base = x_track_location->Attribute( "xml:base" );
	xml_base->set( 3 , track_location_base );

	const pfc::string8 track_path = uriToPath( x_track_location->GetText() , p_path , xml_base->get() );
	if( track_path.is_empty() )
	{
		console::printf( CONSOLE_HEADER"uriToPath() return empty" );
		return;
	}

	// read <location> as is
	if( cfg_read_no_resolve_loc )
	{
		p_callback->on_progress( track_path );

		// file info variables
		file_info_impl f_info;

		// 4.1.1.2.14.1.1.1.3 title
		if( cfg_read_title )
			addInfoHelper( &f_info , "TITLE" , x_track , "title" );

		// 4.1.1.2.14.1.1.1.4 creator
		if( cfg_read_creator )
			addInfoHelper( &f_info , "ARTIST" , x_track , "creator" );

		// 4.1.1.2.14.1.1.1.5 annotation
		addInfoHelper( &f_info , "COMMENT" , x_track , "annotation" );

		// 4.1.1.2.14.1.1.1.5 album
		if( cfg_read_album )
			addInfoHelper( &f_info , "ALBUM" , x_track , "album" );

		// 4.1.1.2.14.1.1.1.9 trackNum
		long int subsong_number = 0;
		if( cfg_read_tracknum )
		{
			const char* str = addInfoHelper( &f_info , "TRACKNUMBER" , x_track , "trackNum" );

			// special case when <location> leads to another playlist
			bool is_playlist = track_path.has_suffix( ".cue" );
			if( is_playlist )
				subsong_number = max( strtol( str , nullptr , 10 ) , 0);
		}

		// insert into playlist
		metadb_handle_ptr f_handle;
		p_callback->handle_create( f_handle , make_playable_location( track_path , subsong_number ) );
		p_callback->on_entry_info( f_handle , playlist_loader_callback::entry_user_requested , filestats_invalid , f_info , false );

		return;
	}

	// add to queue for later batch process
	queue->add( track_path );
	return;
}

void openHelperNoLocation( playlist_loader_callback::ptr p_callback , const tinyxml2::XMLElement *x_track , TrackInfoCache *track_cache )
{
	// 4.1.1.2.14.1.1.1.5 album
	if( cfg_read_album )
	{
		filterHelper( x_track , "album" , "ALBUM" , track_cache , true );
	}

	// 4.1.1.2.14.1.1.1.4 creator
	if( cfg_read_creator )
	{
		filterHelper( x_track , "creator" , "ARTIST" , track_cache , true );
	}

	// 4.1.1.2.14.1.1.1.9 trackNum
	if( cfg_read_tracknum )
	{
		filterHelper( x_track , "trackNum" , "TRACKNUMBER" , track_cache , true );
	}

	// 4.1.1.2.14.1.1.1.3 title
	if( cfg_read_title )
	{
		filterHelper( x_track , "title" , "TITLE" , track_cache , false );
	}

	// add result
	const auto list = track_cache->getList();
	const t_size list_size = list->get_count();
	const t_size max = ( cfg_read_mulitple_match ? list_size : min( 1 , list_size ) );
	for( t_size i = 0 ; i < max ; ++i )
	{
		p_callback->on_entry( list->get_item_ref( i ) , playlist_loader_callback::entry_from_playlist , filestats_invalid , false );
	}
	
	track_cache->session_restart();
	return;
}

void write_helper( const char *p_path , const service_ptr_t<file> &p_file , metadb_handle_list_cref p_data , abort_callback &p_abort )
{
	// new xml document
	tinyxml2::XMLDocument x;
	//x.SetBOM( true );

	// xml declaration
	auto x_decl = x.NewDeclaration( NULL );
	x.InsertEndChild( x_decl );

	// 4.1.1 playlist
	auto x_playlist = xAddElement( &x , &x , "playlist" );
	x_playlist->SetAttribute( "version" , 1 );
	x_playlist->SetAttribute( "xmlns" , "http://xspf.org/ns/0/" );

	// 4.1.1.2.8 date, XML schema dateTime
	if( cfg_write_date )
	{
		const time_t now = time( NULL );
		char time_buf[24] = { 0 };
		struct tm tmp_tm = { 0 };
		gmtime_s( &tmp_tm , &now );
		strftime( time_buf , ( sizeof( time_buf ) - 1 ) , "%Y-%m-%dT%H:%M:%SZ" , &tmp_tm );
		xAddMeta( &x , x_playlist , "date" , time_buf );
	}

	// 4.1.1.2.14 trackList
	auto x_tracklist = xAddElement( &x , x_playlist , "trackList" );

	// for each track
	for( t_size i = 0 , max = p_data.get_size(); i < max ; ++i )
	{
		if( p_abort.is_aborting() )
			return;

		// fetch track info
		const auto track_item = p_data.get_item( i );
		const auto track_info = track_item->get_async_info_ref();

		// 4.1.1.2.14.1.1 track
		auto x_track = xAddElement( &x , x_tracklist , "track" );

		// 4.1.1.2.14.1.1.1.1 location
		if( cfg_write_location )
		{
			const char *track_path = track_item->get_path();
			const auto track_uri = pathToUri( cfg_write_relative_path , track_path , p_path );
			if( !track_uri.is_empty() )
			{
				xAddMeta( &x , x_track , "location" , track_uri );
			}
		}

		// 4.1.1.2.14.1.1.1.3 title
		if( cfg_write_title )
		{
			const char *str = track_info->info().meta_get( "TITLE" , 0 );
			xAddMeta( &x , x_track , "title" , str );
		}

		// 4.1.1.2.14.1.1.1.4 creator
		if( cfg_write_creator )
		{
			const char *str = track_info->info().meta_get( "ARTIST" , 0 );
			xAddMeta( &x , x_track , "creator" , str );
		}

		// 4.1.1.2.14.1.1.1.5 annotation
		if( cfg_write_annotation )
		{
			const char *str = track_info->info().meta_get( "COMMENT" , 0 );
			xAddMeta( &x , x_track , "annotation" , str );
		}

		// 4.1.1.2.14.1.1.1.8 album
		if( cfg_write_album )
		{
			const char *str = track_info->info().meta_get( "ALBUM" , 0 );
			xAddMeta( &x , x_track , "album" , str );
		}

		// 4.1.1.2.14.1.1.1.9 trackNum
		if( cfg_write_tracknum && track_info->info().meta_exists( "TRACKNUMBER" ) )
		{
			const char *tracknum = track_info->info().meta_get( "TRACKNUMBER" , 0 );
			const long num = strtol( tracknum , NULL , 10 );
			if( num > 0 )
			{
				const auto str = std::to_string( num );
				xAddMeta( &x , x_track , "trackNum" , str.c_str() );
			}
		}

		// 4.1.1.2.14.1.1.1.10 duration, in MILLISECONDS!
		if( cfg_write_duration )
		{
			const double track_len = track_item->get_length() * 1000;
			if( track_len > 0 )
			{
				const auto str = std::to_string( lround( track_len ) );
				xAddMeta( &x , x_track , "duration" , str.c_str() );
			}
		}

		// remove track if no info is added
		if( x_track->FirstChildElement() == nullptr )
			x.DeleteNode( x_track );
	}

	// output
	tinyxml2::XMLPrinter x_printer( nullptr , cfg_write_compact );
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


const char* addInfoHelper( file_info_impl *f , const char *meta_name , const tinyxml2::XMLElement *x , const char* e_name )
{
	// return added data
	const char *str = xGetChildElementText( x , e_name );
	if( str != nullptr )
	{
		f->meta_add( meta_name , str );
		return str;
	}
	return nullptr;
}

void filterHelper( const tinyxml2::XMLElement *x , const char* e_name , const char *meta_name , TrackInfoCache *track_cache , const bool use_cache )
{
	const char *x_str = xGetChildElementText( x , e_name );
	if( x_str != nullptr )
		track_cache->filter( e_name , x_str , meta_name , use_cache );
	return;
}


tinyxml2::XMLElement* xAddElement( tinyxml2::XMLDocument *x_doc , tinyxml2::XMLNode *x_parent , const char * e_name )
{
	tinyxml2::XMLElement *x = x_doc->NewElement( e_name );
	x_parent->InsertEndChild( x );
	return x;
}

void xAddMeta( tinyxml2::XMLDocument *x_doc , tinyxml2::XMLNode *x_parent , const char * e_name , const char *meta_text )
{
	if( meta_text != nullptr )
	{
		tinyxml2::XMLElement *x = xAddElement( x_doc , x_parent , e_name );
		x->SetText( meta_text );
	}
	return;
}

void xVerifyDocument( tinyxml2::XMLDocument *x , const char * f )
{
	const tinyxml2::XMLError ret = x->Parse( f );
	if( ret != tinyxml2::XML_NO_ERROR )
	{
		console::printf( CONSOLE_HEADER"XML parse error id: %d, msg: %s" , ret , x->GetErrorStr1() );
		throw exception_io_data();
	}
	return;
}

const tinyxml2::XMLElement* xVerifyElement( const tinyxml2::XMLNode *x , const char * e_name )
{
	const tinyxml2::XMLElement *x_element = x->FirstChildElement( e_name );
	if( x_element == nullptr )
	{
		console::printf( CONSOLE_HEADER"missing %s element!" , e_name );
		throw exception_io_data();
	}

	return x_element;
}

const char * xVerifyAttribute( const tinyxml2::XMLElement *x , const char * a_name )
{
	const char *x_attribute = x->Attribute( a_name );
	if( x_attribute == nullptr )
	{
		console::printf( CONSOLE_HEADER"missing %s attribute!" , a_name );
		throw exception_io_data();
	}
	return x_attribute;
}

void xVerifyVersion( const tinyxml2::XMLElement *x )
{
	int x_playlist_version = -1;
	x->QueryIntAttribute( "version" , &x_playlist_version );
	if( ( x_playlist_version < 0 ) || ( x_playlist_version > 1 ) )
	{
		console::printf( CONSOLE_HEADER"version error: %d" , x_playlist_version );
		throw exception_io_data();
	}
	return;
}

void xVerifyNamespace( const tinyxml2::XMLElement *x )
{
	const char *x_namespace = xVerifyAttribute( x , "xmlns" );

	const pfc::string8 ns = "http://xspf.org/ns/0/";
	if( strncmp( x_namespace , ns , ns.get_length() ) != 0 )
	{
		console::printf( CONSOLE_HEADER"namespace error: %s" , x_namespace );
		throw exception_io_data();
	}
	return;
}

const char * xGetChildElementText( const tinyxml2::XMLElement *x , const char * e_name )
{
	const auto *x_child = x->FirstChildElement( e_name );
	if( x_child != nullptr )
	{
		return x_child->GetText();
	}
	return nullptr;
}


pfc::string8 pathToUri( const bool use_relative , const char *in_path , const char *ref_path )
{
	pfc::string8 out;

	pfc::string8 path_str = in_path;
	if( !filesystem::g_is_remote_safe( path_str ) )
	{
		// target is local
		// try extract relative path
		pfc::string8 tmp_str;
		if( use_relative && filesystem::g_relative_path_create( in_path , ref_path , tmp_str ) )
		{
			// relative path
			path_str.g_swap( path_str , tmp_str );
			path_str.replace_string( "file://" , "" );
			//console::printf( "rel path_str: %s" , path_str.get_ptr() );
		}
		else
		{
			// absolute path
			path_str.replace_string( "file://" , "file:///" );
			
			// rare case, when loaded a track with <location> while it's not in metadb and fb2k didn't reads its meta yet. the generated xspf playlist will not have "file://" scheme
			if( path_str.has_prefix( "\\" ) )
			{
				path_str.insert_chars( 0 , "file:\\\\" );
			}
			//console::printf( "abs path_str: %s" , path_str.get_ptr() );
		}
		
		// create URI
		path_str.replace_char( '\\' , '/' );  // note: linux can have '\' in file name
		out = urlEncodeUtf8( path_str );
	}
	else
	{
		// target is remote
		// do nothing
		out = in_path;
	}
	/*
	console::printf( "\n" );
	console::printf( "in_path: %s" , in_path );
	console::printf( "ref_path: %s" , ref_path );
	console::printf( "out: %s" , out.get_ptr() );
	console::printf( "\n" );
	*/
	return out;
}

pfc::string8 uriToPath( const char *in_uri , const char *ref_path , const pfc::string8 xbase_str )
{
	pfc::string8 out;

	// add xml:base
	if( !xbase_str.is_empty() )
	{
		out += xbase_str;
	}

	pfc::string8 in_str = urlDecodeUtf8( in_uri );
	const bool is_abs_uri = ( in_str.find_first( ':' ) < in_str.get_length() );
	if( is_abs_uri )
	{
		// have scheme
		const bool is_file_scheme = in_str.has_prefix( "file:" );
		if( is_file_scheme )
		{
			// ex: file://
			in_str.replace_string( "file:///" , "" );
			const bool is_root = ( in_str.find_first( ':' ) >= in_str.get_length() );
			if( is_root )
			{
				// ex: file:///music/a.mp3 -> %SystemDrive%\music\a.mp3 -> \music\a.mp3 (this is ok for fb2k too)
				in_str.insert_chars( 0 , "\\" );
			}
			else
			{
				// ex: file:///c:/music/a.mp3 -> c:\music\a.mp3
				// do nothing
			}
			in_str.replace_string( "/" , "\\" );
		}
		else
		{
			// ex: http://
			// do nothing
		}
	}
	else
	{
		// URI reference
		const bool is_rel_path = !in_str.has_prefix( "/" );  // possible to use unix-like path
		in_str.replace_string( "/" , "\\" );
		if( is_rel_path )
		{
			pfc::string8 ref_path_str = ref_path;
			ref_path_str.replace_string( "file://" , "" );
			ref_path_str.truncate_to_parent_path();
			ref_path_str += "\\";
			ref_path_str += in_str;  // let fb2k handle this mess
			ref_path_str.g_swap( ref_path_str , in_str );
		}
	}

	out += in_str;
	/*
	console::printf( "\n" );
	console::printf( "in_uri: %s" , in_uri );
	console::printf( "ref_path: %s" , ref_path );
	console::printf( "out: %s" , out.get_ptr() );
	console::printf( "\n" );
	*/
	return out;
}


pfc::string8 urlEncodeUtf8( const char *in )
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

pfc::string8 urlDecodeUtf8( const char *in )
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
