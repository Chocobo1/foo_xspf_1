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

typedef metadb_handle_list dbList;


template<class T>
class xmlBaseHelper
{
	public:
		void set( const t_size num , const char *val )
		{
			if( num >= XMLBASE_LEN )
			{
				console::printf( CONSOLE_HEADER"xmlBaseHelper::set() num error: %u" , num );
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

		T get() const
		{
			T out;
			for( const auto &i : base )
			{
				out += i;
			}
			return out;  // RVO kicks in
		}

	private:
		static const t_size XMLBASE_LEN = 4;  // playlist, trackList, track, location
		T base[XMLBASE_LEN];
};
typedef xmlBaseHelper<pfc::string8> xmlBaseImpl;

template<class T>
class lruCache
{
		struct cacheData
		{
			std::string name;
			T data;
		};

	public:
		explicit lruCache(const t_size limit = 50) : CACHE_SIZE(limit)
		{
			return;
		}

		bool set( const char *in_name , const T &in_data )
		{
			// check if exist already
			for( const auto &i : cache )
			{
				if( i.name == in_name )
				{
					return true;
				}
			}

			cache.push_front( { in_name , in_data } );
			if( cache.size() > CACHE_SIZE )
				cache.pop_back();
			return false;
		}

		const T *get( const char *in_name )
		{
			for( auto i = cache.cbegin() , end = cache.cend() ; i != end ; ++i )
			{
				if( i->name == in_name )
				{
					cache.splice( cache.begin() , cache , i );  // move to head
					return &cache.front().data;
				}
			}

			return nullptr;
		}

		void remove( const char *in_name )
		{
			for( auto i = cache.cbegin() , end = cache.cend() ; i != end ; ++i )
			{
				if( i->name == in_name )
				{
					cache.erase(i);
					return;
				}
			}
		}

	private:
		const t_size CACHE_SIZE;
		std::list<cacheData> cache;
};
typedef lruCache<dbList> lruCacheHandleList;

class mainThreadTask : public main_thread_callback
{
	public:
	void add_callback( const int t )
	{
		task_sel = t;
		static_api_ptr_t<main_thread_callback_manager> m;
		m->add_callback( this );
		return;
	}

	void callback_run()  // virtual func overwrite
	{
		static_api_ptr_t<library_manager> m;
		switch( task_sel )
		{
			case 0:
			{
				is_library_enabled.set_value( m->is_library_enabled() );
				break;
			}

			case 1:
			{
				l_1.remove_all();

				m->get_all_items( l_1 );
				list_out.set_value( &l_1 );
				break;
			}

			case 2:
			{
				l_2.remove_all();

				static_api_ptr_t<playlist_incoming_item_filter> p;
				p->process_locations( resolve_list_in , l_2 , false , nullptr , nullptr , NULL );
				resolve_list_out.set_value( &l_2 );
				resolve_list_in.remove_all();
				break;
			}

			default:
			{
				console::printf( CONSOLE_HEADER"Invalid task_sel: %d" , task_sel );
				break;
			}
		};

		task_sel = -1;
		return;
	}

	// 0
	std::promise<bool> is_library_enabled;

	// 1
	std::promise<dbList *> list_out;

	// 2
	pfc::list_t<const char *> resolve_list_in;
	std::promise<dbList * > resolve_list_out;

	private:
	int task_sel = -1;

	// 1
	dbList l_1;

	// 2
	dbList l_2;
};

class trackQueue
{
	public:
	void add( const char *in )
	{
		str_list += in;
		return;
	}

	void resolve( playlist_loader_callback::ptr p_callback )
	{
		// let fb2k handle all input

		if( str_list.get_count() == 0 )
			return;

		service_ptr_t<mainThreadTask> m_task( new service_impl_t<mainThreadTask>() );
		for( t_size i = 0 , max = str_list.get_count() ; i < max ; ++i )
		{
			const char *tmp = str_list.get_item_ref( i );
			m_task->resolve_list_in += tmp;
			p_callback->on_progress( tmp );
		}
		
		auto cb_list = m_task->resolve_list_out.get_future();
		m_task->add_callback( 2 );

		// add
		const dbList l = *( cb_list.get() );
		for( t_size i = 0 , max = l.get_count() ; i < max ; ++i )
		{
			p_callback->on_entry( l.get_item_ref( i ) , playlist_loader_callback::entry_from_playlist , filestats_invalid , false );
		}

		str_list.remove_all();
		return;
	}

	private:
	pfc::list_t<pfc::string8> str_list;
};


void open_helper( const char *p_path , const service_ptr_t<file> &p_file , playlist_loader_callback::ptr p_callback , abort_callback &p_abort );
void open_helper_location( const char *p_path , playlist_loader_callback::ptr p_callback , const tinyxml2::XMLElement *x_track , xmlBaseImpl *xml_base , trackQueue *queue );
void open_helper_no_location( playlist_loader_callback::ptr p_callback , const tinyxml2::XMLElement *x_track , const dbList *in_list , lruCacheHandleList *lru_cache );
void write_helper( const char *p_path , const service_ptr_t<file> &p_file , metadb_handle_list_cref p_data , abort_callback &p_abort );

void addInfoHelper( const tinyxml2::XMLElement *x_parent , file_info_impl *f , const char *x_name , const char *db_name );
void filterFieldHelper( const tinyxml2::XMLElement *x_parent , const dbList *in_list , const char *x_name , const char *db_name , dbList *out , lruCacheHandleList *lru_cache = nullptr );

pfc::string8 pathToUri( const char *in_path , const char *ref_path );
pfc::string8 uriToPath( const char *in_uri , const char *ref_path , const pfc::string8 xbase_str );

pfc::string8 urlEncodeUtf8( const char *in );
pfc::string8 urlDecodeUtf8( const char *in );
