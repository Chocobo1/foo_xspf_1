#pragma once

typedef pfc::list_t < metadb_handle_ptr > dbList;


template<class T>
class myCache
{
		struct cacheData
		{
			std::string name;
			T data;
		};

	public:
		void setCache( const char *in_name , const T *in_data )
		{
			cacheData tmp;
			tmp.name = in_name;
			tmp.data = *in_data;
			cache.push_front( std::move( tmp ) );

			if( cache.size() > CACHE_SIZE )
				cache.pop_back();
			return;
		}

		const T *getCache( const char *in_name )
		{
			for( auto i = cache.cbegin() ; i != cache.cend() ; ++i )
			{
				if( i->name == in_name )
				{
					// move to head
					cache.splice( cache.begin() , cache , i );
					return &cache.front().data;
				}
			}

			return nullptr;
		}

	private:
		static const int CACHE_SIZE = 20;
		std::list<cacheData> cache;
};


void open_helper( const char *p_path , const service_ptr_t<file> &p_file , playlist_loader_callback::ptr p_callback , abort_callback &p_abort );
void open_helper_location( const char *p_path , playlist_loader_callback::ptr p_callback , const tinyxml2::XMLElement *x_track , pfc::string8 xml_base[] );
void open_helper_no_location( playlist_loader_callback::ptr p_callback , const tinyxml2::XMLElement *x_track , const dbList *list , const myCache < dbList > *album_cache );
void write_helper( const char *p_path , const service_ptr_t<file> &p_file , metadb_handle_list_cref p_data , abort_callback &p_abort , const bool w_location );

void addInfoHelper( const tinyxml2::XMLElement *x_parent , file_info_impl *f , const char *x_name , const char *db_name );
void filterFieldHelper( const tinyxml2::XMLElement *x_parent , const dbList *list , const char *x_name , const char *db_name , dbList *out , const myCache < dbList > *album_cache = nullptr );

void pathToUri( const char *in_path , const char *ref_path , pfc::string8 *out );
void uriToPath( const char *in_uri , const char *ref_path , const pfc::string8 base[] , pfc::string8 *out );

void setXmlBase( pfc::string8 base[] , const t_size num , const char *val );
pfc::string8 getXmlBase( const pfc::string8 base[] );

pfc::string8 urlEncodeUtf8( const char *in );
pfc::string8 urlDecodeUtf8( const char *in );
