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

typedef metadb_handle_list DbList;

template<class T>
class XmlBaseHelper
{
	public:
		void set( const t_size num , const char *val )
		{
			if( num >= XMLBASE_SIZE )
			{
				console::printf( CONSOLE_HEADER"XmlBaseHelper::set() num error: %u" , num );
				return;
			}

			base[num] = ( val == nullptr ) ? "" : val ;
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
		static const t_size XMLBASE_SIZE = 4;  // playlist, trackList, track, location
		T base[XMLBASE_SIZE];
};
typedef XmlBaseHelper<pfc::string8> XmlBaseImpl;

template<class T>
class LruCache
{
	struct cacheData
	{
		std::string name;
		T data;
	};

	public:
		explicit LruCache( const size_t limit = 50 ) : CACHE_SIZE( limit )
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
					cache.erase( i );
					return;
				}
			}
		}

	private:
		const size_t CACHE_SIZE;
		std::list<cacheData> cache;
};
typedef LruCache<DbList> LruCacheHandleList;


void open_helper( const char *p_path , const service_ptr_t<file> &p_file , playlist_loader_callback::ptr p_callback , abort_callback &p_abort );
void write_helper( const char *p_path , const service_ptr_t<file> &p_file , metadb_handle_list_cref p_data , abort_callback &p_abort );
